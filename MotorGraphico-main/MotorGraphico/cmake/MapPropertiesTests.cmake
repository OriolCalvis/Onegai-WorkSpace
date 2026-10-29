if (BUILD_TESTING)
    add_executable(test_map_properties tests/Editor/MapPropertiesTests.cpp)
    target_include_directories(test_map_properties PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_map_properties PRIVATE motor_level motor_map)
    add_dependencies(test_map_properties copiar_assets)
    add_test(NAME test_map_properties COMMAND test_map_properties)
    set_tests_properties(test_map_properties PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_map_properties)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_map_properties)
    endif ()
endif ()
