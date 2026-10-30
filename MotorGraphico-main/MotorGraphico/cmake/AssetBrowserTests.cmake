if (BUILD_TESTING)
    add_executable(test_asset_browser tests/Editor/AssetBrowserTests.cpp)
    target_include_directories(test_asset_browser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_asset_browser PRIVATE motor_level motor_map motor_project)
    add_dependencies(test_asset_browser copiar_assets)
    add_test(NAME test_asset_browser COMMAND test_asset_browser)
    set_tests_properties(test_asset_browser PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_asset_browser)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_asset_browser)
    endif ()
endif ()
