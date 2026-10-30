# PERGAMINO-01 — El tema pergamino (HUD en retales, dialogo en hoja, cuaderno
# con pasar hoja y papiros) SIN OpenGL: lienzos de trazos, materiales
# fabricados, las dos voces de texto, la vuelta de hoja y el aura de la moral.
# El unico que los dibuja es Render/PergaminoGL (en motor_core, con GL).
#
# Libreria propia y no fuentes dentro de motor_core: motor_core ya mezcla
# fuentes con y sin GL (linea_base_core, «mezcladas»), y cada fuente sin GL
# que entrara ahi seria una prueba mas que necesita ventana.
add_library(motor_pergamino
    src/HUD/Pergamino/Lienzo.cpp
    src/HUD/Pergamino/MaterialesPapel.cpp
    src/HUD/Pergamino/MaterialesPiezas.cpp
    src/HUD/Pergamino/BancoMateriales.cpp
    src/HUD/Pergamino/FuenteAtlas.cpp
    src/HUD/Pergamino/Pincel.cpp
    src/HUD/Pergamino/PaginaCuaderno.cpp
    src/HUD/Pergamino/Cuaderno.cpp
    src/HUD/Pergamino/Papiro.cpp
    src/HUD/Pergamino/HudRetales.cpp
    src/HUD/Pergamino/DialogoHoja.cpp
    src/HUD/Pergamino/MinimapaTinta.cpp
    src/HUD/Pergamino/PaginasJugador.cpp
    src/HUD/Pergamino/InterfazPergamino.cpp
    src/HUD/Pergamino/InterfazDerrota.cpp  # AV13-33: la escena de noqueo y «Has caido»
    src/HUD/Pergamino/CartelesPuertas.cpp  # AV15-02: el cartel con el nombre sobre cada puerta
    # UI-BG3-01: retratos en la CPU para las rejillas y la vista previa, y el
    # cuaderno de creacion de siete hojas (maquetas C02).
    src/HUD/Pergamino/AtlasRetratos.cpp
    src/HUD/Pergamino/PaginasCreacion.cpp
    src/HUD/Pergamino/PaginasCreacionHojas.cpp
    src/HUD/Pergamino/PaginasCreacionHistoria.cpp
    # UI-BG3-01: el combate con la barra de cartas (maquetas C03).
    src/HUD/Pergamino/MesaCombateHud.cpp
    src/HUD/Pergamino/MesaCombateRetales.cpp
    # H2-01: el dado se ve (los dados de la ultima tirada, en el combate y en
    # la prueba de un dialogo).
    src/HUD/Pergamino/DadosPergamino.cpp
    # H2-03: los estados con su icono y sus rondas (placa, retrato del heroe y
    # emergente).
    src/HUD/Pergamino/MesaCombateEstados.cpp
    # TIENDA-02: la tienda en una hoja (plantilla T2, maqueta M06).
    src/HUD/Pergamino/TiendaHoja.cpp
    # COMBATE-01: los numeros de los golpes y el destello cuando te pegan.
    src/HUD/Pergamino/GolpesEnPantalla.cpp
    # AV13-10: el borde del tablero del combate, en tinta sobre el suelo.
    src/HUD/Pergamino/SueloCombate.cpp
    # MAPA-02: la mision seguida, la flecha al objetivo y el mapa grande con chinchetas.
    src/HUD/Pergamino/GuiaMapa.cpp
    src/HUD/Pergamino/MapaGrande.cpp
    src/Game/PasarHoja.cpp
    src/Game/AuraMoral.cpp
)
target_include_directories(motor_pergamino PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
# motor_map: GameSession colisiona contra TileMap (mismo patron que motor_saves).
target_link_libraries(motor_pergamino PUBLIC motor_level motor_map)

if(BUILD_TESTING)
    # test_pergamino: la maquetacion del tema medida sin ventana (donde cae
    # la cinta del foco, cuanto mide el papiro, que la hoja que cae no pase
    # de las tapas) y las reglas de la vuelta de hoja y del aura de la moral.
    # H2-01: los dados de la ultima tirada (DadosPergaminoTests).
    add_executable(test_pergamino tests/HUD/PergaminoTests.cpp tests/HUD/PergaminoPantallasTests.cpp
                                  tests/HUD/BancoMaterialesTests.cpp tests/HUD/PaginasCreacionTests.cpp
                                  tests/HUD/MesaCombateTests.cpp tests/HUD/DadosPergaminoTests.cpp
                                  tests/HUD/TiendaHojaTests.cpp tests/HUD/CicatricesCuadernoTests.cpp)
    target_include_directories(test_pergamino PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_pergamino PRIVATE motor_pergamino)
    if(TARGET copiar_assets)
        add_dependencies(test_pergamino copiar_assets)
    endif()
    add_test(NAME test_pergamino COMMAND test_pergamino)
    set_tests_properties(test_pergamino PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_pergamino)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_pergamino)
    endif()
endif()

# COMBATE-01 — Los numeros de los golpes (GL-free).
if(BUILD_TESTING)
    add_executable(test_golpes_en_pantalla tests/HUD/GolpesEnPantallaTests.cpp)
    target_include_directories(test_golpes_en_pantalla PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_golpes_en_pantalla PRIVATE motor_pergamino)
    add_test(NAME test_golpes_en_pantalla COMMAND test_golpes_en_pantalla)
    set_tests_properties(test_golpes_en_pantalla PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_golpes_en_pantalla)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_golpes_en_pantalla)
    endif()
endif()

# AV13-10 — El borde del tablero en el suelo (GL-free).
if(BUILD_TESTING)
    add_executable(test_suelo_combate tests/HUD/SueloCombateTests.cpp)
    target_include_directories(test_suelo_combate PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(test_suelo_combate PRIVATE motor_pergamino)
    add_test(NAME test_suelo_combate COMMAND test_suelo_combate)
    set_tests_properties(test_suelo_combate PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" LABELS "unit;gl-free")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_suelo_combate)
    if(TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_suelo_combate)
    endif()
endif()
