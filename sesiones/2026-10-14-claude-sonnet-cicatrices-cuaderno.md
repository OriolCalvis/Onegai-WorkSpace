# 14/10 — Claude (Sonnet): AV15-08, las cicatrices en el cuaderno

Encargo de Oriol: «Les cicatrius al quadern» (lo dejó apuntado el Opus 2).

## Qué se hizo
- Copia de trabajo = fuentes + `claude_dialogos04` + `claude_diario01` (las cicatrices y el diario aún no están aplicados en el Mac).
- `include/HUD/Pergamino/PaginasJugador.h`: `FuentesCuaderno::cicatrices` (int, 0 por defecto, así los `FuentesCuaderno{...}` de cinco campos siguen compilando) y `anotaCicatrices(estado, cuantas)`.
- `src/HUD/Pergamino/PaginasJugador.cpp`: `fichaPagina` llama a `anotaCicatrices` tras Pureza/Marcas; fila «Cicatrices: N» solo si N ≥ 1 [propuesto: sin fila si no hay, como Pureza y Marcas].
- `tests/HUD/CicatricesCuadernoTests.cpp` (2 casos) añadido a `test_pergamino` por ancla en `cmake/Pergamino.cmake`.

## Comprobado
- Configure + build Debug de `test_pergamino` y `test_cicatrices`; ambos pasan. El contenedor no tiene cabeceras X11: se desactivó GLFW solo en la copia de prueba (no está en el paquete).
- `aplicar.sh` probado sobre un árbol con las preimágenes: aplica 3 ficheros y el parche de CMake queda igual.
- PaginasJugador.cpp 424 líneas (≤ 500).

## Sin comprobar
- `tools/linea_base_core.py`: la copia no traía `planificacion/linea_base_core.json`.
- `ctest -L gl-free` completo y la captura en pantalla.

## Falta (carril de Claude Cowork)
Ver `out/claude_cicatrices01/LEEME_CLAUDE.md`: `EstadoJuego::cicatrices`, el `FuentesCuaderno{...}` de `InterfazPergamino.cpp` (líneas ~258 y ~306) y rellenarlo en ApplicationHud.cpp.
