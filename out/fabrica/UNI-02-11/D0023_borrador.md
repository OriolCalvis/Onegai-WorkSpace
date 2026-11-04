# D0023 — Gorran y moral: respuestas de Oriol (03/11/2026)

Estado: **aceptada** (decisiones del product owner, contestadas en el chat el 03/11). Origen: `out/fabrica/UNI-02-11/GORRAN_LIOS.md` (preguntas 1–3) y `out/fabrica/UNI-02-11/fable/F-ANTIC_PROPUESTA.md` §7.3.

| # | Decisión | Dónde se aplica |
|---|---|---|
| 1 | Los **4 líos** de la aldea de Gorran caben en un día (coste 5 de 7). 3 cerrados es solo el umbral de la subasta. | `boundington_dia_gorran.json` (sin cambios) |
| 2 | Lío de las velas: si se llega tarde (`bgd_mia_perdida`), **solo se salva Tinet**; el cierre lo respeta. | `boundington_dia_gorran.json`: cierre del lío D (~2833) |
| 3 | Hablar con Gorran de frente (`b2_gorran_hablar`) **lleva a una tregua o alianza** si hay líos cerrados y aval. | `boundington_dia_gorran.json` ~16053 + prueba |
| 4 | La moral es de **un eje con 7 tramos** (la de 08/10, ya programada). El Reglamento Ed. 4 se corrige para decir lo mismo. | `REVISION_REGLAS_ED4` §2.8, `GLOSARIO_DE_MAGNITUDES.md` |

Pendiente de E (datos de aventura, con prueba y mutación): 2, 3 y los dos errores de GORRAN-LIOS (`bgc_barca_sale` + `bgc_hundir_exito` suman dos veces el lío C; `bgb_campana` depende de un flag que se pone después en el fichero).
