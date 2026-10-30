# H2-07 — los enemigos del catálogo con su vida real (2026-10-01, opencode)

Fila del Gantt Fase H. Zona **limpia, verificada antes de escribir nada**:
`include/Game/EnemyCatalogBridge.h`, `tests/Game/EnemyCatalogBridgeTests.cpp`,
`assets/catalogs/monsters.json` y el `.cmake` del test no aparecen en
`git status`. De las 121 `.cpp`/`.h` sucias, ninguna es de este trabajo: son de
`Application*`, `GameSession*`, HUD/Pergamino, `editor/*` y assets de
niveles/mapamundi, de otras sesiones.

## 1. El enunciado del Gantt era falso

`JUGABILIDAD_FASE_H.md` H2-07: «**432 `MonsterDefinition` tienen `maxHealth == 0`
en el JSON fuente. Decidir si el canónico es el JSON o el calculado y
rellenar**».

Medido sobre `assets/catalogs/monsters.json`: **las 432 fichas tienen vida**.
Mínimo 12, máximo 80, y agrupada por tier:

| tier | fichas | valores de vida |
| --- | ---: | --- |
| 1 | 58 | 12 (todas) |
| 2 | 107 | 16 (106), 18 (1) |
| 3 | 144 | 34 (73), 20 (69), 60 (2) |
| 4 | 89 | 40 (48), 24 (21), 70 (19) |
| 5 | 34 | 46 (20), 80 (14) |

Así que no hay nada que rellenar y la decisión sobre el canónico no hace
falta. D-03 (11/09) ya lo había arreglado, sin que la fila del Gantt se
enterara. **Antes de escribir código, medí el enunciado** — y por eso la fila
que entregó no es la quesays el Gantt.

## 2. El defecto real: dos grafías para el mismo enemigo

`assets/objects/` nombra a los enemigos con prefijo:

```
enemy_dragon_negro_anciano      (ObjectCatalog)
dragon_negro_anciano            (monsters.json)
```

`EnemyCatalogBridge::createEnemy()` hacía `monsterCatalog->find(objectId)` a
pelo. De **460 ids de enemigo** en el ObjectCatalog, **ninguno** coincide tal
cual con un id del `MonsterCatalog`; **430** resuelven tras quitar `enemy_`.

Consecuencia medida: esos 430 caían al fallback
`ObjectCatalog.combat.maxHealth`, que es 10 en la mayoría. **Un enemigo de
tier 5 salía con 10 de vida.** Y no solo la vida: `skillIds` sale del
`MonsterDefinition`, así que también jugaba con el ataque del placeholder.

`grep enemy_` en `src/` e `include/`: **cero** prefijos, `substr` o
alternativas de `find`. El único punto de entrada a la ficha es
`EnemyCatalogBridge` — el fallo era de ahí, no de una convención asumida en
otra parte del motor.

## 3. El arreglo, y por qué en el puente

`EnemyCatalog::findMonster()`: **exacto primero, prefijo después**. Solo si el
exacto falla, quita `enemy_` (6 caracteres) y vuelve a mirar.

En el puente y **no en los datos**, por dos razones medidas:

1. `assets/objects/*.json` y `assets/levels/*.json` los tiene otra sesión sin
   commitear ahora mismo (400 ficheros sucios). Renombrar 460 ids en los datos
   rompería además el ObjectCatalog que usa el editor y los saves ya escritos.
2. El flujo es de una sola dirección (`canon → cartas → motor`). Si un día se
   unifican las grafías, ese cambio es de las cartas, y el puente seguirá
   funcionando porque prueba el exacto primero.

**Alcance real, sin adornos:** de 460 enemigos, **430 con ficha** (antes: 0).
Los **30 restantes no la tienen** y siguen sin ella: no hay datos, y aquí no se
inventa contenido. Antes esos 30 caían al placeholder por definición; ahora se
comportan como el fallback que son.

## 4. Falsabilidad

`test_enemy_catalog_bridge`: **12/12 → 13/13**.

Mutaciones:

| mutación | resultado |
| --- | --- |
| quitar el fallback del prefijo | **2 rojos** cazados |
| invertir el orden del `find` | 0 rojos → **test añadido**, luego **1 rojo** |
| quitar la guarda de `nullptr` | **crash** (139) |
| `>` por `>=` en la longitud del prefijo | **NO cazada**, documentada |

La segunda fila es la que merece explicación: **al invertir el orden, los 12
tests de entonces pasaban igual.** Nadie miraba qué ficha se elegía cuando
había dos candidatas. Por eso añadí
`amb_grafies_al_cataleg_gana_la_literal`, con un catálogo que tiene **las dos
grafías a la vez** (`enemy_slime` a 99/tier 4 contra `slime` a 15/tier 1): si
el prefijo se consulta antes, sale 15 y el test cae.

La cuarta la dejo escrita como está: con `>=` se busca el id vacío, que no
está en ningún catálogo, así que es equivalente en la práctica. Prefiero
decirlo a inventar un caso que no distinga nada.

## 5. Verificación

`git archive HEAD` con estos 2 ficheros: **209/210 objetivos, 3/3
ejecutables** (`juego`, `motor_editor`, `level_editor`).

`ctest` sobre el clon limpio: **sin rojos nuevos**. Siguen los 3 *Not Run* de
siempre (`ApplicationTests`/`ApplicationSceneTests` piden `saveGame`/`loadGame`/
`TitleScreen`; `test_application_cableado` enlaza `confirmOptionsCommand` de
H1-03) — zona viva ajena, sin tocar. Más `catalog_loader_contract` de `f2f91a1`
(EST-22, otro commit) y `linea_base_core` con sus 37 de deuda nueva, 4 de ellas
mías (`TargetSelection` y `DiceRollWidget`, núcleo sin montar).

**Nota de método:** `linea_base_core --rapido` **falla en el árbol con 8
problemas y pasa en clon limpio**. Son líneas de `CMakeLists.txt` de otras
sesiones apuntando a ficheros no versionados. No es mío y no lo he tocado; lo
digo porque el hook da el mismo ok que el guard lento, y quien lo lea en el
árbol puede pensar que el guard está roto.

## 6. Commits y ficha

- `MotorGraphico` **`f2a57a6`** — `include/Game/EnemyCatalogBridge.h` (+37/-3) y
  `tests/Game/EnemyCatalogBridgeTests.cpp` (+113).
- `planificacion/seguimiento/H2-07.json` — ficha nueva, en el commit siguiente.

Lo que **no** se tocó: assets de `objects/`, `levels/` y `maps/`; los
`Application*`; el `CMakeLists.txt` (los 8 problemas del guard son de otros);
`MAPA_DEL_CODIGO.md` y `alcance_real.json`, reservados por ALPHA-00 y D0007;
`linea_base_core.json`, que es la foto congelada de D0007/EST-42.