# H0-08 (Fase H) — MapEventRunner, la capa de disparadores entre el mapa y el
# intérprete. Vive en RPG/MapEventRunner y NO toca GameSession, Application ni
# ningún fichero de cruce (ver planificacion/MAPA_DEL_CODIGO.md §1).
#
# GL-free: el runner no conoce GL, y la prueba necesita GameSession solo porque
# `inventario.dar` escribe en ella — igual que SessionCommandHostTests.
#
# copiar_assets y WORKING_DIRECTORY porque los cinco primeros casos juegan el
# asset PUBLICADO (assets/misiones/mision_llave.json), que es el criterio de
# cierre de H0-08: sin el fichero real, la prueba pasaría con un mapa inventado
# y no demostraría que la misión publicada se puede jugar.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_map_document)
    # H3-03 — AdventureValidation.cpp NO esta en ninguna libreria: se compila
    # suelto en cada objetivo que lo necesita (AdventureValidationTests,
    # AssetBrowserStateTests, MisionLlaveTests, MotorEditor, ProblemsStateTests).
    # Es la misma deuda que tenia Locale.cpp antes de H1-05 y que su propio
    # cmake/LocaleTests.cmake:12-15 predijo. Se sigue el patron existente en vez
    # de meterlo en motor_rpg, que tocaria cinco .cmake de otras sesiones.
    add_executable(test_map_event_runner
        tests/RPG/MapEventRunnerTests.cpp
        src/RPG/AdventureValidation.cpp)
    target_include_directories(test_map_event_runner PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_map_event_runner PRIVATE
        motor_level motor_map_document motor_rpg motor_json motor_map)
    add_dependencies(test_map_event_runner copiar_assets)
    add_test(NAME test_map_event_runner COMMAND test_map_event_runner)
    set_tests_properties(test_map_event_runner PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_map_event_runner)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_map_event_runner)
    endif()
endif()
