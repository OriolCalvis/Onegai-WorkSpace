# C2-04a — Estado GL-free de la escena viva (posturas, fuego, emociones).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_escena_viva tests/Game/EscenaVivaTests.cpp)
    target_include_directories(test_escena_viva PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_escena_viva PRIVATE motor_level motor_json)
    add_test(NAME test_escena_viva COMMAND test_escena_viva)
    set_tests_properties(test_escena_viva PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_escena_viva)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_escena_viva)
    endif()
endif()
