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

# U3-05.E08 -- Verificacion de bucle y clip no ciclico a varios dt (gl-free, lee el asset del idle).
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_clip_actor_fps tests/Game/ClipActorFpsTests.cpp)
    target_include_directories(test_clip_actor_fps PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_clip_actor_fps PRIVATE motor_level)
    add_test(NAME test_clip_actor_fps COMMAND test_clip_actor_fps)
    set_tests_properties(test_clip_actor_fps PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_clip_actor_fps)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_clip_actor_fps)
    endif()
endif()

# U3-06.E02 -- Lector de maquinas de estados de animacion (gl-free, Game::MaquinaAnimacion).
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/MaquinaAnimacion.cpp
        src/Game/MaquinaAnimacionValidacion.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_maquina_animacion tests/Game/MaquinaAnimacionTests.cpp)
    target_include_directories(test_maquina_animacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_maquina_animacion PRIVATE motor_level)
    add_test(NAME test_maquina_animacion COMMAND test_maquina_animacion)
    set_tests_properties(test_maquina_animacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_maquina_animacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_maquina_animacion)
    endif()
endif()

# U3-06.E03 -- Seleccion de transicion (gl-free, Game::eligeTransicion / clipDeEstado).
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/SeleccionAnimacion.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_seleccion_animacion tests/Game/SeleccionAnimacionTests.cpp)
    target_include_directories(test_seleccion_animacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_seleccion_animacion PRIVATE motor_level)
    add_test(NAME test_seleccion_animacion COMMAND test_seleccion_animacion)
    set_tests_properties(test_seleccion_animacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_seleccion_animacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_seleccion_animacion)
    endif()
endif()

# U3-06.E04 -- Aplicar cambio de estado sin reinicio indebido (gl-free, Game::pasoAnimacion).
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/PasoAnimacion.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_paso_animacion tests/Game/PasoAnimacionTests.cpp)
    target_include_directories(test_paso_animacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_paso_animacion PRIVATE motor_level)
    add_test(NAME test_paso_animacion COMMAND test_paso_animacion)
    set_tests_properties(test_paso_animacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_paso_animacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_paso_animacion)
    endif()
endif()

# U3-06.E05 -- Entradas de un PNJ y maquina pnj_basico equivalente al oraculo (gl-free,
# Game::entradasDePnj). Usa assets/animaciones/pnj_basico.estados.json (WORKING_DIRECTORY = fuentes).
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/RutinasPnjAnimacion.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_rutinas_pnj_animacion tests/Game/RutinasPnjAnimacionTests.cpp)
    target_include_directories(test_rutinas_pnj_animacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_rutinas_pnj_animacion PRIVATE motor_level)
    add_test(NAME test_rutinas_pnj_animacion COMMAND test_rutinas_pnj_animacion)
    set_tests_properties(test_rutinas_pnj_animacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_rutinas_pnj_animacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_rutinas_pnj_animacion)
    endif()
endif()

# U3-06.E06a -- Catalogo de animacion y controlador con destino por puerto (gl-free,
# Game::CatalogoAnimacion / Game::ControladorAnimacion). Usa assets/animaciones/*.json
# (WORKING_DIRECTORY = fuentes).
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/CatalogoAnimacion.cpp
        src/Game/ControladorAnimacion.cpp)
endif()
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_controlador_animacion tests/Game/ControladorAnimacionTests.cpp)
    target_include_directories(test_controlador_animacion PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_controlador_animacion PRIVATE motor_level)
    add_test(NAME test_controlador_animacion COMMAND test_controlador_animacion)
    set_tests_properties(test_controlador_animacion PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_controlador_animacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_controlador_animacion)
    endif()
endif()
