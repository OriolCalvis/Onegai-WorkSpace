# C8-03b — Lectura y validación del catálogo de eventos del mundo.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_eventos_del_mundo tests/RPG/EventosDelMundoTests.cpp)
    target_include_directories(test_eventos_del_mundo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_eventos_del_mundo PRIVATE motor_rpg motor_json)
    # C8-04a: un caso lee assets/eventos/boundington.json desde el directorio de build.
    add_dependencies(test_eventos_del_mundo copiar_assets)
    add_test(NAME test_eventos_del_mundo COMMAND test_eventos_del_mundo)
    set_tests_properties(test_eventos_del_mundo PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_eventos_del_mundo)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_eventos_del_mundo)
    endif()
endif()
