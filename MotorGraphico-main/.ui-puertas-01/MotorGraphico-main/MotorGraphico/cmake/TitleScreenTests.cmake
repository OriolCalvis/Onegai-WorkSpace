# G-08 (rodaja inicial) — La pantalla de título: lista de proyectos,
# navegación y acciones. TitleScreen es GL-free y solo depende del struct
# Project, así que el test compila la fuente directamente (mismo patrón
# que test_texture con Texture.cpp) sin añadir librerías al registro.
include(CTest)
if(BUILD_TESTING)
    add_executable(test_title_screen
        tests/Game/TitleScreenTests.cpp
        src/Game/TitleScreen.cpp
    )
    target_include_directories(test_title_screen PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_test(NAME test_title_screen COMMAND test_title_screen)
    set_tests_properties(test_title_screen PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free"
    )
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_title_screen)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_title_screen)
    endif()
endif()
