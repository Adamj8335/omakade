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
  beginResetModel();
  m_models.append(model);
  endResetModel();

  connect(model, &QAbstractItemModel::modelReset, this, [this] {
    beginResetModel();
    endResetModel();
  });
  connect(model, &QAbstractItemModel::rowsInserted, this, [this] {
    beginResetModel();
    endResetModel();
  });
  connect(model, &QAbstractItemModel::rowsRemoved, this, [this] {
    beginResetModel();
    endResetModel();
  });
  connect(model, &QAbstractItemModel::dataChanged, this,
          [this, model](const QModelIndex& topLeft, const QModelIndex& bottomRight,
                        const QList<int>& roles) {
            const int offset = offsetFor(model);
            if (offset >= 0) {
              emit dataChanged(index(offset + topLeft.row()), index(offset + bottomRight.row()),
                               roles);
            }
          });
}

int UnifiedGameModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  int count = 0;
  for (const QAbstractItemModel* model : m_models) {
    count += model->rowCount();
  }
  return count;
}

QVariant UnifiedGameModel::data(const QModelIndex& index, int role) const {
  const SourceRow source = mapRow(index.row());
  if (source.model == nullptr) {
    return {};
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
  return roles;
}

void UnifiedGameModel::toggleFavorite(int row) {
  const SourceRow source = mapRow(row);
  if (source.model != nullptr) {
    QMetaObject::invokeMethod(source.model, "toggleFavorite", Q_ARG(int, source.row));
  }
}

void UnifiedGameModel::toggleHidden(int row) {
  const SourceRow source = mapRow(row);
  if (source.model != nullptr) {
    QMetaObject::invokeMethod(source.model, "toggleHidden", Q_ARG(int, source.row));
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

UnifiedGameModel::SourceRow UnifiedGameModel::mapRow(int row) const {
  if (row < 0) {
    return {};
  }
  for (QAbstractItemModel* model : m_models) {
    if (row < model->rowCount()) {
      return {.model = model, .row = row};
    }
    row -= model->rowCount();
  }
  return {};
}

int UnifiedGameModel::offsetFor(const QAbstractItemModel* model) const {
  int offset = 0;
  for (const QAbstractItemModel* candidate : m_models) {
    if (candidate == model) {
      return offset;
    }
    offset += candidate->rowCount();
  }
  return -1;
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
  loadArtworkOverrides();
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
