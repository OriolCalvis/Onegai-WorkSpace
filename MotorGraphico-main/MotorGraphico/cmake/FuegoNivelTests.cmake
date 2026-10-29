# C2-04c — Casillas de fuego y su semilla (GL-free).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_fuego_nivel tests/Game/FuegoNivelTests.cpp)
    target_include_directories(test_fuego_nivel PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_fuego_nivel PRIVATE motor_level motor_json)
    add_test(NAME test_fuego_nivel COMMAND test_fuego_nivel)
    set_tests_properties(test_fuego_nivel PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_fuego_nivel)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_fuego_nivel)
    endif()
endif()
