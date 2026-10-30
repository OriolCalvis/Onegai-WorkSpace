# SESIÓN 2026-09-03 — Codex — C-22: retirada de la resolución por CD

- **Dónde:** `MotorGraphico-main/MotorGraphico/`, `TABLERO.md` y este registro.
- **Commits:** MotorGraphico `7b2decd` (17 archivos, +230/−317; código, demos, comentarios y seguimiento propios).
- **Qué se hizo:** continuación solicitada por Oriol de las tareas libres del `PLA_DEFINITIU.md`. Se releyeron tablero, plan y estado de Git. La sesión de qwen había liberado C-22 dejando cinco consumidores de la API de dados antigua; se reclamó esa continuación antes de crear archivos.

## Terminado y cómo

1. **Retirada de API:** eliminados `CheckOutcome`, `resolve_against_cd`, `resolve_against_defense`, `defense_to_cd` y `CD_MAX` de `DicePoolEngine.h`, junto con sus includes innecesarios. `opposed_degree` conserva las reglas existentes de Ed. 3.
2. **Cinco consumidores migrados:** `demo_dice_engine` comprueba y muestra grados enfrentados; `demo_nd6_distribution` simula pools rivales reales y deja de presentar CD/CA como la resolución actual. Las demos `narrativa_nd6`, `prologo` y `senal_ceniza` ejecutan `GameSession` real mediante `examples/NarrativeCheckSession.h`, una fixture sin mapa ni GL. La fixture construye un beat con el request original; la partida tira los dados y enciende las flags.
3. **Rutas narrativas conservadas:** los guiones ahora especifican primero dados del jugador y después del rival. El prólogo recorre su ruta SUCCESS y La Señal de la Ceniza sus cuatro recorridos anteriores. Se comprueba el total consumido por cada ruta para detectar guiones desalineados: prólogo 11; señal 26/19/11/34.
4. **Regresiones de reglas actuales:** los cuatro grados narrativos, compatibilidad de `cd` antiguo como pool rival, prioridad de `rivalDice` explícito y empate con pools vacíos se comprueban por la sesión real. En el motor de dados se añaden casos de doble pifia, cero éxitos sin pifia, pools vacíos y todo-seises que empata o pierde.
5. **Registro de programación:** conservado el trabajo previo de qwen en `seguimiento/C-22.json`, añadidas las acciones y pruebas de esta continuación y eliminado el pendiente de las cinco demos. Regenerados Markdown, Gantt y HTML del plan. C-22 sigue abierto por RuntimeTileMap y CI.

## Verificación

- Al sustituir la copia de reglas de las demos por GameSession, los guiones antiguos fallaron en `demo_prologo` y `demo_senal_ceniza`: ya no bastaba con fijar el pool del jugador. Tras especificar ambos pools, **5/5 demos afectadas pasan** con las mismas ramas narrativas exigidas.
- Build completa local: `cmake --build /tmp/motorgraphico-c20-build -j4`, correcta, incluidos `juego` y `level_editor`. El enlazador avisa de bibliotecas repetidas en otros targets; no impide la compilación y no se ha modificado ese grafo ajeno.
- `ctest --test-dir /tmp/motorgraphico-c20-build --output-on-failure --label-regex gl-free`: **57/57, 0 fallos**. Incluye la base previa y trabajo de otras sesiones, sin atribuirse sus cambios.
- `python3 MotorGraphico/tools/comprobar_declaraciones_rpg.py MotorGraphico`: **0 errores sobre 93 declaraciones**.
- Búsqueda en `include/src/examples/tests`: cero referencias a los cinco símbolos retirados, incluidos los comentarios obsoletos corregidos.
- SHA-256 idéntico antes/después para los cuatro archivos `GameSession.h/.cpp` y `Application.h/.cpp`: se usó la API pública sin modificar la reserva ajena.
- `git diff --check` y `git diff --cached --check`, limpios. De CMake se prepararon exclusivamente los tres enlaces nuevos de las demos, sin incorporar el scaffolding ajeno ni el include C-11 pendiente.
- Logs locales: `/tmp/motorgraphico-c22-before-fixtures.log`, `/tmp/motorgraphico-c22-demos.log`, `/tmp/motorgraphico-c22-full-build.log` y `/tmp/motorgraphico-c22-ctest.log`.

## Pendientes y convivencia

- Los overrides de `RuntimeTileMap` pertenecen al bloque previo sin versionar. Siguen pendientes de quien consolide esa base, conforme a la regla 2 de la BITACORA.
- CI pendiente para cerrar formalmente el paquete; no se ha ejecutado ni declarado CI remoto.
- El resto de cambios de editor, Map/Runtime/World/Project, guardados, CMake y planificación permanece como estaba. No se añadieron ni borraron los duplicados macOS ajenos `Check 2.h` y el SVG duplicado del Gantt.
- Base y huellas de esta sesión: `/var/folders/87/43f3pv591cq8y6r4px7fy4lc0000gn/T/codex-c22-dice-o97lx95p`.

- **Deja esperando a Oriol:** nada nuevo. El tablero y el plan indican los pendientes técnicos restantes.
