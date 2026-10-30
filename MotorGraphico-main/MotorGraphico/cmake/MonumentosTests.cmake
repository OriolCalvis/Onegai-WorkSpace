# BLOQUES-03 — Game::Monumentos: huellas que son un monumento, tiras por celda
# y celdas cubiertas; geometria de las tiras.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_monumentos tests/Game/MonumentosTests.cpp)
    target_include_directories(test_monumentos PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_monumentos PRIVATE motor_level motor_json)
    add_test(NAME test_monumentos COMMAND test_monumentos)
    set_tests_properties(test_monumentos PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_monumentos)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_monumentos)
    endif()
endif()
