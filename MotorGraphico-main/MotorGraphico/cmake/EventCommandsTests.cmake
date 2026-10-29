# E-04 — El juego de comandas: catalogo, validacion, evaluacion de
# condiciones y el anfitrion de estado narrativo. Incluye la prueba de
# integracion que une E-01 (documento), E-02 (estado), E-03 (planificador)
# y E-04 (significado): el cofre que se abre una sola vez.
#
# Fragmento aparte, como el resto de tests nuevos: evita que varios
# agentes se pisen editando el mismo bloque del CMakeLists.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document AND TARGET motor_rpg)
    add_executable(test_event_commands
        tests/RPG/EventCommandsTests.cpp)
    target_include_directories(test_event_commands PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_event_commands PRIVATE motor_map_document motor_rpg motor_json)
    add_test(NAME test_event_commands COMMAND test_event_commands)
    set_tests_properties(test_event_commands PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_event_commands)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_event_commands)
    endif()
endif()
