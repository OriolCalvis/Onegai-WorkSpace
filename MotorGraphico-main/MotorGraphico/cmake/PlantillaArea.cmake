# AV13-08 — Plantillas geometricas sin GL, enlazadas desde motor_level.
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_plantilla_area tests/Game/PlantillaAreaTests.cpp)
    target_include_directories(test_plantilla_area PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_plantilla_area PRIVATE motor_level)
    add_test(NAME test_plantilla_area COMMAND test_plantilla_area)
    set_tests_properties(test_plantilla_area PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_plantilla_area)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_plantilla_area)
    endif()
endif()
