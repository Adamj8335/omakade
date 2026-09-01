#include "achievements/SteamAchievementApi.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace {
QJsonObject parseObject(const QByteArray& contents, bool* okay) {
  QJsonParseError error;
  const QJsonDocument document = QJsonDocument::fromJson(contents, &error);
  *okay = error.error == QJsonParseError::NoError && document.isObject();
  return *okay ? document.object() : QJsonObject{};
}

SteamApiState playerErrorState(const QString& message) {
  const QString normalized = message.toLower();
  if (normalized.contains(QStringLiteral("private")) ||
      normalized.contains(QStringLiteral("not be found"))) {
    return SteamApiState::PrivateProfile;
  }
  if (normalized.contains(QStringLiteral("key"))) {
    return SteamApiState::InvalidKey;
  }
  return SteamApiState::RemoteError;
}
} // namespace

SteamApiState SteamAchievementApi::classifyHttpResponse(int statusCode, bool networkError) {
  if (statusCode == 401 || statusCode == 403) {
    return SteamApiState::InvalidKey;
  }
  if (statusCode == 429) {
    return SteamApiState::RateLimited;
  }
  if (statusCode >= 200 && statusCode < 300) {
    return SteamApiState::Ready;
  }
  if (statusCode > 0) {
    return SteamApiState::RemoteError;
  }
  if (networkError) {
    return SteamApiState::Offline;
  }
  return SteamApiState::RemoteError;
}

SteamApiState SteamAchievementApi::parse(const QByteArray& playerResponse,
                                         const QByteArray& schemaResponse,
                                         const QByteArray& rarityResponse,
                                         SteamAchievementApiResult* result, QString* error) {
  if (result == nullptr) {
    if (error != nullptr) {
      *error = QStringLiteral("No achievement result destination was provided");
    }
    return SteamApiState::RemoteError;
  }

  bool playerOkay = false;
  bool schemaOkay = false;
  bool rarityOkay = false;
  const QJsonObject playerRoot = parseObject(playerResponse, &playerOkay);
  const QJsonObject schemaRoot = parseObject(schemaResponse, &schemaOkay);
  const QJsonObject rarityRoot = parseObject(rarityResponse, &rarityOkay);
  if (!playerOkay || !schemaOkay) {
    if (error != nullptr) {
      *error = QStringLiteral("Steam returned malformed achievement data");
    }
    return SteamApiState::RemoteError;
  }

  const QJsonObject playerStats = playerRoot.value(QStringLiteral("playerstats")).toObject();
  if (!playerStats.value(QStringLiteral("success")).toBool()) {
    const QString message =
        playerStats.value(QStringLiteral("error"))
            .toString(QStringLiteral("Steam did not return player achievement data"));
    if (error != nullptr) {
      *error = message;
    }
    return playerErrorState(message);
  }

  struct PlayerProgress {
    bool unlocked = false;
    qint64 unlockTime = 0;
  };
  QHash<QString, PlayerProgress> progress;
  for (const QJsonValue& value : playerStats.value(QStringLiteral("achievements")).toArray()) {
    const QJsonObject achievement = value.toObject();
    const QString apiName = achievement.value(QStringLiteral("apiname")).toString();
    if (!apiName.isEmpty()) {
      progress.insert(apiName,
                      {.unlocked = achievement.value(QStringLiteral("achieved")).toInt() != 0,
                       .unlockTime = achievement.value(QStringLiteral("unlocktime")).toInteger()});
    }
  }

  QHash<QString, double> rarity;
  if (rarityOkay) {
    const QJsonArray percentages = rarityRoot.value(QStringLiteral("achievementpercentages"))
                                       .toObject()
                                       .value(QStringLiteral("achievements"))
                                       .toArray();
    for (const QJsonValue& value : percentages) {
      const QJsonObject achievement = value.toObject();
      rarity.insert(achievement.value(QStringLiteral("name")).toString(),
                    achievement.value(QStringLiteral("percent")).toDouble());
    }
  }

  SteamAchievementApiResult parsed;
  const QJsonArray schema = schemaRoot.value(QStringLiteral("game"))
                                .toObject()
                                .value(QStringLiteral("availableGameStats"))
                                .toObject()
                                .value(QStringLiteral("achievements"))
                                .toArray();
  for (const QJsonValue& value : schema) {
    const QJsonObject achievement = value.toObject();
    const QString apiName = achievement.value(QStringLiteral("name")).toString();
    if (apiName.isEmpty()) {
      continue;
    }
    const PlayerProgress player = progress.value(apiName);
    parsed.achievements.append({
        .apiName = apiName,
        .title = achievement.value(QStringLiteral("displayName")).toString(apiName),
        .description = achievement.value(QStringLiteral("description")).toString(),
        .iconUrl = achievement.value(QStringLiteral("icon")).toString(),
        .unlocked = player.unlocked,
        .unlockTime = player.unlockTime,
        .rarity = rarity.value(apiName),
        .hidden = achievement.value(QStringLiteral("hidden")).toInt() != 0,
        .currentProgress = player.unlocked ? 1.0 : 0.0,
        .maximumProgress = 1.0,
    });
    if (player.unlocked) {
      ++parsed.unlocked;
    }
  }
  parsed.total = parsed.achievements.size();

  if (parsed.achievements.isEmpty() && !progress.isEmpty()) {
    for (auto iterator = progress.cbegin(); iterator != progress.cend(); ++iterator) {
      parsed.achievements.append({
          .apiName = iterator.key(),
          .title = iterator.key(),
          .description = {},
          .iconUrl = {},
          .unlocked = iterator.value().unlocked,
          .unlockTime = iterator.value().unlockTime,
          .rarity = rarity.value(iterator.key()),
          .hidden = false,
          .currentProgress = iterator.value().unlocked ? 1.0 : 0.0,
          .maximumProgress = 1.0,
      });
      if (iterator.value().unlocked) {
        ++parsed.unlocked;
      }
    }
    parsed.total = parsed.achievements.size();
  }

  *result = parsed;
  if (error != nullptr) {
    error->clear();
  }
  return SteamApiState::Ready;
}
