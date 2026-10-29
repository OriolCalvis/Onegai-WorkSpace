# H6-03 (Fase H) — OptionsFile: los ajustes del jugador guardados, y los setters
# de OptionsState que los aplican. GL-free y sin assets: el test escribe y borra
# sus propios ficheros en el directorio de trabajo.
#
# WORKING_DIRECTORY porque los ficheros temporales (opf_*.json) tienen que caer
# en el build dir y no entre las fuentes.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_options_file
        tests/Game/OptionsFileTests.cpp)
    target_include_directories(test_options_file PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_options_file PRIVATE motor_level motor_json)
    add_test(NAME test_options_file COMMAND test_options_file)
    set_tests_properties(test_options_file PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_options_file)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_options_file)
    endif()
endif()
