# E-GUION-02 — fabricar_guardados: juega con el arnes de las demos hasta tres momentos (los sectarios del dia 1,
# el Ocaso antes de las campanas y el combate de cinco) y escribe un guardado de cada uno, para el modo guion
# (`juego --guardado g.json --guion s.txt`). GL-free: no abre ventana. Su prueba comprueba que se escriben y que
# se restauran en una sesion limpia.
add_executable(fabricar_guardados examples/fabricar_guardados.cpp)
target_link_libraries(fabricar_guardados PRIVATE motor_level motor_map motor_project motor_rpg motor_saves)  # motor_saves: SaveSlots, las ranuras de --matriz
set_target_properties(fabricar_guardados PROPERTIES CXX_STANDARD 17)
add_dependencies(fabricar_guardados copiar_assets)

if(BUILD_TESTING)
    add_executable(test_guardados_de_captura tests/Engine/GuardadosDeCapturaTests.cpp)
    target_include_directories(test_guardados_de_captura PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    target_link_libraries(test_guardados_de_captura PRIVATE motor_level motor_map motor_project motor_rpg)
    set_target_properties(test_guardados_de_captura PROPERTIES CXX_STANDARD 17)
    add_dependencies(test_guardados_de_captura copiar_assets)
    add_test(NAME test_guardados_de_captura COMMAND test_guardados_de_captura)
    set_tests_properties(test_guardados_de_captura PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free" TIMEOUT 300)
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_guardados_de_captura)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_guardados_de_captura)
    endif()

    # E-MATRIZ-01 — un guardado y una prueba por camino del dia 1 (con su mutacion).
    add_executable(test_matriz_dia1 tests/Engine/MatrizDia1Tests.cpp)
    target_include_directories(test_matriz_dia1 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    target_link_libraries(test_matriz_dia1 PRIVATE motor_level motor_map motor_project motor_rpg)
    set_target_properties(test_matriz_dia1 PROPERTIES CXX_STANDARD 17)
    add_dependencies(test_matriz_dia1 copiar_assets)
    add_test(NAME test_matriz_dia1 COMMAND test_matriz_dia1)
    set_tests_properties(test_matriz_dia1 PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free" TIMEOUT 600)
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_matriz_dia1)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_matriz_dia1)
    endif()
endif()
