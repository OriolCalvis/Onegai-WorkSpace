# 2026-10-01 · zcode · H3-02 (rodaja GL-free): el diario de misiones — contrato `QuestDefinition`, cargador y `Game::QuestLogState`

Sesión de continuación del Gantt de Fase H (`planificacion/gantt_jugabilidad_H.puml`),
siguiendo el MAPA_DEL_CODIGO para las zonas de cruce. Commit MotorGraphico
**`f516f6b`** (9 ficheros, **+907 −0**: todo aditivo; `CMakeLists.txt` entró como
blob HEAD + 8 líneas propias, sin arrastrar el WIP ajeno del árbol).

- **Commits:** MotorGraphico `f516f6b`. Repo raíz: esta sesión + la fila del
  TABLERO + `sesiones/INDICE.md` regenerado.
- **Deja esperando a Oriol:** una discrepancia de diseño REGISTRADA, no resuelta
  (§6): el diario de misiones tiene ahora DOS mitades en vuelo de sesiones
  distintas — la de esta sesión (contrato+estado explícito) y `MISIONES-01` de
  opencode (vista de las flags narrativas). Decidir cuál es el diario — o cómo
  se componen — es decisión de producto, no mía.

| Ruta | Qué |
| --- | --- |
| `include/RPG/Definitions/DefinicionesMisiones.h` | NUEVO — `QuestDefinition` + `QuestObjective` (structs puros, con `tier`/`rarity` normalizados P0) |
| `include/RPG/Catalogs/CatalogLoadersMisiones.h` | NUEVO — `CatalogLoader<QuestDefinition>` (D-01) + `DanglingQuestReferences()` |
| `include/Game/QuestLogState.h` + `src/Game/QuestLogState.cpp` | NUEVOS — el diario GL-free (patrón `OptionsState`/`WorldTravelState`), 1 fuente en `motor_level` |
| `assets/catalogs/quests.json` | NUEVO — semilla de 2 misiones DERIVADAS del contenido publicado |
| `tests/Game/QuestLogTests.cpp` + `cmake/QuestLogTests.cmake` | NUEVOS — 10 casos GL-free, 2 sobre assets publicados |
| `CMakeLists.txt` | +8 (fuente en `motor_level` + `include` del test), por blob |
| `planificacion/seguimiento/H3-02-MISIONES.json` | NUEVO — la ficha |

---

## 1. Por qué esta tarea y no otra

Elegida contra el Gantt y contra el estado real del árbol, con las dependencias
al día: H3-02 (diario de misiones, 3 j) depende de H0-08, cuya mitad GL-free
(`MapEventRunner`, `bdcb3ff`) está versionada. Estaban libres y sin fila viva:
H3-02, H3-06, H3-08 y la fase H4/H5 de autoría; H2-08 la tenía opencode en
curso, H2-03 la reclamó opencode el 29/09 por la noche, y la mitad GL-free de
H3-06 (temporizadores) roza el WIP HUD ajeno (`Iconos`, `HudFicha`).

H3-02 era además **el hueco registrado con nombre propio**:
`AUDITORIA_CONTRATOS.md` lista `QuestDefinition` como contrato inexistente —
`AdventureDefinition.questIds` y `EventDefinition::Option::grantQuestId`
apuntando al vacío, «las misiones son beats+flags sin ficha propia».

Medido antes de escribir:

- `grep questIds` en `include/`+`src/`: solo el cargador de adventures lo lee.
  En `assets/`: **0** usos. `QuestDefinition`/`QuestLog`: 0 resultados.
- `tools/validar_todo.py:21` lo confiesa en un comentario: *«no hay
  QuestDefinition todavía: bitácora/roadmap S2+»*.
- Los 78 adventures publicados llevan `questIds: []` — referencias vacías,
  o sea: nadie podía siquiera colgar una referencia rota.

## 2. Qué se entregó

**El contrato** (`DefinicionesMisiones.h`): `QuestObjective`
(id/description/optional/requiredCount — 1 = sí/no, >1 = contador) y
`QuestDefinition` (objectives, recompensas DECLARATIVAS, `completionFlag`).
Fichero propio y no una entrada más de `DefinicionesContenido.h` porque esa
cabecera la cruzan las familias de contenido con sesiones en vuelo.

**El cargador + el repaso** (`CatalogLoadersMisiones.h`):
`CatalogLoader<QuestDefinition>` con su contrato de campos D-01, y
`DanglingQuestReferences(adventures, events, quests)` — el informe que dice
«adventure X referencia la mision Y y no esta en el catalogo». Hasta hoy un
`questIds` roto no podía reportarlo NADIE.

**El diario** (`Game/QuestLogState`): `start()` copia la ficha (el diario se
lee sin catálogo); `advance()` cuenta con corte en lo requerido e idempotente;
`complete()` EXIGE los obligatorios y el error dice cuáles faltan con su
progreso; `fail()` no esconde la entrada; `displayLines()` con el contrato de
HUD de la casa. No aplica recompensas ni enciende banderas — eso lo hace quien
orquesta (montaje), misma división que `LevelTransition`/`WorldTravelState`.

**La semilla** (`assets/catalogs/quests.json`): 2 misiones DERIVADAS del
contenido publicado, sin inventar lore: `mision_llave` cierra con la bandera
`mision_completada` — exactamente la que el altar del nivel publicado enciende
y a la que reacciona el veterano de `conversacion_ramificada` (H3-03);
`carta_del_santuario` usa los ids reales del asset (`charlas`,
`carta_sellada`). Recompensas a 0: el contenido publicado no tiene ninguna, y
declarar las que no existen sería inventar.

## 3. Verificación

- `test_quest_log` **10/10** en clon limpio de HEAD + estos ficheros, y de
  nuevo en clon fresco del commit final (`f516f6b`).
- **Falsabilidad, 4 mutaciones cazadas** (restaurado y reverificado 10/10
  tras cada una): `complete()` sin exigir obligatorios → cae
  `completar_exige_los_obligatorios_y_dice_cuales_faltan`;
  `advance()` sin corte en lo requerido → cae
  `avanzar_cuenta_se_corta_en_lo_requerido_y_es_idempotente`;
  `DanglingQuestReferences` callando → cae
  `las_referencias_de_aventuras_y_eventos_resuelven`; `displayLines`
  escondiendo fallidas → caen `fallar_no_esconde_la_entrada…` y el orden.
- Suite GL-free completa + `juego` compilan en clon limpio; ver §5 para el
  recuento exacto y la atribución de rojos.
- **Hallazgo corregido en caliente:** `validar_catalogos_rpg.py` exige
  `tier`+`rarity` en TODA entrada de `assets/catalogs/` (P0). La primera
  versión de `quests.json` rompía `test_validar_todo` y
  `test_aventuras_monsterid` — medido en clon, atribuido a mí, corregido:
  campos añadidos al JSON, a la ficha y al cargador (que los LEE, no solo
  los tolera), y el test comprueba ambos.
- `linea_base_core_configura` pasó de rojo (mis ficheros sin versionar en el
  clon, tal como prescribe el par .cmake+include) a verde con el commit
  conjunto — el guarda funcionó como debe.

## 4. Coedición y decoro

- HEAD se movió DOS veces durante la sesión (opencode: `8b338a4`/`0476bb6`
  `tickRound`, luego `40a28cd` H2-05 `repartirBotin`). Los anclajes del blob
  de `CMakeLists.txt` se re-verificaron contra el HEAD nuevo antes de
  commitear; el WIP ajeno del árbol no entra en mi commit.
- Al sincronizar el índice real tras el `commit-tree` (trampa del «cambio
  preparado fantasma»), apareció staged ajeno de otra sesión (docs
  `AUDITORIA_*`/`ESTUDIO_*`): no se ha tocado. Mis 8 ficheros quedaron
  apuntados a sus blobs de HEAD.
- Basura detectada sin tocar: `.git/index 31.lock` (familia de los duplicados
  « 2», no bloquea — git solo mira `index.lock`).

## 5. Atribución de rojos (medida en clones, no a ojo)

Contra clon prístino de `0476bb6` (el HEAD de mi primera verificación) se
midió qué fallaba SIN mis ficheros: `catalog_loader_contract` ×2 (el parser
no entiende la partición EST-22 de `RpgCatalogs.h` — dueño D-01/AUD),
`linea_base_core` (deuda contabilizada), `test_game_session` (el rojo de
H2-05, **curado después por `40a28cd`**). En el clon final del commit `f516f6b`
el recuento y la atribución van en la fila del TABLERO.

## 6. Discrepancia registrada (para Oriol, no resuelta en silencio)

**El diario de misiones tiene DOS mitades en vuelo de sesiones distintas:**

1. **Esta sesión (H3-02):** contrato `QuestDefinition` + catálogo
   `quests.json` + `QuestLogState` con progreso explícito (contadores,
   obligatorios, fallidas). El autor DECLARA la misión y su avance.
2. **`MISIONES-01` (opencode, en el árbol sin fila de TABLERO):**
   `include/Game/DiarioMisiones.h` + `tests/Game/DiarioMisionesTests.cpp`
   untracked, y `cmake/FichaVistaTests.cmake` YA versionado declarando
   `test_diario_misiones`: «el diario es una vista de las flags — la activa
   primero, objetivos resueltos contra el NarrativeState». El autor no
   declara nada nuevo: el diario LEE lo que la narrativa ya sabe
   (`AdventureScript.objectives` con `doneFlag`).

No chocan en ficheros (cero solapamiento de rutas, nombres de test distintos)
pero sí en diseño: dos fuentes de verdad para «misión». La de esta sesión
cierra el hueco de la auditoría (ficha, catálogo, referencias que resuelven)
y soporta contadores y fallidas; la de `MISIONES-01` no añade estado que
guardar. Se pueden componer (vista para lo narrativo, ficha para lo
declarativo), pero decidirlo es de Oriol. Sus ficheros untracked no se han
tocado (regla 2). Nota dejada también en el TABLERO.

## 7. Qué falta para cerrar H3-02 de verdad (el montaje, con dueños)

1. Cargar `quests.json` en `Application::init` (o vía manifiesto) y exponer
   el diario — `Application.*` con filas vivas.
2. Tecla para abrir el diario + pintar `displayLines()` en `ApplicationHud`.
3. Familia `mision.iniciar`/`mision.avanzar`/`mision.completar` en
   `EventCommands`+`SessionCommandHost` para que el CONTENIDO mueva el
   diario sin código (tramo siguiente declarado en la fila).
4. Aplicar recompensas y encender `completionFlag` al completar.
5. Serializar el diario en `SaveGame`.
6. Registrar la familia en el paraguas `RpgCatalogs.h` cuando esté tranquilo.

Mismo bloqueo declarado por H0-08, H3-01 y H6-03: `Application.*` con filas
vivas. La ficha `H3-02-MISIONES.json` lleva el detalle con ficheros.
