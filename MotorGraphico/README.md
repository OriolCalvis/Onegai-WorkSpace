# Motor Gráfico Isométrico Pixel Art (C++)

Implementación de `Core::Resources::ResourceManager<T>` y sus dependencias
(`Result<T>`, `EngineException`), la **Fase 1 completa del Gantt**
(ventana + contexto OpenGL, cámara ortográfica, proyección isométrica y
quad texturizado con `GL_NEAREST`), la **Fase 2 completa** (carga de
atlas de tiles, capas de mapa, parser de mapas TMX, culling), la
**Fase 3 completa** (`IRenderable`/`IUpdatable`, `Entity`,
`IsometricRenderer` con Painter's Algorithm, y la jerarquía
`AnimatedEntity`/`Player`/`Enemy` con animación por frames, movimiento e
IA básica) y el arranque de la **Fase 4** (post-procesado: `Framebuffer`
render-to-texture, tint por vértice en `SpriteBatch`/`Entity`,
iluminación por fragment shader vía `IsometricRenderer::setPostFX()`,
niebla de guerra con memoria de exploración vía `FogOfWar`/`setFog()`,
paletizado/LUT de color vía `ColorLUT`/`setLUT()` y transiciones de
cámara/zoom con easing vía `Camera::transitionTo()`),
siguiendo el diagrama de clases del motor (`motor_grafico_clases.puml`).

Continuación hacia un RPG por turnos estilo Dragon Quest clásico
(roadmap en [`motor_grafico_gantt_rpg.puml`](motor_grafico_gantt_rpg.puml)):
sistema de niveles en JSON (`Core::Json::JsonValue`, `LevelLoader`),
sistema de habilidades (`Skill`/`SkillSet`/`SkillCatalog`,
`ApplySkillEffect()`), combate por turnos con items (`BattleState`) y
catálogo genérico de objetos (`ObjectCatalog`, "todo es un objeto") como
datos y lógica GL-free, más un framework de HUD modular completo
(`IHudElement`/`HudManager`/`HudBar`/`HudPanel` + fuente bitmap propia
`BitmapFont` y los widgets de texto `HudText`/`HudCommandMenu`/
`HudDialogueBox`), luces dinámicas + sombras blob (`DynamicLightMap`,
`BlobShadow`) y un editor de niveles visual (`EditorState` +
`level_editor`, exportando TMX y JSON de nivel) — el roadmap RPG
completo, Fases 6-12. Todo ello unido en una **partida jugable** por
`GameSession` (exploración↔combate, GL-free y verificada de verdad) y
`Application` (el bucle de juego), con `juego` como ejecutable. Ver la
sección "Sistema RPG" más abajo para el estado exacto de cada pieza.

Para crear contenido nuevo manteniendo el estilo, consultar primero la
[guía visual de sprites y moldes](SPRITE_AUTHORING_GUIDE.md): separa los
assets de runtime de las previsualizaciones exclusivas del editor.
Las medidas y el proceso de convertir suelo a isométrica 2:1 están en la
[guía de dimensiones de sprites](SPRITE_DIMENSIONS_GUIDE.md).

El [backlog de sprites](SPRITE_BACKLOG.md) prioriza las familias de arte
necesarias para terminar Boundington y evita duplicar un PNG por objeto.

## Estructura

```
CMakeLists.txt

include/Core/Math/Vector2.h             Vector2, del diagrama de clases
include/Core/Math/Vector4.h             Vector4, color/tint RGBA (Fase 4)
include/Core/Math/GridCoord.h           Coordenada entera de grid
include/Core/Math/UVRect.h              Region UV de un atlas
include/Core/Math/IsoMath.h             Proyeccion isometrica 2:1 (grid<->screen), sin GL
include/Core/Math/GridBounds.h          Rango [minX..maxX]x[minY..maxY], para culling
include/Core/Errors/EngineException.h   Jerarquía de excepciones del motor
include/Core/Errors/Result.h            Result<T>, para operaciones recuperables
include/Core/Resources/ResourceManager.h  Template base genérico
include/Core/Resources/Texture.h        Wrapper RAII de textura GL
include/Core/Resources/TextureManager.h ResourceManager<Texture>
include/Core/Resources/Shader.h         Wrapper RAII de programa GLSL
include/Core/Resources/ShaderManager.h  ResourceManager<Shader>
include/Core/Resources/TextureAtlas.h   Recorta subregiones (UVRect) de un atlas de tiles
include/Engine/Window.h                 Ventana + contexto OpenGL 3.3 (GLFW+GLAD)
include/Render/Camera.h                 Camara ortografica: paneo suavizado, zoom, iso, transiciones con easing (solo glm)
include/Render/SpriteBatch.h            Agrupa quads texturizados en un VBO, minimiza draw calls
include/Render/Tile.h                   Celda de TileMap: GID, colision, variante
include/Render/TileMap.h                Mapa de tiles cargado desde TMX (Tiled), varias capas, culling
include/Render/IRenderable.h            Interfaz: render(SpriteBatch&) + getSortKey()
include/Render/IUpdatable.h             Interfaz: update(deltaTime)
include/Render/Entity.h                 Objeto dibujable en el grid (base de AnimatedEntity/Player/Enemy)
include/Render/AnimatedEntity.h         Entity con animacion por frames (addAnimation()/play())
include/Render/InputState.h             Entrada cruda (4 direcciones) para Player::handleInput
include/Render/ICombatant.h             Fase 7 (RPG): interfaz comun de recibir dano/curacion (Player/Enemy la implementan)
include/Render/Player.h                 AnimatedEntity jugable + ICombatant: movimiento por celda, salud, inventario
include/Render/Enemy.h                  AnimatedEntity con IA + ICombatant: patrulla determinista entre dos limites
include/Render/IsometricRenderer.h      Orquesta un frame: TileMap culled + cola de Entity ordenada + post-FX
include/Render/Framebuffer.h            FBO render-to-texture (Fase 4: base del post-procesado)
include/Render/FogOfWar.h               Fase 4 (2o efecto): niebla con memoria Hidden/Explored/Visible
include/Render/ColorLUT.h               Fase 4 (3er efecto): paletizado/LUT de color, cubo RGB cuantizado
include/Core/Json/JsonValue.h           Fase 6 (RPG): parser JSON propio, sin dependencias, GL-free
include/Render/LevelDefinition.h        Fase 6: datos de nivel (LevelDefinition/EnemySpawn)
include/Render/LevelLoader.h            Fase 6: JSON -> LevelDefinition (usa JsonValue)
include/Render/Skill.h                  Fase 7: Skill/SkillSet/SkillCatalog + ApplySkillEffect(), conectado a ICombatant
include/Render/BattleState.h            Fase 8: combate por turnos (BattleState/BattleParticipant/BattleAction), GL-free
include/Render/HudElement.h             Fase 9: HudAnchor/HudTransform (anclas, sin GL) + interfaz IHudElement
include/Render/HudManager.h             Fase 9: composicion/orden de HUD, proyeccion ortografica en pantalla
include/Render/HudWidgets.h             Fase 9: HudBar (vida/mana), HudPanel
include/Render/BitmapFont.h             Fase 9: fuente bitmap 5x7 procedural (glyphUV()/measureText() puros, drawText() necesita GL)
include/Render/HudTextWidgets.h         Fase 9: HudText, HudCommandMenu (menu navegable), HudDialogueBox (log de combate)
include/Render/ObjectCatalog.h          Fase 10: ObjectDefinition/ObjectCatalog ("todo es un objeto": prop/enemy/pickup), GL-free
include/Render/DynamicLightMap.h        Fase 11: PointLight + composicion CPU a textura de lightmap (attenuationAt/sampleLightAt puras)
include/Render/BlobShadow.h             Fase 11: textura procedural de sombra blob (FillBlobShadowPixels pura)
include/Render/EditorState.h            Fase 12: nucleo GL-free del editor (grid, paletas, herramientas, export TMX/JSON)
include/Render/GameSession.h            Partida: exploracion <-> combate, colision, pickups, encuentros (GL-free)
include/Engine/Application.h            Bucle de juego completo (init/run/shutdown) sobre GameSession

src/Core/Resources/Texture.cpp
src/Core/Resources/TextureManager.cpp
src/Core/Resources/TextureAtlas.cpp
src/Core/Resources/Shader.cpp
src/Core/Resources/ShaderManager.cpp
src/Engine/Window.cpp
src/Render/Camera.cpp
src/Render/SpriteBatch.cpp
src/Render/TileMap.cpp
src/Render/Entity.cpp
src/Render/AnimatedEntity.cpp
src/Render/Player.cpp
src/Render/Enemy.cpp
src/Render/IsometricRenderer.cpp
src/Render/Framebuffer.cpp
src/Render/FogOfWar.cpp
src/Render/ColorLUT.cpp
src/Core/Json/JsonValue.cpp
src/Render/LevelLoader.cpp
src/Render/Skill.cpp
src/Render/HudElement.cpp
src/Render/HudManager.cpp
src/Render/HudWidgets.cpp
src/Render/BattleState.cpp

examples/TextAsset.h              Recurso de prueba sin dependencias gráficas
examples/TextAssetManager.h/.cpp  ResourceManager<TextAsset>
examples/demo_resource_manager.cpp  Demo/test ejecutable (sin GL)
examples/demo_camera.cpp             Demo/test de Camera + IsoMath (sin GL, solo glm)
examples/sandbox_window.cpp          Demo de Window: abre ventana y pinta un color
examples/demo_textured_quad.cpp      Fase 1 completa: Window+Camera+SpriteBatch dibujando un quad
examples/demo_tilemap.cpp            Fase 2: TileMap+TextureAtlas+culling sobre un TMX real (sin GL)
examples/demo_isometric_renderer.cpp Fase 3 (arranque): IsometricRenderer, TileMap+Entity+sortQueue
examples/demo_animated_entity.cpp    Fase 3 (cierre): AnimatedEntity/Player/Enemy + pipeline completo
examples/demo_lighting.cpp           Fase 4 (1er efecto): Framebuffer + shader lightmap + post-FX
examples/demo_fog.cpp                Fase 4 (2o efecto): FogOfWar + shader fog + post-FX
examples/demo_lut.cpp                Fase 4 (3er efecto): ColorLUT + shader lut + post-FX
examples/demo_level_loader.cpp       Fase 6 (RPG, sin GL): JsonValue + LevelLoader
examples/demo_skills.cpp             Fase 7 (RPG, sin GL): SkillSet/SkillCatalog
examples/demo_hud.cpp                Fase 9 (RPG): HudManager + HudBar/HudPanel + post-FX
examples/demo_battle.cpp             Fase 8 (RPG, sin GL): BattleState (combate por turnos)
examples/demo_bitmap_font.cpp        Fase 9 (RPG, sin GL): BitmapFont::glyphUV()/measureText() (puros)
examples/demo_battle_hud.cpp         Fase 9 (RPG, cierre): BattleState + HudCommandMenu/HudDialogueBox/HudBar
examples/demo_object_catalog.cpp     Fase 10 (RPG, sin GL): ObjectCatalog + ObjectSpawn + accion Item en combate
examples/demo_dynamic_lights.cpp     Fase 11: DynamicLightMap (luces animadas) + sombras blob + post-FX
examples/demo_editor_state.cpp       Fase 12 (sin GL): EditorState + round-trips reales TMX/JSON
examples/demo_proyectos.cpp          Proyectos (sin GL): ProjectIndex + ProjectHub, la pantalla de arranque entera
examples/level_editor.cpp            Fase 12: editor visual (pantalla de proyectos + raton + HUD propio; G guarda TMX + JSON)
examples/demo_game_session.cpp       Partida (sin GL): ciclo exploracion <-> combate completo
examples/juego.cpp                   EL JUEGO: main sobre Application (WASD explorar, W/S+ENTER combatir; --nivel/--catalogo)

assets/shaders/sprite.{vert,frag}    Shader minimo de demo_textured_quad/demo_isometric_renderer, reutilizado por el HUD
assets/shaders/lightmap.{vert,frag}  Fase 4: fullscreen-triangle + combinacion escena*luz+ambient
assets/shaders/fog.{vert,frag}       Fase 4: fullscreen-triangle + mezcla escena/niebla con tinte
assets/shaders/lut.{vert,frag}       Fase 4: fullscreen-triangle + lookup en cubo RGB cuantizado
assets/textures/test_checker.png     Textura de prueba (8x8, tablero rojo/amarillo)
assets/maps/test_map.tmx             Mapa TMX de prueba (4x3, 1 capa, colision en 2 celdas)
assets/maps/test_map_invalid.tmx     Fixture de TMX invalido, para probar Result::Error
assets/levels/test_level.json        Fase 6/10: JSON de nivel de ejemplo (mapa + 2 enemigos + 4 objetos)
assets/objects/test_objects.json     Fase 10: catalogo de objetos de ejemplo (arbusto/llave/pocion/eter/slime)

third_party/glad/     GLAD (loader de OpenGL 3.3 Core), vendorizado y comprobado en el repo
third_party/stb/      stb_image.h (carga de PNG/JPG...), vendorizado y comprobado en el repo
third_party/tinyxml2/ tinyxml2 (parser XML, para TMX), vendorizado y comprobado en el repo

```

`.github/workflows/{ci,release}.yml`, `.clang-format`, `.clang-tidy` y
`.pre-commit-config.yaml` viven en la raíz del repositorio (un nivel por
encima de este directorio), no aquí: GitHub Actions solo ejecuta workflows
ubicados en `.github/workflows/` en la raíz. Ver `../BRANCHING.md`.

## Qué está verificado y qué no

- **`ResourceManager<T>`, `Result<T>`, `EngineException`, y el ejemplo `TextAssetManager`/`demo_resource_manager.cpp` están compilados y
  ejecutados** en este mismo entorno (g++ 13, `-std=c++17 -Wall -Wextra`,
  cero warnings, todos los `assert()` pasan; también verificado con
  `-fsanitize=address,undefined`, limpio). Cubre: carga OK, cache por
  `id`, error controlado sin excepción visible para el llamador,
  `get()`/`contains()`/`unload()`/`clear()`.
- **`Camera` (paneo suavizado, zoom, `getViewProjectionMatrix()`,
  `screenToWorld()`, `worldToGrid()`) e `IsoMath` (proyección 2:1
  grid↔screen) están compilados y ejecutados** (`demo_camera.cpp`, target
  `motor_math`, solo depende de `glm` vía `FetchContent`): round-trip
  `screenToGrid(gridToScreen(g)) == g` exacto en una cuadrícula de prueba
  de 41×41 celdas, convergencia del lerp de paneo, e inversa exacta de
  `screenToWorld()` para un zoom≠1. Limpio también con
  `-fsanitize=address,undefined`, `clang-format` y `clang-tidy`.
- **`Texture`/`TextureManager`, `Shader`/`ShaderManager`, `Window`,
  `SpriteBatch` y el quad texturizado completo están compilados,
  enlazados y ejecutados con un contexto OpenGL 3.3 Core real**
  (`demo_textured_quad.cpp`, sobre Xvfb + Mesa/llvmpipe software
  rendering, ya que este entorno no tiene GPU/display): `GL_VERSION`
  reportado es `4.5 (Core Profile) Mesa ...`; se carga
  `assets/textures/test_checker.png` (8×8, confirmado por
  `Texture::getWidth()/getHeight()`), se compila/enlaza
  `assets/shaders/sprite.{vert,frag}`, y se vuelca el framebuffer final a
  un PPM que se inspeccionó pixel a pixel: la esquina tiene el color de
  `glClearColor` y el centro el color exacto de la textura — confirma que
  el pipeline completo (Window → Camera → SpriteBatch → Shader →
  Texture) dibuja lo que debería, no solo que "compila". Limpio con
  ASan+UBSan (con `detect_leaks=0` solo para esta demo: las fugas que
  reporta LeakSanitizer son de la cache interna de Mesa/llvmpipe — todos
  los frames del stack están dentro del driver, no de este código).
- **`TileMap` (parser TMX real vía tinyxml2), `Tile` y `TextureAtlas`
  están compilados y ejecutados** (`demo_tilemap.cpp`, target
  `motor_map`; necesita GLAD para el destructor de `Texture` pero NO
  GLFW/ventana, así que corre sin Xvfb): carga
  `assets/maps/test_map.tmx` (tileset embebido, capa CSV, colisión por
  tile vía `<properties>`) y comprueba celda a celda GID y colisión;
  `gridToScreen`/`screenToGrid` con el `tilewidth`/`tileheight` del
  propio TMX; `getTile()` fuera de rango lanza `std::out_of_range`; un
  TMX con menos celdas de las declaradas (`test_map_invalid.tmx`) y un
  fichero inexistente devuelven `Result<bool>::Error` en vez de
  crashear. `TextureAtlas::defineRegion()`/`getUV()` verificados sobre
  un atlas 16×8 con dos regiones de 8×8. Limpio con ASan+UBSan
  (detección de fugas activada: aquí sí, no toca GL de verdad).
  `TileMap::visibleRange()` (culling, la última tarea de la Fase 2)
  también verificado en `demo_tilemap.cpp`: un viewport enorme centrado
  en el mapa cubre las 4×3 celdas exactas, y una cámara muy lejos del
  mapa da un rango vacío (`GridBounds::isEmpty()`).
- **`IRenderable`/`IUpdatable`, `Entity` e `IsometricRenderer` (arranque
  de la Fase 3) están compilados y ejecutados con un contexto OpenGL
  real** (`demo_isometric_renderer.cpp`, sobre Xvfb): dibuja
  `assets/maps/test_map.tmx` completo (con culling real vía
  `TileMap::visibleRange()`) más tres entidades de prueba, y el
  framebuffer final se inspeccionó pixel a pixel (fondo en las esquinas,
  colores de la textura presentes en el resto) para confirmar que se
  dibuja de verdad. `IsometricRenderer::sortQueue()` (Painter's
  Algorithm) se verifica aparte, sin necesitar un frame de render: tres
  entidades se insertan fuera de orden y, tras `sortQueue()`,
  `renderQueue()` queda ordenada por profundidad ascendente exactamente
  como predice la fórmula `(grid_x+grid_y)*tileHeight/2` del diagrama de
  clases. `glGetError()` limpio tras el render. Limpio con ASan+UBSan
  (mismo caso de `detect_leaks=0` que `demo_textured_quad`, por Mesa).
- **`AnimatedEntity`/`Player`/`Enemy` (cierre de la Fase 3) están
  verificados de forma aislada** (ciclo de animación por frames,
  movimiento discreto de `Player::handleInput`, salud/inventario,
  patrulla determinista de `Enemy::update` con inversión de dirección en
  ambos límites, y el caso borde de un rango de patrulla degenerado)
  compilando y enlazando `AnimatedEntity.cpp`/`Player.cpp`/`Enemy.cpp`
  junto a `Entity.cpp`/`TextureAtlas.cpp`/`Texture.cpp`/`glad.c` con
  `g++ -fsanitize=address,undefined`, sin necesitar contexto OpenGL real
  (un `Texture` con `glID=0` no llama a `glDeleteTextures` en su
  destructor, ver su comentario): limpio, todos los `assert()` pasan.
  **`demo_animated_entity.cpp`** integra las tres clases en el pipeline
  completo (`Window`+`Camera`+`TileMap`+`IsometricRenderer`, mismo patrón
  que `demo_isometric_renderer.cpp`), y está **compilado, enlazado y
  ejecutado con un contexto OpenGL real** (GLFW 3.4 + GLAD, glm 1.0.1 vía
  `FetchContent`): corre N frames con `glGetError() == GL_NO_ERROR`, genera
  `demo_animated_entity_output.ppm` (1280×720, inspeccionado: fondo
  `glClearColor` en las esquinas y color de textura en el centro, igual que
  `demo_textured_quad`/`demo_isometric_renderer`) y sus tres bloques de
  aserciones (`[ANIM]`, `[PLAYER]`, `[ENEMY]`) pasan. Limpio también bajo
  AddressSanitizer + UBSan (verificado en macOS con OpenGL nativo, donde
  LeakSanitizer sí funciona: cero fugas del proyecto).
- **`Framebuffer`, tint por vértice (`SpriteBatch`/`Entity`) y el
  post-procesado de `IsometricRenderer` (`setPostFX`/`applyPostProcessing`,
  arranque de la Fase 4) están compilados, enlazados y ejecutados con un
  contexto OpenGL real** (`demo_lighting.cpp`, sobre OpenGL nativo de
  macOS, `GL_VERSION = 4.1 Metal`): crea un lightmap procedural
  (degradado radial), activa el post-FX y confirma `glGetError() ==
  GL_NO_ERROR` tras el fullscreen-triangle del shader `lightmap`. El PPM
  resultante muestra el efecto de iluminación de verdad (inspeccionado
  con un mapa de luminancia: escena brillante en el centro donde el
  lightmap alcanza ~1.0, fondo azul-noche oscurecido por la luz ambiente
  en los bordes, sin negro puro gracias a `uAmbient` y al
  `setSceneClearColor()` de más abajo). Dos aserciones permanentes en el
  propio demo lo cubren sin inspeccionar a mano: la esquina `(5,5)` debe
  ser `> 0` (fondo no negro) y el centro `> 100` (escena iluminada).
  Limpio también bajo AddressSanitizer + UBSan (cero fugas: el
  `Framebuffer` libera su FBO + textura + VAO vacío, y su move-semantics
  está verificada). Los demos sin post-FX (`demo_textured_quad`,
  `demo_isometric_renderer`, `demo_animated_entity`) siguen idénticos:
  el tint por defecto `{1,1,1,1}` deja el framebuffer sin cambios
  (mismas esquinas/centro que antes de la Fase 4, confirmado).
- **`FogOfWar` (segundo efecto de la Fase 4, niebla de guerra con memoria
  Hidden/Explored/Visible) y su integración en `IsometricRenderer`
  (`setFog`/`setFogColor`/`setFogEnabled`) están compilados, enlazados y
  ejecutados con un contexto OpenGL real** (`demo_fog.cpp`, sobre OpenGL
  nativo de macOS, `GL_VERSION = 4.1 Metal`): revela un radio Chebyshev de 1
  celda alrededor del `Player` cada frame (`beginFrame` degrada
  Visible→Explored, `reveal` vuelve a marcar Visible la visión actual,
  `updateTexture` rasteriza el estado a una textura al tamaño del viewport).
  La lógica de memoria (lo visible se recalcula cada frame, lo explorado se
  queda) está cubierta por aserciones en el propio demo (con dimensiones de
  test fijas 5×5, independientes de `test_map.tmx`). El PPM muestra el
  efecto de verdad: zona visible (escena brillante, junto al jugador) y
  resto oculto (tinte azulado `uFogColor`), confirmado por dos aserciones
  permanentes (centro `> 80` = escena visible, esquina = tinte de niebla).
  Limpio bajo AddressSanitizer + UBSan (cero fugas: la textura de niebla se
  libera en `~FogOfWar`). Los demos sin niebla siguen idénticos (fog off
  por defecto).

### Corrección aplicada: `IsometricRenderer::setFog()` esperaba un `Texture*`

`demo_fog.cpp` llama a `renderer.setFog(fogShader.value(), &fog, width,
height)` pasando un `FogOfWar*`, pero `setFog()` declaraba su segundo
parámetro como `Texture*` (copiado del patrón de `setPostFX()`, que sí
recibe un lightmap como `Texture` normal): no compilaba
(`cannot convert 'FogOfWar*' to 'Texture*'`). A diferencia del lightmap,
`FogOfWar` no envuelve una `Texture`: gestiona su propio nombre GL
internamente y expone `bind(slot)` directamente, así que forzar un
`Texture*` de por medio habría sido más artificial que arreglar la
firma. Se cambió `setFog()`/`m_fog` (renombrado desde `m_fogTexture`) a
`FogOfWar*`, y `applyPostProcessing()` llama a `m_fog->bind(1)` en vez
de `m_fogTexture->bind(1)` — mismo contrato (`bind(unsigned int slot)
const`) que `Texture::bind()`, solo que la clase que lo implementa es
otra. `demo_fog.cpp` no necesitó ningún cambio: su llamada ya era la
correcta, era la firma del motor la que estaba mal.

### Corrección aplicada: flip Y en `FogOfWar::updateTexture()`

`updateTexture()` rasteriza el estado de cada celda a una textura al tamaño
del viewport, pintando un rectángulo en la posición de pantalla del tile.
El cálculo usaba `worldToScreen()` (inversa de `Camera::screenToWorld()`,
que trabaja en coords de **ventana**, origen arriba-izquierda), pero el
buffer `pixels` se sube a una textura GL, que se mapea con origen
**abajo**-izquierda (fila 0 = abajo), y el shader `fog.vert` genera
`vScreenUV` con `1.0 - pos.y` justamente para compensar ese flip al
samplear. Sin corregirlo, la niebla quedaba reflejada verticalmente: como
solo hay contenido cerca del jugador (no en toda la textura), el efecto era
que la zona visible nunca caía donde el shader sampleaba → parecía que la
niebla estaba siempre "oculta" en toda la pantalla. El fix invierte Y al
mapear fila de pantalla → fila de buffer (`m_viewportHeight - yVent`).
Fue sutil porque con una textura toda blanca (caso de diagnóstico) el bug
no se nota: solo aflora cuando hay contenido localizado, como la niebla.

### Corrección aplicada: fondo negro puro con el post-FX activado

`IsometricRenderer::renderFrame()` limpiaba el FBO de escena
(`m_sceneFbo`) siempre a negro puro (`{0,0,0,1}`) antes de dibujar,
ignorando el `glClearColor` que el llamador ya había fijado sobre el
framebuffer de la ventana. El shader `lightmap` combina
multiplicativamente (`escena * max(luz, uAmbient)`, ver
`assets/shaders/lightmap.frag`): en cualquier pixel de fondo sin dibujar
nada, el color de escena era negro, y **ningún** valor de luz —ni
siquiera `uAmbient`— puede aclarar un `0 * lo_que_sea`. El resultado
(confirmado inspeccionando `demo_lighting_output.ppm`: fondo negro puro
en vez del azul noche del resto de demos) no era un fallo del shader ni
del `Framebuffer`, sino que `uAmbient` solo evita que la luz *oscurezca*
por debajo de un mínimo — no puede *aclarar* un pixel que ya era negro.

Se añadió `IsometricRenderer::setSceneClearColor()` (por defecto sigue
siendo negro, mismo comportamiento que antes si nadie lo llama) y
`demo_lighting.cpp` ahora lo fija al mismo azul noche que usa como
`glClearColor` de la ventana, más una aserción permanente (sobre el
pixel `(5,5)`, fuera del mapa de 4×3 celdas, que ya no debe ser negro
puro) que deja esta regresión cubierta en el propio demo en vez de
depender de inspeccionar el PPM a mano.

### Corrección aplicada: bug de move-semantics en `Texture`

El `Texture(Texture&&) = default` original **no** pone a cero `m_glID` en
el objeto de origen. Como el destructor llama a `glDeleteTextures(m_glID)`
sin comprobar más que `!= 0`, tras un move ambos objetos (el movido-desde
y el destino) creían poseer el mismo nombre de textura GL: los dos
destructores intentarían borrarlo → doble-free / comportamiento
indefinido. Se sustituyó por un move explícito que "roba" el recurso y
deja el origen en `m_glID = 0`, igual que `std::unique_ptr`.

## Compilar y ejecutar los demos verificados

GLAD y stb_image ya están vendorizados en `third_party/` (comprobados en
el repo, ver "GLAD/stb_image vendorizados" más abajo), así que
`motor_core` se activa solo. En Linux hacen falta los headers de
desarrollo de X11/GL que usa GLFW (`sudo apt install xorg-dev
libgl1-mesa-dev`, o el equivalente de tu distro):

```
mkdir build && cd build
cmake ..
cmake --build .
./demo_resource_manager
./demo_camera
./demo_tilemap              # sin GL real: no necesita Xvfb
./sandbox_window            # ventana interactiva: se cierra a mano
./demo_textured_quad        # ventana interactiva: se cierra a mano
./demo_isometric_renderer   # ventana interactiva: se cierra a mano
```

`demo_camera` y `sandbox_window`/`demo_textured_quad` (via `motor_core`)
descargan `glm` y `GLFW` respectivamente vía `FetchContent` en la
configuración de CMake (requiere red la primera vez; luego queda
cacheado en `build/`).

Sin `cmake` a mano, `demo_resource_manager` también compila directo (no
tiene dependencias externas):

```
g++ -std=c++17 -Wall -Wextra -Iinclude -o demo examples/demo_resource_manager.cpp examples/TextAssetManager.cpp
./demo
```

`demo_level_loader` y `demo_skills` (Fases 6/7, RPG) son igual de
independientes — GL-free, sin `glm` ni ningún `FetchContent` — así que
también compilan directo con `g++`:

```
g++ -std=c++17 -Wall -Wextra -Iinclude -o demo_level_loader examples/demo_level_loader.cpp src/Core/Json/JsonValue.cpp src/Render/LevelLoader.cpp
./demo_level_loader   # ejecutar desde MotorGraphico/ para que resuelva assets/levels/test_level.json

g++ -std=c++17 -Wall -Wextra -Iinclude -o demo_skills examples/demo_skills.cpp src/Render/Skill.cpp
./demo_skills

g++ -std=c++17 -Wall -Wextra -Iinclude -o demo_battle examples/demo_battle.cpp src/Render/Skill.cpp src/Render/BattleState.cpp src/Render/ObjectCatalog.cpp src/Core/Json/JsonValue.cpp
./demo_battle
# (desde la Fase 10, BattleState referencia ObjectCatalog -- accion Item --
#  y ObjectCatalog usa JsonValue: por eso ambos .cpp van en el enlace)

g++ -std=c++17 -Wall -Wextra -Iinclude -o demo_object_catalog examples/demo_object_catalog.cpp src/Core/Json/JsonValue.cpp src/Render/LevelLoader.cpp src/Render/Skill.cpp src/Render/BattleState.cpp src/Render/ObjectCatalog.cpp
./demo_object_catalog   # ejecutar desde MotorGraphico/ para que resuelva assets/objects/ y assets/levels/

# demo_editor_state necesita ademas motor_map (round-trip TMX contra el
# TileMap real) -> tinyxml2 + Camera (glm; ajustar la ruta -I de glm a
# donde la tenga FetchContent, ej. build/_deps/glm-src):
g++ -std=c++17 -Wall -Wextra -Iinclude -Ibuild/_deps/glm-src -Ithird_party/tinyxml2 -o demo_editor_state examples/demo_editor_state.cpp src/Render/EditorState.cpp src/Core/Json/JsonValue.cpp src/Render/LevelLoader.cpp src/Render/TileMap.cpp src/Core/Resources/TextureAtlas.cpp src/Render/Camera.cpp third_party/tinyxml2/tinyxml2.cpp
./demo_editor_state

# demo_game_session: el ciclo de juego completo, tambien sin GL.
g++ -std=c++17 -Wall -Wextra -Iinclude -Ibuild/_deps/glm-src -Ithird_party/tinyxml2 -o demo_game_session examples/demo_game_session.cpp src/Render/GameSession.cpp src/Render/BattleState.cpp src/Render/Skill.cpp src/Render/ObjectCatalog.cpp src/Render/LevelLoader.cpp src/Core/Json/JsonValue.cpp src/Render/TileMap.cpp src/Core/Resources/TextureAtlas.cpp src/Render/Camera.cpp third_party/tinyxml2/tinyxml2.cpp
./demo_game_session   # ejecutar desde MotorGraphico/ (usa assets/maps/test_map.tmx)
```

### Sin display (CI, contenedores, SSH sin X)

`demo_textured_quad` y `demo_isometric_renderer` aceptan un número de
frames como argumento: corren exactamente esos frames, vuelcan el
framebuffer a un PPM y salen solos, sin esperar a que se cierre la
ventana — así es como los ejecuta `ci.yml`. Con `Xvfb` (pantalla
virtual) y renderizado por software:

```
sudo apt install xvfb
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a ./build/demo_textured_quad 5
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a ./build/demo_isometric_renderer 5
```

### macOS (OpenGL nativo, sin Xvfb)

En macOS no hace falta Xvfb ni Mesa: hay `OpenGL.framework` nativo y GLFW
usa el backend Cocoa. El código ya lo contempla
(`src/Engine/Window.cpp` activa `GLFW_OPENGL_FORWARD_COMPAT` con
`#ifdef __APPLE__`, y `CMakeLists.txt` solo desactiva Wayland bajo
`UNIX AND NOT APPLE`). Solo hace falta CMake (las Command Line Tools dan
clang + Cocoa):

```
brew install cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/demo_resource_manager
./build/demo_camera
./build/demo_tilemap
./build/demo_textured_quad 5           # GL_VERSION reporta "4.1 Metal"
./build/demo_isometric_renderer 5
./build/demo_animated_entity 5
./build/demo_lighting 5
./build/demo_fog 5
```

Verificado en macOS 26.5 con Apple clang 21: los siete demos sin niebla
corren con `glGetError() == GL_NO_ERROR` y sus PPM tienen el contenido
correcto (fondo en esquinas, textura en el centro). Requiere una sesión
gráfica activa (WindowServer); sobre SSH sin reenvío de display los
demos con ventana no podrán crear el contexto.

`demo_lighting` (Fase 4, 1er efecto) corre igual aquí: `glGetError() ==
GL_NO_ERROR`, sus dos aserciones (esquina no negra + centro iluminado)
pasan, y el PPM muestra el efecto de iluminación. Nota: captura el
framebuffer muestreando con sondas de 1 píxel sobre una grilla (160×90)
en vez de un `glReadPixels` masivo, porque en este backend Metal una
lectura completa tras el fullscreen-triangle del post-FX devuelve
contenido incompleto; las sondas puntuales sí funcionan. `demo_fog.cpp`
(Fase 4, 2o efecto) reutiliza el mismo helper por el mismo motivo.

`demo_fog` en concreto todavía no se ha vuelto a correr en macOS tras el
fix de `setFog()` (ver "Corrección aplicada" más arriba): antes del fix
no compilaba, así que no hay una ejecución previa con la que comparar —
pendiente de la primera corrida.

### GLAD/stb_image/tinyxml2 vendorizados

`third_party/glad/`, `third_party/stb/` y `third_party/tinyxml2/` están
en el repositorio (no son un paso manual de cada dev). Se generaron/
descargaron una vez con:

```
pip install glad
python -m glad --profile core --api "gl=3.3" --generator c --out-path third_party/glad
curl -o third_party/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -o third_party/tinyxml2/tinyxml2.h https://raw.githubusercontent.com/leethomason/tinyxml2/master/tinyxml2.h
curl -o third_party/tinyxml2/tinyxml2.cpp https://raw.githubusercontent.com/leethomason/tinyxml2/master/tinyxml2.cpp
```

Si algún día hace falta regenerarlos (nueva versión de OpenGL, otra API,
actualizar tinyxml2), son esos comandos; `CMakeLists.txt` los detecta
automáticamente por la presencia de `third_party/glad/src/glad.c` y
`third_party/stb/stb_image.h`, sin tocar el propio `CMakeLists.txt`.

## Sistema RPG (niveles JSON, habilidades, HUD)

Roadmap y clases nuevas en [`motor_grafico_gantt_rpg.puml`](motor_grafico_gantt_rpg.puml)
y `motor_grafico_clases.puml` (sección "RPG: niveles, habilidades y HUD"):
la continuación del motor genérico hacia la base jugable de un RPG por
turnos estilo Dragon Quest clásico, con contenido (niveles, enemigos,
habilidades) dirigido por datos (JSON) en vez de hardcodeado. Cuatro
fases (6-9); estado real de cada una:

**Fase 6 (sistema de niveles y contenido): implementada y verificada de
verdad.** `JsonValue` (`Core::Json`, parser JSON propio sin dependencias
de terceros — no hay red disponible en el entorno en el que se escribió
para vendorizar nlohmann/json, así que un parser propio de objeto/array/
string/número/bool/null resultó más simple que añadir la dependencia) y
`LevelLoader`/`LevelDefinition`/`EnemySpawn` (JSON → datos de nivel:
mapa TMX referenciado, `playerStart`, enemigos con posición/patrulla/
habilidades opcionales) son **GL-free**, igual que `Camera`: se
compilaron y **ejecutaron de verdad** con `g++ -std=c++17 -Wall -Wextra
-Werror` (cero warnings) y bajo `-fsanitize=address,undefined` (limpio),
sin necesitar `cmake` ni ventana. `demo_level_loader.cpp` cubre parseo
JSON anidado, accesores con default seguro ante tipos/campos que no
coinciden, errores de sintaxis como `Result::Error` (nunca una
excepción), el esquema de nivel completo con valores por defecto de
campos opcionales, y los errores fatales (falta `"map"`, `"enemies"` con
tipo incorrecto, enemigo sin `"type"`). Ver `assets/levels/test_level.json`
para el formato.

**Fase 7 (sistema de habilidades): implementada, incluida la conexión
real con `Player`/`Enemy`.** `Skill`/`SkillSet`/`SkillCatalog` (`Render`,
GL-free) cubren los datos (nombre, coste de maná, poder, efecto,
objetivo) y la lógica de "qué puede usar una entidad y con qué maná"
(`learn`/`knows`/`canUse`/`spend`/`restoreMana`), más un catálogo por id.
La pieza que faltaba —una interfaz común para "recibir un efecto de una
Skill"— ya está: `ICombatant` (`takeDamage`/`heal`/`health`/`maxHealth`/
`isAlive`), que `Player` y `Enemy` ya implementan (ambos tenían ya los
métodos con la misma firma; solo hacía falta formalizar el contrato). De
paso se añadió `maxHealth()` a los dos (antes no existía) y se corrigió
un bug real en `Player::heal()`: no clampaba contra ningún máximo, así
que una curación de sobra podía dejar `m_health` por encima de lo que el
propio `Player` considera su vida máxima — con `ICombatant::maxHealth()`
ya en uso (`ApplySkillEffect()`, un futuro `HudBar` de vida) ese bug
dejaba de ser invisible. `ApplySkillEffect(skill, target)` (`Skill.h`)
aplica el efecto de verdad (`Damage`→`takeDamage`, `Heal`→`heal`) y
devuelve la magnitud real aplicada, no `skill.power` directo (si el
objetivo tiene menos vida que el daño, o menos hueco que la curación, el
clamp de `takeDamage()`/`heal()` se refleja en el valor devuelto). **Todo
esto se verificó de verdad** (`g++ -Wall -Wextra -Werror` limpio +
`-fsanitize=address,undefined` limpio, `demo_skills.cpp`, contra un
`ICombatant` de prueba — no hace falta un `Player`/`Enemy` real con
`TextureAtlas` para probar el contrato); `Player.cpp`/`Enemy.cpp` en sí
solo se pudieron **syntax-check** (`-fsyntax-only`, necesitan GLAD como
el resto de `motor_core`), sin regresión frente al resto de demos
existentes.

**Fase 8 (combate por turnos): MVP implementado y verificado de
verdad.** `BattleState` (`Render`, GL-free — opera sobre `ICombatant`/
`SkillSet`/`SkillCatalog`, nunca sobre `Player`/`Enemy` concretos, mismo
motivo que `LevelLoader`/`Skill`) resuelve una cola de turnos simple sin
iniciativa/velocidad (`resolveAllyAction()` por aliado +
`resolveEnemyTurn()` para todos los enemigos de una vez), con tres
acciones (`Attack` fijo, `UseSkill` vía `ApplySkillEffect()`, `Flee`
inmediata — `Objeto` queda fuera a propósito: los ítems del inventario
del `Player` no tienen ningún efecto asociado todavía, eso es
justamente la Fase 10 de más abajo) y una IA mínima determinista: cada
enemigo usa la primera habilidad que puede pagar (`SkillSet::
knownSkillIds()`, nuevo, orden alfabético — sin eso la elección
dependería del orden interno no especificado de un `unordered_map`,
imposible de testear) y si no puede pagar ninguna, ataque básico.
Desenlaces `Victory`/`Defeat`/`Fled`, con log de texto por acción (para
el futuro cuadro de diálogo del HUD). **Verificado de verdad**: `g++
-Wall -Wextra -Werror` limpio + `-fsanitize=address,undefined` limpio,
`demo_battle.cpp` (ataque básico, éxito/fallo de `UseSkill`, IA
cambiando de habilidad a ataque básico al quedarse sin maná, los tres
desenlaces, índices de aliado/objetivo fuera de rango o ya muertos como
no-op sin crash, y que ninguna acción muta nada después de que el
combate terminó). Los dos pendientes que dejaba este párrafo ya están
cerrados: la acción `Objeto` (Fase 10, más abajo) y la transición
automática exploración↔combate (`GameSession`/`Application`, al final de
esta sección).

**Fase 9 (HUD modular): completa, incluido el texto.** `IHudElement`/
`HudTransform`/`HudAnchor` (`HudElement.h`, anclas de 9 puntos +
desplazamiento, resueltas a píxeles de pantalla) y `HudManager`
(composición por orden de inserción, proyección ortográfica en píxeles
de pantalla — **independiente de `Camera`**, el HUD no se mueve ni
escala con el zoom/paneo del mundo — reutilizando el shader `sprite` ya
existente, sin shader nuevo) más los widgets de color (`HudBar` para
vida/maná, `HudPanel` como contenedor) están en `examples/demo_hud.cpp`.
La lógica de anclas (`HudTransform::resolveTopLeft()`) es pura y **se
verificó de verdad**: se extrajo a un binario suelto fuera del árbol de
ejemplos y se comprobaron los 9 anchors (incluyendo que el offset empuja
hacia el borde derecho/inferior restando, no sumando) antes de incrustar
las mismas aserciones en `demo_hud.cpp`.

Encima de eso, `BitmapFont` (`BitmapFont.h`) cierra el hueco de texto que
quedaba abierto: fuente 5×7 monoespaciada, solo mayúsculas + dígitos +
puntuación básica (una fuente con minúsculas dobla la tabla de glifos sin
añadir legibilidad útil a un HUD retro estilo Dragon Quest, que va en
mayúsculas de todas formas — `drawText()` convierte a mayúsculas sola, el
llamador no tiene que acordarse), con un atlas **procedural** —
`generateAtlas()`, mismo patrón que `makeProceduralLightmap()`/
`makeWhiteTexture()` — en vez de vendorizar una imagen de fuente (no hay
red en este entorno de trabajo para descargarla, mismo motivo que el
parser JSON propio de la Fase 6). Sus dos métodos puros, `glyphUV()` y
`measureText()`, **se verificaron de verdad** (`g++ -Wall -Wextra
-Werror` limpio + `-fsanitize=address,undefined` limpio,
`demo_bitmap_font.cpp`: celdas de atlas conocidas, minúscula/mayúscula
tratadas igual, caracteres fuera de rango cayendo en la celda del
espacio sin crashear, `measureText()` con una línea/multilínea/cadena
vacía) — el resto de la clase (`generateAtlas()`/`drawText()`/el
constructor) necesita GL real, así que ese demo linka `BitmapFont.cpp`
+ `glad.c` **sin construir nunca un `BitmapFont`**, solo para que el
enlazador resuelva los símbolos (mismo patrón que `demo_tilemap.cpp`).

Sobre `BitmapFont` se montan los tres widgets de texto que faltaban
(`HudTextWidgets.h`): `HudText` (una o varias líneas en una posición
fija), `HudCommandMenu` (menú navegable Atacar/Habilidad/Objeto/Huir,
estilo Dragon Quest clásico, con `moveUp()`/`moveDown()` con wraparound y
un cursor de texto `"> "` además del color de resaltado — no depende solo
del color) y `HudDialogueBox` (panel + últimas N líneas de un log de
texto, pensado exactamente para `BattleState::log()`). El demo de cierre,
`examples/demo_battle_hud.cpp`, integra las Fases 6-9 de una vez: un
`BattleState` real (héroe con una `Skill` contra un slime) resuelto y
volcado en un `HudDialogueBox`, con `HudCommandMenu`/`HudBar` de vida
también en pantalla. `HudCommandMenu::moveUp()`/`moveDown()` (wraparound)
y el volcado de `BattleState::log()` en `HudDialogueBox::setLines()` **se
verificaron de verdad sin ventana** (misma técnica que
`testHudTransform()` en `demo_hud.cpp`: lógica pura extraída y probada
antes del pipeline con contexto real) — el pipeline completo con `Window`
solo se pudo **verificar en modo sintaxis** (`g++ -fsyntax-only -Wall
-Wextra -Werror`, sin errores, igual que `demo_hud.cpp`/`demo_lut.cpp`),
pendiente de `./build/demo_battle_hud 5` en macOS.

**Fase 10 (catálogo de objetos, "todo es un objeto"): implementada y
verificada de verdad.** `ObjectCatalog`/`ObjectDefinition`
(`ObjectCatalog.h`, GL-free como todo `motor_level`): un catálogo único
por id donde una llave, un arbusto y un enemigo son la misma cosa —un
`ObjectDefinition`— con distinta categoría (`Prop`/`Enemy`/`Pickup`) y
datos por categoría (`CombatData` con vida/maná/habilidades por TIPO de
enemigo, `PickupData` con efecto `Heal`/`RestoreMana`/`None` — `None` es
una llave: se tiene o no, su uso real es lógica de nivel). Los datos por
categoría son miembros planos, no un `std::variant` (solo dos categorías
con datos; un variant complicaría cada acceso sin ganar nada hoy). La
carga desde JSON (`loadFromString`/`loadFromFile`, sobre el `JsonValue`
de la Fase 6) es **todo-o-nada**: una entrada inválida (sin `"id"`,
categoría o efecto desconocidos) devuelve `Result::Error` sin aplicar
ninguna, y cargar varios archivos hace merge (id repetido: el último
gana). Sobre el catálogo se apoyan las otras dos piezas de la fase:
`ObjectSpawn` en `LevelDefinition` (un `"objects"` genérico en el JSON de
nivel que convive con `"enemies"` — los niveles existentes siguen
cargando sin migración) y la acción **`Objeto` en `BattleState`**
(`BattleActionType::Item` + `BattleParticipant::inventory`, puntero no
propietario al inventario real del actor): consume UNA instancia del
item, aplica el efecto de `PickupData` con clamp real (curar 15 con 5 de
hueco cura 5, mismo criterio que `ApplySkillEffect()`), y todos los
fallos (item que no se tiene, id que no está en el catálogo, categoría
no-Pickup, efecto `None`) dejan línea en el log sin consumir nada. Nota
de compatibilidad: `itemId` se añadió al FINAL de `BattleAction` a
propósito — los llamadores existentes usan llaves posicionales
(`{type, skillId, targetIndex}`) e insertar un `std::string` en medio
los rompía en silencio (el `0` de `targetIndex` pasaría a inicializar un
string desde un puntero nulo: compila y crashea). **Verificado de
verdad**: `g++ -Wall -Wextra -Werror` limpio +
`-fsanitize=address,undefined` limpio, `demo_object_catalog.cpp` (las
tres categorías con defaults, merge con sobreescritura, todos los
errores estructurales sin mutar el catálogo, los JSON reales del repo
—`assets/objects/test_objects.json`, `assets/levels/test_level.json`
ampliado—, y el combate con poción/éter/llave incluyendo el consumo real
del inventario), más regresión de `demo_battle`/`demo_level_loader`/
`demo_skills` (todas pasan sin cambios de comportamiento). Fuera de
alcance de esta fase (documentado en el Gantt): equipo (armas/armaduras
que cambien el daño del ataque básico).

**Fase 11 (iluminación avanzada + sombras): implementada.**
`DynamicLightMap`/`PointLight` (`DynamicLightMap.h`): N focos de luz
puntuales en coordenadas de mundo, con radio, intensidad y **color** (el
shader `lightmap` de la Fase 4 ya combina en RGB, así que luces de color
salen gratis), compuestos en CPU a una textura de lightmap por frame
(`updateTexture()`, mismo patrón exacto que `FogOfWar::updateTexture()`
incluido el flip Y, con cada luz acotada a su bounding-box en pantalla y
acumulación aditiva en float antes del clamp). La clave del diseño: la
textura resultante se enchufa **tal cual** al pase de post-FX existente —
`setPostFX(shader, lightMap.texture(), ...)` donde `demo_lighting.cpp`
pasaba su degradado fijo — **cero cambios en renderer ni shader** para
pasar de "un lightmap global fijo" a "N luces moviéndose". Las sombras
son **blob** (`BlobShadow.h`): elipse oscura difusa a los pies de cada
entidad (textura procedural radial, el color lo pone el tint — misma
filosofía que la textura blanca 1×1 del HUD), vía
`IRenderable::renderShadow()` (virtual con cuerpo vacío, opt-in: nada
existente se rompe) que `Entity` implementa, dibujadas por
`IsometricRenderer` después de los tiles y **antes de todas las
entidades** (una sombra nunca pisa un sprite vecino). Nota: son blobs,
no sombras proyectadas con oclusión real — lo estándar en isométrico 2D
de esta estética. Cambio de motor asociado: `renderFrame()` activa ahora
`GL_BLEND` (alpha estándar) durante el batch de escena — hasta aquí nada
del mundo dibujaba con alpha < 1, así que no estaba activado; con
blending, lo opaco se dibuja idéntico (alpha 1 = src·1 + dst·0), cero
regresión visual. **Verificado**: la lógica pura (`attenuationAt()`,
`sampleLightAt()` — aditiva, clamp por canal, intensity, radio
degenerado sin división por cero — y `FillBlobShadowPixels()` — radial,
monótona, RGB blanco) **se compiló y ejecutó de verdad** con `g++ -Wall
-Wextra -Werror` + `-fsanitize=address,undefined`, limpio, en un binario
suelto (misma técnica que `HudTransform`), y las mismas aserciones están
incrustadas en `demo_dynamic_lights.cpp`; el pipeline con `Window` real
(luces orbitando + sombras + sondas de píxel "centro más iluminado que
esquina") solo se pudo **verificar en modo sintaxis** (`-fsyntax-only`
limpio), pendiente de `./build/demo_dynamic_lights 5` en macOS.
Regresión: todos los demos GL-free re-ejecutados (pasan) y todo el set
GL re-syntax-checkeado (limpio).

**Fase 12 (editor de niveles visual): implementada, con una desviación
documentada.** El Gantt planeaba la UI con Dear ImGui vendorizado; sin
red en este entorno es imposible descargarlo, así que en vez de bloquear
la fase la UI se montó con el **HUD propio de la Fase 9**
(`HudPanel`/`HudText`/`BitmapFont`) — y el diseño lo aprovecha: el
núcleo del editor (`EditorState`, GL-free) no sabe nada de widgets ni de
GL, solo del modelo del nivel (grid de tiles, objetos del catálogo,
punto inicial del jugador, historial undo/redo y herramientas
Pintar/Borrar/Colocar/Quitar/Inicio), así
que si algún día se vendoriza ImGui se migra la vista sin tocar el
núcleo (misma separación modelo/vista que `BattleState` frente al HUD de
combate). Los exportadores son strings puros: `exportTmx()` (tileset
embebido + capa CSV + colisión por `<properties>`, el mismo esqueleto
que `test_map.tmx`, también abre en Tiled) y `exportLevelJson()`
(`"objects"` de la Fase 10; sin patrulla = objeto quieto). **Verificado
de verdad** (`g++ -Wall -Wextra -Werror` + ASan/UBSan limpios,
`demo_editor_state.cpp`): toda la edición (clamps fuera de rango, una
celda = un objeto con reemplazo, paletas vacías sin crash) y — lo
importante — **round-trips reales**: el TMX exportado lo carga
`TileMap::loadFromFile()` (el parser tinyxml2 real del motor) con los
mismos GIDs celda a celda y la misma colisión, y el JSON exportado lo
carga `LevelLoader` con los mismos objetos (incluido un nombre con
comillas escapadas). La app visual (`level_editor.cpp`: ratón vía
`Window::handle()` para pintar/colocar sobre el canvas isométrico con
resaltado de celda, rejilla tenue en celdas vacías, HUD de estado con la
fuente propia) se ejecutó en OpenGL real: 3 frames, captura del editor y
`glGetError() == GL_NO_ERROR`. MVP pendiente: una capa de tiles, sin
panel de propiedades para patrulla/stats y sin navegador visual de
archivos.

#### Proyectos

El editor arranca por una pantalla que lista los **proyectos vivos**
(`assets/proyectos/*.json`): abrirlos, crear uno nuevo y sacar su build.
El formato del manifiesto, las reglas de id y prefijo, y qué es el campo
`entrada` están en **[`FORMATO_PROYECTOS.md`](FORMATO_PROYECTOS.md)**.

```bash
./level_editor                              # la pantalla de proyectos
./level_editor --proyectos                  # lo mismo, por consola
./level_editor --proyecto boundington       # saltarsela
python3 tools/build_proyecto.py boundington # -> builds/boundington/
```

#### Controles del editor

El editor arranca por una **pantalla de proyectos** (ver *Proyectos* más
abajo) y desde ahí se entra a editar.

| Pantalla de proyectos | |
|---|---|
| `W`/`S` | moverse por la lista (da la vuelta en los extremos) |
| `ENTER` | abrir el marcado |
| `N` | crear uno nuevo (`BACKSPACE` borra, `ENTER` crea, `ESC` cancela) |
| `B` | compilar: saca su build en `builds/<id>/` |
| `ESC` | cerrar el editor |

| Edición | |
|---|---|
| ratón izq / der | aplicar herramienta / la contraria |
| `1`..`7` | herramienta: pintar, borrar, colocar objeto, quitar objeto, inicio jugador, rellenar, enlazar nivel |
| `Q`/`E` | ciclar la paleta activa |
| `[` / `]` | subgrupo de objetos anterior / siguiente |
| **`B`** | **buscar en la paleta**: filtra por texto (`taber` → tabernero, taberna…). `B` cierra, `BACKSPACE` borra |
| **`C`** / `SHIFT+C` | capa siguiente / anterior. `CTRL+N` crea una |
| **`R`** | marcar esquina del rectángulo; `R` otra vez lo cierra |
| **`CTRL+C`** / **`CTRL+V`** / **`CTRL+X`** | copiar la selección / pegar bajo el cursor / borrarla |
| **`T`** | dar al objeto bajo el cursor la patrulla del rectángulo marcado |
| `WASD` | pan de cámara (`SHIFT` = ×4) |
| `+`/`-` | zoom (`0` vuelve a 1×), `HOME` centra |
| `Ctrl+Z` / `Ctrl+Y` | deshacer / rehacer |
| **`G`** | guardar en los ficheros **del proyecto abierto** |
| **`V`** | validar el nivel sin salir |
| **`P`** | probar: guarda y lanza `./juego` sobre este nivel |
| **`,`** / **`.`** | escenario anterior / siguiente del proyecto (guarda antes) |
| **`M`** | volver a la pantalla de proyectos (`ESC` hace lo mismo) |

`F5`/`F6`/`F7`/`F8`/`F9` siguen valiendo como alias de `G`/`V`/`P`/`,`/`.`,
pero **manda la letra**: en un portátil Mac las teclas de función son
teclas de medios, así que `F5` de verdad pide `Fn+F5` — con las dos manos
ocupadas en el mapa eso no es una tecla, es una maniobra.

#### Lo que se añadió para poder hacer niveles a mano

La ciudad de Boundington se generó con `tools/gen_ciudad.py` y no se dibujó,
y la razón está en los números: **5.439 objetos** en el catálogo y **95
niveles** a los que puede apuntar una puerta, todos pasando de uno en uno
con `Q`/`E`. Cuatro cosas cambian eso:

- **Buscar** (`B`) filtra la paleta por texto, sin mayúsculas y por
  subcadena — quien busca `taber` no sabe si el id es `tabernero_xila` o
  `npc_taberna`.
- **Capas.** Antes había una sola, y abrir un mapa multicapa y guardar
  **destruía las demás**: el editor avisaba por consola al abrir y el
  destrozo ocurría media hora después. Ahora se cargan todas, las
  herramientas de tile actúan sobre la activa, y el TMX las devuelve
  enteras (round-trip verificado contra `TileMap`).
- **Selección y portapapeles** (`R`, `CTRL+C/V/X`). Copia todas las capas
  y los objetos de dentro; pegar y borrar son **un solo paso de
  historial** aunque toquen mil celdas.
- **Patrulla y destino.** `ObjectSpawn` ya los soportaba, pero el editor
  no los sabía tocar *y `exportLevelJson` ni siquiera los escribía*: abrir
  un nivel con PNJs que patrullan y volver a guardarlo los dejaba a todos
  quietos, sin un solo error. `T` da al objeto bajo el cursor la patrulla
  del rectángulo marcado.

Todo eso vive en `EditorState` (GL-free) y está verificado en
`demo_editor_state`, incluido el round-trip multicapa real.

`P` lanza el juego como **proceso aparte**, no como un modo dentro del
editor: `Application` monta su propia ventana y su propio contexto GL, y
además así un cuelgue probando no se lleva por delante lo que estabas
editando. Es el botón de play de Godot. Guarda antes a propósito.

**Que lo documentado exista** lo comprueba
`python3 tools/verificar_controles.py`, en las dos direcciones: ninguna
tecla anunciada sin enlazar (la cabecera llegó a prometer un `F7` de
«modo jugador» que no existía) y ninguna tecla enlazada sin anunciar.
También cuadra el número de herramientas entre el enum, las teclas, el
menú y la barra de estado — cuatro sitios que hay que tocar a la vez.

### Cierre del ciclo: `GameSession` + `Application` (el juego)

La última pieza que faltaba para que todo lo anterior sea una **partida
jugable**, y el pendiente que las Fases 8-12 fueron arrastrando
("transición automática exploración↔combate: necesita una `Application`
real").

**`GameSession` (`GameSession.h`, GL-free): implementada y verificada de
verdad.** Es el estado de una partida: modo
`Exploration`/`Battle`/`GameOver`, movimiento del jugador con colisión
real (tiles con `collision` del TMX, bordes del mapa, objetos con
`blocksMovement`), recogida de pickups al **inventario real** del
jugador, y —la parte que cerraba el hueco— el **encuentro automático**:
chocar con un objeto de categoría `Enemy` no mueve al jugador y arma un
`BattleState` con el `CombatData` de ese tipo de enemigo (vida, maná y
habilidades del catálogo de la Fase 10). `syncBattleOutcome()` aplica el
desenlace al mundo: `Victory` elimina al enemigo (su celda queda
transitable), `Fled` lo deja donde estaba (huir no mata: habrá que
rodearlo o volver), `Defeat` es `GameOver` terminal. El jugador entra
**por referencia**, así que la vida curada, el maná gastado y las
pociones consumidas en combate se conservan al volver a explorar — es el
mismo jugador, no una copia. **Verificado de verdad** (`g++ -Wall
-Wextra -Werror` + ASan/UBSan limpios, `demo_game_session.cpp`) contra
el `TileMap` y el `ObjectCatalog` **reales**, no mocks: una partida
entera sin ventana, cubriendo colisión/bordes/props, recogida, encuentro,
los tres desenlaces, que el log del combate sobrevive a la destrucción
del `BattleState`, y la continuidad de vida/maná/inventario entre modos.

**`Application` (`Engine/Application.h`): implementada.** La clase que
`motor_grafico_clases.puml` listaba desde el primer día
(`init`/`run`/`shutdown`/`processInput`/`update`/`render`), ahora real:
crea ventana y recursos, carga catálogo y nivel desde JSON, monta mapa +
`Player` + `IsometricRenderer` con sombras blob (Fase 11), y compone el
HUD de la Fase 9 (barra de vida, texto de estado, cuadro de diálogo con
el log, y menú de comandos Atacar/Tajo/Cura/Poción/Huir que solo aparece
en combate). **No decide reglas**: traduce teclado a llamadas de
`GameSession` y dibuja lo que hay — por eso el ciclo de juego se prueba
entero sin ventana y aquí solo queda lo que necesita GL. La cámara sigue
al jugador con `Camera::transitionTo()` (Fase 4) disparada solo al
cambiar de celda. `init()` devuelve `Result` (captura las
`EngineException` de `Window`/`Shader` dentro) y `shutdown()` destruye en
orden inverso, para que ninguna textura llame a `glDeleteTextures` sin
contexto. El ejecutable es **`juego`** (`examples/juego.cpp`): WASD para
moverse, W/S + ENTER en combate, ESC salir. Solo se pudo **verificar en
modo sintaxis** (`-fsyntax-only` limpio), pendiente de `./build/juego` en
macOS.

**Estado global**: Fases 1-4 del Gantt original + 6-12 del RPG +
`GameSession`/`Application`. Extensiones naturales si se continúa:
migrar la UI del editor a ImGui cuando se pueda vendorizar, panel de
propiedades y undo en el editor, equipo en el catálogo (armas/armaduras
que alimenten la fórmula de daño en vez del `kBasicAttackPower` fijo),
varios enemigos por combate, y niveles encadenados (salir de un mapa
para entrar en otro).

## CI/CD

`ci.yml` y `release.yml` viven en `.github/workflows/` en la raíz del
repositorio (GitHub Actions solo ejecuta workflows ubicados ahí) y operan
con `working-directory: MotorGraphico` sobre este subdirectorio. Ver
[`../BRANCHING.md`](../BRANCHING.md) para el detalle de qué hace cada uno,
el modelo de ramas y el versionado.

## Próximo paso sugerido

**Fase 1 y Fase 2 del Gantt completas.** Fase 1: ventana + contexto
OpenGL, cámara ortográfica, grid isométrico de prueba, quad texturizado
con `GL_NEAREST` (`demo_textured_quad.cpp`). Fase 2: `ResourceManager<T>`
(ya existía), carga de atlas de tiles (`TextureAtlas`), sistema de capas
del mapa (`TileMap::m_layers`), parser de mapas TMX
(`TileMap::loadFromFile`, vía tinyxml2) y culling/batching estático
(`TileMap::visibleRange()`) — todo verificado en `demo_tilemap.cpp` y
`demo_isometric_renderer.cpp`.

**Fase 3 (`motor_grafico_gantt.puml`): completa.** `IRenderable`/
`IUpdatable`, `Entity` (base; `render()`/`getSortKey()` implementados,
`update()` puro virtual por diseño), `IsometricRenderer` (cola de
render, `sortQueue()` con Painter's Algorithm real, culling aplicado en
`renderLayer()`) y ahora `AnimatedEntity`/`Player`/`Enemy` (jerarquía de
`Entity`, `motor_grafico_clases.puml`): animación por frames
(`AnimatedEntity::addAnimation()`/`play()`), movimiento discreto por
celda y salud/inventario de `Player` (`Player::handleInput()`), y
patrulla determinista con `int m_aiState` de `Enemy`. `TestProp` en
`demo_isometric_renderer.cpp` sigue siendo solo un stub de prueba; las
entidades reales del motor son estas tres.

**Verificación de `demo_animated_entity.cpp` con contexto OpenGL real:
completada.** Compilado y ejecutado con GLFW 3.4 + GLAD reales (sobre
OpenGL nativo de macOS, `GL_VERSION = 4.1 Metal`): 5 frames con
`glGetError() == GL_NO_ERROR`, framebuffer volcado a PPM e inspeccionado
(fondo en esquinas, textura en el centro), y limpio bajo
AddressSanitizer/UBSan. La Fase 3 queda cerrada al 100%.

**Fase 4 (`motor_grafico_gantt.puml`, pulido visual — opcional):
arrancada.** `Framebuffer` (render-to-texture), tint por vértice
(`Vector4` en `SpriteBatch`/`Entity`) e `IsometricRenderer::setPostFX()`/
`applyPostProcessing()` (fullscreen-triangle + shader `lightmap`,
combinación escena×luz con suelo `uAmbient`) implementados y ejercitados
en `demo_lighting.cpp`, **verificado con contexto OpenGL real** sobre
macOS (`GL_VERSION = 4.1 Metal`): `glGetError() == GL_NO_ERROR`, las dos
aserciones del demo (esquina no negra + centro iluminado) pasan, el PPM
muestra el efecto de iluminación (foco central brillante degradando al
azul-noche oscurecido de los bordes), y limpio bajo
AddressSanitizer/UBSan. Se encontró y corrigió un bug de fondo negro
puro con el post-FX activado (`IsometricRenderer::setSceneClearColor()`,
ver "Corrección aplicada" más arriba). Los demos sin post-FX quedan
idénticos (tint por defecto blanco = sin cambio visible).

**Fase 4, segundo efecto (niebla de guerra): implementada y verificada.**
`FogOfWar` (memoria de exploración Hidden/Explored/Visible,
`reveal()`/`beginFrame()`/`stateAt()`) y su integración en
`IsometricRenderer` (`setFog`/`setFogColor`/`setFogEnabled`, exclusiva
con la iluminación — ver comentario de `applyPostProcessing()`) están
implementados y **verificados con contexto OpenGL real** en macOS
(`GL_VERSION = 4.1 Metal`): `demo_fog` corre con `glGetError() ==
GL_NO_ERROR`, las aserciones de memoria (test con grid 5×5 fijo) y las de
framebuffer (centro visible + esquina oculta) pasan, el PPM muestra la
zona del jugador clara y el resto con tinte de niebla, y limpio bajo
AddressSanitizer/UBSan. Se corrigieron dos bugs por el camino: la firma
de `setFog()` (ver "Corrección aplicada" más arriba) y un **flip Y** en
`updateTexture()` (las celdas se pintaban con origen arriba cuando la
textura GL y el shader `fog.vert` esperan origen abajo — sin el flip la
niebla quedaba desplazada y se veía siempre oculta).

**Fase 4, tercer efecto (paletizado/LUT de color): implementado y verificado.**
`ColorLUT` (cubo RGB de lado `levels` cuantizado a `levels` tonos por canal
— `quantizeChannel()`/`quantizeColor()` puros, sin GL, y `cellColor()` como
getter de test, mismo criterio que `FogOfWar::stateAt()` — codificado como
textura 2D de `levels·levels`×`levels`, `GL_NEAREST` para bordes nítidos
entre escalones) y su integración en `IsometricRenderer` (`setLUT`/
`setLUTEnabled`, exclusivo con luz y niebla y con prioridad más alta —
ver comentario de `applyPostProcessing()`) están **verificados con contexto
OpenGL real** en macOS (`GL_VERSION = 4.1 Metal`): `demo_lut` corre con
`glGetError() == GL_NO_ERROR`, los tests de cuantización
(`quantizeChannel`/`quantizeColor`/`cellColor`) pasan, y el framebuffer
final confirma el posterizado de verdad: con `levels=4`, el pixel central
pasa de `(250,220,90)` (color original del checker) a `(255,255,85)`, con
cada canal exactamente en uno de los escalones `{0,85,170,255}`, y el PPM
completo tiene solo 2 colores únicos (la paleta limitada redujo los tonos
continuos del checker a escalones discretos). Limpio bajo
AddressSanitizer/UBSan (la textura de la LUT se libera en `~ColorLUT`). Los
demos sin LUT siguen idénticos (LUT off por defecto).

**Fase 4, cuarto ítem (transiciones de cámara y zoom): implementado y
verificado.** `Camera::transitionTo(targetPosition, targetZoom, duration,
easing)` anima posición y zoom desde los valores actuales hasta un
destino explícito en `duration` segundos, con una curva de easing
(`Camera::Easing::{Linear, EaseInOutQuad, EaseOutCubic}`), sustituyendo
durante ese tiempo el lerp exponencial continuo de `update()`/`move()`
(pensado para seguir un objetivo que se mueve, no para animaciones de
cámara de una vez tipo cutscene). Al terminar, `m_targetPosition` queda
en el destino de la transición, así que el paneo normal retoma sin salto.
`duration <= 0` es un corte instantáneo (aplica el destino en la
siguiente `update()`), y volver a llamar a `transitionTo()` con una
transición en curso reinicia desde la posición/zoom YA interpolados (no
desde el destino cancelado), evitando saltos al encadenar transiciones.

A diferencia de la LUT, este ítem **no necesita contexto OpenGL** (`Camera`
solo depende de `glm`, igual que `demo_camera.cpp`): se pudo compilar y
**ejecutar de verdad** en este entorno de trabajo, con `g++ -std=c++17
-Wall -Wextra -Werror` (cero warnings) y también bajo
`-fsanitize=address,undefined` (limpio). Los 5 casos nuevos de
`demo_camera.cpp` pasan: `transitionTo(Linear)` llega EXACTO al destino y
termina en `duration` (no solo "cerca", a diferencia del lerp
exponencial); el paneo continuo tras la transición converge sin salto;
`EaseInOutQuad` arranca más lento que lineal y es simétrico en su punto
medio; `duration<=0` aplica el destino de inmediato; y encadenar
`transitionTo()` a mitad de otra no produce saltos. Los casos 1-4
(round-trip iso, paneo exponencial, `screenToWorld`, `worldToGrid`) siguen
pasando sin cambios: la transición es un modo alternativo dentro de
`update()`, no toca el comportamiento existente cuando no está activa.

**Fase 4, quinto ítem (sprite batching dinámico): implementado.**
`SpriteBatch` pasa de un único VBO a un **ring de 3 VBOs** (con su VAO
cada uno): `begin()` rota al siguiente en cada frame nuevo, así que
mientras la GPU aún lee el VBO del frame N para su draw call, el frame
N+1 ya escribe en un VBO distinto, sin bloquear al CPU. `flush()` además
orfana explícitamente el VBO activo (`glBufferData` con `NULL` antes del
`glBufferSubData` real) en vez de depender de que el driver decida
orfanear por su cuenta, y usa `GL_STREAM_DRAW` (no `GL_DYNAMIC_DRAW`: el
patrón real es "se escribe una vez por frame, se lee una vez para el
draw"). No se usa persistent mapping (`glBufferStorage` +
`GL_MAP_PERSISTENT_BIT`): es `ARB_buffer_storage`, GL 4.4+, y el motor
apunta a GL 3.3 Core (ver más arriba, "OpenGL 3.3 Core es ampliamente
compatible"). La interfaz pública de `SpriteBatch`
(`begin()`/`submit()`/`flush()`/`end()`) no cambió, así que
`IsometricRenderer` y los seis demos que la usan (directa o
indirectamente) no necesitaron ningún cambio.

Con esto, los 5 ítems de la Fase 4 del Gantt (iluminación, niebla de
guerra, paletizado/LUT, transiciones de cámara/zoom y batching dinámico)
están **implementados y verificados** con contexto OpenGL real en macOS
(`GL_VERSION = 4.1 Metal`): `demo_lut 5` corre con `glGetError() ==
GL_NO_ERROR` y su posterizado se confirma en el framebuffer (pixel central
`(255,255,85)`, cada canal en un escalón de `levels=4`); y
`demo_textured_quad`/`demo_isometric_renderer`/`demo_animated_entity`/
`demo_lighting`/`demo_fog` siguen pasando limpios tras el cambio de
`SpriteBatch` (ring de VBOs / batching dinámico), sin regresión visual
(`glGetError() == GL_NO_ERROR`, PPM correctos). La Fase 4 queda cerrada al
100%.
