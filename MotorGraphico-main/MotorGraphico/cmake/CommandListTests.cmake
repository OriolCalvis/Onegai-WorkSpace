if (BUILD_TESTING)
    add_executable(test_command_list tests/Editor/CommandListTests.cpp)
    target_include_directories(test_command_list PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_command_list PRIVATE motor_level motor_map)
    add_dependencies(test_command_list copiar_assets)
    add_test(NAME test_command_list COMMAND test_command_list)
    set_tests_properties(test_command_list PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_command_list)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_command_list)
    endif ()
endif ()
