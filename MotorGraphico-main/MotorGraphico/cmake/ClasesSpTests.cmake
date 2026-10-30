# SP-02 — Las SP de NosTale como clases nuevas (tools/importar_clases_sp.py):
# cada clase carga con sus 10 cartas y su pasiva, se elige al crear personaje,
# y sus cartas hacen en combate lo que dice su texto, con los ganchos nuevos
# siempre:, propio: y limpia. La mesa y el dado fijo son los de H2-06
# (tests/Game/CartasPrueba.h).
#
# GL-free. Lee los catalogos reales (assets/catalogs).
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_clases_sp tests/Game/ClasesSpTests.cpp)
    target_include_directories(test_clases_sp PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_clases_sp PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_clases_sp copiar_assets)
    endif()
    add_test(NAME test_clases_sp COMMAND test_clases_sp)
    set_tests_properties(test_clases_sp PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_clases_sp)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_clases_sp)
    endif()
endif()
