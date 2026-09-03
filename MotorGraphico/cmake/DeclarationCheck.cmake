# C-22 — Comprobador estático: cada método declarado en include/RPG/ debe
# tener definición (inline o Clase::metodo en un .cpp). Es análisis del
# árbol con python3, no compila nada; se registra como test GL-free para
# que la batería de CI lo ejecute. Este registro también funciona antes de
# integrar el scaffolding INF-01 (mismo criterio que ConditionEngineTests).
include(CTest)
find_program(MOTOR_PYTHON3 python3)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME rpg_declaration_check
        COMMAND ${MOTOR_PYTHON3}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/comprobar_declaraciones_rpg.py"
            "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(rpg_declaration_check PROPERTIES LABELS "static;gl-free")
endif()
