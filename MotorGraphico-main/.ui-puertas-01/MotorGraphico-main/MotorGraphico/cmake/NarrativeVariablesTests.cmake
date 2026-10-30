# E-02 — Variables numericas con nombre e interruptores locales A-D, y su
# viaje al guardado. Fragmento aparte y no una linea suelta en el
# CMakeLists: es la convencion del proyecto para tests nuevos y la que
# evita que varios agentes se pisen editando el mismo bloque.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_narrative_variables tests/RPG/NarrativeVariablesTests.cpp)
    target_include_directories(test_narrative_variables PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_narrative_variables PRIVATE motor_level motor_rpg motor_json)
    add_test(NAME test_narrative_variables COMMAND test_narrative_variables)
    set_tests_properties(test_narrative_variables PROPERTIES LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_narrative_variables)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_narrative_variables)
    endif()
endif()
