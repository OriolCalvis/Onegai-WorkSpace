# D-01 — Contrato de campos de los cargadores de catalogo: un cargador que no
# encuentra lo que espera falla nombrando el campo y la entrada, en vez de
# devolver defaults en silencio. Lee los 20 assets publicados desde el
# directorio de build (de ahi copiar_assets y WORKING_DIRECTORY), igual que
# demo_catalogos_rpg. GL-free: RpgCatalogs.h es header-only y motor_rpg no
# incluye ni Render ni GL.
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_catalog_contract tests/RPG/CatalogContractTests.cpp)
    target_include_directories(test_catalog_contract PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_catalog_contract PRIVATE motor_rpg)
    add_dependencies(test_catalog_contract copiar_assets)
    add_test(NAME test_catalog_contract COMMAND test_catalog_contract)
    set_tests_properties(test_catalog_contract PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_catalog_contract)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_catalog_contract)
    endif()
endif()
