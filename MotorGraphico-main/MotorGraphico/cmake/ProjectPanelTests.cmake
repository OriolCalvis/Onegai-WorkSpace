# H4-02 / E-11 — test_project_panel: el panel Proyecto de motor_editor sin
# ventana. Crear proyecto, dos mapas, elegir la entrada, guardar y reabrir;
# duplicar sin colisiones; cancelar o fallar sin dejar ficheros a medias.
#
# Todo contra un assetsRoot temporal con espacios en la ruta: del repo solo se
# copian un TMX y su textura, por eso depende de copiar_assets.
if(BUILD_TESTING AND TARGET motor_editor_document)
    add_executable(test_project_panel tests/Editor/ProjectPanelTests.cpp)
    target_include_directories(test_project_panel PRIVATE tests)
    target_link_libraries(test_project_panel PRIVATE motor_editor_document)
    add_dependencies(test_project_panel copiar_assets)
    add_test(NAME test_project_panel COMMAND test_project_panel)
    set_tests_properties(test_project_panel PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_project_panel)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_project_panel)
    endif()
endif()
