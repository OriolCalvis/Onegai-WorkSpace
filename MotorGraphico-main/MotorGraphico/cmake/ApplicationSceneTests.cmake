include(CTest)
if(BUILD_TESTING AND TARGET motor_core)
    add_executable(test_application_scenes tests/Engine/ApplicationSceneTests.cpp)
    target_include_directories(test_application_scenes PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_application_scenes PRIVATE motor_core motor_math motor_level)
    add_dependencies(test_application_scenes copiar_assets)
    add_test(NAME test_application_scenes COMMAND test_application_scenes)
    set_tests_properties(test_application_scenes PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "integration;gl"
        TIMEOUT 120
        RUN_SERIAL TRUE)
endif()
