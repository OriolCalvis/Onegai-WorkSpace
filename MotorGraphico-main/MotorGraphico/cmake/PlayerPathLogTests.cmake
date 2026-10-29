# EST-51 — Rastro de logs del camino del jugador: titulo, partida, nivel,
# combate, tienda, guardar y morir. Las siete etapas del camino dejan entrada
# con su categoria, y el rastro se lee: una linea por evento y secuencia sin
# huecos.
#
# Va en su propio .cmake y no dentro de CMakeLists.txt por la ley 3 del repo
# (AGENTS.md): un include(cmake/X.cmake) va SIEMPRE en el mismo commit que su
# .cmake. Sin fuentes sueltas: motor_log y motor_level son librerias, que es lo
# que la ley 4c pide (no compilar un src/*.cpp dentro del objetivo).
#
# GL-free: solo motor_log (Core/Errors/Result.h y la estandar), motor_level
# (SaveGame, que es donde vive el guardado que el rastro comprueba) y motor_json
# para el guardado. No arrastra Render ni GL, asi que no necesita copiar_assets.

include(CTest)
if(BUILD_TESTING)
    add_executable(test_player_path_log tests/Game/PlayerPathLogTests.cpp)
    target_include_directories(test_player_path_log PRIVATE tests)
    target_link_libraries(test_player_path_log PRIVATE motor_log motor_level motor_json)
    add_test(NAME test_player_path_log COMMAND test_player_path_log)
    set_tests_properties(test_player_path_log PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_player_path_log)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_player_path_log)
    endif()
endif()
