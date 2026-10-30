# PROLOGO-02 — Las cinematicas (Game/Cinematica): el catalogo, cuando empieza
# una, los espectros y las apariciones, y el reproductor contra un escenario de
# mentira (tests/Game/CinematicaTests.cpp).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_cinematicas tests/Game/CinematicaTests.cpp)
    target_include_directories(test_cinematicas PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_cinematicas PRIVATE motor_level motor_json)
    add_test(NAME test_cinematicas COMMAND test_cinematicas)
    set_tests_properties(test_cinematicas PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_cinematicas)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_cinematicas)
    endif()
endif()
