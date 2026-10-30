# AV15-03 — Quitar ruido de la ciudad (ciudad_centro)

Hecha el 14/10/2026 por Claude. Paquete: `MotorGraphico-main/MotorGraphico/out/claude_ciudad01/` (`aplicar.sh`).
Sesión: [sesiones/2026-10-14-claude-ciudad-ruido.md](../sesiones/2026-10-14-claude-ciudad-ruido.md).
Origen: EH-06 de la [evaluación heurística](EVALUACION_HEURISTICA.md) y D0015 §14 («muchos objetos tapan la vista e impiden ir recto»).

## En una frase

La ciudad ya se podía andar casi en recta; lo que molestaba era otra cosa: **una puerta sellada por un PNJ, cuatro
de las siete calles principales con un estorbo en cada carril, y que los objetos altos tapaban al jugador sin
fundirse** (los muros y los bloques sí se funden; los objetos no). Las tres cosas están arregladas en el paquete.

## 1. Cómo se ha medido (`tools/medir_calles.py`, solo lectura)

Reproduce la regla de colisión del juego (`GameSession::tryMovePlayer`): un tile con `collision` bloquea; un objeto con
`blocksMovement` en el catálogo bloquea (los PNJ también); se anda a 4 direcciones. Parte de dos sitios y mide la ruta
más corta (BFS) hasta cada puerta de un interior:

- **inicio del jugador** (32, 32), el puente;
- **la plaza del Casco Antiguo**: la celda libre más cercana al pozo con al menos tres vecinas libres (hoy, (32, 49)).
  *(Mi primera medida usó una celda encajada entre dos PNJ y salía un 11,9 %; era un artefacto del origen.)*

Por puerta: `ruta` (con tiles y objetos), `sin objetos` (solo tiles: lo que daría la ciudad vacía), `recta`
(Manhattan: lo mejor que se puede a 4 direcciones). **El desvío que ponen los objetos es `ruta − sin objetos`.**

## 2. Antes y después

| | Antes | Después |
|---|---|---|
| Puertas alcanzables desde el puente | 18 de 19 | **19 de 19** |
| Puertas alcanzables desde la plaza | 18 de 19 | **19 de 19** |
| Desvío medio sobre la recta, desde el puente | 4,3 % | 3,7 % |
| Desvío medio sobre la recta, desde la plaza | 6,0 % | 5,5 % |
| Pasos que ponen *solo los objetos*, desde el puente | +6 (1,1 %) | +2 (0,3 %) |
| Pasos que ponen *solo los objetos*, desde la plaza | +12 (1,7 %) | +8 (1,1 %) |
| Puertas con desvío < 25 % (el cierre de AV15-03) | 18/18 y 17/18 | **19/19 y 18/19** |
| Puertas con un objeto en una celda de acceso | 1 | **0** |
| Calles principales con un carril recto sin estorbos | 3 de 7 | **7 de 7** |

Lo que **no** se mueve: la puerta de la iglesia sigue a 26 pasos de la plaza para 18 en línea recta (44 %), y es por
la forma de las casas del Casco (0 pasos de los objetos), no por el decorado. Arreglarlo es abrir un paso en el
suelo: ver [PO-44](../BANDEJA_PRODUCT_OWNER.md).

El cierre de AV15-03 pedía «de la plaza a cada tienda se va recto o casi (desvío < 25 %); ningún choque con algo que
parecía andable». **Lo primero se cumple (18 de 19; la 19.ª es la iglesia). Lo segundo, no del todo**, y no lo he podido
medir como pide, porque *qué parece andable* lo decide el dibujo y no los datos: ver §5.

## 3. La lista: lo que bloquea o tapa (170 objetos; 121 sobre calle; 40 son PNJ)

«Celda de dibujo» es el tamaño de la celda del sprite en su hoja (alto de un tile: 32 px), no la altura visible:
una celda de 96 px puede cubrir hasta tres filas por encima de la suya.

| Objeto | Hay | Sobre calle | Tipo | Celda de dibujo | ¿Se funde? |
|---|---:|---:|---|---|---|
| `jardinera` | 20 | 20 | decorado | 64x96 | **sí, con este paquete** |
| `farola_alambique` | 18 | 16 | decorado | 64x96 | **sí, con este paquete** |
| `bandera_ciudad` | 16 | 10 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `farola` | 10 | 10 | decorado | 64x96 | **sí, con este paquete** |
| `mata` | 8 | 6 | decorado | 96x112 | **sí, con este paquete** |
| `sauce` | 8 | 0 | decorado | 96x112 | **sí, con este paquete** |
| `mata_b` | 7 | 6 | decorado | 96x112 | **sí, con este paquete** |
| `arbol_seco` | 6 | 0 | decorado | 96x112 | **sí, con este paquete** |
| `arbol_roble_b` | 4 | 4 | decorado | 96x112 | **sí, con este paquete** |
| `bolardo` | 4 | 0 | decorado | 64x96 | **sí, con este paquete** |
| `fanal_guardianes` | 4 | 4 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `parada_mercado` | 4 | 4 | decorado | 64x96 | **sí, con este paquete** |
| `fardos` | 3 | 0 | decorado | 64x96 | **sí, con este paquete** |
| `banco_brasero` | 2 | 2 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `banco_brasero_o2` | 2 | 2 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `banco_plaza` | 2 | 0 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `barriles` | 2 | 0 | decorado | 64x96 | **sí, con este paquete** |
| `barriles_rotos` | 2 | 1 | decorado | 64x96 | **sí, con este paquete** |
| `parada_mercado_o2` | 2 | 0 | decorado | 64x96 | **sí, con este paquete** |
| `carro_o1` | 1 | 0 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `fuente_viento` | 1 | 1 | monumento/mobiliario | 128x128 | **sí, con este paquete** |
| `grua_muelle` | 1 | 0 | monumento/mobiliario | 128x128 | **sí, con este paquete** |
| `mesa_taberna` | 1 | 0 | monumento/mobiliario | 64x128 | **sí, con este paquete** |
| `pino_b` | 1 | 0 | decorado | 96x112 | **sí, con este paquete** |
| `pozo_o3` | 1 | 1 | monumento/mobiliario | 64x96 | **sí, con este paquete** |
| `anciana` | 6 | 4 | PNJ | — | no (es un PNJ) |
| `npc_race_guardianes_de_la_luz` | 6 | 6 | PNJ | — | no (es un PNJ) |
| `npc_race_medianos_urbanos` | 5 | 5 | PNJ | — | no (es un PNJ) |
| `npc_race_alquimistas_de_aegroum` | 4 | 4 | PNJ | — | no (es un PNJ) |
| `npc_race_elfos_del_bosque` | 4 | 4 | PNJ | — | no (es un PNJ) |
| `mercader` | 3 | 2 | PNJ | — | no (es un PNJ) |
| `estudiante` | 2 | 2 | PNJ | — | no (es un PNJ) |
| `guardia` | 2 | 2 | PNJ | 64x64 | no (es un PNJ) |
| `nino` | 2 | 1 | PNJ | — | no (es un PNJ) |
| `npc_race_enanos_forjadores` | 2 | 1 | PNJ | — | no (es un PNJ) |
| `parroquiano_humilde` | 2 | 1 | PNJ | 64x64 | no (es un PNJ) |
| `bardo` | 1 | 1 | PNJ | — | no (es un PNJ) |
| `sectario_perdido` | 1 | 1 | PNJ | 64x64 | no (es un PNJ) |

Los objetos que **no** bloquean y por eso no cortan el paso (tendederos, flores, carteles, basura) no cuentan aquí.

## 4. Qué se ha cambiado

**a) Los datos (`tools/despejar_calles.py`, determinista e idempotente).** Cambia solo posiciones de objetos en
`assets/levels/ciudad_centro.json` (el suelo no se toca): **14 movidos, 0 quitados.**

- **Puerta de las termas (`puerta_banos`, 47, 60):** su único acceso (47, 61) lo ocupaba un PNJ
  (`npc_race_enanos_forjadores`); pasa a (48, 61). `tools/conectividad.py` decía «todas las puertas alcanzables» porque
  solo mira tiles; este caso solo se ve contando los objetos.
- **Un carril recto por calle principal.** En cada calle se elige el carril con menos estorbos y se apartan los que
  tiene, a la celda libre más cercana que no sea carril ni acceso a puerta, de preferencia arrimada a una pared:
  - avenida norte-sur en cuatro tramos (la plaza del Casco queda fuera: el pozo está en medio del eje): sale el carril
    30 (norte), 31 (puente y Casco norte) y 32 (Casco sur; 1 farola movida);
  - avenida de los monumentos: carril 14 (5 movidos: 2 jardineras, una mata, una farola, un PNJ);
  - calle comercial norte: carril 18 (3 movidos); calle comercial sur: carril 25 (4 movidos).
- **Lo que no se mueve nunca:** pozo, fuentes, fanales, bancos, grúa, carros, mesas y **banderas** (DECO-01 las puso a los
  lados de las puertas del castillo y de la muralla). Si un monumento está en el carril, se elige otro.
- Comprueba al final que **ninguna puerta alcanzable deja de serlo** y que nada que bloquea queda sobre colisión; si no,
  no escribe.

**b) El generador (`tools/gen_ciudad.py`, 4 líneas).** `escribir()` llama a `despejar_mapa()` con el mapa y los objetos
ya definitivos, para que regenerar la ciudad no deshaga esto. Comprobado: desde el nivel actual, el generador da
exactamente lo mismo que el script; el TMX sale idéntico; `ciudad_este` no cambia. Regenerar *desde el nivel ya
despejado* da otras 4 farolas de sitio (el generador las vuelve a colocar según lo que hay ocupado) pero
`--comprobar` sigue pasando.

**c) Lo que tapa al jugador, translúcido (`OcclusionRules` + `Application::render`).** El motor ya fundía tiles
(`OcclusionRules::shouldFadeTile`) y bloques (`BloquesNivel::atenua`), pero no los **objetos** (los marcadores). Ahora un
objeto de categoría prop que bloquea el paso y cae en la ventana por delante del jugador (profundidad 1 a 3, a
lo sumo 2 celdas a un lado: la misma que los bloques) se pinta al 40 %. Los PNJ no se funden. El fundido se apunta en el
marcador (`alfaProp`) y se quita multiplicando, para no pisar los tintes de las cinematicas.

**d) Una guarda.** `python3 tools/medir_calles.py ciudad_centro --comprobar` sale con 1 si hay una puerta tapada o
inalcanzable o una calle principal sin carril recto. Hoy falla sobre el nivel de antes y pasa sobre el de después.
Cabe en `tools/validar_todo.py` cuando se quiera.

## 5. Lo que no sé / no he podido hacer

- **El motor no lo he compilado ni jugado.** Aquí no hay `cmake`. Comprobado: la regla pura con 5 pruebas nuevas
  (21 de 21, `g++ -Wall -Wextra` limpio) y que `ApplicationBucle.cpp` parcheado pasa `g++ -fsyntax-only` con los
  includes reales. **Falta ver el fundido en el juego** (§6) y correr `ctest`.
- **«Parecía andable».** `jardinera` («Jardinera de flores») y `mata` bloquean en el catálogo, pero
  `gen_ciudad.centro()` las coloca como si no bloquearan (`bloquea=False`: sin la regla de las 7 vecinas libres).
  Por eso había 13 jardineras que dejaban una calle en una celda. Con los carriles despejados el paso queda bien; qué
  debe parecer un obstáculo y qué no es de arte (Pixel) y no lo decido yo.
- **Calles que no son carriles.** Los muelles (filas 28 y 32), la ronda y los callejones del Casco no tienen
  carril garantizado; el Casco es «deteriorado y lleno de secretos» por canon.
- **Otras ciudades.** Solo `ciudad_centro` tiene calles principales declaradas (`BANDAS` en `despejar_calles.py`).
  Además `tools/gen_ciudad.py` ya no llega a `ciudad_oeste`: se para en un `assert` porque el título del nivel lleva
  tilde («Dragón»). No es de este paquete; lo dejo anotado.
- El fichero `assets/maps/ciudad_centro.tmx` vivo lleva una cabecera de tileset antigua (`tilecount="156"`, imagen
  de 1280 px; el generador escribe 203 y 1664). No lo toco.

## 6. Qué mirar en el juego

1. `ciudad_centro`, desde el puente hacia el norte: un carril de la avenida sin nada. Lo mismo por las dos calles
   comerciales y la avenida de los monumentos.
2. Las termas (`puerta_banos`, al sur-este del Casco): se puede entrar.
3. Pasa por detrás de un árbol, una farola o una bandera: se funde al 40 %. ¿Demasiado, o poco? Es
   `OcclusionRules::kAlfaProp` (una constante).
4. Un PNJ delante del jugador no se funde (a propósito).
