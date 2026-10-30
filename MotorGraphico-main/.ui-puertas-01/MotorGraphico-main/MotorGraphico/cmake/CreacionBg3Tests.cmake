# UI-BG3-01 — Las pantallas en cuadricula (Oriol, 05/10), sin ventana.
#
# test_kit_basico: el kit del cofre de El Vacio sale de una regla con
# parametros (assets/rules/kit_basico.json) y las 63 clases reciben uno que
# se puede poner sin requisitos y sube su atributo principal y el secundario.
# Y el cofre (Game/CofreKit): se abre al lado, deja el kit en la mochila,
# enciende sus flags y el prologo nunca se queda esperando.
#
# test_cuaderno_creacion: la creacion en siete hojas (Game/CuadernoCreacion):
# rejillas con raton y teclado, busqueda, filtro de papel, la historia en una
# matriz, el nombre escrito y la ficha final sin equipo.
#
# test_barra_combate: la barra de cartas del combate (Game/BarraCombate)
# sobre la mesa real del Bardo contra un lobo: las cartas agrupadas, la
# estacion en su emergente, 1-0 que senalan y juegan, F Pasar, la espera del
# rival y el aviso de la reaccion que salta.
#
# Leen assets/ desde build/ (copiar_assets).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_kit_basico tests/Game/KitBasicoTests.cpp tests/Game/CofreKitTests.cpp)
    target_include_directories(test_kit_basico PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_kit_basico PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_kit_basico copiar_assets)
    endif()
    add_test(NAME test_kit_basico COMMAND test_kit_basico)
    set_tests_properties(test_kit_basico PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_kit_basico)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_kit_basico)
    endif()

    # H2-03: los estados de cada combatiente (EstadosCombateTests), y H2-02: a
    # quien apunta la carta senalada (ObjetivosBarraTests), sobre la misma mesa.
    add_executable(test_barra_combate tests/Game/BarraCombateTests.cpp tests/Game/EstadosCombateTests.cpp
                                      tests/Game/ObjetivosBarraTests.cpp)
    target_include_directories(test_barra_combate PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_barra_combate PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_barra_combate copiar_assets)
    endif()
    add_test(NAME test_barra_combate COMMAND test_barra_combate)
    set_tests_properties(test_barra_combate PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_barra_combate)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_barra_combate)
    endif()

    add_executable(test_cuaderno_creacion tests/Game/CuadernoCreacionTests.cpp)
    target_include_directories(test_cuaderno_creacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_cuaderno_creacion PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_cuaderno_creacion copiar_assets)
    endif()
    add_test(NAME test_cuaderno_creacion COMMAND test_cuaderno_creacion)
    set_tests_properties(test_cuaderno_creacion PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_cuaderno_creacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_cuaderno_creacion)
    endif()
endif()
