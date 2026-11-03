cmake_minimum_required(VERSION 3.16)
project(MotorGraficoIsometrico CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# INF-01.1 — Los demos GL-free ya contienen assert() y devuelven un
# código de error al fallar. CTest los incorpora como primera capa de
# regresión, sin obligar a que un desarrollador tenga ventana/OpenGL.
# Los demos gráficos quedan fuera hasta INF-01.4, donde CI podrá envolverlos
# explícitamente con xvfb-run.
include(CTest)

include_directories(${CMAKE_SOURCE_DIR}/include)

# assets/ (shaders, texturas, mapas TMX, aventuras) se referencia con rutas
# relativas desde los ejecutables (ver demo_textured_quad.cpp, demo_tilemap.cpp):
# se copia junto al binario para que funcionen ejecutados desde build/.
#
# OJO CON ESTO: file(COPY ...) corre al CONFIGURAR, no al compilar. Con eso
# solo, quien anadia un asset nuevo y hacia "cmake --build ." se encontraba con
# que build/assets/ seguia sin el fichero, y el demo fallaba al cargarlo sin
# ninguna pista de por que -- el fichero SI estaba en assets/, delante de sus
# narices. Paso de verdad con boundington_segundo_dia.json y boundington_ocaso.
#
# La copia inicial se queda (deja el arbol listo tras configurar) y ademas hay
# un target que se ejecuta en CADA build y del que cuelgan los demos.
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/tests/fixtures DESTINATION ${CMAKE_BINARY_DIR}/tests)

add_custom_target(copiar_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMENT "Sincronizando assets/ -> build/assets/"
    VERBATIM
)

# --- Demo sin dependencias externas: prueba ResourceManager<T> de forma
# aislada. Compila y pasa en cualquier maquina con g++/clang++ C++17. ---
add_executable(demo_resource_manager
    examples/demo_resource_manager.cpp
    examples/TextAssetManager.cpp
)
# INF-06.2 — Los fixtures de este demo se crean junto al binario, nunca
# en la raíz del repositorio desde la que se haya invocado.
file(TO_CMAKE_PATH "${CMAKE_CURRENT_BINARY_DIR}" DEMO_RESOURCE_MANAGER_OUTPUT_DIR)
target_compile_definitions(demo_resource_manager PRIVATE
    DEMO_OUTPUT_DIRECTORY="${DEMO_RESOURCE_MANAGER_OUTPUT_DIR}"
)

# Pool generacional GL-free: evita punteros colgantes al reciclar objetos
# dinámicos y deja una base común para partículas, proyectiles o encuentros.
add_executable(demo_handle_pool examples/demo_handle_pool.cpp)

# Regla pura de oclusion: test rapido sin OpenGL ni el editor visual.
add_executable(demo_occlusion_rules examples/demo_occlusion_rules.cpp)

# --- Camera + IsoMath: solo necesitan glm (header-only, sin GL). Se
# separan de motor_core (que si necesita GLAD/stb_image/GLFW vendorizados,
# ver mas abajo) para poder compilar y probar la camara y la proyeccion
# isometrica sin ventana ni contexto OpenGL real. ---
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

add_library(motor_math src/Render/Camera.cpp)
target_link_libraries(motor_math PUBLIC glm::glm)
include(cmake/Interpolacion.cmake)  # U3-07: progreso temporal e interpolacion (GL-free)

add_executable(demo_camera examples/demo_camera.cpp)
target_link_libraries(demo_camera PRIVATE motor_math)
include(cmake/PoliticaCamara.cmake)  # U4-01: politica pura de camara (GL-free)
include(cmake/ApplicationCamara.cmake)  # U4-01.E06: adaptador de camara en juego (GL-free)

# RUN-01 — El reloj y el contrato del bucle no dependen de GLFW ni de GL.
# Mantenerlos en su propio target permite probar la salida y el timestep sin
# abrir una ventana ni inicializar una partida.
add_library(motor_engine_loop src/Engine/ApplicationLoop.cpp)

# RUN-02 — Estado de teclado, bindings y snapshots de acciones. Es un núcleo
# puro: GLFW se adapta desde Application y los tests usan teclas simuladas.
add_library(motor_input src/Engine/Input/InputSystem.cpp)
include(cmake/GuionCaptura.cmake)  # E-GUION-01: guion de captura (GL-free)

# --- Fase 6 de motor_grafico_gantt_rpg.puml (sistema de niveles y
# contenido): JsonValue (parser JSON propio, sin dependencias de terceros
# -- ver el comentario de include/Core/Json/JsonValue.h) y, sobre el,
# LevelLoader/Skill (datos de nivel y de habilidades). Ninguno de los dos
# toca GL/GLFW, asi que -- igual que motor_math -- se compilan y prueban
# sin ventana ni contexto OpenGL. ---
add_library(motor_json src/Core/Json/JsonValue.cpp)

# APARIENCIA-01 — Una imagen PNG en memoria, sin OpenGL: la usa el montaje de
# personajes por capas (motor_level) y se prueba sin ventana. stb_image va en
# modo estatico (STB_IMAGE_STATIC): copia privada que no choca con la de
# motor_core ni comparte su volteo vertical.
add_library(motor_imagen src/Core/Imagen/ImagenRGBA.cpp)
target_include_directories(motor_imagen PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(motor_imagen PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb")

add_library(motor_rpg
    src/RPG/SkillExecutor.cpp
    src/RPG/SkillEffect.cpp
    src/RPG/LootResolver.cpp
    src/RPG/Audio.cpp
    src/RPG/SkillDefinitionEnums.cpp
    src/RPG/TierRules.cpp
    src/RPG/CharacterSheet.cpp
    src/RPG/CharacterCreation.cpp
    src/RPG/CharacterCreationScreen.cpp
    src/RPG/InventoryEngine.cpp
    src/RPG/ConditionEngine.cpp
    src/RPG/NarrativeEngine.cpp
    src/RPG/EventInterpreter.cpp
    src/RPG/EventCommands.cpp
    # 24/09/2026 (EST-32) — EventCommands.cpp tenia 566 lineas y hacia cuatro
    # trabajos. El catalogo se queda con el nombre del fichero (es lo que
    # compilan sueltos siete objetivos); el resto sale a su pieza:
    src/RPG/Eventos/ValidacionDeComandas.cpp
    src/RPG/Eventos/CondicionesYPaginas.cpp
    src/RPG/Eventos/AnfitrionNarrativo.cpp
    src/RPG/AdventureSerializer.cpp
    # 22/09/2026 (EST-25) — NarrativeEngine.cpp tenia 744 lineas y su
    # cabecera 543. Partido en datos / carga / flujo:
    src/RPG/Narrative/NarrativeState.cpp
    src/RPG/Narrative/AdventureScriptLoader.cpp
    src/RPG/Narrative/AcompanantesAventura.cpp  # ACOMPANANTES-01: los acompañantes de la aventura
    src/RPG/Narrative/MoralAventura.cpp  # MORAL-01: acto, herida, requires.moral y exitos fijos
    src/RPG/Narrative/ReputacionAventura.cpp  # BND-04: requires.reputacion
    src/RPG/Narrative/CharlaAventura.cpp  # DIALOGOS-01: requires.lugar y los papeles de las opciones
    src/RPG/Narrative/BeatsAventura.cpp  # VARIABLES-01 / OPCIONES-01: condiciones, efectos y opciones de un beat
    src/RPG/Narrative/FranjasDelDia.cpp  # FASE1-BND-01: el dia y la franja de la historia
    src/RPG/Narrative/EventosDelMundo.cpp  # C8-03b: eventos del mundo
    src/RPG/Narrative/HeroeEnLaHistoria.cpp  # HEROE-01: lo que la historia sabe del heroe creado
)
# motor_rpg = Motor Onegai Nd6. GL-free puro: contiene DicePoolEngine,
# CharacterSheet, EnhanceHooks, SkillExecutor, TierRules, InventoryEngine,
# ConditionEngine y 20 catalogos tipados (RpgCatalogs.h header-only +
# RpgCoreDefinitions.h). Todos los headers son include/RPG/*.h y NO
# incluyen Render/*.h ni GL.
target_link_libraries(motor_rpg PUBLIC motor_json)
target_include_directories(motor_rpg PRIVATE "${CMAKE_SOURCE_DIR}/third_party/miniaudio")

# DOC-01 — Los documentos de proyecto son independientes del runtime y
# del editor visual. ProjectIndex conserva el I/O de manifiestos; Project
# y ProjectSettings se validan sin abrir ventana ni cargar assets.
add_library(motor_project
    src/Project/Project.cpp
src/Project/ProjectDatabaseState.cpp
     src/Project/ProjectIndex.cpp
     src/Project/ProjectIndexDuplicate.cpp
     src/Project/ProjectSerializer.cpp
     # 21/09/2026 — ProjectSerializer.cpp tenia 1081 lineas y lo cruzaban
     # seis paquetes. Partido por trabajos (ley de los 500):
     src/Project/SerializerHelpers.cpp
     src/Project/MapSerializer.cpp
     src/Project/MapDeserializer.cpp
)
target_link_libraries(motor_project PUBLIC motor_json motor_map_document)

# DOC-02 — Modelo documental de mapas. No depende de TileMap ni de GL: el
# builder de DOC-03 será el único puente desde estos datos al runtime.
# E-01 — EventLayerDocument va en su propio .cpp y no dentro de
# MapDocument.cpp a proposito: ese fichero lo comparten C-12, C-13,
# E-01 y F-03 (planificacion/MAPA_DEL_CODIGO.md) y varios agentes
# escriben a la vez. Ver la nota de ubicacion en el propio fichero.
# 24/09/2026 (EST-31) — ProblemsState se compilaba SUELTO dentro de tres
# objetivos (motor_editor y dos pruebas): tres copias del mismo objeto y,
# al partirlo, habrian sido nueve. Con libreria propia son cero. GL-free:
# es el modelo de la lista de problemas, no su panel.
add_library(motor_editor_problems
    src/Editor/ProblemsState.cpp
    src/Editor/ProblemsNavigation.cpp
    src/Editor/ProblemsSearch.cpp
)
target_link_libraries(motor_editor_problems PUBLIC motor_map_document motor_rpg)

add_library(motor_map_document
    src/Map/MapDocument.cpp
    # 23/09/2026 (EST-27) — MapDocument.cpp tenia 724 lineas y cuatro
    # responsabilidades. Una por fichero:
    src/Map/TileLayerDocument.cpp
    src/Map/EntityLayerDocument.cpp
    src/Map/RegionLayerDocument.cpp
    src/Map/MapDocumentEdits.cpp src/Map/EventLayerDocument.cpp
    src/Map/RegionPresence.cpp)
target_link_libraries(motor_map_document PUBLIC motor_json motor_project)

# RUN-03 — Máquina de escenas sin ventana. Una Scene concreta decide cómo
# carga un mapa; el manager solo ordena su vida y las transiciones diferidas.
add_library(motor_scene src/Engine/Scene/SceneManager.cpp)

# RUN-04 — Propietario GL-free de entidades runtime y sus handles
# generacionales. El render queda fuera hasta el proxy de RUN-05.
add_library(motor_world src/World/World.cpp)

# DOC-03 — Runtime compacto construido desde MapDocument. Permanece libre de
# render y el adaptador TMX legado (motor_map) depende de él, nunca al revés.
add_library(motor_runtime_map
    src/Runtime/RuntimeTileMap.cpp
    src/Runtime/RuntimeTileMapBuilder.cpp
)
target_link_libraries(motor_runtime_map PUBLIC motor_map_document motor_math)

# H1-05 — motor_i18n: el idioma como libreria GL-free propia.
#
# Locale.cpp era el ejemplo de fuente suelta: no estaba en ninguna libreria
# y se compilaba dentro de cinco objetivos de prueba. Cuando Application.cpp
# empezo a llamarlo en init() no hubo mas remedio que meterlo en motor_core
# -- que lleva OpenGL -- y la guardia lo marco enseguida: una fuente GL-free
# atrapada en una libreria con GL es una prueba que necesita ventana.
#
# Aqui vive en su sitio: motor_core la enlaza (por eso `juego` enlaza) y las
# pruebas GL-free la enlazan en vez de compilar el .cpp sueltas, que es lo que
# la ley 4c pide. OptionsLocaleBridge es header-only desde H1-05 y no necesita
# fuente; cuelga de aqui por su Locale, no por la regla.
add_library(motor_i18n
    src/Engine/Locale.cpp
)
target_link_libraries(motor_i18n PUBLIC motor_json)
target_include_directories(motor_i18n PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(motor_level
    src/Level/LevelLoader.cpp
    src/Level/LevelSerializer.cpp
    # C-14 (1a rebanada) — Puente ObjectSpawn <-> MapEntityDocument. Vive
    # en Level/ y no en Map/ para no invertir la flecha: motor_level ya
    # depende de motor_map_document, no al reves.
    src/Level/LevelDocumentBridge.cpp
    src/Game/Skill.cpp
     # AV13-35: el menu de habilidades antiguo (BattleMenuState, BattleMenuActions, TargetSelection)
     # se borro con el combate sin mesa (planificacion_alcance_y_vision/MEDIDA_COMBATE_VIEJO.md).
     src/Game/BattleState.cpp
     src/Game/BattleStateTurno.cpp  # AV13-11: la iniciativa y quien llega a quien
     src/Game/BattleStateGolpes.cpp  # AV14-01: los golpes que resuelve la batalla
     src/Level/ObjectCatalog.cpp
    src/Editor/CatalogPickerState.cpp
    src/Editor/EditorState.cpp
    # 24/09/2026 (EST-24) — EditorState.cpp tenia 1075 lineas y seis
    # paquetes encima. Partido por trabajo del autor:
    src/Editor/EditorStateHelpers.cpp
    src/Editor/EditorStatePintar.cpp
    src/Editor/EditorStateObjetos.cpp
    src/Editor/EditorStateExportar.cpp
    editor/src/event_editor.cpp
    editor/src/MapPropertiesPanel.cpp
    editor/src/AssetBrowserPanel.cpp
    src/Editor/EditorDocumentIO.cpp
    src/Editor/ObjectInspector.cpp
    src/Editor/ProjectHub.cpp
    src/Game/GameSession.cpp
    # 23/09/2026 (EST-23) — GameSession.cpp tenia 1167 lineas y seis
    # paquetes encima. Partido por tema:
    src/Game/GameSessionComercio.cpp
    src/Game/GameSessionNarrativa.cpp
    src/Game/GameSessionInteraccion.cpp
    src/Game/GameSessionTablero.cpp  # AV13-10: cada uno en su casilla al empezar el combate
    src/Game/GameSessionInventario.cpp
    src/Game/Actuacion.cpp  # ACTUAR-01: actuar en la calle (la tirada, el publico y el oro)
    src/RPG/SessionCommandHost.cpp
    # H0-08 (Fase H): la capa de DISPARADORES que faltaba entre el mapa y el
    # interprete. El interprete arranca la pagina que alguien le da a mano;
    # nadie resolvia Touch/Autorun/Parallel/Interact desde la posicion del
    # jugador. Vive en motor_level y no en motor_rpg porque elige pagina con
    # ActivePageWithInventory, que esta en SessionCommandHost.cpp.
    src/RPG/MapEventRunner.cpp
    # U3-02.E06b: despacho de triggers de region al interprete (API publica).
    src/RPG/RegionTriggerDispatch.cpp
    # H3-01 (Fase H): el atlas (WorldTopology) conectado a LevelTransition.
    # Vive en motor_level porque necesita las dos puntas: las puertas del
    # nivel (Level/) y la transicion que ejecuta Application (Game/SessionTipos).
    src/Game/WorldTravelState.cpp
    src/Game/SaveGame.cpp
    # EST-29 (C-03/C-04/C-08/E-02/G-11) — SaveGameFormat.cpp se extrajo de
    # SaveGame.cpp y nunca se cableo al enlazador: sin el no enlaza ni
    # demo_nova_partida ni el test de cableado (parseSaveGame/
    # serializeSaveGame/applyNarrativeState). Vive con su gemelo, en motor_level.
    src/Game/SaveGameFormat.cpp
    src/Game/Morality.cpp
    # MORAL-01: los seis numeros y el catalogo de actos (ReglasMoral) y lo que la
    # aventura apunta en la moral de la sesion (MoralDeLaSesion).
    src/Game/ReglasMoral.cpp
    src/Game/MoralDeLaSesion.cpp
    src/Game/EventosDeLaSesion.cpp  # C8-03c
    src/Game/EnemyBrain.cpp
    src/Game/PauseMenu.cpp
    src/Game/OptionsState.cpp
    # H6-03 (Fase H): los ajustes del jugador guardados. Vive junto a
    # OptionsState.cpp, que es la pantalla a la que da puente.
    src/Game/OptionsFile.cpp
    # H3-08 (Fase H): la politica de DEC-09 (volver al ultimo punto seguro)
    # hecha codigo, y la pantalla de derrota que solo ofrece cargar si hay partida.
    src/Game/DefeatRecovery.cpp
    # H1-04 (Fase H): el tramo GL-free de «Nueva partida» — envoltorio de
    # CharacterCreationScreen cuyo finalizar() construye Y recalcula la
    # ficha (construir() solo no deja healthCap a punto para Player).
    src/Game/NewGameFlow.cpp
    src/Game/GameOverScreen.cpp
    # H6-01 (Fase H): la POLITICA de sonido — que suena en cada momento del
    # juego. RPG/AudioEngine (GDB-11) sabe COMO sonar y ningun ejecutable lo
    # llamaba; en MAPA_DEL_CODIGO.md cae en «sin paquete asignado». Esta pieza
    # es el cuando: musica exclusiva por GameMode, efectos que no la cortan, y
    # el puente de los volumenes 0..10 de OptionsValues (H6-03) al 0..1 por bus
    # que quiere el motor. Vive en motor_level porque consume GameMode y
    # OptionsValues, y la flecha Game -> RPG ya es la que hay.
    src/Game/AudioDirector.cpp
    src/Game/InventoryScreen.cpp
    src/Game/InventoryMenuState.cpp
    src/Game/TiendaEstado.cpp  # TIENDA-02: la tienda en dos pestanas, sin GL
    # H3-02 (Fase H): el diario de misiones. Su progreso propio
    # (QuestLogState) se retiro en AV14-22 (D0016): el estado son las flags y
    # el diario es Game/DiarioMisiones (abajo), con la ficha QuestDefinition.
    # H1-02 (Fase H): rescate de G-14. HudTimer i ShopEngine vivien
    # solo en tests/Game/G14HudTimersAndCommerceTests.cpp; aqui pasan a
    # motor_level para que Application/H3-04 los consiga.
    src/HUD/HudTimer.cpp
    # H2-01 (Fase H): el dado se ve — widget de tirada GL-free (solo texto;
    # lo pinta Application; ver src/HUD/DiceRollWidget.cpp).
    src/HUD/DiceRollWidget.cpp
    # H2-03 (Fase H): condiciones en el HUD — widget GL-free (solo texto;
    # lo pinta Application; ver src/HUD/ConditionWidget.cpp).
    src/HUD/ConditionWidget.cpp
    # H3-06 (Fase H): temporizadores de mision visibles — widget GL-free
    # (solo texto; lo pinta Application; ver src/HUD/MissionTimerWidget.cpp).
    src/HUD/MissionTimerWidget.cpp
    src/Game/ShopEngine.cpp
    # FICHA-01: la ficha del jugador como datos y el contrato de iconos del
    # HUD (GL-free; el widget que los pinta vive en motor_core).
    src/Game/FichaVista.cpp
    src/HUD/Iconos.cpp
    src/Game/DiarioMisiones.cpp  # MISIONES-01: diario = vista de las flags
    src/Game/FichasDeMisiones.cpp  # AV14-22: las fichas de quests.json para el diario (D0016)
    src/Game/RumboNiveles.cpp  # MAPA-01: por que puerta se va hacia el objetivo
    src/Game/TableroCombate.cpp  # AV13-06: las casillas del nivel donde se pelea
    src/Game/RutaCasillas.cpp  # AV13-07: la ruta por casillas y hasta donde se llega
    src/Game/Transiciones.cpp  # AV16-10: las capas de transicion entre suelos (dual-grid)
    src/Game/PlantillaArea.cpp  # AV13-08 (ChatGPT): las plantillas de area sobre casillas
    src/Game/Lugares.cpp  # AV15-02: los sitios de la ciudad por su nombre (lugares.json)
    src/Game/OrdenDeTurno.cpp  # AV13-11: el orden de turno por iniciativa
    src/Game/AlcanceCartas.cpp  # AV13-13: a quien llega cada carta sobre el tablero
    src/Game/EfectosDeGolpe.cpp  # AV14-02: lo que hace cada golpe en el cuerpo y en tu hoja
    src/Game/Cicatrices.cpp  # AV13-33: las cicatrices que deja morir, fuera de las partidas
    src/Game/EscenaNoqueo.cpp  # AV13-33: la escena de noqueo o rescate antes de «Has caido»
    src/Game/FrasesDeCombate.cpp  # EH-16: el resultado de las tiradas, en palabras
    src/Game/MarcasMapa.cpp  # MAPA-02: la mision seguida y las chinchetas (van en la partida)
    src/Game/Codice.cpp  # CODICE-01: lo conocido, desde beats/ficha/flags
    src/Game/Companeros.cpp  # CLASES-03: companeros reclutables por flag
    # H2-06 (Fase H): las cartas de clase en combate. Los ganchos de la carta
    # (effects[].mechanicHook), el catalogo que los junta y la mesa que
    # BattleState consulta cuando el heroe tiene ficha (Game/MesaCombate.h).
    src/Game/GanchosCarta.cpp
    src/Game/CatalogoCartas.cpp
    src/Game/CartasEnCombate.cpp
    src/Game/CartasJugar.cpp
    src/Game/CartasTiradas.cpp
    src/Game/CartasBloqueo.cpp  # AV13-30: el critico contra el bloqueo y la pifia que da ataque de oportunidad
    src/Game/CartasEnemigas.cpp  # AV18-06: los enemigos juegan las cartas nativas de su repertorio
    src/Game/CartasAcompanantes.cpp  # ACOMPANANTES-01: los acompanantes de la historia en la mesa
    src/Game/Refuerzos.cpp  # ROSITA-01: los que llegan a mitad del combate (la compania de Rosita)
    # APARIENCIA-01: personajes por capas (manifiesto, montaje byte a byte
    # como el probador, paso Apariencia de la creacion y equipo inicial
    # puesto). GL-free; la textura la sube Render/HojaPersonaje.
    src/Game/Personajes/ManifiestoPersonajes.cpp
    src/Game/Personajes/MontajePersonajes.cpp
    src/Game/Personajes/MontajeCache.cpp
    src/Game/Personajes/MontajeFilas.cpp
    src/Game/Personajes/BrilloEquipo.cpp
    src/Game/Personajes/FamiliasDeParticulas.cpp  # PART-01: las familias de particulas (equipo y cartas)
    src/Game/Personajes/VisibilidadRasgos.cpp
    src/Game/Personajes/RasgosApariencia.cpp
    src/Game/Personajes/EditorApariencia.cpp
    src/Game/Personajes/Vestuario.cpp
    src/Game/Personajes/KitBasico.cpp  # UI-BG3-01: el kit del cofre de El Vacio, con una regla
    src/Game/CuadernoCreacion.cpp  # UI-BG3-01: la creacion en siete hojas con rejillas (GL-free)
    src/Game/CuadernoCreacionVistas.cpp
    src/Game/CofreKit.cpp  # UI-BG3-01: el cofre de El Vacio (kit a la mochila y sus flags)
    src/Game/BarraCombate.cpp  # UI-BG3-01: la barra de cartas del combate (GL-free)
    src/Game/BarraAcompanante.cpp  # AV13-29: la barra del acompañante que juegas tu
    src/Game/ParticulasDeCartas.cpp  # PART-02d: las particulas de las cartas al jugarse (GL-free)
    src/Game/EstadosCombate.cpp  # H2-03: los estados de cada combatiente, con icono y rondas (GL-free)
    src/Game/BarraObjetivos.cpp  # H2-02: a quien apunta la carta senalada de la barra (GL-free)
    # SPRITES-01 (01/10/2026): que hoja dibuja a cada cosa, desde datos
    # (assets/textures/sprites.json). GL-free; el que sube las texturas es
    # Render/BancoAtlas en motor_core.
    src/Game/RegistroSprites.cpp
    src/Game/Bloques.cpp  # BLOQUES-01: que celda levanta bloque y con que caras (GL-free)
    src/Game/Bandada.cpp  # BLOQUES-02: la bandada de palomas (GL-free)
    src/Game/Mascotes.cpp  # MASC-01: mascotas que pasean, saludan o huyen (GL-free)
    src/Game/FullaMascotes.cpp  # MASC-01: la hoja de mascotas como datos (GL-free)
    src/Game/Monumentos.cpp  # BLOQUES-03: edificios con sprite propio (GL-free)
    src/Game/Cinematica.cpp  # PROLOGO-02: las cinematicas (ordenes, espectros y apariciones; GL-free)
    src/Game/EscenaViva.cpp  # C2-04a: estado vivo de la escena (GL-free)
    src/Game/FuegoCeldas.cpp  # C2-04c: casillas de fuego y semilla (GL-free)
)
# GameSession.cpp usa TileMap (motor_map) para la colision, pero
# motor_map se declara mas abajo en este archivo: en vez de reordenar
# (motor_map depende a su vez de motor_math/tinyxml2), cada ejecutable
# que use GameSession linka ambas librerias -- mismo patron que
# demo_editor_state. Ninguna de las dos trae GLAD/GLFW: siguen siendo
# GL-free.
target_link_libraries(motor_level PUBLIC motor_json motor_project motor_rpg motor_runtime_map motor_world motor_imagen)
include(cmake/Emisor.cmake)  # U4-07: emisor de particulas generico (GL-free)

# H0-05 — Selector de guardados (G-08/G-11) como libreria GL-free propia:
# TitleMenuState y SaveSlotState viven aqui y ya no se compilan sueltos en
# test_title_flow, test_save_slots, demos ni en juego/test de cableado.
# Antes eran fuentes directas en 4 objetivos (deuda 4c). motor_core los
# consume via este enlace.
add_library(motor_saves
    src/Game/TitleMenuState.cpp
    src/Game/SaveSlotState.cpp)
target_include_directories(motor_saves PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(motor_saves PUBLIC
    motor_level motor_project motor_map_document motor_rpg motor_json motor_map)

# PERGAMINO-01 — el tema pergamino sin GL (lo dibuja Render/PergaminoGL).
include(cmake/Pergamino.cmake)

add_executable(demo_level_loader examples/demo_level_loader.cpp)
target_link_libraries(demo_level_loader PRIVATE motor_level)

add_executable(demo_skills examples/demo_skills.cpp)
target_link_libraries(demo_skills PRIVATE motor_level)

# AV13-35: demo_battle (el combate por turnos de la Fase 8, sin mesa de cartas) se borro con el
# combate antiguo.

# ================== NUEVOS: Motor Nd6 Onegai (Fase 0) ==================
# demo_nd6_distribution — distribución de grados + Context Enhancements.
# GL-free. Sin GL, sin ventana. Se ejecuta en línea de comandos para
# validar estadísticas Nd6 (debe mostrar SANE percentages).
add_executable(demo_nd6_distribution examples/demo_nd6_distribution.cpp)
target_link_libraries(demo_nd6_distribution PRIVATE motor_rpg)
set_target_properties(demo_nd6_distribution PROPERTIES CXX_STANDARD 17)
# demo_battle_nd6 — combate Player vs Enemy x50 via ApplyBasicAttackNd6
# (camino Render → SkillExecutor → Nd6). Usa motor_level (Skill.cpp).
add_executable(demo_battle_nd6 examples/demo_battle_nd6.cpp)
target_link_libraries(demo_battle_nd6 PRIVATE motor_level)
set_target_properties(demo_battle_nd6 PROPERTIES CXX_STANDARD 17)
# ====================================================================
# demo_catalogos_rpg — C3: carga 20 RPG Catalog<T> (~2416) + ObjectCatalog
# (~2525) y smoke test de InventoryEngine + ConditionEngine. GL-free.
add_executable(demo_catalogos_rpg examples/demo_catalogos_rpg.cpp)
target_link_libraries(demo_catalogos_rpg PRIVATE motor_level)
set_target_properties(demo_catalogos_rpg PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_catalogos_rpg copiar_assets)

# ====================================================================
# demo_mundo — B24: el mundo de Egaroth (19 naciones, 95 ciudades, 26 zonas)
# desde assets/catalogs/locations.json. El mapa esta fechado en 2000 b.f.
# GL-free, corre en CI.
add_executable(demo_mundo examples/demo_mundo.cpp)
target_link_libraries(demo_mundo PRIVATE motor_level)
set_target_properties(demo_mundo PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_mundo copiar_assets)

# demo_este_sur — tres prototipos regionales aislados (Taurengrad,
# Dhin Thyraxion y Naka't-ol). Verifica JSON de nivel y TMX reales.
add_executable(demo_este_sur examples/demo_este_sur.cpp)
target_link_libraries(demo_este_sur PRIVATE motor_level motor_map)
set_target_properties(demo_este_sur PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_este_sur copiar_assets)

# ====================================================================
# demo_boundington — los tres dias en Boundington (1981 b.f.): el primer dia
# con sus tres cierres, el segundo y el Ocaso (BND-ESCENAS-01). Juega varias
# rutas y comprueba que no queden beats muertos. GL-free, corre en CI.
# BND-ALT-01: las rutas de las alternativas, en su propio fichero.
add_executable(demo_boundington examples/demo_boundington.cpp examples/demo_boundington_alternativas.cpp)
target_link_libraries(demo_boundington PRIVATE motor_level)
set_target_properties(demo_boundington PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_boundington copiar_assets)

# explorar_boundington — DIALOGOS-01: juega la campaña entera cientos de veces
# con héroes de la creación de personaje y estilos (azar/bondadoso/cruel), y
# dice atascos, beats y opciones sin ver, finales y la moral. GL-free.
add_executable(explorar_boundington examples/explorar_boundington.cpp)
target_link_libraries(explorar_boundington PRIVATE motor_level)
set_target_properties(explorar_boundington PROPERTIES CXX_STANDARD 17)
add_dependencies(explorar_boundington copiar_assets)

# Precuela de la Taberna Humilde: rutas de rescate y tragedia persistentes.
add_executable(demo_precuela_boundington examples/demo_precuela_boundington.cpp)
target_link_libraries(demo_precuela_boundington PRIVATE motor_rpg)
set_target_properties(demo_precuela_boundington PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_precuela_boundington copiar_assets)

# demo_narrativa_nd6 — valida el puente NarrativeEngine -> DicePoolEngine:
# un beat con effect "skillCheck" empaqueta la tirada y la resolucion Nd6
# enciende la flag del grado (BOTCH/PARTIAL/SUCCESS/CRITICAL). GL-free.
add_executable(demo_narrativa_nd6 examples/demo_narrativa_nd6.cpp)
target_link_libraries(demo_narrativa_nd6 PRIVATE motor_level motor_map)
set_target_properties(demo_narrativa_nd6 PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_narrativa_nd6 copiar_assets)

# demo_temporizadores — temporizadores de mision F1 (bitacora 2.18):
# tiempo narrativo, avisos por umbral, expiracion por tiempo/salir de zona.
add_executable(demo_temporizadores examples/demo_temporizadores.cpp)
target_link_libraries(demo_temporizadores PRIVATE motor_rpg)
set_target_properties(demo_temporizadores PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_temporizadores copiar_assets)

# demo_prologo — el prologo onirico de Boundington (Cap. 1-2 del canon).
# Reproduce boundington_prologo.json forzando SUCCESS en las dos tiradas
# Nd6 y comprueba el cierre y la cobertura de beats. GL-free.
add_executable(demo_prologo examples/demo_prologo.cpp)
target_link_libraries(demo_prologo PRIVATE motor_level motor_map)
set_target_properties(demo_prologo PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_prologo copiar_assets)

# ====================================================================
# demo_campana_boundington — "Los Perdidos de Boundington" entera: prologo
# + tres dias con UN SOLO NarrativeState. Comprueba los finales y que no
# quede ningun beat muerto (72/72). GL-free, corre en CI.
add_executable(demo_campana_boundington examples/demo_campana_boundington.cpp)
target_link_libraries(demo_campana_boundington PRIVATE motor_level)
set_target_properties(demo_campana_boundington PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_campana_boundington copiar_assets)

# ====================================================================
# demo_narrativa_combate — el efecto "startBattle": un beat pide combate,
# GameSession lo ejecuta con BattleState de verdad y la flag de resultado
# llega DESPUES, al cerrarse la batalla. Prueba las dos ramas y el caso de
# monsterId inexistente. GL-free.
add_executable(demo_narrativa_combate examples/demo_narrativa_combate.cpp)
target_link_libraries(demo_narrativa_combate PRIVATE motor_level motor_map)
set_target_properties(demo_narrativa_combate PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_narrativa_combate copiar_assets)

# ====================================================================
# demo_proyectos — los proyectos vivos: descubrir, revisar y crear. Es el
# nucleo de la pantalla de arranque del editor, GL-free y probado en CI.
# demo_creacion_personaje — de la eleccion a la ficha, con las reglas del GDD.
add_executable(demo_creacion_personaje examples/demo_creacion_personaje.cpp)
target_link_libraries(demo_creacion_personaje PRIVATE motor_level)
set_target_properties(demo_creacion_personaje PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_creacion_personaje copiar_assets)

# demo_plaza_razas — la Plaza de las Razas, recorrida sin abrir ventana.
add_executable(demo_plaza_razas examples/demo_plaza_razas.cpp)
target_link_libraries(demo_plaza_razas PRIVATE motor_level)
set_target_properties(demo_plaza_razas PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_plaza_razas copiar_assets)

add_executable(demo_proyectos examples/demo_proyectos.cpp)
target_link_libraries(demo_proyectos PRIVATE motor_level)
set_target_properties(demo_proyectos PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_proyectos copiar_assets)

# Fase 10: catalogo generico de objetos ("todo es un objeto") + accion
# Item en BattleState + ObjectSpawn en LevelDefinition. GL-free entero.
# Lee assets/objects/test_objects.json y assets/levels/test_level.json
# (copiados junto al binario por el file(COPY assets ...) de arriba).
add_executable(demo_object_catalog examples/demo_object_catalog.cpp)
target_link_libraries(demo_object_catalog PRIVATE motor_level)

# Fase 12: nucleo del editor de niveles (EditorState), GL-free. Los
# round-trips de exportacion se prueban contra el TileMap/LevelLoader
# REALES, por eso linka tambien motor_map (GL-free, ver arriba).
add_executable(demo_editor_state examples/demo_editor_state.cpp)
target_link_libraries(demo_editor_state PRIVATE motor_level motor_map)

# El panel de propiedades de un objeto colocado (ObjectInspector):
# GL-free como el resto del nucleo del editor. Prueba la edicion por
# teclado (texto, escala, salidas, botin, eventos, propiedades) y el
# round-trip de los campos nuevos del spawn contra LevelLoader.
add_executable(demo_object_inspector examples/demo_object_inspector.cpp)
target_link_libraries(demo_object_inspector PRIVATE motor_level motor_map)
set_target_properties(demo_object_inspector PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_object_inspector copiar_assets)

# Cierre del ciclo de juego: GameSession (exploracion <-> combate). Como
# demo_editor_state, linka tambien motor_map (colision contra el TileMap
# real) y sigue siendo GL-free: prueba una partida entera sin ventana.
add_executable(demo_game_session examples/demo_game_session.cpp)
target_link_libraries(demo_game_session PRIVATE motor_level motor_map)

# Ciclo social/comercial de la ciudad (NPCs, tiendas, puertas entre
# mapas). GL-free igual que demo_game_session: se ejecuta en CI.
add_executable(demo_ciudad examples/demo_ciudad.cpp)
target_link_libraries(demo_ciudad PRIVATE motor_level motor_map)

# Linea de eventos / narrativa (P0-13): NarrativeEngine + una aventura
# real de 3 PNJs resuelta en las 6 permutaciones de orden, primero con el
# motor aislado y luego dentro de una GameSession sobre ciudad_centro.
# GL-free, se ejecuta en CI como los demas demos-test.
add_executable(demo_aventura examples/demo_aventura.cpp)
target_link_libraries(demo_aventura PRIVATE motor_level motor_map)

# G-08.1: demo GL-free de carrega de manifests per TitleMenuState. Compila
# TitleMenuState.cpp i SaveSlotState.cpp directament (mateix patro que
# cmake/TitleFlowTests.cmake), sense GL. Sense tocar Application.cpp (cruce).
add_executable(demo_title_menu
    examples/demo_title_menu.cpp
    src/Game/TitleMenuState.cpp
    src/Game/SaveSlotState.cpp)
target_include_directories(demo_title_menu PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(demo_title_menu PRIVATE
    motor_level motor_project motor_map_document motor_rpg motor_json motor_map)
set_target_properties(demo_title_menu PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_title_menu copiar_assets)

# G-07.1: verificacio GL-free de la congercio de serializers (aventura + nivell)
# sobre assets reals i fixtures congelades. Sense GL, nomes motor_json + motor_rpg + motor_level.
add_executable(demo_congelacio examples/demo_congelacio.cpp)
target_link_libraries(demo_congelacio PRIVATE motor_level)
set_target_properties(demo_congelacio PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_congelacio copiar_assets)

# G-09.1: demo GL-free d'integracio "Nova partida" (manifest -> NewGame ->
# level -> character -> saves). Sense tocar Application.cpp (cruce G-08),
# GameSession.cpp (G-12) ni files d'altres agents. Compila TitleMenuState.cpp
# + SaveSlotState.cpp directament (com demo_title_menu).
add_executable(demo_nova_partida
    examples/demo_nova_partida.cpp
    src/Game/TitleMenuState.cpp
    src/Game/SaveSlotState.cpp)
target_include_directories(demo_nova_partida PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(demo_nova_partida PRIVATE
    motor_level motor_project motor_map_document motor_rpg motor_json motor_map)
set_target_properties(demo_nova_partida PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_nova_partida copiar_assets)

# Fractura #1: EnemyBrain, la logica del enemigo (vida + patrulla + IA)
# separada del sprite. GL-free puro -- ni siquiera necesita motor_map.
add_executable(demo_enemy_brain examples/demo_enemy_brain.cpp)
target_link_libraries(demo_enemy_brain PRIVATE motor_level)

# Test del motor de tiradas Nd6 con dados GUIONIZADOS (examples/
# ScriptedRng.h): grados, ventaja/desventaja, critico, pool vacio, techo
# de dados y reproducibilidad de la semilla. No confundir con
# demo_nd6_distribution, que imprime porcentajes y NO puede fallar: este
# usa require() y aborta. Solo necesita headers de RPG/ -> no enlaza
# ninguna libreria.
add_executable(demo_dice_engine examples/demo_dice_engine.cpp)

# GLAD y stb_image estan vendorizados en third_party/ (comprobados en el
# repositorio, no son un paso manual): GLAD no tiene soporte CMake
# oficial tipo FetchContent (requiere un paso de generacion desde su XML
# de especificacion) y stb_image es header-only. Se generaron una vez
# con:
#
#   pip install glad
#   python -m glad --profile core --api "gl=3.3" --generator c \
#       --out-path third_party/glad
#   curl -o third_party/stb/stb_image.h \
#       https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
#
# (el propio proyecto necesita un .cpp que haga
#  #define STB_IMAGE_IMPLEMENTATION antes de incluir stb_image.h una
#  unica vez; ver stb_image_impl.cpp mas abajo). Los "if EXISTS" de abajo
# se dejan como red de seguridad (por si alguien borra third_party/ a
# mano), no como paso esperado.
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/third_party/glad)
set(STB_DIR ${CMAKE_SOURCE_DIR}/third_party/stb)

# --- Fase 2: TileMap/Tile/TextureAtlas (mapas y tiles). Solo necesitan
# tinyxml2 (vendorizado en third_party/, igual que GLAD/stb_image: XML de
# un solo .h/.cpp, mas simple vendorizarlo que un FetchContent) para
# parsear TMX, y motor_math (Camera, para TileMap::visibleRange). Nada de
# GLAD ni GLFW: TextureAtlas solo usa los getters inline de Texture.h
# (ancho/alto), no su implementacion .cpp (que si necesita GLAD). Por eso
# se pueden compilar y probar sin X11 ni Xvfb (ver demo_tilemap.cpp). ---
set(TINYXML2_DIR ${CMAKE_SOURCE_DIR}/third_party/tinyxml2)

add_library(motor_tmx_import
    src/Import/TmxMapImporter.cpp
    ${TINYXML2_DIR}/tinyxml2.cpp
)
target_include_directories(motor_tmx_import PUBLIC ${TINYXML2_DIR})
target_link_libraries(motor_tmx_import PUBLIC motor_map_document)

add_library(motor_map
    src/Core/Resources/TextureAtlas.cpp
    src/Render/TileMap.cpp
)
target_link_libraries(motor_map PUBLIC motor_runtime_map motor_tmx_import)

if (EXISTS ${GLAD_DIR}/src/glad.c)
    # demo_tilemap.cpp SI construye un Texture real (aunque con glID=0,
    # ver su comentario), asi que el ejecutable -- no motor_map -- es
    # quien necesita Texture.cpp/glad.c. Mantenerlos fuera de motor_map
    # evita compilarlos dos veces si algo linka motor_map junto a
    # motor_core (que ya trae su propia copia), lo que daria simbolos
    # duplicados en el enlazado.
    add_executable(demo_tilemap
        examples/demo_tilemap.cpp
        src/Core/Resources/Texture.cpp
        ${GLAD_DIR}/src/glad.c
    )
    target_include_directories(demo_tilemap PRIVATE ${GLAD_DIR}/include)
    target_link_libraries(demo_tilemap PRIVATE motor_map)
    if (WIN32)
        # glad.c resuelve las funciones OpenGL mediante opengl32 en Windows.
        target_link_libraries(demo_tilemap PRIVATE opengl32)
    endif()

    # Fase 9: BitmapFont, pero SOLO sus dos metodos estaticos y puros
    # (glyphUV()/measureText(), ver el comentario de BitmapFont.h) -- este
    # demo nunca construye un BitmapFont de verdad (eso llamaria a
    # generateAtlas(), que hace GL real y necesita un contexto ya
    # inicializado). Se enlaza contra BitmapFont.cpp + Texture.cpp +
    # SpriteBatch.cpp + glad.c solo porque el enlazador debe resolver esos
    # simbolos (referenciados por BitmapFont.cpp aunque nunca se ejecuten
    # en este binario) -- mismo patron que demo_tilemap de arriba, sin
    # necesitar Window/GLFW: ningun simbolo de glad se LLAMA de verdad en
    # este binario, asi que compila, enlaza Y EJECUTA sin contexto GL real.
    add_executable(demo_bitmap_font
        examples/demo_bitmap_font.cpp
        src/Render/BitmapFont.cpp
        src/Core/Resources/Texture.cpp
        src/Render/SpriteBatch.cpp
        ${GLAD_DIR}/src/glad.c
    )
    target_include_directories(demo_bitmap_font PRIVATE ${GLAD_DIR}/include)

    # Fractura #3 (ARCHITECTURE.md): HudManager::update() debe alcanzar a
    # TODOS los widgets. Mismo patron de enlazado que demo_bitmap_font --
    # enlaza SpriteBatch/Texture/Shader/glad solo para resolver los
    # simbolos de render(), que este binario nunca llama: prueba
    # aritmetica pura de HudBar::update sin contexto GL.
    add_executable(demo_hud_update
        examples/demo_hud_update.cpp
        src/Render/HudWidgets.cpp
        src/Render/HudElement.cpp
        src/Render/HudManager.cpp
        src/Render/SpriteBatch.cpp
        src/Core/Resources/Texture.cpp
        src/Core/Resources/Shader.cpp
        ${GLAD_DIR}/src/glad.c
    )
    target_include_directories(demo_hud_update PRIVATE ${GLAD_DIR}/include)
    # Shader.cpp (compilado arriba) incluye <glm/fwd.hpp>: glm es header-only
    # y se trae via FetchContent mas arriba. Sin este link, el build falla
    # con 'glm/fwd.hpp file not found' (bug preexistente: el target se anadio
    # sin enlazar glm cuando empezo a compilar Shader.cpp).
    target_link_libraries(demo_hud_update PRIVATE glm::glm)

    # Fractura #1: Enemy delegando en EnemyBrain, test de no regresion de
    # la patrulla. Mismo patron de enlazado que demo_bitmap_font (glad
    # enlazado pero nunca llamado: atlas nullptr, no se dibuja).
    add_executable(demo_enemy_sprite
        examples/demo_enemy_sprite.cpp
        src/Render/Enemy.cpp
        src/Game/EnemyBrain.cpp
        src/Render/AnimatedEntity.cpp
        src/Render/Entity.cpp
        src/Render/SpriteBatch.cpp
        src/Core/Resources/Texture.cpp
        src/Core/Resources/TextureAtlas.cpp
        ${GLAD_DIR}/src/glad.c
    )
    target_include_directories(demo_enemy_sprite PRIVATE ${GLAD_DIR}/include)
else ()
    message(STATUS "demo_tilemap / demo_bitmap_font / demo_hud_update omitidos: falta third_party/glad.")
endif ()

if (EXISTS ${GLAD_DIR}/src/glad.c AND EXISTS ${STB_DIR}/stb_image.h)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    if (UNIX AND NOT APPLE)
        # Solo X11: evita depender de wayland-client/xkbcommon (no
        # instalados por defecto en CI) cuando solo hace falta X11/Xvfb.
        set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    endif ()
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)

    # third_party/stb_image_impl.cpp: unico .cpp del proyecto donde se
    # define STB_IMAGE_IMPLEMENTATION (stb_image.h es header-only y esa
    # macro debe aparecer en una unica unidad de compilacion).
    file(WRITE ${CMAKE_BINARY_DIR}/stb_image_impl.cpp
        "#define STB_IMAGE_IMPLEMENTATION\n#include <stb_image.h>\n")

    add_library(motor_core
        src/Core/Resources/Texture.cpp
        src/Core/Resources/TextureManager.cpp
        src/Core/Resources/Shader.cpp
        src/Core/Resources/ShaderManager.cpp
        src/Engine/Window.cpp
        src/Render/SpriteBatch.cpp
        src/Render/Entity.cpp
        src/Render/AnimatedEntity.cpp
        src/Render/Player.cpp
        src/Render/Enemy.cpp
        src/Game/EnemyBrain.cpp
        src/Game/TitleScreen.cpp
        src/Render/IsometricRenderer.cpp
        src/Render/Framebuffer.cpp
        src/Render/FogOfWar.cpp
        src/Render/ColorLUT.cpp
        src/Render/HudElement.cpp
        src/Render/HudManager.cpp
        src/Render/HudWidgets.cpp
        src/Render/BitmapFont.cpp
        src/Render/HudTextWidgets.cpp
        src/Render/HudIconos.cpp  # FICHA-01: atlas de iconos del HUD
        src/Render/HudIconList.cpp  # MISIONES-01: lista con iconos (tienda)
        src/Engine/PantallaNuevaPartida.cpp  # NUEVA-PARTIDA-01: creacion de personaje
        # PERGAMINO-01 — el unico que dibuja el tema pergamino (motor_pergamino,
        # sin GL, lo describe en lienzos) y el aura de la moral del jugador.
        # Sustituyen a HudMinimap, HudFicha y ApplicationMenuJugador.
        src/Render/PergaminoGL.cpp
        src/Render/AuraJugador.cpp
        src/Render/FuegoNivel.cpp  # C2-04c: el fuego de arde/apaga
        src/Render/PintaDestellos.cpp  # PART-02d: las particulas sobre un sprite y las cartas jugadas
        # SPRITES-01 — las hojas de sprites del registro, subidas y recortadas.
        src/Render/BancoAtlas.cpp
        src/Render/BloquesNivel.cpp  # BLOQUES-01: los bloques del nivel en la cola del renderer
        src/Render/VidaNivel.cpp  # BLOQUES-02 + MASC-01: la capa de vida (palomas y mascotas)
        src/Render/PolvoDePasos.cpp  # U4-07.E07: el polvo de pasos del jugador (GL)
        src/Render/HojaPersonaje.cpp  # APARIENCIA-01: hoja montada en la GPU + vista previa
        src/Render/DynamicLightMap.cpp
        src/Render/BlobShadow.cpp
        src/Engine/Application.cpp
        # H1-05 — Locale ya NO esta aqui: vive en motor_i18n (GL-free),
        # enlazada mas abajo. Ponerla en motor_core hacia que `juego`
        # enlazara, pero atrapaba una fuente sin OpenGL en una libreria
        # con OpenGL.
        # 24/09/2026 (EST-20, relevo A) — Application.cpp tenia 2493 lineas
        # y 14 paquetes encima. Partido por FLUJO: el bucle del fotograma,
        # las pantallas superpuestas y el par abrir-proyecto/cerrar. Lo que
        # comparten dos de ellos vive en include/Engine/ApplicationInternal.h
        # (cabecera sola: dos funciones de tres lineas no pagan un .cpp, y uno
        # sin GL aqui habria empeorado la mezcla de orillas de motor_core).
        src/Engine/ApplicationBucle.cpp
        src/Engine/ApplicationPantallas.cpp
        src/Engine/ApplicationCierre.cpp
        # 24/09/2026 (EST-20, relevo B) — el refresco del HUD y la carga de
        # nivel, los dos flujos largos que quedaban.
        src/Engine/ApplicationHud.cpp
        src/Engine/ApplicationNivel.cpp
        src/Engine/ApplicationCinematica.cpp  # PROLOGO-02: las cinematicas en el juego
        src/Engine/ApplicationCinematicaEscena.cpp  # C2-04b: la escena viva en el juego
        src/Engine/ApplicationCinematicaTitulo.cpp
        src/Engine/ApplicationCamara.cpp  # U4-01.E06
        ${GLAD_DIR}/src/glad.c
        ${CMAKE_BINARY_DIR}/stb_image_impl.cpp
    )
    target_include_directories(motor_core PUBLIC
        ${GLAD_DIR}/include
        ${STB_DIR}
    )
    # motor_map (TileMap/TextureAtlas) publico: IsometricRenderer.h los
    # expone en su propia interfaz publica, y no trae GLAD/GLFW consigo
    # (ver comentario mas arriba), asi que no reintroduce ninguna
    # dependencia nueva sobre motor_core.
    target_link_libraries(motor_core PUBLIC glfw glm::glm motor_map motor_engine_loop motor_input motor_guion_captura motor_scene motor_saves motor_captura_gl motor_i18n motor_pergamino)

    add_executable(sandbox_window examples/sandbox_window.cpp)
    target_link_libraries(sandbox_window PRIVATE motor_core)

    # Fase 1, ultima tarea: quad texturizado con GL_NEAREST. Usa Window +
    # Texture/Shader (motor_core) y Camera (motor_math) a la vez.
    add_executable(demo_textured_quad examples/demo_textured_quad.cpp)
    target_link_libraries(demo_textured_quad PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 2 (culling) + Fase 3 (arranque): IsometricRenderer dibujando un
    # TileMap culled + un par de entidades de prueba, con Painter's
    # Algorithm real.
    add_executable(demo_isometric_renderer examples/demo_isometric_renderer.cpp)
    target_link_libraries(demo_isometric_renderer PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 3 (cierre): AnimatedEntity/Player/Enemy sobre el mismo pipeline
    # (TileMap + IsometricRenderer) que demo_isometric_renderer, con
    # contexto OpenGL real.
    add_executable(demo_animated_entity examples/demo_animated_entity.cpp)
    target_link_libraries(demo_animated_entity PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 4 (arranque): post-procesado + iluminacion. Mismo pipeline que
    # demo_animated_entity, pero activando el post-FX de IsometricRenderer
    # (Framebuffer + fullscreen-triangle + shader lightmap) con un lightmap
    # procedural.
    add_executable(demo_lighting examples/demo_lighting.cpp)
    target_link_libraries(demo_lighting PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 4 (2o efecto): niebla de guerra con memoria de exploracion.
    # Mismo pipeline, pero activando el post-FX de niebla (shader fog +
    # textura de visibilidad de FogOfWar) que revela un radio alrededor del
    # Player cada frame.
    add_executable(demo_fog examples/demo_fog.cpp)
    target_link_libraries(demo_fog PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 4 (3er efecto): paletizado / LUT de color. Mismo pipeline, pero
    # activando el post-FX de LUT (shader lut + ColorLUT, cubo RGB
    # cuantizado a "levels" tonos por canal) para un look de paleta
    # limitada, estilo pixel art.
    add_executable(demo_lut examples/demo_lut.cpp)
    target_link_libraries(demo_lut PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 9 de motor_grafico_gantt_rpg.puml: HUD modular. Mismo pipeline
    # de mundo que demo_lighting/demo_fog/demo_lut, con una SpriteBatch y
    # un HudManager dibujando HudBar/HudPanel encima (reutilizando el
    # shader "sprite", ver el comentario de HudManager::render()).
    add_executable(demo_hud examples/demo_hud.cpp)
    target_link_libraries(demo_hud PRIVATE motor_core motor_math motor_captura_gl)

    # Fase 9 (cierre): demo integrador -- BattleState (Fase 8) resuelto de
    # verdad y volcado en HudCommandMenu/HudDialogueBox (widgets de texto
    # nuevos) + HudBar de vida, todo con BitmapFont::generateAtlas() real
    # (a diferencia de demo_bitmap_font.cpp, este SI construye un
    # BitmapFont). Sin mundo isometrico (no hace falta TileMap/
    # IsometricRenderer para probar el HUD de combate). Depende de
    # motor_level (BattleState/Skill) ademas de motor_core/motor_math.
    add_executable(demo_battle_hud examples/demo_battle_hud.cpp)
    target_link_libraries(demo_battle_hud PRIVATE motor_core motor_math motor_level)

    # Fase 11: iluminacion avanzada + sombras. Mismo pipeline y shader
    # "lightmap" que demo_lighting, pero con DynamicLightMap (N point
    # lights animadas, compuestas en CPU por frame) y sombras blob
    # (CreateBlobShadowTexture + setBlobShadows) bajo Player/Enemy.
    add_executable(demo_dynamic_lights examples/demo_dynamic_lights.cpp)
    target_link_libraries(demo_dynamic_lights PRIVATE motor_core motor_math motor_captura_gl)

    # ESTETICA-01: el brillo del equipo por lo fuerte que es (estilo NosTale),
    # con HojaPersonaje + AuraJugador sobre el IsometricRenderer de verdad.
    add_executable(demo_brillo_equipo examples/demo_brillo_equipo.cpp)
    target_link_libraries(demo_brillo_equipo PRIVATE motor_core motor_math motor_level motor_captura_gl)

    # Fase 12: editor de niveles visual. UI con el HUD propio (Fase 9),
    # no con Dear ImGui (sin red para vendorizarlo en el entorno de
    # desarrollo, ver EditorState.h); el nucleo GL-free ya esta verificado
    # en demo_editor_state. Usa GLFW directamente (raton) via
    # Window::handle(), ademas de motor_core/motor_math/motor_level.
    add_executable(level_editor editor/src/level_editor.cpp)
    target_link_libraries(level_editor PRIVATE motor_core motor_math motor_level motor_captura_gl)
    # El editor consume sus iconos de previsualización desde assets/:
    # sincronizarlos en cada build evita que un asset nuevo falle solo
    # cuando el ejecutable se lanza desde build/.
    add_dependencies(level_editor copiar_assets)

    # El juego: Application (bucle completo) sobre GameSession. Cierra el
    # motor de punta a punta -- Fases 1-4 del Gantt original + 6-12 del
    # RPG en una partida jugable. motor_core ya arrastra motor_map
    # (PUBLIC), que es lo que GameSession necesita para la colision.
    add_executable(juego examples/juego.cpp)
    target_link_libraries(juego PRIVATE motor_core motor_math motor_level motor_saves)
else ()
    message(STATUS
        "motor_core / sandbox_window omitidos: falta third_party/glad y/o "
        "third_party/stb (ver instrucciones en CMakeLists.txt). "
        "demo_resource_manager sigue disponible sin ellos.")
endif ()

# demo_este_norte — el cuadrante Este-Norte de Egaroth (experimento 4-IAs,
# ZCode): 22 asentimientos + 33 interiores en Choubar, Gongorguma,
# Mistarium y Nocturnsea. Carga todo lo generado desde el manifiesto y
# comprueba puertas y fichas de catalogo. GL-free, corre en CI.
add_executable(demo_este_norte examples/demo_este_norte.cpp)
target_link_libraries(demo_este_norte PRIVATE motor_level)
set_target_properties(demo_este_norte PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_este_norte copiar_assets)

# demo_senal_ceniza — "La Senal de la Ceniza": el espejo oriental de la
# campana de Boundington, ano 2000 b.f. (EV-2000-CRUZADA01E). Cuatro rutas
# (ojos abiertos / medio / ciego / criticos) y cobertura de beats. GL-free.
add_executable(demo_senal_ceniza examples/demo_senal_ceniza.cpp)
target_link_libraries(demo_senal_ceniza PRIVATE motor_level motor_map)
set_target_properties(demo_senal_ceniza PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_senal_ceniza copiar_assets)

if(BUILD_TESTING)
    # C-20 — Regresiones del propietario unico de las condiciones.
    include(cmake/ConditionEngineTests.cmake)

    # C-22 — Comprobador estático de declaraciones sin definición en include/RPG.
    include(cmake/DeclarationCheck.cmake)

    # C-15 — addLevelToProject: la funcion que vivio declarada sin cuerpo.
    # Fragmento aparte, como el resto de tests nuevos: evita que varios
    # agentes se pisen editando el mismo bloque de este fichero.
    include(cmake/ProjectLevelsTests.cmake)
    # E-02 — variables narrativas e interruptores locales A-D.
    include(cmake/NarrativeVariablesTests.cmake)
    include(cmake/PuertaCerradaTests.cmake)  # G4-01: puertas cerradas por flags
    # C8-03b — catálogo de eventos del mundo (lectura y validación).
    include(cmake/EventosDelMundoTests.cmake)
    include(cmake/EscenaVivaTests.cmake)  # C2-04a: posturas, fuego y emociones
    include(cmake/PosicionBocadilloTests.cmake)  # C2-04d: posicion del bocadillo
    include(cmake/FuegoNivelTests.cmake)  # C2-04c: casillas de fuego y semilla
    # E-03 — planificador de eventos (esperas, paralelos, orden determinista).
    include(cmake/EventInterpreterTests.cmake)
    # E-04 — juego de comandas y su integracion con el planificador.
    include(cmake/EventCommandsTests.cmake)
    # E-07 — validacion estatica de eventos.
    include(cmake/AdventureValidationTests.cmake)
    include(cmake/OpcionesDialogoTests.cmake)  # OPCIONES-01 / VARIABLES-01
    include(cmake/FaseUnoBoundingtonTests.cmake)  # FASE1-BND-01: franjas, tabla, descanso, huir y sin matar
    include(cmake/MoralTests.cmake)  # MORAL-01: la moral de varios factores
    include(cmake/ReputacionAventuraTests.cmake)  # BND-04: requires.reputacion
    include(cmake/HeroeEnLaHistoria.cmake)  # HEROE-01: la historia sabe quien es el heroe creado
    include(cmake/Charlas.cmake)  # DIALOGOS-01: lugar, "sigue" y las paginas de charla
    include(cmake/Cinematica.cmake)  # PROLOGO-02: las cinematicas del prologo
    include(cmake/RumboNiveles.cmake)  # MAPA-01: el objetivo en el minimapa
    include(cmake/TableroCombate.cmake)  # AV13-06: el tablero del combate a lo BG3
    include(cmake/RutaCasillas.cmake)  # AV13-07: la ruta por casillas
    include(cmake/RegionTriggersTests.cmake)  # U3-02.E02: pertenencia de casilla a regiones
    include(cmake/PlantillaArea.cmake)  # AV13-08 (ChatGPT): las plantillas de area
    include(cmake/Contenedores.cmake)
include(cmake/Botin.cmake)
include(cmake/SimularEconomia.cmake)
include(cmake/MonstruosAcciones.cmake)  # AV18-02: datos, persistencia y papiro de botín
    include(cmake/Lugares.cmake)  # AV15-02b (ChatGPT): los sitios de Boundington por su nombre
    include(cmake/OrdenDeTurno.cmake)  # AV13-11: el orden de turno por iniciativa
    include(cmake/EfectosDeGolpe.cmake)  # AV14-02: el golpe en el cuerpo y en la hoja
    include(cmake/Cicatrices.cmake)  # AV13-33: las cicatrices que deja morir
    include(cmake/EscenaNoqueo.cmake)  # AV13-33: la escena de noqueo o rescate
    include(cmake/FrasesDeCombate.cmake)  # EH-16: las tiradas en palabras
    include(cmake/GuiaMapa.cmake)  # MAPA-02: seguir una mision, la flecha y las chinchetas
    # E-08 — depuracion paso a paso (pausa, puntos de parada, traza).
    include(cmake/EventDebugTests.cmake)
    # E-04 (2a rebanada) — familias de comando que necesitan la partida.
    include(cmake/SessionCommandHostTests.cmake)
    # H0-08 (Fase H) — MapEventRunner: los cuatro disparadores conducidos por
    # casillas, jugados sobre el asset publicado assets/misiones/mision_llave.
    include(cmake/MapEventRunnerTests.cmake)
    include(cmake/RegionTriggerDispatchTests.cmake)  # U3-02.E06b
    # H6-01 (Fase H) — AudioDirector: la politica de sonido, probada contra un
    # doble de AudioEngine (sin dispositivo, sin assets, corre en CI).
    include(cmake/AudioDirectorTests.cmake)
    # H3-01 (Fase H) — WorldTravelState: viajar entre naciones, con la ruta
    # Boundington -> mapamundi -> otra nacion -> vuelta sobre los assets reales.
    include(cmake/WorldTravelStateTests.cmake)
    # H3-02 (Fase H) — el diario de misiones: contrato QuestDefinition +
    # cargador + progreso, sobre el catalogo publicado assets/catalogs/quests.json
    # y las referencias de adventures.json.
    include(cmake/QuestLogTests.cmake)
    # H6-03 (Fase H) — OptionsFile: volumen por bus e idioma que sobreviven a
    # salir y entrar, con la politica de un fichero roto o a medias.
    include(cmake/OptionsFileTests.cmake)
    # H3-08 (Fase H) — DefeatRecovery + GameOverScreen::create(plan).
    include(cmake/DefeatRecoveryTests.cmake)
    # H2-01 (Fase H) — el dado se ve: caras, exitos, grado y marca de pifia.
    include(cmake/DiceRollWidgetTests.cmake)
    # H2-03 (Fase H) — condiciones en el HUD, con rondas restantes.
    include(cmake/ConditionWidgetTests.cmake)
    # H3-06 (Fase H) — temporizadores de mision visibles en el HUD.
    include(cmake/MissionTimerWidgetTests.cmake)
    # E-15 — Lista de problemas y busqueda de eventos (sin interfaz).
    include(cmake/ProblemsStateTests.cmake)
    # G-17 — Idioma seleccionable de la interfaz (nucleo, sin pantalla).
    include(cmake/LocaleTests.cmake)
    # G-17 — Complemento 6 tests (3 idiomas publicados / placeholders /
    # hot-reload / DEC-15 ids sin traducir / nombres / fallback+orphan).
    include(cmake/G17LocaleExtensionTests.cmake)
    # E-09 — La mision de la llave: prueba de aceptacion de la fase E.
    include(cmake/MisionLlaveTests.cmake)
    # G-07 (mecanismo) — los ficheros de ayer se siguen abriendo hoy.
    include(cmake/SchemaFixtureTests.cmake)
    # E-11 — nombres visibles, ids generados y duplicar proyectos.
    include(cmake/ProjectNamingTests.cmake)
    # H4-01 (Fase H) — la pantalla de proyectos: duplicar y borrar desde el
    # hub (test_project_hub). Primera prueba automatica de ProjectHub.
    include(cmake/ProjectHubTests.cmake)
    # G-11 — ranuras de guardado por proyecto y puntos seguros (DEC-09).
    include(cmake/SaveSlotsTests.cmake)
    # G-03 — historial por comandas del editor (agente anterior).
    include(cmake/EditorStateHistoryTests.cmake)
    # G-09 — creacion de personaje dentro de Nueva partida (agente anterior).
    include(cmake/CharacterCreationFlowTests.cmake)
    # H1-04 (Fase H) — Game::NewGameFlow: el flujo de Nueva partida con la
    # ficha recalculada (test_h104_new_game_flow).
    include(cmake/H104NewGameFlowTests.cmake)
    include(cmake/ProjectManifestTests.cmake)

    # INF-01.1 — Registro inicial de los demos GL-free verificados en la línea
    # base. Las pruebas se ejecutan desde el directorio de build porque sus rutas
    # de assets son relativas a ese cwd. Se añaden más demos en INF-01.2/01.3 al
    # convertir sus aserciones y fixtures en tests por dominio.
# RUN-11.3 — Cada test registrado se apunta en una propiedad global, y
    # de ella sale el target agregado "suite_gl_free" (al final del
    # fichero). CI construye ESE target y no una lista de nombres escrita a
    # mano en el YAML: anadir un test aqui ya no obliga a acordarse de
    # tocar tambien el workflow, que era como se quedaron fuera de la
    # compilacion de CI test_project y compania.
    function(add_motor_gl_free_demo_test target)
        if(TARGET ${target})
            add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
            set_tests_properties(${target} PROPERTIES
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                LABELS "gl-free;smoke"
            )
            set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS ${target})
        endif()
    endfunction()

    function(add_motor_unit_test target source)
        add_executable(${target} ${source})
        target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
        set_tests_properties(${target} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            LABELS "unit;gl-free"
        )
        set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS ${target})
    endfunction()

    add_motor_gl_free_demo_test(demo_resource_manager)
    add_motor_gl_free_demo_test(demo_handle_pool)
    add_motor_gl_free_demo_test(demo_occlusion_rules)
    add_motor_gl_free_demo_test(demo_camera)
    add_motor_gl_free_demo_test(demo_tilemap)
    add_motor_gl_free_demo_test(demo_editor_state)

    # RUN-11.3 — El resto de demos GL-free, promovidos a CTest tras
    # comprobar uno a uno que salen con codigo 0 y que dos ejecuciones
    # seguidas dan EXACTAMENTE la misma salida (deterministas: los que
    # tiran dados usan ScriptedRng o semilla fija). Cubren el vertical
    # slice entero -- carga de niveles, catalogos, combate Nd6, narrativa,
    # temporizadores, mundo y las campanas completas -- que hasta ahora
    # solo se verificaba ejecutandolos a mano.
    #
    # Todos leen assets/ con rutas relativas al cwd, y por eso el
    # WORKING_DIRECTORY de add_motor_gl_free_demo_test es el de build.
    add_motor_gl_free_demo_test(demo_level_loader)
    add_motor_gl_free_demo_test(demo_object_catalog)
    add_motor_gl_free_demo_test(demo_catalogos_rpg)
    add_motor_gl_free_demo_test(demo_skills)
    add_motor_gl_free_demo_test(demo_battle_nd6)
    add_motor_gl_free_demo_test(demo_dice_engine)
    add_motor_gl_free_demo_test(demo_nd6_distribution)
    add_motor_gl_free_demo_test(demo_enemy_brain)
    add_motor_gl_free_demo_test(demo_game_session)
    add_motor_gl_free_demo_test(demo_object_inspector)
    add_motor_gl_free_demo_test(demo_creacion_personaje)
    add_motor_gl_free_demo_test(demo_mundo)
    add_motor_gl_free_demo_test(demo_ciudad)
    add_motor_gl_free_demo_test(demo_aventura)
    add_motor_gl_free_demo_test(demo_title_menu)
    add_motor_gl_free_demo_test(demo_congelacio)
    add_motor_gl_free_demo_test(demo_nova_partida)
    add_motor_gl_free_demo_test(demo_temporizadores)
    add_motor_gl_free_demo_test(demo_narrativa_nd6)
    add_motor_gl_free_demo_test(demo_narrativa_combate)
    add_motor_gl_free_demo_test(demo_prologo)
    add_motor_gl_free_demo_test(demo_boundington)
    add_motor_gl_free_demo_test(demo_precuela_boundington)
    add_motor_gl_free_demo_test(demo_campana_boundington)
    add_motor_gl_free_demo_test(demo_senal_ceniza)
    add_motor_gl_free_demo_test(demo_este_norte)
    add_motor_gl_free_demo_test(demo_este_sur)

    # Auditoria de cobertura (2026-09-03): de las 47 fuentes de produccion
    # que enlaza `juego`, 23 no las tocaba ni un test ni una demo de CTest
    # -- entre ellas TODOS los widgets del HUD y toda la cadena
    # Entity/AnimatedEntity/Enemy. Estas tres demos ya existian, ya son
    # GL-free (enlazan glad pero no llaman a ninguna funcion GL, ver el
    # comentario de demo_bitmap_font mas arriba) y estaban construyendose
    # sin que nadie las ejecutase.
    #
    # Promovidas con el mismo criterio que RUN-11.3: comprobado una a una
    # que salen con codigo 0 SIN contexto GL y que dos ejecuciones seguidas
    # dan exactamente la misma salida. Cubren HudElement/HudManager/
    # HudWidgets, Entity/AnimatedEntity/Enemy y TextureAtlas, que hasta
    # ahora solo se probaban a mano.
    add_motor_gl_free_demo_test(demo_bitmap_font)
    add_motor_gl_free_demo_test(demo_hud_update)
    add_motor_gl_free_demo_test(demo_enemy_sprite)

    # demo_proyectos NO entra: ademas de assets/ necesita tools/
    # build_proyecto.py en el cwd (compila un proyecto de verdad desde la
    # pantalla del hub), y CMake solo copia assets/ al directorio de build.
    # Promoverlo pide antes decidir si tools/ se copia tambien -- es
    # TOOL-09 (empaquetado), no esto.

    # INF-01.2 — Primer test real bajo tests/. HandlePool es header-only y
    # no necesita assets ni OpenGL, por lo que valida el runner nuevo con el
    # menor acoplamiento posible.
    add_motor_unit_test(test_handle_pool tests/Core/HandlePoolTests.cpp)

    add_motor_unit_test(test_strong_id tests/Core/StrongIdTests.cpp)

    # RUN-01 — Smoke determinista del ciclo eventos -> input -> updates fijos
    # -> render. No depende de GLFW ni de lógica de juego.
    add_motor_unit_test(test_application_loop tests/Engine/ApplicationLoopTests.cpp)
    target_link_libraries(test_application_loop PRIVATE motor_engine_loop)

    add_motor_unit_test(test_input_system tests/Engine/InputSystemTests.cpp)
    target_link_libraries(test_input_system PRIVATE motor_input)

    add_motor_unit_test(test_scene_manager tests/Engine/SceneManagerTests.cpp)
    target_link_libraries(test_scene_manager PRIVATE motor_scene)

    add_motor_unit_test(test_world tests/World/WorldTests.cpp)
    # C-17 — World.h expone EnemyBrain/SkillSet (world->enemy.brain/skills) y
    # los tests de ciclo de vida construyen ambos, cuyas implementaciones
    # viven en motor_level (src/Game/EnemyBrain.cpp, src/Game/Skill.cpp).
    target_link_libraries(test_world PRIVATE motor_world motor_level)

    # INF-01.3 — Reglas puras del modelo del editor, separadas de los
    # round-trips con assets y de la UI OpenGL del editor visual.
    add_motor_unit_test(test_editor_document_io tests/Editor/EditorDocumentIOTests.cpp)
    target_link_libraries(test_editor_document_io PRIVATE motor_level motor_tmx_import)
    add_motor_unit_test(test_editor_state tests/Editor/EditorStateTests.cpp)
    target_link_libraries(test_editor_state PRIVATE motor_level)

    add_motor_unit_test(test_project tests/Project/ProjectTests.cpp)
    target_link_libraries(test_project PRIVATE motor_project)

    add_motor_unit_test(test_project_serialization tests/Project/SerializationTests.cpp)
    target_link_libraries(test_project_serialization PRIVATE motor_project)

    add_motor_unit_test(test_map_document tests/Map/MapDocumentTests.cpp)
    # E-01 — La capa de eventos: una casilla un evento, comandos de tipo
    # desconocido que no pierden datos, y limite de anidamiento.
    add_motor_unit_test(test_event_layer_document tests/Map/EventLayerDocumentTests.cpp)
    target_link_libraries(test_event_layer_document PRIVATE motor_map_document motor_project)
    # U3-02.E06a — Triggers de region en paginas de evento (referencias, ediciones, carga).
    add_motor_unit_test(test_event_region_trigger tests/Map/EventRegionTriggerTests.cpp)
    target_link_libraries(test_event_region_trigger PRIVATE motor_map_document motor_project)
    target_link_libraries(test_map_document PRIVATE motor_map_document)

    add_motor_unit_test(test_runtime_tilemap tests/Runtime/RuntimeTileMapBuilderTests.cpp)
    target_link_libraries(test_runtime_tilemap PRIVATE motor_runtime_map)

    # AV19-02 — La altura del terreno (rejilla, rampas, regla de paso, JSON del nivel).
    add_motor_unit_test(test_alturas tests/Level/AlturasTests.cpp)
    target_link_libraries(test_alturas PRIVATE motor_level motor_json)

    add_motor_unit_test(test_tmx_importer tests/Import/TmxImporterTests.cpp)
    target_link_libraries(test_tmx_importer PRIVATE motor_tmx_import)

    add_motor_unit_test(test_map_round_trip tests/Integration/MapRoundTripTests.cpp)
    target_link_libraries(test_map_round_trip PRIVATE motor_tmx_import motor_project motor_runtime_map)

    # RUN-10 — Guardado de partida: formato, escritura atomica y round-trip
    # sobre una GameSession. Sin mapa (TileMap nullptr) y sin fixtures en
    # assets/: el unico fichero que toca lo escribe y borra el propio test.
    # motor_map entra por el enlazado de GameSession (TileMap), no porque
    # el test abra ningun TMX.
    # Regresion del recorrido del mundo: un nivel con muchas puertas
    # destapaba una referencia colgante en WorldTopology (ver la cabecera
    # del test). Sin assets: el propio test escribe y borra sus niveles.
    add_motor_unit_test(test_world_topology tests/Level/WorldTopologyTests.cpp)
    target_link_libraries(test_world_topology PRIVATE motor_level)

    add_motor_unit_test(test_save_game tests/Game/SaveGameTests.cpp)
    target_link_libraries(test_save_game PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-01 — Objetivo de las habilidades en combate. Sin assets ni cwd: el
    # dado entra por ScriptedRng (examples/, utilidad de tests igual que
    # Check.h), de ahi el include de examples/ en vez de duplicar la clase
    # dentro del test.
    add_motor_unit_test(test_battle_state tests/Game/BattleStateTests.cpp)
    target_include_directories(test_battle_state PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    target_link_libraries(test_battle_state PRIVATE motor_level motor_rpg motor_json)

    # C-03 — Traspaso del estado del jugador entre sesiones. Sin mapa ni
    # assets, igual que test_save_game.
    add_motor_unit_test(test_game_session tests/Game/GameSessionTests.cpp)
    target_link_libraries(test_game_session PRIVATE motor_level motor_map motor_rpg motor_json)
    # C8-03c — la sesión evalúa los eventos del mundo.
    include(cmake/EventosDeLaSesionTests.cmake)
    # ACTUAR-01 — Actuar en la calle: la tirada, el publico, el oro y el descanso de cada sitio.
    add_motor_unit_test(test_actuacion tests/Game/ActuacionTests.cpp)
    target_link_libraries(test_actuacion PRIVATE motor_level motor_map motor_rpg motor_json)
    add_motor_unit_test(test_transiciones tests/Game/TransicionesTests.cpp)  # AV16-10
    target_link_libraries(test_transiciones PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — Economia del negoci: funcions pures (occupancyPercent,
    # incomeFor) i cobrament de lloguers amb moral.
    add_motor_unit_test(test_business_economy tests/Game/BusinessEconomyTests.cpp)
    target_link_libraries(test_business_economy PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — Moral: tramos, clamp, historial y fraccion de Morality
    # (pieza pura GL-free; hasta ahora solo se ejercitaba indirectamente).
    add_motor_unit_test(test_morality tests/Game/MoralityTests.cpp)
    target_link_libraries(test_morality PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — Flujo GameMode del negocio: interact abre el cartel, compra,
    # renta con moral por el CAMBIO y cobro integrado. Etapa que delegó
    # BusinessEconomyTests.cpp a un "futur test d'integració".
    add_motor_unit_test(test_business_mode tests/Game/BusinessModeTests.cpp)
    target_link_libraries(test_business_mode PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — Màquina d'estats del GameMode: Exploracion <-> Dialogo/Tienda,
    # comprar/vender y GameOver de salida. Complemento GL-free puro.
    add_motor_unit_test(test_game_mode tests/Game/GameModeTests.cpp)
    target_link_libraries(test_game_mode PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — El traspàs entre sesiones (dependencia C-03): captureCarryOver
    # / applyCarryOver llevan oro, moral, pasos y negocios intactos.
    add_motor_unit_test(test_carry_over tests/Game/CarryOverTests.cpp)
    target_link_libraries(test_carry_over PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — La patrulla y el libro de combate de EnemyBrain puros, que
    # GameSession posee sin tocar GL. Nunca tuvo test unitario.
    add_motor_unit_test(test_enemy_brain tests/Game/EnemyBrainTests.cpp)
    target_link_libraries(test_enemy_brain PRIVATE motor_level motor_map motor_rpg motor_json)

    # H3-04 — Comercio con datos reales: openShop() (comercio.abrir, sin
    # adyacencia) -> buy -> sell -> closeInteraction. Tests GL-free sobre
    # el catalogo real (precios pocion 30, ether 45, pan 8 de DEC-05).
    # GameSession + GameSessionComercio ya estan en motor_level.
    add_motor_unit_test(test_shop_flow tests/Game/ShopFlowTests.cpp)
    target_link_libraries(test_shop_flow PRIVATE motor_level motor_map motor_rpg motor_json)
    # TIENDA-02 — La tienda en dos pestanas (Game::TiendaEstado): comprar y vender
    # separados, lo que se ensena es lo que se cobra y el foco no se sale.
    add_motor_unit_test(test_tienda_estado tests/Game/TiendaEstadoTests.cpp)
    target_link_libraries(test_tienda_estado PRIVATE motor_level motor_map motor_rpg motor_json)

    # CAER-01 — Huir y caer segun la mision (Oriol, 08/10): caer en un combate
    # de la historia no acaba la partida salvo que sea ineludible; huir tiene
    # su flag; los combates del mapa siguen acabandola.
    add_motor_unit_test(test_caer_en_mision tests/Game/CaerEnMisionTests.cpp)
    target_link_libraries(test_caer_en_mision PRIVATE motor_level motor_map motor_rpg motor_json)

    # C-24 — La resolución de combate Nd6 (ApplyBasicAttackNd6 y
    # ApplySkillEffect) y SkillSet/SkillCatalog. ScriptedRng escribe la
    # cara a mano para fijar el grado exacto.
    add_motor_unit_test(test_skill tests/Game/SkillTests.cpp)
    target_include_directories(test_skill PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    target_link_libraries(test_skill PRIVATE motor_level motor_rpg motor_json)

    # D-03 — Prova del pla dels 432 monstres (PLA_DEFINITIU §D-03): target
    # propi per no editar CatalogContentTests (zona viva D-02/D-07). Conviu
    # amb tests/RPG/MonsterCatalogTests.cpp (WIP huerfano, regla 2: intacte).
    add_motor_unit_test(test_monster_plan_proof tests/RPG/MonsterPlanProofTests.cpp)
    target_link_libraries(test_monster_plan_proof PRIVATE motor_level motor_rpg motor_json)

    # Refuerzo de C-24: red sobre el ObjectCatalog del camino jugado.
    # Fichero sin paquete del plan (MAPA_DEL_CODIGO, tabla 3); el catalogo
    # de la ciudad (97 objetos) queda fijado contra regresion.
    add_motor_unit_test(test_object_catalog tests/Level/ObjectCatalogTests.cpp)
    target_link_libraries(test_object_catalog PRIVATE motor_level motor_json)

    # C-02 — El minimapa HudMinimap guardaba la sesion y habia que
    # reengancharlo en cada cambio de nivel (test_hud_minimap). Su sustituto,
    # Pergamino::MinimapaTinta (PERGAMINO-01), la recibe en cada fotograma y
    # no la guarda: la prueba vive en test_pergamino.
    if (EXISTS ${GLAD_DIR}/src/glad.c)
        # Fractura #1 — Player::populate_from_class_race_background() mirror of
        # EnemyBrain::populate_from_monster_catalog() for player characters.
        add_motor_unit_test(test_player_populate tests/Render/PlayerPopulateTests.cpp)
        target_sources(test_player_populate PRIVATE
            src/Render/Player.cpp
            src/Render/AnimatedEntity.cpp
            src/Render/Entity.cpp
            src/Render/SpriteBatch.cpp
            src/Core/Resources/Texture.cpp
            src/Core/Resources/TextureAtlas.cpp
            src/RPG/CharacterSheet.cpp
            src/RPG/TierRules.cpp
            src/RPG/ConditionEngine.cpp
            ${GLAD_DIR}/src/glad.c
        )
        target_include_directories(test_player_populate PRIVATE ${GLAD_DIR}/include)
        target_link_libraries(test_player_populate PRIVATE motor_json motor_input)
        # C-06 — wrapText con cajas degeneradas. Mismo patron de enlazado:
        # BitmapFont.cpp referencia Texture/SpriteBatch desde drawText(),
        # que este binario nunca llama (las funciones probadas son
        # estaticas y puras).
        add_motor_unit_test(test_bitmap_font tests/Render/BitmapFontTests.cpp)
        target_sources(test_bitmap_font PRIVATE
            src/Render/BitmapFont.cpp
            src/Core/Resources/Texture.cpp
            src/Render/SpriteBatch.cpp
            ${GLAD_DIR}/src/glad.c
        )
        target_include_directories(test_bitmap_font PRIVATE ${GLAD_DIR}/include)

        # C-09 — Propiedad de los recursos GL: la asignacion por movimiento
        # libera el nombre del destino antes de robar el del origen. Doble GL
        # falso: el test redirige glad_glDeleteTextures sin abrir contexto.
        add_motor_unit_test(test_texture tests/Core/TextureTests.cpp)
        target_sources(test_texture PRIVATE
            src/Core/Resources/Texture.cpp
            ${GLAD_DIR}/src/glad.c
        )
        target_include_directories(test_texture PRIVATE ${GLAD_DIR}/include)
    else ()
        message(STATUS "test_player_populate / test_bitmap_font omitidos: falta third_party/glad.")
    endif ()

    # C-10 — Round-trip JSON: lo que escribe JsonWriter (incluidos los
    # escapes \uXXXX de los caracteres de control) vuelve a entrar por
    # JsonValue::parse sin perdidas. Sin assets ni GL.
    add_motor_unit_test(test_json tests/Core/JsonTests.cpp)
    target_link_libraries(test_json PRIVATE motor_json)

    # C-19 — Una sola funcion de culling (Render/Culling.h, header-only).
    # El test lleva dentro la version historica como oraculo y compara los
    # dos sobre diez camaras y cuatro tamanos de mapa. Sin GL: Camera
    # (motor_math) y RuntimeTileMap (motor_runtime_map) son GL-free.
    add_motor_unit_test(test_occlusion tests/Render/OcclusionTests.cpp)
    target_link_libraries(test_occlusion PRIVATE motor_runtime_map motor_math)

    # Pruebas de Camera::transitionTo (Fase 4 - Pulido visual)
    add_motor_unit_test(test_camera_transition tests/Engine/CameraTransitionTests.cpp)
    target_link_libraries(test_camera_transition PRIVATE motor_math)

    # Pruebas de AnimatedEntity (Fase 3 - Entidades y animacion)
    add_motor_unit_test(test_animated_entity tests/Render/AnimatedEntityTests.cpp)
    target_link_libraries(test_animated_entity PRIVATE motor_core motor_math)

    # C-14 (1a rebanada) — Paridad real entre la ficha del documento y la
    # del nivel: los once campos de autoria van y vuelven. Sin assets ni GL.
    add_motor_unit_test(test_level_document_bridge tests/Level/LevelDocumentBridgeTests.cpp)
    target_link_libraries(test_level_document_bridge PRIVATE motor_level motor_map_document)

# D-05 (meitat motor) — InventoryEngine sense doble bonus a 2M y escut
# distingit de la ma esquerra. GL-free pur (motor_rpg).
add_motor_unit_test(test_inventory_engine tests/RPG/InventoryEngineTests.cpp)
target_link_libraries(test_inventory_engine PRIVATE motor_rpg)

# G-12 (rodaja de calcul) — el bonus de vida de l'equip arriba al healthCap.
add_motor_unit_test(test_sheet_life_cap tests/RPG/SheetLifeCapTests.cpp)
target_link_libraries(test_sheet_life_cap PRIVATE motor_rpg)

# G-12b — equipar refresca la ficha al instante (recompute_derived en equip).
add_motor_unit_test(test_sheet_equipment_cap tests/RPG/SheetEquipmentCapTests.cpp)
target_link_libraries(test_sheet_equipment_cap PRIVATE motor_rpg)

# H2-04 (Fase H) — hitos y tierUp: grantMilestones() + el healthCap se
# refresca al subir de tier. El último caso carga classes.json real.
add_motor_unit_test(test_sheet_milestones tests/RPG/SheetMilestonesTests.cpp)
target_link_libraries(test_sheet_milestones PRIVATE motor_rpg)
add_dependencies(test_sheet_milestones copiar_assets)

# GDB-10.5 — Tests estadísticos/deterministas para LootResolver (Fase 6)
add_motor_unit_test(test_loot_resolver tests/RPG/LootResolverTests.cpp)
target_link_libraries(test_loot_resolver PRIVATE motor_rpg)

# GDB-10.5 — Tests para tablas de encuentros
add_motor_unit_test(test_encounter_tables tests/RPG/EncounterTableTests.cpp)
target_link_libraries(test_encounter_tables PRIVATE motor_rpg)

# GDB-11.5 — Tests para el sistema de audio
add_motor_unit_test(test_audio tests/RPG/AudioTests.cpp)
target_link_libraries(test_audio PRIVATE motor_rpg)

# GDB-12.2/12.3/12.4 — Tests para pantallas de juego (Pause, GameOver, Inventario)
add_motor_unit_test(test_gdb12_screens tests/Game/GDB12ScreenTests.cpp)
target_link_libraries(test_gdb12_screens PRIVATE motor_level)

# G-10 — Menús de pausa/opciones + HUD (HudBar animado, HudCommandMenu
# wraparound, HudDialogueBox truncado, sincronización PauseMenu ↔
# HudCommandMenu). Complemento de test_gdb12_screens (8/8 agent anterior).
include(cmake/G10PauseAndHudTests.cmake)

# G-13 — Comandes de combat i objectius des de dades (dep G-09/D-02/D-09).
# Menu buildBattleMenuSkillActions + selectTargets + translateToBattleAction
# sobre BattleState. 6 tests GL-free (build/select/cancel/e2e_dany_cura/
# assets_publicat_492/condicions_publicades).
include(cmake/G13CombatCommandsTests.cmake)

# G-14 — HUD timers congelados + diàlegs wrapText + botiga (shop) amb
# equipment 413 preus valids (DEC-05). REV-02 (panell amagat, texto
# narratiu prioritari sobre log) + REV-03 (botiguer sense duplicats,
# preus valids sense contingut de prova). 6 tests GL-free complement.
include(cmake/G14HudTimersAndCommerceTests.cmake)

# H1-05 / G-17 — bridge OptionsState ↔ Locale (mapatge de codis d'idioma,
# canvi en calent amb taules reals d'assets/i18n/).
include(cmake/OptionsLocaleBridgeTests.cmake)
endif()

# RUN-11.3 — Target agregado con TODO lo que CTest ejecuta bajo la etiqueta
# gl-free. Se construye con:
#
#   cmake --build build --target suite_gl_free
#   ctest --test-dir build --output-on-failure --label-regex gl-free
#
# Es lo que corre CI. Existe para que la lista viva en un solo sitio (las
# llamadas a add_motor_gl_free_demo_test/add_motor_unit_test de arriba) en
# vez de duplicada en el YAML del workflow.
if(BUILD_TESTING)
    get_property(_motor_gl_free_targets GLOBAL PROPERTY MOTOR_GL_FREE_TARGETS)
    if(_motor_gl_free_targets)
        add_custom_target(suite_gl_free)
        add_dependencies(suite_gl_free ${_motor_gl_free_targets})
        list(LENGTH _motor_gl_free_targets _motor_gl_free_count)
        message(STATUS "Suite GL-free: ${_motor_gl_free_count} objetivos")
    endif()
endif()

# G-08 (rodaja inicial) — Pantalla de título: lista de proyectos, navegación y acciones.
include(cmake/TitleScreenTests.cmake)

# C-05 — Regresion del cierre de recursos con contexto GL real.
include(cmake/ApplicationShutdownTests.cmake)

# C-24 — Xarxa de seguretat: tests GL-free d'Application (refreshHud, loadLevel, save/load, campanya, GameMode).
include(cmake/ApplicationTests.cmake)

# opencode 19/09 — Cableado de Application "que el jugador vea lo que ya
# existe" (Pendiente 34): catalogo de equipo en la sesion y ESC de pausa.
include(cmake/ApplicationCableadoTests.cmake)

# D-01 — Contrato de campos de los cargadores de catalogo (Fase D).
include(cmake/CatalogContractTests.cmake)

# D-02 — Contenido real del catalogo de habilidades y hechizos (Fase D).
include(cmake/CatalogContentTests.cmake)

# opencode 19/09 — Guiones contra catalogos: startBattle.monsterId/talk/skillCheck.
include(cmake/AventurasCatalogNetTests.cmake)

# D-03 — Monstres: catàleg carregat amb vida, defenses, estadístiques i habilitats.
include(cmake/MonsterCatalogTests.cmake)

# D-05 (part catalogs RPG) — Equipment/Consumable/LootTable contents
# carregats des de assets/catalogs publicats amb noms DEC-02.
include(cmake/ObjectAndLootCatalogTests.cmake)

# C-16 — Transiciones de escenas sobre Application real.
include(cmake/ApplicationSceneTests.cmake)

# C-07 — Les aventures autònomes s'enganxen a la sessió i disparen els
# seus beats, i NO encadenen la cronologia de Boundington. GL-free.
include(cmake/ApplicationNarrativeWiringTests.cmake)

# Tutorial de Boundington en la zona principal — comerciantes, emboscada y combate.
include(cmake/BoundingtonTutorialTests.cmake)
include(cmake/BoundingtonDia1Tests.cmake)

# D-06 — Creació de personatge: bonificacions de classe/raça/trasfond
# i set eleccions narratives arriben a la CharacterSheet.
include(cmake/CharacterCreationTests.cmake)

# D-06 (part catàlegs RPG) — Classes/Races/Backgrounds contingut real
# carregats des de assets/catalogs publicats (DEC-02).
include(cmake/ClassesRacesBackgroundsCatalogTests.cmake)

# D-11 — Vocabulari d'efectes d'habilitat.
include(cmake/SkillEffectTests.cmake)

# D-11 — SkillExecutor pipeline: context bonuses, opposed rolls, combined outputs.
include(cmake/SkillExecutorTests.cmake)

# D-11 (part assets publicats) — parse_effect_scaling, magnitude_by_degree i
# SkillExecutor sobre skills.json publicat (492 entrades).
include(cmake/SkillEffectRealAssetsTests.cmake)

# D-08 — Bridge ObjectCatalog → MonsterCatalog: enemics amb stats reals.
include(cmake/EnemyCatalogBridgeTests.cmake)

# D-08 (part assets publicats) — Bridge sobre monsters.json real.
include(cmake/EnemyCatalogRealAssetsTests.cmake)

# D-09 — Les cartes apliquen les seves condicions (conditions assets reals).
include(cmake/ConditionsRealAssetsTests.cmake)

# D-11 — Flag speedZero per `velocidad_0` (agarrado/anclado/paralizado).
include(cmake/SpeedZeroFlagTests.cmake)

# D-10 — Validació de catàlegs reals: validar_todo.py comprova
# referències creuades i camps mecànics de tots els JSONs d'assets/.
include(cmake/CatalogValidationTests.cmake)

# D-07 — Resto de catàlegs (passives, feats, traits, deities,
# conditions, summons, mounts, traps, locations). Contingut real.
include(cmake/RestOfCatalogsTests.cmake)

# La regeneración debe conservar la conexión de la ciudad y ser idempotente.
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_city_regeneration
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tests/tools/test_regenerar_ciudad.py")
    set_tests_properties(test_city_regeneration PROPERTIES LABELS "unit;gl-free" TIMEOUT 30)
endif()

# AV16-08 — Un nivel como Excel y de vuelta (tools/nivel_a_excel.py, tools/excel_a_nivel.py).
if(BUILD_TESTING AND MOTOR_PYTHON3)
    add_test(NAME test_nivel_excel
        COMMAND ${MOTOR_PYTHON3} "${CMAKE_CURRENT_SOURCE_DIR}/tests/tools/test_nivel_excel.py")
    set_tests_properties(test_nivel_excel PROPERTIES LABELS "unit;gl-free" TIMEOUT 60)
endif()

include(cmake/PostProcessingTests.cmake)

include(cmake/ProjectCatalogTests.cmake)

include(cmake/MotorEditor.cmake)
# H4-02 / E-11 — test_project_panel: mapas de un proyecto desde el panel, sin ventana.
include(cmake/ProjectPanelTests.cmake)
include(cmake/EventEditorTests.cmake)
include(cmake/CommandListTests.cmake)
include(cmake/E09BridgeTests.cmake)
include(cmake/CatalogPickerTests.cmake)
include(cmake/AssetBrowserStateTests.cmake)
include(cmake/AssetBrowserTests.cmake)
include(cmake/ProjectDatabaseTests.cmake)
include(cmake/NewGameFlowTests.cmake)
include(cmake/FabricarGuardados.cmake)  # E-GUION-02: guardados para el modo guion (GL-free)
include(cmake/MapPropertiesTests.cmake)
include(cmake/EditorToolsTests.cmake)
include(cmake/CatalogLoaderContractCheck.cmake)
include(cmake/InventoryMenuTests.cmake)
include(cmake/FichaVistaTests.cmake)
# MISIONES-01 — el diario es una vista de las flags. En su propio fichero:
# el compartido (FichaVistaTests) arrastra las fuentes de tres sesiones mas.
include(cmake/DiarioMisionesTests.cmake)
# AV14-22 (D0016) — el diario completo: la hoja de Misiones y su prueba con Boundington.
include(cmake/DiarioCompleto.cmake)
# AV14-23 (D0015 §15) — la reputacion por faccion en la tienda y la ficha, y los rumores.
include(cmake/ComercioReputacion.cmake)
# PNJ-02 (ronda 15/10, pnj_motor) — los PNJ andan y hacen su oficio (rutinas_pnj.json).
include(cmake/RutinasPnj.cmake)
include(cmake/ClipsAnimacion.cmake)
# AV15-05a — confirmar lo caro e irreversible (EH-22, 23, 29, 30).
include(cmake/Confirmar.cmake)
include(cmake/AparienciaTests.cmake)
include(cmake/MontajeCacheTests.cmake)  # PERF-1: la cache de atlas del montaje
include(cmake/CreacionBg3Tests.cmake)  # UI-BG3-01: kit del cofre, cuaderno de creacion y barra de cartas
# SPRITES-01 — el indice de hojas de sprites y sus reglas de busqueda.
include(cmake/RegistroSpritesTests.cmake)
include(cmake/BloquesTests.cmake)
include(cmake/BandadaTests.cmake)
include(cmake/MascotesTests.cmake)
include(cmake/MonumentosTests.cmake)
include(cmake/TitleFlowTests.cmake)
include(cmake/C24RenderRpgTests.cmake)

# EST-10 - Sistema de logs Core/Log (datos + controlador + sinks) y su prueba GL-free.
include(cmake/LogTests.cmake)

# EST-11 - ScopeGuard: el `finally` de C++ (norma N4) y su prueba GL-free.
include(cmake/ScopeGuardTests.cmake)
include(cmake/LevelEditorNucleoTests.cmake)
include(cmake/EditorOpcionesTests.cmake)
include(cmake/EditorMapFraming.cmake)
include(cmake/FormatoPPMTests.cmake)
# EST-56 — Cuatro fragmentos que existian, registraban seis pruebas y no
# los incluia nadie: las pruebas estaban escritas, pasaban, y CI no las
# ejecutaba nunca. Tres se dejaron sin include() a proposito en su dia
# («CMakeLists.txt tenia cambios sin commitear de otra sesion»), y ahi se
# quedaron. Una nota en un comentario no es un recordatorio: es un olvido
# con fecha.
include(cmake/ExportarTests.cmake)
include(cmake/LineaBaseCore.cmake)
include(cmake/ValidarTodoTests.cmake)
include(cmake/BuscarSobranteTests.cmake)

# EST-51 — el rastro de logs del camino del jugador. Va al final para que
# motor_log y motor_level ya existan cuando se enlaza la prueba.
include(cmake/PlayerPathLogTests.cmake)

# EST-52 — ninguna excepcion cruza la frontera de un modulo. Al final, igual
# que EST-51, para que motor_log y motor_level ya existan al enlazar.
include(cmake/BoundaryExceptionsTests.cmake)


# H2-06 (Fase H) — las cartas de clase en combate (Game::CartasEnCombate).
# Al final, igual que H2-02: la prueba enlaza motor_level.
include(cmake/CartasCombateTests.cmake)

# SP-02 — las SP de NosTale como clases nuevas (enlaza motor_level).
include(cmake/ClasesSpTests.cmake)

# CP-01 — las clases propias (enlaza motor_level).
include(cmake/ClasesPropiasTests.cmake)

# AV13-04 (COMBATE_BG3.md §6) — la probabilidad exacta de una tirada enfrentada;
# D0013: el critico son dos 6 (enlaza motor_rpg).
include(cmake/ProbabilidadPool.cmake)

# ACOMPANANTES-01 — el simulador del estudio de dificultad (enlaza motor_level).
include(cmake/SimuladorDificultad.cmake)
