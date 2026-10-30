# DIALOGOS-01 — Las charlas a lo D&D: requires.lugar, "sigue" y las paginas
# que se saltan solas si se han quedado sin temas (tests/RPG/CharlasTests.cpp).
include(CTest)
if(BUILD_TESTING AND TARGET motor_rpg)
    add_executable(test_charlas tests/RPG/CharlasTests.cpp)
    target_include_directories(test_charlas PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_charlas PRIVATE motor_level motor_map_document motor_map motor_rpg motor_json)
    add_test(NAME test_charlas COMMAND test_charlas)
    set_tests_properties(test_charlas PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_charlas)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_charlas)
    endif()
endif()
