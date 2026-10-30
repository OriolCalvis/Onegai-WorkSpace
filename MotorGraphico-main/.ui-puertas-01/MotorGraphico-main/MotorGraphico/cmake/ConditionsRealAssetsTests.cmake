# D-09 — Conditions aplicades amb assets publicats.
# Complement a BattleStateTests (agent anterior): target nou separado
# sin tocar BattleState.cpp ni BattleStateTests.cpp. GL-free.

if (BUILD_TESTING)
    add_executable(test_conditions_real_assets tests/RPG/ConditionsRealAssetsTests.cpp)
    target_include_directories(test_conditions_real_assets PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_conditions_real_assets PRIVATE
        motor_level
        motor_rpg
        motor_json
        motor_core
    )
    if (TARGET copiar_assets)
        add_dependencies(test_conditions_real_assets copiar_assets)
    endif ()
    add_test(NAME test_conditions_real_assets COMMAND test_conditions_real_assets)
    set_tests_properties(test_conditions_real_assets PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_conditions_real_assets)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_conditions_real_assets)
    endif ()
endif ()
