# FASE1-BND-01 — Lo que faltaba de la fase 1 de «Los Perdidos de Boundington —
# revisión para jugarla entera»: las franjas del dia, la tabla aleatoria, los
# descansos, huir a 0 PV y pelear sin matar. La lectura, el motor, la escritura
# de vuelta, la validacion, la sesion y el rotulo del HUD. La validacion no esta
# en ninguna libreria: se compila aqui, como en test_opciones_dialogo.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document AND TARGET motor_rpg AND TARGET motor_pergamino)
    add_executable(test_fase_uno_boundington
        tests/RPG/FaseUnoBoundingtonTests.cpp
        src/RPG/AdventureValidation.cpp
        src/RPG/AdventureBeatValidation.cpp)
    target_include_directories(test_fase_uno_boundington PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_fase_uno_boundington PRIVATE motor_pergamino motor_map_document motor_level
                                                            motor_map motor_rpg motor_json)
    add_test(NAME test_fase_uno_boundington COMMAND test_fase_uno_boundington)
    set_tests_properties(test_fase_uno_boundington PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_fase_uno_boundington)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_fase_uno_boundington)
    endif()
endif()
