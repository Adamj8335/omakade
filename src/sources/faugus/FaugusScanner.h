#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct FaugusGameRecord {
  QString gameId;
  QString title;
  QString executablePath;
  QString runner;
  QString coverPath;
  QString heroPath;
  qint64 playtimeSeconds = 0;
  bool flatpak = false;
};

struct FaugusScanResult {
  QVector<FaugusGameRecord> games;
  QStringList roots;
  QStringList warnings;
  bool incomplete = false;
};

class FaugusScanner final {
public:
  [[nodiscard]] static QStringList discoverRoots();
  [[nodiscard]] static FaugusScanResult scan(const QStringList& roots);
};
