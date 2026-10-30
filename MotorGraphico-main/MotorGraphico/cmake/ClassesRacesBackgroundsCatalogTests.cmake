# D-06 — Classes, races i trasfons (part de catàlegs RPG, contingut real).
# Complement de test_character_creation (ja existent) que prova els 3 catàlegs
# directament contra assets/catalogs publicats.

if(BUILD_TESTING)
    add_executable(test_catalog_classes_races_backgrounds
        tests/RPG/ClassesRacesBackgroundsCatalogTests.cpp)
    target_include_directories(test_catalog_classes_races_backgrounds PRIVATE tests)
    target_link_libraries(test_catalog_classes_races_backgrounds PRIVATE
        motor_rpg
        motor_json
        motor_core
    )
    if(TARGET copiar_assets)
        add_dependencies(test_catalog_classes_races_backgrounds copiar_assets)
    endif()
    add_test(NAME test_catalog_classes_races_backgrounds
        COMMAND test_catalog_classes_races_backgrounds)
    set_tests_properties(test_catalog_classes_races_backgrounds PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_catalog_classes_races_backgrounds)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_catalog_classes_races_backgrounds)
    endif()
endif()
