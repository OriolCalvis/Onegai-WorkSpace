# C-23: contratos visuales comprobados contra un contexto OpenGL real.
if(BUILD_TESTING AND TARGET motor_core)
    add_executable(test_post_processing tests/Render/PostProcessingTests.cpp)
    target_include_directories(test_post_processing PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_post_processing PRIVATE motor_core motor_math)
    add_dependencies(test_post_processing copiar_assets)
    add_test(NAME test_post_processing COMMAND test_post_processing)
    set_tests_properties(test_post_processing PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "integration;gl"
        TIMEOUT 60 RUN_SERIAL TRUE)
    foreach(demo fog dynamic_lights lighting lut)
        if(TARGET demo_${demo})
            add_test(NAME test_post_processing_demo_${demo} COMMAND demo_${demo} 3)
            set_tests_properties(test_post_processing_demo_${demo} PROPERTIES
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                LABELS "integration;gl" TIMEOUT 60 RUN_SERIAL TRUE)
        endif()
    endforeach()
endif()
