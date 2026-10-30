# BLOQUES-01 — Game::Bloques: que celdas levantan bloque y con que caras
# (expuestas, puerta, ventanas fijas), sin ventana; y que bloques.json
# nombra tiles que existen en terreno_iso.json. Lee assets/ desde build/.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_bloques tests/Game/BloquesTests.cpp)
    target_include_directories(test_bloques PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_bloques PRIVATE motor_level motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_bloques copiar_assets)
    endif()
    add_test(NAME test_bloques COMMAND test_bloques)
    set_tests_properties(test_bloques PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_bloques)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_bloques)
    endif()
endif()
