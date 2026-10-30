# EST-10 — Sistema de logs Core/Log: LogRecord y LogConfig son los DATOS,
# Logger es el CONTROLADOR, y LogFormat mas los tres sinks (memoria, consola,
# fichero) son las PIEZAS. Forma canonica: PLAN_ESTABILIZACION.md seccion 2.
#
# motor_log es una libreria y no fuentes sueltas dentro del objetivo de prueba
# a proposito: la ley del repo (AGENTS.md, regla 4c) prohibe compilar un
# src/*.cpp suelto dentro de un objetivo, y tener la libreria es lo que
# permite que quien consuma el log la enlace en vez de recompilarla.
#
# GL-free: solo depende de Core/Errors/Result.h (cabecera) y de la biblioteca
# estandar. No arrastra Render, GL ni assets, y por eso no necesita
# copiar_assets.
add_library(motor_log
    src/Core/Log/LogConfig.cpp
    src/Core/Log/LogFormat.cpp
    src/Core/Log/Logger.cpp
    src/Core/Log/ConsoleSink.cpp
    src/Core/Log/FileSink.cpp
)

# Primer consumidor REAL del log (ley 4: nada se cierra si no lo alcanza un
# ejecutable, y medir_alcance solo cuenta juego/motor_editor/level_editor — un
# demo o una prueba no los salva). Es el editor y no el juego porque su main.cpp
# es la frontera de excepciones del proceso: el sitio donde N4 pide que la
# captura produzca un log con categoria. Se enlaza aqui y no en
# cmake/MotorEditor.cmake para no tocar un fichero que tiene WIP de otra
# sesion; como este include va al final de CMakeLists, motor_editor ya existe.
if(TARGET motor_editor)
    target_link_libraries(motor_editor PRIVATE motor_log)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test_log tests/Core/LogTests.cpp)
    target_include_directories(test_log PRIVATE tests)
    target_link_libraries(test_log PRIVATE motor_log)
    add_test(NAME test_log COMMAND test_log)
    set_tests_properties(test_log PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_log)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_log)
    endif()
endif()
