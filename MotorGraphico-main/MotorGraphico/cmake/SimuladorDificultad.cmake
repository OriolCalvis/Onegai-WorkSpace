# ACOMPANANTES-01 (06/10/2026) — El simulador del estudio de dificultad.
#
# Oriol: «un combate 1 contra 3 [...] mueres un 40 % de las veces; 2 contra 3,
# pocas veces; 3 contra 3, solo si son todos mas fuertes que tu», con el
# Apostata, el Señor de las Armas, la Furia de los Creadores y el Bardo, y un
# ranking por niveles de fuerza con el equipo al maximo de su tier.
#
# Combates con el motor de verdad (BattleState, la mesa de cartas y los
# acompañantes) y heroes creados como en el juego. No es una prueba: es la
# herramienta para rehacer las cuentas cuando cambien las cartas, el equipo o
# los enemigos (examples/simulador_dificultad.cpp dice los modos). La prueba de
# humo solo comprueba que arranca y crea las fichas.
#
#   cd build && ./simulador_dificultad niveles 25
if(TARGET motor_level)
    add_executable(simulador_dificultad examples/simulador_dificultad.cpp)
    target_link_libraries(simulador_dificultad PRIVATE motor_level motor_map motor_rpg motor_json)
    set_target_properties(simulador_dificultad PROPERTIES CXX_STANDARD 17)
    if(TARGET copiar_assets)
        add_dependencies(simulador_dificultad copiar_assets)
    endif()
    if(BUILD_TESTING)
        add_test(NAME simulador_dificultad_humo COMMAND simulador_dificultad fichas)
        set_tests_properties(simulador_dificultad_humo PROPERTIES
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            LABELS "smoke;gl-free")
    endif()
endif()
