# E-03 — Planificador, esperes i processos parallels.
#
# Target de tests GL-free: el interprete no conoce ni sesion ni render.
# Los comandos son opacos para el; solo necesita un EventHost que decida
# que hace cada uno, y los tests usan un doble sin ningun C++ de negocio.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_event_interpreter
        tests/RPG/EventInterpreterTests.cpp
    )
    target_include_directories(test_event_interpreter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_event_interpreter
        PRIVATE
            motor_level
            motor_map_document
            motor_rpg
            motor_json
    )
    add_test(
        NAME test_event_interpreter
        COMMAND test_event_interpreter
    )
    set_tests_properties(test_event_interpreter PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_event_interpreter)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_event_interpreter)
    endif()
endif()
