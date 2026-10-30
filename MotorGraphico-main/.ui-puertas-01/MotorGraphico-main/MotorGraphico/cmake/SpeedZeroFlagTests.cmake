# D-11 — Flag speedZero per velocity_0 (agarrado/anclado/paralizado).
# Complementa ConditionsRealAssetsTests (D-09): target nou separat, GL-free.
# NO enllaca motor_core per no col·lisionar amb l'editor actiu (E-14/E-16).

if (BUILD_TESTING)
    add_executable(test_speed_zero_flag tests/RPG/SpeedZeroFlagTests.cpp)
    target_include_directories(test_speed_zero_flag PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_speed_zero_flag PRIVATE
        motor_rpg
        motor_json
    )
    if (TARGET copiar_assets)
        add_dependencies(test_speed_zero_flag copiar_assets)
    endif ()
    add_test(NAME test_speed_zero_flag COMMAND test_speed_zero_flag)
    set_tests_properties(test_speed_zero_flag PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_speed_zero_flag)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_speed_zero_flag)
    endif ()
endif ()
