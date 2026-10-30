if (BUILD_TESTING)
    add_executable(test_g10_pause_and_hud tests/Game/G10PauseAndHudTests.cpp)
    target_include_directories(test_g10_pause_and_hud PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_g10_pause_and_hud PRIVATE motor_level motor_core motor_math)
    add_test(NAME test_g10_pause_and_hud COMMAND test_g10_pause_and_hud)
    set_tests_properties(test_g10_pause_and_hud PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_g10_pause_and_hud)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_g10_pause_and_hud)
    endif ()

    # H1-03 — pantalla de Opciones (GL-free, patron PauseMenu). Va en este
    # mismo .cmake y no en uno nuevo para no tocar el bloque de includes de
    # CMakeLists.txt, fichero de cruce con WIP vivo. Solo enlaza lo que
    # usa: OptionsState no toca HudWidgets/BitmapFont (motor_core), asi que
    # el test sobrevive a rojos de Application (mismo desacople que
    # test_skill_effect_real_assets en su dia).
    add_executable(test_options_state tests/Game/OptionsStateTests.cpp)
    target_include_directories(test_options_state PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_options_state PRIVATE motor_level motor_math)
    add_test(NAME test_options_state COMMAND test_options_state)
    set_tests_properties(test_options_state PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_options_state)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_options_state)
    endif ()
endif ()
