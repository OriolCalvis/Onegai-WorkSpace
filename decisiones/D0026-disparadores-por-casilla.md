# D0026 — Disparadores por casilla que llevan al jugador (04/11/2026)

Estado: **aceptada** (Oriol, chat del 04/11). Origen: petición de Oriol («portar al jugador on ha de començar la cinematica i un cop alla executar l'evento») y diseño de Fable `out/fabrica/UNI-02-11/fable/F-DISPARADORES.md`.

| # | Decisión |
|---|---|
| 1 | Campo opcional `disparador` en las cinemáticas con zona (`cinematicas.json`): `lleva` (casilla, orientación, cómo), `accion` (`cinematica` / `beat` / `combate` / `marca`) y `once`. Se ejecuta con los pasos que ya existen; `puntoDeEntrada()` compartido por motor, arnés y explorador; validador en `validar_cinematicas.py`. |
| 2 | Por defecto el jugador **va andando** al punto; si no hay camino, fundido y aparece allí. |
| 3 | `combate <id>` lanza un **encuentro existente**, por la misma puerta que el efecto `startBattle` de los beats. |
| 4 | Una zona repetible (`once: false`) **se rearma al salir de la zona**, como los Touch del mapa. |

Plan: 6 pasos E del diseño (E1 punto de entrada compartido, E2 parser + pasos + pruebas gl-free con mutación, E3 validador, E4 combate, E5 `once:false`, E6 migrar `plaza_sermon`, `bgd_velas`, `base_ruta_*`).

## Proximidad y visión (Oriol, 04/11; estudio `fable/F-DISPARADORES_PROXIMIDAD.md`)

| # | Decisión |
|---|---|
| 5 | Orden: condiciones → (**zona** pisada **o** dentro del **radio**) → **línea de visión** al foco → `lleva` + acción. La zona actual sigue valiendo (nada de lo que funciona se rompe). |
| 6 | Radio por ámbito del nivel: **interior pequeño = al entrar** (sin visión); **interior grande = 5**; **exterior = 10**. Distancia Chebyshev. `disparador.radio` / `vision` pueden sobreescribirlo. |
| 7 | Ámbito: campo `ambito` en el nivel; si falta, se deduce (prefijo `interior_*` y tamaño: < 200 casillas = pequeño). |
| 8 | Tapa la vista: tile con colisión o objeto con el campo nuevo **`tapaVista`** (paredes, vallas, carros, armarios, portones). PNJ y objetos pequeños no tapan. La línea de visión se extrae de `TableroCombate::seVen` a un módulo compartido. |
