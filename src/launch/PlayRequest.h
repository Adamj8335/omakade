#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QVariantMap>

class GameLauncher;
class UnifiedGameModel;

// Identifies one installation of a game the way Omakade stores it: the source name, the
// Heroic or Lutris runner (often empty), and the source's own game id. The text form
// "Source:runner:id" is what `omakade --play` and Sunshine app entries carry. The id is
// everything after the second colon, so RetroArch content paths survive unchanged.
struct LaunchKey {
  QString source;
  QString runner;
  QString appId;

  [[nodiscard]] static LaunchKey parse(const QString& text);
  [[nodiscard]] QString toString() const;
  [[nodiscard]] bool isValid() const { return !source.isEmpty() && !appId.isEmpty(); }
};

namespace PlayRequest {
// Finds the row and installation that match the key, searching every installation of a
// linked game. Hidden games are included so a Sunshine entry keeps working.
[[nodiscard]] QVariantMap findInstallation(const UnifiedGameModel& games, const LaunchKey& key,
                                           int* row);
// Launches the matching installation through the owning platform and records the launch.
// Returns false and fills `error` when the game is missing, not installed, or fails.
bool perform(UnifiedGameModel& games, GameLauncher& launcher, const LaunchKey& key,
             QString* error);
} // namespace PlayRequest
