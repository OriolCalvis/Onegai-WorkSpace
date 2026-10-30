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

# U3-05.E07a -- Adaptador ClipActor (gl-free). La fuente y la prueba se anaden aparte
# de las anteriores para no tocar sus bloques.
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/ClipActor.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_clip_actor tests/Game/ClipActorTests.cpp)
    target_include_directories(test_clip_actor PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_clip_actor PRIVATE motor_level)
    add_test(NAME test_clip_actor COMMAND test_clip_actor)
    set_tests_properties(test_clip_actor PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_clip_actor)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_clip_actor)
    endif()
endif()

# U3-05.E07b -- El idle del cuerpo base como clip (primera integracion en `juego`).
# IdleCuerpoBase (gl-free) en motor_level; EntidadClip (AnimatedEntity + ClipActor) en motor_core.
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/IdleCuerpoBase.cpp)
endif()
if(TARGET motor_core)
    target_sources(motor_core PRIVATE src/Engine/EntidadClip.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_core AND TARGET motor_level)
    add_motor_unit_test(test_idle_cuerpo_base tests/Engine/IdleCuerpoBaseTests.cpp)
    target_link_libraries(test_idle_cuerpo_base PRIVATE motor_core motor_level motor_math)
    set_tests_properties(test_idle_cuerpo_base PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    # Enlaza motor_core (EntidadClip) igual que test_animated_entity: nunca abre ventana ni GL.
    # Este fichero se incluye tras crear el agregado, de ahi la dependencia explicita.
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_idle_cuerpo_base)
    endif()
endif()
