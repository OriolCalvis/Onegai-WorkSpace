# E-GUION-01 — El guion de captura (Engine/GuionCaptura): parseo, estado de teclas por fotograma y lista
# de capturas. GL-free: solo Result y el vocabulario de teclas (motor_input). Application lo enlaza via motor_core.
add_library(motor_guion_captura src/Engine/GuionCaptura.cpp)
target_link_libraries(motor_guion_captura PUBLIC motor_input)

if(BUILD_TESTING)
    add_executable(test_guion_captura tests/Engine/GuionCapturaTests.cpp)
    target_include_directories(test_guion_captura PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_compile_definitions(test_guion_captura PRIVATE MOTOR_FUENTES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(test_guion_captura PRIVATE motor_guion_captura)
    add_test(NAME test_guion_captura COMMAND test_guion_captura)
    set_tests_properties(test_guion_captura PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_guion_captura)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_guion_captura)
    endif()
endif()
