# AV15-07 (contraste) — Contraste y tintas — Claude Sonnet, 14/10/2026

Estado: terminada (borrador para Oriol). Solo documentos; no se ha tocado código ni `Tema.h`.

Pedido por Oriol: «preparar la legibilidad (AV15-07): medir el contraste de cada texto sobre el pergamino y proponer las tintas, sin tocar código». Solo la parte de contraste; las líneas de 45–75, la trama del suelo, el tamaño en Opciones y el temblor de AV15-07 quedan sin hacer.

Leídos: `Tema.h`, `BancoMateriales.cpp`, `MaterialesPapel.cpp`, `Cuaderno.cpp`, `Pincel.cpp`, `PaginaCuaderno.cpp`, `TiendaHoja.cpp`, `DadosPergamino.cpp` y las texturas de `assets/textures/pergamino/`.

Hecho:
- `planificacion_alcance_y_vision/CONTRASTE_Y_TINTAS.md`: papeles medidos, matriz tinta por papel, 27 textos por rol, propuesta de tintas (Camino A y B), pestañas, dónde cambiaría cada cosa, lo no comprobado y el método con su script.
- Correcciones en `GUIA_DE_ESTILO.md` (PAL-05, §10, F18) y `EVALUACION_HEURISTICA.md` (EH-09, §5): la hoja de las capturas es la textura `hoja_a.png`, no un efecto del harness.

Hallazgo de paso: `hoja_b.png` no se usa (la elección de piel solo distingue por parámetros y las dos hojas no tienen ninguno; leído, no ejecutado).

Pendiente de Oriol: elegir entre aclarar la hoja (arte) o oscurecer `kTinta3`; ver las tintas nuevas jugando; la regla de estados (texto en `kTinta` y el color en borde, icono o chip).
