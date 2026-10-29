# Canon ACTUAR-01 — Actuar en la calle

Oriol (08/10/2026): «podamos hacer una performance en la calle y depende de nuestra competencia musical y
tiradas, atraer a la gente a nuestro espectáculo».

Código: `include/Game/Actuacion.h` (las reglas) y `src/Game/Actuacion.cpp`; lo llama `GameSession::interact`.
Datos: `tools/actuaciones.py` (los sitios de Boundington) escribe `assets/objects/actuaciones.json` y los
coloca en sus niveles. Formato del bloque: `FORMATO_NIVELES.md` (`actuacion`). Pruebas:
`tests/Game/ActuacionTests.cpp`.

## 1. Cómo se juega

- En algunos sitios de la ciudad hay un **rincón del músico** (una caja en el suelo). Al lado sale el aviso
  **«[E] Actuar junto a la fuente»**.
- Al pulsar, el héroe actúa: una **tirada enfrentada de Interpretación** (la carta `interpretacion`: tira con
  CAR) contra la indiferencia de los que pasan (los dados del sitio). La hoja del diálogo enseña la tirada,
  como las de los beats.
- **Lo que lleva en la mano principal suma dados** si sirve para actuar (la etiqueta «Instrumento» o
  «Malabares»): +1 si es común, +2 si es raro y +3 si es épico o mejor. Sin nada, canta y baila a capela.
  Los nueve instrumentos de INSTRUMENTOS-01, el laúd y el tambor de los bardos y las bolas de malabares.
- **El grado dice cuánta gente se para y cuánto echa**:

| Grado | Quién se para | Oro por cabeza |
|---|---|---|
| Pierdes (o pifias) | nadie (con la pifia, además, se ríen) | — |
| Parcial (empate) | un cuarto de los que pasan | 1 |
| Éxito | la mitad | 2 |
| Crítico | todos | 3 |

  Del parcial para arriba, unos pocos más al azar (hasta un cuarto más).
- **Luego, el sitio descansa**: los que pasan por allí ya te han visto. Hasta que no andas **120 pasos** (el
  tiempo del juego son los pasos), allí no se vuelve a actuar («Date una vuelta y vuelve luego»). Se puede
  ir a otro sitio. Las cuentas son variables del estado narrativo (`actuacion.pasos` y
  `actuacion.<sitio>`): se guardan con la partida, y quien no actúa nunca no las lleva.

## 2. Los sitios de Boundington

| Sitio | Nivel | Celda | Dados del rival | Afluencia |
|---|---|---|---|---|
| Junto a la fuente | `ciudad_centro` | (30, 33) | 3 | 12 |
| A las puertas de la ópera | `ciudad_centro` | (25, 12) | 4 | 16 |
| En la plaza del mercado | `ciudad_centro` | (33, 48) | 2 | 10 |
| En la taberna | `interior_taberna` | (6, 5) | 2 | 6 |

La plaza de la fuente es la de todos; a la puerta de la ópera pasa más gente, pero más exigente; la del
mercado y la taberna, poca y fácil. Cada sitio es un objeto propio (`actuacion_fuente`...) y se dibuja como
la caja de los props (`sprites.json`: el patrón `actuacion_`).

## 3. Cuánto da (simulado)

Oro medio por actuación (200.000 tiradas):

| Dados del héroe | Fuente | Ópera | Mercado | Taberna |
|---|---|---|---|---|
| 2 (CAR 2, a capela) | 6 | 8 | 6 | 3 |
| 3 (CAR 2 y un instrumento) | 7 | 9 | 7 | 4 |
| 4 | 8 | 10 | 7 | 4 |
| 6 (un bardo con un instrumento raro) | 10 | 13 | 9 | 5 |

Con 3 dados en la fuente: pierde 4 de cada 10, parcial 2, éxito 3,5 y crítico 0,5. Es un oficio para ganarse
unas monedas, no una mina: una poción vale 30.

## 4. Lo que es de Oriol y lo que es propuesta

- **De Oriol**: actuar en la calle, que dependa de la competencia musical y de las tiradas, y atraer a la
  gente.
- **Propuesta de Claude** (se cambia sin problema): los cuatro sitios, sus dados y su afluencia, la tabla de
  público y oro, los 120 pasos de descanso, lo que suma cada instrumento y que las bolas de malabares también
  cuenten.

## 5. Lo que falta

- **Que la gente se vea venir**: hoy el público se cuenta en el diálogo («Se forma un corro de 7 personas»).
  Que los PNJ de alrededor se acerquen a mirar pide que se muevan, y hoy los PNJ del mapa están quietos.
- **El Ocaso** (la Matanza): ahora se puede actuar igual. ¿Se cierra?
- Más sitios en los otros barrios (el este, el oeste, Surysal) y en las ciudades de fuera.
