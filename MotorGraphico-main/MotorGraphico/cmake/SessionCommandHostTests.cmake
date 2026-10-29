# E-04 (segunda rebanada) — Las familias de comando que necesitan la
# partida: inventario, oro y cambio de mapa, mas las condiciones de
# inventario. Vive en RPG/SessionCommandHost y NO toca GameSession, que
# comparten seis paquetes (ver planificacion/MAPA_DEL_CODIGO.md).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_map_document)
    add_executable(test_session_command_host
        tests/RPG/SessionCommandHostTests.cpp)
    target_include_directories(test_session_command_host PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_session_command_host PRIVATE
        motor_level motor_map_document motor_rpg motor_json motor_map)
    add_test(NAME test_session_command_host COMMAND test_session_command_host)
    set_tests_properties(test_session_command_host PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_session_command_host)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_session_command_host)
    endif()
endif()
