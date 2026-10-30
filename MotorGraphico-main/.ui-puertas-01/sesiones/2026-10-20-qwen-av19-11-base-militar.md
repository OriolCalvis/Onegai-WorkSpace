# 2026-10-20 · qwen · AV19-11 la Base Militar, el patio donde empieza el Ocaso

Cola del orquestador (`orquestador/qwen.md`, tarea 3 de 6, la segunda del orden 1 → 3 → 2 → 4 → 5 → 6).
Rama **`agente/qwen/AV19-11`**, encadenada sobre `agente/qwen/AV19-09` (`18fec4e`). Mismo worktree
(`~/onegai-ramas/qwen-AV19-09`) y mismo build: el disco está al 95 % y seis worktrees no caben (ver la
sesión de AV19-09).

## Qué se hizo

**`base_militar` (36×32, exterior)** con `tools/av19_base_militar.py` (nuevo), que reutiliza la clase
`Nivel` de `gen_niveles_av19.py` —la misma que ya escribe TMX, comprueba las casillas de llegada de las
puertas y declara las regiones de altura— y los escritores de catálogo/manifiesto/lugares de
`av19_ocaso.py`. 28 objetos, 888 casillas pisables, 3 regiones de altura, inicio (18,29) dentro del
portón.

El recinto: anillo de `viejo_muralla` con **portón doble** al sur (17-18,30), salida al **oeste** (1,16)
y salida al **norte** (10,1); fuera del anillo, `roca`, para que nadie salga a pasear por la grava del
borde del mapa. Patio de `patio_fortificado` con sus dos caminos de adoquín (del portón al centro y el
transversal de la instrucción).

- **Barracones** al oeste: el tejado es una **meseta a altura 1** (x 4..11, y 4..11). El motor no apila
  casillas, así que un bloque no puede ser a la vez barracón que bloquea y tejado pisable: se sube por
  cualquier lado (un nivel de diferencia se pasa, y en combate cuesta el doble) y la pared la dibuja solo
  la altura, que es lo que dice FORMATO_NIVELES §Altura.
- **Torre de vigía con campana** al norte: zócalo de `viejo_militar` que bloquea (x 16..20, y 3..7) y
  **adarve a altura 2** (x 17..19, y 4..6), con el hueco de la puerta (18,7) y su peldaño (18,8) a
  altura 1. La campana (`campana_torre`, flotante) en (19,5).
- **Patio**: pozo `pozo_o1` (14,18), abrevadero (24,12), tres caballos (26-27,13-14), cuatro armeros con
  armas, tres muñecos de instrucción (12-16,22), cuatro banderas, la **forja del herrero militar**
  (28,20) con su yunque (29,20), y el carro con el que entra Aigren (19,27).
- **Muralla trasera baja**: tres casillas al este (34,14-16) con `muralla_baja`, que no bloquea: por ahí
  saltan los tres saqueadores.

**Alturas ya las lee el motor.** En AV19-03 el campo `alturas` existía pero el binario lo ignoraba; ahora
`LevelLoader`, `GameSession`, `GameSessionTablero` e `IsometricRenderer` lo usan (AV19-02 entró), así que
el tejado y la torre salen de verdad, no como promesa.

## Los pasos

| puerta | casilla | destino | llegada |
|---|---|---|---|
| `porton_base_militar` (doble) | (17,30) (18,30) | `ciudad_oeste` | (8,12) |
| `salida_base_al_oeste` | (1,16) | `ciudad_centro_ocaso` | (3,12) |
| `salida_base_al_norte` | (10,1) | `ciudad_centro_ocaso` | (8,3) |
| `puerta_cuartel_patio` | (12,10) | `interior_cuartel` | (2,10) |
| `puerta_base_militar` (en el Casco) | (8,11) | `base_militar` | (17,29) |

Y dos puertas de siempre **redirigidas** (el interior se conserva y pasa a abrirse desde el patio, como
pedía la ficha):

- `ciudad_oeste`: `puerta_cuartel` (8,11) ya no entra en `interior_cuartel`, entra en `base_militar` (17,29).
- `interior_cuartel`: `salida_interior_cuartel` (1,10) ya no sale a la calle, sale al patio (8,13).

**La puerta de vuelta al Casco no la podía poner el Excel**: `base_militar` no existía cuando Oriol
dibujó. Sin ella, `validar_puertas` avisa («no tiene puerta de vuelta») y, peor, `validar_mundo_unido`
dice **«sin retorno al mapamundi» de los tres mapas del Ocaso**: la base los mete en el grafo del mundo
al apuntar a ellos. Va en `PUERTAS_EXTRA` de `av19_ocaso.py`, en la casilla (8,11), que es justo donde el
mapa de día tiene `puerta_cuartel`.

## El patio de armas es uno solo

La ficha preguntaba si el castillo y la base comparten patio. **Sí, y es este**: la escena del castillo
del día 2 (herrero militar herrando, soldados con lanzas de madera, abrevadero) se juega en
`base_militar`, que existe igual los tres días. Aquí están la forja con su yunque, el abrevadero y cuatro
casillas reservadas para los soldados con lanzas. AV19-13 solo tiene que añadir en `interior_castillo` la
puerta que conecta, si Oriol quiere que también se entre desde dentro.

## Marcadores nuevos (12 ids en `boundington_av19_objetos.json`)

Puertas: `porton_base_militar`, `salida_base_al_oeste`, `salida_base_al_norte`,
`puerta_cuartel_patio` (más `puerta_base_militar`, que pone `av19_ocaso.py`). Piezas que esperan arte:
**`muralla_baja`**, **`abrevadero`**, **`caballo`**, **`armero_armas`**, **`muneco_instruccion`**,
**`campana_torre`**, **`forja_militar`**, **`lanza_madera`**.

## Tres fallos que encontró la primera pasada (la guarda, no yo)

1. **La torre y el tejado se solapaban**: la torre en x 16..20 y el tejado en x 13..17 se comían dos
   columnas, y la casilla reservada para un fanático (16,6) caía dentro del zócalo que bloquea. El tejado
   ha pasado a ser la meseta encima de los barracones y la torre se queda sola al norte.
2. **Una bandera tapaba a un soldado**: las banderas estaban en (13,13) y (23,13), justo donde van las dos
   filas de diez. Movidas a (12,13) y (24,13).
3. **La guarda de acantilados era demasiado estricta**: denunciaba el adarve a nivel 2 rodeado de zócalo a
   nivel 0, que es exactamente lo que se quiere (ahí el motor dibuja la pared). Ahora solo cuenta entre
   casillas que **se pisan** las dos.

## Una trampa desactivada de paso

`tools/av19_niveles.py::NIVELES_AV1906()` devolvía `[ocaso()]`, o sea el `ciudad_centro_ocaso` de 64×64
de AV19-06. Cualquiera que ejecutara `gen_niveles_av19.py` (que es lo que se ejecuta para regenerar los
niveles de la cola) **reescribía el mapa de Oriol de 96×64 y sus 1145 objetos con la versión vieja**, sin
fallar ni avisar. Ahora devuelve `[]` con el motivo escrito; `ocaso()` se queda como histórico.

## Verificación

- `validar_mundo_unido.py`: **todo se alcanza y de todo se vuelve** (117 niveles, 369 puertas).
- `validar_puertas.py`: 127 niveles, 371 puertas, **0 problemas nuevos** (los 12 avisos son los de
  siempre, ninguno de mis niveles).
- `conectividad.py`: `base_militar` 888/888 (100 %), **0 errores** en el repo.
- `validar_todo.py`: **2 errores, los dos heredados** (los PNJ de la Barriada sin fila propia y la daga de
  Aigren), igual que en AV19-09 y que en la base.
- Guardas propias del generador: ni una reserva en colisión ni tapada por un objeto, ni dos objetos en una
  casilla, ningún acantilado entre casillas pisables, y flood fill **contando la altura** desde el inicio:
  las 13 reservas, las 20 casillas de soldados, las 4 de lanzas y las 5 puertas se alcanzan todas.
- Captura: `out/planos/base_militar.png`.
- `ctest -L gl-free`: ver el INFORME (la línea base de `bfc5a22` son 14 rojos).
- **No lo he jugado en el motor**: las alturas deberían verse (el motor ya las lee), pero la comprobación
  es la de los validadores y el flood fill, no una captura del juego.
