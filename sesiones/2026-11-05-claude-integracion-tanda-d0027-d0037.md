# 05/11/2026 — Claude: integración de la tanda D0027–D0037 en el checkout

**Encargo:** Oriol, «Sí, integra-la». **Commit:** `2d13bb47`, merge de `agente/claude/UNI-02-11` @ `56db5e9b` en `recuperat/software-copia-30-10`. Sin push.

- Un conflicto, `sesiones/INDICE.md`, resuelto regenerándolo. El motor (`src`, `include`, `tests`, aventuras) queda idéntico al de la rama.
- Verificación en el checkout, preset `macos-debug`: compilación completa sin errores (49 s), `ctest -L gl-free -j6` 304/304 en 135 s, `validar_todo` 0 errores y 20 avisos. Logs: `out/fabrica/UNI-02-11/merge3_*.log`.
- No se tocan los cambios sin commitear que ya había: `.obsidian/workspace.json` y `out/fabrica/agy/`, de la cola nocturna.
- El worktree `~/onegai-ramas/UNI-02-11` no se ha borrado: espera a la decisión de Oriol sobre seguir en él (norma de limpieza).
- Pendiente de jugar a mano: refuerzos, invocaciones, cartas de línea o cono y los dos mineros en su escena (ver D0036 y D0037). Valores por defecto sin confirmar: el dado de huir (D0030 §3) y los UUID de partidas antiguas (D0037 §2).
