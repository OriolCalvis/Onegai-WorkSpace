# INT-02 — Los interiores del este amueblados (y el viajero de la plaza)

Oriol, 05/10/2026, sobre lo que quedaba pendiente de PNJ-02:

> 1. La frase del yokai de la plaza («He visto caer tres ciudades. Todas olían así la
>    víspera») se fue con él. ¿Quieres que la diga otro? Que lo susurre Chronos
>    disfrazado de mortal.
> 2. Amueblamos las zonas.

Y lo que eligió: **un viajero anónimo** (nadie dice que sea Chronos), **los 33 interiores
del este** y **un toque por nación**.

## Láminas

| Fichero | Qué es |
| --- | --- |
| `chronos_01_plaza_viajero.png` | La Plaza de las Razas en el juego, con el viajero al borde, y de cerca. |
| `int2_01_piezas_nuevas.png` | Las piezas nuevas del kit: diez muebles y cinco paredes. |
| `int2_02_antes_ahora_grytoz.png` | La taberna de Grytoz antes (la caja vacía con el tendero) y ahora. |
| `int2_03_tabernas.png` | Las 15 tabernas. |
| `int2_04_mercados.png` | Los 10 mercados. |
| `int2_05_herrerias_y_locales_propios.png` | Las 4 herrerías y los 4 locales propios de las capitales. |
| `int2_06…13_de_cerca_*.png` | De cerca: la taberna-teatro de Qethatos, la de Guskedor (la mesa de los clanes), la de Klimnebra (perchas y cartas), la de Venordemn (espejos y cristales), la Posada del Faro, el Archivo de la Bruma, la Arena de clanes y la Casa de fletamento. |

Las capturas son del juego (`juego --nivel`), sin el jugador ni el HUD.

## 1. El viajero de la plaza

- **Quién:** «Viajero de paso» (`chronos_disfrazado`), un viejo con capucha y hábito
  grises, ojos dorados, barba blanca, bastón de roble y un **reloj de arena** en la
  mano izquierda. Nadie lo llama Chronos: está en la ficha (`_nota`), no en el juego.
- **Qué dice:** «(Susurra, sin mirarte.) He visto caer tres ciudades. Todas olian asi la
  vispera.» (ASCII, como el resto de la plaza).
- **Dónde:** al borde de la plaza (14, 10), de paso. Es opcional: un beat suyo
  (`beat_plaza_viajero`, flag `plaza_hablado_chronos`) que no cuenta para cerrar la
  plaza, que se sigue cerrando con los once pueblos.
- **El reloj de arena** es una pieza nueva solo de PNJ (`reloj_arena`, mano izquierda,
  `"soloPnj": true`): la creación no la ofrece.
- `demo_plaza_razas`: 14 beats; habla primero con el viajero y comprueba la flag y que la
  plaza no se cierra por eso. PNJ: 249 en 325 aspectos (uno más).

## 2. Los 33 interiores del este

Antes eran cajas de 13×11 con el tendero solo. Ahora son salas del kit de INT-01 (las dos
paredes del fondo, el frente abierto, suelos y alfombras, muebles en las 4 orientaciones)
con el tamaño de su asentamiento y el toque de su nación.

### El toque de cada nación

| Nación | Paredes y suelo | Lo que la distingue |
| --- | --- | --- |
| **Gongorguma** (orcos) | piedra cálida con **máscaras de teatro**, escudos, armas, antorchas; tierra batida | «Aquí se bebe de pie»: mesas largas sin asientos y pocas mesas con dos taburetes; **estandartes, tambores de guerra, tótems de huesos**, braseros. En Guskedor, la mesa de guerra de los jefes de clan al fondo («si buscas a un clan, pregunta al fondo»); en Qethatos, **la taberna con escenario**. |
| **Choubar** (hombres pájaro, mercaderes del río) | madera con **cuerdas de flete** y cartas de navegar; tablones | **perchas** en lugar de sillas, la **mesa de cartas** del flete, rollos de cuerda, cabrestantes, cajas y equipajes. |
| **Mistarium** (tieflings de la bruma) | estuco azul en los pueblos, papel noble azul con **espejos** en ciudades y capital; espiga y mármol | sillas nobles rojas, **espejos de corte**, **cristales de bruma** que humean, libros que flotan, lámparas de cristal; en el mercado, «poco y caro»: vitrinas en lugar de puestos. |
| **Nocturnsea** (Grytoz, los muertos) | piedra oscura con **telarañas**, nichos, rejas y vitrales; losas oscuras | **velas derretidas**, huesos, candelabros, el reloj parado; ningún hogar (la sopa, al asador); en la posada, **ataúdes por cama**. |

### Las salas

| Asentamiento (tier) | Locales |
| --- | --- |
| Guskedor (capital, Gongorguma) | taberna 14×11 · mercado 16×12 · herrería 11×9 · **Arena de clanes** 18×13 (sala alta: el foso de arena, bancos, braseros, jaulas, muñecos) |
| Qethatos (ciudad) | **taberna-teatro** 16×12 · mercado 14×11 |
| Ciudad XIII, XIV, XV (ciudad) | taberna 13×10 · mercado 14×11 |
| Pueblo VI, VII (pueblo) | taberna 11×9 |
| Klimnebra (capital, Choubar) | taberna · mercado · herrería (ladrillo con cuerdas) · **Casa de fletamento** 14×10 (mostrador con reja, caja fuerte, mesas de cartas) |
| Venordemn (capital, Mistarium) | taberna · mercado · herrería («hierro de las ruinas»: columnas rotas, vigas) · **Archivo de la Bruma** 17×12 (sala alta: estanterías, espejos enfrentados, la mesa de los mapas) |
| Ciudad, Ciudad I (ciudad) | taberna · mercado |
| Pueblo, Pueblo I, Pueblo II (pueblo) | taberna |
| Grytoz (capital, Nocturnsea) | taberna · mercado · herrería (arpones, cadenas) · **Posada del Faro** 15×11 (siete ataúdes, el reloj parado) |

En total, 1071 muebles. Los locales hermanos de una misma nación no salen iguales: hay
dos plantillas por tamaño y cada una también **en espejo** (x↔y: la puerta en la otra
pared y los muebles girados). Cada tendero sigue siendo el que era
(`en_<nación>_<local>_tendero`), detrás de su mostrador; la puerta de la ciudad lleva ahora
a la estera de la entrada.

### Cómo se rehace

```
python3 tools/iso/fer_interiors.py en          # todo el kit y los 33 del este
python3 tools/gen_interiores_kit.py en         # solo las salas (en_grytoz: las de Grytoz)
```

- Los planos: `tools/interiors_kit/este.py` (el kit de cada nación y el `Pla` que pone una
  sala en espejo) y `tools/interiors_kit/este_sales.py` (las plantillas y cuál va en cada
  local). El generador comprueba que de la entrada se llega a todo y al tendero.
- Las piezas nuevas: `tools/iso/mobles_nacions.py`; las paredes nuevas, en
  `tools/iso/sales.py` (telaraña, espejo, máscaras, cuerdas).
- `tools/gen_este_norte.py` (el generador del cuadrante) escribía estos interiores
  vacíos: si se vuelve a correr, después hay que correr `gen_interiores_kit.py en`. **Ojo:**
  hoy ya no corre (`gen_ciudad.py` ya no tiene `PIEDRA_VIEJA`); no es de este encargo.

## 3. Las sombras de los muebles (INT-03)

Oriol: «en los muebles las sombras hacen que parezca que el objeto esté volando».
El motor ponía a cada objeto una mancha redonda (la sombra «blob») en la punta de
delante del rombo, donde se apoyan los pies de un personaje. Un mueble ocupa todo el
rombo: la mancha quedaba delante y por debajo de su base, separada, y parecía que flotaba.

- **Ahora** la sombra va dibujada en la hoja, debajo de cada pieza: la planta de la
  pieza (lo que tapa desde arriba) sobre el suelo, un píxel más ancha y un poco hacia la
  derecha (la luz del kit viene de la izquierda), recortada a su rombo. Las plantas sin
  caras (copas, matas) la hacen con una elipse del ancho del dibujo; las piezas planas
  (estera, flores) no tienen. Muebles, decorados de la calle y vegetación, todos igual.
- **El motor** ya no les pone la mancha: los manifiestos del kit dicen
  `"sombraBlob": false` (`HojaSprites::sombraBlob` → `Entity::setSombraBlob`). Las
  paredes, tampoco. Los personajes y los props de PixelLab siguen con la suya.
- `tools/iso/peces_iso.py` (`Escena.ombra`), `exportar_mobles.py`, `exportar_decorats.py`
  (ya no mueve sus hojas al final de `sprites.json`); `test_registro_sprites` y
  `test_render_static_entity` lo comprueban.

| Fichero | Qué es |
| --- | --- |
| `int3_sombras_antes_ahora.png` | La taberna de Guskedor: la mancha del motor y la sombra dibujada. |

## Ficheros

- **Nuevos:** `tools/iso/mobles_nacions.py`, `tools/interiors_kit/este.py`,
  `tools/interiors_kit/este_sales.py`, `Claude outputs/int2/`,
  `planificacion/seguimiento/INT-02.json`.
- **Kit:** `tools/iso/sales.py`, `mobles.py`, `exportar_mobles.py` (ya no mueve las hojas
  del kit al final de `sprites.json`), `assets/textures/kit_parets.*`, `kit_mobles.*`,
  `assets/objects/mobles_kit.json`.
- **Salas:** `assets/levels/interior_en_*.json` y `assets/maps/interior_en_*.tmx` (33),
  las puertas de `assets/levels/ciudad_en_*.json` (15), los alias de las salidas en
  `assets/textures/sprites.json`, `mobles_kit.json` en `assets/proyectos/este_norte.json`,
  `tools/gen_interiores_kit.py` (`en` filtra por prefijo), `tools/interiors_kit/__init__.py`,
  `tools/gen_este_norte.py` (el aviso).
- **El viajero:** `tools/actores_pnj.py`, `tools/actores_oficios.py`,
  `assets/textures/personajes.json`, `personajes.md`, `personajes/equipo/mano_izq_*.png`,
  `pnj_boundington.*`, `assets/objects/plaza_razas_objetos.json`,
  `assets/levels/plaza_de_las_razas.json`, `assets/adventures/plaza_de_las_razas.json`.
- **Pruebas:** `examples/demo_este_norte.cpp` (carga también `mobles_kit.json`),
  `tests/Game/RegistroSpritesTests.cpp` (las salidas del este son la estera),
  `examples/demo_plaza_razas.cpp`.

## Verificación

- `demo_este_norte`: 22 exteriores y 33 interiores cargan; todos sus objetos tienen ficha.
- `test_registro_sprites`, `test_project_catalog`, `test_personajes_capas`,
  `test_editor_apariencia`: OK. `demo_plaza_razas`: 14 beats, todos alcanzables.
- `validar_todo`: 0 errores (`este_norte`: 55 niveles contra 1467 objetos);
  `validar_pnj`: 249 PNJ, 325 aspectos; `conectividad.py` OK.
- `ctest`: las 18 fallas de siempre; `linea_base_core`, las 35 deudas previas.
- El generador es determinista: correrlo otra vez no cambia ni un byte.
- PixelLab: 0 generaciones.
