# Claude Opus 2 — tarea actual (15/10)
**Asignada:** 14/10 (noche) por Claude (coordinador) · **Modelo:** Opus, esfuerzo **alto** (high) · **Repo:** `MotorGraphico-main`
· **Tipo:** trabajo nuevo, entrega en paquete como hasta ahora. Lee antes el PROTOCOLO de `orquestador/README.md`.
La tarea anterior (D0013 + AV13-04, paquete `claude_dados01`) y su informe están en `orquestador/historial/claude_opus2.md`.

**Base:** el árbol de ahora en el Mac, que **ya lleva aplicados los once paquetes** de `Software/REVISAR.md`
(los `.aplicado` son del 14/10 22:28) y sin commitear. No toques ninguno de los ficheros que llevan esos paquetes;
solo los lees. Si necesitas cambiar uno, **para** y escríbelo en el INFORME.

## AV14-15 — Recorrido automático de todos los botones de todos los menús, con informe
De «Pendiente» del TABLERO. Oriol (14/10): «revisar que todos los botones vayan bien y fluidos en los menús también es importante».
(La tenía zcode como SIGUIENTE; zcode no se ha lanzado (PO-02) y se queda con su CAPTURAS-UI.)
- **Zona (solo esto):** NUEVOS `examples/recorrido_botones.cpp`, `cmake/RecorridoBotones.cmake`, su `include()` en
  `CMakeLists.txt` (por ancla, como `out/claude_reputacion01/aplica_ediciones.py`), y el informe y las capturas en
  `out/claude_botones01/informe/`. **No** escribas en `capturas_ui/` (es la zona de zcode).
  Sin tocar `Application.*`: usa `ApplicationTestAccess` (amiga de `Application`, como en `tests/Engine/ApplicationTests.cpp`).
- **Qué:** un ejecutable que recorre título, creación, pausa (con las confirmaciones nuevas de `claude_confirmar01`),
  opciones, inventario, tienda, diálogo, negocio y combate; pulsa cada botón con ratón y con teclado y apunta:
  si responde, si cambia al pasar por encima y al pulsar (los 5 estados de `GUIA_DE_ESTILO.md`), y cuánto tarda
  en salir la pantalla siguiente.
- **Hecho cuando:** `out/claude_botones01/informe/INFORME.md` lista cada botón con OK/FALLA, el motivo y su captura;
  el ejecutable se registra en CTest con etiqueta `gl` (no `gl-free`); `ctest -L gl-free` sigue igual.
  Paquete `out/claude_botones01/` con `aplicar.sh` que compruebe la preimagen. Fila en el TABLERO y fichero de sesión.
- Lo que falle **no lo arregles**: va en el informe (lo arregla Claude Cowork en AV14-16).

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits / paquete:**
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
