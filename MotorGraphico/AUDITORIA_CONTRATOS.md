# Auditoría de contratos del editor RPG — S1

*Rama `feature/editor-s1-contratos` (base `aff3801`) · 17/08/2026 · línea base
verificada: `demo_proyectos`, `demo_editor_state`, `validar_enlaces.py`
(78 niveles, 138 puertas, 0 problemas) y `conectividad.py` (100% alcanzable)
en verde.*

Entregable S1 del `ROADMAP_RPG_EDITOR.md`: inventario de qué contratos de
datos existen, quién los lee y quién los escribe, qué falta respecto a la
hoja de ruta, y qué referencias cruzadas están vigiladas hoy y cuáles no.

Regla de lectura: esto describe lo que hay en el código **ahora**. Si el
código y este documento discrepan, manda el código y este documento es un
bug (regla 8.8 de la bitácora: las cifras no se fijan, se recalculan).

---

## 1. Contratos existentes

### 1.1 Nivel: `LevelDefinition` (`include/Level/LevelDefinition.h`)

| Tipo | Papel | Estado |
|---|---|---|
| `LevelDefinition` | el QUÉ del nivel: `name`, `mapPath` (ruta al TMX), `playerStart`, `enemies[]`, `objects[]` | vigente |
| `ObjectSpawn` | la INSTANCIA (contrato central del roadmap): `objectId` + `position` + patrulla + transición (`targetLevel`, `targetPosition`, `hasTargetPosition`) + variante local (`displayName`, `variant`, `scale`, `effectOverride`, `properties`) | vigente |
| `EnemySpawn` | formato antiguo de la Fase 6 (`type` + `skillIds` por spawn) | **legacy: `GameSession` lo ignora** (ver FORMATO_NIVELES.md §1) |

- Lee: `LevelLoader` (`src/Render/LevelLoader.cpp`, parseo todo-o-nada).
- Escribe: el editor vía `EditorState` (round-trip TMX+JSON probado por
  `demo_editor_state`).
- Decisión de diseño clave: **las puertas viven en el spawn, no en el
  catálogo** (`targetLevel` es de esta puerta concreta; dos posadas
  comparten definición con destinos distintos).
- Asset: `assets/levels/*.json` (97 ficheros).

### 1.2 Objeto: `ObjectDefinition` (`include/Level/ObjectCatalog.h`)

El TIPO reutilizable (contrato central del roadmap). Un solo struct con
datos por categoría siempre presentes (decisión documentada en el header:
miembros planos en vez de `std::variant`; se reevaluará si crecen las
categorías con datos).

| Campo | Significativo si |
|---|---|
| `id`, `name`, `category`, `spriteId`, `raceId`, `blocksMovement`, `interactable` | siempre |
| `combat` (`maxHealth`, `maxMana`, `skillIds`) | `category == Enemy` |
| `pickup` (`effect`, `power`, `price`) | `category == Pickup` |
| `dialogue` (`lines[]`) | `category == Npc` |
| `shop` (`items[]`, `buybackPercent`) | `category == Npc` |
| `business` (`price > 0` = cartel en venta, `baseIncome`, `businessName`) | `category == Prop` |

- `ObjectCategory`: **Prop, Enemy, Pickup, Npc** (4). El parser acepta
  `"prop" | "enemy" | "pickup" | "npc"` (`src/Level/ObjectCatalog.cpp:11-34`).
- `ObjectCatalog : ICatalog<ObjectDefinition>` con `ids()` e
  `ids(category)` para paletas de autoría. **`loadFromFile` hace merge**:
  cargar varios ficheros acumula; id repetido = el último gana.
- Lee: `Application::loadLevel` (vía los catálogos que se le pasen),
  `GameSession` (interacción/comercio), el editor.
- Assets: `assets/objects/*.json` (31 ficheros).

### 1.3 Narrativa: `AdventureScript` (`include/RPG/NarrativeEngine.h`)

El GUION jugable. Modelo completo en `FORMATO_AVENTURAS.md`:

- `NarrativeState`: flags (strings) = todo el estado narrativo;
  `beatsFired` es contabilidad interna aparte a propósito.
- `NarrativeBeat`: `trigger` (`Talk`/`Enter`/`Auto` + `target`) +
  `requires` (`allFlags`/`anyFlags`/`notFlags`) + `speaker`/`lines` +
  `effects` (`SetFlag`, `ClearFlag`, `GrantGold`, `Log`, `SkillCheck`,
  `StartBattle`) + `once`.
- `ObjectiveDefinition`: texto atado a una `doneFlag` (el diario es una
  vista de las flags, nunca una segunda fuente de verdad).
- Carga: `loadFromFile → Result<AdventureScript>`, todo-o-nada; tipo de
  trigger/efecto desconocido = error de carga.
- Efectos diferidos: `SkillCheckRequest` y `BattleRequest` los empaqueta
  el motor narrativo y los resuelve `GameSession` (no hay RNG ni catálogos
  en `NarrativeEngine`).
- Assets: `assets/adventures/*.json` (9 guiones).

**Ojo — dos "aventuras" distintas:** `RPG::AdventureScript` (el guion)
vs `RPG::AdventureDefinition` (la CARTA del catálogo, ver 1.5). El
encuentro entre ambas está pendiente a propósito (comment en
`NarrativeEngine.h`): el guion llevará el id de su carta cuando toque.

### 1.4 Proyecto: `Project` (`include/Editor/ProjectIndex.h`)

El pack de contenido. Manifiesto `assets/proyectos/<id>.json` +
`index.json` (se declara, no se deduce — el prefijo se rompió con
`ciudad_en_*`; ver FORMATO_PROYECTOS.md).

- `ProjectIndex::scan/check/create/remove` + `orphans()` (ficheros que no
  reclama ningún manifiesto). `check()` verifica existencia de cada
  fichero declarado. `playable()` = hay `entry` y hay niveles.
- `entrada` admite ruta o nombre suelto (resuelve aventuras → niveles);
  `entryKind` anota a qué resolvió.
- Lee: `demo_proyectos`, la pantalla de arranque del editor
  (`ProjectHub`), `tools/build_proyecto.py` (réplica Python del cierre).

### 1.5 RPG Nd6: 21 definiciones + `Catalog<T>` (`include/RPG/Definitions/`, `include/RPG/Catalogs/RpgCatalogs.h`)

`Catalog<T> : ICatalog<T>` (plantilla única, `RpgCatalogs.h:729`) con 21
aliases (`RpgCatalogs.h:822-842`):

| Definición | Alias | Entradas | Referencias cruzadas que emite |
|---|---|---:|---|
| `SkillDefinition` (SkillDefinition.h) | `SkillCatalog`, `SpellCatalog` (comparten struct) | 483+210 | `apply_condition_id`, `granted_summon_id` |
| `ClassDefinition` | `ClassCatalog` | 61 | `passiveId`, `startingSkillIds`, `learnableSkillIds`, `startingEquipmentIds`, `specializationIds` |
| `RaceDefinition` | `RaceCatalog` | 43 | `compatibleDeityIds`, `passiveTraitId`/`activeTraitId`/`racialTraitId` |
| `BackgroundDefinition` | `BackgroundCatalog` | 50 | `startingEquipmentIds`, `skillProficiencyIds`, `featureIds`, `factionId` |
| `PassiveDefinition` | `PassiveCatalog` | 66 | `required_tag` (skills) |
| `FeatDefinition` | `FeatCatalog` | 6 | `requiredClassId`, `grantedPassiveIds`, `grantedSkillIds` |
| `TraitDefinition` | `TraitCatalog` | 2 | `linkedSkillId` |
| `DeityDefinition` | `DeityCatalog` | 10 | `prayerSkillIds`, `grantedPassiveIds` |
| `ConditionDefinition` | `ConditionCatalog` | 15 | — |
| `EquipmentDefinition` | `EquipmentCatalog` | 413 | `linkedSkillId` |
| `ConsumableDefinition` | `ConsumableCatalog` | 31 | `grantConditionId`, `grantPassiveId` |
| `MonsterDefinition` | `MonsterCatalog` | 432 | `skillIds`, `passiveId`, `lootIds`, `conditionsInflictedIds` |
| `SummonDefinition` | `SummonCatalog` | 201 | `attackSkillIds`, `passiveIds` |
| `MountDefinition` | `MountCatalog` | 50 | `grantedPassiveIdsWhileMounted` |
| `TrapDefinition` | `TrapCatalog` | 51 | `applyConditionId`, `saveAttribute` |
| `LootTableDefinition` | `LootTableCatalog` | 201 | `entries.refId` → `equip_*`/`cons_*`/`__gold__`/otras tablas |
| `NpcDefinition` | `NpcCatalog` | 12 | `raceId`, `classId`, `factionId`, `shopLootId`, `dialogueId` ⚠️, `monsterId` |
| `AdventureDefinition` | `AdventureCatalog` | 78 | `questIds` ⚠️, `requiredNpcIds`, `mainFactionId`, story cards |
| `EventDefinition` | `EventCatalog` | 1 | `options.grantQuestId` ⚠️, `startCombatWithMonsterId`, `levelTransitionId` |
| `LocationDefinition` | `LocationCatalog` | 140 (19+95+26) | `controlledBy`, `npcIds`, `deityIds`, `raceIds`; **fechado 2000 b.f. en `epoch`** |

⚠️ = campo que referencia un tipo de contrato que **todavía no existe**
(ver sección 2).

- Assets: `assets/catalogs/*.json` (24 ficheros + `MANIFIESTO.json`).
- Los cargan los demos (`demo_catalogos_rpg`, `demo_mundo`,
  `demo_creacion_personaje`) y `CharacterCreation`; el runtime de combate
  (`GameSession`/`BattleState`) **todavía no** consume la mayoría (ver
  riesgos).

### 1.6 Ficha de personaje: `CharacterSheet` + `CharacterCreation`

- `CharacterSheet` (`include/RPG/CharacterSheet.h`): la fuente única de
  verdad del PJ/enemigo (identidad, tier, stats en 6 capas, equipo
  6+1 slots, inventarios, skills/hechizos/pasivas, condiciones, oro).
  `healthCap()`/`defenseValues()` leen de cachés que solo llena
  `recompute_derived(TierRules, ClassDefinition*)`.
- `CharacterCreation::construir(CreationChoice) → Result<CharacterSheet>`
  ya valida y rellena la ficha completa contra los catálogos
  (`races/classes/backgrounds/deities.json`) — **cualquier camino nuevo
  que cree fichas debe delegar aquí** (regla anti-duplicación,
  ARCHITECTURE.md §3).

### 1.7 Validación C++ del editor (EN CURSO POR OTRA SESIÓN)

`include/Editor/EditorValidation.h` + `examples/demo_editor_state.cpp`
están siendo ampliados en este momento por otra sesión de agente
(+91/+47 líneas sin commitear, 17/08). Esta auditoría no los toca ni los
duplica: la validación de la sección 3 es la capa **Python de contenido**
(referencias entre ficheros), complementaria a la validación C++ de
**estructura del nivel en edición**.

---

## 2. Contratos que faltan (contra el roadmap)

| Contrato del roadmap | Estado | Dónde se nota hoy |
|---|---|---|
| `QuestDefinition` | **no existe** | `AdventureDefinition.questIds` y `EventDefinition.options.grantQuestId` apuntan al vacío; las misiones son beats+flags sin ficha propia |
| `DialogueGraph` | **no existe** | `NpcDefinition.dialogueId` apunta al vacío; el diálogo real es `DialogueData.lines` (sin ramas) + beats de `AdventureScript` |
| `ScreenDefinition` | **no existe** | las pantallas (pausa, tienda, equipo…) son código de HUD, no datos |
| `ItemDefinition` unificado | **parcial** | hay tres representaciones según contexto: `PickupData` (mundo), `EquipmentDefinition` (equipo), `ConsumableDefinition` (consumo). El roadmap pide separar pickup del equipable; hoy el mismo concepto vive en dos catálogos sin enlace |
| `ObjectSpawn` con eventos/condiciones locales | **no existe** | el spawn no tiene `conditions`, `lootOverride` ni eventos; el roadmap lo pide explícitamente |
| `ILoadable` | pendiente desde ARCHITECTURE.md §2 | cada cargador repite `loadFromFile/loadFromString → Result` sin interfaz común |

Regla para S2 en adelante: **ninguno de estos contratos se añade sin su
validador y sin un fixture de prueba** (lección de la bitácora: un contrato
sin validador se rompe en silencio).

---

## 3. Referencias cruzadas: qué vigila quién

| Referencia | Vigilancia actual | Quién |
|---|---|---|
| `ObjectSpawn.targetLevel` → nivel existente, celda transitable, con vuelta | ✅ | `tools/validar_enlaces.py` |
| Alcanzabilidad desde `playerStart` (flood fill) | ✅ | `tools/conectividad.py` |
| Egaroth = un solo grafo (no islas) | ✅ | `tools/validar_mundo_unido.py` |
| Esquema estructural de `assets/catalogs/*.json` (campos obligatorios + tipos) | ✅ | `tools/validar_catalogos_rpg.py` |
| Contrato ObjectCatalog de `assets/objects/libreria_*.json` | ✅ | `tools/validar_libreria_dnd.py` |
| Manifiesto de proyecto: ficheros declarados existen | ✅ | `ProjectIndex::check()` (C++) |
| `objectId` con ficha **dentro de una build** | ✅ | `tools/build_proyecto.py` |
| Controles anunciados ↔ enlazados (bidireccional) | ✅ | `tools/verificar_controles.py` |
| `ObjectSpawn.objectId` → catálogos **del proyecto** (fuente, no build) | ❌ nadie | el loader no valida ids (decisión documentada): objeto invisible sin error |
| `CombatData.skillIds` → skills registradas (legacy: solo `tajo`/`cura`/`golpe_gelatinoso` en `Application::init`) | ❌ nadie | caída silenciosa al ataque básico |
| `SkillCheckRequest.skillId` → `assets/catalogs/skills.json` | ⚠️ runtime avisa e ignora | sin validación estática |
| `BattleRequest.monsterId` → ObjectDefinition de categoría enemy | ⚠️ runtime enciende `flagDefeat` | sin validación estática |
| Referencias internas de los catálogos RPG (tabla 1.5: `skillIds`, `passiveId`, `lootIds`, `linkedSkillId`, `refId` de loot…) | ❌ nadie (validar_catalogos_rpg.py es estructural) | el puente las midió una vez (1.910 rotas antes del fix de ids prefijados); nada vigila que no reaparezcan |
| `Project.entry` → aventura/nivel que resuelve | ⚠️ `entryKind` lo anota y la pantalla lo muestra | sin fallo de validador |
| Imagen de tileset declarada por un TMX existe | ⚠️ solo en runtime (`loadLevel` aborta) | sin validador |
| Flags de aventura (requires/doneFlag) | — | libre por diseño: el contenido inventa flags sin recompilar; no se valida |

**Los ❌ son el alcance del entregable S1 nº 2 (`tools/validar_todo.py`).**
Criterio de terminado del roadmap: cada error reportado con fichero,
entidad y coordenada/índice.

---

## 4. Documentación desactualizada detectada

1. `FORMATO_NIVELES.md` §2 dice que `category` admite 3 valores
   (`prop`/`enemy`/`pickup`); el parser acepta **`npc`** desde la Fase de
   comercio (`src/Level/ObjectCatalog.cpp:30`). Corregir al cerrar S1.
2. `FORMATO_NIVELES.md` §5 lista como skills registradas `tajo`, `cura`,
   `golpe_gelatinoso` — cierto solo para el `SkillCatalog` legacy; el
   sistema Nd6 tiene ~500 skills + 210 hechizos en `assets/catalogs/`.
   Mismo nombre, dos catálogos (ver riesgo 2).
3. La tabla 7.9 de la bitácora (proyectos y niveles) y el doc del atlas
   (Gliaddokx "sin resolver") están por detrás de los datos; la bitácora
   ya establece que esas cifras no se fijan (8.8).

---

## 5. Riesgos vivos que afectan a S2+

1. **`objectId` no se valida al cargar** (deliberado): id roto = objeto
   invisible/inerte. Los validadores de la sección 3 son la red; el
   roadmap además quiere que el botón Playtest lo explique.
2. **Dos `SkillCatalog` con el mismo nombre**: `::SkillCatalog`
   (`Game/Skill.h:103`, legacy, registrado a mano) vs
   `RPG::Catalogs::SkillCatalog` (`RpgCatalogs.h:822`, Nd6). Confundirlos
   compila y falla en silencio.
3. **`EnemySpawn` sigue parseándose** y `GameSession` lo ignora: niveles
   viejos con `enemies[]` cargan sin sus enemigos.
4. **Doble propiedad de oro e inventario**: `GameSession::m_gold` +
   `Player` legacy vs `CharacterSheet.gold/inventarios`. Al enganchar el
   sheet único (pendiente) hay que declarar el dueño o se desincroniza.
5. **Caché de `CharacterSheet`**: `set_character_sheet` sin
   `recompute_derived` previo deja `healthCap() == 0` → GameOver
   instantáneo. Toda `populate_*` debe recomputar primero
   (`EnemyBrain` se protege con `max(1)`; `Player` no).
6. **`GameSession` crea enemigos desde `CombatData` plano**, no desde
   `MonsterCatalog`: los 432 monstruos tipados solo viven en demos.
7. La máquina de capítulos de `Application::update()` hace dos
   `loadLevel("assets/levels/ciudad_centro.json")` forzados: cualquier
   trabajo de viaje entre naciones debe condicionarlos.

---

## 6. Siguiente entregable S1

`tools/validar_todo.py`: entrada única que ejecuta los validadores
existentes y añade las comprobaciones ❌ de la sección 3, con salida
`fichero → entidad → problema`. Modo `--proyecto <id>`: valida solo el
pack (catálogos del manifiesto vs niveles del manifiesto), que es la
unidad de trabajo del editor. Sin dependencias nuevas (stdlib), paths
relativos a `Path(__file__)` (regla 2.15 de la bitácora), y se registra
su ejecución en la Parte 0.
