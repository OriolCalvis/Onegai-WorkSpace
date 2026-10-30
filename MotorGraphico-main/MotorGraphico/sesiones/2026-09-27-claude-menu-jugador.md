# 27/09/2026 — Claude: game feel (relevo de CONTROLES-02) y el menú del jugador

Encargo de Oriol: seguir con el game feel, y después que los menús estén bien
ubicados y se pueda comprar; inventario, equipo, estadísticas, pasivas,
condiciones con iconos en pantalla y en la ficha; misiones, mapa, lore,
personajes y datos curiosos. «D&D 5e súper simplificado» se entiende como
referencia de UX: el reglamento sigue siendo el canon de 4 stats
(CON/DES/INT/CAR), tiers por hitos y Nd6. No se cambia ninguna regla.

Todo está **en el árbol, sin commit** (git-safe.sh vive fuera de la carpeta
conectada). Cada paquete tiene su JSON en `planificacion/seguimiento/`:
CONTROLES-02 (actualizado), FICHA-01, MISIONES-01, CODICE-01, TIENDA-01.

## Qué se ve ahora

- **Muro**: al rechazar la sesión un paso, el sprite mira al obstáculo y
  rebota 80 ms (`Player::bump`). El resto de CONTROLES-02 (rumbo en reposo,
  fase continua, profundidad interpolada) era de Codex y se conserva.
- **TAB → menú del jugador** con seis pestañas (A/D): Inventario (icono
  consumible/equipo), Equipo (icono por ranura, nombre de catálogo), **Estado**
  (la ficha: stats base/raza/clase/equipo/dotes/condiciones = total, PV/PM/oro/
  inspiración, tier, ranuras, pasivas/dotes/rasgos, condiciones con rondas y
  acumulaciones), Acciones, **Misiones** (vista de las flags: activa primero,
  objetivos con casilla) y **Códice** (personajes hablados, lugares entrados y
  actual, deidades —la propia primero—, lore de clase y aventuras, razas
  conocidas). W/S recorren; el pie muestra el detalle de la fila.
- **Tienda** centrada (560×340) sobre el cuadro de diálogo, oro en el título,
  icono comprar/vender, fila en rojo si no llega el oro, detalle con efecto y
  precio, «No hay artículos disponibles». El cartel de negocio comparte panel.
- **Iconos**: `assets/textures/iconos.png` (56 de 16 px) + `iconos.json`,
  generados por `tools/gen_iconos.py`; las 16 condiciones tienen icono propio.

## Cómo está partido (ley del repo)

GL-free en `motor_level`: `Game::FichaVista`, `Hud::Iconos`,
`Game::construirDiario`, `Game::Codice`. Con textura en `motor_core`:
`HudIconos` (atlas), `HudFicha`, `HudIconList`, y las filas de cada pestaña en
`src/Engine/ApplicationMenuJugador.cpp` (funciones libres; `Application.h` no
gana miembros: sólo cambian de tipo `m_inventoryStatusWidget`,
`m_inventoryListWidget` y `m_shopMenu`). `Application.cpp` queda en 1393
líneas (misma cuenta que al empezar): una línea para el rebote y los
transforms de tienda/negocio. `ApplicationHud.cpp` 364.

## Verificación

Oráculo en el contenedor (HEAD + estos ficheros; motor_core no enlaza en HEAD
por `publishSheetSkills`/`BattleState::tickRound`, ajenos): test_render_player
55/55, test_ficha_vista 5/5, test_diario_misiones 3/3, test_codice 3/3;
ApplicationHud/Pantallas/MenuJugador/HudFicha/HudIconList/HudIconos y el
`Application.cpp` del árbol compilan. `linea_base_core --rapido` sólo avisa de
que los ficheros nuevos no están versionados (van en el mismo commit que sus
líneas de CMake). **Falta el humo visual**: `./juego`, TAB, A/D por las seis
pestañas; hablar con un tendero, comprar y vender.

## Avisos

- Otro agente estaba reescribiendo `GameSession.h` (EST-23b) y
  `Application.cpp` durante la sesión (el árbol cambió entre lecturas; a las
  13:56 `GameSession.h` fue 21 bytes durante un rato). No toqué GameSession.
- El trabajo de Codex en CONTROLES-02 y G12-INVENTARIO-VISTA está en
  `stash@{0}` (un stash ajeno de las 13:56). Recuperé sólo los seis ficheros
  de CONTROLES-02; `InventoryScreen.cpp` chocará al hacer `stash pop`:
  conservar `ficha()`, `inventoryIconIds()` y `statusLabels()` de aquí.
- `_to_delete/tmp_claude_2026-09-27/` tiene seis `.tmp` que el sandbox no
  dejó borrar. `out/oraculo_claude/` son tars del oráculo (ignorado).
- Pendientes de producto: precio de recompra visible (GameSession no expone
  `m_shopBuybackPercent`), niveles visitados persistentes para el códice,
  modo detalle con W/S en la ficha (hover de 17.1).

---

## Segunda tanda (misma sesión): editor con ayuda e iconos, y creación de personaje

Encargo: «termina de añadir elementos HUD, tooltips, referencias visuales a
todas las opciones del editor del juego; luego la creación del personaje al
crear una partida nueva». Paquetes: **EDITOR-UX-01** y **NUEVA-PARTIDA-01**
(JSON en `planificacion/seguimiento/`).

- **motor_editor**: `Editor::EditorAyuda` (GL-free, en motor_editor_document)
  da texto e icono a las 14 acciones y texto a 22 controles de los paneles;
  `tools/gen_iconos_editor.py` genera `assets/textures/iconos_editor.{png,json}`
  (28 iconos). `EditorIconos` los monta para ImGui; `ToolsPanel` e
  `InspectorPanel` salen de `EditorApplication.cpp` (597 → 500 líneas) con
  botones de icono, tooltips, pie con el uso del ratón y ventana de ayuda con
  leyenda de las marcas del mapa; `LayerPanel` y los menús con tooltip. El
  harness de humo lee ahora los puntos de Rellenar/buscar objeto/fila poción de
  `ToolsPanel::testPoint`. `test_editor_ayuda` 4/4. `level_editor` (UI propia
  sin ratón sobre widgets) no se toca.
- **Nueva partida**: `Engine::PantallaNuevaPartida` envuelve `Game::NewGameFlow`
  (H1-04) con los widgets del HUD: 14 pasos con lista de iconos, detalle de
  la opción, aviso y pista. Al terminar, la ficha se vincula al Player
  (`set_character_sheet`), PV/PM al máximo, y arranca la escena. Hasta hoy el
  jugador **no tenía ficha**: `hasCharacterSheet()` era siempre falso.
  `Application.h` +1 miembro justificado; `Application.cpp` sigue en 1393.
- Pendiente: humo en el Mac (`ctest -R motor_editor_smoke`; `./juego` →
  Nueva partida); nombre por lista A/D hasta que haya entrada de texto;
  SaveGame no guarda la ficha; `InventoryRules.playerClass` sin rellenar.

---

## Tercera tanda: seis clases de la 3.ª iteración y compañeros (CLASES-03)

Oriol pasó el texto corregido de seis clases (Monje del Despertar, Señor de
las Armas, Bardo de las Cuatro Estaciones, Apóstata, Furia de los Creadores,
Mago Chasqueador). `tools/importar_clases_companeros.py` es la fuente en el
repo: hace upsert en classes/skills/passives/summons/npcs.json respetando la
sangría; los `[pendiente]` se conservan tal cual. Cada clase: pasiva, 7
habilidades, 3 de Creador; el Apóstata con sus dos invocaciones; un NPC
compañero reclutable por clase (`companion.recruitable`, flag `joinFlag`).
`Game::Companeros` recluta por flag y el Códice los lista como
`[Companero]`. `validar_catalogos_rpg.py` OK; `test_clases_companeros` 3/3.
Pendiente: meter a los compañeros en combate (BattleState solo lleva al
jugador) y un comando de evento que los recluta.

---

## Cuarta tanda: el árbol no compilaba (`make` de Oriol, 18:50)

Causas, ninguna de los paquetes de hoy: (1) EST-23b a medias en
`GameSession.h`/`GameSession.cpp`/`SaveGame.cpp` (SessionData sin terminar,
`basePriceOf`/`resolveShopOffers` sin declarar) más H2-05 `repartirBotin` en
`GameSessionInteraccion.cpp`; (2) `Application.cpp` de HEAD llama a
`publishSheetSkills` y `BattleState::tickRound`, que solo existían en
`stash@{0}` (nadie los versionó desde d7bc4e1); (3) el WIP ajeno de
`Application.cpp/h` (H1-05 Locale) y su prueba de cableado dependían de eso.

Qué se hizo: copias de todo el WIP ajeno en `out/wip_ajeno_2026-09-27/`
(con LEEME y parches árbol/índice), HEAD restaurado en esos ficheros y los
cambios de Claude reaplicados sobre HEAD (`Application.cpp` 1259 líneas =
HEAD; `Application.h` +1 miembro). Del stash se INCORPORAN al árbol
`publishSheetSkills` y `tickRound` con sus pruebas (BattleMenuActions,
BattleState, BattleStateTests 17/17, G13CombatCommandsTests 8/8). En el
oráculo compilan motor_core, motor_level, motor_editor (objetos) y toda la
suite GL-free salvo `test_adventure_validation`, que ya en HEAD enlaza dos
veces `RPG::ValidateAdventure` (AdventureValidation.cpp y
AdventureBeatValidation.cpp, EST-33): ajeno, sin tocar.
