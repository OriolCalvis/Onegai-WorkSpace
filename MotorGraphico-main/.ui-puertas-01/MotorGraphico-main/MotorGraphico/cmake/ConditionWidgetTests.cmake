# H2-03 (Fase H) — condiciones en el HUD, con rondas restantes.
# Vive en HUD/ y no toca Application, GameSession, CharacterSheet,
# ConditionEngine ni ningun otro fichero de cruce (ver
# planificacion/MAPA_DEL_CODIGO.md §1).
#
# GL-free: la pieza no conoce GL, y la prueba tampoco. Sin assets: ficha y
# catalogo escritos a mano mas integracion con ConditionEngine de verdad,
# asi que no necesita copiar_assets.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_condition_widget
        tests/Game/ConditionWidgetTests.cpp)
    target_include_directories(test_condition_widget PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_condition_widget PRIVATE
        motor_level motor_rpg motor_json)
    add_test(NAME test_condition_widget COMMAND test_condition_widget)
    set_tests_properties(test_condition_widget PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_condition_widget)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_condition_widget)
    endif()
endif()
