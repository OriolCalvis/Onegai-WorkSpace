# 11/09/2026 — D-03: monstres del catàleg amb vida real

- **Gantt:** corte B, fase D (depende de D-01, commitejat). Triat amb `MAPA_DEL_CODIGO.md`: `RpgCatalogs.h` lliure (D-04 commitejat `795f8f6`, només segell aliè al diff) y `CatalogContentTests.cpp` igual; fora Application/editor (codex), `tests/Game|Engine` (C-24) y condicions.
- **Fila reclamada antes del primer fichero.** Sin commit a propósito (regla 2). Rama `feature/editor-s1-contratos`.

## El bug (medido)

El cargador buscaba `maxHealth/ca/defMental/resFis/precMag/movement` en la RAÍZ, pero el asset los anida: `defenses{ca, defMental, maxHealth, movement, precMag, resFis}` + respaldo `derived{ca, defensaMental, movimiento, resistenciaFisica, vida}`. Resultado: 432/432 con `maxHealth = 0`, `ca = 10`. `stats{CON,DES,INT,CAR}` (mayúsculas) y `statarr_json` ya se entendían; `tier`/`skillIds` en raíz ya iban.

## Qué se ha hecho (`include/RPG/Catalogs/RpgCatalogs.h` + test)

1. `from_json<MonsterDefinition>`: vida/defensas con prioridad `defenses{}` → `derived{}` (claves ES) → legados de raíz → defaults. DEC-02: manda el asset.
2. `lootIds`: el botín va en `loot`, no en `lootIds` (siempre vacío antes). Una línea, mismo patrón.
3. `expected_fields()`: +`defenses`, +`derived`, `lootIds`→alias `loot` (el contract-test exige unicidad: verificado).
4. `tests/RPG/CatalogContentTests.cpp`: `acolito_de_penumbra_dice_lo_que_dice_su_carta` (vida 12, ca 11, defensas, stats 1/1/4/2, tier 1, rol, loot ×2) + `cap_monstre_del_cataleg_te_vida_i_stats` (432 con `maxHealth > 0` y stats no nulas — el criterio del paquete).
5. Fuera a propósito: `passive` es objeto `{name, description}`, no id — mapearlo a `passiveId` sería inventar identidad; queda para D-11. `magnitudeHint` de ataques sin dato en carta: intacto.

## Verificación (build limpio en temporal, assets copiados por `copiar_assets`)

- `test_catalog_content` **12/12** (5 D-02 + 5 D-04 + 2 D-03 nuevos).
- Falsabilidad: con el loader de HEAD fallan EXACTAMENTE los 2 nuevos (`maxHealth == 12`, `conVida == 432`); los otros 10 pasan.
- Regresión: `test_catalog_contract` 7/7 (los 20 cargadores declaran; assets estructurales cargan).
- Incidencia de método (ya vista 2 veces): los binarios de catálogos leen `assets/` relativo al cwd — correrlos fuera del build dir da 12 fails falsos (`r.isOk()` en cada `carga_*`). Solo vale el run desde el build dir / CTest.

## Trabajo ajeno en vivo (regla 2)

- zcode D-04 commitejat; su fila desapareció del tablero pero `RpgCatalogs.h`/`CatalogContentTests.cpp` siguen con diff (verificado: SOLO el sello `marcar_paquetes`, cero lógica) — por eso esta rebanada era segura.
- `Application.h` trae WIP real ajeno (C-17 `refreshWorldMarkers` + C-23 fog/lightmap): ni mirado más.
- `EventLayerDocument.h/.cpp` untracked (¿E-01 en marcha sin fila?) — E-01 vetado de momento.
- codex C-14 commitejat (`05edf18`, 67/67); C-24 (otro opencode) sigue en `tests/Game` con ficheros nuevos.

## Pendiente de commit (pathspec, cuando se asiente)

`include/RPG/Catalogs/RpgCatalogs.h` (mis hunks) + `tests/RPG/CatalogContentTests.cpp` (mis 2 tests). Súmense D-09, D-05-motor, D-05-maxHealth anteriores.
