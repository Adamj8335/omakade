#pragma once

#include <Qt>

namespace GameRoles {
enum Role {
  Title = Qt::UserRole + 1,
  Subtitle,
  Description,
  Hours,
  Progress,
  AchievementsUnlocked,
  AchievementsTotal,
  Favorite,
  Recent,
  AccentStart,
  AccentEnd,
  CoverMark,
  Year,
  AppId,
  CoverPath,
  HeroPath,
  LogoPath,
  InstallPath,
  Source,
  Hidden,
};
}
