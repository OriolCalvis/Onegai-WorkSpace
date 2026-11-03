# Capturas del juego (03/11/2026, `agente/claude/UNI-02-11@e43db51d`, preset `macos-debug`)

Cómo: `./juego [--nivel X] N` (modo humo: vuelca el último fotograma a `juego_output.ppm`), convertido a PNG con `sips`. Ventana real de 1280×720 en el Mac, OpenGL real.

| Fichero | Qué se ve | Qué demuestra |
|---|---|---|
| `00_titol_f60.png` | Pantalla de título: pergamino con Continuar/Nueva partida/Opciones/Salir sobre la ciudad | El arranque normal llega al título y lo dibuja bien. |
| `01_ciudad_centro_f90.png`, `02_ciudad_centro_f100.png` | Casco Antiguo: HUD (PV, oro, objetivo), minimapa, PNJ, río, edificio del jugador translúcido | Cámara U4-01 centrada en el jugador; oclusión de edificios. |
| `03_ciudad_centro_ocaso_f300.png` | Boundington al ocaso (oeste y casco) | El nivel carga, pero **se ve de día**. |
| `04_afueras_ocaso_f300.png` | Las Afueras al ocaso | El nivel carga, también **de día** y muy vacío (encaja con la decoración que falta, F-REST.1). |
| `05_prado_del_rio_f120.png` | Prado del río (inicio del prólogo) | El nivel inicial carga. |

## Qué NO se ha podido capturar todavía
La mesa de combate con 5 enemigos, la persecución de los sectarios, las campanas del Ocaso, el polvo de los pasos y la animación de PNJ en movimiento necesitan **pulsar teclas** dentro del juego. Desde aquí no se puede:
- enviar teclas con `osascript`/System Events pide el permiso de Accesibilidad de macOS (la petición se quedó colgada, sin respuesta), y ese permiso solo lo puede dar Oriol;
- el juego no tiene repetición de entradas ni guion de captura.

## Hallazgos (pendientes de confirmar jugando)
1. Con `--nivel` se ve el objetivo del **prólogo** («Acércate a la orilla del río») en cualquier mapa: el nivel no carga la aventura de su día. Es normal en el modo de prueba, pero significa que el Ocaso no se puede comprobar así.
2. El jugador sale con el **cuerpo base** (sin pelo ni ropa) porque se salta el creador. Falta comprobar si en una partida nueva normal sale vestido.
3. Los niveles `*_ocaso` **no tienen tinte de atardecer** en el modo de prueba. Puede depender del estado del día (que aquí no se carga): por confirmar.
