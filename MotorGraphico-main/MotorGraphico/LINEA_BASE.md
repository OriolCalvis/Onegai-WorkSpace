# Línea base — foto congelada del motor

*Fotografía fechada y verificada del repositorio antes de tocar DOC-01.
Entregable de BASE-01 (Gantt Fase 0). Congelada el **28 de agosto de 2026**
sobre `feature/editor-s1-contratos` @ `008585f`.*

> **Naturaleza de este documento.** Es un inventario, no documentación de
> uso: cada pieza ya tiene la suya. Lo que no existía era UNA foto fechada
> de la base. Las cifras de aquí se midieron el día de la fecha con los
> comandos indicados en el anexo; si el número que ves en el código no
> cuadra, el código manda y este documento queda viejo a propósito.

## 1. Código

### 1.1 Módulos (include/ + src/)

| Módulo | Headers | .cpp | Responsabilidad |
|---|---:|---:|---|
| `Core` | 17 | 6 | JSON, matemáticas, recursos y errores (`Result<T>`); GL-free |
| `Editor` | 5 | 4 | Estado del editor y hub de proyectos; GL-free |
| `Engine` | 3 | 2 | Ventana, `Application`, bucle principal |
| `Game` | 5 | 5 | `GameSession`, `BattleState`, `EnemyBrain`, moral; GL-free |
| `Level` | 4 | 2 | `LevelDefinition`, `LevelLoader`, `ObjectCatalog`; GL-free |
| `RPG` | 16 | 10 | Catálogos tipados, dados, habilidades, narrativa, hojas de personaje |
| `Render` | 25 | 19 | TileMap, entidades, renderer isométrico, HUD, post-procesado |

Total aproximado: **32.900 líneas** entre `include/`, `src/` y `examples/`.

### 1.2 Targets CMake

- **6 librerías:** `motor_core`, `motor_math`, `motor_json`, `motor_rpg`, `motor_level`, `motor_map`.
- **49 ejecutables:** `juego` (runtime), `level_editor` (autoría visual) y 47 demos.
  Los demos GL corren solo con pantalla (bajo `xvfb` en CI); el resto es GL-free.
- `include(CTest)` activo: bajo `BUILD_TESTING` los demos GL-free verificados
  se registran como pruebas (`add_motor_gl_free_demo_test`) y hay una función
  `add_motor_unit_test` para pruebas de dominio (trabajo INF-01 en curso en
  el árbol de trabajo el día de la congelación).

### 1.3 Ejecutables de referencia

| Binario | Qué comprueba |
|---|---|
| `juego` | Runtime completo: campaña de Boundington con ventana |
| `level_editor` | Editor con pantalla de proyectos (`--proyectos`, `--proyecto <id>`, `--nuevo`) |
| `demo_campana_boundington` | 72/72 beats de la campaña, invariante canónico de la Matanza |
| `demo_editor_state` | Estado del editor sin GL |
| `demo_proyectos` | Índice de proyectos, manifiestos y huérfanos sin GL |
| `demo_catalogos_rpg` | Los catálogos tipados cargan desde JSON |
| `demo_mundo` | `locations.json` (19 naciones, 95 ciudades, 26 zonas) |

## 2. Activos

| Colección | Ruta | Cantidad |
|---|---|---:|
| Mapas TMX | `assets/maps/*.tmx` | 97 |
| Niveles JSON | `assets/levels/*.json` | 97 |
| Aventuras | `assets/adventures/*.json` | 9 |
| Catálogos RPG | `assets/catalogs/*.json` | 24 |
| Catálogos de objetos | `assets/objects/*.json` | 32 |
| Manifiestos de proyecto | `assets/proyectos/` | 5 proyectos + índice |
| Texturas (recursivo) | `assets/textures/` | 61 PNG |
| Shaders | `assets/shaders/` | 8 |
| Reglas | `assets/rules/tier_rules.json` | 1 |
| Experimento mundial | `assets/world_experiment/` | 4 paquetes regionales |

Proyectos vivos: `boundington` (jugable), `oeste_norte` (jugable),
`este_norte`, `mundo` y `este_sur` (packs de escenario sin `entrada`).

## 3. Contratos de la base gráfica

Los cuatro contratos viven documentados EN SUS HEADERS (fuente de verdad);
esto es el resumen congelado. Regla de ARCHITECTURE.md que aplican: un
concepto, una representación, un propietario, una dirección.

### Camera (`include/Render/Camera.h`, lib `motor_math`)
Ortográfica 2D con paneo suavizado (lerp exponencial hacia `m_targetPosition`)
y zoom. `update(dt)` es `IUpdatable` formal (fractura #3 resuelta).
`transitionTo()` es animación de una vez con easing; no interfiere con el
paneo continuo. `getViewProjectionMatrix()` invierte Y a propósito (convenio
"abajo en pantalla = +y mundo", igual que `stbi_set_flip_vertically_on_load`
e `IsoMath`). `worldToGrid()` delega en `IsoMath`, no reimplementa.

### TileMap (`include/Render/TileMap.h`, lib `motor_map`)
Carga TMX por `loadFromFile()` → `Result<bool>` (nunca lanza). Subconjunto
soportado, fijado aquí porque es contrato: tileset EMBEBIDO (sin `.tsx`
externo), `<data encoding="csv">` únicamente, colisión por propiedad `collision`
del tile. UN tileset por mapa (decisión explícita). El tileset se expone
completo (nombre, imagen, tamaños, columnas, primer GID, GIDs de colisión)
porque el editor debe reescribirlo sin pérdida. `getTile()` lanza
`std::out_of_range` fuera de rango: índice inválido = bug del llamador.
`visibleRange(camera)` es sobre-aproximación segura para culling.

### Entity (`include/Render/Entity.h`)
Objeto dibujable en el grid. Implementa `IRenderable` + `IUpdatable`;
`render()` encola en `SpriteBatch` (no dibuja hasta `flush`). Sort key =
Painter's Algorithm `(grid_x + grid_y) * tileHeight/2`. No posee atlas
(punteros no propietarios; los gestiona `Application`).
`setCharacterSprite()` es override opcional para actores (hoja propia,
tamaño y ancla) — sin él, `render()` es byte-idéntico. `setVisualScale()`
mantiene los pies en la celda. `setAtlas()` existe por transición de nivel.

### IsometricRenderer (`include/Render/IsometricRenderer.h`)
Orquesta el frame: fondo con culling + entidades con Painter's sobre UN
`SpriteBatch`. No posee camera/mapa/shader; sí el batch y el atlas del
tileset. Foco de oclusión opcional (`setOcclusionFocus`) para no tapar al
personaje. `removeFromQueue()` espejo de `addToQueue()` (pickups/enemigos
que desaparecen).

## 4. Stubs y deuda marcados (sin corregir)

BASE-01.3 manda MARCAR, no arreglar.

### 4.1 Marcadores en código
19 `TODO/FIXME/STUB` en 15 ficheros, concentrados en:
`RPG/CharacterCreation*.h`, `RPG/DicePoolEngine.h`, `RPG/NarrativeEngine.*`,
`RPG/CharacterSheet.h`, `RPG/InventoryEngine.h`, `Render/TileMap.h`,
`Render/HudManager.h`, `Render/HudElement.h`, `Render/Enemy.h`,
`Game/BattleState.h`, `Engine/Application.h`, `Editor/ProjectHub.h`,
`Level/ObjectCatalog.cpp`.

### 4.2 Fracturas de ARCHITECTURE.md §4
Las 7 fracturas del análisis de coherencia constan **resueltas** (Enemy →
EnemyBrain, ICatalog común, IUpdatable, IsoMath único, Render/ depurado,
Tile correcto, StaticEntity expuesto). No hay fractura abierta ahí; las
nuevas se registran en su tabla.

### 4.3 Deuda registrada en la bitácora (Parte 4.3)
El atlas no llega a los mapamundis: sin conectar con `LevelTransition`; banco de conformidad
Java↔C++ sin ejecutar; `Player::populate_from_class_race_background()`
pendiente; `GameSession`/`BattleState` sin pasar por el CharacterSheet único;
432 monstruos con `maxHealth==0` en JSON fuente; ConditionEngine con 8
efectos activadores stub; `Result<T>` sin namespace; parche legacy de
`demo_skills`; capa CD/`DefenseBlock` en desuso tras la Ed. 3.

### 4.4 Paquetes a REPLANTEAR (auditoría del 28/08)
- **DOC-03:** el puente editor→runtime sigue siendo texto TMX reparsedo.
- **EDT-02:** planifica ImGui, descartado ya en `level_editor.cpp` (decisión INF-02).
- **EDT-12:** deshacer/rehacer por snapshot completo, no por comandos.

## 5. Estado concurrente en la congelación

El árbol de trabajo contenía el día de la foto trabajo ajeno sin commitear
(declarado en la bitácora Parte 0): INF-01 en curso (`tests/` nuevo +
`CMakeLists.txt`) y la sesión de propiedades de instancia del editor
(`EditorState.*`, `level_editor.cpp`, `Application.*`, `GameSession.*`,
`ObjectCatalog.h`, `demo_editor_state.cpp`). Esta línea base describe
`HEAD` + ese estado declarado; no incorpora ni revierte ese trabajo.

## Anexo — comandos de reproducción

```bash
grep -cE 'add_executable\(' CMakeLists.txt            # 49
grep -cE 'add_library\(' CMakeLists.txt               # 6
ls assets/maps/*.tmx | wc -l                          # 97
ls assets/levels/*.json | wc -l                       # 97
find assets/textures -name '*.png' | wc -l            # 61
grep -rn 'TODO\|FIXME\|STUB' include src | wc -l      # 19
```

## 6. Verificación de la línea base (BASE-02)

*Ejecutada el mismo 28/08/2026 en macOS (clang, Cocoa), sobre el árbol con
el trabajo concurrente declarado en §5. Se registran los fallos reales, sin
ocultarlos (regla de la ficha).*

| Comprobación | Resultado |
|---|---|
| `cmake -S . -B build` | **OK** (configura con el scaffolding INF-01 presente) |
| `cmake --build build -j8` | **OK, 0 errores**; solo warnings cosméticos del linker («ignoring duplicate libraries») |
| `ctest` | **10/10 en verde** (6 demos smoke GL-free + 4 unit: handle_pool, editor_state, project, save_game) |
| Demos GL-free (batería propia) | **36/36 en verde**, incluida la campaña completa (`demo_campana_boundington`, `demo_precuela_boundington`), el mundo (`demo_mundo`, `demo_este_norte`, `demo_este_sur`) y el motor Nd6 Ed. 3 (`demo_dice_engine`, `demo_battle_nd6`) |
| `tools/validar_enlaces.py` | **78 niveles, 138 puertas, 0 problemas** |
| `tools/conectividad.py` | Todo al 100% **excepto `mapamundi` (42%)** — fallo CONOCIDO y aplazado (bitácora Parte 0, 17/08 y 19/08): 13 `desembarco_landmass_*` inalcanzables andando; el modelo walk-only no encaja con un mapa de islas. Se aplaza a la tarea de viajar entre naciones |
| `tools/validar_todo.py` | FAIL por el error conocido de conectividad + **14 avisos** = deuda de contenido registrada (deidades ausentes de la Parte 4.1: `el_sepulturero_amable`, `la_luna_hueca`, `el_toro_de_gongorguma`; más `la_serpiente_de_contratos`, `morpheo`, `self`; el monstruo `paje_traslucido`; y `mundo`/`este_sur` sin `entrada`, esperado en la tabla 7.9) |
| Demos GL (11) + `juego` + `level_editor` | **No verificados en esta pasada**: necesitan ventana; en CI corren bajo `xvfb` |

### Observaciones de CI (para INF-01/INF-05)

- `ci.yml` corre SOLO en `ubuntu-latest`: Debug + ASan/UBSan, dos demos GL bajo `xvfb`.
- El CI **no ejecuta ctest** todavía: el scaffolding de pruebas vive sin
  commitear en el árbol (INF-01 en curso); cuando aterrice, `ctest` debe
  entrar en el workflow.
- macOS queda hoy como segunda plataforma verificada en local (esta pasada);
  Windows no se ha compilado nunca (deuda de INF-05/TOOL-10).

### Lectura BASE-02

La base compila y pasa todo lo verificable sin ventana. El único rojo es la
deuda aplazada del mapamundi, que ya tiene dueño y tarea asignada. La línea
base está **congelada y verificada**: BASE-01 + BASE-02 cerrados.

## 7. Demos GL vs GL-free (INF-05.3)

Inventario congelado para el CI multiplataforma.

**Necesitan ventana (GL):** `juego`, `level_editor` y estos 11 demos:
`creacion_personaje`*, `demo_animated_entity`, `demo_battle_hud`,
`demo_dynamic_lights`, `demo_fog`, `demo_hud`, `demo_isometric_renderer`,
`demo_lighting`, `demo_lut`, `demo_textured_quad`, `sandbox_window`.
En el CI de Linux corren bajo `xvfb` los que acaban en 5 frames solos
(`demo_textured_quad 5`, `demo_isometric_renderer 5`).

(*) `creacion_personaje` abre ventana por construcción, pero su lógica de
pantalla es GL-free y `demo_creacion_personaje` pasa sin ventana.

**GL-free (el resto, 36):** verificados en la tabla de §6. Los registrados
en ctest llevan etiqueta `gl-free` (y `smoke`/`unit`); el CI los ejecuta en
las tres plataformas vía `ctest -L gl-free`.
