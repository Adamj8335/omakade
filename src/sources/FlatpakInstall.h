#pragma once

#include <QDir>
#include <QFileInfo>
#include <QString>

// Reports whether a Flatpak application is installed for the system or the current user
// without spawning the flatpak binary, so launch paths never block the interface.
inline bool flatpakAppInstalled(const QString& appId) {
  const QStringList roots = {QStringLiteral("/var/lib/flatpak/app/"),
                             QDir::homePath() + QStringLiteral("/.local/share/flatpak/app/")};
  for (const QString& root : roots) {
    if (QFileInfo(root + appId).isDir()) {
      return true;
    }
  }
  return false;
}
