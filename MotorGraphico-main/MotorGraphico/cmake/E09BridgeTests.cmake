# E-09 — Puente EventEditorState → MapDocument.
#
# Prueba que save() convierte los eventos del editor (E-05) al formato
# del documento (E-01): eventos con posicion, paginas con trigger y
# condiciones, y comandos con tipo. La limitacion conocida (params map
# no representable en EventPage.commands vector<string>) se documenta
# en la sesion y en los comentarios del test.
include(CTest)
if(BUILD_TESTING AND TARGET motor_map_document AND TARGET motor_rpg)
    add_executable(test_e09_bridge
        tests/Editor/E09BridgeTests.cpp
        editor/src/event_editor.cpp)
    target_include_directories(test_e09_bridge PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/editor/include")
    target_link_libraries(test_e09_bridge PRIVATE motor_map_document motor_rpg motor_json)
    add_test(NAME test_e09_bridge COMMAND test_e09_bridge)
    set_tests_properties(test_e09_bridge PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_e09_bridge)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_e09_bridge)
    endif()
endif()
