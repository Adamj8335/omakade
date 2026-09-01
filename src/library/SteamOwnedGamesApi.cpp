#include "library/SteamOwnedGamesApi.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QtGlobal>

QString SteamOwnedGamesApi::messageForState(SteamApiState state, const QString& detail) {
  switch (state) {
  case SteamApiState::Offline:
    return QStringLiteral("Steam is unreachable. Showing cached owned games.");
  case SteamApiState::InvalidKey:
    return QStringLiteral("Steam rejected the API key. Replace it in settings.");
  case SteamApiState::PrivateProfile:
    return detail.isEmpty() ? QStringLiteral("Steam game details are private for this profile")
                            : detail;
  case SteamApiState::RateLimited:
    return QStringLiteral("Steam is rate limiting requests. Try again later.");
  case SteamApiState::RemoteError:
    return detail.isEmpty() ? QStringLiteral("Steam could not refresh owned games.") : detail;
  case SteamApiState::Ready:
    return {};
  }
  return detail;
}

SteamApiState SteamOwnedGamesApi::parse(const QByteArray& contents,
                                        QVector<SteamOwnedGameRecord>* games, QString* error) {
  if (games == nullptr) {
    if (error != nullptr) {
      *error = QStringLiteral("No owned-games destination was provided");
    }
    return SteamApiState::RemoteError;
  }
  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(contents, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    if (error != nullptr) {
      *error = QStringLiteral("Steam returned malformed owned-game data");
    }
    return SteamApiState::RemoteError;
  }
  const QJsonObject response = document.object().value(QStringLiteral("response")).toObject();
  if (!response.contains(QStringLiteral("game_count"))) {
    if (error != nullptr) {
      *error = QStringLiteral("Steam game details are private for this profile");
    }
    return SteamApiState::PrivateProfile;
  }
  const qint64 gameCount = response.value(QStringLiteral("game_count")).toInteger(-1);
  const QJsonValue gamesValue = response.value(QStringLiteral("games"));
  if (gameCount < 0 || gameCount > 100000 || (gameCount > 0 && !gamesValue.isArray())) {
    if (error != nullptr) {
      *error = QStringLiteral("Steam returned invalid owned-game data");
    }
    return SteamApiState::RemoteError;
  }

  QVector<SteamOwnedGameRecord> parsed;
  const QJsonArray entries = gamesValue.toArray();
  if (entries.size() != gameCount) {
    if (error != nullptr) {
      *error = QStringLiteral("Steam returned an incomplete owned-game list");
    }
    return SteamApiState::RemoteError;
  }
  parsed.reserve(entries.size());
  QSet<QString> appIds;
  for (const QJsonValue& value : entries) {
    const QJsonObject game = value.toObject();
    const qint64 appId = game.value(QStringLiteral("appid")).toInteger();
    const QString title = game.value(QStringLiteral("name")).toString().trimmed();
    const QString normalizedAppId = QString::number(appId);
    if (appId <= 0 || title.isEmpty() || appIds.contains(normalizedAppId)) {
      if (error != nullptr) {
        *error = QStringLiteral("Steam returned invalid owned-game entries");
      }
      return SteamApiState::RemoteError;
    }
    appIds.insert(normalizedAppId);
    parsed.append(
        {.appId = normalizedAppId,
         .title = title.left(300),
         .playtimeMinutes = qBound(0, game.value(QStringLiteral("playtime_forever")).toInt(),
                                   60 * 24 * 365 * 200)});
  }
  *games = parsed;
  if (error != nullptr) {
    error->clear();
  }
  return SteamApiState::Ready;
}
