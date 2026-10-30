# C-15 — addLevelToProject. Fragmento aparte y no una linea suelta en el
# CMakeLists: es la convencion que ya sigue este proyecto (ver
# ProjectManifestTests.cmake, ConditionEngineTests.cmake...) y la que
# evita que varios agentes se pisen editando el mismo bloque del
# CMakeLists a la vez.
include(CTest)
if(BUILD_TESTING AND TARGET motor_project)
    add_executable(test_project_levels tests/Project/ProjectLevelsTests.cpp)
    target_include_directories(test_project_levels PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_project_levels PRIVATE motor_project)
    add_test(NAME test_project_levels COMMAND test_project_levels)
    set_tests_properties(test_project_levels PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_project_levels)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_project_levels)
    endif()
endif()
