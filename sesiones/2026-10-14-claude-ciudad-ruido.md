# 2026-10-14 · Claude (Sonnet) · AV15-03 — Quitar ruido de la ciudad

Encargo de Oriol (14:39): «treure soroll de la ciutat (AV15-03): fer una llista dels objectes que tapen la vista o
tallen el pas, deixar carrers rectes de la plaça a cada botiga i fer translúcid el que tapa el jugador».
Informe completo, con tablas: [RUIDO_CIUDAD.md](../planificacion_alcance_y_vision/RUIDO_CIUDAD.md).

## Qué hay ahora

1. **Una medida que se puede repetir** (`tools/medir_calles.py`, solo lectura): rutas más cortas desde el puente y
   desde la plaza del Casco hasta cada puerta, con la colisión del juego; cuánto desvío ponen *solo los objetos*;
   puertas tapadas; carriles rectos; qué quitar para ahorrar más. `--comprobar` sale con 1 si algo falla.
2. **Los datos** (`tools/despejar_calles.py`): 14 objetos movidos, 0 quitados, en `ciudad_centro.json`. La puerta de las
   termas estaba sellada por un PNJ; las 7 calles principales tienen ahora un carril recto de punta a punta (eran 3).
3. **El generador** llama a lo mismo (`tools/gen_ciudad.py`, 4 líneas) para que regenerar no lo deshaga.
4. **Los objetos que tapan se funden** al 40 % cuando bloquean el paso y caen delante del jugador
   (`OcclusionRules::propInFadeWindow` + `Application::render`); el motor solo fundía tiles y bloques.

## Lo que más cuenta

- La ciudad **ya se andaba casi en recta**: los objetos solo ponían +1 % (puente) y +2 % (plaza) de pasos. El problema
  no era el desvío sino una puerta sellada, estorbos en los carriles y que lo alto tapaba al jugador.
- Mi primera cifra desde la plaza (11,9 %) salía de una celda mal elegida, encajada entre PNJ; la buena es el 6,0 %.
- **No he compilado el motor ni lo he jugado** (aquí no hay `cmake`). Comprobado: la regla pura (21/21 con
  `g++ -Wall -Wextra`), que `ApplicationBucle.cpp` parcheado pasa `-fsyntax-only` contra los includes reales,
  que el paquete se aplica, se niega a la segunda y se deshace, y que el generador da lo mismo que el script.

## Ficheros

- Nuevos: `tools/despejar_calles.py`, `tools/medir_calles.py`, `planificacion_alcance_y_vision/RUIDO_CIUDAD.md`.
- Tocados (por ancla, paquete `out/claude_ciudad01/`): `include/Render/OcclusionRules.h`,
  `tests/Render/OcclusionRulesTests.cpp`, `include/Engine/Application.h` (un campo, **en la misma línea**: ya pasa de 500),
  `src/Engine/ApplicationBucle.cpp` (+17, queda en 418), `tools/gen_ciudad.py`, `assets/levels/ciudad_centro.json`.
- No toca el combate, `GameSession`, ni el suelo (TMX).

## Para Oriol

- **PO-44** (bandeja): ¿abrimos un paso entre la plaza y la iglesia (26 pasos para 18 en recta)?
- Mirar el fundido en el juego (§6 del informe); `OcclusionRules::kAlfaProp` es una constante.
- `tools/gen_ciudad.py` no llega a `ciudad_oeste` (título con tilde); ya fallaba antes de esto.
