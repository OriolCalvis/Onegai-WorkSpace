# Creador de apariencias en el juego · APARIENCIA-01 · 29/09/2026

El probador de personajes (ARTE-02) ya está dentro del juego. En **Nueva
partida** hay un paso nuevo, **«Qué aspecto tienes»**, justo después de la
raza, y una **vista previa del personaje andando** a la derecha de todos los
pasos. El personaje que creas es el que camina por Boundington, y lo que
equipas se le ve encima.

## Cómo probarlo en el Mac

1. Compilar como siempre (hay una librería nueva, `motor_imagen`, y dos
   pruebas nuevas: `test_personajes_capas` y `test_editor_apariencia`).
2. `./juego` → **Nueva partida**.
3. **Paso 1, raza**: al moverte con W/S, a la derecha anda cada pueblo.
4. **Paso 2, apariencia**: W/S cambia de fila y **A/D (o las flechas)** cambia
   el valor. Filas: piel y talla (solo pueblos de cuerpo liso), ojos, peinado y
   su color, barba y su color, cabeza, torso, piernas, pies, capa, **AL AZAR** y
   **COMO TU PUEBLO**.
5. **Paso 3, clase**: la vista previa lleva puesto el equipo inicial de la
   clase marcada.
6. **Paso 5, reparto de stats**: prueba el Coloso de Ceniza con CON 1 y luego
   con CON 3: la armadura de placas aparece cuando la carta la permite.
7. **Resumen**: dice con qué empiezas puesto. Enter y a jugar.
8. En partida, **TAB** abre el cuaderno de pergamino: **→** pasa a la hoja
   **Equipo**, **S** baja al torso y **Enter** lo desequipa: la armadura va a
   la mochila y se ve la ropa de diario que elegiste. En la hoja
   **Inventario**, Enter la vuelve a poner.

## Qué hay aquí

| Fichero | Qué es |
| --- | --- |
| `creacion_apariencia.mp4` / `.gif` | 34 s del juego real: razas, apariencia, al azar y clases con su equipo. |
| `capturas/` | 36 capturas a 1280×720 del recorrido completo, de la raza al jugador vestido en la ciudad. Las 1-31 son de antes del HUD de pergamino; las 32-34, con el cuaderno nuevo. |
| `jugador_en_partida_zoom.png` | El jugador en partida ampliado: los cuatro lados, sin armadura y con ella. |
| `jugador_con_pergamino_zoom.png` | Ya con PERGAMINO-01: con armadura, desequipada desde el cuaderno y equipada otra vez. |
| `apariencia01.patch` | Todos los cambios del código como parche, rehecho sobre el árbol del Mac del 29/09 a las 18:00, con PERGAMINO-01 dentro (ya están aplicados en el repo; es copia de seguridad). |

Las capturas y el vídeo salen del juego compilado en Linux con OpenGL por
software (las mismas herramientas que las capturas de menús del 28/09).

## Decisiones que tomaste

- El paso de apariencia va **después de la raza** (el cuerpo sale del pueblo).
- El **equipo inicial empieza puesto**: se equipa lo primero de cada ranura
  que la clase pueda llevar; lo demás se queda en la mochila.

## Lo que conviene saber

- **Escudos**: en `equipment.json` los escudos, orbes y focos van en la ranura
  `MainHand`, así que no conviven con el arma. El caballero templario empieza
  con la espada puesta y el escudo en la mochila. Es una decisión de canon
  (¿moverlos a `OffHand`?).
- **Guardar**: la apariencia no se guarda todavía, igual que la ficha
  (`SaveGame` no la serializa). Es lo siguiente que haría.
- **Arreglos de paso**: la creación ya no se queda congelada en 1/14; el
  título y las pistas ya no se parten palabra a palabra; A/D funcionan además
  de las flechas; y el jugador ya no se queda de espaldas al pararse.
- **Visto al probar**: al quitarte la armadura, los PV no bajan al nuevo
  máximo (sale 35/29 hasta que te la vuelves a poner). Es del desequipar, no
  del aspecto, pero con el equipo puesto de salida se ve enseguida.
- **Commit**: todo está en el árbol, sin commit. `cmake/AparienciaTests.cmake`
  tiene que ir en el mismo commit que su `include()` en `CMakeLists.txt`.
