# PNJ-02 (ronda del 15/10, carril pnj_motor) — Los PNJ se mueven y hacen cosas.
#
# Game::RutinasPnj (GL-free) entra en motor_level: la usa GameSession. El
# bocadillo sobre la cabeza (BocadillosPnj.cpp, sin GL) entra en
# motor_pergamino, junto a los carteles de las puertas. La prueba juega las
# reglas con un catalogo de prueba escrito dentro de ella (no el del juego) y,
# si ya esta, comprueba que assets/catalogs/rutinas_pnj.json se lee.
if(TARGET motor_level)
    target_sources(motor_level PRIVATE src/Game/RutinasPnj.cpp src/Game/RutinasPnjCatalogo.cpp)
endif()
if(TARGET motor_pergamino)
    target_sources(motor_pergamino PRIVATE src/HUD/Pergamino/BocadillosPnj.cpp)
endif()
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_rutinas_pnj tests/Game/RutinasPnjTests.cpp)
    target_include_directories(test_rutinas_pnj PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_rutinas_pnj PRIVATE motor_level)
    add_test(NAME test_rutinas_pnj COMMAND test_rutinas_pnj)
    set_tests_properties(test_rutinas_pnj PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_rutinas_pnj)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_rutinas_pnj)
    endif()
endif()
