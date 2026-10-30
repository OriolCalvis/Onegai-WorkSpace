# 2026-10-08 · Claude (Cowork) · BND-ESCENAS-01 (+ FASE1-BND-01) — Boundington con las escenas del vault

Encargo de Oriol (08/10, en catalán): «Té, continua la programació dins el motor gràfic del següent:», con la lista
«Boundington: lo que hay que sacar del gestor documental». Detalle de la sesión, pruebas y decisiones:
[`MotorGraphico/sesiones/2026-10-08-claude-bnd-escenas.md`](../MotorGraphico-main/MotorGraphico/sesiones/2026-10-08-claude-bnd-escenas.md).
Relevo: `MotorGraphico/claude/relevo-2026-10-08-bnd.md`.

- **Commits:** ninguno en el Mac; aplicado en el árbol de trabajo de MotorGraphico con preimagen (`out/claude_fase1_bnd/`, `out/claude_bnd_escenas/`: 28 ficheros, comprobados uno a uno).
- **Deja esperando a Oriol:** las 12 preguntas de `CANON_BOUNDINGTON_CAMPANA01.md` §8 (las que paran el Ocaso: la vida de Sandes, la ficha de los guardias, en qué noche es la fiesta, quién escribe los textos que faltan) y revisar lo que decidió Claude (§9.1, §10.1) y sus textos (§9.2).

| Ruta (`MotorGraphico/`) | Qué |
|---|---|
| `planificacion/CANON_BOUNDINGTON_CAMPANA01.md` | NUEVO — la Campaña 01 del vault, escena a escena, con el texto literal; qué pide al motor; las preguntas; cómo quedó en el juego. |
| `include/RPG/Narrative/FranjasDelDia.h`, `src/RPG/Narrative/FranjasDelDia.cpp` | NUEVOS — el día y la franja de la historia. |
| `NarrativeBeat.h`, `BeatsAventura.cpp`, `NarrativeEngine.*`, `GameSessionNarrativa.cpp`, `GameSessionInteraccion.cpp`, `AdventureBeatValidation.cpp` | `pasarFranja`, `tablaAleatoria`, `descanso`, `darObjeto`; `huyen` y `sinMatar` en los combates. |
| `HudRetales.*`, `MinimapaTinta.cpp` | «Día 1 · Mañana» debajo del nombre de la zona. |
| `assets/adventures/boundington_primer_dia.json`, `_segundo_dia.json`, `_ocaso.json` | Las escenas: 78, 47 y 27 beats. |
| `examples/demo_boundington.cpp`, `demo_campana_boundington.cpp`, `tests/RPG/FaseUnoBoundingtonTests.cpp`, `cmake/FaseUnoBoundingtonTests.cmake`, `CMakeLists.txt` | Las pruebas: 152 de 152 beats disparados; 9 casos del motor. |
| `tools/validar_todo.py`, `FORMATO_AVENTURAS.md` | `darObjeto` y los enemigos del grupo validados; el formato documentado. |
| `planificacion/seguimiento/FASE1-BND-01.json`, `BND-ESCENAS-01.json` | NUEVOS. |
