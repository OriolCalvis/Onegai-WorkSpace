cmake_minimum_required(VERSION 3.16)
project(MotorGraficoIsometrico CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# assets/ (shaders, texturas, mapas TMX) se referencia con rutas relativas
# desde los ejecutables (ver demo_textured_quad.cpp, demo_tilemap.cpp):
# se copia junto al binario para que funcionen ejecutados desde build/.
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# --- Demo sin dependencias externas: prueba ResourceManager<T> de forma
# aislada. Compila y pasa en cualquier maquina con g++/clang++ C++17. ---
add_executable(demo_resource_manager
    examples/demo_resource_manager.cpp
    examples/TextAssetManager.cpp
)

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

add_library(motor_level
    src/Level/LevelLoader.cpp
    src/Game/Skill.cpp
    src/Game/BattleState.cpp
    src/Level/ObjectCatalog.cpp
    src/Editor/EditorState.cpp
    src/Game/GameSession.cpp
    src/Game/Morality.cpp
    src/Game/EnemyBrain.cpp
)
# GameSession.cpp usa TileMap (motor_map) para la colision, pero
# motor_map se declara mas abajo en este archivo: en vez de reordenar
# (motor_map depende a su vez de motor_math/tinyxml2), cada ejecutable
# que use GameSession linka ambas librerias -- mismo patron que
# demo_editor_state. Ninguna de las dos trae GLAD/GLFW: siguen siendo
# GL-free.
target_link_libraries(motor_level PUBLIC motor_json motor_rpg)

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

# ====================================================================
# demo_mundo — B24: el mundo de Egaroth (19 naciones, 95 ciudades, 26 zonas)
# desde assets/catalogs/locations.json. El mapa esta fechado en 2000 b.f.
# GL-free, corre en CI.
add_executable(demo_mundo examples/demo_mundo.cpp)
target_link_libraries(demo_mundo PRIVATE motor_level)
set_target_properties(demo_mundo PROPERTIES CXX_STANDARD 17)

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

add_library(motor_map
    src/Core/Resources/TextureAtlas.cpp
    src/Render/TileMap.cpp
    ${TINYXML2_DIR}/tinyxml2.cpp
)
target_include_directories(motor_map PUBLIC ${TINYXML2_DIR})
target_link_libraries(motor_map PUBLIC motor_math)

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
    target_link_libraries(motor_core PUBLIC glfw glm::glm motor_map)

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
    add_executable(level_editor examples/level_editor.cpp)
    target_link_libraries(level_editor PRIVATE motor_core motor_math motor_level)

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
