# FICHA-01 — La ficha del jugador como datos (Game::FichaVista) y el
# contrato de iconos (Hud::Iconos), ambos GL-free y dentro de motor_level.
#
# test_ficha_vista: los cuatro stats se desglosan (base/raza/clase/equipo/
# dotes/condiciones) y el total es el del motor; los nombres salen de los
# catalogos reales y lo que no existe se legibiliza sin romper; las 16
# condiciones de conditions.json tienen icono propio y las desconocidas caen
# en el generico. Lee assets/ desde build/ (copiar_assets).
#
# test_diario_misiones (MISIONES-01): el diario es una vista de las flags —
# la activa primero, objetivos resueltos contra el NarrativeState, sin
# duplicar la activa ni caerse con nulos.
#
# test_codice (CODICE-01): lo conocido sale de beats disparados, nivel
# actual, ficha y flags codice_<id>; las deidades siempre, la propia primero.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_ficha_vista tests/Game/FichaVistaTests.cpp)
    target_include_directories(test_ficha_vista PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_ficha_vista PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_ficha_vista copiar_assets)
    endif()
    add_test(NAME test_ficha_vista COMMAND test_ficha_vista)
    set_tests_properties(test_ficha_vista PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_ficha_vista)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_ficha_vista)
    endif()

    # test_diario_misiones (MISIONES-01) vive en cmake/DiarioMisionesTests.cmake:
    # registrar aqui su bloque hacia depender el registro de una prueba de tres
    # fuentes que las otras tres sesiones (FICHA-01, CODICE-01, CLASES-03) aun
    # no tienen versionadas. Ver el comentario de cabecera de ese fichero.

    add_executable(test_codice tests/Game/CodiceTests.cpp)
    target_include_directories(test_codice PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_codice PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_codice copiar_assets)
    endif()
    add_test(NAME test_codice COMMAND test_codice)
    set_tests_properties(test_codice PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_codice)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_codice)
    endif()

    # CLASES-03: seis clases de la 3.ª iteracion con sus cartas, y companeros
    # reclutables por flag (Game::Companeros).
    add_executable(test_clases_companeros tests/Game/ClasesCompanerosTests.cpp)
    target_include_directories(test_clases_companeros PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_clases_companeros PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_clases_companeros copiar_assets)
    endif()
    add_test(NAME test_clases_companeros COMMAND test_clases_companeros)
    set_tests_properties(test_clases_companeros PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_clases_companeros)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_clases_companeros)
    endif()
endif()
