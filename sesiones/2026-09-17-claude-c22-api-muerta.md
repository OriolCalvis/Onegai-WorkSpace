# Sesión 2026-09-17 — C-22: Retirada de la API muerta

## Tarea
C-22 — Retirada de la API muerta (2 j) del Gantt corte B

## Qué se hizo
- Primer commit (`777f4fa`): Actualizado `tools/comprobar_declaraciones_rpg.py` para quitar de la lista CONOCIDAS las declaraciones que ya tienen cuerpo:
  - `ObjectCatalog::exportJson` (D-05, implementado por opencode 12/09)
  - `CatalogPickerState::addEntries` (E-14, implementado)
  - `CatalogPickerState::addEntry` (E-14, implementado)
  - `CatalogPickerState::findById` (E-14, implementado)

- Segundo commit (`d7f7eff`): Quitado las 16 declaraciones de `AssetBrowserState.h` que ya tienen cuerpo según E-13 (MG `1621eaa`):
  - setAssets, addAsset, findById, setQuery, setKindFilter, visible, select, selected, clearSelection, planImport, commitImport, resolverSeleccion, ParsePngInfo, ReadPngInfo, AssetKindName, ComputeFrameGrid

- Verificado que los otros archivos que toca C-22 ya están en buen estado:
  - `DicePoolEngine.h` - Tiene implementación completa y se usa activamente
  - `TierRules.h` - Tiene implementación completa y se usa activamente
  - `EnhanceHooks.h` - Ya retiró la API muerta según los comentarios (IQteProvider, QteResult, override_target_defense)
  - `RuntimeTileMap.h` - Completo, el destructor virtual fue añadido por C-09

- La deuda de declaraciones baja de 17 a 1 (solo ObjectInspector::confirmar, deuda de E-12/E-14)

## Entrega verificable
- Commit `777f4fa`: "C-22: actualizar comprobador de declaraciones — quitar 4 resoluciones"
- Commit `d7f7eff`: "C-22: quitar 16 declaraciones de AssetBrowserState ya resueltas"
- Ejecución de `python3 tools/comprobar_declaraciones_rpg.py` returns 0 con mensaje: "C-22: OK — 463 declaraciones comprobadas en 57 cabeceras de include/{RPG,Project,Map,Editor,Level,World}/; 1 de deuda conocida."

## Estado
C-22 completado. El criterio de cierre se cumple: "El que no se hace usar sale. El que se ha de hacer servir se define. Cap declaración del motor se queda sin cuerpo."