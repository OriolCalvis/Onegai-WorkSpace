# 2026-09-28 · zcode · H2-04 (Gantt Fase H, rodaja GL-free): hitos y tierUp con la ficha coherente

**Encargo de Oriol (standing):** seguir el Gantt del MotorGraphico con el
MAPA_DEL_CODIGO bajo coedición. Tercera tarea de la tanda (H1-04 `61d89d7`
e H8-03 `4fc8c00` los días anteriores).

## Por qué esta tarea

La zona GameSession seguía ocupada (EST-23b de Claude sin aterrizar; filas
H2-05/H2-08 de opencode vivas), así que H2-04 no se podía tomar entera (su
ficha pide «contar los hitos en GameSession»). Pero al leer su «qué ya
existe» con el MAPA saltaba un defecto real en la parte GL-free:
`CharacterSheet::tierUp` (implementado desde el corte B, **cero llamantes**
fuera de la propia clase) subía el `tier` **sin refrescar
`cachedHealthCap`**: quien lo llamara leería la vida tope del tier VIEJO —
`Player::maxHealth()` incluido. El mismo patrón de bug que H1-04 cerró
para la creación (ficha sin `recompute_derived` → PJ a 0 PV). Además el
comentario de `tierUp` prometía «único camino: mediante grantMilestone»
— un método que no existía.

## Qué hice

- **`CharacterSheet` gana `milestones` + `grantMilestones(count, rules)`**
  (total histórico, no saldo: el tier queda como función de los hitos
  concedidos jamás; repetir la llamada con el mismo total no salta más).
  Devuelve `TierChange{advanced, from, to}` para el HUD («subes a tier 3»).
- **Fix del cap obsoleto**: `tierUp` refresca `cachedHealthCap` con el tier
  nuevo. La fórmula vive UNA sola vez en `healthCapWith(rules, baseHealth)`
  (la usan `healthCap()` y el refresco, no pueden divergir);
  `cachedBaseHealth` recuerda el `baseHealth` del último `recompute_derived`
  — sin él, subir de tier sin catálogo a mano era imposible de refrescar.
  Fuera la línea muerta `needed = 1 + tier + 1` (se sobrescribía sola).
- **`tests/RPG/SheetMilestonesTests.cpp`** (8 casos, tabla por defecto de
  TierRules: t1 bonus 0/cap 35, t2 4/50, t3 9/70): cap refrescado ==
  ficha gemela puesta a mano en el tier nuevo y recomputada (equivalencia
  real, no tautología), ídem con el clamp del tier por medio, hitos
  insuficientes acumulan sin subir, varios tiers de una vez, techo
  TIER_MAX, `count<=0` no desordena, ficha que nunca pasó por recompute, y
  clase REAL de `classes.json` (63 clases, todas con baseHealth).
- `CMakeLists.txt` (+4 líneas junto a sus hermanos G-12 con
  `add_motor_unit_test`) y MAPA (+1 ruta). Cruce por blob construido
  (HEAD + solo mis líneas) como en H1-04.

Detalle que mordió y quedó corregido en el propio test: `TIER_MIN` es 0
(Civil), no 1 — el caso «sin recompute previo» asumía tier inicial 1.

## Verificación (clon limpio de `4fc8c00`)

- `test_sheet_milestones` **8/8 PASS**.
- Sin regresión: `test_sheet_life_cap`, `test_sheet_equipment_cap`,
  `test_character_creation`, `test_character_creation_flow`,
  `test_inventory_engine` (5/5), `test_h104_new_game_flow`,
  `test_shop_flow`, `test_battle_state`, `test_player_populate` — PASS.
- `test_game_session` **23/24**: el rojo `al_ganar_el_botin_del_monstruo_
  entra_en_la_mochila` es **preexistente de HEAD** — medido también con mi
  cambio revertido (1 fallo igual). Es el caso de H2-05 de opencode, cuya
  fila ya lo marcaba «SIN VERIFICAR / cuando EST-23b aterrice». No es mío
  y no lo toco; queda registrado aquí para su dueño.

## Commit

- MotorGraphico `7da10a0` (rama `feature/editor-s1-contratos`, padre
  `4fc8c00`): 5 ficheros, +272/−10. Guard con `update-ref <nuevo>
  <esperado>`; índice real realineado con `update-index --cacheinfo`.
- Raíz: TABLERO (fila a Hecho) + este fichero + INDICE regenerado.

## Qué queda de H2-04 (para el dueño de la zona GameSession cuando EST-23b aterrice)

1. Contar los hitos al terminar aventura: donde hoy se procesa
   `grantMilestoneOnEnd` (ya se parsea, `RpgCatalogs.h`), una llamada:
   `sheet.grantMilestones(n, tierRules)` — el cap queda coherente solo.
2. Política de PV actuales al subir de tier (la ficha no guarda vida
   actual): quién la tenga (Player/BattleState) decide si remata al cap
   nuevo. Documentado en la cabecera de `grantMilestones`.
3. «Mostrar el tier en la ficha»: `sheet.tier` ya es legible y el
   `TierChange` da el before/after para el aviso del HUD.
4. Serializar `sheet.milestones` en el guardado (SaveGameFormat, otra
   rodaja).
