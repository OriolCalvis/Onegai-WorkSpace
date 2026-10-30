# G-07 (el mecanismo, ANTES de la congelacion) — ficheros de ayer que
# tienen que seguir abriendose hoy.
#
# Cuatro formatos se escriben a disco y se leen mas tarde: proyecto,
# mapa, aventura y guardado. Nada impedia que un cambio en un
# serializador dejara ilegible lo que ya hay escrito, y eso no da error
# de compilacion ni rompe ningun test: el proyecto del usuario deja de
# abrirse y ya esta. En esta misma rama, E-01 metio una capa de eventos
# dentro del mapa y E-02 dos claves dentro del guardado.
#
# Las fixtures de tests/fixtures/esquemas/ se LEEN, nunca se regeneran
# desde el test: un test que regenerara su propia referencia pasaria
# siempre. Regenerarlas es a mano y deliberado:
#   ./test_schema_fixtures --escribir tests/fixtures/esquemas
#
# NO es la congelacion de G-07: congelar pide que los formatos esten
# terminados y E-16 todavia va a tocar el de proyecto. Es el mecanismo
# que G-07 necesitara, puesto ya porque el agujero esta abierto hoy.
include(CTest)
if(BUILD_TESTING AND TARGET motor_project AND TARGET motor_level)
    add_executable(test_schema_fixtures tests/Integration/SchemaFixtureTests.cpp)
    target_include_directories(test_schema_fixtures PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_schema_fixtures PRIVATE
        motor_level motor_project motor_map_document motor_rpg motor_json)
    add_test(NAME test_schema_fixtures COMMAND test_schema_fixtures)
    set_tests_properties(test_schema_fixtures PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_schema_fixtures)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_schema_fixtures)
    endif()
endif()
