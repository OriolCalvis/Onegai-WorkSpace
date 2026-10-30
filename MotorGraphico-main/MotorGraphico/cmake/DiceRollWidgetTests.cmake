# H2-01 (Fase H) — el dado se ve: widget de tirada y grados en el HUD.
# Vive en HUD/ y no toca Application, GameSession ni ningun otro fichero de
# cruce (ver planificacion/MAPA_DEL_CODIGO.md §1).
#
# GL-free: la pieza no conoce GL, y la prueba tampoco. Sin assets: casos con
# PoolResult escrito a mano mas un caso de integracion con semilla fija
# (Xoroshiro128p), asi que no necesita copiar_assets.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_dice_roll_widget
        tests/Game/DiceRollWidgetTests.cpp)
    target_include_directories(test_dice_roll_widget PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_dice_roll_widget PRIVATE
        motor_level motor_rpg motor_json)
    add_test(NAME test_dice_roll_widget COMMAND test_dice_roll_widget)
    set_tests_properties(test_dice_roll_widget PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_dice_roll_widget)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_dice_roll_widget)
    endif()
endif()
