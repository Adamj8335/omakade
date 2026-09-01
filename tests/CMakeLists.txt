qt_add_executable(omakade_core_tests
  CoreTests.cpp
)

target_link_libraries(omakade_core_tests PRIVATE
  omakade_core
  Qt6::Core
  Qt6::Gui
  Qt6::Test
)

add_test(NAME omakade_core_tests COMMAND omakade_core_tests)
set_tests_properties(omakade_core_tests PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_test(NAME omakade_qml_smoke COMMAND omakade --smoke-test)
set_tests_properties(omakade_qml_smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 10
)

add_test(NAME omakade_controller_navigation COMMAND omakade --controller-navigation-test)
set_tests_properties(omakade_controller_navigation PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 10
)

add_test(NAME omakade_controller_navigation_tiled
  COMMAND omakade --controller-navigation-test --owned-layout-test --render-size=820x590
)
set_tests_properties(omakade_controller_navigation_tiled PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 10
)

add_test(NAME omakade_render_smoke
  COMMAND omakade --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-render.png
          --render-size=820x590
)
set_tests_properties(omakade_render_smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 10
)

add_test(NAME omakade_owned_layout_render
  COMMAND omakade --owned-layout-test
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-owned-layout.png
          --render-size=820x590
)
set_tests_properties(omakade_owned_layout_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 10
)

add_test(NAME omakade_uninstalled_detail_render
  COMMAND omakade --owned-layout-test --uninstalled-layout-test
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-uninstalled-detail.png
          --render-size=820x590
)
set_tests_properties(omakade_uninstalled_detail_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 10
)
