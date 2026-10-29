# C8-03c — La sesión de juego evalúa los eventos del mundo (sin GL).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_motor_unit_test(test_eventos_de_la_sesion tests/Game/EventosDeLaSesionTests.cpp)
    target_link_libraries(test_eventos_de_la_sesion PRIVATE motor_level motor_map motor_rpg motor_json)
endif()
