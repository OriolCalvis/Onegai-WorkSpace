# MORAL-01 — La moral de varios factores (Oriol, 08/10/2026): las cuentas de
# una noche, las reglas y los ficheros del juego, las cinco rutas de
# Boundington, el guardado, la aventura (acto, herida, requires.moral, exitos)
# y la sesion. Lee assets/rules y tools/moral de la fuente (MOTOR_FUENTE).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_rpg)
    add_executable(test_moral tests/Game/MoralTests.cpp)
    target_include_directories(test_moral PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/RPG")
    target_compile_definitions(test_moral PRIVATE MOTOR_FUENTE="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(test_moral PRIVATE motor_level motor_map_document motor_map motor_rpg motor_json)
    add_test(NAME test_moral COMMAND test_moral)
    set_tests_properties(test_moral PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_moral)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_moral)
    endif()
endif()
