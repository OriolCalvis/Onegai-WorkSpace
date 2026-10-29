add_executable(test_export_panel
    tests/Editor/ExportPanelTests.cpp
    src/Core/Json/JsonValue.cpp
    editor/src/ExportPanel.cpp)
target_include_directories(test_export_panel PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/include")
target_link_libraries(test_export_panel PRIVATE motor_core)
add_test(NAME test_export_panel COMMAND test_export_panel)
set_tests_properties(test_export_panel PROPERTIES
    LABELS "unit;gl-free" TIMEOUT 600)
set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_export_panel)
if (TARGET suite_gl_free)
    add_dependencies(suite_gl_free test_export_panel)
endif()
