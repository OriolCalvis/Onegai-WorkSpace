# H3-01 (Fase H) — WorldTravelState: el atlas (WorldTopology) conectado a
# LevelTransition. Vive en Game/ y no toca Application, GameSession ni ningún
# otro fichero de cruce (ver planificacion/MAPA_DEL_CODIGO.md §1).
#
# GL-free: la pieza no conoce GL, y la prueba tampoco.
#
# copiar_assets y WORKING_DIRECTORY porque los tres últimos casos juegan el
# mundo PUBLICADO (assets/levels/ciudad_centro.json, mapamundi.json,
# mundi_landmass_*.json, es_dhin_thyraxion.json): el criterio de cierre de
# H3-01 es una ruta del jugador y con fixtures seguiría en verde el día que
# alguien borre un desembarco.
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_world_travel_state
        tests/Game/WorldTravelStateTests.cpp)
    target_include_directories(test_world_travel_state PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_world_travel_state PRIVATE
        motor_level motor_project motor_map_document motor_rpg motor_json motor_map)
    add_dependencies(test_world_travel_state copiar_assets)
    add_test(NAME test_world_travel_state COMMAND test_world_travel_state)
    set_tests_properties(test_world_travel_state PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_world_travel_state)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_world_travel_state)
    endif()
endif()
