# 2026-09-29 · qwen · El índice de proyectos se escribía dos veces, y sin atomicidad

**Encargo de Oriol (29/09, segunda tanda):** «coge el gantt del motor graphico y
sigue programando, gracias. utiliza mapadelcodigo, pero piensa que hay mas agentes
trabajando en el proyecto de manera simultanea».

- **Commits:** MotorGraphico-main `4f7996a` (3 ficheros: una cabecera interna nueva
  y los dos `.cpp` de `ProjectIndex`). Repo raíz: el commit que incluye esta sesión
  y el TABLERO.
- **Deja esperando a Oriol:** H8-06 (`Application.cpp`), ya con la medida hecha —
  ver «El cuello de botella es total».

## Por qué no una tarea del Gantt, y cómo elegí

El Gantt de Fase H se ha vaciado hoy a un ritmo que no da tiempo a elegir: entre
anoche y ahora otras sesiones han cerrado **H0-08, H1-01, H2-01, H2-02, H2-04,
H3-01, H3-02, H3-03, H3-04, H4-01, H6-03, H8-04, G-11 y AV0-01**, y están vivas
H3-06, H1-03, H1-05, H2-05, H3-08, E-12/H4-09, G11-RANURAS-IO y CAPTURAS-UI. Las
premisas de H0-02, H1-01 y H2-07 estaban además caducadas (ya estaban hechas).

Con **395 entradas sin versionar** en el repo, medí en vez de suponer:

| Zona | Estado |
|---|---|
| `src/Engine/`, `include/Engine/` | **Los 7 `Application*.cpp` sucios** (opencode H1-03/H1-05/H3-06, Claude `PantallaNuevaPartida`) |
| `Game/`, `Render/`, `HUD/`, `Editor/`, `editor/` | sucios (Codex, Claude, opencode) |
| `assets/` (maps 92, textures 42, levels 22, catalogs 5) | sucios |
| **`src/Project/`, `include/Project/`, `src/RPG/`, `Map/`, `Level/`, `World/`** | **LIMPIOS** |

Y `tools/buscar_sobrante.py --repetido` (registrado en CTest como
`test_buscar_sobrante`) señalaba, justo en la zona limpia, cuatro grupos de código
idéntico entre `ProjectIndex.cpp` y `ProjectIndexDuplicate.cpp`. Debajo había algo
más que estética.

## El defecto

`escribeIndice` —que reescribe `assets/proyectos/index.json`, **la lista de todos
los proyectos**— existía dos veces, byte a byte, y las dos hacían
`fopen(ruta, "wb")` y escribían encima. Un corte a mitad dejaba el índice vacío o a
medias, y el motor no encontraba ningún proyecto aunque los 6 manifiestos
estuvieran intactos.

No era un riesgo hipotético sino el mecanismo de siempre, con fecha de hoy: el
arreglo de la escritura atómica (`5ecaab2`, *fix(save): preserve previous files
during atomic replacement*) llegó a `SerializerHelpers.cpp` y a `SaveGame.cpp`, y el
índice se quedó fuera **en los dos sitios a la vez**. Los manifiestos ya se guardaban
con `saveProjectAtomic` y las partidas con `ProjectIO::atomicWrite`: el índice era el
único fichero del sistema de proyectos que se truncaba.

Las otras tres copias (`leeFichero` 16 líneas, `esc` 11, `existe` 8) no eran inocuas
tampoco: son las que hacen que un arreglo tenga que aplicarse dos veces.

## Qué hice

- **NUEVA `src/Project/ProjectIndexInterno.h`** (101 líneas): las cuatro funciones,
  una vez, `inline` (las incluyen dos unidades de traducción de la misma librería) y
  en `namespace Editor::ProjectIndexIO` con `using namespace ProjectIndexIO;` en cada
  `.cpp` — el mismo patrón que `ProjectIO` en `SerializerHelpers.h`, precisamente para
  que los cuerpos y las llamadas no cambien ni una línea. Vive en `src/` y no en
  `include/` siguiendo `src/Editor/ProjectMapsDetail.h`: no la incluye nadie más, y
  así no suma una cabecera pública sin consumidor a las 13 que ya hay.
- `escribeIndice` construye el texto y lo pasa por `ProjectIO::atomicWrite`. **Los
  bytes son exactamente los de antes**, que es lo que `test_project_manifests` exige
  con su ida y vuelta canónica.
- Devuelve `Result<bool>` en vez de `bool` y los tres puntos de llamada propagan
  `errorMessage()`: antes un fallo decía «no se pudo actualizar el índice» sin decir
  por qué, y es justo ahí donde el autor necesita el motivo.
- `ProjectIndex.cpp` 402 → 357 líneas; `ProjectIndexDuplicate.cpp` 359 → 313.
  **No toco `CMakeLists.txt`** (sucio, con trabajo de opencode y Claude): la cabecera
  nueva no es una fuente y los dos `.cpp` ya estaban en `motor_project`.

## Verificación (build aislado `out/build/qwen-indice`)

- `buscar_sobrante.py --repetido`: **12 grupos → 8**, y los cuatro de `ProjectIndex`
  fuera de la lista.
- `ctest -R "project|title|mision_llave|buscar_sobrante"`: **15/15 PASS**
  (`test_project_manifests`, `test_project`, `test_project_serialization`,
  `test_project_database`, `test_project_catalog`, `test_project_naming`,
  `test_project_levels`, `test_project_hub`, `test_project_panel`,
  `test_title_screen`, `test_title_flow`, `demo_title_menu`, `demo_mision_llave`,
  `test_buscar_sobrante`).
- `linea_base_core.py`: **no añade nada a la deuda** (0 menciones a
  `ProjectIndexInterno`); la única mención a `ProjectIndex` es una *baja* ajena.

## El commit, y por qué fue por pathspec

Al stagear mis tres ficheros apareció que **el índice de git es compartido y otra
sesión tenía 7 ficheros staged**: su H3-08 (`DefeatRecovery.h/.cpp`,
`GameOverScreen.h/.cpp`, `DefeatRecoveryTests.cpp`, `cmake/DefeatRecoveryTests.cmake`
y `CMakeLists.txt`, 512 líneas). Un `git commit` normal me los habría llevado y
atribuido. Se commiteó con `git commit -F … -- <mis 3 rutas>`, que construye un
índice temporal: **los 7 staged ajenos siguen exactamente donde estaban** (verificado
después con `git diff --cached --stat`). El hook `pre-commit` pasó porque ya va en
modo `--indice` y mira lo que se commitea, no el árbol.

Intenté antes la vía `GIT_INDEX_FILE` y el guard del shell la denegó («relocated Git
command through an unrecognized program»); el commit por pathspec hace lo mismo sin
pelearse con él.

## Lo que NO arreglo (reglas 2 y 6)

1. **`demo_nova_partida` está ROJO, y no es mío.** Elige clase del catálogo, y
   `assets/catalogs/classes.json` tiene **216 líneas sin commitear** donde
   `mago_chasqueador` —que **en HEAD no existe**, verificado con `git show`— entra con
   `startingEquipment` vacío. Falla el `require(!hoja.inventoryEquipment.empty())` de
   `examples/demo_nova_partida.cpp:154`. Es WIP de la importación de clases de
   compañeros; se documenta y no se toca.
2. **`linea_base_core.py` está en rojo por WIP ajeno:** 9 problemas instantáneos
   (`.cmake` que compilan fuentes sin versionar: `Pergamino.cmake`,
   `ProjectPanelTests.cmake`, `RegistroSpritesTests.cmake`, `MotorEditor.cmake`) y
   **44 deudas nuevas** contra la línea base del 28/09. Las 44 son la ola de mitades
   GL-free de hoy (`TargetSelection`, `DiceRollWidget`, `ConditionWidget`,
   `MissionTimerWidget`, `QuestLogState`, `WorldTravelState`, `OptionsFile`,
   `AudioDirector`, `Companeros`, `ProjectHub`…). El trinquete no se puede re-congelar
   sin decidir antes H8-06, y sigue pendiente el arreglo de `medir_alcance.py`
   (§6.3 del juicio de clases): la guardia aún lista `include/RPG/EventosInterno.h`
   como inalcanzable cuando lo incluyen cuatro `.cpp`.
3. **El cuello de botella es total, y ya está medido.** `buscar_sobrante.py --muertos`
   da **13 funcionalidades terminadas que nadie enchufó, 7.645 líneas**:
   `AssetBrowserState` (1.405), `MapEventRunner` (1.134), `TargetSelection` (839),
   `ProjectDatabaseState` (657), `Audio` (653), `QuestLogState` (570), `OptionsFile`
   (507), `ProblemsPanel` (454), `ConditionWidget` (346), `DiceRollWidget` (333),
   `TitleScreen` (280), `LootResolver` (241), `BattleMenuState` (226). Comprobado una
   a una: **las 13 necesitan un fichero que está sucio** (`Application.*`, HUD,
   `EditorApplication`). Cada sesión que entrega una mitad GL-free añade una a la
   lista. Eso es H8-06, y es la fila roja que el Gantt pone como prerequisito de
   H0-01/02/03/06/07.
4. **Mi H0-07 de ayer alimenta una vía muerta, y hay que decirlo.** `TitleScreen`
   (que lista TODOS los proyectos y por tanto enseñaría `este_sur` y `mundo`) **no la
   consume nadie**; el juego usa `TitleMenuState`, que abre UN proyecto. O sea: los
   dos proyectos ya son jugables y el índice ya los resuelve, pero **en el juego no
   hay ninguna pantalla que los ofrezca**. H0-07 sigue sin ruta de usuario visible, y
   el arreglo pasa por `Application.cpp`. Lo registro en vez de dar la tarea por
   cerrada del todo.
5. **`src/Editor/ProjectMapsDetail.h` es una tercera copia** de la misma I/O de
   proyecto (`readText`, `writeFile`, `removeAll`, `saveProject`…). Está en otra
   librería y unificarlas es un refactor mayor con `ProjectMaps` (H4-02); se señala,
   no se hace.
6. **El TABLERO tiene una tabla mal formada:** una fila de opencode
   (`MISIONES-01`) está insertada ENTRE la cabecera y el separador `|---|`. Es
   trabajo en vuelo, no se toca; se anota para quien consolide.
