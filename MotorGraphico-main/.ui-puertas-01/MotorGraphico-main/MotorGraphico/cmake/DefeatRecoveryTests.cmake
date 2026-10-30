# H3-08 (Fase H) — DefeatRecovery: la política de DEC-09 (volver al último punto
# seguro) y la pantalla de derrota que solo ofrece cargar si hay partida.
#
# GL-free. El último caso escribe ranuras de verdad con el servicio de G-11 en un
# directorio temporal (mismo patrón Sandbox que tests/Game/SaveSlotsTests.cpp),
# así que la sobrecarga plan(SaveSlots) no queda sin probar.
include(CTest)
if(BUILD_TESTING AND TARGET motor_saves)
    add_executable(test_defeat_recovery
        tests/Game/DefeatRecoveryTests.cpp)
    target_include_directories(test_defeat_recovery PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_defeat_recovery PRIVATE
        motor_saves motor_level motor_project motor_map_document motor_rpg motor_json motor_map)
    add_test(NAME test_defeat_recovery COMMAND test_defeat_recovery)
    set_tests_properties(test_defeat_recovery PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_defeat_recovery)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_defeat_recovery)
    endif()
endif()
