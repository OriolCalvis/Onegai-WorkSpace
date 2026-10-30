# U3-05.E02 -- Clips de animacion con eventos de fotograma (Game::ClipAnimacion).
# Pieza GL-free en motor_level. Las fuentes se anaden siempre (tambien con
# BUILD_TESTING=OFF); solo la prueba va bajo BUILD_TESTING. Las E04+ anaden
# aqui sus fuentes y pruebas.
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/ClipAnimacion.cpp
        src/Game/CursorClip.cpp
        src/Game/EventosClip.cpp)
endif()
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_clips_animacion tests/Game/ClipsAnimacionTests.cpp)
    target_include_directories(test_clips_animacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_clips_animacion PRIVATE motor_level)
    add_test(NAME test_clips_animacion COMMAND test_clips_animacion)
    set_tests_properties(test_clips_animacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_clips_animacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_clips_animacion)
    endif()

    add_executable(test_cursor_clip tests/Game/CursorClipTests.cpp)
    target_include_directories(test_cursor_clip PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_cursor_clip PRIVATE motor_level)
    add_test(NAME test_cursor_clip COMMAND test_cursor_clip)
    set_tests_properties(test_cursor_clip PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_cursor_clip)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_cursor_clip)
    endif()

    add_executable(test_eventos_clip tests/Game/EventosClipTests.cpp)
    target_include_directories(test_eventos_clip PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_eventos_clip PRIVATE motor_level)
    add_test(NAME test_eventos_clip COMMAND test_eventos_clip)
    set_tests_properties(test_eventos_clip PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_eventos_clip)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_eventos_clip)
    endif()
endif()
