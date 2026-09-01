#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct LutrisGameRecord {
  QString id;
  QString slug;
  QString title;
  QString runner;
  QString installPath;
  QString platform;
  QString coverPath;
  int year = 0;
  qint64 lastPlayed = 0;
  int playtimeMinutes = 0;
  bool flatpak = false;
};

struct LutrisScanResult {
  QVector<LutrisGameRecord> games;
  QStringList databasePaths;
  QStringList warnings;
  bool incomplete = false;
};

class LutrisScanner final {
public:
  [[nodiscard]] static QStringList discoverDatabases();
  [[nodiscard]] static LutrisScanResult scan(const QStringList& databasePaths);
};
