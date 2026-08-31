cmake_minimum_required(VERSION 3.24)

project(Omakade VERSION 0.2.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Concurrent Core Gui Network Qml Quick QuickControls2 Sql Test)
qt_standard_project_setup(REQUIRES 6.8)

add_library(omakade_core STATIC
  src/app/AppSettings.cpp
  src/app/AppSettings.h
  src/app/SingleInstance.cpp
  src/app/SingleInstance.h
  src/input/ControllerInput.cpp
  src/input/ControllerInput.h
  src/library/LibraryFilterModel.cpp
  src/library/LibraryFilterModel.h
  src/library/GameRoles.h
  src/library/MockGameModel.cpp
  src/library/MockGameModel.h
  src/library/SteamGameModel.cpp
  src/library/SteamGameModel.h
  src/launch/SteamLauncher.cpp
  src/launch/SteamLauncher.h
  src/sources/steam/SteamScanner.cpp
  src/sources/steam/SteamScanner.h
  src/sources/steam/ValveKeyValues.cpp
  src/sources/steam/ValveKeyValues.h
  src/theme/OmarchyTheme.cpp
  src/theme/OmarchyTheme.h
)

target_include_directories(omakade_core PUBLIC src)
target_link_libraries(omakade_core PUBLIC
  PkgConfig::SDL3
  Qt6::Concurrent
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Sql
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(omakade_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_add_executable(omakade
  src/app/main.cpp
)

qt_add_qml_module(omakade
  URI Omakade
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/components/GameCard.qml
    qml/components/GlassButton.qml
    qml/components/LibraryView.qml
    qml/screens/GameDetails.qml
)

set_target_properties(omakade PROPERTIES
  OUTPUT_NAME omakade
  MACOSX_BUNDLE_GUI_IDENTIFIER io.github.omakade.Omakade
  WIN32_EXECUTABLE TRUE
)

target_compile_definitions(omakade PRIVATE
  OMAKADE_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(omakade PRIVATE
  omakade_core
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::QuickControls2
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(omakade PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GNUInstallDirs)
install(TARGETS omakade
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES packaging/io.github.omakade.Omakade.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES resources/icons/io.github.omakade.Omakade.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
install(FILES packaging/io.github.omakade.Omakade.metainfo.xml
  DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
