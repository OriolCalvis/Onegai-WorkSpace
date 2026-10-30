# EST-11 — ScopeGuard: el `finally` de C++ que pide la norma N4 (los errores se
# devuelven como Result<T> y un retorno temprano no puede dejar el trabajo a
# medias).
#
# Es solo cabecera, asi que el objetivo no enlaza ninguna libreria del motor y
# no hay fuente de src/ compilada suelta dentro de él (ley 4c). El consumidor
# real que lo estrena es FileSink::open, que retira el directorio que acaba de
# crear cuando la apertura del fichero falla.
include(CTest)
if(BUILD_TESTING)
    add_executable(test_scope_guard tests/Core/ScopeGuardTests.cpp)
    target_include_directories(test_scope_guard PRIVATE tests)
    add_test(NAME test_scope_guard COMMAND test_scope_guard)
    set_tests_properties(test_scope_guard PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_scope_guard)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_scope_guard)
    endif()
endif()
