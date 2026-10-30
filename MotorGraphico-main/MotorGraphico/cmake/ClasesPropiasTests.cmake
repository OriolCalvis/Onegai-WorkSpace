# CP-01 — Las clases propias (planificacion/CANON_CLASES_PROPIAS.md):
# las 20 clases que pidio Oriol (bardos, clerigos, paladines, ingeniero,
# mercader, alquimista, cartomante, druida, domador, chamanes, tempestario,
# necromago y piratas), con sus 10 cartas jugables, su pasiva, su set de tier
# 2 y el gancho azar_una. La mesa y el dado fijo son los de H2-06
# (tests/Game/CartasPrueba.h).
#
# GL-free. Lee los catalogos reales (assets/catalogs).
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_clases_propias tests/Game/ClasesPropiasTests.cpp)
    target_include_directories(test_clases_propias PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_clases_propias PRIVATE motor_level motor_map motor_rpg motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_clases_propias copiar_assets)
    endif()
    add_test(NAME test_clases_propias COMMAND test_clases_propias)
    set_tests_properties(test_clases_propias PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_clases_propias)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_clases_propias)
    endif()
endif()
