# G4-01 — Puertas cerradas por flags (cerradaSi / beatCerrada). Fragmento
# aparte, como el resto de tests nuevos del proyecto.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_puerta_cerrada tests/Game/PuertaCerradaTests.cpp)
    target_include_directories(test_puerta_cerrada PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_puerta_cerrada PRIVATE motor_level)
    add_test(NAME test_PuertaCerrada COMMAND test_puerta_cerrada)
    set_tests_properties(test_PuertaCerrada PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_puerta_cerrada)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_puerta_cerrada)
    endif()
endif()
