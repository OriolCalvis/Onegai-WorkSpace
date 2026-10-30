# U4-07 — Emisor de particulas generico (Game/Emisor). GL-free.
# La fuente entra SIEMPRE en motor_level (las familias de particulas viven alli); solo los tests dependen
# de BUILD_TESTING. E02: lector de la definicion. E03: emisiones con residuo. E04: crearParticula.
target_sources(motor_level PRIVATE src/Game/Emisor/DefinicionEmisor.cpp src/Game/Emisor/EmisionConResiduo.cpp
    src/Game/Emisor/CrearParticula.cpp)

if(BUILD_TESTING)
    add_executable(test_emisor tests/Game/EmisorTests.cpp)
    target_include_directories(test_emisor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_emisor PRIVATE motor_level motor_json)
    add_test(NAME test_emisor COMMAND test_emisor)
    set_tests_properties(test_emisor PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_emisor)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_emisor)
    endif()
endif()

if(BUILD_TESTING)
    add_executable(test_emisor_particula tests/Game/EmisorParticulaTests.cpp)
    target_include_directories(test_emisor_particula PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_emisor_particula PRIVATE motor_level motor_json)
    add_test(NAME test_emisor_particula COMMAND test_emisor_particula)
    set_tests_properties(test_emisor_particula PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_emisor_particula)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_emisor_particula)
    endif()
endif()
