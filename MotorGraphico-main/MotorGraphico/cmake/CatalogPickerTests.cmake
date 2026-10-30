if (BUILD_TESTING)
    add_executable(test_catalog_picker tests/Editor/CatalogPickerTests.cpp src/Editor/CatalogPickerState.cpp)
    target_include_directories(test_catalog_picker PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_catalog_picker PRIVATE motor_rpg motor_json)
    add_dependencies(test_catalog_picker copiar_assets)
    add_test(NAME test_catalog_picker COMMAND test_catalog_picker)
    set_tests_properties(test_catalog_picker PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_catalog_picker)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_catalog_picker)
    endif ()
endif ()
