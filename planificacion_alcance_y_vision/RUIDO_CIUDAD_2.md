# AV15-03b — Boundington menos caótica (2.ª vuelta, `ciudad_centro`)

Hecha el 15/10/2026 por Claude (carril `ciudad` de la [ronda del 15/10](../orquestador/ronda_15-10.md)).
Paquete: `MotorGraphico-main/MotorGraphico/out/claude_ciudad02/` (`aplicar.sh`).
Sesión: [sesiones/2026-10-15-claude-ciudad-2.md](../sesiones/2026-10-15-claude-ciudad-2.md).
Viene de: [RUIDO_CIUDAD.md](RUIDO_CIUDAD.md) (AV15-03, 1.ª vuelta) y EH-06 de la [evaluación heurística](EVALUACION_HEURISTICA.md).
Oriol, tras jugar la 1.ª vuelta: «la ciudad principal de Boundington se sigue viendo demasiado caótica».

## En una frase

La 1.ª vuelta abrió el paso; el caos era otra cosa: **demasiadas cosas, de demasiados tipos, repartidas por igual
en todas partes.** Ahora hay 190 objetos en vez de 297. El decorado está junto en unos pocos sitios con sentido
(la plaza, el puente, el muelle de carga, las puertas) y las calles entre ellos están limpias.

**Aviso importante:** la máquina virtual de Oriol se colgó al empezar (device_bash falló en todas las llamadas;
también les pasó a otros agentes). No he podido compilar, ni pasar `ctest`, ni sacar capturas del juego. Las
imágenes del paquete son una **vista esquemática hecha desde los datos** (suelo del TMX + objetos del nivel),
no capturas. Dejo la sonda del juego lista para Claude Cowork (§7).

## 1. Por qué se veía caótica (medido sobre el nivel de ahora)

Cómo se ha medido: `herramientas/medir_ruido.py` (solo lectura). «Decorado» = ropa, banderas, luces,
vegetación y mobiliario; no cuenta puertas, carteles, barcos ni PNJ. «Por pantalla» = lo que cabe en
1280x720 con tiles de 64x32 a zoom 1, alrededor de cinco sitios: plaza (31,47), puente (32,32),
Comercial (20,26), Casco sur (12,54) y muelles (48,27).

| Causa | Número |
|---|---|
| Objetos en el nivel | **297** (203 de decorado; 164 de ellos sobre la calle) |
| Lo que se ve en una pantalla, de media | **55,6** objetos de decorado + 11,4 PNJ, de **16 tipos** distintos |
| Ropa tendida | **62 celdas, 38 cuerdas**; 35 cuelgan sobre un paso; 47 sobre adoquín; 26 en el Distrito Comercial |
| Ropa sin atar | En el Casco, solo 4 de 16 cuerdas tienen las dos puntas en una pared: el resto flota en patios y cruces |
| Postes altos (banderas + farolas + fanales) | **48**: uno cada 5,9 casillas de media. 8 de las 16 banderas, en la ronda y las puertas de la muralla |
| Plantas sueltas por la calle | 20 jardineras y 15 matas al azar; dejaban una calle en **una celda** 12 y 9 veces |
| Sin jerarquía | El mismo vocabulario en todos los barrios. La avenida de los monumentos (el eje del poder) llevaba robles y jardineras; el Comercial, 52 objetos (26 de ropa) |
| PNJ | 41; 34 de relleno sorteados al azar por todo el mapa |
| Suelo | Las calles alternan filas de `pavimento_antiguo` (501 celdas) y `adoquin_limpio`: se ve rayado. El Casco mezcla adoquín y patio de tierra, grava y ruinas (343 celdas). **No lo he tocado** (§6) |

Densidad por barrio (decorado por cada 100 celdas de calle) y tipos distintos:

| Barrio | Antes | Tipos antes |
|---|---:|---:|
| Norte monumental | 4,0 | 5 |
| Distrito Comercial | 9,3 | 5 |
| Río y muelles | 16,8 | 11 |
| Casco Antiguo | 10,7 | 16 |

## 2. El criterio

De los apuntes de usabilidad (jerarquía, espacio vacío, «un fondo con textura mete ruido»; hitos únicos):

1. **Jerarquía de calles.** Los ejes (la avenida norte-sur y la de los monumentos) van limpios: solo luz, a
   ritmo. Las calles comerciales son escaparate. Los callejones del Casco tienen la vida (ropa, flores, basura).
2. **Cada barrio, pocos tipos.** Norte: banderas y farolas de alambique. Comercial: farolas, jardineras y
   paradas. Casco: ropa, árboles pobres, flores, farolas viejas.
3. **Momentos y calle despejada entre ellos.** La plaza, el puente, el muelle de carga y las puertas
   concentran el decorado.
4. **La vista va a las puertas.** Farolas a los dos lados de la ópera, la universidad y la biblioteca;
   jardineras a los lados de la joyería, el banco y la sastrería; paradas en los mercados. Ninguna farola a
   3 casillas o menos de una puerta (su entrada se ve y se llega recto).

## 3. Qué se ha cambiado

Todo en el generador, con la semilla de siempre. El nivel sale de regenerar. **El suelo no cambia**: el CSV del
TMX regenerado es idéntico al de ahora, y el paquete no trae TMX.

**`tools/gen_ciudad.py` (`centro()` y `escribir()`)**
- Banderas: 16 → 6. Quedan la puerta del castillo, la puerta norte de la muralla y la cabeza norte del puente.
- Farolas de alambique: en el puente (4), a los lados de tres monumentos (6) y cuatro en el Comercial a ritmo
  largo (separación 14 en vez de 8). Antes: 8 al azar por la avenida, 6 en el Comercial, 2 en el cruce.
- Farolas viejas del Casco: 10 → 6 (separación 12), nunca a 3 casillas o menos de una puerta.
- Jardineras sueltas y matas del Comercial: fuera. Jardineras solo a pares en tres tiendas.
- Plaza: sin jardineras ni banderas (ya tiene fanales, bancos, paradas, dos robles y el pozo).
- Sauces del muelle: 8 → 4, enmarcando el puente. Robles de la avenida de los monumentos: fuera.
- Casco: árboles pobres 10 → 6; las matas sobre adoquín, fuera; basura y barriles rotos, solo junto a la
  taberna y la herrería.
- Comercial **sin ropa** (`sin_ropa` antes de las manzanas; el azar se consume igual).
- PNJ de relleno: 25 → 18 (se sortean 8 en el Comercial, 6 en el Casco y 4 en la plaza; antes 34 sorteos).
- **PNJ con papel en las aventuras: no se sortean.** `guardia`, `anciana`, `bardo`, `mercader`,
  `parroquiano_humilde` (y, como antes, `sectario_perdido` y `carterista`) se conservan del nivel, **en su
  sitio**: las 16 instancias siguen en la misma casilla. Lo he mirado en `assets/adventures/`: las misiones los
  buscan por su id (`trigger.target`).
- `escribir()` pone en el nivel la clave `"carriles"` (§5).

**`tools/gen_ciudad_deco.py`**
- `ropa_entre_casas()` (nueva): se quedan solo las cuerdas con las dos puntas atadas a una pared, como mucho
  12, primero en la calle con alma (de la herrería a la taberna). Si faltan, tiende nuevas allí, en pasos de
  1 a 3 celdas entre dos paredes, lejos de las puertas y separadas entre sí.
- `al_carrer()`: parámetro nuevo `lluny_porta` (por defecto 0: los otros barrios no cambian).

**`tools/despejar_calles.py`**
- `despejar()` devuelve también `"carriles"`: las casillas de los carriles rectos que elige.
- `--escribir` las guarda en el nivel. **`--carriles`** es una guarda nueva: los carriles del nivel son los
  calculados y están libres (sale con 1 si no).

**`assets/levels/ciudad_centro.json`**: regenerado. 297 → 190 objetos, más `"carriles"` (232 casillas).

Determinista: dos pasadas dan el mismo fichero byte a byte. Y estable: regenerar desde el nivel nuevo da
exactamente los mismos objetos (antes, 4 farolas cambiaban de sitio en cada pasada).

## 4. Antes y después

| Tipo | Antes | Después |
|---|---:|---:|
| tendedero (todas las piezas) | 62 | 17 (12 cuerdas, todas en el Casco) |
| jardinera | 20 | 6 |
| farola_alambique | 18 | 14 |
| bandera_ciudad | 16 | 6 |
| mata (con variantes) | 15 | 3 |
| farola | 10 | 6 |
| sauce | 8 | 4 |
| arbol_seco | 6 | 1 |
| arbol_roble | 4 | 2 |
| pino | 1 | 2 |
| basura_taberna | 2 | 1 |
| PNJ | 41 | 34 (los 16 con papel, iguales; relleno 25 → 18) |
| Puertas, carteles, pasos, actuaciones, barcos, mascotas, mobiliario de plaza y muelle | igual | igual |
| **Total** | **297** | **190** |

| Medida | Antes | Después |
|---|---:|---:|
| Decorado / decorado sobre la calle | 203 / 164 | **103 / 77** |
| Por pantalla, de media: decorado | 55,6 | **29,4** |
| Por pantalla, de media: tipos de decorado | 16,0 | 13,8 |
| Postes altos; distancia media al vecino | 48; 5,9 | **30; 7,0** |
| Ropa: celdas / cuerdas | 62 / 38 | 17 / 12 |
| Norte monumental (decorado por 100 celdas; tipos) | 4,0; 5 | 2,2; 2 |
| Distrito Comercial | 9,3; 5 | 2,5; 3 |
| Río y muelles | 16,8; 11 | 11,7; 11 |
| Casco Antiguo | 10,7; 16 | 6,0; 14 |
| Objetos que bloquean (sobre calle) | 170 (121) | 109 (71) |

Por pantalla, los cinco sitios: plaza 61 → 31, puente 64 → 37, Comercial 64 → 31, Casco sur 39 → 26,
muelles 50 → 22 (decorado). Los tipos por pantalla bajan poco porque una pantalla ve dos o tres barrios a la
vez, y los momentos (plaza, puente, muelle) conservan su mobiliario.

Comparativa: `out/claude_ciudad02/comparativa/hoja_antes_despues.png` (cinco parejas) y
`plano_antes_despues.png` (la planta entera; en azul, los carriles).

## 5. Contrato con `pnj_motor`: `"carriles"`

`ciudad_centro.json` lleva `"carriles": [[x, y], ...]`: 232 casillas, ordenadas por fila y columna. Son el carril
recto de las siete calles principales de `despejar_calles.BANDAS` (avenida norte-sur, columnas 31-32, en cuatro
tramos; avenida de los monumentos, fila 14; calle comercial norte, fila 19; calle comercial sur, fila 25). Un
PNJ puede cruzarlas, pero no pararse en ellas.

**El cargador C++ la ignora.** `src/Level/LevelLoader.cpp` (`parseDefinition`) solo lee `name`, `map`,
`mapDocument`, `playerStart`, `enemies`, `objects`, `fog` y `lighting`, cada uno por su nombre; una clave que no
conoce no se mira. Tampoco la rechazan las herramientas que he mirado (`validar_puertas.py`,
`validar_enlaces.py`, `conectividad.py`, `medir_calles.py`). No hay esquema JSON de niveles en `schemas/`. Falta
documentarla en `FORMATO_NIVELES.md`, que no es de mi zona.

## 6. Guardas

Pasadas en una copia limpia de los ficheros de Oriol (ver la nota del §7):

- `python3 tools/medir_calles.py ciudad_centro --comprobar`: **bien**. 19/19 puertas alcanzables desde el
  puente y desde la plaza; ninguna tapada; 7/7 carriles rectos. Desvío desde el puente 3,7 % (igual); desde la
  plaza, 5,5 % → 4,4 %. Lo que ponen los objetos: +2 y +8 pasos (igual). La iglesia sigue en el 44 % desde la
  plaza por la forma de las casas (PO-44, de la 1.ª vuelta).
- `python3 tools/conectividad.py`: ciudad_centro 2490/2490 celdas alcanzables (100 %); todas las puertas.
- `python3 tools/despejar_calles.py ciudad_centro --carriles`: bien (232 casillas, libres y calculadas).
- Puertas, carteles, pasos, actuaciones y los 16 PNJ con papel: la misma casilla que antes. Las salidas de los
  interiores siguen apuntando bien.
- Pruebas C++ que leen ciudad_centro, **leídas, no pasadas**: `ActuacionTests` (las tres `actuacion_*` siguen,
  una vez cada una), `LugaresTests` (las puertas a interiores no cambian), `BoundingtonTutorialTests` y
  `RumboNivelesTests` (no miran el decorado). **Falta pasar `ctest -R "ciudad|boundington|nivel"`.**
- `aplicar.sh`: probado sobre una copia limpia. Aplica (4 ficheros), se niega la 2.ª vez («CAMBIADO …
  NO APLICADO») y, tras aplicarlo, regenerar da lo mismo. **La copia de prueba fue en mi contenedor, no en
  `/tmp/ag/ciudad/prueba`**: la máquina virtual no respondía.

## 7. Lo que queda

**Para Claude Cowork (lo que no he podido hacer):**
- Compilar y pasar `ctest -R "ciudad|boundington|nivel"` con el paquete aplicado.
- Las capturas del juego: `herramientas/hazla_sonda_ciudad.sh <build> antes_` con el nivel de ahora,
  aplicar, `… despues_`, y `herramientas/hoja.py <carpeta> hoja.png` junta las parejas (acepta capturas de
  1280x720). Mirarlas: el juicio es visual.
- Si alguien pasa entero `tools/regenerar_ciudad.sh`, los pasos 3-6 (interiores, carteles, `ambientar_boundington`,
  enganche) van después de `despejar_calles`. Conviene acabar con
  `python3 tools/despejar_calles.py ciudad_centro --escribir` para que `"carriles"` y los carriles libres sigan.

**Arte (Pixel):**
- Que lo que bloquea se lea como barrera: `jardinera` y las farolas son de 64x96 y parecen andables desde
  ciertos ángulos (ya lo decía la 1.ª vuelta).
- Un emblema o una placa propia para la plaza del Casco y el puente: ahora son los dos hitos.

**Suelo (no lo he tocado):**
- El rayado de `pavimento_antiguo` y `adoquin_limpio` en filas alternas en todas las calles del norte. Si la
  calle principal fuera un solo suelo y el carril otro, se leería la jerarquía desde el suelo. Pide regenerar el
  TMX: es una decisión de Oriol.

**Preguntas para Oriol (PO):**
1. **Ropa en el Comercial.** La he quitado: el canon pone cuerdas en los «barrios populares», y he tratado el
   Comercial como escaparate. Si lo quieres con ropa, es una línea (`m.sin_ropa` en `centro()`).
2. **Seis `anciana`.** Las misiones buscan «la anciana» por su id. ¿Tienen papel las seis o solo una? Si es una,
   sobran cinco PNJ en el Casco.
3. **Las banderas de la puerta sur** (camino a Egaroth). Las he quitado; si es la entrada que quieres marcar,
   vuelven dos.
4. El pozo de la plaza pasa de mirar a un lado (`pozo_o3`) a otro (`pozo_o2`): `orientar()` lo gira hacia el lado
   más despejado, y ahora la plaza tiene menos cosas. Es cosmético.
