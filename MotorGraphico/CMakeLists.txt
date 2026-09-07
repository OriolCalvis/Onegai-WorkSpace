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

add_executable(demo_camera examples/demo_camera.cpp)
target_link_libraries(demo_camera PRIVATE motor_math)

# RUN-01 — El reloj y el contrato del bucle no dependen de GLFW ni de GL.
# Mantenerlos en su propio target permite probar la salida y el timestep sin
# abrir una ventana ni inicializar una partida.
add_library(motor_engine_loop src/Engine/ApplicationLoop.cpp)

# RUN-02 — Estado de teclado, bindings y snapshots de acciones. Es un núcleo
# puro: GLFW se adapta desde Application y los tests usan teclas simuladas.
add_library(motor_input src/Engine/Input/InputSystem.cpp)

# --- Fase 6 de motor_grafico_gantt_rpg.puml (sistema de niveles y
# contenido): JsonValue (parser JSON propio, sin dependencias de terceros
# -- ver el comentario de include/Core/Json/JsonValue.h) y, sobre el,
# LevelLoader/Skill (datos de nivel y de habilidades). Ninguno de los dos
# toca GL/GLFW, asi que -- igual que motor_math -- se compilan y prueban
# sin ventana ni contexto OpenGL. ---
add_library(motor_json src/Core/Json/JsonValue.cpp)

add_library(motor_rpg
    src/RPG/SkillExecutor.cpp
    src/RPG/SkillDefinitionEnums.cpp
    src/RPG/TierRules.cpp
    src/RPG/CharacterSheet.cpp
    src/RPG/CharacterCreation.cpp
    src/RPG/CharacterCreationScreen.cpp
    src/RPG/InventoryEngine.cpp
    src/RPG/ConditionEngine.cpp
    src/RPG/NarrativeEngine.cpp
)
# motor_rpg = Motor Onegai Nd6. GL-free puro: contiene DicePoolEngine,
# CharacterSheet, EnhanceHooks, SkillExecutor, TierRules, InventoryEngine,
# ConditionEngine y 20 catalogos tipados (RpgCatalogs.h header-only +
# RpgCoreDefinitions.h). Todos los headers son include/RPG/*.h y NO
# incluyen Render/*.h ni GL.
target_link_libraries(motor_rpg PUBLIC motor_json)

# DOC-01 — Los documentos de proyecto son independientes del runtime y
# del editor visual. ProjectIndex conserva el I/O de manifiestos; Project
# y ProjectSettings se validan sin abrir ventana ni cargar assets.
add_library(motor_project
    src/Project/Project.cpp
    src/Project/ProjectIndex.cpp
    src/Project/ProjectSerializer.cpp
)
target_link_libraries(motor_project PUBLIC motor_json motor_map_document)

# DOC-02 — Modelo documental de mapas. No depende de TileMap ni de GL: el
# builder de DOC-03 será el único puente desde estos datos al runtime.
add_library(motor_map_document src/Map/MapDocument.cpp)

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

add_library(motor_level
    src/Level/LevelLoader.cpp
    src/Game/Skill.cpp
    src/Game/BattleState.cpp
    src/Level/ObjectCatalog.cpp
    src/Editor/EditorState.cpp
    src/Editor/ObjectInspector.cpp
    src/Editor/ProjectHub.cpp
    src/Game/GameSession.cpp
    src/Game/SaveGame.cpp
    src/Game/Morality.cpp
    src/Game/EnemyBrain.cpp
)
# GameSession.cpp usa TileMap (motor_map) para la colision, pero
# motor_map se declara mas abajo en este archivo: en vez de reordenar
# (motor_map depende a su vez de motor_math/tinyxml2), cada ejecutable
# que use GameSession linka ambas librerias -- mismo patron que
# demo_editor_state. Ninguna de las dos trae GLAD/GLFW: siguen siendo
# GL-free.
target_link_libraries(motor_level PUBLIC motor_json motor_project motor_rpg motor_runtime_map)

add_executable(demo_level_loader examples/demo_level_loader.cpp)
target_link_libraries(demo_level_loader PRIVATE motor_level)

add_executable(demo_skills examples/demo_skills.cpp)
target_link_libraries(demo_skills PRIVATE motor_level)

# Fase 8 de motor_grafico_gantt_rpg.puml: combate por turnos. GL-free
# igual que LevelLoader/Skill (BattleState opera sobre ICombatant/
# SkillSet, no sobre Player/Enemy concretos).
add_executable(demo_battle examples/demo_battle.cpp)
target_link_libraries(demo_battle PRIVATE motor_level)

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
# demo_boundington — el primer dia en Boundington (1981 b.f.), la primera
# aventura con final ramificado. Juega tres rutas, comprueba los tres
# cierres y que no queden beats muertos. GL-free, corre en CI.
add_executable(demo_boundington examples/demo_boundington.cpp)
target_link_libraries(demo_boundington PRIVATE motor_level)
set_target_properties(demo_boundington PROPERTIES CXX_STANDARD 17)
add_dependencies(demo_boundington copiar_assets)

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
        src/Render/HudMinimap.cpp
        src/Render/BitmapFont.cpp
        src/Render/HudTextWidgets.cpp
        src/Render/DynamicLightMap.cpp
        src/Render/BlobShadow.cpp
        src/Engine/Application.cpp
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
    target_link_libraries(motor_core PUBLIC glfw glm::glm motor_map motor_engine_loop motor_input)

    add_executable(sandbox_window examples/sandbox_window.cpp)
    target_link_libraries(sandbox_window PRIVATE motor_core)

    # Fase 1, ultima tarea: quad texturizado con GL_NEAREST. Usa Window +
    # Texture/Shader (motor_core) y Camera (motor_math) a la vez.
    add_executable(demo_textured_quad examples/demo_textured_quad.cpp)
    target_link_libraries(demo_textured_quad PRIVATE motor_core motor_math)

    # Fase 2 (culling) + Fase 3 (arranque): IsometricRenderer dibujando un
    # TileMap culled + un par de entidades de prueba, con Painter's
    # Algorithm real.
    add_executable(demo_isometric_renderer examples/demo_isometric_renderer.cpp)
    target_link_libraries(demo_isometric_renderer PRIVATE motor_core motor_math)

    # Fase 3 (cierre): AnimatedEntity/Player/Enemy sobre el mismo pipeline
    # (TileMap + IsometricRenderer) que demo_isometric_renderer, con
    # contexto OpenGL real.
    add_executable(demo_animated_entity examples/demo_animated_entity.cpp)
    target_link_libraries(demo_animated_entity PRIVATE motor_core motor_math)

    # Fase 4 (arranque): post-procesado + iluminacion. Mismo pipeline que
    # demo_animated_entity, pero activando el post-FX de IsometricRenderer
    # (Framebuffer + fullscreen-triangle + shader lightmap) con un lightmap
    # procedural.
    add_executable(demo_lighting examples/demo_lighting.cpp)
    target_link_libraries(demo_lighting PRIVATE motor_core motor_math)

    # Fase 4 (2o efecto): niebla de guerra con memoria de exploracion.
    # Mismo pipeline, pero activando el post-FX de niebla (shader fog +
    # textura de visibilidad de FogOfWar) que revela un radio alrededor del
    # Player cada frame.
    add_executable(demo_fog examples/demo_fog.cpp)
    target_link_libraries(demo_fog PRIVATE motor_core motor_math)

    # Fase 4 (3er efecto): paletizado / LUT de color. Mismo pipeline, pero
    # activando el post-FX de LUT (shader lut + ColorLUT, cubo RGB
    # cuantizado a "levels" tonos por canal) para un look de paleta
    # limitada, estilo pixel art.
    add_executable(demo_lut examples/demo_lut.cpp)
    target_link_libraries(demo_lut PRIVATE motor_core motor_math)

    # Fase 9 de motor_grafico_gantt_rpg.puml: HUD modular. Mismo pipeline
    # de mundo que demo_lighting/demo_fog/demo_lut, con una SpriteBatch y
    # un HudManager dibujando HudBar/HudPanel encima (reutilizando el
    # shader "sprite", ver el comentario de HudManager::render()).
    add_executable(demo_hud examples/demo_hud.cpp)
    target_link_libraries(demo_hud PRIVATE motor_core motor_math)

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
    target_link_libraries(demo_dynamic_lights PRIVATE motor_core motor_math)

    # Fase 12: editor de niveles visual. UI con el HUD propio (Fase 9),
    # no con Dear ImGui (sin red para vendorizarlo en el entorno de
    # desarrollo, ver EditorState.h); el nucleo GL-free ya esta verificado
    # en demo_editor_state. Usa GLFW directamente (raton) via
    # Window::handle(), ademas de motor_core/motor_math/motor_level.
    add_executable(level_editor editor/src/level_editor.cpp)
    target_link_libraries(level_editor PRIVATE motor_core motor_math motor_level)
    # El editor consume sus iconos de previsualización desde assets/:
    # sincronizarlos en cada build evita que un asset nuevo falle solo
    # cuando el ejecutable se lanza desde build/.
    add_dependencies(level_editor copiar_assets)

    # El juego: Application (bucle completo) sobre GameSession. Cierra el
    # motor de punta a punta -- Fases 1-4 del Gantt original + 6-12 del
    # RPG en una partida jugable. motor_core ya arrastra motor_map
    # (PUBLIC), que es lo que GameSession necesita para la colision.
    add_executable(juego examples/juego.cpp)
    target_link_libraries(juego PRIVATE motor_core motor_math motor_level)
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

# C-20 — Regresiones del propietario unico de las condiciones.
include(cmake/ConditionEngineTests.cmake)

# C-22 — Comprobador estático de declaraciones sin definición en include/RPG.
include(cmake/DeclarationCheck.cmake)
include(cmake/ProjectManifestTests.cmake)

# INF-01.1 — Registro inicial de los demos GL-free verificados en la línea
# base. Las pruebas se ejecutan desde el directorio de build porque sus rutas
# de assets son relativas a ese cwd. Se añaden más demos en INF-01.2/01.3 al
# convertir sus aserciones y fixtures en tests por dominio.
if(BUILD_TESTING)
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
    add_motor_gl_free_demo_test(demo_battle)
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
    target_link_libraries(test_world PRIVATE motor_world)

    # INF-01.3 — Reglas puras del modelo del editor, separadas de los
    # round-trips con assets y de la UI OpenGL del editor visual.
    add_motor_unit_test(test_editor_state tests/Editor/EditorStateTests.cpp)
    target_link_libraries(test_editor_state PRIVATE motor_level)

    add_motor_unit_test(test_project tests/Project/ProjectTests.cpp)
    target_link_libraries(test_project PRIVATE motor_project)

    add_motor_unit_test(test_project_serialization tests/Project/SerializationTests.cpp)
    target_link_libraries(test_project_serialization PRIVATE motor_project)

    add_motor_unit_test(test_map_document tests/Map/MapDocumentTests.cpp)
    target_link_libraries(test_map_document PRIVATE motor_map_document)

    add_motor_unit_test(test_runtime_tilemap tests/Runtime/RuntimeTileMapBuilderTests.cpp)
    target_link_libraries(test_runtime_tilemap PRIVATE motor_runtime_map)

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

    # C-02 — Reenganche de la sesion en el minimapa. Mismo patron de
    # enlazado que demo_bitmap_font / demo_hud_update: HudMinimap.cpp
    # referencia SpriteBatch (y este, glad) desde render(), que este
    # binario NUNCA llama -- se enlazan solo para resolver los simbolos,
    # asi que el test corre sin ventana ni contexto GL. Enlazar
    # motor_core en su lugar arrastraria GLFW a un test gl-free.
    if (EXISTS ${GLAD_DIR}/src/glad.c)
        add_motor_unit_test(test_hud_minimap tests/Render/HudMinimapTests.cpp)
        target_sources(test_hud_minimap PRIVATE
            src/Render/HudMinimap.cpp
            src/Render/HudElement.cpp
            src/Render/SpriteBatch.cpp
            src/Core/Resources/Texture.cpp
            ${GLAD_DIR}/src/glad.c
        )
        target_include_directories(test_hud_minimap PRIVATE ${GLAD_DIR}/include)
        target_link_libraries(test_hud_minimap PRIVATE motor_level motor_map motor_rpg motor_json)
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
        message(STATUS "test_hud_minimap / test_bitmap_font omitidos: falta third_party/glad.")
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

# D-01 — Contrato de campos de los cargadores de catalogo (Fase D).
include(cmake/CatalogContractTests.cmake)
