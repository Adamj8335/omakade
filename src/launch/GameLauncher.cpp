#include "launch/GameLauncher.h"

#include "launch/SteamLauncher.h"

#include <QDesktopServices>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrlQuery>

namespace {
bool validLutrisId(const QString& id) {
  static const QRegularExpression digits(QStringLiteral("^[1-9][0-9]*$"));
  return digits.match(id).hasMatch();
}

bool validHeroicTarget(const QString& id, const QString& runner) {
  static const QRegularExpression appId(QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._-]{0,255}$"));
  return appId.match(id).hasMatch() &&
         (runner == QStringLiteral("legendary") || runner == QStringLiteral("gog") ||
          runner == QStringLiteral("nile"));
}
} // namespace

GameLauncher::GameLauncher(QObject* parent) : QObject(parent) {}

QString GameLauncher::lastError() const { return m_lastError; }

LaunchCommand GameLauncher::lutrisCommand(const QString& id, bool flatpak) {
  if (!validLutrisId(id)) {
    return {};
  }
  const QString target = QStringLiteral("lutris:rungameid/%1").arg(id);
  return flatpak
             ? LaunchCommand{QStringLiteral("flatpak"),
                             {QStringLiteral("run"), QStringLiteral("net.lutris.Lutris"), target}}
             : LaunchCommand{QStringLiteral("lutris"), {target}};
}

LaunchCommand GameLauncher::heroicCommand(const QString& id, const QString& runner, bool flatpak) {
  if (!validHeroicTarget(id, runner)) {
    return {};
  }
  QUrl url(QStringLiteral("heroic://launch"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("appName"), id);
  query.addQueryItem(QStringLiteral("runner"), runner);
  query.addQueryItem(QStringLiteral("gui"), QStringLiteral("false"));
  url.setQuery(query);
  const QString target = url.toString(QUrl::FullyEncoded);
  return flatpak
             ? LaunchCommand{QStringLiteral("flatpak"),
                             {QStringLiteral("run"), QStringLiteral("com.heroicgameslauncher.hgl"),
                              QStringLiteral("--no-gui"), target}}
             : LaunchCommand{QStringLiteral("heroic"), {QStringLiteral("--no-gui"), target}};
}

bool GameLauncher::launch(const QString& source, const QString& id, bool flatpak,
                          const QString& runner) {
  if (source.compare(QStringLiteral("Steam"), Qt::CaseInsensitive) == 0) {
    const QUrl url = SteamLauncher::launchUrl(id);
    if (!url.isValid() || url.isEmpty()) {
      setError(QStringLiteral("This game has an invalid Steam App ID."));
      return false;
    }
    if (!QDesktopServices::openUrl(url)) {
      setError(QStringLiteral("Steam could not open the game. Check that Steam is installed."));
      return false;
    }
    setError({});
    return true;
  }
  if (source.compare(QStringLiteral("Lutris"), Qt::CaseInsensitive) == 0) {
    return launchLutris(id, flatpak, false);
  }
  if (source.compare(QStringLiteral("Heroic"), Qt::CaseInsensitive) == 0) {
    return launchHeroic(id, runner, flatpak, false);
  }
  setError(QStringLiteral("%1 games cannot be launched yet.").arg(source));
  return false;
}

bool GameLauncher::manage(const QString& source, const QString& id, bool flatpak,
                          const QString& runner) {
  if (source.compare(QStringLiteral("Steam"), Qt::CaseInsensitive) == 0) {
    const QUrl url = SteamLauncher::manageUrl(id);
    if (!url.isValid() || url.isEmpty() || !QDesktopServices::openUrl(url)) {
      setError(QStringLiteral("Steam could not open the game details."));
      return false;
    }
    setError({});
    return true;
  }
  if (source.compare(QStringLiteral("Lutris"), Qt::CaseInsensitive) == 0) {
    return launchLutris(id, flatpak, true);
  }
  if (source.compare(QStringLiteral("Heroic"), Qt::CaseInsensitive) == 0) {
    return launchHeroic(id, runner, flatpak, true);
  }
  setError(QStringLiteral("%1 does not provide game management yet.").arg(source));
  return false;
}

bool GameLauncher::launchLutris(const QString& id, bool flatpak, bool manageOnly) {
  const QString executable = flatpak ? QStringLiteral("flatpak") : QStringLiteral("lutris");
  const bool available = !QStandardPaths::findExecutable(executable).isEmpty();
  LaunchCommand command;
  if (manageOnly) {
    command = flatpak ? LaunchCommand{QStringLiteral("flatpak"),
                                      {QStringLiteral("run"), QStringLiteral("net.lutris.Lutris")}}
                      : LaunchCommand{QStringLiteral("lutris"), {}};
  } else {
    command = lutrisCommand(id, flatpak);
  }
  if (!available || !command.isValid()) {
    setError(validLutrisId(id) ? QStringLiteral("Lutris is not installed.")
                               : QStringLiteral("This game has an invalid Lutris ID."));
    return false;
  }
  if (!QProcess::startDetached(command.program, command.arguments)) {
    setError(QStringLiteral("Lutris could not be started."));
    return false;
  }
  setError({});
  return true;
}

bool GameLauncher::launchHeroic(const QString& id, const QString& runner, bool flatpak,
                                bool manageOnly) {
  const QString executable = flatpak ? QStringLiteral("flatpak") : QStringLiteral("heroic");
  const bool available = !QStandardPaths::findExecutable(executable).isEmpty();
  LaunchCommand command;
  if (manageOnly) {
    command =
        flatpak
            ? LaunchCommand{QStringLiteral("flatpak"),
                            {QStringLiteral("run"), QStringLiteral("com.heroicgameslauncher.hgl")}}
            : LaunchCommand{QStringLiteral("heroic"), {}};
  } else {
    command = heroicCommand(id, runner, flatpak);
  }
  if (!available || !command.isValid()) {
    setError(validHeroicTarget(id, runner)
                 ? QStringLiteral("Heroic is not installed.")
                 : QStringLiteral("This game has an invalid Heroic target."));
    return false;
  }
  if (!QProcess::startDetached(command.program, command.arguments)) {
    setError(QStringLiteral("Heroic could not be started."));
    return false;
  }
  setError({});
  return true;
}

void GameLauncher::setError(const QString& error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
