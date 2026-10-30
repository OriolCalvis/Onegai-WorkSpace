# EST-52 — Ninguna excepcion cruza la frontera de un modulo.
#
# La norma N4 pide que a la frontera de un modulo todo fallo se convierta en
# Result<T> y que una excepcion no salga. Esto no se comprueba leyendo codigo:
# se comprueba llamando a la frontera con entradas que la obligan a fallar y
# mirando que devuelve un error por su canal, jamas una excepcion.
#
# Cubre la frontera del log (sink que falla, sink nulo, reloj nulo, id de
# sesion invalido, ruta de fichero imposible) y la del guardado (leer basura,
# guardar donde no se puede).
#
# Va en su propio .cmake y no dentro de CMakeLists.txt por la ley 3 del repo
# (AGENTS.md): un include(cmake/X.cmake) va SIEMPRE en el mismo commit que su
# .cmake. Sin fuentes sueltas: motor_log y motor_level son librerias, que es lo
# que la ley 4c pide (no compilar un src/*.cpp dentro del objetivo).
#
# GL-free: solo motor_log (Core/Log y Core/Errors/Result.h), motor_level (ahi
# vive SaveGame, la otra frontera que se prueba) y motor_json. Sin Render ni GL.

include(CTest)
if(BUILD_TESTING)
    add_executable(test_boundary_exceptions tests/Core/BoundaryExceptionsTests.cpp)
    target_include_directories(test_boundary_exceptions PRIVATE tests)
    target_link_libraries(test_boundary_exceptions PRIVATE motor_log motor_level motor_json)
    add_test(NAME test_boundary_exceptions COMMAND test_boundary_exceptions)
    set_tests_properties(test_boundary_exceptions PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_boundary_exceptions)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_boundary_exceptions)
    endif()
endif()
