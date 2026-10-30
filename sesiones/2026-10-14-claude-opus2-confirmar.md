# 2026-10-14 · Claude (Opus, segunda sesión) · AV15-05a — Confirmar lo caro e irreversible

La tarea de `orquestador/claude_opus2.md` asignada a las 12:31: **EH-22, EH-23, EH-29 y EH-30** de la
evaluación heurística. Hasta ahora, salir, volver al título, cargar, guardar encima y comprar un negocio
se hacían al pulsar ENTER, sin aviso, y lo comprado no se podía revender.

## Qué hay ahora

1. **Una sola pregunta para todo** (`Game/Confirmacion`, nuevo, sin GL): título, una línea de detalle,
   «No» y «Sí», y una nota bajo el «Sí» con lo que pasará. **El foco empieza en «No»**: un ENTER de más
   no hace nada caro. ESC vuelve atrás, al sitio de antes.
2. **La pausa pregunta (EH-22, EH-23)** en su propio papiro, que cambia el título y las filas
   (`PauseMenu::pregunta`):
   - **Salir del juego** y **Volver al título**: «Perderás lo jugado desde las 12:31.» Si no hay ninguna
     partida guardada: «Perderás todo lo que no hayas guardado.»
   - **Cargar partida**: «Vuelves a la de las 12:31: pierdes lo de después.»
   - **Guardar partida** con una partida ya guardada: «¿GUARDAR ENCIMA? Sustituye la partida de las
     12:31.»
   - **Si no se pierde nada, no pregunta**: si la partida de ahora es igual que la guardada
     (`Game::mismaPartida`, compara las dos sin la hora), sale, vuelve, carga o guarda directamente.
     La hora sale como «las 12:31» si es de hoy y «el 14/09 a las 06:58» si no.
   - ESC en el título sigue saliendo sin preguntar (no se ha tocado).
3. **El negocio tiene su hoja de pergamino (EH-29)** (`HUD/Pergamino/HojaNegocio`, nuevo), en lugar del
   cuadro gris de siempre:
   - **En venta:** el nombre, la renta y tu oro, y la fila «Comprar · 1000 ORO». Si no te llega, la fila
     sale apagada y dice cuánto te falta.
   - **ENTER en Comprar ya no paga:** abre «¿COMPRARLO? Sastrería por 1000 de oro.», y bajo el «Sí»: «Te
     quedarán 500 de oro. Te dará 24 de oro por ciclo.»
   - **Tu negocio:** subir y bajar el alquiler (cada fila dice lo que te dará y lo que cuesta de moral;
     esto no pregunta porque se deshace con la otra fila), **Revender** y Salir.
   - Después de cada cosa, una línea dice lo que ha pasado: «Has comprado Sastrería. Te quedan 500 de oro.»
4. **Se puede revender (EH-30)**: «¿REVENDERLO? Sastrería: dejará de darte renta.», y bajo el «Sí»:
   «Recibes 500 de los 1000 que pagaste.» `GameSession::sellCurrentBusiness(pct)` devuelve el oro, quita
   el negocio y lo apunta en el registro. **El porcentaje es el 50 %** mientras Oriol no conteste la
   **PO-42** (en la BANDEJA). Está en una sola constante: `Game::NegocioEstado::kReventaPct`.
5. **El ratón** elige filas en la hoja del negocio como en la tienda, pero **no con la pregunta abierta**:
   así pasar el ratón por encima no puede dejar el foco en «Sí».

## Ficheros

- Nuevos:
  - `include/Game/Confirmacion.h` y `src/Game/Confirmacion.cpp`: la pregunta y los textos;
  - `include/Game/NegocioEstado.h` y `src/Game/NegocioEstado.cpp`: las filas del negocio, el foco y la
    pregunta;
  - `include/HUD/Pergamino/HojaNegocio.h` y `src/HUD/Pergamino/HojaNegocio.cpp`: la hoja;
  - `cmake/Confirmar.cmake` y `tests/Game/ConfirmarTests.cpp`.
- Tocados:
  - `Application.cpp`: solo `confirmPauseCommand`, `confirmBusinessCommand` y la entrada de pausa y
    negocio. Baja de 1347 a 1196 líneas.
  - `PauseMenu.h` y `PauseMenu.cpp`: el modo pregunta.
  - `GameSessionComercio.cpp`: la reventa.
  - `GameSession.h`: una declaración. Sigue con las mismas líneas: he juntado un comentario.
  - **`InterfazPergamino.cpp`, `InterfazPergamino.h` y `ApplicationHud.cpp`**, como Oriol permitió,
    **sin añadir líneas**: el caso Business pinta la hoja, el título de la pausa sale de `PauseMenu`, y el
    cuadro gris y el registro ya no salen debajo de la hoja.
  - `Application.h`: un miembro, en la misma línea que `m_businessMenu`.
  - `ApplicationSceneTests.cpp`: la regla de qué cuadros se ven en modo negocio.
- No se ha tocado nada del combate.

## Comprobado

- **`test_confirmar`** (nueva, 8 casos, gl-free):
  - la tabla de cuándo pregunta la pausa (con y sin partida guardada, con y sin cambios);
  - el modo pregunta de `PauseMenu` (ESC vuelve a la fila de antes);
  - `mismaPartida`;
  - comprar pide confirmación, y «No» no cobra;
  - sin oro, la fila sale apagada y no hace nada;
  - revender: la pregunta, ESC y la pérdida;
  - el alquiler no pregunta y Salir cierra;
  - la hoja pinta dos papiros con la pregunta abierta, y la última fila es el «Sí».
- **GL** (`test_application_scenes`, bajo Xvfb): 12/12.
- **gl-free:** 223/225. Fallan los dos de siempre: `test_marcar_paquetes` y `test_ruta_casillas` (este mide tiempo). Siguen en verde `test_business_mode`, `test_business_economy` y `test_g10_pause_and_hud`.
- `linea_base_core`: 0 de deuda nueva. Ningún fichero tocado pasa de 500 líneas, salvo los que ya lo
  hacían, y esos bajan o se quedan igual.
- **En el juego de verdad** (`juego`, ventana 1280×720 con el GLFW de capturas), en `ciudad_centro` junto
  al cartel de la Sastrería: comprar, revender y volver a comprar, y desde la pausa guardar encima, salir,
  volver al título y cargar. **Después de cargar, «Salir del juego» sale sin preguntar**, porque no hay
  nada que perder. Están en `out/claude_confirmar01/capturas/`, con los guiones que las sacan.
- **El paquete** aplicado sobre los cuatro de antes da exactamente mi árbol. Si se aplica dos veces,
  se niega y no toca nada.

## Cómo se aplica

Después de los cuatro de `REVISAR.md`, desde `MotorGraphico-main/MotorGraphico`:

```
bash out/claude_confirmar01/aplicar.sh
```

Entra **por ancla**: cada trozo de antes tiene que estar una vez en su fichero, y si alguno no encaja, no
toca nada y dice cuál. Lo de antes queda en `out/claude_confirmar01/antes_de_aplicar/`.

## Lo que queda

- **F5 y F9** (guardado y carga rápidos) siguen sin preguntar. Son atajos para quien sabe lo que hace,
  y la tarea hablaba de la pausa. Si Oriol quiere que F9 avise también, es la misma pregunta.
- **La PO-42**: el porcentaje de la reventa.
- **«Sin cambios»** compara la partida de ahora con la guardada, sin la hora. Si un día la partida guarda
  algo que cambia solo (un reloj de juego, por ejemplo), la pausa preguntaría siempre. Entonces habría
  que quitar ese campo en `mismaPartida`.
