if (BUILD_TESTING)
    add_executable(test_character_creation_flow tests/RPG/CharacterCreationFlowTests.cpp)
    target_include_directories(test_character_creation_flow PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_character_creation_flow PRIVATE motor_rpg)
    add_dependencies(test_character_creation_flow copiar_assets)
    add_test(NAME test_character_creation_flow COMMAND test_character_creation_flow)
    set_tests_properties(test_character_creation_flow PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_character_creation_flow)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_character_creation_flow)
    endif ()
endif ()
