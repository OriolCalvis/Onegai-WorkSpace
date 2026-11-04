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

# E-DISPARADORES-1 E3 — validar_cinematicas.py con un nivel de mentira (foco y destino de «lleva», una sola accion, zona,
# radio, ambito, tapaVista y el aviso de vision).
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_validar_cinematicas COMMAND "${MOTOR_PYTHON3}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_validar_cinematicas.py")
    set_tests_properties(test_validar_cinematicas PROPERTIES LABELS "unit;gl-free" TIMEOUT 60)
endif()
endif()
