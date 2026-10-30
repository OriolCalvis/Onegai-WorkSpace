# SPRITES-01 — Game::RegistroSprites: el indice real de assets/textures/
# sprites.json carga sin avisos, cada hoja existe con la rejilla que
# declara (motor_imagen), y las reglas de busqueda (alias, patrones por
# prefijo y sufijo, raza) devuelven lo que las tablas de ApplicationNivel
# devolvian a mano. Lee assets/ desde build/ (copiar_assets).
include(CTest)
if(BUILD_TESTING AND TARGET motor_level)
    add_executable(test_registro_sprites tests/Game/RegistroSpritesTests.cpp)
    target_include_directories(test_registro_sprites PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(test_registro_sprites PRIVATE motor_level motor_imagen motor_json)
    if(TARGET copiar_assets)
        add_dependencies(test_registro_sprites copiar_assets)
    endif()
    add_test(NAME test_registro_sprites COMMAND test_registro_sprites)
    set_tests_properties(test_registro_sprites PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_registro_sprites)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_registro_sprites)
    endif()
endif()
