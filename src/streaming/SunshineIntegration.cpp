#include "streaming/SunshineIntegration.h"

#include "app/AppSettings.h"
#include "launch/PlayRequest.h"
#include "library/GameRoles.h"
#include "library/UnifiedGameModel.h"
#include "sources/FlatpakInstall.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QHash>
#include <QImageReader>
#include <QJsonDocument>
#include <QPainter>
#include <QProcess>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QUrl>

namespace {
constexpr auto kMarker = "omakade";
constexpr auto kFlatpakId = "dev.lizardbyte.app.Sunshine";
constexpr int kBoxArtWidth = 600;
constexpr int kBoxArtHeight = 800;

QString localPath(const QString& value) {
  const QUrl url(value);
  return url.isLocalFile() ? url.toLocalFile() : value;
}
} // namespace

SunshineIntegration::SunshineIntegration(UnifiedGameModel* games, AppSettings* settings,
                                         const QString& appsPath, const QString& imageRoot,
                                         QObject* parent)
    : QObject(parent), m_games(games), m_settings(settings), m_appsPath(appsPath),
      m_imageRoot(imageRoot) {
  if (m_imageRoot.isEmpty()) {
    m_imageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                  QStringLiteral("/omakade/sunshine");
  }
  if (m_appsPath.isEmpty()) {
    detect();
  }
  m_syncTimer.setSingleShot(true);
  m_syncTimer.setInterval(1500);
  connect(&m_syncTimer, &QTimer::timeout, this, [this] { sync(); });
  if (m_games != nullptr) {
    connect(m_games, &QAbstractItemModel::modelReset, this, &SunshineIntegration::scheduleSync);
  }
  if (m_settings != nullptr) {
    connect(m_settings, &AppSettings::sunshineChanged, this, [this] { sync(); });
  }
  if (!detected()) {
    setStatus(QStringLiteral("Sunshine was not found. Install it from Omarchy's menu to stream "
                             "with Moonlight."));
  } else if (m_settings != nullptr &&
             (m_settings->sunshineOmakadeApp() || m_settings->sunshineGameApps())) {
    scheduleSync();
  } else {
    setStatus(QStringLiteral("Sunshine detected. Nothing is exported yet."));
  }
  if (detected()) {
    refreshRestartState();
  }
}

void SunshineIntegration::rememberWrittenList(const QByteArray& contents) {
  QDir().mkpath(m_imageRoot);
  QSaveFile marker(m_imageRoot + QStringLiteral("/apps.sha1"));
  if (marker.open(QIODevice::WriteOnly)) {
    marker.write(QCryptographicHash::hash(contents, QCryptographicHash::Sha1).toHex());
    marker.commit();
  }
}

void SunshineIntegration::refreshRestartState() {
  QFile marker(m_imageRoot + QStringLiteral("/apps.sha1"));
  QFile apps(m_appsPath);
  if (!marker.open(QIODevice::ReadOnly) || !apps.open(QIODevice::ReadOnly)) {
    return;
  }
  const QByteArray written = marker.readAll().trimmed();
  const QByteArray current =
      QCryptographicHash::hash(apps.readAll(), QCryptographicHash::Sha1).toHex();
  if (written != current) {
    // Someone else wrote the list since; Sunshine's own web UI reloads on save.
    return;
  }
  const qint64 modified = QFileInfo(m_appsPath).lastModified().toSecsSinceEpoch();
  auto* process = new QProcess(this);
  connect(process, &QProcess::finished, this, [this, process, modified](int, QProcess::ExitStatus) {
    const QString output = QString::fromUtf8(process->readAllStandardOutput());
    process->deleteLater();
    qint64 activeSince = -1;
    bool active = false;
    for (const QString& line : output.split(QLatin1Char('\n'))) {
      if (line.startsWith(QStringLiteral("ActiveEnterTimestamp=@"))) {
        activeSince = line.mid(22).trimmed().toLongLong();
      } else if (line.startsWith(QStringLiteral("ActiveState="))) {
        active = line.mid(12).trimmed() == QStringLiteral("active");
      }
    }
    if (activeSince < 0) {
      return;
    }
    const bool needed = active && modified > activeSince;
    if (needed != m_restartNeeded) {
      m_restartNeeded = needed;
      emit stateChanged();
    }
  });
  connect(process, &QProcess::errorOccurred, process, &QObject::deleteLater);
  process->start(QStringLiteral("systemctl"),
                 {QStringLiteral("--user"), QStringLiteral("show"), QStringLiteral("--timestamp=unix"),
                  QStringLiteral("-p"), QStringLiteral("ActiveEnterTimestamp,ActiveState"),
                  serviceUnit()});
}

bool SunshineIntegration::streaming() { return qEnvironmentVariableIsSet("SUNSHINE_APP_ID"); }

void SunshineIntegration::detect() {
  const QString nativeDir = QDir::homePath() + QStringLiteral("/.config/sunshine");
  const QString flatpakDir =
      QDir::homePath() + QStringLiteral("/.var/app/") + QLatin1String(kFlatpakId) +
      QStringLiteral("/config/sunshine");
  if (!QStandardPaths::findExecutable(QStringLiteral("sunshine")).isEmpty() ||
      QFileInfo::exists(nativeDir + QStringLiteral("/apps.json"))) {
    m_appsPath = nativeDir + QStringLiteral("/apps.json");
    m_flatpak = false;
  } else if (flatpakAppInstalled(QLatin1String(kFlatpakId))) {
    m_appsPath = flatpakDir + QStringLiteral("/apps.json");
    m_flatpak = true;
  }
}

void SunshineIntegration::scheduleSync() {
  if (detected()) {
    m_syncTimer.start();
  }
}

QString SunshineIntegration::shellQuote(const QString& value) {
  QString quoted = value;
  quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
  return QLatin1Char('\'') + quoted + QLatin1Char('\'');
}

QString SunshineIntegration::commandPrefix(bool flatpakSunshine) {
  // Flatpak Sunshine runs commands inside its sandbox, so reach the host binary explicitly.
  return flatpakSunshine ? QStringLiteral("flatpak-spawn --host omakade")
                         : QStringLiteral("omakade");
}

bool SunshineIntegration::isOmakadeEntry(const QJsonObject& entry) {
  return entry.contains(QLatin1String(kMarker));
}

QJsonObject SunshineIntegration::omakadeEntry(const QString& prefix, const QString& imagePath) {
  // An empty cmd keeps the desktop stream alive like the stock Steam Big Picture entry, and
  // the undo step closes the window when the Moonlight session ends.
  QJsonObject entry;
  entry.insert(QStringLiteral("name"), QStringLiteral("Omakade"));
  entry.insert(QStringLiteral("cmd"), QStringLiteral(""));
  entry.insert(QStringLiteral("detached"), QJsonArray{prefix});
  QJsonObject undo;
  undo.insert(QStringLiteral("do"), QStringLiteral(""));
  undo.insert(QStringLiteral("undo"), prefix + QStringLiteral(" --quit"));
  entry.insert(QStringLiteral("prep-cmd"), QJsonArray{undo});
  if (!imagePath.isEmpty()) {
    entry.insert(QStringLiteral("image-path"), imagePath);
  }
  entry.insert(QLatin1String(kMarker), QStringLiteral("app"));
  return entry;
}

QJsonObject SunshineIntegration::gameEntry(const QString& title, const QString& launchKey,
                                           const QString& prefix, const QString& imagePath) {
  QJsonObject entry;
  entry.insert(QStringLiteral("name"), title);
  entry.insert(QStringLiteral("cmd"), QStringLiteral(""));
  entry.insert(QStringLiteral("detached"),
               QJsonArray{prefix + QStringLiteral(" --play ") + shellQuote(launchKey)});
  if (!imagePath.isEmpty()) {
    entry.insert(QStringLiteral("image-path"), imagePath);
  }
  entry.insert(QLatin1String(kMarker), launchKey);
  return entry;
}

QJsonObject SunshineIntegration::mergeEntries(const QJsonObject& existing,
                                              const QJsonArray& ours) {
  QJsonObject result = existing;
  QJsonArray apps;
  for (const QJsonValue& value : existing.value(QStringLiteral("apps")).toArray()) {
    if (!value.isObject() || !isOmakadeEntry(value.toObject())) {
      apps.append(value);
    }
  }
  for (const QJsonValue& value : ours) {
    apps.append(value);
  }
  result.insert(QStringLiteral("apps"), apps);
  if (!result.contains(QStringLiteral("env"))) {
    result.insert(QStringLiteral("env"), QJsonObject{});
  }
  return result;
}

QString SunshineIntegration::exportImage(const QString& sourcePath, const QString& name) {
  const QString source = localPath(sourcePath);
  if (source.isEmpty() || !QFileInfo::exists(source)) {
    return {};
  }
  const QString hash = QString::fromLatin1(
      QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha1).toHex().left(16));
  const QString target = m_imageRoot + QLatin1Char('/') + hash + QStringLiteral(".png");
  const QFileInfo targetInfo(target);
  if (targetInfo.exists() &&
      targetInfo.lastModified() >= QFileInfo(source).lastModified()) {
    return target;
  }
  QImageReader reader(source);
  reader.setAutoTransform(true);
  if (source.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)) {
    reader.setScaledSize(QSize(kBoxArtWidth * 2 / 3, kBoxArtWidth * 2 / 3));
  }
  const QImage image = reader.read();
  if (image.isNull()) {
    return {};
  }
  QDir().mkpath(m_imageRoot);
  QImage boxArt(kBoxArtWidth, kBoxArtHeight, QImage::Format_ARGB32_Premultiplied);
  boxArt.fill(QColor(24, 26, 30));
  QPainter painter(&boxArt);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  if (source.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)) {
    painter.drawImage((kBoxArtWidth - image.width()) / 2, (kBoxArtHeight - image.height()) / 2,
                      image);
  } else {
    const QImage scaled = image.scaled(kBoxArtWidth, kBoxArtHeight, Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);
    painter.drawImage((kBoxArtWidth - scaled.width()) / 2, (kBoxArtHeight - scaled.height()) / 2,
                      scaled);
  }
  painter.end();
  return boxArt.save(target, "PNG") ? target : QString{};
}

bool SunshineIntegration::sync() {
  if (!detected() || m_games == nullptr || m_settings == nullptr) {
    return false;
  }
  QFile file(m_appsPath);
  if (!file.open(QIODevice::ReadOnly)) {
    // Sunshine writes its default list on first start; never invent one for it.
    setStatus(QStringLiteral("Start Sunshine once so it creates %1").arg(m_appsPath));
    return false;
  }
  const QByteArray previous = file.readAll();
  file.close();
  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(previous, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    setStatus(QStringLiteral("Could not read %1: %2").arg(m_appsPath, parseError.errorString()));
    return false;
  }

  const QString prefix = commandPrefix(m_flatpak);
  QJsonArray ours;
  if (m_settings->sunshineOmakadeApp()) {
    ours.append(omakadeEntry(prefix, exportImage(m_iconSource, QStringLiteral("omakade"))));
  }
  int games = 0;
  if (m_settings->sunshineGameApps()) {
    QVector<int> rows;
    QHash<QString, int> titleCounts;
    for (int row = 0; row < m_games->rowCount(); ++row) {
      const QModelIndex game = m_games->index(row, 0);
      if (game.data(GameRoles::Hidden).toBool() || !game.data(GameRoles::Installed).toBool()) {
        continue;
      }
      rows.append(row);
      ++titleCounts[game.data(GameRoles::Title).toString().toLower()];
    }
    for (int row : rows) {
      const QModelIndex game = m_games->index(row, 0);
      const QString source = game.data(GameRoles::Source).toString();
      const QString launchKey = LaunchKey{.source = source,
                                          .runner = game.data(GameRoles::Runner).toString(),
                                          .appId = game.data(GameRoles::AppId).toString()}
                                    .toString();
      QString title = game.data(GameRoles::Title).toString();
      if (titleCounts.value(title.toLower()) > 1) {
        // Sunshine identifies apps by name, so two stores' copies need distinct names.
        title += QStringLiteral(" (%1)").arg(source);
      }
      ours.append(gameEntry(title, launchKey, prefix,
                            exportImage(game.data(GameRoles::CoverPath).toString(), launchKey)));
      ++games;
    }
  }

  // Drop box art for games that are no longer exported so the folder does not grow forever.
  QSet<QString> usedImages;
  for (const QJsonValue& value : ours) {
    usedImages.insert(value.toObject().value(QStringLiteral("image-path")).toString());
  }
  for (const QFileInfo& image :
       QDir(m_imageRoot).entryInfoList({QStringLiteral("*.png")}, QDir::Files)) {
    if (!usedImages.contains(image.absoluteFilePath())) {
      QFile::remove(image.absoluteFilePath());
    }
  }

  const QJsonObject merged = mergeEntries(document.object(), ours);
  const QByteArray contents = QJsonDocument(merged).toJson(QJsonDocument::Indented);
  if (contents == previous) {
    m_exportedGames = games;
    setStatus(ours.isEmpty() ? QStringLiteral("Nothing is exported to Sunshine")
                             : QStringLiteral("Sunshine app list is up to date"));
    return true;
  }
  const QString backup = m_appsPath + QStringLiteral(".omakade-backup");
  if (!QFileInfo::exists(backup)) {
    QFile::copy(m_appsPath, backup);
  }
  QSaveFile output(m_appsPath);
  if (!output.open(QIODevice::WriteOnly) || output.write(contents) != contents.size() ||
      !output.commit()) {
    setStatus(QStringLiteral("Could not write %1").arg(m_appsPath));
    return false;
  }
  rememberWrittenList(contents);
  m_exportedGames = games;
  m_restartNeeded = true;
  setStatus(ours.isEmpty()
                ? QStringLiteral("Removed Omakade from Sunshine. Restart Sunshine to apply.")
                : QStringLiteral("Wrote %1 Sunshine app(s). Restart Sunshine to apply.")
                      .arg(ours.size()));
  return true;
}

void SunshineIntegration::restartSunshine() {
  if (m_busy || streaming()) {
    return;
  }
  m_busy = true;
  emit stateChanged();
  auto* process = new QProcess(this);
  connect(process, &QProcess::finished, this,
          [this, process](int exitCode, QProcess::ExitStatus status) {
            m_busy = false;
            if (status == QProcess::NormalExit && exitCode == 0) {
              m_restartNeeded = false;
              setStatus(QStringLiteral("Sunshine restarted. Moonlight will show the new list."));
            } else {
              setStatus(QStringLiteral("Could not restart Sunshine: %1")
                            .arg(QString::fromUtf8(process->readAllStandardError()).trimmed()));
            }
            process->deleteLater();
          });
  connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError) {
    m_busy = false;
    setStatus(QStringLiteral("Could not run systemctl to restart Sunshine"));
    process->deleteLater();
  });
  process->start(QStringLiteral("systemctl"),
                 {QStringLiteral("--user"), QStringLiteral("restart"), serviceUnit()});
}

QString SunshineIntegration::serviceUnit() {
  // The Arch package installs app-dev.lizardbyte.app.Sunshine.service with a
  // sunshine.service alias that only exists once the unit is enabled, so prefer the real
  // unit name whenever its file is present.
  const QString packaged = QStringLiteral("app-dev.lizardbyte.app.Sunshine.service");
  for (const QString& directory :
       {QStringLiteral("/usr/lib/systemd/user/"), QStringLiteral("/etc/systemd/user/"),
        QDir::homePath() + QStringLiteral("/.config/systemd/user/"),
        QDir::homePath() + QStringLiteral("/.local/share/systemd/user/")}) {
    if (QFileInfo::exists(directory + packaged)) {
      return packaged;
    }
  }
  return QStringLiteral("sunshine.service");
}

void SunshineIntegration::setStatus(const QString& text) {
  m_statusText = text;
  emit stateChanged();
}
