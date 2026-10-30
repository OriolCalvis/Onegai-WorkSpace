if (BUILD_TESTING)
    # H1-04 (Gantt Fase H) — el tramo GL-free de «Nueva partida»:
    # Game::NewGameFlow (envoltorio de CharacterCreationScreen + finalizar
    # que recalcula la ficha). Enlaza motor_level, que ya compila el flow
    # y trae motor_rpg (CharacterCreation/Screen/Sheet/TierRules).
    # Ojo: cmake/NewGameFlowTests.cmake YA existe (24/09) y cablea el test
    # trivial viejo de CharacterCreationFlowTests.cpp; por eso este lleva
    # el prefijo de la tarea.
    add_executable(test_h104_new_game_flow tests/Game/NewGameFlowTests.cpp)
    target_include_directories(test_h104_new_game_flow PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_h104_new_game_flow PRIVATE motor_level)
    add_dependencies(test_h104_new_game_flow copiar_assets)
    add_test(NAME test_h104_new_game_flow COMMAND test_h104_new_game_flow)
    set_tests_properties(test_h104_new_game_flow PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_h104_new_game_flow)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_h104_new_game_flow)
    endif ()
endif ()
