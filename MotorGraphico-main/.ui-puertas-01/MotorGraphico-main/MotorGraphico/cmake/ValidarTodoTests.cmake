# INF-06 / D-10 — orquestador de validaciones de contenido.
#
# Ejecuta tools/validar_todo.py, que orquesta los 5 validadores
# (enlaces, conectividad, referencias catalogos, objectIds niveles
# y aventuras. Devuelve 0 si hay 0 errores (avisos permitidos;
# deuda de contenido conocida).
#
# NO necesita build del motor: pure Python sobre assets/ publicados.
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if (BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_validar_todo
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/validar_todo.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(test_validar_todo PROPERTIES
        LABELS "static;gl-free" TIMEOUT 300)
endif ()
