# G-02 — Las herramientas de edicion que faltan: cuentagotas, rectangulo y
# linea, cada una como un solo paso de historial (EditorState::paintRect,
# paintLine, pickTileAt). Mitad sin interfaz; la herramienta activa, el
# arrastre y los atajos son de la capa ImGui (E-10).
#
# Fragmento aparte, como el resto de tests nuevos: no toca el bloque
# compartido de test_editor_state. GL-free.
if (BUILD_TESTING)
    add_executable(test_editor_tools tests/Editor/EditorToolsTests.cpp)
    target_include_directories(test_editor_tools PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_editor_tools PRIVATE motor_level)
    add_test(NAME test_editor_tools COMMAND test_editor_tools)
    set_tests_properties(test_editor_tools PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_tools)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_tools)
    endif ()
endif ()
