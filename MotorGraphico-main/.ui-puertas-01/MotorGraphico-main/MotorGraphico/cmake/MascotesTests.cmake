# MASC-01 — Game::Mascotes i Game::FullaMascotes: passegen per cel·les
# transitables a prop de casa, saluden o fugen del jugador, dormen a casa i
# trien el frame de la fulla segons l'estat; deterministes.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_mascotes tests/Game/MascotesTests.cpp)
    target_include_directories(test_mascotes PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_mascotes PRIVATE motor_level)
    add_test(NAME test_mascotes COMMAND test_mascotes)
    set_tests_properties(test_mascotes PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_mascotes)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_mascotes)
    endif()
endif()
