# D0024 — La tregua con Gorran se pacta en la subasta y le protege (04/11/2026)

Estado: **aceptada** (Oriol, chat del 03–04/11). Origen: hallazgo de E-MATRIZ-02 (`out/fabrica/UNI-02-11/MATRIZ_GORRAN.md`): `b2_gorran_tregua` (D0023.3) no se puede alcanzar jugando, porque al cerrar el tercer lío `b2_subasta_aviso` pone `bnd2_subasta_ya` y `cinematicas.json` (~1083) oculta a `gorran`. Lo mismo pasa con `b2_gorran_trato`.

| # | Decisión |
|---|---|
| 1 | La tregua se pacta **en la subasta**: con `bnd2_aval_serra`, la subasta ofrece una vía de tregua en lugar de pelea (página gemela: el menú ya tiene 4 opciones, el máximo). Sustituye a la tregua hablando con Gorran en su salón. |
| 2 | Con tregua, **la tregua le protege**: las opciones de pelea se cambian por las de pacto. Solo se puede romper con una opción explícita de traición. |
| 3 | Las pruebas no pueden hablar con quien el jugador no ve: el arnés respeta `ocultoSi` de `cinematicas.json`. |
