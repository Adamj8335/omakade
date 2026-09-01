#include "metadata/IgdbApi.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <cmath>

namespace {
QJsonObject firstObject(const QByteArray& contents, QString* error) {
  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(contents, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
    if (error != nullptr) {
      *error = QStringLiteral("IGDB returned invalid data");
    }
    return {};
  }
  const QJsonArray values = document.array();
  if (values.isEmpty() || !values.at(0).isObject()) {
    if (error != nullptr) {
      *error = QStringLiteral("No matching IGDB game was found");
    }
    return {};
  }
  return values.at(0).toObject();
}

int boundedInteger(const QJsonValue& value) {
  const double number = value.toDouble();
  return std::isfinite(number) && number > 0 && number <= 2147483647.0
             ? static_cast<int>(std::round(number))
             : 0;
}
} // namespace

QByteArray IgdbApi::steamMappingQuery(const QString& appId) {
  static const QRegularExpression numeric(QStringLiteral("^[1-9][0-9]*$"));
  if (!numeric.match(appId).hasMatch()) {
    return {};
  }
  return QStringLiteral("fields game; where uid = \"%1\" & external_game_source.name = "
                        "\"Steam\"; limit 1;")
      .arg(appId)
      .toUtf8();
}

QByteArray IgdbApi::gameQuery(qint64 gameId) {
  if (gameId <= 0) {
    return {};
  }
  return QStringLiteral("fields name,aggregated_rating,aggregated_rating_count; where id = %1; "
                        "limit 1;")
      .arg(gameId)
      .toUtf8();
}

QByteArray IgdbApi::timeToBeatQuery(qint64 gameId) {
  if (gameId <= 0) {
    return {};
  }
  return QStringLiteral("fields game_id,hastily,normally,completely,count; where game_id = %1; "
                        "limit 1;")
      .arg(gameId)
      .toUtf8();
}

bool IgdbApi::parseSteamMapping(const QByteArray& contents, qint64* gameId, QString* error) {
  if (gameId == nullptr) {
    return false;
  }
  const QJsonObject value = firstObject(contents, error);
  const qint64 parsed = value.value(QStringLiteral("game")).toInteger();
  if (parsed <= 0) {
    if (error != nullptr && error->isEmpty()) {
      *error = QStringLiteral("IGDB returned an invalid game mapping");
    }
    return false;
  }
  *gameId = parsed;
  return true;
}

bool IgdbApi::parseGame(const QByteArray& contents, IgdbGameInsight* insight, QString* error) {
  if (insight == nullptr) {
    return false;
  }
  const QJsonObject value = firstObject(contents, error);
  const qint64 parsedId = value.value(QStringLiteral("id")).toInteger();
  const QString title = value.value(QStringLiteral("name")).toString().trimmed();
  if (parsedId <= 0 || title.isEmpty()) {
    if (error != nullptr && error->isEmpty()) {
      *error = QStringLiteral("IGDB returned incomplete game data");
    }
    return false;
  }
  insight->gameId = parsedId;
  insight->title = title;
  const double score = value.value(QStringLiteral("aggregated_rating")).toDouble(-1);
  insight->criticScore =
      std::isfinite(score) && score >= 0 && score <= 100 ? static_cast<int>(std::round(score)) : -1;
  insight->criticReviewCount =
      boundedInteger(value.value(QStringLiteral("aggregated_rating_count")));
  return true;
}

bool IgdbApi::parseTimeToBeat(const QByteArray& contents, IgdbGameInsight* insight,
                              QString* error) {
  if (insight == nullptr) {
    return false;
  }
  const QJsonObject value = firstObject(contents, error);
  const qint64 parsedId = value.value(QStringLiteral("game_id")).toInteger();
  if (parsedId <= 0 || (insight->gameId > 0 && insight->gameId != parsedId)) {
    if (error != nullptr && error->isEmpty()) {
      *error = QStringLiteral("IGDB returned time data for a different game");
    }
    return false;
  }
  insight->gameId = parsedId;
  insight->rushedSeconds = boundedInteger(value.value(QStringLiteral("hastily")));
  insight->normalSeconds = boundedInteger(value.value(QStringLiteral("normally")));
  insight->completeSeconds = boundedInteger(value.value(QStringLiteral("completely")));
  insight->timeSampleCount = boundedInteger(value.value(QStringLiteral("count")));
  return true;
}
