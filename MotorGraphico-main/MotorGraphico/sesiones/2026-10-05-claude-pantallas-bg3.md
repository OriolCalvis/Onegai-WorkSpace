# Pantallas en cuadrícula (maquetas) · 05/10/2026

Sesión de Claude (Cowork), después de H2-06. Encargo de Oriol:

- Un cofre en El Vacío con el equipo básico de la clase, para empezar con puntos en
  sus características.
- Los menús siguen en el estilo antiguo y en lista. Prefiere cuadrículas con ratón o
  teclado; BG3 como ejemplo.
- ¿Cuándo se ven las cartas en combate? El HUD «es una lista de la compra».

Eligió:

- El kit va a la mochila.
- Pergamino con la distribución de BG3.
- Barra de cartas abajo.
- Maquetas antes de programar.

## Qué se hizo

**No hay código del juego cambiado.**

- Diez maquetas a 1280×720 y un lienzo de diseño en claude.ai con las maquetas y las
  capturas de hoy para comparar.
- Las maquetas están en `Claude outputs/pantallas_bg3_2026-10-05/`, con su LEEME.
- El generador, `bg3.py`, está junto al del estudio de UI
  (`estudio_uiux_2026-09-28/maquetas/_generador/pergamino/`).
- Los fondos son capturas del juego real sin HUD (plaza de Boundington y El Vacío con
  un cofre), sacadas con un interruptor local que no se entrega.
- Los retratos salen de `razas_andando.png` y de `tools/probador_personaje.py`, y los
  textos, del catálogo.

Seguimiento: `planificacion/seguimiento/UI-BG3-01.json` (PROPUESTA).

## Lo que encontré por el camino

- Solo 44 de los 63 kits iniciales dan bono a la característica principal de su
  clase.
  - El Bardo (CAR) recibe DES.
  - La cota de malla de la Furia pide CON 1, que solo da la propia cota.
  - El cofre necesita kits revisados: sin requisitos y con +1 en la principal y la
    secundaria.
- El juego no tiene ratón (`InputSystem` solo conoce teclas).
- El catálogo de cartas perdió las tildes («Cancion del Otono»): las maquetas las
  ponen a mano.
