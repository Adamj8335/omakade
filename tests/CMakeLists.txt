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
  TIMEOUT 30
)

add_test(NAME omakade_controller_navigation
  COMMAND omakade --controller-navigation-test --uninstalled-layout-test
)
set_tests_properties(omakade_controller_navigation PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_controller_navigation_tiled
  COMMAND omakade --controller-navigation-test --owned-layout-test --uninstalled-layout-test
          --render-size=820x590
)
set_tests_properties(omakade_controller_navigation_tiled PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_controller_navigation_wide
  COMMAND omakade --controller-navigation-test --owned-layout-test --uninstalled-layout-test
          --render-size=1600x900
)

add_test(NAME omakade_couch_navigation
  COMMAND omakade --couch --couch-navigation-test --render-size=1920x1080
)
set_tests_properties(omakade_couch_navigation PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_thousand_game_startup
  COMMAND omakade --couch --stress-test --benchmark --benchmark-max-ms=500
)
set_tests_properties(omakade_couch_thousand_game_startup PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1;HYPRLAND_INSTANCE_SIGNATURE="
  TIMEOUT 5
)

add_test(NAME omakade_couch_thousand_game_navigation
  COMMAND omakade --couch --stress-test --couch-navigation-test --render-size=1920x1080
)
set_tests_properties(omakade_couch_thousand_game_navigation PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1;HYPRLAND_INSTANCE_SIGNATURE="
  TIMEOUT 10
)

add_test(NAME omakade_couch_owned_navigation
  COMMAND omakade --couch --couch-navigation-test --owned-layout-test
          --uninstalled-layout-test --render-size=1920x1080
)
set_tests_properties(omakade_couch_owned_navigation PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1;HYPRLAND_INSTANCE_SIGNATURE="
  TIMEOUT 10
)
set_tests_properties(omakade_controller_navigation_wide PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_render_smoke
  COMMAND omakade --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-render.png
          --render-size=820x590
)

add_test(NAME omakade_couch_render
  COMMAND omakade --couch
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_reduced_motion_render
  COMMAND omakade --couch --reduced-motion
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-reduced-motion.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_reduced_motion_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_light_opaque_render
  COMMAND omakade --couch
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-light-opaque.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_light_opaque_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1;XDG_STATE_HOME=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/light-state;XDG_CONFIG_HOME=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/empty-config;HYPRLAND_INSTANCE_SIGNATURE="
  TIMEOUT 30
)

foreach(couch_case IN ITEMS 720p 1440p ultrawide 4k)
  if(couch_case STREQUAL "720p")
    set(couch_size 1280x720)
  elseif(couch_case STREQUAL "1440p")
    set(couch_size 2560x1440)
  elseif(couch_case STREQUAL "ultrawide")
    set(couch_size 2560x1080)
  else()
    set(couch_size 3840x2160)
  endif()
  add_test(NAME omakade_couch_render_${couch_case}
    COMMAND omakade --couch
            --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-${couch_case}.png
            --render-size=${couch_size}
  )
  set_tests_properties(omakade_couch_render_${couch_case} PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
    TIMEOUT 30
  )
endforeach()

add_test(NAME omakade_couch_render_200_percent
  COMMAND omakade --couch
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-200-percent.png
          --render-size=1280x720
)
set_tests_properties(omakade_couch_render_200_percent PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1;QT_SCALE_FACTOR=2"
  TIMEOUT 30
)

add_test(NAME omakade_couch_detail_render
  COMMAND omakade --couch --uninstalled-layout-test
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-detail.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_detail_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_keyboard_render
  COMMAND omakade --couch
          --render-overlay=couch-search
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-keyboard.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_keyboard_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_browse_render
  COMMAND omakade --couch
          --render-overlay=couch-browse
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-browse.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_browse_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_text_entry_render
  COMMAND omakade --couch
          --render-overlay=couch-entry
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-text-entry.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_text_entry_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_settings_render
  COMMAND omakade --couch --owned-layout-test
          --render-overlay=couch-settings-top
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-settings.png
          --render-size=1920x1080
)
set_tests_properties(omakade_couch_settings_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_couch_settings_720p_render
  COMMAND omakade --couch --owned-layout-test
          --render-overlay=couch-settings-bottom
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-couch-settings-720p.png
          --render-size=1280x720
)
set_tests_properties(omakade_couch_settings_720p_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)
set_tests_properties(omakade_render_smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_render_space_separated_options
  COMMAND omakade --render-screenshot ${CMAKE_CURRENT_BINARY_DIR}/omakade-render-spaced.png
          --render-size 820x590
)
set_tests_properties(omakade_render_space_separated_options PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_owned_layout_render
  COMMAND omakade --owned-layout-test
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-owned-layout.png
          --render-size=820x590
)
set_tests_properties(omakade_owned_layout_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)

add_test(NAME omakade_uninstalled_detail_render
  COMMAND omakade --owned-layout-test --uninstalled-layout-test
          --render-screenshot=${CMAKE_CURRENT_BINARY_DIR}/omakade-uninstalled-detail.png
          --render-size=820x590
)
set_tests_properties(omakade_uninstalled_detail_render PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_FORCE_STDERR_LOGGING=1"
  TIMEOUT 30
)
