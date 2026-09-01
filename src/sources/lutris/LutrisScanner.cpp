#include "sources/lutris/LutrisScanner.h"

#include "sources/FlatpakInstall.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>

#include <cmath>

namespace {
QString firstArtwork(const QString& dataRoot, const QString& slug) {
  if (slug.isEmpty()) {
    return {};
  }
  const QString base = dataRoot + QStringLiteral("/coverart/") + slug;
  for (const QString& extension : {QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
                                   QStringLiteral(".png"), QStringLiteral(".webp")}) {
    const QString candidate = base + extension;
    if (QFileInfo::exists(candidate)) {
      return candidate;
    }
  }
  return {};
}

QString columnOrDefault(const QSet<QString>& columns, const QString& name,
                        const QString& fallback) {
  return columns.contains(name) ? QStringLiteral("COALESCE(%1, %2)").arg(name, fallback) : fallback;
}
} // namespace

QStringList LutrisScanner::discoverDatabases() {
  const QString dataHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
  const QString home = QDir::homePath();
  QStringList candidates = {
      dataHome + QStringLiteral("/lutris/pga.db"),
      home + QStringLiteral("/.var/app/net.lutris.Lutris/data/lutris/pga.db"),
  };
  candidates.removeDuplicates();

  // A leftover database from a removed native or Flatpak Lutris would collide with the live
  // one on bare row ids and could not launch anything, so only keep it when its launcher exists.
  const bool nativeInstalled = !QStandardPaths::findExecutable(QStringLiteral("lutris")).isEmpty();
  const bool flatpakInstalled = flatpakAppInstalled(QStringLiteral("net.lutris.Lutris"));
  QStringList found;
  for (const QString& path : candidates) {
    if (!QFileInfo(path).isFile()) {
      continue;
    }
    const bool flatpak = path.contains(QStringLiteral("/.var/app/net.lutris.Lutris/"));
    if ((flatpak && !flatpakInstalled && nativeInstalled) ||
        (!flatpak && !nativeInstalled && flatpakInstalled)) {
      continue;
    }
    found.append(path);
  }
  return found;
}

LutrisScanResult LutrisScanner::scan(const QStringList& databasePaths) {
  LutrisScanResult result;
  QSet<QString> importedIds;
  static const QRegularExpression validId(QStringLiteral("^[1-9][0-9]*$"));

  for (const QString& path : databasePaths) {
    if (!QFileInfo(path).isFile()) {
      continue;
    }
    result.databasePaths.append(path);
    const QString connection = QStringLiteral("omakade-lutris-scan-%1")
                                   .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    {
      QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
      database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
      database.setDatabaseName(path);
      if (!database.open()) {
        result.incomplete = true;
        result.warnings.append(
            QStringLiteral("Could not read %1: %2").arg(path, database.lastError().text()));
      } else {
        QSet<QString> columns;
        QSqlQuery schema(database);
        if (schema.exec(QStringLiteral("PRAGMA table_info(games)"))) {
          while (schema.next()) {
            columns.insert(schema.value(1).toString());
          }
        }
        const QSet<QString> required = {QStringLiteral("id"), QStringLiteral("slug"),
                                        QStringLiteral("name"), QStringLiteral("installed"),
                                        QStringLiteral("configpath")};
        if (!columns.contains(required)) {
          result.incomplete = true;
          result.warnings.append(
              QStringLiteral("%1 does not contain a supported Lutris library").arg(path));
        } else {
          const QString statement =
              QStringLiteral("SELECT CAST(id AS TEXT), COALESCE(slug, ''), COALESCE(name, ''), ") +
              columnOrDefault(columns, QStringLiteral("runner"), QStringLiteral("''")) +
              QStringLiteral(", ") +
              columnOrDefault(columns, QStringLiteral("directory"), QStringLiteral("''")) +
              QStringLiteral(", ") +
              columnOrDefault(columns, QStringLiteral("platform"), QStringLiteral("''")) +
              QStringLiteral(", ") +
              columnOrDefault(columns, QStringLiteral("year"), QStringLiteral("0")) +
              QStringLiteral(", ") +
              columnOrDefault(columns, QStringLiteral("lastplayed"), QStringLiteral("0")) +
              QStringLiteral(", ") +
              columnOrDefault(columns, QStringLiteral("playtime"), QStringLiteral("0")) +
              QStringLiteral(
                  " FROM games WHERE installed = 1 AND configpath IS NOT NULL "
                  "AND TRIM(CAST(configpath AS TEXT)) != '' ORDER BY name COLLATE NOCASE");
          QSqlQuery query(database);
          if (!query.exec(statement)) {
            result.incomplete = true;
            result.warnings.append(
                QStringLiteral("Could not import %1: %2").arg(path, query.lastError().text()));
          } else {
            const QString dataRoot = QFileInfo(path).absolutePath();
            const bool flatpak = path.contains(QStringLiteral("/.var/app/net.lutris.Lutris/"));
            while (query.next()) {
              const QString id = query.value(0).toString();
              const QString title = query.value(2).toString().trimmed();
              if (!validId.match(id).hasMatch() || title.isEmpty() || importedIds.contains(id)) {
                continue;
              }
              const QString slug = query.value(1).toString();
              result.games.append({
                  .id = id,
                  .slug = slug,
                  .title = title,
                  .runner = query.value(3).toString(),
                  .installPath = query.value(4).toString(),
                  .platform = query.value(5).toString(),
                  .coverPath = firstArtwork(dataRoot, slug),
                  .year = query.value(6).toInt(),
                  .lastPlayed = query.value(7).toLongLong(),
                  .playtimeMinutes = static_cast<int>(std::round(query.value(8).toDouble() * 60.0)),
                  .flatpak = flatpak,
              });
              importedIds.insert(id);
            }
          }
        }
        database.close();
      }
    }
    QSqlDatabase::removeDatabase(connection);
  }
  return result;
}
