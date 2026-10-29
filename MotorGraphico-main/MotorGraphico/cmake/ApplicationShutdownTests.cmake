# C-05: prueba de Application real; requiere display/contexto (Linux: xvfb).
include(CTest)
if(BUILD_TESTING AND TARGET motor_core)
    add_executable(test_application_shutdown tests/Engine/ApplicationShutdownTests.cpp)
    target_include_directories(test_application_shutdown PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_application_shutdown PRIVATE motor_core motor_math motor_level)
    add_dependencies(test_application_shutdown copiar_assets)
    add_test(NAME test_application_shutdown COMMAND test_application_shutdown)
    set_tests_properties(test_application_shutdown PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "integration;gl"
        TIMEOUT 60
        RUN_SERIAL TRUE
    )
endif()
