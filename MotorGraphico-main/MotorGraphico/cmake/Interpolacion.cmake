# U3-07 — Interpolacion pura (Core/Math/Interpolacion). GL-free.
# La fuente entra SIEMPRE en motor_math; solo el test depende de BUILD_TESTING.
target_sources(motor_math PRIVATE src/Core/Math/Interpolacion.cpp)

if(BUILD_TESTING)
    add_executable(test_interpolacion tests/Core/InterpolacionTests.cpp)
    target_include_directories(test_interpolacion PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_interpolacion PRIVATE motor_math)
    add_test(NAME test_interpolacion COMMAND test_interpolacion)
    set_tests_properties(test_interpolacion PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_interpolacion)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_interpolacion)
    endif()
endif()
