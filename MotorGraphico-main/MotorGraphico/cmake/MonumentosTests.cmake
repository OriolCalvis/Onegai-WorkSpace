# BLOQUES-03 — Game::Monumentos: huellas que son un monumento, tiras por celda
# y celdas cubiertas; geometria de las tiras.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_monumentos tests/Game/MonumentosTests.cpp)
    target_include_directories(test_monumentos PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_monumentos PRIVATE motor_level motor_json motor_map)
    # Las pruebas de ciudad_centro leen assets/ del build (cwd = build).
    if(TARGET copiar_assets)
        add_dependencies(test_monumentos copiar_assets)
    endif()
    add_test(NAME test_monumentos COMMAND test_monumentos)
    set_tests_properties(test_monumentos PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_monumentos)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_monumentos)
    endif()
endif()

# J-4 / J-4b — las casas de tile antiguo (viejo_chabola...) toman el sprite de su familia: base entera de la
# casa y cara de la puerta transitable.
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_monumentos_antiguos tests/Game/MonumentosAntiguosTests.cpp)
    target_include_directories(test_monumentos_antiguos PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_monumentos_antiguos PRIVATE motor_level motor_json motor_map)
    if(TARGET copiar_assets)
        add_dependencies(test_monumentos_antiguos copiar_assets)
    endif()
    add_test(NAME test_monumentos_antiguos COMMAND test_monumentos_antiguos)
    set_tests_properties(test_monumentos_antiguos PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_monumentos_antiguos)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_monumentos_antiguos)
    endif()
endif()
