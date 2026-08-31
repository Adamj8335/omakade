#include "app/AppSettings.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

AppSettings::AppSettings(const QString& path, QObject* parent)
    : QObject(parent), m_path(path.isEmpty() ? defaultPath() : path) {
  load();
}

bool AppSettings::reducedMotion() const { return m_reducedMotion; }

void AppSettings::setReducedMotion(bool value) {
  if (m_reducedMotion == value) {
    return;
  }
  m_reducedMotion = value;
  save();
  emit reducedMotionChanged();
}

int AppSettings::artworkCacheLimitMb() const { return m_artworkCacheLimitMb; }

void AppSettings::setArtworkCacheLimitMb(int value) {
  value = qBound(128, value, 8192);
  if (m_artworkCacheLimitMb == value) {
    return;
  }
  m_artworkCacheLimitMb = value;
  save();
  emit artworkCacheLimitMbChanged();
}

QString AppSettings::defaultPath() {
  return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
         QStringLiteral("/omakade/config.toml");
}

void AppSettings::load() {
  QFile file(m_path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }
  const QString contents = QString::fromUtf8(file.readAll());
  const QRegularExpression motion(QStringLiteral("(?m)^reduced_motion\\s*=\\s*(true|false)\\s*$"));
  const QRegularExpressionMatch motionMatch = motion.match(contents);
  if (motionMatch.hasMatch()) {
    m_reducedMotion = motionMatch.captured(1) == QStringLiteral("true");
  }
  const QRegularExpression limit(QStringLiteral("(?m)^artwork_cache_limit_mb\\s*=\\s*(\\d+)\\s*$"));
  const QRegularExpressionMatch limitMatch = limit.match(contents);
  if (limitMatch.hasMatch()) {
    m_artworkCacheLimitMb = qBound(128, limitMatch.captured(1).toInt(), 8192);
  }
}

void AppSettings::save() const {
  QDir().mkpath(QFileInfo(m_path).absolutePath());
  QSaveFile file(m_path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }
  file.write(QStringLiteral("reduced_motion = %1\nartwork_cache_limit_mb = %2\n")
                 .arg(m_reducedMotion ? QStringLiteral("true") : QStringLiteral("false"))
                 .arg(m_artworkCacheLimitMb)
                 .toUtf8());
  file.commit();
}
