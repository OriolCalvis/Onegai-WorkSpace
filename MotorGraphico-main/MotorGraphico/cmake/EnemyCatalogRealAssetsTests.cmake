# D-08 — Enemics del joc són els del catàleg. Complement a
# test_enemy_catalog_bridge (que utilitza JSON sintètic): aquest target
# prova EnemyCatalogBridge sobre assets/catalogs/monsters.json PUBLICATS
# reals, validant "Llup vs Drac reserves de daus i defenses diferents".

if(BUILD_TESTING)
    add_executable(test_enemy_catalog_real_assets
        tests/Game/EnemyCatalogRealAssetsTests.cpp)
    target_include_directories(test_enemy_catalog_real_assets PRIVATE tests)
    target_link_libraries(test_enemy_catalog_real_assets PRIVATE
        motor_level
        motor_rpg
        motor_json
        motor_core
    )
    if(TARGET copiar_assets)
        add_dependencies(test_enemy_catalog_real_assets copiar_assets)
    endif()
    add_test(NAME test_enemy_catalog_real_assets
        COMMAND test_enemy_catalog_real_assets)
    set_tests_properties(test_enemy_catalog_real_assets PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_enemy_catalog_real_assets)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_enemy_catalog_real_assets)
    endif()
endif()
