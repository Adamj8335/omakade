#pragma once

#include "achievements/SteamAchievementApi.h"

#include <QByteArray>
#include <QString>
#include <QVector>

struct SteamOwnedGameRecord {
  QString appId;
  QString title;
  int playtimeMinutes = 0;
};

class SteamOwnedGamesApi final {
public:
  [[nodiscard]] static QString messageForState(SteamApiState state, const QString& detail = {});
  [[nodiscard]] static SteamApiState
  parse(const QByteArray& response, QVector<SteamOwnedGameRecord>* games, QString* error = nullptr);
};
