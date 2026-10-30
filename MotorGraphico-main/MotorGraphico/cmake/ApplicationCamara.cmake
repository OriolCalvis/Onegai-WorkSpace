# U4-01.E06 — Adaptador de camara del juego (Engine/ApplicationCamara). GL-free: solo Camera + politica pura.
# El test lee ademas fuentes del juego (MOTOR_FUENTES_DIR) para vigilar quien lee posicionVista().
if(BUILD_TESTING)
    add_executable(test_application_camara tests/Engine/ApplicationCamaraTests.cpp src/Engine/ApplicationCamara.cpp)
    target_include_directories(test_application_camara PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_compile_definitions(test_application_camara PRIVATE MOTOR_FUENTES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(test_application_camara PRIVATE motor_math)
    add_test(NAME test_application_camara COMMAND test_application_camara)
    set_tests_properties(test_application_camara PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_application_camara)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_application_camara)
    endif()
endif()
