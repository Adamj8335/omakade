#include "sources/steam/SteamScanner.h"

#include "sources/steam/ValveKeyValues.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

namespace {
struct Activity {
  qint64 lastPlayed = 0;
  int playtimeMinutes = 0;
};

QString cleanPath(const QString& path) {
  return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

bool isTool(const QString& name) {
  const QString normalized = name.trimmed().toLower();
  return normalized.startsWith(QStringLiteral("proton ")) ||
         normalized.startsWith(QStringLiteral("steam linux runtime")) ||
         normalized.startsWith(QStringLiteral("steamworks common redistributables")) ||
         normalized.startsWith(QStringLiteral("steam runtime"));
}

QString firstMatchingFile(const QString& directory, const QStringList& filters) {
  const QDir dir(directory);
  const QStringList matches = dir.entryList(filters, QDir::Files, QDir::Name);
  return matches.isEmpty() ? QString{} : dir.absoluteFilePath(matches.first());
}

const ValveKeyValues* descend(const ValveKeyValues& root, const QStringList& path) {
  const ValveKeyValues* current = &root;
  for (const QString& part : path) {
    current = current->object(part);
    if (current == nullptr) {
      return nullptr;
    }
  }
  return current;
}

QHash<QString, Activity> readActivity(const QStringList& roots) {
  QHash<QString, Activity> result;
  for (const QString& root : roots) {
    QDir userdata(root + QStringLiteral("/userdata"));
    for (const QString& user : userdata.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
      ValveKeyValues values;
      if (!ValveKeyValuesParser::parseFile(
              userdata.absoluteFilePath(user + QStringLiteral("/config/localconfig.vdf")),
              &values)) {
        continue;
      }
      const ValveKeyValues* apps = descend(
          values, {QStringLiteral("UserLocalConfigStore"), QStringLiteral("Software"),
                   QStringLiteral("Valve"), QStringLiteral("Steam"), QStringLiteral("apps")});
      if (apps == nullptr) {
        continue;
      }
      for (auto iterator = apps->objects.cbegin(); iterator != apps->objects.cend(); ++iterator) {
        bool numeric = false;
        iterator.key().toULongLong(&numeric);
        if (!numeric) {
          continue;
        }
        Activity& activity = result[iterator.key()];
        activity.lastPlayed = qMax(
            activity.lastPlayed, iterator.value().value(QStringLiteral("LastPlayed")).toLongLong());
        activity.playtimeMinutes = qMax(activity.playtimeMinutes,
                                        iterator.value().value(QStringLiteral("Playtime")).toInt());
      }
    }
  }
  return result;
}

QStringList libraryPaths(const QString& steamRoot, QStringList* warnings, bool* incomplete) {
  QStringList paths{steamRoot};
  QString libraryFile = steamRoot + QStringLiteral("/config/libraryfolders.vdf");
  if (!QFileInfo::exists(libraryFile)) {
    libraryFile = steamRoot + QStringLiteral("/steamapps/libraryfolders.vdf");
  }

  ValveKeyValues root;
  QString error;
  if (!ValveKeyValuesParser::parseFile(libraryFile, &root, &error)) {
    warnings->append(QStringLiteral("Could not read %1: %2").arg(libraryFile, error));
    *incomplete = true;
    return paths;
  }
  const ValveKeyValues* folders = root.object(QStringLiteral("libraryfolders"));
  if (folders == nullptr) {
    folders = &root;
  }
  for (auto iterator = folders->values.cbegin(); iterator != folders->values.cend(); ++iterator) {
    bool numeric = false;
    iterator.key().toInt(&numeric);
    if (numeric && !iterator.value().isEmpty()) {
      paths.append(cleanPath(iterator.value()));
    }
  }
  for (auto iterator = folders->objects.cbegin(); iterator != folders->objects.cend(); ++iterator) {
    bool numeric = false;
    iterator.key().toInt(&numeric);
    const QString path = iterator.value().value(QStringLiteral("path"));
    if (numeric && !path.isEmpty()) {
      paths.append(cleanPath(path));
    }
  }
  paths.removeDuplicates();
  return paths;
}

void resolveArtwork(SteamGameRecord* game, const QStringList& steamRoots) {
  for (const QString& root : steamRoots) {
    QDir userdata(root + QStringLiteral("/userdata"));
    for (const QString& user : userdata.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
      const QString grid = userdata.absoluteFilePath(user + QStringLiteral("/config/grid"));
      if (game->coverPath.isEmpty()) {
        game->coverPath = firstMatchingFile(grid, {game->appId + QStringLiteral("p.*")});
      }
      if (game->heroPath.isEmpty()) {
        game->heroPath = firstMatchingFile(grid, {game->appId + QStringLiteral("_hero.*")});
      }
      if (game->logoPath.isEmpty()) {
        game->logoPath = firstMatchingFile(grid, {game->appId + QStringLiteral("_logo.*")});
      }
    }
  }
  for (const QString& root : steamRoots) {
    const QString cache = root + QStringLiteral("/appcache/librarycache/") + game->appId;
    if (game->coverPath.isEmpty()) {
      game->coverPath = firstMatchingFile(
          cache, {QStringLiteral("library_600x900.*"), QStringLiteral("header.*")});
    }
    if (game->heroPath.isEmpty()) {
      game->heroPath =
          firstMatchingFile(cache, {QStringLiteral("library_hero.*"), QStringLiteral("header.*")});
    }
    if (game->logoPath.isEmpty()) {
      game->logoPath = firstMatchingFile(cache, {QStringLiteral("logo.*")});
    }
  }
}
} // namespace

QStringList SteamScanner::discoverSteamRoots() {
  const QString home = QDir::homePath();
  QStringList candidates = {
      home + QStringLiteral("/.local/share/Steam"),
      home + QStringLiteral("/.steam/steam"),
      home + QStringLiteral("/.var/app/com.valvesoftware.Steam/data/Steam"),
  };
  QStringList roots;
  for (const QString& candidate : candidates) {
    if (QFileInfo::exists(candidate + QStringLiteral("/steamapps"))) {
      const QString canonical = QFileInfo(candidate).canonicalFilePath();
      roots.append(canonical.isEmpty() ? cleanPath(candidate) : canonical);
    }
  }
  roots.removeDuplicates();
  return roots;
}

SteamScanResult SteamScanner::scan(const QStringList& steamRoots) {
  SteamScanResult result;
  result.steamRoots = steamRoots;
  const QHash<QString, Activity> activity = readActivity(steamRoots);
  QSet<QString> importedIds;

  for (const QString& steamRoot : steamRoots) {
    const QStringList discoveredLibraries =
        libraryPaths(steamRoot, &result.warnings, &result.incomplete);
    result.libraryPaths.append(discoveredLibraries);
    for (const QString& library : discoveredLibraries) {
      QDir steamapps(library + QStringLiteral("/steamapps"));
      if (!steamapps.exists()) {
        result.warnings.append(QStringLiteral("Steam library is unavailable: %1").arg(library));
        result.incomplete = true;
        continue;
      }
      const QStringList manifests =
          steamapps.entryList({QStringLiteral("appmanifest_*.acf")}, QDir::Files, QDir::Name);
      for (const QString& filename : manifests) {
        ValveKeyValues parsed;
        QString error;
        const QString manifest = steamapps.absoluteFilePath(filename);
        if (!ValveKeyValuesParser::parseFile(manifest, &parsed, &error)) {
          result.warnings.append(QStringLiteral("Could not read %1: %2").arg(manifest, error));
          result.incomplete = true;
          continue;
        }
        const ValveKeyValues* app = parsed.object(QStringLiteral("AppState"));
        if (app == nullptr) {
          result.warnings.append(QStringLiteral("Missing AppState in %1").arg(manifest));
          result.incomplete = true;
          continue;
        }
        const QString appId = app->value(QStringLiteral("appid"));
        const QString name = app->value(QStringLiteral("name")).trimmed();
        const int stateFlags = app->value(QStringLiteral("StateFlags")).toInt();
        if (appId.isEmpty() || name.isEmpty() || (stateFlags & 4) == 0 || isTool(name) ||
            importedIds.contains(appId)) {
          continue;
        }

        const Activity gameActivity = activity.value(appId);
        SteamGameRecord game{
            .appId = appId,
            .title = name,
            .installDirectory = app->value(QStringLiteral("installdir")),
            .libraryPath = cleanPath(library),
            .manifestPath = manifest,
            .coverPath = {},
            .heroPath = {},
            .logoPath = {},
            .lastPlayed = gameActivity.lastPlayed,
            .playtimeMinutes = gameActivity.playtimeMinutes,
        };
        resolveArtwork(&game, steamRoots);
        result.games.append(game);
        importedIds.insert(appId);
      }
    }
  }

  result.libraryPaths.removeDuplicates();

  std::sort(result.games.begin(), result.games.end(), [](const auto& left, const auto& right) {
    return left.title.localeAwareCompare(right.title) < 0;
  });
  return result;
}
