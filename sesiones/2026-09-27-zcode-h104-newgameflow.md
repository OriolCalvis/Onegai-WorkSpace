# 2026-09-27 · zcode · H1-04 (Gantt Fase H, rodaja GL-free): Game::NewGameFlow — Nueva partida con ficha viva

**Encargo de Oriol:** seguir el Gantt del MotorGraphico usando el
MAPA_DEL_CODIGO, con más agentes trabajando a la vez. Fila reclamada en
TABLERO antes de tocar nada; zonas ajenas vivas respetadas.

## Qué había y qué faltaba

El Gantt de Fase H (`planificacion/gantt_jugabilidad_H.puml`, doc
`JUGABILIDAD_FASE_H.md` §H1) tenía **H1-04** («G-09: creación de personaje
integrada») libre: ninguna fila viva la reclamaba y su dependencia (H0-07,
título del manifiesto) está Hecha (opencode 22/09).

Estado real encontrado (MAPA_DEL_CODIGO + código, no rehacer lo vivo):

- `RPG::CharacterCreation` + `CharacterCreationScreen` **ya están en
  producción** (`motor_rpg`, con `test_character_creation_flow` 5/5 sobre
  assets reales). El «mover de tests/ a src/» de la ficha ya estaba hecho.
- `Player::populate_from_class_race_background` **ya está en HEAD** y
  testado (`tests/Render/PlayerPopulateTests.cpp`): la deuda BITACORA
  4.3/4.4.5 está saldada por otra sesión.
- Lo que faltaba de verdad: **el tramo entre la pantalla y el jugador**.
  `Screen::construir()` = `CharacterCreation::construir()` y NADIE llama a
  `recalcular()`: la ficha sale con `healthCap()==0` y defensas a cero
  (cache de `recompute_derived()`). `Player::set_character_sheet()`
  sincroniza `health = maxHealth = healthCap()` → **quien cablee «Nueva
  partida» tal cual instala un PJ a 0 PV**. Además la pantalla habla en
  teclas crudas ('w'/'s'/'+'/'\n'/ESC) y cada consumidor tendría que
  re-traducir InputAction por su cuenta.

## Qué hice

Rodaja GL-free con el patrón H1-03/H1-05 (producción primero, cableado de
`Application` para el dueño del cruce):

- **`include/Game/NewGameFlow.h` + `src/Game/NewGameFlow.cpp`** (nuevos,
  en `motor_level`): envoltorio de `CharacterCreationScreen` con estado
  `Creando/Terminado/Cancelado`, navegación al estilo `OptionsState`
  (`moveUp/moveDown/adjustLeft/adjustRight/confirm/cancel/textChar/
  backspace`; ESC en Raza cancela, en los demás pasos retrocede; input
  restante = no-op tras terminar) y **`finalizar(TierRules)`** =
  `construir() + recalcular()` → ficha con vida/defensas al día y su
  equipo inicial, lista para `Player::set_character_sheet(&ficha)` y con
  `inventoryEquipment`/`gold` para G-12/H2-08.
- **`tests/Game/NewGameFlowTests.cpp`** (7 casos, assets reales 61×43×12;
  la clase/elegidos se seleccionan por catálogo para no depender del
  orden del JSON): cancelación en Raza sin ficha, `finalizar` sin
  terminar = Error, flujo completo con `healthCap()>0` + defensas ≠ 0 +
  stats {6,4,3,3} + equipo inicial + oro, reparto izq/derecha con bolsa
  de 12 (sin gastar no avanza y avisa), ESC intermedio retrocede,
  no-op tras Terminado, determinismo entre dos flujos.
- **`cmake/H104NewGameFlowTests.cmake`** + 2 líneas de cruce en
  `CMakeLists.txt` (fuente en `motor_level`, test en suite gl-free).
  Ojo: `cmake/NewGameFlowTests.cmake` YA existía (ed84b18, 24/09) y
  cablea el test trivial viejo `tests/CharacterCreationFlowTests.cpp`
  (54 líneas, reglas vacías, no prueba nada) bajo el target
  `test_new_game_flow` — por eso el mío lleva prefijo de tarea. Si algún
  día se retire ese test viejo, el nombre queda libre.
- `MAPA_DEL_CODIGO.md`: las 3 rutas nuevas en la sección 3.

## Verificación (clon limpio de `7b1deb3`)

- `test_h104_new_game_flow` **7/7 PASS**.
- Sin regresión: `test_character_creation_flow`, `test_character_creation`,
  `test_new_game_flow`, `test_options_state`, `test_game_session`,
  `test_shop_flow`, `test_g13_combat_commands` → **ctest 8/8**.
- El árbol commitado es byte a byte el probado (diff de contenido de los
  6 ficheros contra el clon: iguales).
- No ejecuté `juego`/motor_core: el rojo de HEAD por `tickRound` no
  versionado (Pendiente 1 del TABLERO, dueño opencode) sigue vivo y mi
  rodaja no lo toca (`NewGameFlow.cpp` solo entra en `motor_level`).

## Commit

- MotorGraphico `61d89d7` (rama `feature/editor-s1-contratos`, padre
  `7b1deb3`): +430 líneas, solo mis 6 rutas, sin borrados. Ficheros de
  cruce (`CMakeLists.txt`, `MAPA_DEL_CODIGO.md`) subidos como **blob
  construido** (HEAD + solo mis líneas); índice real realineado con
  `update-index --cacheinfo` para no dejar cambios fantasma. Guard con
  `update-ref <nuevo> <esperado>`: mi primer intento apuntó a `master`
  por error y la guarda lo paró — **master está 6 semanas por detrás,
  la rama viva es `feature/editor-s1-contratos`** (ya avisado el 19/08;
  sigue sin resolverse).
- Coedición vista: mientras trabajaba, otro agente commitó `7b1deb3`
  (H3-04) y manipuló el índice (staged de `Player.cpp` que luego
  retiró). Sin roces: mis ediciones del árbol vivo viven junto a su WIP
  en regiones distintas.

## Qué queda de H1-04 (para el dueño del cruce de la zona H)

El montaje en `Application`: modo de juego «Nueva partida» tras el título
→ crear `RPG::CharacterCreation` (carga `assets/catalogs`), montar
`Game::NewGameFlow`, pintar `pantalla()` (pasos/opciones/aviso/«N de 14»),
traducir InputAction a la API del flow y, en `Terminado`,
`finalizar(tierRules)` + `Player::set_character_sheet(&ficha)` (y
«Cancelar» vuelve al título). Ruta de cierre de la ficha H1-04: Título →
Nueva partida → clase/raza/trasfondo → la ficha sale con stats y equipo.
