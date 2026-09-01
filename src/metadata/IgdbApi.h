#pragma once

#include <QByteArray>
#include <QString>

struct IgdbGameInsight {
  qint64 gameId = 0;
  QString title;
  int criticScore = -1;
  int criticReviewCount = 0;
  int rushedSeconds = 0;
  int normalSeconds = 0;
  int completeSeconds = 0;
  int timeSampleCount = 0;
};

class IgdbApi final {
public:
  [[nodiscard]] static QByteArray steamMappingQuery(const QString& appId);
  [[nodiscard]] static QByteArray gameQuery(qint64 gameId);
  [[nodiscard]] static QByteArray timeToBeatQuery(qint64 gameId);

  [[nodiscard]] static bool parseSteamMapping(const QByteArray& contents, qint64* gameId,
                                              QString* error = nullptr);
  [[nodiscard]] static bool parseGame(const QByteArray& contents, IgdbGameInsight* insight,
                                      QString* error = nullptr);
  [[nodiscard]] static bool parseTimeToBeat(const QByteArray& contents,
                                            IgdbGameInsight* insight,
                                            QString* error = nullptr);
};
