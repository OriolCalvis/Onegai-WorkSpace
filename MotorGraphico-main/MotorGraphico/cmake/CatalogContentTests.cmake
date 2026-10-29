# D-02 — Contenido real del catálogo de habilidades y hechizos: tres cartas
# concretas con sus valores escritos en el test (tier 1, tier 5 y una
# utilitaria de check), un hechizo, y que dos cartas diferentes den números
# diferentes. Lee los assets publicados desde el directorio de build (de
# ahí copiar_assets y WORKING_DIRECTORY), igual que test_catalog_contract.
# GL-free: RpgCatalogs.h es header-only y motor_rpg no incluye ni Render
# ni GL.
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_catalog_content tests/RPG/CatalogContentTests.cpp)
    target_include_directories(test_catalog_content PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_catalog_content PRIVATE motor_rpg)
    add_dependencies(test_catalog_content copiar_assets)
    add_test(NAME test_catalog_content COMMAND test_catalog_content)
    set_tests_properties(test_catalog_content PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_catalog_content)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_catalog_content)
    endif()
endif()
