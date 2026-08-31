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
