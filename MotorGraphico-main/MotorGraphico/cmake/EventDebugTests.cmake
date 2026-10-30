# E-08 — Depuracion paso a paso sobre el interprete de E-03: pausa,
# stepOnce, puntos de parada y traza en orden temporal. GL-free.
#
# Fichero de test aparte de EventInterpreterTests.cpp a proposito: ese lo
# esta ampliando otra sesion, y anadirle casos mientras tanto es como se
# pierde trabajo.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document)
    add_executable(test_event_debug
        tests/RPG/EventDebugTests.cpp
        src/RPG/EventInterpreter.cpp)
    target_include_directories(test_event_debug PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_event_debug PRIVATE motor_map_document)
    add_test(NAME test_event_debug COMMAND test_event_debug)
    set_tests_properties(test_event_debug PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_event_debug)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_event_debug)
    endif()
endif()
