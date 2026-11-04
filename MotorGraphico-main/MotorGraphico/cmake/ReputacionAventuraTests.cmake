# BND-04 — requires.reputacion de las aventuras (RPG puro, sin GL).
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_reputacion_aventura tests/RPG/ReputacionAventuraTests.cpp)
    target_include_directories(test_reputacion_aventura PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/RPG")
    target_link_libraries(test_reputacion_aventura PRIVATE motor_rpg motor_json)
    add_test(NAME test_reputacion_aventura COMMAND test_reputacion_aventura)
    set_tests_properties(test_reputacion_aventura PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_reputacion_aventura)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_reputacion_aventura)
    endif()
endif()
