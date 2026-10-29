# opencode 19/09 — Red de regresion del check [5/5] Aventuras de
# validar_todo.py: los guiones deben referenciar enemigos/targets que existen
# en los catalogos (startBattle.monsterId, trigger.target, skillCheck.skillId).
# Analisis del arbol con python3 (mismo patron que DeclarationCheck.cmake);
# GL-free. Ruta de ASSETS relativa al propio script (bitacora 2.15), no a CWD.
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_aventuras_monsterid
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/validar_todo.py" --solo-aventuras)
    set_tests_properties(test_aventuras_monsterid PROPERTIES
        LABELS "content;gl-free"
        TIMEOUT 60)
endif()
