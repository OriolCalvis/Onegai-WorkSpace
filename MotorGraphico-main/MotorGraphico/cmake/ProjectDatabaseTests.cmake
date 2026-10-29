if (BUILD_TESTING)
    add_executable(test_project_database tests/Editor/ProjectDatabaseTests.cpp src/Project/ProjectDatabaseState.cpp)
    target_include_directories(test_project_database PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_project_database PRIVATE motor_json)
    add_test(NAME test_project_database COMMAND test_project_database)
    set_tests_properties(test_project_database PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_project_database)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_project_database)
    endif ()
endif ()
