# BLOQUES-02 — Game::Bandada: los pajaros se posan en azoteas, despegan
# cuando el jugador se acerca, vuelan alto y aterrizan; determinista.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_bandada tests/Game/BandadaTests.cpp)
    target_include_directories(test_bandada PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_bandada PRIVATE motor_level)
    add_test(NAME test_bandada COMMAND test_bandada)
    set_tests_properties(test_bandada PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_bandada)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_bandada)
    endif()
endif()
