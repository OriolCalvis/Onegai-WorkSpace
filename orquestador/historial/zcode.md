
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# zcode — tarea actual
**Asignada:** 14/10 por Claude · **Repo:** `MotorGraphico-main` · **Tipo:** continúa tu fila (worktree aislado, como ya decía)

## CAPTURAS-UI — terminarla
Tu fila viva desde el 28/09 (petición de Oriol para revisar lo visual). Hoy **no existe** `MotorGraphico/capturas_ui/`.
Zona: `examples/capturas_ui.cpp`, `cmake/CapturasUi.cmake`, `capturas_ui/` (PNGs + `INDICE.md`). Sin tocar `Application.*` ni `CMakeLists.txt` del árbol.

**Hecho cuando:** hay PNG de título, selección, creación de personaje paso a paso, exploración, prólogo, inventario (pestañas), pausa, opciones (+idioma), tienda, diálogo, combate, negocio y game over (los que existan), y `INDICE.md` dice cómo regenerarlas. En el informe lista qué pantallas **no** se pudieron capturar y por qué.

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV14-15 — Recorrido automático de todos los botones
Plan: `planificacion_alcance_y_vision/` (AV14, carril paralelo). Oriol (14/10): «revisar que todos los botones vayan bien y fluidos en los menús también es importante». Va sobre lo de CAPTURAS-UI.
- **Zona (solo esto):** NUEVOS `examples/recorrido_botones.cpp`, `cmake/RecorridoBotones.cmake` y `capturas_ui/botones/` (informe y capturas). Sin tocar `Application.*` (puedes usar `ApplicationTestAccess`, que es amiga de `Application`, como hacen las sondas).
- **Qué:** un ejecutable que recorre título, creación, pausa, opciones, inventario, tienda, diálogo y combate, pulsa cada botón (ratón y teclado), y apunta si responde, si cambia al pasar por encima y lo que tarda la pantalla siguiente.
- **Hecho cuando:** `capturas_ui/botones/INFORME.md` lista cada botón con OK/FALLA y su captura. Fila en el TABLERO.

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`


---
## Hasta el 15/10 (pasada a historial al poner la cola del 15/10)

# zcode — tarea actual
**Asignada:** 14/10 por Claude · **Repo:** `MotorGraphico-main` · **Tipo:** continúa tu fila (worktree aislado, como ya decía)

## CAPTURAS-UI — terminarla
Tu fila viva desde el 28/09 (petición de Oriol para revisar lo visual). Hoy **no existe** `MotorGraphico/capturas_ui/`.
Zona: `examples/capturas_ui.cpp`, `cmake/CapturasUi.cmake`, `capturas_ui/` (PNGs + `INDICE.md`). Sin tocar `Application.*` ni `CMakeLists.txt` del árbol.

**Hecho cuando:** hay PNG de título, selección, creación de personaje paso a paso, exploración, prólogo, inventario (pestañas), pausa, opciones (+idioma), tienda, diálogo, combate, negocio y game over (los que existan), y `INDICE.md` dice cómo regenerarlas. En el informe lista qué pantallas **no** se pudieron capturar y por qué.

---
## (Actualizado 14/10 noche por Claude, coordinador)
Tu SIGUIENTE (AV14-15, el recorrido de botones) **pasa a Claude Opus 2**, porque no te has lanzado (PO-02). Su informe irá en `out/claude_botones01/informe/`, **no** en tu `capturas_ui/`: tu zona sigue siendo tuya. Tu tarea actual, CAPTURAS-UI, no cambia (`capturas_ui/` sigue sin existir). Lo anterior, en `orquestador/historial/zcode.md`.

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
