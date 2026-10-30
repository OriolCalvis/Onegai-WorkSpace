# OPCIONES-01 / VARIABLES-01 — Opciones en el dialogo y variables en las
# aventuras (fase 1 de «Los Perdidos de Boundington — revisión para jugarla
# entera»): la lectura, el motor, la escritura de vuelta, la validacion, la
# sesion y la hoja del dialogo. La validacion no esta en ninguna libreria: se
# compila aqui, como en test_adventure_validation.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document AND TARGET motor_rpg AND TARGET motor_pergamino)
    add_executable(test_opciones_dialogo
        tests/RPG/OpcionesDialogoTests.cpp
        src/RPG/AdventureValidation.cpp
        src/RPG/AdventureBeatValidation.cpp)
    target_include_directories(test_opciones_dialogo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_opciones_dialogo PRIVATE motor_pergamino motor_map_document motor_level motor_map
                                                        motor_rpg motor_json)
    add_test(NAME test_opciones_dialogo COMMAND test_opciones_dialogo)
    set_tests_properties(test_opciones_dialogo PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_opciones_dialogo)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_opciones_dialogo)
    endif()
endif()
