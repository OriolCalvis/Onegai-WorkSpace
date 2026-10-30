# Pantallas en cuadrícula · maquetas · 05/10/2026

Propuesta para validar antes de programar, como se hizo con el pergamino. No hay
código del juego cambiado.

Encargo de Oriol (05/10):

- Los menús siguen en el estilo antiguo y en lista. Los quiere en **cuadrícula**,
  que se manejen con **ratón o teclado**, con BG3 como ejemplo.
- En combate, ¿cuándo se ven las cartas? El HUD de combate es «una lista de la
  compra, no un HUD de un videojuego».
- Un **cofre en El Vacío** (la primera escena) con el equipo básico de la clase,
  para empezar con puntos en las características que la clase necesita.

Lo que eligió:

- El cofre deja el kit **en la mochila**.
- Estilo **pergamino con la distribución de BG3**.
- **Barra de cartas abajo** en el combate.
- **Maquetas primero**.

## Qué hay aquí

| Fichero | Qué enseña |
| --- | --- |
| `C01_titulo.png` | El título en papiro. «Continuar» apagado lleva su motivo debajo. |
| `C02a_creacion_raza.png` | Cuadrícula de las 43 razas con su retrato. El personaje va en el centro con sus atributos (base 0 + raza) y la ficha de la raza a la derecha. |
| `C02b_creacion_clase.png` | Filtros por papel y cuadrícula de clases, con el atributo principal en la esquina. El personaje lleva el kit del cofre. La ficha enseña la pasiva y la mano de cartas (7 + 3 de creador). |
| `C02c_creacion_historia.png` | Las siete preguntas del trasfondo en una sola hoja, cinco tarjetas por fila. Al pasar el ratón sale la ficha emergente. |
| `C02d_creacion_nombre.png` | Nombre con sugerencias, resumen con «editar» en cada elección y Empezar la aventura. |
| `C03a_combate_tu_turno.png` | Barra de cartas (1-0 o clic). La marcada se amplía a la izquierda. Encima, lo que queda del turno. También el orden de turno, la placa del enemigo y el registro. |
| `C03b_combate_estacion.png` | El Bardo elige estación al empezar el combate. |
| `C03c_combate_reaccion.png` | En el turno enemigo la barra se apaga y la reacción preparada salta sola. |
| `C04a_vacio_cofre.png`, `C04b_vacio_cofre_abierto.png` | El personaje empieza sin equipo. El cofre deja el kit en la mochila y da el objetivo de ponérselo. |
| `hoy/` | Capturas del juego actual, para comparar. |

Los fondos son capturas del juego real sin HUD: la plaza de Boundington y El Vacío
con un cofre. Los retratos salen de los atlas del juego (`razas_andando.png` y el
montador de `tools/probador_personaje.py`). Los textos salen del catálogo.

## Lo que cambia respecto a hoy

- **La creación pasa de 14 pasos a 7 hojas del cuaderno**: Raza, Aspecto, Clase,
  Origen, Historia, Deidad y Nombre. Se cambia de hoja con Q/E o con un clic en la
  pestaña. Origen y Deidad usan la misma cuadrícula que Raza.
- **Todo se puede hacer con ratón o con teclado.** Pasar el ratón por encima enseña
  la ficha y un clic elige. Con teclado, WASD mueve el foco y ENTER elige. El juego
  hoy no tiene ratón: hay que añadirlo a `InputSystem`.
- **Las cartas se ven todo el combate**, en una barra abajo y no como filas de un
  menú. Las gastadas salen en gris, las de tier 2 apagadas y la reacción preparada
  brillando.

## Para regenerarlas

El generador está junto al del estudio de UI:

    Claude outputs/estudio_uiux_2026-09-28/maquetas/_generador/pergamino/bg3.py

Se ejecuta desde esa carpeta con `python3 bg3.py` (o solo algunas: `python3 bg3.py
C02 C03`). Necesita Playwright y Pillow.

- Estilos: `fuente/sistema/bg3.css`.
- Datos extraídos del catálogo: `datos/catalogo.json`.
- Retratos: `fuente/img/sprites/`.
- Fondos nuevos: `fuente/img/fondos/fondo_{combate_plaza,titulo_ciudad,vacio_cofre}.png`.

A `lib.py` solo se le añadió el glifo del rayo de las reacciones.

## Pendiente de decidir

1. Si se aprueba la dirección: cuadrícula, pergamino y distribución tipo BG3.
2. Los 14 pasos en 7 hojas, con las siete preguntas de la historia juntas.
3. **El kit de cada clase**. Hoy solo 44 de los 63 kits iniciales dan bono a la
   característica principal de su clase.
   - El Bardo (CAR) solo recibe DES.
   - La cota de la Furia pide CON 1, que solo da la propia cota.
   - El kit del Bardo de las maquetas es una propuesta: sable, armadura de cuero
     (+1 DES) y amuleto de foco (+1 CAR), todo sin requisitos.
   - Hay que decidir si los 63 kits salen de una regla (un arma de la clase y piezas
     con +1 en su principal y su secundaria) o de una lista hecha a mano.
4. Qué pasa con las cartas fuera del combate: la hoja Acciones del cuaderno puede
   enseñar el mazo.
