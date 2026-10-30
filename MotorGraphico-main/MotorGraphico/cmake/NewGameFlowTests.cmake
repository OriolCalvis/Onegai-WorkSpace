if (BUILD_TESTING)
    add_executable(test_new_game_flow tests/CharacterCreationFlowTests.cpp)
    target_include_directories(test_new_game_flow PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_new_game_flow PRIVATE motor_rpg motor_project)
    add_dependencies(test_new_game_flow copiar_assets)
    add_test(NAME test_new_game_flow COMMAND test_new_game_flow)
    set_tests_properties(test_new_game_flow PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_new_game_flow)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_new_game_flow)
    endif ()
endif ()
