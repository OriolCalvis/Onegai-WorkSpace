# H3-02 (Fase H) — QuestDefinition: la ficha de una mision y su catalogo.
# El progreso propio (QuestLogState) se retiro en AV14-22 (D0016): el estado
# son las flags y el diario es Game/DiarioMisiones; aqui se prueba la ficha.
#
# GL-free: la ficha no conoce GL, y la prueba tampoco.
#
# copiar_assets y WORKING_DIRECTORY porque los dos últimos casos juegan el
# catálogo PUBLICADO (assets/catalogs/quests.json y adventures.json): el
# criterio de cierre de H3-02 dice que las referencias de las aventuras
# resuelven, y con fixtures seguiría en verde el día que alguien borre la
# semilla real.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_quest_log
        tests/Game/QuestLogTests.cpp)
    target_include_directories(test_quest_log PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_quest_log PRIVATE
        motor_level motor_rpg motor_json)
    add_dependencies(test_quest_log copiar_assets)
    add_test(NAME test_quest_log COMMAND test_quest_log)
    set_tests_properties(test_quest_log PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_quest_log)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_quest_log)
    endif()
endif()
