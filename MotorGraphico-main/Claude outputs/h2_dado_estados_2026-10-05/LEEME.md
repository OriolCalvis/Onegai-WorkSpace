# El dado, los estados, los objetivos y el reloj se ven (H2-01, H2-03, H2-02 y H3-06) · 05/10/2026

Después de la barra de cartas, cuatro paquetes de la fase H que se quedaron a medias porque
no había dónde pintarlos. Ahora van en el HUD en pergamino. De paso, «Volver al título» ya
no revienta.

Está **aplicado en el árbol del Mac, sin commit**. El registro de la sesión está en
`sesiones/2026-10-05-claude-dado-estados.md`, y el seguimiento en
`planificacion/seguimiento/H2-01-DADO.json`, `H2-03-CONDICIONES.json`,
`H2-02-OBJETIVOS.json` y `H3-06-TIMER.json`.

## Qué mirar

- `combate_dado_y_estados.gif`: un combate de principio a fin, con la tirada, el estado en
  el saqueador, el de Nerea, los dos emergentes y el final.
- `capturas/`:

| | |
|---|---|
| 01–04 | El panel de la última tirada: la tuya (con su halo, recién salida), la del rival, una carta que queda en parcial y el registro oculto |
| 05–07 | La prueba de Percepción del prólogo: la primera página sin dados, la de «(Tirada de...)» con la tira, y el resultado ya sin ella |
| 08–13 | Los estados: Canción del Otoño en la placa (2), una ronda después (1), Invocación Elemental bajo el retrato de Nerea, los emergentes y el final sin estados |
| 14–17 | Detalles ampliados: los paneles, la tira del diálogo, la placa y el contador del héroe por ronda |
| 18–22 | A quién apunta la carta: Atacar marca al saqueador, una carta tuya te marca a ti, un área marca a todos, un clic en la placa la juega, y la retícula ampliada |
| 23–26 | El temporizador de misión: en el retal del objetivo; 0:24 → 0:19 explorando y 0:18 tras 6 s de combate (quieto); la cuenta hasta 0:00, en rojo el último cuarto; y el beat del final como diálogo |

## Cómo se lee

- **Los dados** cuentan como el reglamento:
  - el 6 en verde, un éxito;
  - el 5 en ámbar, medio;
  - con ventaja, el 4 también en ámbar;
  - con desventaja, el 5 no cuenta.
- **La pifia** se ve de lejos: los 1 en rojo, el panel con marco rojo y «¡PIFIA!» en una caja
  roja. Perder la tirada da «FALLO», que no es pifia.
- **Los estados** van en una ficha con su icono, el borde rojo si te perjudican (verde si te
  ayudan) y las rondas que le quedan en la esquina. El número baja cada ronda.
  - Los del enemigo van en su placa, con el nombre.
  - Los tuyos, bajo tu retrato en el orden de turno.
  - Con el ratón encima sale qué hace.
- **Los objetivos**: la carta señalada pone una retícula carmín (cuatro esquinas) en la
  placa y en el retrato de quien alcanzaría.
  - W/S, las flechas arriba/abajo o el ratón eligen a otro.
  - Un clic sobre el marcado juega la carta.
  - Nada se gasta hasta jugarla: mirar otra carta es volver atrás.
- **El temporizador de misión** va en el retal del objetivo: «TIEMPO», lo que queda y la
  barra, en rojo el último cuarto.
  - Solo corre explorando: en combate, en un diálogo o con 15 s sin tocar nada se queda
    quieto.
  - Al acabarse, sale el beat de la aventura como diálogo.
  - Para la prueba usé una copia del prólogo con un reloj de 25 s en barra, solo en el
    contenedor. El reloj de verdad del prólogo es secreto y no se ve.

## Para Oriol

1. **Con los dados a la vista se nota el principio del juego.**
   - Nerea recién creada (base 0, sin el kit) ataca con 1 dado.
   - El saqueador defiende con 0 dados y ataca con 1.
   - Los primeros combates son casi a cara o cruz.
   - No lo he tocado: ¿lo dejamos así, o un mínimo de dados para defender?
2. Cuando una acción tira varias veces (un ataque de área), solo se ve la última tirada. El
   registro las tiene todas.
3. Las condiciones que duran más allá del combate todavía no se ven al explorar.
4. Las tildes que faltan («Percepcion», «Cancion del Otono») vienen de los catálogos.
5. Los combates del juego todavía son de uno contra uno. Elegir objetivo ya funciona con
   varios enemigos (probado con dos), pero solo se notará cuando un combate junte a más de
   uno.
6. Los temporizadores no tienen nombre en el JSON, así que el retal dice «TIEMPO».
   - ¿Le añado un campo «nombre» opcional («Antes de que caiga la torre»)?
   - Hoy ninguna aventura tiene uno visible: el del prólogo es secreto.
