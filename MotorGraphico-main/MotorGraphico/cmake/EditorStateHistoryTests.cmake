# G-03 — Historial per comandes (complement sobre mapa gran 200x200, 4 capes,
# 1000 operacions barallades). Complement a tests/Editor/EditorStateTests.cpp
# (agent anterior). Target nou separat; NO toca EditorState.h/.cpp. GL-free.

if (BUILD_TESTING)
    add_executable(test_editor_state_history
        tests/Editor/EditorStateHistoryTests.cpp)
    target_include_directories(test_editor_state_history PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_editor_state_history PRIVATE motor_level)
    add_test(NAME test_editor_state_history
        COMMAND test_editor_state_history)
    set_tests_properties(test_editor_state_history PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY
        MOTOR_GL_FREE_TARGETS test_editor_state_history)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_state_history)
    endif ()
endif ()
