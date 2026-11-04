# H2-06 (Fase H) — Las cartas de clase en combate: Game::CartasEnCombate, la
# mesa que BattleState consulta cuando el heroe tiene ficha. Bardo de las
# Cuatro Estaciones, El Apostata, Senor de las Armas y Furia de los Creadores,
# cada carta contra lo que dice su texto.
#
# GL-free. Lee los catalogos reales (assets/catalogs): lo que se prueba es que
# la carta publicada, con sus ganchos, hace lo que dice. El dado es fijo (todas
# las caras iguales) para que cada resultado se pueda contar a mano.
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_cartas_combate tests/Game/CartasEnCombateTests.cpp
        tests/Game/CartasClasesTests.cpp
        tests/Game/AcompanantesTests.cpp  # ACOMPANANTES-01: Venides y Xila en la mesa
        tests/Game/CartasEnemigasTests.cpp)  # AV18-06: el repertorio nativo de los enemigos
    target_include_directories(test_cartas_combate PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_cartas_combate PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_cartas_combate copiar_assets)
    endif()
    add_test(NAME test_cartas_combate COMMAND test_cartas_combate)
    set_tests_properties(test_cartas_combate PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_cartas_combate)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_cartas_combate)
    endif()
endif()
