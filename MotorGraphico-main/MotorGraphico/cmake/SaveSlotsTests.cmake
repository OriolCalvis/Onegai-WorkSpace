# G-11 — Selector de guardados y recuperacion de derrota, mitad sin
# pantalla.
#
# Hoy hay UN fichero, savegame.json, y dos teclas: no hay ranuras, no hay
# Continuar, y desde Game Over no se va a ninguna parte. Peor, todos los
# proyectos escriben en el mismo sitio, asi que abrir un proyecto y
# guardar pisa la partida del otro sin avisar.
#
# Aqui estan las ranuras separadas por proyecto, el resumen que se enseña
# sin cargar la partida, y DEC-09 —la decision que Oriol SI contesto—
# hecha comprobable: no se guarda en medio de una accion, y al volver no
# se repite ninguna recompensa.
#
# Las pantallas (SaveLoadScene, GameOverScene) son GL y van con G-10.
# SaveSlotState.cpp se compila AQUI y todavia no dentro de ninguna
# libreria, por la misma razon que ProblemsState y Locale: no tocar el
# bloque compartido del CMakeLists.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_save_slots
        tests/Game/SaveSlotsTests.cpp
        src/Game/SaveSlotState.cpp)
    target_include_directories(test_save_slots PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_save_slots PRIVATE
        motor_level motor_map_document motor_rpg motor_json motor_map)
    add_test(NAME test_save_slots COMMAND test_save_slots)
    set_tests_properties(test_save_slots PROPERTIES
        LABELS "unit;gl-free"
        # Una regresion al abrir FIFO no debe dejar la suite bloqueada.
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_save_slots)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_save_slots)
    endif()
endif()
