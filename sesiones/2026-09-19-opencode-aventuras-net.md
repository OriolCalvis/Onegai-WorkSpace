# opencode — 2026-09-19 — Red de regresión de guiones contra catálogos

Fila: «Red de regresión de guiones contra catálogos (`[5/5] Aventuras`)» → **Hecho** (19/09).

## Qué se ha hecho

La «red que falta» del Pendiente 65: el check `[5/5] Aventuras` de
`tools/validar_todo.py` (startBattle.monsterId → categoría `enemy`,
trigger.target → objeto, skillCheck.skillId → skills.json) no estaba
registrado en CTest y por eso sobrevivían errores de contenido. Ahora:

1. **`tools/validar_todo.py`**: rama selectiva `--solo-aventuras` en `main()`
   (misma lógica de salida; sin tocar el resto del tool).
2. **`cmake/AventurasCatalogNetTests.cmake`** (NUEVO): `test_aventuras_monsterid`,
   labels `content;gl-free`, mismo patrón que `DeclarationCheck.cmake`
   (comprobación con python3, rutas relativas al script — bitácora 2.15).
3. **`CMakeLists.txt`**: un include.

Verificación: re-configuración de cero + `ctest -R test_aventuras_monsterid`
**verde**; la comprobación da **0 errores / 0 avisos** contra HEAD actual.

## Coordinación

- **Carrera sobre `CMakeLists.txt`**: un hermano reescribió el fichero
  (mtime 15:37) después de mi primer edit y se llevó mi include sin commitear
  (patrón del punto 33 del Pendiente). Reaplicado contra el contenido actual y
  re-verificado con `cmake` + `ctest` (el test se registra de nuevo).
- **Pendiente 32 quedó obsoleto durante la sesión**: la limpieza C-22 de las
  16 declaraciones de `AssetBrowserState` ya está commiteada (`d7f7eff` +
  sesión 17/09 claude); no he tocado nada de eso.
- **No he tocado** zonas ajenas en vuelo: editor/E-*, EVT-05
  (GameSession.h/SessionCommandHost/EventInterpreter…), fila Combate Ed.4 de
  qwen (`TierRules.h`/`DicePoolEngine.h`/`Morality`), SkillEffect.cpp.
- Mi rodaja candidata de `test_tier_rules` se **descartó**: qwen tiene una fila
  viva que editará exactamente `include/RPG/TierRules.h`/`DicePoolEngine.h`.

## Pendientes registrados

- **Sin commit** (protocolo: espero luz verde). Solo míos: `validar_todo.py`
  (rama `--solo-aventuras`), `cmake/AventurasCatalogNetTests.cmake` (nuevo) y
  una línea de include en `CMakeLists.txt`.
- Los 2 ficheros rojos del clon limpio de HEAD (EventInterpreterTests — EVT-05,
  EditorStateTests — editor) son de sus dueños con arreglo sin commitear; no
  implican mis targets.