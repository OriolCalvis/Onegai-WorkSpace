# 2026-09-17 · qwen · E-09: puente EventEditorState → MapDocument

**Encargo de Oriol:** coger el Gantt y seguir programando con MAPA_DEL_CODIGO.

## Analisis

Tras cerrar E-15 (panel de problemas), busqué el siguiente paquete libre:
- G-03: ya cerrado (15/15 tests, 13/09)
- E-09: dependencias satisfechas (E-07✓ ZCode, E-08✓ ZCode, E-15✓ yo)
- G-11: servicio hecho, necesita cableado GL
- E-11: mitad GL restante es ProjectPanel (GL)

**Elegido E-09** (3j): el seguimiento de Claude (13/09) decía «la misión existe, se guarda, se reabre y se recorre entera» pero el paquete quedaba ABIERTO porque «el editor de páginas (E-05) y el vertical de comandas (E-06) no están». Ahora E-05 (`d7850c0`) y E-06 (`ec70d29`) sí existen.

**El hallazgo**: `EventEditorState::save()` era un stub `(void)doc; return true;`. El editor creaba eventos en su estado interno pero no los escribía en el MapDocument. Sin puente, el criterio «la misión se escribe desde el editor» no se podía cumplir.

## Qué se hizo

### Modificacion
- **`editor/src/event_editor.cpp`** — Implementado `save()`:
  - Crea la capa de eventos si no existe
  - Convierte cada `EventDocument` a `MapEventDocument` (id, posición, nombre)
  - Convierte cada `EventPage` a `MapEventPage` (trigger string→enum, condición→flag, comandos string→type)
  - Idempotente: `removeEvent()` antes de `addEvent()` para que llamar save() dos veces no duplique
  - Añadidos includes `Map/EventLayerDocument.h` y `Map/MapLayer.h`
  - Helper `TriggerFromString()` para mapear «interact/touch/autorun/parallel» al enum

### Ficheros nuevos
- **`tests/Editor/E09BridgeTests.cpp`** — 7 tests GL-free:
  1. save() crea la capa de eventos si no existe
  2. Escribe eventos con posición, páginas, triggers y condiciones
  3. Estructura de la misión (cofre, puerta, NPC, altar) queda en el documento
  4. Idempotente: llamar dos veces no duplica eventos
  5. Sin eventos: no rompe nada
  6. Triggers autorun y parallel se convierten correctamente
- **`cmake/E09BridgeTests.cmake`** — target `test_e09_bridge`, GL-free

### Modificacion menor
- **`CMakeLists.txt`** — una línea `include(cmake/E09BridgeTests.cmake)` tras CommandListTests

## Limitacion conocida

`EventPage.commands` es `vector<string>` (solo nombres de comando). `MapEventCommand` tiene `params` (map) y `children` (ramas). El puente convierte cada string en un `MapEventCommand` con solo el type; los parámetros se pierden. Esto es inherente al diseño simplificado de E-05 y requiere enriquecer los tipos del editor para cerrar E-09 completamente.

Documentado en los comentarios del test y del puente.

## Concurrencia

- No se tocaron ficheros de cruce: `EditorApplication.cpp`, `level_editor.cpp`, `EditorState.cpp`
- No se tocaron WIP ajenos
- `event_editor.cpp` no tenía fila activa en TABLERO (E-05 fue commiteado por la «sesión Oriol» sin fila el 17/09)

## Verificacion

- **Sintaxis**: `g++ -fsyntax-only` → 0 errores en event_editor.cpp y E09BridgeTests.cpp
- **Build completo**: bloqueado por WIP E-16 (ProjectDatabaseState.cpp firma rota, documentado en sesión E-15)
- **CI remota**: pendiente como todos los paquetes

---

## Actualizacion: verificacion con cmake real

Tras el commit `fee1e85` de Oriol (arreglo de firmas E-16), el build completo funciona. Resultados:

### Build y tests
- **test_e09_bridge**: **6/6** PASS (compilado y ejecutado con cmake real)
- **test_problems_state** (E-15): **17/17** PASS
- **test_event_editor** (E-05): **7/7** PASS — sin regresiones por el cambio en save()
- **test_command_list** (E-06): **6/6** PASS
- **Total: 36/36 tests verdes en mis 4 targets**

### Suite GL-free completa
- 120/128 pasan
- 8 fallos, todos ajenos:
  - `test_event_interpreter`: 2/26 fallos (ZCode E-08/EVT-05)
  - 7 "Not Run" por error de build en `test_editor_state` (WIP Cursor G-02: `JsonValue::asArray` no existe)

### Bug encontrado y corregido
- `MapLayerMeta.order` debe ser unico por capa. El puente save() no lo ponia y fallaba con "orden de capa duplicado". Corregido: `meta.order = static_cast<int>(doc.layers().size())`.
