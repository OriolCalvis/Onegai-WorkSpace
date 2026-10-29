# APARIENCIA-01 — Personajes por capas en el motor y el paso «Apariencia» de
# Nueva partida, sin ventana (motor_level + motor_imagen, GL-free).
#
# test_personajes_capas: el manifiesto se lee entero; la mezcla es la de
# Pillow; los 62 personajes horneados (actores_capas, razas_andando,
# personajes_historia) salen byte a byte montados en C++ desde su
# composicion; el orden de capas es el del probador; la cache respeta su
# presupuesto; el reparto de la hoja (ciclos que empiezan quietos).
#
# test_editor_apariencia: filas segun el pueblo, valores con la vuelta, la
# barba que sigue al pelo, AL AZAR determinista y siempre montable en las 43
# razas, el equipo inicial puesto (Vestuario) y el flujo de 15 pasos con
# finalizar() entregando la ficha equipada.
#
# Las dos leen assets/ desde build/ (copiar_assets).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_personajes_capas tests/Game/PersonajesCapasTests.cpp)
    target_include_directories(test_personajes_capas PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_personajes_capas PRIVATE motor_level motor_imagen motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_personajes_capas copiar_assets)
    endif()
    add_test(NAME test_personajes_capas COMMAND test_personajes_capas)
    set_tests_properties(test_personajes_capas PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_personajes_capas)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_personajes_capas)
    endif()

    # ESTETICA-01: el brillo del equipo por lo fuerte que es (escalera,
    # catalogo, mapa de capas, halo y destellos deterministas).
    add_executable(test_brillo_equipo tests/Game/BrilloEquipoTests.cpp)
    target_include_directories(test_brillo_equipo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_brillo_equipo PRIVATE motor_level motor_imagen motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_brillo_equipo copiar_assets)
    endif()
    add_test(NAME test_brillo_equipo COMMAND test_brillo_equipo)
    set_tests_properties(test_brillo_equipo PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_brillo_equipo)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_brillo_equipo)
    endif()

    add_executable(test_editor_apariencia tests/Game/EditorAparienciaTests.cpp tests/Game/RasgosAparienciaTests.cpp)
    target_include_directories(test_editor_apariencia PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_editor_apariencia PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_editor_apariencia copiar_assets)
    endif()
    add_test(NAME test_editor_apariencia COMMAND test_editor_apariencia)
    set_tests_properties(test_editor_apariencia PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_apariencia)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_apariencia)
    endif()

    # CAR-1: pelo, barba y ojos solo donde la raza los admite (editor, «Al azar» y partida vieja).
    add_executable(test_editor_apariencia_razas tests/Game/EditorAparienciaRazasTests.cpp)
    target_include_directories(test_editor_apariencia_razas PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_editor_apariencia_razas PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_editor_apariencia_razas copiar_assets)
    endif()
    add_test(NAME test_editor_apariencia_razas COMMAND test_editor_apariencia_razas)
    set_tests_properties(test_editor_apariencia_razas PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_apariencia_razas)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_apariencia_razas)
    endif()
endif()
