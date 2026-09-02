#pragma once

#include "sources/steam/SteamScanner.h"

#include <QByteArray>
#include <QString>

enum class SteamApiState {
  Ready,
  Offline,
  InvalidKey,
  PrivateProfile,
  RateLimited,
  RemoteError,
};

struct SteamAchievementApiResult {
  int unlocked = 0;
  int total = 0;
  QVector<SteamAchievementRecord> achievements;
};

class SteamAchievementApi final {
public:
  [[nodiscard]] static QString authenticatedHost();
  [[nodiscard]] static SteamApiState classifyHttpResponse(int statusCode, bool networkError);
  [[nodiscard]] static bool isNoStatsResponse(const QByteArray& playerResponse);
  [[nodiscard]] static bool isPrivateProfileResponse(const QByteArray& playerResponse);
  [[nodiscard]] static SteamApiState parse(const QByteArray& playerResponse,
                                           const QByteArray& schemaResponse,
                                           const QByteArray& rarityResponse,
                                           SteamAchievementApiResult* result,
                                           QString* error = nullptr);
};
