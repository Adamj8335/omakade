#include "library/UnifiedGameModel.h"

#include "library/GameRoles.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QSaveFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include <algorithm>

namespace {
constexpr qint64 kMaximumArtworkBytes = 32 * 1024 * 1024;
constexpr qint64 kMaximumArtworkPixels = 64 * 1024 * 1024;

QString localUrl(const QString& path) {
  return path.isEmpty() ? QString{} : QUrl::fromLocalFile(path).toString();
}

QString runnerFor(const QModelIndex& game) {
  const QString runner = game.data(GameRoles::Runner).toString();
  return runner.isNull() ? QStringLiteral("") : runner;
}
} // namespace

UnifiedGameModel::UnifiedGameModel(const QString& databasePath, QObject* parent)
    : QAbstractListModel(parent),
      m_connectionName(QStringLiteral("omakade-artwork-%1").arg(QUuid::createUuid().toString())) {
  if (!databasePath.isEmpty()) {
    openArtworkDatabase(databasePath);
  }
}

UnifiedGameModel::~UnifiedGameModel() {
  if (m_database.isValid()) {
    m_database.close();
    m_database = {};
    QSqlDatabase::removeDatabase(m_connectionName);
  }
}

void UnifiedGameModel::addSourceModel(QAbstractItemModel* model) {
  if (model == nullptr || m_models.contains(model)) {
    return;
  }
  m_models.append(model);
  rebuildRows();

  connect(model, &QAbstractItemModel::modelReset, this, &UnifiedGameModel::rebuildRows);
  connect(model, &QAbstractItemModel::rowsInserted, this, &UnifiedGameModel::rebuildRows);
  connect(model, &QAbstractItemModel::rowsRemoved, this, &UnifiedGameModel::rebuildRows);
  connect(model, &QAbstractItemModel::dataChanged, this, [this] {
    if (!m_rows.isEmpty()) {
      emit dataChanged(index(0), index(m_rows.size() - 1));
    }
  });
}

int UnifiedGameModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_rows.size();
}

QVariant UnifiedGameModel::data(const QModelIndex& index, int role) const {
  const SourceRow source = mapRow(index.row());
  if (source.model == nullptr) {
    return {};
  }
  const QVector<SourceRow> members = groupRows(source);
  if (role == GameRoles::Linked) {
    return members.size() > 1;
  }
  if (role == GameRoles::LinkedSources) {
    QStringList sources;
    for (const SourceRow& member : members) {
      const QString name = member.model->index(member.row, 0).data(GameRoles::Source).toString();
      if (!sources.contains(name)) {
        sources.append(name);
      }
    }
    return sources.join(QStringLiteral(" + "));
  }
  if (role == GameRoles::Favorite || role == GameRoles::Recent || role == GameRoles::Hidden) {
    bool value = role == GameRoles::Hidden;
    for (const SourceRow& member : members) {
      const bool memberValue = member.model->index(member.row, 0).data(role).toBool();
      value = role == GameRoles::Hidden ? value && memberValue : value || memberValue;
    }
    return value;
  }
  if (role == GameRoles::Hours) {
    int hours = 0;
    for (const SourceRow& member : members) {
      hours = std::max(hours, member.model->index(member.row, 0).data(role).toInt());
    }
    return hours;
  }
  const QString override = m_coverOverrides.value(gameKey(source));
  if (role == GameRoles::CoverPath && QFileInfo::exists(override)) {
    return localUrl(override);
  }
  if (role == GameRoles::CustomCover) {
    return QFileInfo::exists(override);
  }
  return source.model->index(source.row, 0).data(role);
}

QHash<int, QByteArray> UnifiedGameModel::roleNames() const {
  QHash<int, QByteArray> roles =
      m_models.isEmpty() ? QHash<int, QByteArray>{} : m_models.constFirst()->roleNames();
  roles.insert(GameRoles::CustomCover, "customCover");
  roles.insert(GameRoles::Linked, "linked");
  roles.insert(GameRoles::LinkedSources, "linkedSources");
  return roles;
}

void UnifiedGameModel::toggleFavorite(int row) {
  const SourceRow source = mapRow(row);
  if (source.model == nullptr) {
    return;
  }
  const bool desired = !data(index(row), GameRoles::Favorite).toBool();
  for (const SourceRow& member : groupRows(source)) {
    if (member.model->index(member.row, 0).data(GameRoles::Favorite).toBool() != desired) {
      QMetaObject::invokeMethod(member.model, "toggleFavorite", Q_ARG(int, member.row));
    }
  }
}

void UnifiedGameModel::toggleHidden(int row) {
  const SourceRow source = mapRow(row);
  if (source.model == nullptr) {
    return;
  }
  const bool desired = !data(index(row), GameRoles::Hidden).toBool();
  for (const SourceRow& member : groupRows(source)) {
    if (member.model->index(member.row, 0).data(GameRoles::Hidden).toBool() != desired) {
      QMetaObject::invokeMethod(member.model, "toggleHidden", Q_ARG(int, member.row));
    }
  }
}

bool UnifiedGameModel::setCustomCover(int row, const QUrl& sourceUrl) {
  const SourceRow source = mapRow(row);
  const QString sourcePath = sourceUrl.toLocalFile();
  const QFileInfo sourceInfo(sourcePath);
  if (source.model == nullptr || !m_database.isOpen() || !sourceInfo.isFile() ||
      sourceInfo.size() <= 0 || sourceInfo.size() > kMaximumArtworkBytes) {
    return false;
  }

  QImageReader reader(sourcePath);
  const QSize size = reader.size();
  const QByteArray format = reader.format().toLower();
  const bool supported = format == "jpg" || format == "jpeg" || format == "png" || format == "webp";
  if (!supported || !size.isValid() || size.width() > 16384 || size.height() > 16384 ||
      static_cast<qint64>(size.width()) * size.height() > kMaximumArtworkPixels) {
    return false;
  }

  const QString key = gameKey(source);
  if (key.isEmpty() || !QDir().mkpath(m_artworkRoot)) {
    return false;
  }
  const QString extension = format == "jpeg" ? QStringLiteral("jpg") : QString::fromLatin1(format);
  QFile input(sourcePath);
  if (!input.open(QIODevice::ReadOnly)) {
    return false;
  }
  const QByteArray contents = input.readAll();
  const QString digest = QString::fromLatin1(
      QCryptographicHash::hash(key.toUtf8() + contents, QCryptographicHash::Sha256).toHex());
  const QString destination =
      m_artworkRoot + QLatin1Char('/') + digest + QLatin1Char('.') + extension;
  QSaveFile output(destination);
  if (!output.open(QIODevice::WriteOnly) || output.write(contents) != contents.size() ||
      !output.commit()) {
    return false;
  }

  const QModelIndex game = source.model->index(source.row, 0);
  QSqlQuery query(m_database);
  query.prepare(
      QStringLiteral("INSERT OR REPLACE INTO artwork_overrides(source, runner, app_id, cover_path) "
                     "VALUES(?, ?, ?, ?)"));
  query.addBindValue(game.data(GameRoles::Source).toString());
  query.addBindValue(runnerFor(game));
  query.addBindValue(game.data(GameRoles::AppId).toString());
  query.addBindValue(destination);
  if (!query.exec()) {
    QFile::remove(destination);
    return false;
  }

  const QString previous = m_coverOverrides.value(key);
  m_coverOverrides.insert(key, destination);
  if (!previous.isEmpty() && previous != destination) {
    QFile::remove(previous);
  }
  emit dataChanged(index(row), index(row), {GameRoles::CoverPath, GameRoles::CustomCover});
  return true;
}

bool UnifiedGameModel::resetCustomCover(int row) {
  const SourceRow source = mapRow(row);
  const QString key = gameKey(source);
  if (source.model == nullptr || !m_database.isOpen() || !m_coverOverrides.contains(key)) {
    return false;
  }
  const QModelIndex game = source.model->index(source.row, 0);
  QSqlQuery query(m_database);
  query.prepare(QStringLiteral(
      "DELETE FROM artwork_overrides WHERE source = ? AND runner = ? AND app_id = ?"));
  query.addBindValue(game.data(GameRoles::Source).toString());
  query.addBindValue(runnerFor(game));
  query.addBindValue(game.data(GameRoles::AppId).toString());
  if (!query.exec()) {
    return false;
  }
  const QString path = m_coverOverrides.take(key);
  QFile::remove(path);
  emit dataChanged(index(row), index(row), {GameRoles::CoverPath, GameRoles::CustomCover});
  return true;
}

QVariantList UnifiedGameModel::installations(int row) const {
  QVariantList result;
  const SourceRow source = mapRow(row);
  for (const SourceRow& member : groupRows(source)) {
    result.append(gameMap(member));
  }
  return result;
}

QVariantList UnifiedGameModel::linkCandidates(int row, const QString& search) const {
  QVariantList result;
  const SourceRow selected = mapRow(row);
  if (selected.model == nullptr) {
    return result;
  }
  QString query = search.trimmed();
  const QString selectedGroup = m_groupForGame.value(gameKey(selected));
  for (const SourceRow& candidate : m_rows) {
    const QString candidateKey = gameKey(candidate);
    if (candidateKey == gameKey(selected) ||
        (!selectedGroup.isEmpty() && m_groupForGame.value(candidateKey) == selectedGroup)) {
      continue;
    }
    const QModelIndex game = candidate.model->index(candidate.row, 0);
    if (!query.isEmpty() &&
        !game.data(GameRoles::Title).toString().contains(query, Qt::CaseInsensitive)) {
      continue;
    }
    result.append(gameMap(candidate));
    if (result.size() == 50) {
      break;
    }
  }
  return result;
}

bool UnifiedGameModel::linkGames(int row, const QString& sourceName, const QString& runner,
                                 const QString& appId) {
  const SourceRow selected = mapRow(row);
  SourceRow target;
  for (QAbstractItemModel* model : m_models) {
    for (int sourceRow = 0; sourceRow < model->rowCount(); ++sourceRow) {
      const QModelIndex game = model->index(sourceRow, 0);
      if (game.data(GameRoles::Source).toString() == sourceName && runnerFor(game) == runner &&
          game.data(GameRoles::AppId).toString() == appId) {
        target = {.model = model, .row = sourceRow};
        break;
      }
    }
    if (target.model != nullptr) {
      break;
    }
  }
  const QString selectedKey = gameKey(selected);
  const QString targetKey = gameKey(target);
  if (selectedKey.isEmpty() || targetKey.isEmpty() || selectedKey == targetKey ||
      !m_database.isOpen() ||
      (!m_groupForGame.value(selectedKey).isEmpty() &&
       m_groupForGame.value(selectedKey) == m_groupForGame.value(targetKey))) {
    return false;
  }

  const QString selectedGroup = m_groupForGame.value(selectedKey);
  const QString targetGroup = m_groupForGame.value(targetKey);
  const QString groupId = QUuid::createUuid().toString(QUuid::WithoutBraces);
  if (!m_database.transaction()) {
    return false;
  }
  QSqlQuery merge(m_database);
  merge.prepare(QStringLiteral(
      "UPDATE game_link_members SET group_id = ?, is_primary = 0 WHERE group_id = ?"));
  for (const QString& existingGroup : {selectedGroup, targetGroup}) {
    if (existingGroup.isEmpty()) {
      continue;
    }
    merge.bindValue(0, groupId);
    merge.bindValue(1, existingGroup);
    if (!merge.exec()) {
      m_database.rollback();
      return false;
    }
  }
  QSqlQuery insert(m_database);
  insert.prepare(QStringLiteral(
      "INSERT OR REPLACE INTO game_link_members(group_id, source, runner, app_id, is_primary) "
      "VALUES(?, ?, ?, ?, ?)"));
  for (const SourceRow& member : {selected, target}) {
    const QModelIndex game = member.model->index(member.row, 0);
    insert.bindValue(0, groupId);
    insert.bindValue(1, game.data(GameRoles::Source).toString());
    insert.bindValue(2, runnerFor(game));
    insert.bindValue(3, game.data(GameRoles::AppId).toString());
    insert.bindValue(4, gameKey(member) == selectedKey);
    if (!insert.exec()) {
      m_database.rollback();
      return false;
    }
  }
  if (!m_database.commit()) {
    return false;
  }
  loadLinks();
  rebuildRows();
  return true;
}

bool UnifiedGameModel::unlinkGames(int row) {
  const SourceRow source = mapRow(row);
  const QString groupId = m_groupForGame.value(gameKey(source));
  if (groupId.isEmpty() || !m_database.isOpen()) {
    return false;
  }
  QSqlQuery query(m_database);
  query.prepare(QStringLiteral("DELETE FROM game_link_members WHERE group_id = ?"));
  query.addBindValue(groupId);
  if (!query.exec()) {
    return false;
  }
  loadLinks();
  rebuildRows();
  return true;
}

UnifiedGameModel::SourceRow UnifiedGameModel::mapRow(int row) const {
  if (row < 0 || row >= m_rows.size()) {
    return {};
  }
  return m_rows.at(row);
}

QString UnifiedGameModel::gameKey(const SourceRow& source) const {
  if (source.model == nullptr) {
    return {};
  }
  const QModelIndex game = source.model->index(source.row, 0);
  const QString gameSource = game.data(GameRoles::Source).toString();
  const QString appId = game.data(GameRoles::AppId).toString();
  if (gameSource.isEmpty() || appId.isEmpty()) {
    return {};
  }
  return gameSource + QChar::Null + runnerFor(game) + QChar::Null + appId;
}

UnifiedGameModel::SourceRow UnifiedGameModel::sourceForKey(const QString& key) const {
  for (QAbstractItemModel* model : m_models) {
    for (int row = 0; row < model->rowCount(); ++row) {
      const SourceRow source{.model = model, .row = row};
      if (gameKey(source) == key) {
        return source;
      }
    }
  }
  return {};
}

QVector<UnifiedGameModel::SourceRow> UnifiedGameModel::groupRows(const SourceRow& source) const {
  QVector<SourceRow> rows;
  if (source.model == nullptr) {
    return rows;
  }
  rows.append(source);
  const QString groupId = m_groupForGame.value(gameKey(source));
  if (groupId.isEmpty()) {
    return rows;
  }
  for (QAbstractItemModel* model : m_models) {
    for (int row = 0; row < model->rowCount(); ++row) {
      const SourceRow candidate{.model = model, .row = row};
      if (gameKey(candidate) != gameKey(source) &&
          m_groupForGame.value(gameKey(candidate)) == groupId) {
        rows.append(candidate);
      }
    }
  }
  return rows;
}

QVariantMap UnifiedGameModel::gameMap(const SourceRow& source) const {
  QVariantMap result;
  if (source.model == nullptr) {
    return result;
  }
  const QModelIndex game = source.model->index(source.row, 0);
  const auto roles = source.model->roleNames();
  for (auto iterator = roles.cbegin(); iterator != roles.cend(); ++iterator) {
    result.insert(QString::fromUtf8(iterator.value()), game.data(iterator.key()));
  }
  const QString override = m_coverOverrides.value(gameKey(source));
  if (QFileInfo::exists(override)) {
    result.insert(QStringLiteral("coverPath"), localUrl(override));
    result.insert(QStringLiteral("customCover"), true);
  } else {
    result.insert(QStringLiteral("customCover"), false);
  }
  return result;
}

void UnifiedGameModel::rebuildRows() {
  beginResetModel();
  m_rows.clear();
  QSet<QString> addedGroups;
  for (QAbstractItemModel* model : m_models) {
    for (int row = 0; row < model->rowCount(); ++row) {
      const SourceRow source{.model = model, .row = row};
      const QString groupId = m_groupForGame.value(gameKey(source));
      if (groupId.isEmpty()) {
        m_rows.append(source);
      } else if (!addedGroups.contains(groupId)) {
        SourceRow representative = sourceForKey(m_primaryForGroup.value(groupId));
        m_rows.append(representative.model == nullptr ? source : representative);
        addedGroups.insert(groupId);
      }
    }
  }
  endResetModel();
}

bool UnifiedGameModel::openArtworkDatabase(const QString& path) {
  m_databasePath = path;
  m_artworkRoot = QFileInfo(path).absolutePath() + QStringLiteral("/artwork");
  m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
  m_database.setDatabaseName(path);
  if (!m_database.open()) {
    return false;
  }
  QSqlQuery query(m_database);
  if (!query.exec(QStringLiteral(
          "CREATE TABLE IF NOT EXISTS artwork_overrides (source TEXT NOT NULL, runner TEXT NOT "
          "NULL, app_id TEXT NOT NULL, cover_path TEXT NOT NULL, PRIMARY KEY(source, runner, "
          "app_id))"))) {
    return false;
  }
  if (!query.exec(QStringLiteral(
          "CREATE TABLE IF NOT EXISTS game_link_members (group_id TEXT NOT NULL, source TEXT NOT "
          "NULL, runner TEXT NOT NULL, app_id TEXT NOT NULL, is_primary INTEGER NOT NULL DEFAULT "
          "0, PRIMARY KEY(source, runner, app_id))"))) {
    return false;
  }
  loadArtworkOverrides();
  loadLinks();
  return true;
}

void UnifiedGameModel::loadArtworkOverrides() {
  QSqlQuery query(m_database);
  if (!query.exec(
          QStringLiteral("SELECT source, runner, app_id, cover_path FROM artwork_overrides"))) {
    return;
  }
  while (query.next()) {
    const QString key = query.value(0).toString() + QChar::Null + query.value(1).toString() +
                        QChar::Null + query.value(2).toString();
    m_coverOverrides.insert(key, query.value(3).toString());
  }
}

void UnifiedGameModel::loadLinks() {
  m_groupForGame.clear();
  m_primaryForGroup.clear();
  QSqlQuery query(m_database);
  if (!query.exec(QStringLiteral(
          "SELECT group_id, source, runner, app_id, is_primary FROM game_link_members"))) {
    return;
  }
  while (query.next()) {
    const QString groupId = query.value(0).toString();
    const QString key = query.value(1).toString() + QChar::Null + query.value(2).toString() +
                        QChar::Null + query.value(3).toString();
    m_groupForGame.insert(key, groupId);
    if (query.value(4).toBool()) {
      m_primaryForGroup.insert(groupId, key);
    }
  }
}
