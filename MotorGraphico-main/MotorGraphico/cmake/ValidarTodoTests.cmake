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
    # UNI-02-11 B1: ningun beat auto pegajoso (se reabriria en bucle y taparia a los posteriores).
    add_test(NAME test_validar_beats_auto
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_validar_beats_auto.py")
    set_tests_properties(test_validar_beats_auto PROPERTIES LABELS "unit;gl-free")
    add_test(NAME validar_beats_auto
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/validar_beats_auto.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(validar_beats_auto PROPERTIES LABELS "static;gl-free")
    # E-MATRIZ-02 A0: ningun beat con mas de kMaxOpciones (4) opciones; el cargador rechaza la aventura entera.
    add_test(NAME test_validar_opciones_beat
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_validar_opciones_beat.py")
    set_tests_properties(test_validar_opciones_beat PROPERTIES LABELS "unit;gl-free")
    # E-MATRIZ-01/02: el inventario de decisiones (dia 1 y dia de Gorran).
    add_test(NAME test_matriz_decisiones
        COMMAND ${MOTOR_PYTHON3} -I "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_matriz_decisiones.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(test_matriz_decisiones PROPERTIES LABELS "unit;gl-free")
    # E-MISIONES-01: el inventario de misiones del dia 1 (todos los beats en una mision; alcanzabilidad estatica).
    add_test(NAME test_matriz_misiones
        COMMAND ${MOTOR_PYTHON3} -I "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_matriz_misiones.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(test_matriz_misiones PROPERTIES LABELS "unit;gl-free")
    add_test(NAME validar_opciones_beat
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tools/validar_opciones_beat.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(validar_opciones_beat PROPERTIES LABELS "static;gl-free")
endif ()
