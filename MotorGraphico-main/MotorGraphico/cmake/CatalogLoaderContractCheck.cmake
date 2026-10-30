# D-01 — El contrato de cada cargador RPG dice la verdad.
#
# catalog_loader_contract: analisis estatico (python3, no compila nada) que
# compara las claves que cada CatalogLoader<T>::from_json lee de la entrada
# con las que declara expected_fields(). De esa declaracion viven el modo
# AllDeclared, los `required` de D-02..D-09 y la linea base de D-10: una
# lectura sin declarar escapa a las tres redes sin que nada lo diga.
# catalog_loader_contract_selftest demuestra que el guardia salta (sobre
# copias; no toca el repositorio).
#
# test_catalog_loader_regressions: las regresiones que el guardia destapo
# (f23dba9 dejo a todas las razas con velocidad 30 y sin deidades).
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME catalog_loader_contract
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/comprobar_contrato_cargadores.py"
            "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(catalog_loader_contract PROPERTIES LABELS "static;gl-free" TIMEOUT 30)
    add_test(NAME catalog_loader_contract_selftest
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/comprobar_contrato_cargadores.py"
            --autoprueba "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(catalog_loader_contract_selftest PROPERTIES LABELS "static;gl-free" TIMEOUT 60)
endif()
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_catalog_loader_regressions tests/RPG/CatalogLoaderRegressionTests.cpp)
    target_include_directories(test_catalog_loader_regressions PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_catalog_loader_regressions PRIVATE motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_catalog_loader_regressions copiar_assets)
    endif()
    add_test(NAME test_catalog_loader_regressions COMMAND test_catalog_loader_regressions)
    set_tests_properties(test_catalog_loader_regressions PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_catalog_loader_regressions)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_catalog_loader_regressions)
    endif()
endif()
