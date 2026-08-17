## Imported Claude Cowork project instructions

# LA BIBLIA — lectura obligatoria antes de trabajar

Este workspace (`Software/`) lo atacan **varios agentes en paralelo** (Claude, Codex, Cursor, ZCode…). La coordinación vive en un único fichero:

## → `BITACORA_DEL_PROYECTO.md` (en la raíz de `Software/`)

**Antes de tocar cualquier cosa**, léelo entero. Como mínimo: su PROTOCOLO, la Parte 0 (qué ha hecho cada sesión), la sección 2.1 (quién manda en qué) y la Parte 3 (trampas conocidas — todas han mordido de verdad).

**Al terminar tu sesión**, deja huella ahí: commit + entrada en la Parte 0 (append-only), y actualiza Parte 1/2/3 si procede.

Reglas que no admiten atajo:

1. El flujo es en una sola dirección: `canon (Onegai 2) → cartas (dndWeebCC) → motor (MotorGraphico)`. Sin aristas de vuelta.
2. Si encuentras trabajo ajeno sin commitear, **no lo deshagas ni lo commits** — documéntalo en la Parte 0.
3. No resuelvas discrepancias en silencio: lo que no puedas resolver, regístralo.
4. `require()` y nunca `assert()` en demos (Release define NDEBUG). Sin tildes/ñ en `lines`/`speaker`/`text` de JSONs de aventura. `ensure_ascii=False` en generadores Python.

Los cuatro directorios: `Onegai 2/` (el LORE, manda siempre) · `dndWeebCC-master/` (reglamento y mapa) · `MotorGraphico-main/` (ejecución C++) · `Onegai-Core/` (el puente).
