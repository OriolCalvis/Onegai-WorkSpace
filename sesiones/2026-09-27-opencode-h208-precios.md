# H2-08: barrido de datos + recuperación de H2-05 tras un `git stash` ajeno

**Autor:** opencode · **Fecha:** 27/09/2026 · **MotorGraphico** · sin commit

## Resumen en tres líneas

1. **H2-05 había desaparecido del árbol** y está **recuperado** (no verificado).
2. **H2-08-precios** es un hallazgo real y mecánico, pero **queda bloqueado** por
   EST-23b: parche y pruebas escritos, cero líneas aplicadas.
3. El barrido de los 25 catálogos explica por qué H2-04, H2-08-consumibles y
   H0-08 no eran programables hoy: **a los datos ejecutables no les llega nadie**.

## 1. H2-05: lopertise un `git stash` ajeno

Al empezar a trabajar, `grep repartirBotin src/Game/GameSessionInteraccion.cpp`
devolvió **0** y los 2 casos de `test_game_session` habían desaparecido del
fichero de pruebas. El reflog explicaba el resto:

```
7b1deb3 HEAD@{0}: reset: moving to HEAD
7b1deb3 HEAD@{1}: commit: H3-04: restaurar test_shop_flow en CMakeLists.txt
482675e HEAD@{2}: reset: moving to HEAD~1
```

Tres vueltas de *commit + reset* del mismo mensaje (H3-04, de Claude) y, además,
`stash@{0}` con un WIP que **incluye código mío**: el blob
`ae998aca…` (dangling) y la versión de `tests/Game/GameSessionTests.cpp` dentro
del stash traen `repartirBotin` y el caso
`al_ganar_el_botin_del_monstruo_entra_en_la_mochila`.

O sea: **otro agente hizo `git stash` sobre el árbol sucio compartido** y se llevó
trabajo sin commitear de otra sesión (el mío, y también el test de H2-08 de esta
misma sesión, que había quedado dentro del mismo stash).

**Recuperación (sin `git stash pop`, que would've aplicado un WIP ajeno enorme):**

- `src/Game/GameSessionInteraccion.cpp`: re-aplicados los 2 hunks de H2-05
  (`repartirBotin` + captura de `enemyObjectId` antes de destruir la entidad) con
  la nomenclatura del fichero actual (`m_inventory`, `m_monsterCatalog`…), no la
  del blob (`sessionData()->…`, que era el refactor a medias de EST-23b).
- `include/Game/GameSession.h`: reintroducida la declaración de 4 líneas junto a
  `startBattle`.
- `tests/Game/GameSessionTests.cpp`: recuperados los 2 casos del stash y colocados
  antes de `main()`.

**Estado: sin verificar.** `motor_level` no compila con EST-23b en el aire, así que
los 2 casos no se han ejecutado. En la sesión anterior la verificación dio 22/22; eso
ya no es evidencia de nada porque el árbol cambió debajo.

## 2. H2-08-PRECIOS: hallazgo real, aplicación bloqueada

**El bug:** `EquipmentDefinition::price` no lo lee nadie en `src/Game/`. La ruta
de precio real es `GameSession::basePriceOf()` → `ObjectDefinition::pickup.price`,
y en todos los catálogos de objetos y niveles hay **0** objetos de equipo con
precio a mano.

**Por qué duele:** 282 de las 413 piezas del catálogo son alcanzables como objeto
(143 desde `loot_tables.json`, 74 desde `classes.json`, 38 desde
`pregen_characters.json`, 27 desde `monsters.json`) y llegan por vías que no pasan
por el catálogo de objetos. Con precio 0, `resolveShopOffers` **descarta** el
artículo y `sell` responde «eso no tiene precio aquí»: **el botín de H2-05 no se
puede vender.** Es el eslabón que faltaba entre H2-05 y el comercio.

**Lo escrito y no aplicado** (todo en
`MotorGraphico/planificacion/seguimiento/H2-08-PRECIOS.md`): 25 líneas en dos
ficheros, sin `Application.*` y sin miembros nuevos, más 2 casos en
`GameSessionTests.cpp` que cubren las tres reglas (precio del PNJ > catálogo real
> precio del objeto) y el `sell` al 60%.

**Por qué no se aplicó:** `GameSessionComercio.cpp` estaba en vuelo por EST-23b
(Claude) —el diff ya traía su refactor a medias, `40 miembros` → `1 miembro`— y la
ley del repo dice que el testigo de un fichero grande lo lleva uno solo. Además
`motor_level` no compilaba, así que nada habría sido verificable. Se quitaron mis
hunks a mano, sin `git checkout` (que habría borrado su WIP) y sin `#if 0`
(código muerto, prohibido por la ley).

Segundo rostro del mismo bug, **no tocado** por ser carril de Codex (G12):
`src/Game/InventoryMenuState.cpp:56` — `pickup.price <= 0` ⇒
`ItemCategory::Quest`, o sea una pieza de equipo sin precio se clasifica como
objeto de misión en la mochila.

## 3. El barrido: por qué hoy no había nada programable

Método: para cada `assets/catalogs/*.json`, la unión de sus claves contra
`grep -rlw` en `src include` (37.364 líneas de código). Lo que sale son campos que
nadie lee; el filtro bueno es *de qué tipo* es cada uno.

**Con datos pero sin forma ejecutable (prosa):**

| Catálogo | Campo | Qué es en realidad |
|---|---|---|
| `consumables.json` (31) | `effect.description` | prosa; **0** campos ejecutables (`healHp`, `clearOneCondition`… todos vacíos) |
| `conditions.json` (16) | `cureConditions` (13/16) | frases: «Superar la tirada de liberación» |
| `equipment.json` (413) | `restrictions` | «Precisión: DES», «Habilidad de Oro Viejo: …» |
| `adventures.json` | `enemigoIds`, `villanoIds`, `trampaIds` | sin `Struct` en C++: no llegan |
| `pregen_characters.json` | 18 campos | sin `Struct` en C++ |
| `classes.json` | `healthScaling`, `primaryResource` | sin `Struct` en C++ |
| `npcs.json` | `attitude`, `palanca`, `secretHook` | sin `Struct` en C++ |
| `events.json` | `playerOptions`, `oneTime`, `continuesTo` | sin `Struct` en C++ |

**Con forma pero sin consumidor:** `skills.json: evolvesInto, upgradePath`,
`spells.json: evolvesInto, school`, `passives.json: synergyTags`,
`races.json: activeTrait, passiveTrait`.

**Consecuencia sobre el Gantt de hoy:**

- **H2-04** (hitos de nivel): `grantMilestoneOnEnd` vacío en todos los assets y
  `CharacterSheet::tierUp()` sin llamantes. Sin dato, cablearlo es otra ruta
  muerta (misma lección que `LootResolver`).
- **H2-08 consumibles**: la mitad «el juego lee el catálogo» no puede dar
  comportamiento sin inventar la semántica de la prosa.
- **H0-08 eventos de mapa**: el código **ya está entero** —el intérprete se crea
  (`ApplicationNivel.cpp:314`), arranca `Autorun`/`Parallel` y corre cada frame
  (`ApplicationBucle.cpp:186`), incluido el cambio de mapa (`mapa.ir`). Lo que
  falta es **contenido y un dato**: ningún nivel declara `mapDocument`
  (`LevelLoader.cpp:42` ya lo lee) y la capa de eventos de
  `assets/misiones/mision_llave.json` tiene **0** eventos publicados. El generador
  (`demo_mision_llave`) no compila porque su `.cmake` exige `TARGET motor_slots`, y
  ese target no existe en el repo. Bloqueo de contenido, no de código.
- **H1-04** (G-09, creación de personaje): 2,5 j cruzando `Application` (escena
  nueva) + Render/HUD (14 pasos de pantalla) + `Player::populate_from_class_race_background`,
  en zonas reservadas por Codex. Descartado por forma, no por valor.

## 3bis. El árbol se movió dos veces más mientras escribía esto

Al terminar los docs, EST-23b aterrizó (commit `61d89d7`, H1-04 de otro
agente) y **volvió a pisar `GameSessionInteraccion.cpp` y `GameSession.h`**: mis
hunks de H2-05 y la declaración desaparecieron por segunda vez. Se han
re-aplicado con la nomenclatura de su versión final (`sessionData()->m_battle`,
`m_inventory`, `m_monsterCatalog`, `m_mode`).

Y el bloqueo no es solo que «no compila todavía»: **`motor_level` no compila en
este instante**, con dos incoherencias distintas del mismo refactor:

- `src/Game/GameSession.cpp:31` sigue incluyendo `Game/SessionData.h`, y ese
  fichero **ya no está en `include/Game/`** (sí queda `SessionTipos.h`).
- `src/Game/GameSessionNarrativa.cpp:43` llama a `sessionData()`, que la
  cabecera ya no declara.

Son 20 errores, todos de EST-23b, ninguno mío. No se han tocado: la ley del
repo y el protocolo mandan no arreglar el WIP de otro, y encima H1-04 acaba de
entrar en el árbol (`include/Game/NewGameFlow.h`, commit `61d89d7`), así que la
zona de `GameSession` tiene dos agents distintos dentro ahora mismo.

**Consecuencia práctica: H2-05 y H2-08 están en el árbol pero sin verificar, y
cualquier agente que siga tocando `GameSession*` puede pisarlos otra vez.** Lo
que los haría inmunes es un commit, y el commit no se hace sin luz verde.

## 4. Estado del árbol al terminar

- `include/Game/GameSession.h` +1 declaración (mía, 4 líneas con comentario),
  re-aplicada por tercera vez.
- `src/Game/GameSessionInteraccion.cpp`: H2-05 re-aplicado (2.ª y 3.ª vez).
- `tests/Game/GameSessionTests.cpp`: 2 casos de H2-05 recuperados.
- `planificacion/seguimiento/H2-08-PRECIOS.md`: nuevo (hallazgo + parche + pruebas).
- `TABLERO.md`: fila H2-05 con el estado real (recuperada, sin verificar) y fila
  H2-08 bloqueada con el hallazgo.
- **Cero líneas en `GameSessionComercio.cpp`** (fichero de Claude) y **cero en
  `Application.*`**.
- `GameSession.cpp` muestra un diff de 54/57 líneas que **no es mío**: es el
  refactor de EST-23b.

`motor_level` no compila (EST-23b). En cuanto aterrice: `make test_game_session`
(esperar 24 casos) y, si se aplica H2-08, sus 2 casos más.

### Huellas para detectar si esto sigue en pie

El árbol estuvo mudándose **más de seis veces** durante la sesión (los
`md5` de los tres ficheros cambiaron entre lecturas). Estas son las huellas con
las que H2-05 queda coherente y completo:

```
a839e0c1a1c813336bd727bee6c37222  src/Game/GameSessionInteraccion.cpp
73cfcdf5f2b6f450582f3df69028bd66  include/Game/GameSession.h
3b98bc6c5bb5361772b3eafbeb1264c7  tests/Game/GameSessionTests.cpp
```

Comprobación rápida de coherencia (las 6 piezas):

```
grep -c "void GameSession::repartirBotin"  src/Game/GameSessionInteraccion.cpp  # 1
grep -c "repartirBotin(enemyObjectId)"     src/Game/GameSessionInteraccion.cpp  # 1
grep -c "std::string enemyObjectId;"       src/Game/GameSessionInteraccion.cpp  # 1
grep -c "enemyObjectId = fight->objectId"  src/Game/GameSessionInteraccion.cpp  # 1
grep -c "void repartirBotin"               include/Game/GameSession.h           # 1
grep -c "MOTOR_TEST_CASE(al_ganar_el_botin\|MOTOR_TEST_CASE(huir_no_suelta" \
     tests/Game/GameSessionTests.cpp                                              # 2
```

**Aviso para quien siga:** el `.cpp` está en el estilo viejo (`m_inventory`,
`m_battle`) y la cabecera en el nuevo de EST-23b (`sessionData()`), porque al
aterrizar el refactor el `.cpp` volvió a HEAD. Es incoherente por sí solo, pero
el cuerpo de `repartirBotin` es portable tal cual: al cerrar EST-23b solo hay que
cambiar esos cuatro `m_` por `sessionData()->m_`.
