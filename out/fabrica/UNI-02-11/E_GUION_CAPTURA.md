# E-GUION-01 — Modo guion de captura (aprobado por Oriol el 03/11: «Mode guió al joc»)

Base: `agente/claude/UNI-02-11` (después de GORRAN-LIOS). Motor: `MotorGraphico-main/MotorGraphico/`.

## Problema
Las comprobaciones visuales (combate con 5 enemigos, persecución de los sectarios, campanas del Ocaso, polvo de pasos, animación de PNJ) necesitan pulsar teclas, y desde Claude no se puede (System Events pide el permiso de Accesibilidad de macOS). `juego N` solo vuelca el último fotograma. Además, con `--nivel` el jugador sale con el marcador provisional `personaje.png` (`src/Engine/Application.cpp:378`) porque se salta el creador.

## Supuestos (paso 0)
- La entrada pasa por un solo punto: `src/Engine/Application.cpp:919` (`m_input.update(readGameInput(...))`).
- `Render::VuelcaFramebufferPPM` (`include/Render/CapturaGL.h`) sirve para cualquier fotograma, no solo el último.

## Comportamiento esperado
`./juego [--nivel X] --guion guion.txt`: el juego lee un fichero de texto y, en cada fotograma, aplica las teclas del guion como si se pulsaran de verdad (aunque la ventana no tenga el foco), con un **dt fijo** (1/60) para que los fotogramas sean reproducibles. Formato, una orden por línea (`#` para comentarios):
```
<fotograma> pulsa <TECLA>      # la mantiene pulsada
<fotograma> suelta <TECLA>
<fotograma> toca <TECLA>       # pulsa un fotograma y suelta
<fotograma> captura <nombre>   # vuelca captura_<nombre>.ppm
<fotograma> fin
```
Las teclas tienen nombres legibles (W A S D, ENTER, ESC, ESPACIO, E, TAB, M, flechas, números). Si una línea es errónea, el juego no arranca y dice la línea y el motivo.
Con `--nivel` o `--guion` sin partida, el jugador se viste con un aspecto por defecto (humano, pelo y ropa básicos) a través del mismo camino que el creador (`PantallaNuevaPartida`), no con `personaje.png`.

## Alcance
Puede tocar: `include/Engine/GuionCaptura.h` y `src/Engine/GuionCaptura.cpp` (nuevos, sin GL), `tests/Engine/GuionCapturaTests.cpp` (nuevo) y su registro en `cmake/`, `src/Engine/Application.cpp` (inyección en :919 y vestido por defecto), `src/Engine/ApplicationBucle.cpp` (dt fijo y capturas), `include/Engine/Application.h`, `examples/juego.cpp` (`--guion`), `src/Engine/PantallaNuevaPartida.cpp` / su `.h` (solo para exponer el vestido por defecto).
No toca: datos de aventura, reglas, HUD ni el comportamiento sin `--guion` (salvo el vestido por defecto con `--nivel`).

## Criterios de aceptación
1. `GuionCapturaTests` (sin GL): parseo correcto, errores con número de línea, estado de teclas por fotograma (pulsa/suelta/toca) y lista de capturas. **Mutación:** que `toca` dure 2 fotogramas → la prueba falla.
2. Un guion de ejemplo `tools/guiones/titulo_nueva_partida.txt` (título → Nueva partida → creador → ciudad) produce sus capturas en el Mac y el jugador sale vestido.
3. Sin `--guion`, la suite gl-free sigue a 290/290 y `juego 3` se comporta igual.

## Verificación
`cmake --build out/build/macos-debug --target juego test_guion_captura && ctest --test-dir out/build/macos-debug -R guion` y la ejecución del guion de ejemplo (capturas a `out/fabrica/UNI-02-11/capturas/`).

Presupuesto: 60 min.
