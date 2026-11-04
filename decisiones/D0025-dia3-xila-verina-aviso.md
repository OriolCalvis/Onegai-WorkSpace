# D0025 — Día 3: Xila con resaca, Verina y el aviso de Xila (04/11/2026)

Estado: **aceptada** (Oriol, chat del 04/11). Origen: hallazgos de E-MATRIZ-03 (`out/fabrica/UNI-02-11/MATRIZ_DIA3.md`).

| # | Decisión | Dónde |
|---|---|---|
| 1 | Si Xila se emborrachó el día 1 (`bnd1_xila_borracho`), **se queda visible el día 3** hasta que se le habla, para poder recuperarle (`b2_xila_resaca`). Hoy `cinematicas.json:398` lo oculta con `bnd2_encargo`, la misma flag que exige el beat. | `cinematicas.json` (`ocultoSi` de `xila`) |
| 2 | Subir a Verina (`b2_rosita_verina_arriba`) pide **`confianza_rosita >= 2`** (antes 3, que solo se alcanzaba acertando cuatro decisiones concretas). | `boundington_segundo_dia.json` ~1172 |
| 3 | El aviso de Xila del día 3 se hace **más accesible**: quitar alguna de las condiciones que lo hacen frágil (honor del día 1, esperar a la tarde sin correr a avisar a Venides, no callar el informe). Recomendación aplicada: que también valga correr a avisar a Venides; el honor del día 1 se mantiene. | `assets/eventos/boundington.json` ~25 y beats que lo alimentan |
