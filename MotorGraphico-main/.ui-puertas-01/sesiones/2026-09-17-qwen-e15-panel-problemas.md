# 2026-09-17 · qwen · E-15: panel de problemas y busqueda (mitad ImGui)

**Encargo de Oriol:** coger el Gantt del motor y seguir programando con MAPA_DEL_CODIGO, teniendo en cuenta agentes simultaneos.

## Analisis previo

1. Leidos BITACORA, TABLERO, Gantt corte B, MAPA_DEL_CODIGO y sesiones del dia.
2. **Agentes activos hoy (17/09):**
   - Claude: E-14 (aporto patch alternativo, dejo la sesion duena)
   - Codex: E-12 (validacion de llegada de puertas, `d5ec681`)
   - ZCode: E-07 cerrado (`30ca8c4`) + E-08 nucleo probado (`1662a64`)
   - OpenCode: G-12b (equipar refresca ficha) + EVT-05 (interruptores locales, `4720b62`)
   - Sesion "Oriol" (sin filas): E-05 (`d7850c0`), E-06 (`ec70d29`), E-14 (`6a3be66`), E-16 WIP (ProjectDatabaseState roto)
3. **Paquetes libres analizados:** E-13 (WIP de otra sesion qwen encima), E-15 (mitad GL-free hecha, dependencias satisfechas), E-16 (WIP en vuelo), F-* (bloqueados por DEC-06), G-01 (bloqueado por DEC-13).
4. **Elegido E-15**: la mitad GL-free (ProblemsState.h/.cpp, 17 tests) estaba versionada desde el 13/09 por Claude. E-06 aterrizó hoy (`ec70d29`), así que la infraestructura del panel ImGui está disponible. Zona limpia: ningún otro agente toca `editor/src/ProblemsPanel.cpp` ni `editor/include/ProblemsPanel.h`.

## Que se hizo

### Ficheros nuevos
- **`editor/include/ProblemsPanel.h`** — Clase ProblemsPanel con API de dibujo (`draw`), callback de navegacion (`NavigateCallback`), `requestFocus()` y `testPoint()` para humo. Tres pestañas: Problemas, Buscar, Referencias.
- **`editor/src/ProblemsPanel.cpp`** — Implementacion ImGui completa:
  - **Pestana Problemas**: lista con severidad coloreada, sitio (mapa > evento / pagina [ruta] tipo), mensaje. Tooltip con detalle. Controles Ir ahi / Anterior / Siguiente. Aviso de cambio de foco (MovedToNext, Lost). Detalle del problema enfocado.
  - **Pestana Buscar**: caja de texto que llama `SearchEventsByName`. Resultados con nombre e id. Sin distinguir mayusculas ni acentos (NormalizeForSearch de DEC-15).
  - **Pestana Referencias**: combo de tipo (Bandera/Variable/Interruptor local/Evento/Mapa/Objeto) + desplegable de nombres conocidos (`CollectReferenceNames`) + texto libre. Llama `FindReferences`.
  - **Barra inferior**: contadores de errores/avisos y puerta de playtest (`canPlaytest()`).

### Modificacion
- **`cmake/MotorEditor.cmake`** — Añadidos `editor/src/ProblemsPanel.cpp`, `src/Editor/ProblemsState.cpp`, `src/RPG/AdventureValidation.cpp`, `src/RPG/EventCommands.cpp` al target `motor_editor`. Mínimo cambio: una sola linea.

### Verificacion
- **Sintaxis**: `g++ -std=c++17 -fsyntax-only` → 0 errores.
- **Tests GL-free existentes**: `test_problems_state` **17/17** verdes (binario precompilado de `out/build/macos-debug`).
- **Build completo bloqueado**: `ProjectDatabaseState.cpp` (WIP sin commitear de E-16, sesion "Oriol") tiene firma `Result<void>` en el .cpp pero `bool` en el header. Rompe `motor_project` y casca toda la cadena. Zona ajena; documentado en Pendiente del TABLERO.

## Concurrencia

- No se tocaron ficheros de cruce: `EditorApplication.cpp` (E-10), `level_editor.cpp` (cruce de 10 paquetes), `EditorState.cpp` (cruce).
- No se tocaron WIP ajenos: `AssetBrowserState.h` (E-13/qwen), `ProjectDatabaseState.h/.cpp` (E-16), `InventoryEngine.cpp` (G-12b/opencode), `SkillEffect.cpp` (migracion BUFF/DEBUFF), `GameSession.h` (EVT-05).

## Pendiente / siguiente paso

- **Integracion en EditorApplication.cpp**: instanciar `ProblemsPanel`, llamar `panel.draw(state, &doc, status, navigate)` en el bucle principal. Toca E-10; pendiente del pase de integracion.
- **canPlaytest() como puerta**: F-01 (playtest) y G-06 (exportacion) deben consultarla antes de lanzar.
- **CI remota**: como todos los paquetes de esta rama.
