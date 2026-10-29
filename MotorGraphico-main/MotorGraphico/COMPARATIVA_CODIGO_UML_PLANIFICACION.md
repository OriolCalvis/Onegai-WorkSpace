# Comparativa: estructura del código vs UML vs planificación general

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe la comparativa entre la estructura del código, el UML y la planificación de entonces, foto del 03/09/2026. El mapa código↔paquetes se genera ahora con `tools/marcar_paquetes.py --mapa` en `planificacion/MAPA_DEL_CODIGO.md`, y el de clases con `tools/mapa_clases.py`. De las 7 citas de codigo que contiene, 7 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

*Foto del 3 de septiembre de 2026 sobre HEAD `50098f8` + `2ca74eb` (C-20 de codex, ya commiteado) y el trabajo en vuelo declarado en el TABLERO (migración INF-03 sin commitear). Encargo de Oriol: analizar la estructura del código, compararla con el UML y la planificación, plantear preguntas y remarcar puntos de mejora. Documento hermano de `LINEA_BASE.md` (foto de módulos, 28/08) y de `analisis_clases/` (foto clase a clase, 31/08): aquí el eje es la **comparación contra los diagramas y los planes**, no el inventario puro.*

## 0. Titular

**El UML de clases no se toca desde el commit inicial del repositorio** (`2f801c5`, 11/08, «antes de la integración Onegai-Core»). Describe el motor isométrico de las Fases 1–12: ~60 elementos (clases, interfaces, enums) y unos pocos miles de líneas. El código de hoy son **101 cabeceras, ~23.800 líneas en `include/`+`src/`** (más 2.708 del editor, ~10.200 de demos y ~3.500 de tests): el UML se quedó a tres generaciones de distancia y **todo lo que define la identidad actual del proyecto —el módulo RPG completo, la narrativa, los proyectos editoriales, el guardado, la nueva arquitectura Runtime— no aparece en él**.

**La planificación, en cambio, ya se reconcilió consigo misma.** La auditoría del 28/08 (`planificacion/ESTADO_REAL_vs_ROADMAP.md`) detectó que «el Gantt está planificado sobre una línea base que ya no existe», y el plan definitivo (`planificacion/PLA_DEFINITIU.md`, corte B, fases C–G) se genera desde una única fuente de datos (`planificacion/pla/dades.py`) para que markdown, Gantt y web no puedan divergir. El trabajo vivo (Fase C: C-09/C-10/C-21 cerrados por qwen, C-20 cerrado por codex, ambos el 03/09) sigue el rastro del plan con sesión y commit.

Quedan tres desajustes accionables, por este orden:

1. **El UML como pieza viva de arquitectura no existe**: nadie lo ha actualizado en 15 commits-worth de evolución, y las notas que contiene afirman cosas del código que ya no son ciertas (ver §2.4).
2. **Conviven tres numeraciones de fases y dos copias de algún Gantt** (subtareas F0–F8 del roadmap de 98 paquetes, F0–F5 del editor por decisión 2.18 de la bitácora, y C–G del corte B) — cada doc hay que fecharlo antes de saber si manda.
3. **La reestructuración INF-03 sigue sin commitear**: módulos enteros (`Map/ World/ Runtime/ Import/ Project/`, `editor/`, `tests/`) viven untracked y ~25 ficheros modificados. Es el riesgo nº1 de colisión entre agentes (ya registrado en `analisis_clases/INDEX.md`, falta nº1).

---

## 1. La estructura real del código (foto 03/09)

12 módulos en `include/`+`src/` con espejo de implementación, más el editor visual y dos árboles de verificación:

| Módulo | LOC | Contenido esencial | Estado git |
|---|---:|---|---|
| `Core/` | 1.702 | JSON propio (parser + writer con \uXXXX), math isométrica, ids fuertes, `HandlePool` generacional, `Result<T>`, excepciones, `ResourceManager<T>` | commiteado |
| `Render/` | 4.614 | Camera, SpriteBatch (ring buffer), TileMap (adaptador), IsometricRenderer, post-FX, luces dinámicas, sombras blob, HUD (7 widgets) y BitmapFont con acentos | mezclado (DOC-03 en vuelo) |
| `RPG/` | 5.599 | DicePoolEngine (enfrentada Ed.3), CharacterSheet único, Inventory/Condition/Skill engines, NarrativeEngine (beats/flags/objetivos/timers), CharacterCreation, 21 definiciones + 20 catálogos tipados | mezclado (C-20 de codex, NarrativeEngine en vuelo) |
| `Game/` | 3.258 | GameSession (6 modos), BattleState Nd6, EnemyBrain, Morality, SaveGame atómico | SaveGame sin trackear |
| `Engine/` | 2.802 | Application, Window, ApplicationLoop fixed-step, InputSystem, SceneManager | Application/Window modificados en vuelo |
| `Editor/` | 2.535 | EditorState (capas, undo por snapshot, clipboard), ObjectInspector, ProjectHub, ProjectIndex | mezclado (INF-03) |
| `Project/` | 1.245 | Project, ProjectSettings, ProjectSerializer canónico v1, DocumentRef | **sin trackear** |
| `Level/` | 887 | LevelDefinition/LevelLoader, ObjectCatalog, WorldTopology | mezclado |
| `Map/` | 867 | MapDocument, capas tipadas (Tile/Entity/Region) | **sin trackear** |
| `Import/` | 296 | TmxMapImporter (TMX como importación, no fuente de verdad) | **sin trackear** |
| `Runtime/` | 239 | RuntimeTileMap + builder en memoria (sustituye el round-trip por texto) | **sin trackear** |
| `World/` | 241 | World ECS-lite con EntityHandle generacional | **sin trackear** |
| `editor/src/` | 2.708 | level_editor.cpp (movido desde `examples/` por INF-03) | **sin trackear** |
| `tests/` | ~3.540 | 15 suites unitarias + integración (MapRoundTrip, ConditionEngine, World, SceneManager…) | **sin trackear** |

Los ficheros más grandes del árbol marcan dónde está la deuda estructural: `level_editor.cpp` (2.708), `src/Engine/Application.cpp` (1.726), `src/Editor/EditorState.cpp` (889), `include/RPG/Catalogs/RpgCatalogs.h` (845), `src/Game/GameSession.cpp` (813).

> Trabajo concurrente respetado según protocolo: nada de lo anterior se ha tocado ni commiteado desde esta sesión.

---

## 2. Comparativa con el UML (`motor_grafico_clases.puml`)

### 2.1 Lo que sigue siendo fiel

Las fundaciones del diagrama aguantan bien y siguen siendo la columna vertebral del motor: `Core::Math` (Vector2/GridCoord/UVRect), la jerarquía de `EngineException` con `Result<T>` para rutas calientes, `ResourceManager<T>` con TextureManager/ShaderManager, el pipeline `Render` (Camera→SpriteBatch→TileMap→IsometricRenderer con painter's algorithm, luces y sombras blob — cuyas notas del UML siguen siendo exactas), el framework de HUD (IHudElement/HudTransform/widgets/BitmapFont) y `JsonValue`. La división GL-free vs GL que el UML anota en notas («se comprueba sin ventana») se convirtió en el criterio arquitectural del repo (suite `gl-free`, ADR-004/007/008).

### 2.2 Lo que existe pero ya no es como el diagrama dice

| Concepto | El UML dice | El código hoy | Gravedad |
|---|---|---|---|
| `Enemy` | subclase de AnimatedEntity con `m_aiState`, implementa ICombatant | **`Game/EnemyBrain`** (renombrado): IA de patrulla/persecución, constructor distinto, `populate_from_monster_catalog()` y `RPG::CharacterSheet*` dual-mode | alta — buscar «Enemy» guía a un fichero que ya no manda |
| `Player` | `m_health` + `m_inventory` propios, ICombatant directo | dual-mode: `m_sheet` externo autoritativo con fallback legacy; el PJ sigue **sin** `populate_from_class_race_background()` (deuda 4.3) | alta |
| `Skill` + `ApplySkillEffect()` | struct plano (mpCost/power/effect/target) + función de 2 args | `SkillDefinition` (20+ campos) + `SkillExecutor` pipeline de 9 pasos con enfrentada Nd6 Ed.3, sin CA/CD; el demo_skills aún parchea la API legacy de 2 args | alta |
| `BattleState` | cola de turnos simple, IA determinista, 3 desenlaces | `BattleStateOnegai`: enfrentada pura, condiciones con stacking, oleadas encoladas, `syncBattleOutcome` | media |
| `TileMap` | fuente de verdad del grid, `loadFromFile` TMX | adaptador: hereda de `RuntimeTileMap` (DOC-03 en vuelo); TMX pasa a formato de **importación** (`Import/TmxMapImporter`) | media |
| `EditorState` | 4 herramientas, 1 capa, sin undo, MVP | **7 herramientas** (PaintTile, EraseTile, PlaceObject, RemoveObject, SetPlayerStart, FillTiles, LinkLevel), capas múltiples renombrables, undo/redo por snapshot, clipboard, inspector por instancia, eventos, botín, variantes | media |
| `ObjectCategory` | Prop/Enemy/Pickup | Prop/Enemy/Pickup/**Npc** | baja |
| `BitmapFont` | «solo mayúsculas + dígitos», atlas 16×4 | atlas 16×5 con acentos (À Á È É Í Ò Ó Ù Ú Ñ Ç Ü «·» ¿ ¡) + `wrapText`/`clipText` | baja — pero las notas del UML afirman lo contrario del código |
| `Application` | 9 miembros; orquesta managers | **~80 miembros**: tienda, negocio, temporizadores, tablas raciales/de frames, creación de personaje… | alta (ver §2.4) |

### 2.3 Lo que directamente no aparece en el UML

Paquetes enteros de la identidad actual del motor, ausentes del diagrama:

- **`RPG/` al completo salvo el esbozo Skill/SkillSet/SkillCatalog/ICombatant**: DicePoolEngine (el corazón del reglamento Ed.3), CharacterSheet, InventoryEngine, ConditionEngine, SkillExecutor, **NarrativeEngine** (beats, flags, objetivos, temporizadores F1 — la aventura entera vive aquí), los 20 catálogos tipados (2.416 entradas) y CharacterCreation (14 pasos).
- **`Game/`**: SaveGame atómico (F5/F9), Morality, EnemyBrain.
- **`Project/`**: ProjectIndex/ProjectHub/Serializer/Settings/DocumentRef — el sistema de proyectos (Parte 7 de la bitácora), que resuelve el problema de los 96 niveles de cuatro autores.
- **`Map/ World/ Runtime/ Import/`**: la nueva arquitectura documental (MapDocument, capas tipadas, ECS-lite, builder en memoria) — precisamente la capa que los ADR-001..010 y MIGRATION_MAP describen.
- **`tests/`**: 15 suites; el UML ni menciona verificación más allá de demos.
- **Engine nuevo**: ApplicationLoop fixed-step (ADR-007), InputSystem con bindings (ADR-008), SceneManager con pila y transiciones diferidas (ADR-009).

### 2.4 El detalle que más engaña

La nota del UML bajo `Application` dice: *«NO decide reglas: traduce teclado a llamadas de GameSession y dibuja el resultado»*. Hoy `Application.cpp` (1.726 líneas) contiene lógica de tienda, de negocio, de temporizadores, tablas raciales de frames hardcodeadas y la creación de personaje. **El diagrama describe el ideal al que la ficha RUN-01 llama «adelgazarla» — y el código se alejó de él.** Un diagrama que solo se lee como «qué hay» no delata esta deriva; uno con notas normativas, sí. Ese es exactamente el valor que el UML podría aportar y hoy no aporta porque está congelado.

---

## 3. Comparativa con la planificación general

### 3.1 Genealogía de los planes (fechar antes de creer)

| # | Documento | Fecha | Alcance | Estado |
|---|---|---|---|---|
| 1 | `motor_grafico_gantt.puml` | pre-11/08 | Motor isométrico, fases 1–5 | histórico, cerrado |
| 2 | `motor_grafico_gantt_rpg.puml` | pre-11/08 | Fases RPG 6–12 | histórico, cerrado |
| 3 | `planificacion/motor_grafico_roadmap_ejecutable_v2.md` | ~24/08 | Creator 1.0: 98 paquetes, 629 j, fases 0–8 | base de la auditoría; numeración F0–F8 |
| 4 | `planificacion/ESTADO_REAL_vs_ROADMAP.md` | 28/08 | Auditoría plan↔código: 32% global, +INF-01..06, 3 REPLANTEAR, cortes A/B/C | foto congelada — **ya desfasada por INF-03 en vuelo** (dice «INF-03 0%» y el árbol tiene 5 módulos nuevos hechos) |
| 5 | `planificacion/PLA_DEFINITIU.md` (+`pla/dades.py`, Gantt corte B, web) | 31/08 | **El plan vivo**: corte B, fases C–G, 54 paquetes, 177 j ≈ 118 semanas | vigente, generado de una sola fuente |
| 6 | `planificacion/subtareas/F0..F8.puml` + `gantt_subtareas_F0.puml` (copia en la raíz de Software/) | ~28/08 | Subtareas atómicas del roadmap de 98 paquetes | ¿desfasado respecto al corte B? nadie lo ha declarado |
| 7 | `ROADMAP_RPG_EDITOR.md` | 17/08 | Producto editor F0–F5 (decisión 2.18 de la bitácora) | pista paralela de producto, convivencia no resuelta en los docs |
| 8 | `GAMEMACHINE_NECESIDADES.md` | ~14/08 | Mapa de brecha de 24 subsistemas contra el GDD | referencia; parcialmente absorbido por 3–5 |
| 9 | `architecture/` ADR-001..010 + MIGRATION_MAP | 24–28/08 | Decisiones de la arquitectura nueva | vigente; es «el UML que sí se mantiene» |

### 3.2 Estado del plan vivo contra el código

- **Fase C (Consolidación) en marcha y trazada**: C-09 (propiedad GL de Texture), C-10 (round-trip JSON con \uXXXX) y C-21 (retirada de IRng/SeededRng) cerrados el 03/09 por qwen; C-20 (un solo escritor de condiciones) cerrado el 03/09 por codex con 11 regresiones y ASan/UBSan, dejando el resto (enganche de `on_damage_received` al combate) en la lista libre. Toda sesión tiene ficha en `sesiones/` y commit: el sistema D0001 funciona.
- **Editor F0–F5 (pista producto)**: F0 (auditoría/barrido de etiquetas) cerrado el 28/08; F1 (eventos + temporizadores) a medias — la fila del dt activo lleva en el TABLERO desde el 19/08.
- **El patrón que la auditoría del 28/08 destapó sigue siendo el riesgo estructural de la planificación**: los planes se escriben sobre fotos del código, el código evoluciona a saltos (la reestructuración INF-03 metió 5 módulos en pocos días), y los docs intermedios envejecen sin aviso. `PLA_DEFINITIU` lo mitiga generándose de `dades.py`; `ESTADO_REAL` y `LINEA_BASE` lo mitigan declarándose fotos fechadas. Los Gantt sueltos (fila 6 de la tabla) no llevan ninguna de las dos defensas.

### 3.3 Desajustes plan↔código vivos

1. **Tres numeraciones de fases coexisten sin mapa de equivalencias**: F0–F8 (roadmap 98 paquetes y sus subtareas), F0–F5 (editor, decisión 2.18) y C–G (corte B). Un agente nuevo que lea el TABLERO («Fase C») y `ROADMAP_RPG_EDITOR.md` («F0») no tiene forma de saber que son pistas distintas con dueños distintos.
2. **`gantt_subtareas_F0.puml` duplicado**: existe en `Software/` (raíz del workspace) y en `MotorGraphico/planificacion/subtareas/`. Es la trampa de los duplicados de la Parte 3 en versión planificación: nadie sabe cuál manda.
3. **`ESTADO_REAL_vs_ROADMAP.md` miente por omisión**: es foto del 28/08 y lo dice, pero es además el doc que alguien consultaría para «saber el estado» — y subestima INF-03/DOC-01..03 en ~5 módulos y 15 suites de tests.
4. **`motor_grafico_flujo.puml` (raíz del repo)**: mismo problema que el de clases — congelado en `2f801c5`, describe un `Application::init/run` sin tienda/negocio/modos, y convive con ADR-007 (fixed-step loop) que dibuja otro bucle distinto del que él pinta.

---

## 4. Preguntas (a la bandeja de Oriol / del equipo)

1. **¿El UML de clases se mantiene como diagrama vivo o se declara histórico?** Si vivo: ¿quién lo actualiza y con qué regla (¿cada commit que añade un módulo, ¿una tarea de la Fase G)? Si histórico: ¿le ponemos nota de cabecera y remitimos a `analisis_clases/`? Hoy es lo primero que un recién llegado leería como «la arquitectura», y está tres generaciones atrás.
2. **¿Qué numeración gobierna el trabajo hasta el corte B?** Si es C–G (lo que sugiere `PLA_DEFINITIU`), ¿se retiran las `subtareas/F0–F8` del árbol activo (a `planificacion/historico/`) o se regeneran como subtareas de C–G? ¿Y cómo se relaciona la pista producto F0–F5 del `ROADMAP_RPG_EDITOR` con la C–G — quién coordina el cruce?
3. **¿Cuándo se commitea la reestructuración INF-03?** Módulos enteros sin trackear + ~25 ficheros M es el riesgo nº1 de colisión (ya mordió el 16/08 con la carrera de git). ¿Su dueño la cierra, o se coordina un commit por pathspec cuando ninguna sesión esté tocando esos ficheros?
   > *Errata del 03/09 (tarde):* resuelto mientras se escribía la REVISION_GLOBAL_Y_MENUS.md — Oriol autorizó el rescate y qwen lo ejecutó (`9e40405`, `52d696f`, `dccdbfe`). Quedan solo los CI/consolidaciones de C-11/C-20/C-22, ya registrados en el TABLERO.
4. **RUN-05 (migrar Entity a componentes) vs el `World/` ECS-lite ya escrito**: el roadmap original planifica migrar la herencia a componentes; la Fase C ya trae un World generacional probado pero **sin consumidor**. ¿Cablearlo en la consolidación o marcar RUN-05 como REPLANTEAR cuarta?
5. **¿El adelgazamiento de `Application` (1.726 líneas con tienda/negocio/timers/tablas) tiene hueco en el corte B** o se registra como deuda explícita con ficha? El UML y RUN-01 ya lo piden; la Fase C es el momento natural («cap concepte amb dues implementacions vives»).
6. **`MotorGraphico-HUD-Redesign/`** (carpeta con `.design`, manifest y validation report en la raíz del repo): ¿es una propuesta viva de otra sesión, material de estudio o descartable? No aparece en el TABLERO ni en las sesiones.
7. **`Incomplete request` + `Incomplete request.zip`** en la raíz de `MotorGraphico-main/`: ¿basura de una sesión cortada? (candidato obvio a INF-06, pero no lo borro sin confirmación — regla 2 del protocolo).

---

## 5. Puntos de mejora (priorizados)

**P1 — Decidir el destino del UML y ejecutarlo (barato, alto valor).** Opción A (mínima): nota de cabecera en `motor_grafico_clases.puml` y `motor_grafico_flujo.puml` — «histórico al 11/08, previo a Onegai-Core; la arquitectura vigente vive en `architecture/` y `analisis_clases/`» — media jornada. Opción B (la buena): diagrama por paquetes regenerado a partir de `analisis_clases/`, con la regla de protocolo «toca una clase pública → toca su diagrama de paquete», y separando GL-free/GL que ya es el criterio del repo. El UML es la única vista global que existe y hoymina la confianza de quien la abre.

**P2 — Un solo árbol de planificación vivo.** Declarar `PLA_DEFINITIU` + `architecture/` como el frente vigente; mover a `planificacion/historico/` los Gantt 1–2, el roadmap v2 y (si Oriol confirma la pregunta 2) las subtareas F0–F8; eliminar/deducar la copia de `gantt_subtareas_F0.puml` de la raíz de Software/. Cada doc histórico lleva su fecha de congelación arriba, como `LINEA_BASE`.

**P3 — Commitear INF-03 cuanto antes.** Es la mejora de mayor riesgo/precio actual: hasta que no aterrice, cualquier agente que toque Editor/Engine/Level puede pisar trabajo sin trackear, y los docs de estado (ESTADO_REAL, LINEA_BASE §contratos) siguen subestimando el árbol. Si no se puede cerrar aún, al menos una fila «En curso» en el TABLERO declarándola con su dueño.

**P4 — Adelgazar `Application` con criterio de Fase C.** Extraer a `Game/` (o a servicios) tienda, negocio, temporizadores visuales y tablas raciales hardcodeadas; dejar en Application solo orquestación de managers y traducción de input. Es la regla de oro del propio `ARCHITECTURE.md` («un concepto, una representación, un propietario») aplicada al bucle, y desbloquea que SceneManager/ApplicationLoop/InputSystem (ya escritos y testeados, hoy sin consumidor) se enchufen sin pelearse con el Dios actual.

**P5 — Cerrar las dobles representaciones ya detectadas** (condiciones CharacterSheet-vs-ConditionEngine, manifiesto ProjectIndex-vs-ProjectSerializer, triple vía de entrada InputSystem/KeyEdge/InputState). Ya están en el plan C; este punto solo pide no dejarlas caer delante de features nuevas — cada mes vivo, duplican el coste de toda ficha que las roza.

**P6 — Cobertura de tests donde está el juego, no solo la infra.** Los 15 tests nuevos cubren Map/World/Runtime/Engine/Editor; el camino jugable (GameSession/BattleState/NarrativeEngine) sigue verificándose por demos-aserción. Seguir promoviendo demos a ctest (como RUN-11 hizo con 25) y añadir la prueba de conformidad Java↔C++ de la Parte 4.3 antes de que el reglamento diverja más.

**P7 — Ejecutar INF-06 de una vez**: duplicados « 2» (`Check 2.h`, `FORMATO_NIVELES 2.md`, `motor_grafico_gantt_paquetes 2.svg`), salidas `.ppm`/`.png` en la raíz del motor, `Incomplete request*`, `.DS_Store` versionados. Es media jornada presupuestada que evita editar el fichero equivocado — la trampa documentada más reincidente.

**P8 — Si se regenera el UML (P1-B), que refleje las trampas reales**: dual-mode de Player/EnemyBrain con `CharacterSheet*` externo, el rename Enemy→EnemyBrain, las 4 categorías de objeto, las 7 herramientas, BitmapFont con acentos, y los módulos con estado git distinto (una marca [WIP] como hace `analisis_clases/`).

---

## 6. Verificación de esta comparativa

- Bitácora leída entera (PROTOCOLO, Parte 0, 2.1, Parte 3) + TABLERO antes de trabajar; fila reclamada antes de crear ficheros.
- Fuentes contrastadas: `motor_grafico_clases.puml` (íntegro), `motor_grafico_flujo.puml` (cabecera), `planificacion/ESTADO_REAL_vs_ROADMAP.md` (íntegro), `planificacion/PLA_DEFINITIU.md` (§1–4), `analisis_clases/INDEX.md`, `ROADMAP_RPG_EDITOR.md` (cabeceras), inventario de `planificacion/` y `architecture/`.
- Datos medidos en el árbol: LOC por módulo (`find include/<m> src/<m> -name '*.h' -o -name '*.cpp' | xargs cat | wc -l`), 101 cabeceras, `git log --follow` del UML (último toque: `2f801c5`, commit inicial), enums `ObjectCategory` (4 valores) y `EditorTool` (7) verificados contra los headers.
- Trabajo concurrente (C-20 de codex, migración INF-03) respetado: esta sesión solo añade este fichero y no commitea nada ajeno.
