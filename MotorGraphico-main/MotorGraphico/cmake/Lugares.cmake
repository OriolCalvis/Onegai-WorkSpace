# AV15-02b — Contrato del catalogo de rotulos, sin motor ni GL.
if(BUILD_TESTING AND TARGET motor_json AND TARGET motor_level)
    add_executable(test_lugares tests/Game/LugaresTests.cpp)
    target_include_directories(test_lugares PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_lugares PRIVATE motor_level motor_json)  # AV15-02: y Game::Lugares
    add_test(NAME test_lugares COMMAND test_lugares)
    set_tests_properties(test_lugares PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_lugares)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_lugares)
    endif()
endif()

# AV16-08: prueba del contrato; los datos reales siguen pendientes de PO-48 y Qwen.
find_program(MOTOR_PYTHON3 python3)
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_validar_lugares
        COMMAND "${MOTOR_PYTHON3}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_validar_lugares.py")
    set_tests_properties(test_validar_lugares PROPERTIES LABELS "unit;gl-free")
endif()
