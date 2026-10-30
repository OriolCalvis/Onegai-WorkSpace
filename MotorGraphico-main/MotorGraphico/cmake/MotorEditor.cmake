# E-10: la UI solo pertenece a motor_editor; los modelos siguen sin GL/ImGui.
# EDITOR-UX-01: EditorAyuda (texto e icono por accion y por control) va con las
# acciones: sin ImGui, con prueba propia.
add_library(motor_editor_document src/Editor/EditorDocumentSession.cpp src/Editor/EditorActions.cpp src/Editor/EditorAyuda.cpp
    # H4-02 / E-11: los mapas de un proyecto y el modelo del panel Proyecto, sin ImGui.
    src/Editor/ProjectMaps.cpp src/Editor/ProjectMapsFiles.cpp src/Editor/ProjectMapsJson.cpp
    src/Editor/ProjectPanelState.cpp)
target_link_libraries(motor_editor_document PUBLIC motor_level motor_map motor_tmx_import)

if(TARGET motor_core AND TARGET glfw)
    set(IMGUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
    add_library(editor_imgui STATIC
        "${IMGUI_ROOT}/imgui.cpp" "${IMGUI_ROOT}/imgui_draw.cpp"
        "${IMGUI_ROOT}/imgui_tables.cpp" "${IMGUI_ROOT}/imgui_widgets.cpp"
        "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp")
    target_include_directories(editor_imgui PUBLIC "${IMGUI_ROOT}" "${IMGUI_ROOT}/backends")
    target_link_libraries(editor_imgui PRIVATE glfw ${CMAKE_DL_LIBS})
    # EDITOR-UX-01: iconos, tooltips y los paneles Herramientas/Inspector fuera
    # de EditorApplication.cpp (597 -> 500 lineas).
    add_executable(motor_editor editor/src/main.cpp editor/src/EditorApplication.cpp editor/src/LayerPanel.cpp
        editor/src/ProblemsPanel.cpp editor/src/EditorIconos.cpp editor/src/ToolsPanel.cpp editor/src/InspectorPanel.cpp
        editor/src/ProjectPanel.cpp  # H4-02: arbol de proyectos y mapas, nuevo/duplicar/renombrar/quitar
        src/RPG/AdventureValidation.cpp)
    target_include_directories(motor_editor PRIVATE editor/include)
    target_link_libraries(motor_editor PRIVATE editor_imgui motor_editor_problems motor_core motor_editor_document motor_project_catalog)
    add_dependencies(motor_editor copiar_assets)
endif()

if(BUILD_TESTING)
    add_executable(test_editor_document_session tests/Editor/EditorDocumentSessionTests.cpp)
    target_include_directories(test_editor_document_session PRIVATE tests)
    target_link_libraries(test_editor_document_session PRIVATE motor_editor_document)
    add_dependencies(test_editor_document_session copiar_assets)
    add_test(NAME test_editor_document_session COMMAND test_editor_document_session)
    set_tests_properties(test_editor_document_session PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_document_session)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_document_session)
    endif()
    add_executable(test_editor_actions tests/Editor/EditorActionsTests.cpp)
    target_include_directories(test_editor_actions PRIVATE tests)
    target_link_libraries(test_editor_actions PRIVATE motor_editor_document)
    add_dependencies(test_editor_actions copiar_assets)
    add_test(NAME test_editor_actions COMMAND test_editor_actions)
    set_tests_properties(test_editor_actions PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_actions)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_actions)
    endif()
    add_executable(test_editor_ayuda tests/Editor/EditorAyudaTests.cpp)
    target_include_directories(test_editor_ayuda PRIVATE tests)
    target_link_libraries(test_editor_ayuda PRIVATE motor_editor_document)
    add_dependencies(test_editor_ayuda copiar_assets)
    add_test(NAME test_editor_ayuda COMMAND test_editor_ayuda)
    set_tests_properties(test_editor_ayuda PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_ayuda)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_ayuda)
    endif()
    add_executable(test_editor_layers tests/Editor/EditorLayerTests.cpp)
    target_include_directories(test_editor_layers PRIVATE tests)
    target_link_libraries(test_editor_layers PRIVATE motor_editor_document)
    add_dependencies(test_editor_layers copiar_assets)
    add_test(NAME test_editor_layers COMMAND test_editor_layers)
    set_tests_properties(test_editor_layers PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_editor_layers)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_editor_layers)
    endif()
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(TARGET motor_editor AND Python3_Interpreter_FOUND)
        foreach(scale IN ITEMS 1 2)
            add_test(NAME test_motor_editor_smoke_${scale}
                COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/Editor/test_motor_editor_smoke.py"
                    --binary "$<TARGET_FILE:motor_editor>" --assets "${CMAKE_BINARY_DIR}/assets" --scale "${scale}")
            set_tests_properties(test_motor_editor_smoke_${scale} PROPERTIES LABELS "gl;editor;smoke" TIMEOUT 60)
        endforeach()
    endif()
endif()
