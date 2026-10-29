# D-07 — Els catàlegs restants (passives, feats, traits, deities,
# conditions, summons, mounts, traps, locations). Complement de
# test_catalog_content (D-02/D-10) i de CatalogContract (D-01).

if(BUILD_TESTING)
    add_executable(test_catalog_restants
        tests/RPG/RestOfCatalogsTests.cpp)
    target_include_directories(test_catalog_restants PRIVATE tests)
    target_link_libraries(test_catalog_restants PRIVATE
        motor_rpg
        motor_json
        motor_core
    )
    if(TARGET copiar_assets)
        add_dependencies(test_catalog_restants copiar_assets)
    endif()
    add_test(NAME test_catalog_restants COMMAND test_catalog_restants)
    set_tests_properties(test_catalog_restants PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_catalog_restants)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_catalog_restants)
    endif()
endif()
