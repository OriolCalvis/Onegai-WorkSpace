# D0022 — Respuestas de Oriol al recull de decisiones (03/11/2026)

Estado: **aceptada** (decisiones del product owner). Origen: `out/fabrica/UNI-02-11/DECISIONS_PENDENTS_ORIOL.md`, contestado por Oriol el 03/11 en el mismo fichero (copia literal en `out/fabrica/UNI-02-11/DECISIONS_PENDENTS_ORIOL_RESPOSTES.md`). D0021 (C8-02) se perdió con el `.git` del 29/10; el número no se reutiliza.

| # | Decisión | Dónde se aplica |
|---|---|---|
| 1.1 | `agente/claude/UNI-02-11` es la **rama de integración** (sustituye a `claude/unificacion-28-10`, perdida). Base de las tandas nuevas. | worktrees, `agy_nocturno.sh` (1.6) |
| 1.2 | Las comprobaciones visuales las hace Claude con capturas que justifiquen cada afirmación. | `out/fabrica/UNI-02-11/capturas/` |
| 1.3 | PO-04: estudiar cómo conectar el repo a GitHub antes de publicar. Sin push hasta que Oriol elija destino. | PO-04 |
| 1.4 | Oriol renueva la clave de Qwen (Bailian Token Plan). | — |
| 1.5 | Permisos de solo lectura para Antigravity de noche: los pone Oriol. | `settings.json` de Antigravity |
| 1.6 | `agy_nocturno.sh`: base = rama de 1.1 y relevo solo en la respuesta. Script de Oriol. | `agy_nocturno.sh` |
| 1.7 | Oriol baja los primeros sonidos (CC0 o Erang, fila en `LICENCIAS.md`). | `assets/audio/` |
| 1.8 | PixelLab: generar la lista pendiente en el estilo actual, más **peinados decorados** y otros elementos que amplíen el juego. Los créditos son limitados: primero lo barato y de más impacto; se puede retocar después. | `out/fabrica/UNI-02-11/PIXELLAB_ENCARREC.md` |
| 2.1–2.6 | Aprobados: so U1-01 (cuando haya sonidos), misiones U3-01, diario = H3-02 (datos) + MISIONES-01 (vista), `simular_economia.py` a 4 días, cobertura real de premios, validador de beats con cinemáticas y eventos. Los lotes de código del Pla U siguen necesitando el ámbito de ficheros congelado antes de empezar. | Pla U, `tools/` |
| 3.1–3.8 | Todo según la recomendación del recull. Además: **probar los «líos» de Gorran de verdad, estudiar qué queremos y avanzar.** `genera.py` y `tools/parche_c_trama.py` se retiran (no ejecutar). | día 2 / Gorran |
| 4.x | Las zonas perdidas van a Fable con encargos de contexto mínimo. | `out/fabrica/UNI-02-11/fable/` |
| PO-52, 53, 54, 55, 57 | Aceptadas según la recomendación de la bandeja. PO-53 se vuelve a medir con la economía de 4 días. | `BANDEJA_PRODUCT_OWNER.md` |
| 6.1 | Ojos de Venides: **azul grisáceo** (Oriol). | canon (la ficha la crea Oriol en la vault) |
| 6.2 | El aviso de Xila hace sonar las campanas (ampliación de Oriol del 29/10). | `b3_xila_avisa` |
| 6.3 | `cultista_carcelero` → **«cultista de la barricada»** (Oriol deja elegir a Claude: no exige inventar un vínculo con Verina que la vault no da). | aventuras de Boundington |
| 6.4 | Músicos: la línea del cuarteto **se reparte entre los 3** (Oriol deja elegir a Claude: así los tres tienen voz sin añadir texto nuevo). | aventuras de Boundington |
| 7.x | Fable revisa cada tema antiguo para **adaptar la idea a las funcionalidades pactadas hoy** (no restaurar tal cual) y proponer cómo dar más calidad y gameplay. 7.5 (pasar D0003/4/6/9/12 a `accepted`) queda sin confirmar. | `out/fabrica/UNI-02-11/fable/` |
