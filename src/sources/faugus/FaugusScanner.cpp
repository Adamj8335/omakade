#include "sources/faugus/FaugusScanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

namespace {
constexpr qint64 kMaximumJsonBytes = 16 * 1024 * 1024;

bool validGameId(const QString& gameId) {
  static const QRegularExpression valid(
      QStringLiteral("^[\\p{L}\\p{N}_-][\\p{L}\\p{N}_.-]{0,254}$"),
      QRegularExpression::UseUnicodePropertiesOption);
  return valid.match(gameId).hasMatch();
}

QString artworkPath(const QString& root, const QString& directory, const QString& gameId) {
  const QString base = root + QLatin1Char('/') + directory + QLatin1Char('/') + gameId;
  for (const QString& extension : {QStringLiteral(".png"), QStringLiteral(".jpg"),
                                   QStringLiteral(".jpeg"), QStringLiteral(".webp")}) {
    if (QFileInfo(base + extension).isFile()) {
      return base + extension;
    }
  }
  return {};
}

QString expandedPath(QString path) {
  if (path.startsWith(QStringLiteral("~/"))) {
    path.replace(0, 1, QDir::homePath());
  } else if (path.startsWith(QStringLiteral("$HOME/"))) {
    path.replace(0, 5, QDir::homePath());
  }
  return QDir::cleanPath(path);
}
} // namespace

QStringList FaugusScanner::discoverRoots() {
  const QString data = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
  const QString home = QDir::homePath();
  QStringList candidates = {
      data + QStringLiteral("/faugus-launcher"),
      home + QStringLiteral("/.var/app/io.github.Faugus.faugus-launcher/data/faugus-launcher")};
  candidates.removeDuplicates();
  QStringList roots;
  for (const QString& root : candidates) {
    if (QFileInfo(root + QStringLiteral("/games.json")).isFile()) {
      roots.append(root);
    }
  }
  return roots;
}

FaugusScanResult FaugusScanner::scan(const QStringList& roots) {
  FaugusScanResult result;
  QSet<QString> gameIds;
  for (const QString& root : roots) {
    const QString gamesPath = root + QStringLiteral("/games.json");
    QFile file(gamesPath);
    if (!file.exists()) {
      continue;
    }
    result.roots.append(root);
    if (!file.open(QIODevice::ReadOnly) || file.size() > kMaximumJsonBytes) {
      result.incomplete = true;
      result.warnings.append(QStringLiteral("Could not read %1").arg(gamesPath));
      continue;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
      result.incomplete = true;
      result.warnings.append(
          QStringLiteral("Could not parse %1: %2").arg(gamesPath, error.errorString()));
      continue;
    }
    const bool flatpak =
        root.contains(QStringLiteral("/.var/app/io.github.Faugus.faugus-launcher/"));
    for (const QJsonValue& value : document.array()) {
      const QJsonObject game = value.toObject();
      const QString gameId = game.value(QStringLiteral("gameid")).toString().trimmed();
      const QString title = game.value(QStringLiteral("title")).toString().trimmed();
      const QString path = game.value(QStringLiteral("path")).toString().trimmed();
      if (!validGameId(gameId) || title.isEmpty() || path.isEmpty() || gameIds.contains(gameId)) {
        continue;
      }
      QString cover = artworkPath(root, QStringLiteral("covers"), gameId);
      if (cover.isEmpty()) {
        cover = artworkPath(root, QStringLiteral("icons"), gameId);
      }
      result.games.append(
          {.gameId = gameId,
           .title = title,
           .executablePath = expandedPath(path),
           .runner = game.value(QStringLiteral("runner")).toString(),
           .coverPath = cover,
           .heroPath = artworkPath(root, QStringLiteral("banners"), gameId),
           .playtimeSeconds = qMax<qint64>(0, game.value(QStringLiteral("playtime")).toInteger()),
           .flatpak = flatpak});
      gameIds.insert(gameId);
    }
  }
  return result;
}
