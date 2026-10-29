# UI-BG3-01 — las maquetas en el juego: ratón, 7 hojas, cofre y barra de cartas · 05/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-05-claude-pantallas-bg3.md`, la de las
maquetas. Oriol las aprobó: «aplícalo al juego, m'encanta». Mejor 7 hojas que 14 pasos, y
los 63 kits del cofre parametrizados («parametrízalo»).

La entrega, con capturas del juego real, un GIF del combate, la lista de kits y las
preguntas abiertas, está en `Claude outputs/ui_bg3_2026-10-05/LEEME.md`.

## Cómo está hecho

```
Engine::Input   RawInputState/InputSnapshot con ratón (posición, botones, rueda) y texto; teclas Q F L ← → ↑ ↓ 1-0
Engine          ratonEnPapiro (título, pausa, opciones, derrota); PantallaNuevaPartida solo traduce la entrada
Game (motor_level, sin GL)
  CuadernoCreacion   las 7 hojas: rejillas, búsqueda, filtro, historia en matriz, nombre; ficha sin equipo
  KitBasico          la regla del kit (assets/rules/kit_basico.json) para las 63 clases
  CofreKit           el cofre de El Vacío: kit a la mochila, flags y objetivo; el prólogo lo espera
  BarraCombate       la barra de cartas como datos: vista por carta, foco, emergente, registro,
                     espera del rival y aviso de la reacción; devuelve la fila del menú de siempre
HUD/Pergamino (motor_pergamino, sin GL)
  Lienzo             Zona (lo que responde al ratón), zonaEn, desplaza
  AtlasRetratos      retratos montados por capas (los sube PergaminoGL)
  PaginasCreacion*   el cuaderno de creación pintado (C02)
  MesaCombate*       el combate pintado (C03): bandeja, carta grande, turno, orden, placa, registro, emergente
  InterfazPergamino  pintaTitulo, pintaCreacion, pintaCofre y pintaCombate
Application      teclasCombate, turnoRival, confirmBattleCommand(fila), pasaTurno; fuera el panel del
                 enemigo, el menú de comandos, menuDeCartas y lineasDeCombate
```

- **El combate se sigue resolviendo igual.** La barra devuelve la fila del menú de
  siempre (0 Atacar, 1 Poción, 2 Huir, 3+ la mesa) y `confirmBattleCommand` la juega.
  Lo único nuevo en las reglas es `#pasar` (F Pasar) en `CartasEnCombate`: cierra el
  turno como lo cerraría la acción principal.
- **El rival actúa tras su rótulo** «TURNO DE …» (0,8 s, C03c). La barra lleva la cuenta
  y `turnoRival()` lo resuelve cuando toca: turno enemigo, `tickRound` y desenlace.
- **Lora de 13 px en el atlas de letras**, para los nombres de las cartas (72 px de
  ancho). Va al final del generador para que el resto del atlas no se mueva; lo comprobé
  píxel a píxel.
- **Sin ficha, las habilidades de siempre salen como cartas** (`vistaDeBatalla` con el
  catálogo legado). Desde H2-06 no se podían elegir.

## Verificación

- Pruebas nuevas en verde:
  - `test_barra_combate` 8/8;
  - `test_pergamino` 49/49, con 4 casos del combate pintado;
  - `test_cuaderno_creacion` 12/12;
  - `test_kit_basico` 11/11.
- `test_cartas_combate` 30/30, con `#pasar`.
- ctest en el contenedor, sobre el árbol del Mac de las 13:40 UTC más el paquete:
  203/211. Las 8 que fallan fallan igual sin el paquete. `test_application_scenes` baja
  de 5 fallos a 4: la prueba de selección del combate ya pasa con la barra.
- Guardia entera: las mismas 44 deudas. `Application.h` baja a 515 líneas y 89 miembros;
  `Application.cpp`, a 1188 líneas.
- Juego real con OpenGL por software: título, creación con clics, cofre y kit puesto, y
  combates enteros. Las capturas están en la entrega.
- **Aplicado al árbol del Mac**, sin commit: 74 ficheros, 30 nuevos y 44 modificados.
  - La pre-imagen md5 coincide en los 74.
  - Otras sesiones habían cambiado tres desde las 09:40 UTC: `interior_vacio.json` (mapa
    propio), `boundington.json` y `ApplicationNivel.cpp`. Los fusioné a 3 bandas sin
    conflictos y compilé y probé el árbol fusionado antes de escribir.
  - La post-imagen coincide en los 74.

## Pendiente

- **El commit.** Los ficheros nuevos van con sus líneas de CMake en el mismo commit:
  `cmake/CreacionBg3Tests.cmake` y su `include()`, y las fuentes nuevas de `motor_level`
  y `motor_pergamino`.
- **Preguntas a Oriol**, en el LEEME de la entrega:
  - el crítico del saqueador (22) contra un bardo de 19 PV sin el kit;
  - el fondo del título;
  - qué pasa si no cargan las letras del pergamino;
  - las tildes de los nombres de las cartas en el catálogo.
