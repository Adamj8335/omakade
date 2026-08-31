#include "achievements/AchievementModel.h"
#include "app/AppSettings.h"
#include "app/SingleInstance.h"
#include "input/ControllerInput.h"
#include "launch/SteamLauncher.h"
#include "library/LibraryFilterModel.h"
#include "library/MockGameModel.h"
#include "library/SteamGameModel.h"
#include "theme/OmarchyTheme.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QIcon>
#include <QKeyEvent>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QWindow>

#include <memory>

int main(int argc, char* argv[]) {
  QElapsedTimer startupTimer;
  startupTimer.start();

  QGuiApplication::setApplicationName(QStringLiteral("Omakade"));
  QGuiApplication::setApplicationDisplayName(QStringLiteral("Omakade"));
  QGuiApplication::setApplicationVersion(QStringLiteral(OMAKADE_VERSION));
  QGuiApplication::setOrganizationName(QStringLiteral("Omakade"));
  QGuiApplication::setDesktopFileName(QStringLiteral("io.github.omakade.Omakade"));
  QQuickStyle::setStyle(QStringLiteral("Basic"));

  QGuiApplication application(argc, argv);
  application.setWindowIcon(QIcon::fromTheme(QStringLiteral("io.github.omakade.Omakade")));

  OmarchyTheme theme;
  const bool smokeTest = application.arguments().contains(QStringLiteral("--smoke-test"));
  const bool demoMode = smokeTest || application.arguments().contains(QStringLiteral("--demo"));
  const bool benchmarkMode = application.arguments().contains(QStringLiteral("--benchmark"));
  const bool stressMode = application.arguments().contains(QStringLiteral("--stress-test"));
  if (benchmarkMode) {
    qInfo() << "Theme ready in" << startupTimer.elapsed() << "ms";
  }
  SingleInstance singleInstance;
  if (!smokeTest && !singleInstance.claimOrNotify()) {
    return EXIT_SUCCESS;
  }
  AppSettings preferences;
  ControllerInput controller;
  std::unique_ptr<QAbstractItemModel> games;
  SteamGameModel* steamLibrary = nullptr;
  if (demoMode || stressMode) {
    games = std::make_unique<MockGameModel>(nullptr, stressMode ? 1000 : 100);
  } else {
    auto steam = std::make_unique<SteamGameModel>(QString{}, &preferences);
    steamLibrary = steam.get();
    games = std::move(steam);
  }
  LibraryFilterModel library;
  library.setSourceModel(games.get());
  AchievementModel achievements(steamLibrary == nullptr ? QStringLiteral(":memory:")
                                                        : steamLibrary->databasePath(),
                                &preferences);
  SteamLauncher launcher;

  QObject::connect(&controller, &ControllerInput::keyRequested, &application,
                   [&application](int key, int modifiers) {
                     QWindow* window = application.focusWindow();
                     if (window == nullptr) {
                       return;
                     }
                     const auto keyboardModifiers = static_cast<Qt::KeyboardModifiers>(modifiers);
                     QCoreApplication::postEvent(
                         window, new QKeyEvent(QEvent::KeyPress, key, keyboardModifiers));
                     QCoreApplication::postEvent(
                         window, new QKeyEvent(QEvent::KeyRelease, key, keyboardModifiers));
                   });

  QQmlApplicationEngine engine;
  QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError>& warnings) {
    for (const QQmlError& warning : warnings) {
      qWarning().noquote() << warning.toString();
    }
  });
  engine.rootContext()->setContextProperty(QStringLiteral("Theme"), &theme);
  engine.rootContext()->setContextProperty(QStringLiteral("Library"), &library);
  engine.rootContext()->setContextProperty(QStringLiteral("SteamLibrary"), steamLibrary);
  engine.rootContext()->setContextProperty(QStringLiteral("Launcher"), &launcher);
  engine.rootContext()->setContextProperty(QStringLiteral("Preferences"), &preferences);
  engine.rootContext()->setContextProperty(QStringLiteral("Controller"), &controller);
  engine.rootContext()->setContextProperty(QStringLiteral("Achievements"), &achievements);
  engine.rootContext()->setContextProperty(QStringLiteral("DemoMode"), demoMode || stressMode);
  engine.rootContext()->setContextProperty(QStringLiteral("StartupMilliseconds"),
                                           startupTimer.elapsed());

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
      [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
  engine.loadFromModule(QStringLiteral("Omakade"), QStringLiteral("Main"));
  if (benchmarkMode) {
    qInfo() << "QML loaded in" << startupTimer.elapsed() << "ms";
  }
  if (engine.rootObjects().isEmpty()) {
    qCritical() << "Omakade failed to create its QML root object";
    return EXIT_FAILURE;
  }

  auto* rootWindow = qobject_cast<QWindow*>(engine.rootObjects().constFirst());
  if (auto* quickWindow = qobject_cast<QQuickWindow*>(rootWindow)) {
    QObject::connect(
        quickWindow, &QQuickWindow::frameSwapped, &application,
        [&application, &controller, &startupTimer, benchmarkMode] {
          qInfo() << "First frame in" << startupTimer.elapsed() << "ms";
          controller.start();
          if (benchmarkMode) {
            application.quit();
          }
        },
        Qt::SingleShotConnection);
  }
  QObject::connect(&singleInstance, &SingleInstance::activationRequested, &application,
                   [rootWindow] {
                     if (rootWindow != nullptr) {
                       rootWindow->show();
                       rootWindow->requestActivate();
                     }
                   });

  if (steamLibrary != nullptr) {
    QTimer::singleShot(0, steamLibrary, &SteamGameModel::refresh);
  }

  if (smokeTest) {
    QTimer::singleShot(600, &application, &QCoreApplication::quit);
  }

  return application.exec();
}
