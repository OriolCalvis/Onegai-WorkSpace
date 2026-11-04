# C2-04d — Posicion del bocadillo sobre un actor (funcion pura, sin GL).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_posicion_bocadillo tests/Game/PosicionBocadilloTests.cpp)
    target_include_directories(test_posicion_bocadillo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_posicion_bocadillo PRIVATE motor_level)
    add_test(NAME test_posicion_bocadillo COMMAND test_posicion_bocadillo)
    set_tests_properties(test_posicion_bocadillo PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_posicion_bocadillo)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_posicion_bocadillo)
    endif()
endif()
