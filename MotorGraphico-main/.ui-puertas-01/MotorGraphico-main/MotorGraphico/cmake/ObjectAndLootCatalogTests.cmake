# D-05 — Objectes, equipament i consumibles + Loot Tables (catalogs RPG).
# Target nou sense col·lisions amb cap altre cmake fragment; l'única línia
# que toca a CMakeLists.txt és: include(cmake/ObjectAndLootCatalogTests.cmake)

if(BUILD_TESTING)
    add_executable(test_object_and_loot_catalog tests/RPG/ObjectAndLootCatalogTests.cpp)
    target_include_directories(test_object_and_loot_catalog PRIVATE tests)
    target_link_libraries(test_object_and_loot_catalog PRIVATE
        motor_rpg
        motor_json
        motor_core
        motor_level
    )
    if(TARGET copiar_assets)
        add_dependencies(test_object_and_loot_catalog copiar_assets)
    endif()
    add_test(NAME test_object_and_loot_catalog COMMAND test_object_and_loot_catalog)
    set_tests_properties(test_object_and_loot_catalog PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_object_and_loot_catalog)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_object_and_loot_catalog)
    endif()
endif()
