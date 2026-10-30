if (BUILD_TESTING)
    add_executable(test_event_editor tests/Editor/EventEditorTests.cpp)
    target_include_directories(test_event_editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_event_editor PRIVATE motor_level motor_map)
    add_dependencies(test_event_editor copiar_assets)
    add_test(NAME test_event_editor COMMAND test_event_editor)
    set_tests_properties(test_event_editor PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_event_editor)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_event_editor)
    endif ()
endif ()
