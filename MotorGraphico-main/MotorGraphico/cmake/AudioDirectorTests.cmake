# H6-01 (Fase H) — AudioDirector, la política de sonido: qué suena en cada
# momento del juego. Vive en Game/AudioDirector y consume RPG/AudioEngine (GDB-11)
# a través de su interfaz abstracta, así que la prueba NO abre dispositivo de
# audio ni necesita que exista: el motor es un doble que graba las llamadas.
# Por eso es GL-free y corre en CI sin tarjeta de sonido.
#
# No necesita copiar_assets: con cero ficheros en assets/audio (que es el estado
# real hoy, H6-02) la pieza debe arrancar en silencio y decirlo, y eso es justo
# lo que prueban los casos de bind() sin tocar el disco.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_audio_director
        tests/Game/AudioDirectorTests.cpp)
    target_include_directories(test_audio_director PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_audio_director PRIVATE
        motor_level motor_rpg motor_json)
    add_test(NAME test_audio_director COMMAND test_audio_director)
    set_tests_properties(test_audio_director PROPERTIES
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_audio_director)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_audio_director)
    endif()
endif()
