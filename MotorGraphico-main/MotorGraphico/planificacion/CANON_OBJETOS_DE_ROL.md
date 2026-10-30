# Canon INSTRUMENTOS-01 — Los instrumentos y los objetos de rol

Oriol (08/10/2026): «Aprovechamos para crear una variedad de instrumentos musicales? Escobas y otros
objetos que no sean armas pero podamos usar en un juego de rol? Para hacer el tonto, por ejemplo una
almohada, cerveza, una antorcha, estandarte, shirukens, por ejemplo podamos hacer una performance en la
calle y depende de nuestra competencia musical y tiradas, atraer a la gente a nuestro espectáculo, por
ejemplo también una cuerda, hacemos monturas, solo disp fuera de la ciudad después de terminar
boundington, alguna cosa más que veas que pueda ser necesaria en este aspecto?».

Datos: `tools/objetos_rol.py`. Dibujo: `tools/actores_formas_objetos.py` (la forma en la mano) y la
palabra que la elige (`actores_estetica.FORMAS_DE_ARMA`). Los escribe en los catálogos
`tools/importar_objetos_rol.py` (se puede volver a ejecutar).

## 1. Qué son

- **Equipo de la mano principal**: se ven en la mano como un arma, en las cuatro tallas y de frente, de
  perfil y de espaldas. Todos comunes, de tier 1 y de nivel 1, sin bonos (salvo el estandarte: +1 CAR).
  Brillan poco, como todo el equipo de tier 1 (`BrilloEquipoTests`: toda pieza brilla de 1 a 10).
- **Lo que no se coge** (las ganzúas) va solo a la mochila: un objeto de la ciudad sin pieza de equipo.
- **Nadie los lleva al empezar**: se compran en las tiendas de Boundington (§4).

## 2. Los instrumentos (9)

Todos en la taquillera (la de la ópera). Tiran con CAR (`Precisión: CAR`), como el laúd del Juglar, y
llevan la etiqueta **«Instrumento»**, que es la que mira la actuación en la calle
(`CANON_ACTUAR_EN_LA_CALLE.md`). Desde ACTUAR-01 también la llevan el laúd y el tambor de los bardos
(`tools/cp_bardos.py`).

| Instrumento | Oro | Cómo se ve |
|---|---|---|
| Flauta de madera | 30 | Flauta de boj de seis agujeros. |
| Flauta de pan | 35 | Cinco cañas atadas con una tira de cuero. |
| Pandereta | 40 | Parche de piel con sonajas de latón. |
| Lira | 80 | Dos brazos curvos y caja de madera. |
| Trompeta | 90 | De latón, en horizontal. |
| Gaita | 110 | Fuelle de tela roja, tres roncones y el puntero. |
| Vihuela | 120 | Cintura estrecha y boca redonda. |
| Arpa de mano | 130 | Arpa pequeña con cuerdas doradas. |
| Violín y arco | 150 | El violín en una mano y el arco en la otra (a dos manos, `PAREJA`). |

## 3. Los objetos de rol (13) y las ganzúas

| Objeto | Oro | Tienda | Etiquetas | Notas |
|---|---|---|---|---|
| Antorcha | 3 | Posadero | Herramienta | La forma de la tea que ya había. |
| Cuerda de cáñamo | 10 | Armero | Herramienta | Enrollada, en la mano. |
| Pala | 15 | Herrero | Herramienta | A dos manos; se lleva como un bastón. |
| Pico de minero | 20 | Herrero | Herramienta | A dos manos; se lleva como un arma corta. |
| Caña de pescar | 15 | Posadero | Herramienta | Con el sedal y un corcho rojo. |
| Catalejo | 120 | Bibliotecaria | Herramienta | De latón con funda de cuero. |
| Escoba | 5 | Posadero | Arma improvisada | De paja; se lleva como un bastón. |
| Almohada | 6 | Posadero | Arma improvisada | Para dormir o para una guerra de almohadas. |
| Sartén | 10 | Herrero | Arma improvisada | De hierro negro. |
| Jarra de cerveza | 8 | Tabernero | Arma improvisada | En la mano derecha (el puño tapaba la de la izquierda). |
| Bolas de malabares | 20 | Taquillera | Arma improvisada, Malabares | Tres bolas de colores; suman al actuar en la calle, como un instrumento. |
| Shurikens | 60 | Armero | Arrojadiza | El único que es un arma de verdad (`Precisión: DES`). |
| Estandarte de batalla | 80 | Armero | — | +1 CAR, a dos manos; su propio grupo de poses (el asta derecha, la tela arriba y sin tapar la cara). «Estandarte» a secas ya es un mueble (`mobles_kit.json`). |
| Ganzúas | 50 | Armero | — | Solo a la mochila. |

Todos los de la tabla menos los shurikens y las ganzúas llevan además **«Objeto de rol»** (lo que no es
un arma), y todo lo que se coge, la etiqueta de su peso (`Arma Ligera` o `Arma Media`), que es la que deja
equiparlo. Las etiquetas:

- **Herramienta**: sirve para algo fuera del combate.
- **Arma improvisada**: para hacer el tonto.
- **Instrumento**: §2.

## 4. Las tiendas

Nunca las de los tres comerciantes del tutorial (el tendero del mercado, el joyero y el sastre): sus listas
se quedan como las presenta el tutorial (`ObjectCatalogTests`, `ProjectCatalogTests`; lo comprueba
`objetos_rol.revisa()`). El hierro, en el herrero; lo de casa y de camino, en el posadero; lo de
aventurero, en el armero; la jarra, en el tabernero; el catalejo, en la bibliotecaria; los instrumentos y
las bolas, en la taquillera.

## 5. Lo que es de Oriol y lo que es propuesta

- **De Oriol**: una variedad de instrumentos; las escobas, la almohada, la cerveza, la antorcha, el
  estandarte, los shurikens y la cuerda; hacer el tonto; la actuación en la calle (con la competencia
  musical y las tiradas, atraer a la gente); las monturas, solo fuera de la ciudad y al terminar
  Boundington.
- **Propuesta de Claude** (se cambia sin problema): qué instrumentos, los precios, las descripciones y
  las tiendas; la pala, el pico, la caña de pescar, el catalejo, la sartén, las bolas de malabares y las
  ganzúas; el +1 CAR del estandarte.

## 6. Lo que viene

- **ACTUAR-01** (hecho): actuar en la calle (`CANON_ACTUAR_EN_LA_CALLE.md`).
- **MONTURAS-01**: las monturas, fuera de la ciudad cuando se termina Boundington (`bnd3_epilogo`).
