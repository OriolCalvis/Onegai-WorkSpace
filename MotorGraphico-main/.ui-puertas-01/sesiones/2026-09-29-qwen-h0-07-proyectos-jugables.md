# 2026-09-29 · qwen · H0-07/H3-01: dos proyectos publicados no eran jugables

**Encargo de Oriol (29/09):** «coge el gantt del motor graphico y sigue programando,
gracias. utiliza mapadelcodigo, pero piensa que hay mas agentes trabajando en el
proyecto de manera simultanea».

- **Commits:** MotorGraphico-main `23be3cb` (2 manifiestos + la prueba). En el repo
  raíz, el commit que incluye esta sesión y el TABLERO — construido como HEAD + solo
  mis hunks (blob + `update-index`), porque `TABLERO.md` está sucio con filas de
  Codex, opencode y la otra sesión qwen.
- **Deja esperando a Oriol:** (1) la entrada de `este_sur` es **provisional** — la
  evidencia es canónica pero elegir la puerta de una región es suyo; (2) la
  discrepancia de H0-07, marcada HECHO el 22/09 sin cumplir su criterio de cierre.

## Qué elegí y por qué (Gantt Fase H + `MAPA_DEL_CODIGO.md` §1 ficheros de cruce)

La instrucción de Oriol era explícita sobre la concurrencia, así que la elección de
tarea se hizo contra `git status`, no contra el Gantt. Con **166 entradas sin
versionar** en `MotorGraphico-main`, casi todo lo valioso del Gantt está ocupado:

| Tarea del Gantt | Por qué NO |
|---|---|
| H0-08 eventos de mapa (`MapEventRunner`) | **Otra sesión qwen en vuelo ahora mismo**: `include/RPG/MapEventRunner.h` y `src/RPG/MapEventRunner.cpp` existen desde las 01:25 de hoy y su fila del TABLERO estaba actualizada con las mismas rutas que yo iba a usar. La sesión terminó y lo commiteó como `bdcb3ff` mientras yo escribía esto. Regla 2. |
| H3-02 diario de misiones | Claude: `include/Game/DiarioMisiones.h`, `src/Game/DiarioMisiones.cpp`, `tests/Game/DiarioMisionesTests.cpp` y `seguimiento/MISIONES-01.json`, todo `??`. |
| H1-03 / H1-05 / H2-05 / H2-08 | opencode, con `Application.*`, `GameSession*` e `InventoryScreen.*` sucios. |
| H1-01 G-13 a producción | Ya hecho en el árbol sin commitear (`include/Game/BattleMenuActions.h`, consumido por `Application.cpp:45`). |
| H2-04 hitos y tierUp | La cerró zcode el 28/09 (`sesiones/2026-09-28-zcode-h204-hitos-tierup.md`). |
| H2-07 monstruos con `maxHealth=0` | **Premisa caducada**: `CatalogLoadersCriaturas.h:42` ya lee `defenses.maxHealth` → `derived.vida` → raíz, y las **432/432** fichas llevan el dato (0 discrepancias entre `defenses` y `derived`). |
| H0-02 teclas MenuLeft/MenuRight | Ya hecho: `src/Engine/Input/InputSystem.cpp:34-35` (opencode, 23/09). |
| H4/H5 de autoría | `EditorApplication.cpp`, `LayerPanel.cpp`, `EditorState.*` sucios (Codex E-12) y Claude está cableando `InspectorPanel`/`ToolsPanel`/`EditorIconos` (`??`). |

Lo que quedó libre y en ficheros **limpios**: la cadena proyecto → título.

## El defecto

`assets/proyectos/mundo.json` y `este_sur.json` llevan `"entry": ""` desde el 16/08.

- `Project::playable()` es `!entry.empty() && !levels.empty()` → falso en los dos.
- `TitleScreen::fromProjects` los manda a `skipped()`: el título **no los ofrece**.
  Ese es literalmente el criterio de cierre de H0-07 («en el título aparecen
  `este_sur` y `mundo`, que hoy no salen»).
- `validar_todo.py [4/5]` listaba 4 proyectos de 6, y los otros dos salían como
  **AVISO** («sin entrada: el proyecto se edita pero no se juega»), no como error.
  Un aviso que nadie mira.

**Y el agujero de la prueba, que es la parte interesante.** `tests/Project/ManifestTests.cpp`
—que sí lee los manifiestos publicados reales y está registrado en CTest como
`test_project_manifests`— comprobaba la entrada **dentro** de un condicional:

```cpp
if (!indexed->entry.empty())
    require(indexed->entryKind != Project::EntryKind::Missing, ...);
```

Con la entrada vacía la comprobación no se ejecutaba. Era una prueba verde que no
probaba nada justo en el caso que importa: el mismo patrón que las sesiones
`bcc15ad34` y `b301a7464` encontraron tres veces seguidas.

## Por qué `mapamundi.json` es la entrada de `mundo` (evidencia, no invención)

- `tools/gen_mapamundi.py` documenta la cadena que construye:
  `mapamundi --desembarco--> mundi_landmass_N --ciudad--> ciudad_en_X`, y su línea
  265 añade las puertas de **regreso** apuntando a `assets/levels/mapamundi.json`.
- `assets/levels/mapamundi.json` (`name: "Egaroth"`, `_fuente:` generado por ese
  script) tiene `playerStart`, `map: assets/maps/mapamundi.tmx` (existe, 29 KB), 8
  desembarcos con destino y 6 sin él. Medido: los 8 cuadrantes apuntan a él
  (in-degree 8).
- **No estaba declarado en ningún manifiesto** (verificado con `grep -rn mapamundi
  assets/proyectos/`: solo salía el catálogo `mapamundi_objetos.json` y el nombre
  del proyecto). O sea: la raíz del grafo del mundo era huérfana del sistema de
  proyectos. Ahora `mundo` declara 9 niveles y `validar_todo.py [3/5]` los valida
  contra sus catálogos sin un solo error nuevo.

Los 6 `desembarco_landmass_9..14` sin destino **no son un bug y no se tocan**: el
canon tiene CATORCE masas (`world_grid.json`, rejilla 26×26) y solo ocho tienen
mapa. El generador los deja marcados a propósito —«un mapamundi al que le faltan
seis islas sin decirlo es un mapamundi que miente». Es el «mapamundi al 42%» que el
TABLERO aplaza a la tarea de viajar entre naciones (H3-01).

## Por qué `es_taurengrad.json` para `este_sur` — PROVISIONAL, para Oriol

Evidencia, de `assets/world_experiment/chatgpt/este_sur/locations.json`:
`east_south_taurengrad_complex` es el único `kind: city_complex` del paquete, con
`era_start 8739 → era_end 2000` (vigente en la fecha del snapshot) y
`status: canon_previo`; es el nudo de las `Rutas del Laberinto` que conectan las
aldeas minotauras. Las otras dos candidatas (Dhin Thyraxion «Ciudad Caldera» y Naka
tol «Cúpula de Mareas») tienen in/out = 1 y ninguna designación equivalente.

**No es una decisión de canon**: `connections.json` prohíbe fijar unilateralmente
la frontera, la ruta o la capital histórica de **Bosmurg**, y Taurengrad no es
Bosmurg. Aun así es un campo reversible y se registra aquí para que Oriol lo vete
si prefiere otra puerta.

## Qué cambié — 3 ficheros, los tres LIMPIOS antes de tocarlos

| Fichero | Cambio |
|---|---|
| `MotorGraphico/assets/proyectos/mundo.json` | `entry: "mapamundi.json"`; `mapamundi.json` a `levels`; `mapamundi.tmx` a `maps`. +3 −1 |
| `MotorGraphico/assets/proyectos/este_sur.json` | `entry: "es_taurengrad.json"`. +1 −1 |
| `MotorGraphico/tests/Project/ManifestTests.cpp` | La comprobación de entrada pasa de condicional a incondicional, con `kSoloEscenarios` (hoy vacía) como única vía de excepción explícita. +36 −2 |

**`CMakeLists.txt` no se toca**: al extender una prueba ya registrada no hace falta
nada. Eso importa porque está sucio con trabajo de opencode y Claude encima, y cada
línea ahí es un conflicto.

Sobre `kSoloEscenarios`: `ProjectIndex::check()` documenta que un pack **puede** ser
solo escenarios y que eso «no es un error de datos». No se contradice: se exige que
sea una decisión explícita. Un proyecto publicado que no se puede jugar se lista ahí
con su motivo; si no está, se publica para jugarse. Así el siguiente no vuelve a
pasar en silencio, y nadie tiene que inventar doctrina nueva.

## Verificación

- `tools/validar_todo.py`: **[4/5] de 4 a 6 proyectos OK**; avisos **14 → 12**;
  `RESULTADO: OK — 0 errores`. `[1/5]` intacto (`validar_enlaces` 78 niveles / 138
  puertas / 0 problemas; `conectividad`; `validar_mundo_unido` «todo se alcanza y de
  todo se vuelve»). `[3/5]` valida los 9 niveles de `mundo` contra sus catálogos.
- Build **aislado** en `out/build/qwen-h007` (convención del repo: un directorio por
  agente, para no pisar las builds de opencode/codex). Configuración OK, 108
  objetivos gl-free.
- `ctest -R "project|title|validar_todo|mision_llave"`: **13/13 PASS**, incluidos
  `demo_title_menu` (Claude, G-08.1: `TitleMenuState::fromManifest` contra
  manifiestos reales), `demo_mision_llave` (aceptación de E-09) y `test_title_screen`.
- **Falsabilidad comprobada**: con `"entry": ""` en la copia de assets del build (no
  en el fuente), `test_project_manifests` sale con **código 1** y el mensaje nuevo.
  Restaurado con `copiar_assets` y vuelta a verde. Antes del cambio, ese mismo caso
  pasaba en silencio.
- Los 3 ficheros estaban limpios antes, así que el commit es por pathspec normal: no
  hace falta el truco de blob+update-index en este repo. En el repo `Software/` sí
  (`TABLERO.md` está sucio con filas ajenas).

## Hallazgos que NO arreglo (reglas 2 y 6)

1. **HEAD de `MotorGraphico-main` está rojo.** `Application.cpp:905` —versionado—
   llama `battle->tickRound()`, y ni la declaración (`BattleState.h:166`) ni la
   definición (`BattleState.cpp:379`) están en HEAD: viven solo en el árbol. Un clon
   limpio no compila `motor_core`. Verificado con `git show HEAD:...` y no de
   memoria. La cura es de opencode (zona H0-04/H0-05); no la commiteo. Yo he
   trabajado en el árbol, que es lo que hacemos todos.
2. **H0-07 estaba marcada HECHO (22/09, opencode)** con el cableado
   `init(startAtTitle)` → `TitleMenuState`, pero su criterio de cierre («en el
   título aparecen `este_sur` y `mundo`») no se cumplía: no hay cableado que valga
   si los manifiestos no declaran entrada. No deshago ni reabro su fila: la cierro
   yo y lo registro aquí.
3. **El Gantt de Fase H (17/09) tiene las premisas caducadas.** H0-02, H1-01, H2-07,
   H3-04 y H3-05 ya están hechas y H2-04 la cerró zcode: 6 de 60 tareas. Elegir una
   tarea libre me costó ~15 comprobaciones contra `git status` y el código. Conviene
   una pasada de **H8-04** («25 docs desactualizados») sobre el propio Gantt antes de
   que otro agente vuelva a programar algo ya cerrado. `JUGABILIDAD_FASE_H.md` y
   `gantt_jugabilidad_H.puml` siguen además **sin versionar** (`??`).
4. **12 avisos de contenido, todos aguas arriba.** 11 `deidadIds` y 1 `enemigoIds` de
   `assets/catalogs/adventures.json` que no existen en `deities.json`/`monsters.json`.
   Medido: las 10 deidades del motor son **exactamente** las 10 cartas de
   `dndWeebCC-master/data/cartas/deidades/`, o sea que el motor es fiel y lo que
   falta está en la capa de cartas — la BITACORA del 12/08 ya lo dejó a medias
   («0/17→10/17 resueltas»). `morpheo` y `self` no parecen deidades de Egaroth. No lo
   toco: el flujo es `canon → cartas → motor` y dndWeebCC no es mi fila.
5. **`demo_proyectos` no está registrado en CTest** (`CMakeLists.txt:975` lo excluye
   porque necesita `tools/build_proyecto.py` en el cwd, y CMake solo copia `assets/`).
   Es el único ejecutable que recorre el índice real y lista `orphans()`, que es la
   red que habría cantado `mapamundi.json` como huérfano. Promoverlo pide decidir si
   se copia también `tools/` (TOOL-09). Se señala, no se hace.

## Cierre de filas propias y bandejas

`MAPA-CLASES` (28/09) seguía en «En curso» con los 6 ficheros + el índice
commiteados en MG `bb0147e`. No estaba terminada —la sección 6 de `00-INDEX.md`
deja tres cabos—, así que en vez de moverla sin más se reparte lo que falta donde
toca, y la fila pasa a «Hecho» para que nadie vea `juicio_clases/` como zona
ocupada:

- a **«Pendiente — libre»**: `07-editor.md` (37 clases del editor + las 12
  cabeceras de su capa de estado) y el arreglo de `tools/medir_alcance.py` que
  `D0007` necesita antes de re-congelar la línea base (§6.3).
- a **«Esperando a Oriol»**: las **4 decisiones** del §6.2, que hasta hoy no
  estaban en su bandeja y por tanto no existían para nadie: iniciativa (§3.16),
  `SceneManager` (§3.2), los cuatro subsistemas compilados que no están en el
  binario (§3.9) y el cableado del editor a su capa de estado (§4). Sin ellas el
  diseño de «lo que hay + lo que esperamos que haya» no se puede escribir sin
  inventar la respuesta.
