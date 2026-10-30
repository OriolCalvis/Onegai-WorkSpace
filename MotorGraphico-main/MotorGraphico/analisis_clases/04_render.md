# 04 — Módulo `Render`

*Análisis clase a clase · 31/08/2026 (retomado por zcode) · 25 headers / 19 cpp = 44 ficheros, 4.536 líneas. Mayores: BitmapFont.cpp (466), IsometricRenderer.cpp (279), TileMap.cpp (187), FogOfWar.cpp (187). No hay ficheros propios para HudText/HudDialogueBox (viven en `HudTextWidgets.h`) ni CharacterSprite (es el override `setCharacterSprite()` de Entity).*

**Trabajo en vuelo NO commiteado (reestructuración DOC-03, HEAD `3258f6f`):** `HudMinimap.h/.cpp`, `IsometricRenderer.h/.cpp`, `Tile.h`, `TileMap.h/.cpp` (+98/−261 líneas): migración del mapa a `Runtime/RuntimeTileMap`; `Tile` pasó de POD a alias (`using Tile = RuntimeTile`), `TileMap` pasa de parsear TMX él mismo a heredar de `RuntimeTileMap` y delegar en `TmxMapImporter`+`RuntimeTileMapBuilder`. **Los contratos congelados de LINEA_BASE §3 quedan desfasados en este punto** (p.ej. el `getTile` que lanza ya vive en `RuntimeTileMap`).

---

## Núcleo del pipeline

### `Camera` (`include/Render/Camera.h`) — clase concreta : `IUpdatable`
**Qué es:** cámara ortográfica 2D con paneo suavizado, zoom y transiciones de un uso (cutscenes).
**Estado:** `m_position`/`m_targetPosition` (lerp exponencial, `m_lerpSpeed=8`), `m_zoom`, viewport, bloque de transición (active/elapsed/duration/easing/start*/target*).
**API:** `update(dt)` avanza lerp o transición; `move(delta)`; `setZoom` (mín 0.01); `setViewportSize`; `transitionTo(pos, zoom, dur, easing)` con `Easing{Linear, EaseInOutQuad, EaseOutCubic}` — reinicia SIEMPRE desde la posición interpolada actual, duración ≤0 = corte; `isTransitioning()`; getters; `getViewProjectionMatrix()` (glm::ortho con Y invertida a propósito: mundo +y = abajo en pantalla); `screenToWorld()`; `worldToGrid()` (delega en `IsoMath`, no reimplementa).
**Flujo:** la posee Application; IsometricRenderer y `visibleRange` solo leen.
**Trampas:** durante transición, `move()` no tiene efecto visible hasta que termine; `setViewportSize` no reconfigura FBOs de post-FX.

### `TileMap` (`include/Render/TileMap.h`) — clase concreta : `RuntimeTileMap` [WIP]
**Qué es:** adaptador TMX→RuntimeTileMap; conserva metadatos del tileset para demos/editor durante DOC-03.
**Estado:** `m_tilesetImage` (ruta de `<image source>` resuelta contra la carpeta del TMX, normalización manual de `..` sin std::filesystem), `m_tilesetImageSource` (cruda, para re-escribir al guardar — el editor destruyó una vez `ciudad_centro.tmx` por no hacerlo), name/columns/firstGid/tileCount/imageWxH/tileWxH, `m_collisionGids`.
**API:** `loadFromFile(path)` nunca lanza (captura `MapParseException` → `Result::Error`); `parseOrThrow()` (privada) = tinyxml2 + `TmxMapImporter().importFile()` + `RuntimeTileMapBuilder().build()` + move al subobjeto; ~10 getters de tileset; `visibleRange(camera)` (GridBounds desde las 4 esquinas con margen ±1).
**Flujo:** la cargan Application/demos; la dibuja IsometricRenderer vía la interfaz RuntimeTileMap; el editor usa los getters para re-escribir el TMX.
**Trampas:** solo tileset EMBEBIDO, solo CSV, UN tileset. `visibleRange` está **duplicado** con `visibleRangeFor()` en IsometricRenderer.cpp.

### `Tile` (`include/Render/Tile.h`) — alias de tipo [WIP]
`using Tile = RuntimeTile`. Era un POD {tilesetID, variant, collision}; el dato vive ahora en Runtime/. Cero funciones propias.

### `IsometricRenderer` (`include/Render/IsometricRenderer.h`) — orquestadora [WIP]
**Qué es:** monta el frame completo (capas con culling + sombras + entidades ordenadas) sobre UN `SpriteBatch`, y aplica pases fullscreen de post-procesado.
**Estado:** punteros NO propietarios cámara/mapa/atlas/shader; `m_spriteBatch` por valor; `m_renderQueue` (vector `IRenderable*`); `m_sceneFbo` como `optional<Framebuffer>` (perezoso); luz (`m_lightShader/m_lightmapTexture/m_ambientColor{0.35,0.35,0.4}`), niebla (`m_fogShader/m_fog/m_fogColor`), LUT, `m_shadowTexture`; oclusión (`m_occlusionFocus` optional + enabled + `alpha=0.40`); `m_sceneClearColor` (negro; existe porque el shader lightmap multiplica y 0·luz=0).
**API:** `addToQueue/removeFromQueue` (erase-remove); `setAtlas`; `setOcclusionFocus/clear/Enabled/Alpha` (clamp 0.05–1); `renderQueue()` para tests; `sortQueue()`; `renderFrame()` → `Result<bool>` nunca lanza; `setPostFX/setAmbientColor/...`; `setFog/...`; `setLUT/...`; `setBlobShadows`; privadas `renderLayer(i)`, `applyPostProcessing()`, `ensureSceneFbo(w,h)`.
**Flujo `renderFrame()`:** valida punteros → FBO si hay FX → uniforms → blend → `begin()` → capas (culling por `visibleRangeFor`, tiles atenuados por `OcclusionRules`) → `sortQueue()` → sombras de TODOS → render de TODOS → `end()` → `applyPostProcessing()` (fullscreen-triangle con `emptyVAO`, prioridad LUT > niebla > luz).
**Trampas:** **luz, niebla y LUT son EXCLUSIVOS entre sí** (sin FBO ping-pong); `ensureSceneFbo` recrea el FBO aunque el tamaño coincida; el `glClear` de la ventana lo hace el llamador.

### `SpriteBatch` (`include/Render/SpriteBatch.h`) — clase concreta no copiable
**Qué es:** agrupa quads en VBOs dinámicos, un draw call por textura.
**Estado:** `m_vertices` (`Vertex{Vector2 pos, uv; Vector4 color}`); **ring de 3 VAO/VBO** (triple-buffering anti-stall, GL 3.3 sin persistent mapping); textura actual.
**API:** `begin()` (limpia y rota el ring); `submit(pos, size, uv, tex, tint)` (flush si cambia textura; tint plano por quad); `flush()` → `Result<bool>` (orphaning `glBufferData(NULL)` + `glBufferSubData`, GL_STREAM_DRAW); `end()`.
**Trampas:** sin límite de tamaño del buffer (mitigado por culling); un solo slot de textura.

### `Framebuffer` (`include/Render/Framebuffer.h`) — RAII, movible
FBO RGBA8 + VAO vacío para fullscreen-triangle. Ctor lanza `EngineException` si dims ≤0 o incompleto; move que anula el origen (anti doble-free). Sin depth/stencil (irrelevante en 2D con Painter's). GL_NEAREST.

### `IRenderable` / `IUpdatable` — interfaces
`render(SpriteBatch&)` + `getSortKey()` puras, `renderShadow(...)` virtual con cuerpo (opt-in). `update(float)` pura. Separadas a propósito.

## Entidades

### `Entity` (`include/Render/Entity.h`) — base : `IRenderable`+`IUpdatable`
**Qué es:** objeto dibujable en el grid; implementa render/sort/sombra para que las subclases solo hagan `update()`.
**Estado:** protected: `m_gridPosition`, `m_offset` px, `m_spriteID`, `m_atlas` (no propietario), `m_tint`, override de personaje (`m_characterAtlas/drawW/H/anchor`), `m_visualScale=1`; private: `m_tileWidth/Height`.
**API:** ctor `(gridPos, spriteID, atlas, 64, 32)`; `render()` — camino character (hoja 64×64, inset 1px anti-ghosting, **swap v0/v1** compensando el flip de stb_image) o camino tile nativo, ambos con `m_visualScale` manteniendo pies en tierra; `getSortKey()` = `(x+y)*(tileHeight/2)*65536 + x`; `renderShadow()` elipse 50%·40% tint negro α=0.45 sobre textura radial; setters de posición/offset/character/escala/tint/atlas.
**Trampas:** `kSortKeyMultiplier = 1<<16` puede desbordar `int` en mapas gigantes; el swap de UV es parche local al camino character.

### `StaticEntity` — Entity con `update(){}` deliberado (arbustos, puertas, pickups). Pública para que el editor no dibuje objetos a mano (fractura #7).

### `AnimatedEntity` — Entity con secuencias nombradas
`m_animations` (nombre→frames), `m_frameTime=0.15`, bucle `while` para dt grandes; `play(name)` no-op si ya activa o no existe (hot path permisivo). Base directa de Player y Enemy.

### `Player` (`include/Render/Player.h`) — : `AnimatedEntity`+`ICombatant`
**Qué es:** PJ: movimiento discreto por celda en `handleInput()` (no en update) + contrato de combate dual-mode.
**Estado:** `m_health/m_maxHealth=100`, inventario vector<int>, stats legacy `[CON,DES,INT,CAR]`, `m_sheet` no propietario.
**API:** `handleInput(input)` (±1 celda + animación walk_*/idle, prioridad fija arriba>abajo>izq>der); ICombatant completo (hoja si `m_sheet`, legacy si no); `set_character_sheet` (sincroniza HP solo si estaba a default); `takeDamage/heal` clampados.
**Trampas:** **HP actual vive SIEMPRE en `m_health` aunque haya sheet** (dualidad parcial); los setters legacy con sheet activo escriben la copia muerta.

### `Enemy` (`include/Render/Enemy.h`) — : `AnimatedEntity`+`ICombatant`
Toda la lógica delega en `EnemyBrain` por composición. Ctor `(pos, patrolMin, patrolMax, atlas, ..., 0.5)`; `update` avanza animación+brain con callback de colisión que devuelve SIEMPRE true (GameSession mueve sus enemigos por su cuenta) — **el Enemy de demo atraviesa muros**.

### `ICombatant` (`include/Render/ICombatant.h`) — interfaz
takeDamage/heal/health/maxHealth/isAlive + stats, `defenses()`, `initiative_bonus` (default DES), tier, combatant_id. **Contrato RPG sin GL que vive en Render/** — ubicación discutible.

## Fuente y HUD

### `BitmapFont` — fuente procedural 5×7, atlas 16×5 = 80 celdas
ASCII 32–95 + fila extendida de acentuadas (À Á È É Í Ò Ó Ù Ú Ñ Ç Ü ¿ ¡ ·). `m_atlas` Texture PROPIETARIO generado en ctor; `m_scale=3`. Estáticas puras sin GL: `glyphUV/glyphUVCodepoint`, `wrapText` (corta en espacios, guiona largas, respeta `\n`, acentuadas = 1 glifo), `clipText` (elipsis), `measureText`; de instancia `drawText` (un quad por glifo). **Solo CAJA ALTA** (minúsculas normalizadas). Decodificador UTF-8 1–3 bytes (4 bytes → glifo espacio). **`clipText` solo lo usa el test** — F0 a medio desplegar.

### `HudElement.h` — `HudAnchor` (9 anclas) + `HudTransform` (POD con `resolveTopLeft` pura) + `IHudElement` (interfaz render/update/visible).

### `HudManager` — compone widgets en UNA pasada con proyección en píxeles de pantalla (independiente de Camera; `glm::ortho(0,vw,vh,0)` SIN Y invertida). `update(dt)` a TODOS, también invisibles (lección del bug de la barra de moral); `render` solo visibles. Z-order = orden de inserción.

### `HudBar` — barra animada (vida/mana/moral): quads tintados sobre whiteTexture 1×1; `m_displayedValue` con suavizado exponencial y corte de convergencia; franja ghost cuando displayed>real; `setThresholdColors` por ratio REAL. Solo horizontal; sin texto propio.

### `HudPanel` — quad contenedor con marco opcional (composición por superposición; se añade primero).

### `HudText` (`HudTextWidgets.h`) — línea(s) con `wrapText` pre-aplicado si `size.x>0` (F0). **Envuelve ancho pero no limita alto** — desborda por abajo; no usa `clipText`.

### `HudCommandMenu` — menú navegable estilo DQ: índice resaltado + prefijo `"> "` (accesible en B/N); `moveUp/moveDown` con wraparound; `contentSize()` estática que mide CON prefijo (para que el editor dimensione sin duplicar cuenta); sin lógica de input (correcto); sin recorte propio.

### `HudDialogueBox` — panel + últimas N líneas (`m_maxLines=4` default); `setLines()` **envuelve al ancho útil ANTES de truncar** (F0: el orden importa). Sin scroll ni paginado (deliberado).

### `HudMinimap` [WIP] — minimapa cenital (NO isométrico): un quad por celda + marcadores leídos de GameSession en cada render; ventana `m_maxCells=24` centrada en el jugador; bloqueada = colisión en CUALQUIER capa o celda vacía; orden de pintado objetos→enemigos→jugador. **Sin memoria de exploración** (no se conecta a FogOfWar); coste maxCells² cada frame.

## Post-procesado

### `FogOfWar` — niebla 3 estados (Hidden/Explored/Visible) con memoria, rasterizada a textura CPU-side: `reveal(origin, radius)` cuadrado Chebyshev; `beginFrame()` degrada Visible→Explored; `updateTexture(camera)` pinta celdas no-Hidden como RECTÁNGULO con flip Y (bug de niebla reflejada documentado); `stateAt()` fuera de rango = Hidden sin lanzar. **Recorre el mapa ENTERO** por frame (sin culling); subida completa `glTexImage2D`; visión sin línea de visión.

### `ColorLUT` — LUT 3D desenrollada en textura 2D; estáticas puras `quantizeChannel/quantizeColor`; **solo genera la LUT IDENTIDAD** (los grading no-identidad no tienen vía de entrada).

### `PointLight` + `DynamicLightMap` — N luces puntuales compuestas en lightmap viewport-size por frame en CPU; `attenuationAt` lineal clampada; acumulación float por bounding-box con flip Y y clamp final; `lights()` mutable a propósito (parpadeos). Mismo patrón de re-subida completa.

### `BlobShadow` — namespace: `FillBlobShadowPixels` pura (círculo blanco, alpha de caída t²) + `CreateBlobShadowTexture` (el llamador es dueño del delete).

### `OcclusionRules` — namespace, 1 función inline: tile sólido se atenúa solo si está 1–2 filas por delante en profundidad (x+y) y a distancia lateral (x−y) ≤2 del foco.

---

## Qué falta / deuda del módulo

1. **Reestructuración DOC-03 sin commitear** (7 ficheros): contratos de LINEA_BASE ya desfasados; riesgo de colisión con quien toque lo mismo.
2. **Duplicación `visibleRange`** (TileMap vs `visibleRangeFor` en el renderer): consolidar.
3. **Post-FX exclusivos** (luz/niebla/LUT no encadenables; falta FBO ping-pong); `ensureSceneFbo` recrea sin necesidad.
4. **FogOfWar sin culling** ni línea de visión; re-subidas completas por frame (ídem DynamicLightMap).
5. **F0 del texto a medio desplegar:** `clipText` testeado pero sin consumidor de producción; `HudText` sin límite de alto; sin scissor real.
6. **Fractura #1 parcial en Player** (HP siempre en `m_health`; setters legacy → copia muerta).
7. **Ubicaciones discutibles:** `ICombatant` en Render/; permisos 600 en `Enemy.h`/`TileMap.cpp`.
8. **Menores:** overflow posible de `getSortKey` (×65536); UTF-8 sin 4 bytes; `ColorLUT` solo identidad; minimapa sin FogOfWar.
