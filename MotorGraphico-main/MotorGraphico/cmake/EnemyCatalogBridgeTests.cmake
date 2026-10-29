if (BUILD_TESTING)
    add_executable(test_enemy_catalog_bridge tests/Game/EnemyCatalogBridgeTests.cpp)
    target_include_directories(test_enemy_catalog_bridge PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_enemy_catalog_bridge PRIVATE motor_level motor_rpg motor_json)
    add_dependencies(test_enemy_catalog_bridge copiar_assets)
    add_test(NAME test_enemy_catalog_bridge COMMAND test_enemy_catalog_bridge)
    set_tests_properties(test_enemy_catalog_bridge PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_enemy_catalog_bridge)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_enemy_catalog_bridge)
    endif ()
endif ()
