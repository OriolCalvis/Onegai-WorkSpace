# La guardia del core (tools/linea_base_core.py).
#
# Dos pruebas, a proposito separadas:
#
#   linea_base_core_configura — instantanea. Comprueba que cada
#       include(cmake/X.cmake) de CMakeLists.txt tiene su fichero
#       versionado y que no se mezclan `XData`/`XState` locales con
#       flujo de `XController` en un `.cpp`. No tiene linea base ni
#       admite excepciones.
#
#   linea_base_core — ~15 s. Mide con analizar_duplicados.py y compara
#       con planificacion/linea_base_core.json: ficheros que no alcanza
#       ningun ejecutable, pruebas que se prueban a si mismas, paquetes
#       cerrados sin consumidor, y el techo de 500 lineas por fichero.
#       La lista puede bajar; no puede subir.
#
# Y la prueba de tools/marcar_paquetes.py, que vive aqui porque el bloque
# que escribe en cada fichero tiene que respetar este mismo techo (lo lee
# de la linea base) y no puede pisar trabajo sin commitear.
#
# (Hasta el 24/09 este fichero no se incluia desde CMakeLists.txt; desde
# EST-56 si: include(cmake/LineaBaseCore.cmake).)
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if (BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME linea_base_core_configura
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/linea_base_core.py"
            --rapido --raiz "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(linea_base_core_configura PROPERTIES
        LABELS "static;gl-free" TIMEOUT 60)

    add_test(NAME linea_base_core
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/linea_base_core.py"
            --raiz "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(linea_base_core PROPERTIES
        LABELS "static;gl-free" TIMEOUT 300)

    add_test(NAME test_linea_base_core
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_linea_base_core.py")
    set_tests_properties(test_linea_base_core PROPERTIES
        LABELS "static;gl-free" TIMEOUT 60)

    # Siembra su propio arbol en un temporal: una herramienta que escribe en
    # el arbol no se prueba escribiendo en el arbol.
    add_test(NAME test_marcar_paquetes
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_marcar_paquetes.py")
    set_tests_properties(test_marcar_paquetes PROPERTIES
        LABELS "static;gl-free" TIMEOUT 60)
endif ()
