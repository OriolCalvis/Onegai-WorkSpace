# EST-56 — Lo que sobra: codigo al que no se refiere nadie, y repetido.
#
# tools/buscar_sobrante.py contesta dos preguntas que medir_alcance.py no
# contesta. La primera, porque medir_alcance no sigue los #include entre
# CABECERAS: el 24/09 daba 43 «sin alcance» y siete eran los
# CatalogLoaders* que RpgCatalogs.h incluye una por una. Un listado con
# falsos positivos no se usa para borrar.
#
# La prueba siembra su propio repositorio de bolsillo en un temporal: un
# detector probado contra el arbol que analiza no es falsable (mismo
# criterio que test_exportar.py).
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if (BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_buscar_sobrante
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_buscar_sobrante.py")
    set_tests_properties(test_buscar_sobrante PROPERTIES
        LABELS "static;gl-free" TIMEOUT 60)
endif ()
