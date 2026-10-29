# MAPA-02 — Seguir una mision, la flecha al objetivo y el mapa grande con
# chinchetas. Dos pruebas GL-free:
#   test_marcas_mapa — Game::MarcasMapa (poner y quitar chinchetas, el limite
#       de cinco, los colores) y la clave "mapa" del guardado (ida y vuelta, y
#       un guardado de antes sin ella); la mision seguida del diario.
#   test_guia_mapa — la flecha (calculaFlecha: dentro, en el borde, sin pisar
#       los retales) y seguir una mision desde una fila de la hoja Misiones.
if(BUILD_TESTING AND TARGET motor_level AND TARGET motor_json)
    add_executable(test_marcas_mapa tests/Game/MarcasMapaTests.cpp)
    target_include_directories(test_marcas_mapa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_marcas_mapa PRIVATE motor_level motor_json)
    add_test(NAME test_marcas_mapa COMMAND test_marcas_mapa)
    set_tests_properties(test_marcas_mapa PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_marcas_mapa)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_marcas_mapa)
    endif()
endif()
if(BUILD_TESTING AND TARGET motor_pergamino)
    add_executable(test_guia_mapa tests/HUD/GuiaMapaTests.cpp)
    target_include_directories(test_guia_mapa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_guia_mapa PRIVATE motor_pergamino)
    add_test(NAME test_guia_mapa COMMAND test_guia_mapa)
    set_tests_properties(test_guia_mapa PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_guia_mapa)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_guia_mapa)
    endif()
endif()
