#include "sources/heroic/HeroicScanner.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {
constexpr qint64 kMaximumJsonBytes = 32 * 1024 * 1024;

struct Metadata {
  QString title;
  QString coverUrl;
  QString heroUrl;
};

QJsonDocument readJson(const QString& path, HeroicScanResult* result, bool required = true) {
  QFile file(path);
  if (!file.exists()) {
    return {};
  }
  if (!file.open(QIODevice::ReadOnly) || file.size() > kMaximumJsonBytes) {
    if (required) {
      result->incomplete = true;
    }
    result->warnings.append(QStringLiteral("Could not read %1").arg(path));
    return {};
  }
  QJsonParseError error;
  const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    if (required) {
      result->incomplete = true;
    }
    result->warnings.append(
        QStringLiteral("Could not parse %1: %2").arg(path, error.errorString()));
    return {};
  }
  return document;
}

QHash<QString, Metadata> readMetadata(const QString& root, const QString& filename,
                                      const QString& key, HeroicScanResult* result) {
  QHash<QString, Metadata> metadata;
  const QJsonDocument document =
      readJson(root + QStringLiteral("/store_cache/") + filename, result, false);
  const QJsonArray games = document.object().value(key).toArray();
  for (const QJsonValue& value : games) {
    const QJsonObject game = value.toObject();
    const QString appId = game.value(QStringLiteral("app_name")).toVariant().toString();
    if (appId.isEmpty()) {
      continue;
    }
    metadata.insert(appId,
                    {.title = game.value(QStringLiteral("title")).toString(),
                     .coverUrl = game.value(QStringLiteral("art_square")).toString(),
                     .heroUrl = game.value(QStringLiteral("art_background"))
                                    .toString(game.value(QStringLiteral("art_cover")).toString())});
  }
  return metadata;
}

QString cachedArtwork(const QString& root, const QString& appId, const QString& url) {
  if (!appId.isEmpty()) {
    const QString iconBase = root + QStringLiteral("/icons/") + appId;
    for (const QString& extension : {QStringLiteral(".jpg"), QStringLiteral(".png")}) {
      if (QFileInfo(iconBase + extension).isFile()) {
        return iconBase + extension;
      }
    }
  }
  if (url.startsWith(QStringLiteral("http://")) || url.startsWith(QStringLiteral("https://"))) {
    const QString digest = QString::fromLatin1(
        QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Sha256).toHex());
    const QString cached = root + QStringLiteral("/images-cache/") + digest;
    if (QFileInfo(cached).isFile()) {
      return cached;
    }
  }
  return {};
}

void appendGame(HeroicScanResult* result, QSet<QString>* keys, const QString& root,
                const QString& runner, const QString& appId, const QString& fallbackTitle,
                const QString& installPath, const Metadata& metadata, bool flatpak) {
  const QString key = runner + QLatin1Char(':') + appId;
  const QString title = metadata.title.isEmpty() ? fallbackTitle : metadata.title;
  if (appId.isEmpty() || title.trimmed().isEmpty() || keys->contains(key)) {
    return;
  }
  result->games.append({.key = key,
                        .appId = appId,
                        .runner = runner,
                        .title = title.trimmed(),
                        .installPath = installPath,
                        .coverPath = cachedArtwork(root, appId, metadata.coverUrl),
                        .heroPath = cachedArtwork(root, QString{}, metadata.heroUrl),
                        .flatpak = flatpak});
  keys->insert(key);
}

void scanLegendary(const QString& root, bool flatpak, HeroicScanResult* result,
                   QSet<QString>* keys) {
  QString path = root + QStringLiteral("/legendaryConfig/legendary/installed.json");
  if (!QFileInfo(path).isFile()) {
    path = QFileInfo(root).dir().absoluteFilePath(QStringLiteral("legendary/installed.json"));
  }
  if (!QFileInfo(path).isFile()) {
    return;
  }
  const auto metadata = readMetadata(root, QStringLiteral("legendary_library.json"),
                                     QStringLiteral("library"), result);
  const QJsonObject games = readJson(path, result).object();
  for (auto iterator = games.begin(); iterator != games.end(); ++iterator) {
    const QJsonObject game = iterator.value().toObject();
    if (game.value(QStringLiteral("is_dlc")).toBool()) {
      continue;
    }
    const QString appId = game.value(QStringLiteral("app_name")).toString(iterator.key());
    appendGame(result, keys, root, QStringLiteral("legendary"), appId,
               game.value(QStringLiteral("title")).toString(appId),
               game.value(QStringLiteral("install_path")).toString(), metadata.value(appId),
               flatpak);
  }
}

void scanGog(const QString& root, bool flatpak, HeroicScanResult* result, QSet<QString>* keys) {
  const QString path = root + QStringLiteral("/gog_store/installed.json");
  if (!QFileInfo(path).isFile()) {
    return;
  }
  const auto metadata =
      readMetadata(root, QStringLiteral("gog_library.json"), QStringLiteral("games"), result);
  const QJsonArray games =
      readJson(path, result).object().value(QStringLiteral("installed")).toArray();
  for (const QJsonValue& value : games) {
    const QJsonObject game = value.toObject();
    if (game.value(QStringLiteral("is_dlc")).toBool()) {
      continue;
    }
    const QString appId = game.value(QStringLiteral("appName")).toVariant().toString();
    const QString installPath = game.value(QStringLiteral("install_path")).toString();
    QString title = appId;
    const QJsonDocument info = readJson(
        installPath + QStringLiteral("/goggame-") + appId + QStringLiteral(".info"), result, false);
    if (info.isObject()) {
      title = info.object().value(QStringLiteral("name")).toString(title);
    }
    appendGame(result, keys, root, QStringLiteral("gog"), appId, title, installPath,
               metadata.value(appId), flatpak);
  }
}

void scanNile(const QString& root, bool flatpak, HeroicScanResult* result, QSet<QString>* keys) {
  const QString base = root + QStringLiteral("/nile_config/nile");
  const QString path = base + QStringLiteral("/installed.json");
  if (!QFileInfo(path).isFile()) {
    return;
  }
  QHash<QString, Metadata> metadata;
  const QJsonArray library =
      readJson(base + QStringLiteral("/library.json"), result, false).array();
  for (const QJsonValue& value : library) {
    const QJsonObject product = value.toObject().value(QStringLiteral("product")).toObject();
    const QString appId = product.value(QStringLiteral("id")).toVariant().toString();
    const QJsonObject detail = product.value(QStringLiteral("productDetail")).toObject();
    const QJsonObject details = detail.value(QStringLiteral("details")).toObject();
    metadata.insert(
        appId,
        {.title = product.value(QStringLiteral("title")).toString(),
         .coverUrl = detail.value(QStringLiteral("iconUrl")).toString(),
         .heroUrl = details.value(QStringLiteral("backgroundUrl1"))
                        .toString(details.value(QStringLiteral("backgroundUrl2")).toString())});
  }
  const QJsonArray installed = readJson(path, result).array();
  for (const QJsonValue& value : installed) {
    const QJsonObject game = value.toObject();
    const QString appId = game.value(QStringLiteral("id")).toVariant().toString();
    appendGame(result, keys, root, QStringLiteral("nile"), appId, appId,
               game.value(QStringLiteral("path")).toString(), metadata.value(appId), flatpak);
  }
}
} // namespace

QStringList HeroicScanner::discoverRoots() {
  const QString config = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
  const QString home = QDir::homePath();
  QStringList candidates = {
      config + QStringLiteral("/heroic"),
      home + QStringLiteral("/.var/app/com.heroicgameslauncher.hgl/config/heroic")};
  candidates.removeDuplicates();
  QStringList roots;
  for (const QString& root : candidates) {
    if (QFileInfo(root).isDir()) {
      roots.append(root);
    }
  }
  return roots;
}

HeroicScanResult HeroicScanner::scan(const QStringList& roots) {
  HeroicScanResult result;
  QSet<QString> keys;
  for (const QString& root : roots) {
    if (!QFileInfo(root).isDir()) {
      continue;
    }
    const bool flatpak = root.contains(QStringLiteral("/.var/app/com.heroicgameslauncher.hgl/"));
    const int before = result.games.size();
    scanLegendary(root, flatpak, &result, &keys);
    scanGog(root, flatpak, &result, &keys);
    scanNile(root, flatpak, &result, &keys);
    const bool hasLegendary =
        QFileInfo(root + QStringLiteral("/legendaryConfig/legendary/installed.json")).isFile() ||
        QFileInfo(root).dir().exists(QStringLiteral("legendary/installed.json"));
    if (result.games.size() > before || hasLegendary ||
        QFileInfo(root + QStringLiteral("/gog_store/installed.json")).isFile() ||
        QFileInfo(root + QStringLiteral("/nile_config/nile/installed.json")).isFile()) {
      result.roots.append(root);
    }
  }
  return result;
}
