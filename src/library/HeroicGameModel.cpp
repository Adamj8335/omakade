#include "library/HeroicGameModel.h"

#include "library/GameRoles.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

namespace {
QColor colorFor(const QString& key, int offset) {
  const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha256);
  return QColor::fromHsl((static_cast<unsigned char>(hash.at(offset)) * 359) / 255, 115,
                         offset == 0 ? 105 : 72);
}

QString localUrl(const QString& path) {
  return path.isEmpty() ? QString{} : QUrl::fromLocalFile(path).toString();
}

QString storeName(const QString& runner) {
  if (runner == QStringLiteral("legendary")) {
    return QStringLiteral("Epic");
  }
  if (runner == QStringLiteral("gog")) {
    return QStringLiteral("GOG");
  }
  if (runner == QStringLiteral("nile")) {
    return QStringLiteral("Amazon");
  }
  return QStringLiteral("Heroic");
}
} // namespace

HeroicGameModel::HeroicGameModel(const QString& omakadeDatabasePath, QObject* parent)
    : QAbstractListModel(parent),
      m_connectionName(QStringLiteral("omakade-heroic-%1").arg(reinterpret_cast<quintptr>(this))) {
  if (openDatabase(omakadeDatabasePath) && ensureSchema()) {
    loadDatabase();
  }
}

HeroicGameModel::~HeroicGameModel() {
  m_database.close();
  m_database = {};
  QSqlDatabase::removeDatabase(m_connectionName);
}

int HeroicGameModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_games.size());
}

QVariant HeroicGameModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_games.size()) {
    return {};
  }
  return valueForRole(m_games.at(index.row()), role);
}

QHash<int, QByteArray> HeroicGameModel::roleNames() const {
  return {{GameRoles::Title, "title"},
          {GameRoles::Subtitle, "subtitle"},
          {GameRoles::Description, "description"},
          {GameRoles::Hours, "hours"},
          {GameRoles::Progress, "progress"},
          {GameRoles::AchievementsUnlocked, "achievementsUnlocked"},
          {GameRoles::AchievementsTotal, "achievementsTotal"},
          {GameRoles::Favorite, "favorite"},
          {GameRoles::Recent, "recent"},
          {GameRoles::AccentStart, "accentStart"},
          {GameRoles::AccentEnd, "accentEnd"},
          {GameRoles::CoverMark, "coverMark"},
          {GameRoles::Year, "year"},
          {GameRoles::AppId, "appId"},
          {GameRoles::CoverPath, "coverPath"},
          {GameRoles::HeroPath, "heroPath"},
          {GameRoles::LogoPath, "logoPath"},
          {GameRoles::InstallPath, "installPath"},
          {GameRoles::Source, "source"},
          {GameRoles::Runner, "runner"},
          {GameRoles::Flatpak, "flatpak"},
          {GameRoles::Hidden, "hidden"}};
}

bool HeroicGameModel::heroicDetected() const { return m_heroicDetected; }
QString HeroicGameModel::statusText() const { return m_statusText; }
QString HeroicGameModel::errorText() const { return m_errorText; }

void HeroicGameModel::toggleFavorite(int row) {
  if (row < 0 || row >= m_games.size() || !m_database.isOpen()) {
    return;
  }
  Game& game = m_games[row];
  game.favorite = !game.favorite;
  QSqlQuery query(m_database);
  query.prepare(QStringLiteral("UPDATE heroic_games SET favorite = ? WHERE game_key = ?"));
  query.addBindValue(game.favorite);
  query.addBindValue(game.heroic.key);
  if (!query.exec()) {
    game.favorite = !game.favorite;
    setStatus(m_statusText, query.lastError().text());
    return;
  }
  emit dataChanged(index(row), index(row), {GameRoles::Favorite});
}

void HeroicGameModel::toggleHidden(int row) {
  if (row < 0 || row >= m_games.size() || !m_database.isOpen()) {
    return;
  }
  Game& game = m_games[row];
  game.hidden = !game.hidden;
  QSqlQuery query(m_database);
  query.prepare(QStringLiteral("UPDATE heroic_games SET hidden = ? WHERE game_key = ?"));
  query.addBindValue(game.hidden);
  query.addBindValue(game.heroic.key);
  if (!query.exec()) {
    game.hidden = !game.hidden;
    setStatus(m_statusText, query.lastError().text());
    return;
  }
  emit dataChanged(index(row), index(row), {GameRoles::Hidden});
}

void HeroicGameModel::refresh() { refreshFromRoots(HeroicScanner::discoverRoots()); }
void HeroicGameModel::refreshFromRoots(const QStringList& roots) {
  applyScan(HeroicScanner::scan(roots));
}

bool HeroicGameModel::openDatabase(const QString& path) {
  if (path != QStringLiteral(":memory:")) {
    QDir().mkpath(QFileInfo(path).absolutePath());
  }
  m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
  m_database.setDatabaseName(path);
  if (!m_database.open()) {
    setStatus(QStringLiteral("Heroic cache unavailable"), m_database.lastError().text());
    return false;
  }
  return true;
}

bool HeroicGameModel::ensureSchema() {
  QSqlQuery query(m_database);
  return query.exec(QStringLiteral(
      "CREATE TABLE IF NOT EXISTS heroic_games (game_key TEXT PRIMARY KEY, app_id TEXT NOT NULL, "
      "runner TEXT NOT NULL, name TEXT NOT NULL, directory TEXT, cover_path TEXT, hero_path TEXT, "
      "flatpak INTEGER NOT NULL DEFAULT 0, favorite INTEGER NOT NULL DEFAULT 0, hidden INTEGER NOT "
      "NULL DEFAULT 0, observed_at INTEGER NOT NULL)"));
}

void HeroicGameModel::loadDatabase() {
  QVector<Game> loaded;
  QSqlQuery query(m_database);
  if (!query.exec(QStringLiteral(
          "SELECT game_key, app_id, runner, name, directory, cover_path, hero_path, flatpak, "
          "favorite, hidden FROM heroic_games ORDER BY name COLLATE NOCASE"))) {
    setStatus(QStringLiteral("Could not load cached Heroic games"), query.lastError().text());
    return;
  }
  while (query.next()) {
    HeroicGameRecord record{.key = query.value(0).toString(),
                            .appId = query.value(1).toString(),
                            .runner = query.value(2).toString(),
                            .title = query.value(3).toString(),
                            .installPath = query.value(4).toString(),
                            .coverPath = query.value(5).toString(),
                            .heroPath = query.value(6).toString(),
                            .flatpak = query.value(7).toBool()};
    loaded.append({.heroic = record,
                   .favorite = query.value(8).toBool(),
                   .hidden = query.value(9).toBool(),
                   .accentStart = colorFor(record.key, 0),
                   .accentEnd = colorFor(record.key, 1)});
  }
  beginResetModel();
  m_games = loaded;
  endResetModel();
}

void HeroicGameModel::applyScan(const HeroicScanResult& result) {
  m_heroicDetected = !result.roots.isEmpty();
  if (result.incomplete || (result.roots.isEmpty() && !m_games.isEmpty())) {
    setStatus(QStringLiteral("Heroic scan interrupted; kept the cached library"),
              result.warnings.join(QLatin1Char('\n')));
    return;
  }
  if (!m_database.transaction()) {
    setStatus(QStringLiteral("Could not update Heroic games"), m_database.lastError().text());
    return;
  }
  QSqlQuery query(m_database);
  bool okay = query.exec(QStringLiteral("UPDATE heroic_games SET observed_at = 0"));
  for (const HeroicGameRecord& game : result.games) {
    query.prepare(QStringLiteral(
        "INSERT INTO heroic_games(game_key, app_id, runner, name, directory, cover_path, "
        "hero_path, "
        "flatpak, observed_at) VALUES(?, ?, ?, ?, ?, ?, ?, ?, strftime('%s', 'now')) ON "
        "CONFLICT(game_key) DO UPDATE SET app_id = excluded.app_id, runner = excluded.runner, name "
        "= excluded.name, directory = excluded.directory, cover_path = excluded.cover_path, "
        "hero_path = excluded.hero_path, flatpak = excluded.flatpak, observed_at = "
        "excluded.observed_at"));
    query.addBindValue(game.key);
    query.addBindValue(game.appId);
    query.addBindValue(game.runner);
    query.addBindValue(game.title);
    query.addBindValue(game.installPath);
    query.addBindValue(game.coverPath);
    query.addBindValue(game.heroPath);
    query.addBindValue(game.flatpak);
    okay = okay && query.exec();
  }
  okay = okay && query.exec(QStringLiteral("DELETE FROM heroic_games WHERE observed_at = 0"));
  if (!okay || !m_database.commit()) {
    m_database.rollback();
    setStatus(QStringLiteral("Could not update Heroic games"), query.lastError().text());
    return;
  }
  loadDatabase();
  setStatus(m_heroicDetected ? QStringLiteral("Imported %1 Heroic game(s)").arg(result.games.size())
                             : QStringLiteral("Heroic was not found"),
            result.warnings.join(QLatin1Char('\n')));
}

QVariant HeroicGameModel::valueForRole(const Game& game, int role) const {
  switch (role) {
  case GameRoles::Title:
    return game.heroic.title;
  case GameRoles::Subtitle:
    return QStringLiteral("Heroic · %1").arg(storeName(game.heroic.runner));
  case GameRoles::Description:
    return QStringLiteral("Installed locally through Heroic.");
  case GameRoles::Hours:
  case GameRoles::Progress:
  case GameRoles::AchievementsUnlocked:
  case GameRoles::AchievementsTotal:
  case GameRoles::Year:
    return 0;
  case GameRoles::Favorite:
    return game.favorite;
  case GameRoles::Recent:
    return false;
  case GameRoles::AccentStart:
    return game.accentStart;
  case GameRoles::AccentEnd:
    return game.accentEnd;
  case GameRoles::CoverMark:
    return game.heroic.title.left(1).toUpper();
  case GameRoles::AppId:
    return game.heroic.appId;
  case GameRoles::CoverPath:
    return localUrl(game.heroic.coverPath);
  case GameRoles::HeroPath:
    return localUrl(game.heroic.heroPath);
  case GameRoles::LogoPath:
    return QString{};
  case GameRoles::InstallPath:
    return game.heroic.installPath;
  case GameRoles::Source:
    return QStringLiteral("Heroic");
  case GameRoles::Runner:
    return game.heroic.runner;
  case GameRoles::Flatpak:
    return game.heroic.flatpak;
  case GameRoles::Hidden:
    return game.hidden;
  default:
    return {};
  }
}

void HeroicGameModel::setStatus(const QString& status, const QString& error) {
  m_statusText = status;
  m_errorText = error;
  emit statusChanged();
}
