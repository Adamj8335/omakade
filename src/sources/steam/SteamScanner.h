#pragma once

#include <QStringList>
#include <QVector>

struct SteamGameRecord {
  QString appId;
  QString title;
  QString installDirectory;
  QString libraryPath;
  QString manifestPath;
  QString coverPath;
  QString heroPath;
  QString logoPath;
  qint64 lastPlayed = 0;
  int playtimeMinutes = 0;

  bool operator==(const SteamGameRecord&) const = default;
};

struct SteamScanResult {
  QVector<SteamGameRecord> games;
  QStringList steamRoots;
  QStringList libraryPaths;
  QStringList warnings;
  bool incomplete = false;
};

class SteamScanner final {
public:
  [[nodiscard]] static QStringList discoverSteamRoots();
  [[nodiscard]] static SteamScanResult scan(const QStringList& steamRoots);
};
