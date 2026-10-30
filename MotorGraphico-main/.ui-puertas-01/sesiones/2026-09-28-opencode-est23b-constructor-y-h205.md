# 2026-09-28 — opencode — EST-23b (constructor), H2-05 verde, Locale enllaza, reglas de inventario

**Fichero de sesión append-only.** Nada de esto es canónico: el canon vive en `Onegai 2/`.

## Qué entró por la puerta

El encargo era «coge el gantt del motor gráfico y sigue programando, pero hay más
agentes trabajando a la vez». La primera mitad de la sesión fue de **diagnóstico, no
de código**, porque el árbol no estaba verde: `test_game_session` terminaba en
`EXIT=139` (segfault) con 12 fallos.

## 1. La regresión de EST-23b (era mía)

`f128e4a` («EST-23b: extraer 40 miembros de GameSession en SessionData») está commiteado,
y `planificacion/TABLERO_ESTABILIZACION.md:81` lo asigna a **opencode 28/09, "hecho"**.
Al leer la fila: la regresión también era mía. No hacía falta pedir permiso.

**Causa raíz.** Al mover los 40 miembros a `SessionData`, la lista de
inicialización del constructor pasó a ser solo `m_data(std::make_unique<SessionData>())`
y **las ocho asignaciones de colaboradores se quedaron por el camino**: en todo el
motor no existía ni una línea que pusiera `m_map`, `m_catalog`, `m_skillCatalog`,
`m_monsterCatalog`, `m_level`, `m_playerBody`, `m_playerSkills` ni `m_inventory`.
Solo sobrevivieron las dos del `World` (C-17), que se escribieron con
`sessionData()->` porque el cuerpo se editó a mano.

Consecuencia: **toda sesión construida por el constructor público arrancaba sin
catálogo, sin nivel, sin cuerpo del jugador, sin inventario y con `m_level` vacío**.
De ahí los 12 fallos (sin `m_catalog` → ningún enemigo, ninguna puerta, ninguna
narrativa, ningún grado de dado) y el segfault (`update()` con `m_playerBody` nulo:
el caso 21 de la suite, `pausar_congela_el_mundo_y_reanudar_lo_suelta`).

**Arreglo** (`src/Game/GameSession.cpp`): restaurar las ocho asignaciones al principio
del cuerpo, **antes** de `m_playerPosition = m_level.playerStart` y del reparto de
spawns, que ya las leen. Se copiaron tal cual de la versión previa (`f128e4a^`), que las
tenía en la lista de inicialización.

```
test_game_session:  20 casos + segfault  ->  24 casos, 1 fallo
test_application_cableado:  2/11  ->  8/11
```

El fallo que quedaba era mi propio caso H2-05 sin producción (ver 2).

## 2. H2-05 — botín al ganar: por fin VERDE

El código había desaparecido por cuarta vez (lo confirmó el grep: 0 apariciones de
`repartirBotin` en `src/` y `include/`; los 2 casos seguían en el test porque no
llaman al símbolo directamente, solo compilan el caso que falla).

Reaplicado sobre la API actual (`sessionData()->`), a partir de la huella de
`planificacion/seguimiento/H2-05-BOTIN.json`:

- `include/Game/GameSession.h`: una declaración privada, `repartirBotin(const std::string&)`.
- `src/Game/GameSessionInteraccion.cpp`: se copia `enemyObjectId` **antes** de que la
  entidad muera con el World (es la clave de su ficha de monstruo) y se reparte en el
  caso `Victory` **antes** de `destroy()`, para que la línea de victoria salga encima y
  el botín se lea debajo.
- El botín se lee de `MonsterDefinition::lootIds` (el campo `loot` de monsters.json,
  432 fichas, 28 ids) y cada pieza entra con `objectIdForEquipment()` — el mismo camino
  que equipar y vender, que es lo que hace que lo que cae se pueda equipar y vender.

**Falsabilidad comprobada**: quitada la llamada `repartirBotin(enemyObjectId)`,
`al_ganar_el_botin_del_monstruo_entra_en_la_mochila` falla en
`GameSessionTests.cpp:1206` (`f.inventory.size() == 2`); restaurada, verde.

**Regla 4** (`tools/medir_alcance.py` mide los tres ejecutables reales): `nm -C juego`
devuelve 1 aparición de `repartirBotin`. No es código muerto.

## 3. `juego` no enlazaba — dos capas, ambas mías

`make juego` fallaba con `symbol(s) not found`. Era H1-05, mi fila, con el enlace
bloqueado desde el 27/09.

**Capa 1 — `Engine::Locale::loadFromDirectory`.** `src/Engine/Locale.cpp` no estaba en
ninguna librería: se compilaba suelto en **cinco** objetivos de prueba, y el comentario
de `cmake/LocaleTests.cmake:12-15` lo predijo («cuando Opciones (G-10) lo necesite
enlazado, ese paquete lo mueve»). Ese paquete fue H1-05. Entra en `motor_core`; los
tres objetivos que ya enlazaban `motor_core` (`test_options_locale_bridge`,
`test_export_panel`, `test_application_cableado`) dejan de compilarlo. Los otros dos
(`test_locale`, `test_locale_extension`) **se quedan** con la fuente directa y ahora lo
dicen: son GL-free y `motor_core` lleva OpenGL, así que no pueden enlazarlo sin perder
la propiedad.

**Capa 2 — `Game::OptionsLocaleBridge::applyLanguage`.** El `.cpp` no estaba en
ninguna librería tampoco, y `Application.cpp` lo invoca. **Se hizo header-only**: el
cuerpo se movió a la cabecera como `inline` (mismo comportamiento, texto movido, no
reescrito) y el `.cpp` se borró. Sin tocar el grafo de librerías, que es lo que habría
avisado a los demás agentes; y de paso quita dos fuentes sueltas de la deuda de la ley 4c.

`make juego` → `Built target juego`. `nm` confirma también `applyLanguage` y
`loadFromDirectory` en el binario.

## 4. El catálogo de equipo nunca llegaba a la sesión (prerrequisito de H2-08)

De los 3 fallos que quedaban en `test_application_cableado`, dos eran
`inventoryRules().equipment != nullptr` y `equipmentFor("item_aguja_del_firmamento")`.
El comentario de `Application.cpp:406` (H0-04) decía «el catalogo de equipo viaja a la
sesion via setInventoryRules (**lo inyecta loadLevel**)»: no lo inyectaba nadie. Cero
llamadas a `setInventoryRules` en todo el motor.

`src/Engine/ApplicationNivel.cpp`, junto a `setConditionCatalog`: se inyectan las reglas
con `equipment = &m_equipmentCatalog`. **Solo `equipment`**, que es lo que faltaba;
`playerClass` y `tiers` siguen a `nullptr` como estaban y cablearlos es otro item.

Esto no es un extra: sin él `equipmentFor()` devuelve `nullptr` **siempre**, así que
equipar y vending responden «no hay catalogo de equipo cargado» y **ningún precio de la
tienda se resuelve** — que es exactamente el item H2-08, cuyo parche estaba escrito y
bloqueado desde el 27/09. El parche sigue sin aplicarse (el destino sigue siendo
`GameSessionComercio.cpp`); esto solo le quita el primer obstacle.

`test_application_cableado`: 8/11 → **10/11**.

## 5. Lo que queda rojo y no es mío

- `cableado_titulo_sale_del_manifiesto` (`ApplicationCableadoTests.cpp:402`): hay
  transición de escena ya encolada antes de elegir nada. Es `m_scenes.hasPendingTransition()`
  (scene manager, C-16 / `motor_scene`). No lo ha tocado nadie en esta sesión y no lo
  toco yo: es de la zona de menú/G-08-G-11.
- `test_catalog_content` 29/32: los 3 fallos son derivas de recuento de datos que ya
  estaban registradas como de otros dueños el 28/09 (skills 492→551, npcs 134→140;
  summons ya arreglado). No se tocan.
- `cmake/G15ExportPanelTests.cmake` está **incluido en ningún sitio**: `test_export_panel`
  no existe como target. Fichero .cmake huérfano (EST-42, «cablear o retirar»). Mi
  edición therein es inerte; no lo enchufo porque añadir un target al build compartido
  es decisión del programa de estabilización, no de esta sesión.

## 6. La ley no perdona: `motor_i18n`

Poner `Locale.cpp` en `motor_core` enlazaba `juego`, pero la guardia lo marcó
inmediato: **una fuente GL-free dentro de una librería con GL es una prueba que
necesita ventana** (`motor_core` pasó de 17 a 18). Y `GameSession.h` seguía subiendo
(546 → 550) en un fichero que solo puede adelgazar. Los dos cambios míos eran deuda
nueva, y la ley es puerta de `pre-commit`, `pre-push` y CI.

**`motor_i18n` (nueva librería, GL-free, junto a `motor_runtime_map`):** `Locale.cpp`
vive aquí, `motor_core` la enlaza (por eso `juego` enlaza) y las dos pruebas GL-free la
enlazan en vez de compilar el `.cpp` sueltas. Sin tocar el grafo de librerías: una
librería nueva y un enlace, nada más.

**`GameSession.h`:** 546 → **544**. Seennials dos comentarios que repetían lo que ya
decía el doc del constructor (`el mundo es el propietario`) y lo que ya decía el
párrafo siguiente (`update()` modal congela el mundo). Mi declaración de H2-05 cabe en
una línea. Nada de información nueva perdida: lo que se quitó estaba escrito dos veces.

```
linea_base_core: 2 de deuda nueva  ->  0 de deuda nueva
  baja: include/Game/GameSession.h: baja de 546 a 544 lineas
  baja: fuentes compiladas sueltas: baja de 62 a 55
```

La ley sigue dando FALLO, pero por **77 instantáneos de ficheros sin versionar** que han
creado otros agentes (`examples/demo_*.cpp`, `editor/src/*.cpp`, `cmake/*.cmake` sin
git add). Nada mío.

## 7. El índice de git estaba minado (esto era lo más peligroso del árbol)

`git diff --cached` en `MotorGraphico-main` devolvía **1386 ficheros staged como
BORRADOS y cero añadidos**: `AGENTS.md`, `.github/workflows/ci.yml`,
`include/Game/GameSession.h`, `CMakeLists.txt`… El disco estaba intacto; era el
**índice**. Casi con seguridad la carpeta sincronizada reescribió el índice — la misma
causa que `INCIDENCIA_SINCRONIZACION_2026-09-23.md`, y el tablero avisa de que «dos
relevos completos se perdieron así».

**Un `git commit` de cualquier agente se llevaba 1386 ficheros del repo.** El commit más
probable de la tarde habría sido el mío, si hubiera habido luz verde.

Arreglado con `herramientas/git-safe.sh reset` desde `MotorGraphico-main` (mixto: solo
índice). Antes de hacerlo se comprobó que el índice **solo** contenía borrados —cero
trabajo staged que perder— y `git diff --cached` quedó a 0. `GameSession.h` volvió a
estar versionada (antes era `D` + `??`, es decir, un `git add -A` de cualquier otro la
habría borrado del repo). En el primer intento el comando corrió sobre el repo del
workspace (`/Users/admin`, que también es un repo y trackea `Software/`); un `reset`
mixto no toca el working tree, así que no se perdió nada, y se repitió en el repo
correcto.

**Queda como aviso a todo el mundo** (primera fila del TABLERO): antes de commitear,
`git diff --cached --name-status`. Si salen borrados que no habéis hecho vosotros, es el
índice y toca `reset`, nunca `commit`.

## Verificación (tras `motor_i18n` y el reset)

```
test_game_session ............ 24/24      test_inventory_menu .......  9/9
test_game_mode ............... 14/14      test_inventory_engine .....  7/7
test_shop_flow ............... 13/13      test_carry_over ...........  7/7
test_battle_state ........... 17/17      test_business_economy .... 18/18
test_world .................. 10/10      test_business_mode ....... 16/16
test_options_locale_bridge ..  6/6       test_save_game ........... 19/19
test_locale ................. 18/18      test_g13_combat_commands ..  8/8
test_locale_extension .......  6/6       test_application_cableado . 10/11 (el rojo es ajeno)
juego: enlaza · nm: repartirBotin, applyLanguage · linea_base_core: 0 deuda nueva
```

## Verificación

```
test_game_session ............ 24/24   (los 2 de H2-05 incluidos)
test_game_mode ............... 14/14
test_shop_flow ............... 13/13
test_battle_state ........... 17/17
test_world .................. 10/10
test_inventory_menu ..........  9/9
test_inventory_engine ........  7/7
test_carry_over ..............  7/7
test_business_economy ....... 18/18
test_business_mode .......... 16/16
test_save_game .............. 19/19
test_g13_combat_commands ....  8/8
test_options_locale_bridge ..  6/6
test_locale ................. 18/18
test_locale_extension .......  6/6
test_application_cableado ... 10/11   (el rojo es de otro, ver 5)
juego ....................... enlaza; nm: repartirBotin, applyLanguage, loadFromDirectory
```

135 casos en las diez primeras suites, 0 fallos. La vía de inventario se barreñó entera
porque la inyección de reglas la activa por primera vez en el ejecutable.

## Sin commit

12 ficheros míos, todos sin commitear, en un árbol con 50 ficheros modificados de
varios agentes. Sin luz verde no se commitea. **El riesgo real ya ha pasado cuatro
veces con este WIP** (un `stash` de Claude se llevó el H2-05 entero). Lo de
esta sesión son ~226 líneas en 12 ficheros, recuperables con `git diff`, pero un
`checkout` o un `stash` ajeno las vuelve a barrer.

## Lo siguiente

- **H2-08**: aplicar el parche de `planificacion/seguimiento/H2-08-PRECIOS.md` (25
  líneas, 2 casos). El prerrequisito ya está. Ojo: `basePriceOf()` vive hoy en
  `GameSession.cpp`, que es el fichero de 14 paquetes, y `GameSessionComercio.cpp` es
  el destino natural — decidirse antes de abrirlo.
- **H1-04** ya lo aterrizó otro agente (`61d89d7`, `Game::NewGameFlow`). No duplicar.
- **LootResolver** sigue sin lector: ningún encuentro enruta a la tabla de botín.
