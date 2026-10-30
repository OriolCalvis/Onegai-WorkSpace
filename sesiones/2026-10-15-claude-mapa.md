# 2026-10-15 · Claude (Opus) · carril `mapa` — MAPA-02

Ronda del 15/10 (`orquestador/ronda_15-10.md`). Pedido de Oriol: «necesito ayudas en los minimapas para poder marcar las misiones y los objetivos que el jugador considere importantes».

Paquete: `MotorGraphico/out/claude_mapa01/` (por ancla, formato de `claude_reputacion01`).

## Qué hay

1. **Seguir una misión.** TAB > Misiones: ENTER sobre una misión empezada la sigue (deja la anterior). El pie dice «ENTER Seguir». La seguida lleva SEGUIDA; la empezada que no sigues, EN CURSO. La seguida manda en el retal OBJETIVO, el minimapa y la flecha. Por defecto, la activa de hoy. Se guarda en la partida.
2. **Flecha al objetivo.** Si el sitio (el objeto o la puerta del primer paso) no se ve: retal pequeño en el borde con el icono de misión, «N PASOS» y una punta carmín hacia él. No pisa retales ni carteles. Si se ve: un ◆ pequeño encima (o a sus pies si arriba hay un retal). Sin objetivo en el nivel: la chincheta más cercana. Con `animacionesReducidas`, quieta.
3. **Mapa grande (M).** El nivel entero (la imagen del minimapa), los sitios por su nombre, tú, el objetivo, las chinchetas y la leyenda; columna con el objetivo y lo que hay bajo el cursor. Flechas/WASD (mantenidas, a saltos) o ratón; ENTER o clic pone o quita una chincheta (5 por nivel, 3 colores que se alternan, cada una con su número). M o ESC cierra. Las chinchetas salen también en el minimapa. Se guardan por nivel.
4. **Pruebas** GL-free: `test_marcas_mapa` (4) y `test_guia_mapa` (5).

El retal de ayudas dice ahora «TAB Menú · M Mapa · ESC Pausa». La leyenda del minimapa crece con sus renglones (antes «Cerca: …» se salía del retal con cinco claves).

## Cómo se ha hecho (y lo que no)

- La VM de Cowork no respondía (device_bash caído toda la sesión). Trabajé en el contenedor de Claude con los ficheros de `$M` traídos a mano.
- Compilado y en verde en el contenedor: `test_marcas_mapa`, `test_guia_mapa`, `test_pergamino`, `test_save_game`, `test_diario_completo`, `test_diario_misiones`, `test_input_system`, `test_rumbo_niveles`, `test_confirmar`. Probado el paquete sobre una copia limpia (aplica, segunda vez se niega, compila, pruebas en verde).
- **`juego` no se ha enlazado** (no hay GLFW en el contenedor). `Application.cpp`, `ApplicationHud.cpp`, `PergaminoGL.cpp` y el resto de `src/Engine` pasan `g++ -fsyntax-only` con una cabecera GLFW de mentira. Falta: compilar `juego` y jugarlo.
- Las capturas son de una sonda sin GL (rasteriza los lienzos por software; el mundo es un fondo liso). Fuente en `out/claude_mapa01/sonda/`.

## Decisiones

- Todo el estado nuevo vive en `Pergamino::GuiaMapa`, que sustituye a `m_minimapa` en `InterfazPergamino` (la cabecera ya tenía 15 `m_`).
- Datos en `Game::MarcasMapa` (nuevo, `motor_level`): un solo campo en `SaveGameData` (`marcasMapa`, clave `"mapa"`, solo si hay algo).
- La tecla M: `InputKey::M` + `InputAction::ToggleMap` en `InputMap.h` y `InputSystem.cpp` (fuera de la zona listada, pero sin ella no hay tecla).
- Colores de las chinchetas: `kInfo`, `kExito`, `kAviso` de `Tema.h`, siempre con número (no solo color).

## Dudas para Oriol

- ¿Entrada «Mapa» en la pausa? No la he puesto: `PauseMenu` no es de este carril.
- ¿Los pasos son casillas (lo que hay) o prefieres «2 puertas» cuando el objetivo está en otro nivel?
