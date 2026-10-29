# D-03 — Monstres. Test que lee el catálogo publicado desde
# assets/catalogs/monsters.json y comprueba: 1) ningún monstruo carga con
# vida 0, 2) las defensas, estadísticas, tier y habilidades llegan con
# valores sensatos, 3) al menos una fracción de la muestra no tiene
# defaults en todos los campos.
# GL-free como todos los tests de catálogo: RpgCatalogs.h es header-only.
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_monster_catalog tests/RPG/MonsterCatalogTests.cpp)
    target_include_directories(test_monster_catalog PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_monster_catalog PRIVATE motor_rpg)
    add_dependencies(test_monster_catalog copiar_assets)
    add_test(NAME test_monster_catalog COMMAND test_monster_catalog)
    set_tests_properties(test_monster_catalog PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_monster_catalog)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_monster_catalog)
    endif()
endif()
