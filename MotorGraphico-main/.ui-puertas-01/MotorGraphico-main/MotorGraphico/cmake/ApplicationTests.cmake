include(CTest)
if(BUILD_TESTING AND TARGET motor_core)
    add_executable(test_application tests/Engine/ApplicationTests.cpp)
    target_include_directories(test_application PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_application PRIVATE motor_core motor_math motor_level)
    add_dependencies(test_application copiar_assets)
    add_test(NAME test_application COMMAND test_application)
    set_tests_properties(test_application PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl"
        TIMEOUT 60
        RUN_SERIAL TRUE
    )
endif()