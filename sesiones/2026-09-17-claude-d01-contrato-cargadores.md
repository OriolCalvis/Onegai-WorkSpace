# 2026-09-17 · Claude (Cowork) · D-01: guardia del contrato de los cargadores + razas sin velocidad

**Encargo de Oriol:** seguir con el Gantt usando `MAPA_DEL_CODIGO`, con cuidado por los demás agentes. Va después de [E-14](2026-09-17-claude-e14-colision.md) y [G-02](2026-09-17-claude-g02-herramientas.md).

## Elección

Lo que queda libre del Gantt está bloqueado por decisiones (DEC-06, DEC-13), cae en zonas activas (shell de codex, E-13/E-15 de qwen, E-08 de zcode) o es del agente que recorre el plan sin fila (E-16, G-13). **E-04 `comercio.abrir`** se descartó: obliga a tocar `GameSession.h`, que tenía WIP ajeno, y cuatro ficheros de test de otros paquetes que lo usan como «el comando sin cablear». Se escogió el pendiente de **D-01** que el propio registro dejaba escrito: una comprobación estática de que `expected_fields()` sigue diciendo lo que `from_json` lee. Su zona es de ficheros nuevos más `RpgCatalogs.h` en solo lectura.

## Qué se hizo

- `tools/comprobar_contrato_cargadores.py`, con `--autoprueba` (7 casos sobre copias), registrado en CTest como `catalog_loader_contract` y `catalog_loader_contract_selftest` (`cmake/CatalogLoaderContractCheck.cmake`).
- **Hallazgo real, arreglado:** `f23dba9` (13/09) quitó de `RaceDefinition::from_json` las lecturas de `baseSpeed` y `compatibleDeityIds`, mientras la declaración ganaba los alias del asset (`speed`, `compatibleDeities`). Desde entonces las 43 razas cargaban velocidad 30 y ninguna deidad, sin error; Hondaforja declara 25. El test de D-06 no lo veía porque solo comprueba que la velocidad esté en [20, 50]. Se amplió la fila del TABLERO para cubrir ese arreglo de dos lecturas antes de hacerlo.
- `tests/RPG/CatalogLoaderRegressionTests.cpp` (`test_catalog_loader_regressions`).
- Una excepción con su motivo: `EquipmentDefinition.defenseBonuses` se declara sin leerse a propósito (vacío en las 413 piezas y vigilado por la línea base de D-10).
- Anexo `planificacion/seguimiento/D-01-contrato.json`, con el detalle y lo pendiente.

## Verificación

- El guardia da OK en el árbol arreglado y los 6 desajustes de razas sobre HEAD sin arreglar; la autoprueba pasa 7/7.
- La regresión pasa 3/3 con el arreglo (también con ASan+UBSan) y falla 3/3 sin él.
- Siguen verdes `classes_races_backgrounds` 7, `catalog_contract` 7, `character_creation` 4, `catalog_content` 32 y `validar_catalogos.py`.
- **Sin verificar:** CMake real y CI (g++ 11 en la VM, sin cmake).

## ⚠️ Commits automáticos de «Eigent User»

Entre las 10:54 y las 10:56 una herramienta firmada «Eigent User» hizo cinco commits (`9010984`, `2d05aae`, `626b18f`, `a289fd6`, `42f464c`) con mensajes genéricos («Save workspace before Task…», «Checkpoint Task workspace changes»):
- **`9010984`** versionó **todo el WIP en vuelo de otras sesiones** (E-14, E-15 ProblemsPanel, E-16, G-13, G-12b, D-11…) de una vez, contra la regla 2 del protocolo.
- **`42f464c`** se llevó mis ficheros de D-01 medio minuto antes de mi commit. Por eso `2e388ff`, que es el que describe el cambio, solo contiene el anexo de seguimiento; el código está en `42f464c` y es idéntico al verificado.
- Además añade `terminal_logs/blocking_commands.log` en la raíz del repo, y sigue haciéndolo.

No lo he deshecho, porque reescribir la historia pisaría a todo el mundo. **Oriol: conviene parar o configurar esa herramienta.**

## Para quien siga

- `rpg_declaration_check` está en rojo en HEAD: 33 declaraciones sin cuerpo en `AssetBrowserState.h`, `CommandListState.h` y `EventEditorState.h`, que llegaron versionadas por los commits de arriba.
- Falta decidir `defenseBonuses` con D-05/G-12, y D0003 sigue en `proposed`.
