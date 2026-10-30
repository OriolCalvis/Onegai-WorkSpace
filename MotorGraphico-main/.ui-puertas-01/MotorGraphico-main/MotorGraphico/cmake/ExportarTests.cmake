# G-06 — Exportador de proyectos (tools/exportar.py).
#
# Es analisis y copia de ficheros con python3, no compila nada: se
# registra como prueba GL-free para que la bateria de CI la ejecute con
# el resto. Siembra su propio repositorio de bolsillo en un temporal, asi
# que no depende de assets/ ni de que otro agente este tocando el arbol.
#
# ANTICOLISION (2026-09-20). Este fichero NO esta incluido todavia desde
# CMakeLists.txt a proposito: cuando se escribio, CMakeLists.txt tenia
# cambios sin commitear de otra sesion, y meter un include() en el arbol
# sucio de otro es la forma conocida de que HEAD deje de configurar (un
# include() commiteado sin su .cmake). La linea que falta, dentro del
# bloque if(BUILD_TESTING) y junto a los demas include(cmake/...):
#
#     include(cmake/ExportarTests.cmake)
#
# Mientras tanto la prueba se ejecuta sola:
#     python3 tools/test_exportar.py
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if (BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_exportar
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_exportar.py")
    set_tests_properties(test_exportar PROPERTIES
        LABELS "static;gl-free" TIMEOUT 60)
endif ()
