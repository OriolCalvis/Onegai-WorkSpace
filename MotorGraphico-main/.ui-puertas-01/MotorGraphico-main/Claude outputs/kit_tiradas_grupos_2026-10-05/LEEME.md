# El kit a 2, 2, 1, 1, el nombre de los relojes, todas las tiradas y los combates en grupo · 05/10/2026

Lo que pediste al ver los dados. Está **aplicado en el árbol del Mac, sin commit**. El registro
de la sesión está en `sesiones/2026-10-05-claude-kit-tiradas-grupos.md`, y el seguimiento en
`planificacion/seguimiento/KIT-TOPES-01.json` y `COMBATE-GRUPO-01.json`.

La revisión de la campaña (para jugarla entera, con las submisiones de reputación) va en un
documento vivo aparte: «Los Perdidos de Boundington — revisión para jugarla entera».

## Qué mirar

- `tiradas_en_cola.gif`: dos devotos, Canción del Otoño con su «1/2» y su «2/2», y la tirada del
  rival después. El héroe cae en ese golpe: es un crítico de 22.
- `capturas/`:

| | |
|---|---|
| 01 | El reloj con nombre: «El vacío se cierra 0:39» (detalle en la 13) |
| 02–03 | El cofre con las cinco piezas del bardo y la ficha: CON 4 (3 de la raza y 1 del equipo), DES 2, INT 2 (1 y 1), CAR 2 |
| 04–07 | Dos devotos: las placas, la tirada del área (1/2 y 2/2) y la del rival (detalle del panel en la 12) |
| 08 | Cinco devotos: las placas compactas, con su número siempre a la vista (detalle en la 14) |
| 09–11 | Tres saqueadores: Nerea cae en la primera ronda |

## Qué ha cambiado

1. **El kit del cofre suma exactamente +2 al principal, +2 al secundario y +1 a los otros dos**,
   en las 63 clases y sin requisitos. La raza va encima. El cofre lleva hasta cinco piezas.
2. **Topes de dados por tier**, solo para el equipo:
   - el atributo más alto llega como mucho al tope;
   - el segundo, a 3/4 del tope más uno;
   - los otros dos, a la mitad.

   Con el tope de Tier 1 (4), el mejor equipo da 4, 4, 2 y 2: nunca tres arriba. La ficha recorta
   lo que se pase. **Los topes 2, 4, 6, 8, 8 y 8 son una propuesta mía.**
3. **Los temporizadores pueden tener nombre** (`"nombre"` en la aventura). El HUD lo enseña en
   lugar de «TIEMPO».
4. **Todas las tiradas se ven**, una detrás de otra, 1,6 segundos cada una:
   - el panel dice «2/3» si la acción tiró varias veces;
   - el rival no actúa hasta que se han visto, y ganar o perder tampoco se aplica antes.
5. **Combates en grupo:**
   - una aventura puede pedir varios enemigos;
   - si se repite un nombre, van numerados;
   - al ganar caen todos; al huir se quedan los vivos;
   - con muchos enemigos, las placas van compactas.
6. **El Ocaso pelea como en la historia**: los hombres de Venides retienen a los Perdidos.

   | | Con 20 hombres | Con 10 | Solo |
   |---|---|---|---|
   | La puerta del cuartel | 1 devoto | 2 devotos | 3 devotos |
   | Los fanáticos | 1 | 1 | 2 |
   | Los captores de Verina | el carcelero | el carcelero | carcelero y devoto |

   El devoto es nuevo: la ficha Cultist de PARTIDAS.docx, con 9 PV.

## Para Oriol

1. **Los combates en grupo matan al héroe solo.** Simulé 2.000 combates con la mesa de verdad
   (un bardo de Tier 1 con el kit, 24 PV, que solo usa Atacar):

   | Contra | Gana |
   |---|---|
   | 1 saqueador | 82–89 % |
   | 2 saqueadores | 17–35 % |
   | 3 saqueadores | 0–3 % |
   | 2 devotos | 78–89 % |
   | 3 devotos | 36–62 % |

   Por eso el Ocaso depende de los hombres de Venides. La historia está pensada para varios
   aventureros: **¿tendrá el héroe compañeros en el combate?**
2. **El crítico de «todos 6» con un dado.** Un enemigo sin atributos tira un dado, saca un 6 una
   vez de cada seis, y si gana es crítico ×1,5: 22 de daño. ¿Pedimos al menos dos dados para ese
   crítico?
3. **Ponerse el kit no cura:** sube la vida máxima pero no la actual. Nerea entra en su primer
   combate con 19/24. ¿Lo curamos al ponerse el kit?
4. **El equipo de Tier 1 todavía se pasa de los topes.** Deja juntar INT 8 o CAR 8 (en partida se
   recorta a 4), y el 4, 4, 2, 2 no se alcanza con las piezas que hay. Si quieres que el mejor
   reparto exista y sea muy difícil, hay que rehacer piezas.
5. Las tiradas se ven todas, pero la vida baja al resolverse la acción, no al ver su tirada. Con
   muchas tiradas lo sincronizaremos, como dijiste.
