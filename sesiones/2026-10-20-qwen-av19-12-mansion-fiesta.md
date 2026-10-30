# 2026-10-20 · qwen · AV19-12 la fiesta dentro de la mansión (32×28) y la calle de noche

Cola del orquestador (`orquestador/qwen.md`, tarea 4 de 6). Rama **`agente/qwen/AV19-12`**, encadenada
sobre `agente/qwen/AV19-10` (`fc5d082`). Mismo worktree y mismo build (disco al 95 %).

## Un defecto encontrado en mi propio trabajo de AV19-05

`Nivel.paret()` y `Nivel.murs()` **solo colocan pared donde el suelo es `BUIT`** (gid 0), que es el
convenio de los interiores de HEAD (`interior_banco`: 24 paredes sobre 24 casillas vacías). En AV19-05
pinté suelo en toda la rejilla antes de llamar a `murs()`, así que no se levantó **ni una pared**:

| nivel | objetos `paret_*` | casillas con gid 0 |
|---|---|---|
| `interior_banco` (HEAD, bien) | 24 | 24 |
| `mansion_barrio_alto` (AV19-05) | **1** (puesto a mano) | **0** |
| `tienda_abandonada` (AV19-05) | **1** (puesto a mano) | **0** |

O sea: la mansión y la tienda de AV19-05 eran un campo abierto con muebles, sin muros ni tabiques.
Ningún validador lo decía (`conectividad.py` daba 100 % porque todo era andable). **La mansión queda
arreglada aquí** (175 objetos `paret_*`/verja/seto). **`tienda_abandonada` sigue igual**: no la toco
porque está fuera del alcance de AV19-12 y sus casillas las usan beats de Z.AI/Codex; queda dicho aquí
y en el INFORME para quien la quiera rehacer con el mismo generador.

## La mansión 32×28

`tools/av19_mansion.py` (nuevo). **240 objetos, 896 casillas con suelo y todas alcanzables** (100 %),
inicio (2,25) en el recibidor. El suelo se pinta **solo dentro de las habitaciones** y los muros se
levantan donde queda `BUIT`, que es como lo hace HEAD.

- **El salón con damero**: x 1..13, y 1..12, alternando `int_marbre` / `int_marbre_negre` (existe el
  tile oscuro, así que no hace falta `marmol_real`).
- **La chimenea** en (6,0) con `brasero` delante y **hueco para los cuatro músicos**: cuatro
  `atril_musica` en (8..11,2) y las casillas (8..11,3) reservadas.
- **La escalera doble**: dos `escalera_mansion_p0_sube` en (17,22) y (18,22), en el ala este.
- **El guardarropa**: cuarto pequeño (x 17..19, y 10..13) con sus tres tabiques y **cuatro
  `perchero_ropa`** + el `baul` de las máscaras.
- **La cocina** (x 15..19, y 1..8, suelo de baldosa) **por donde sale un criado**: `criado_bandeja` en
  (17,5) y la puerta `salida_cocina_mansion` en (20,5) al jardín.
- **El jardín con verja**: x 21..30, `verja_dorada` en el borde este y sur, **setos recortados**
  (`seto_recortado`) en dos pares, la **fuente iluminada** (`fuente` (26,6) + `estatua_fuente` (26,5))
  y **siete faroles de papel** (`farolillo_papel`).
- **La ventana por la que se trepa**: `salida_ventana_mansion` en (21,8), en el jardín, justo delante de
  la pieza de pared con ventana (20,8): el jardín la alcanza, que es lo que pedía la ficha.
- **Comedor** (x 1..13, y 14..20) con la mesa de banquete puesta y **recibidor** de mármol (y 22..27).

**El piso de arriba** (`mansion_barrio_alto_p1`, 22×12, 96 objetos, 264 casillas, 100 % alcanzables):
**pasillo de puertas cerradas** (y=5..7) con cuatro puertas al norte y tres al sur, los dormitorios de
AV19-05 intactos, y **el despacho** de mármol (x 16..20, y 9..11) con `escritorio` (el de patas de
león), `atril_grimorio`, `cofre`, `estanteria_libros`, `silla_noble`, `bola_cristal` y `candelabro`.
La **ventana para salir corriendo** da **al jardín de la propia mansión** (21,9), no a la calle: así
«si te pillan» aterriza debajo de la ventana por la que se trepa y no queda un paso sin vuelta.

**Las puertas cerradas NO bloquean.** Todas las piezas `paret_*` bloquean, también las que se llaman
`_porta` (son el marco, no el hueco): con el hueco cerrado, las habitaciones quedan inaccesibles y
`conectividad.py` cuenta cada mueble de dentro como «puerta inalcanzable» (ERROR). Así que el hueco va
sin objeto que bloquee y la puerta cerrada es un marcador propio, **`puerta_cerrada_mansion`**
(`blocksMovement: false`, interactuable, «Cerrada por dentro»): se ve cerrada y Codex la cierra de
verdad por bandera si quiere.

## La calle de noche (`ciudad_este`)

- **Farolas en las esquinas**: regla determinista —casilla andable y vacía con muro a un lado vertical
  **y** a uno horizontal—. Salen **51 esquinas**; se ponen **26 alternas** para no convertir la calle en
  un bosque de farolas (todas serían 51 objetos que bloquean). `farola` ya existe en
  `pixellab_objetos.json`.
- **Cortinas con luz en las ventanas**: **110 fachadas** dan a calle; se pone `ventana_luz` en **una de
  cada tres (37)**, sobre casilla de edificio y por eso `flotante: true`.
- **Las dos puertas de la mansión corregidas**: apuntaban a la versión vieja de 22×16 y
  `validar_puertas` ya decía «llegada (14,10) … hay un objeto que bloquea». Ahora
  `ventana_mansion_barrio_alto` (17,16) llega a (22,8) y `puerta_mansion_barrio_alto` (24,21) a (1,24).
- **Las tres casillas de la escena** (libres y comprobadas): el **cruce del cobrador** (10,20), la
  **esquina del vigilante** (20,26) y **la calle donde Edul te espera** (26,20).
- Edición **quirúrgica**: se detecta la sangría del fichero (aquí indent=2) y se comprueba que las
  únicas líneas que cambian son las dos puertas; el resto son objetos añadidos. En AV19-11 esto mordió
  con `ciudad_oeste.json` (indent=1) y un cambio de cuatro líneas salió como un diff de 1070+1070.

Ni las farolas ni las cortinas llevan `disponibleDia`: el motor no las apaga de día. Quién las ve y
cuándo es de Codex, que las condiciona con `apariciones` como el resto del decorado por franja.

## Marcadores nuevos (8)

Esperan arte: **`verja_dorada`**, **`seto_recortado`**, **`farolillo_papel`**, **`ventana_luz`**,
**`criado_bandeja`**, **`puerta_cerrada_mansion`**. Puertas: `salida_cocina_mansion`,
`ventana_huida_mansion`. Todo lo demás son ids que ya existían (`mobles_kit.json` tiene `escritorio`,
`atril`, `piano`, `atril_musica`, `mesa_banquete_*`, `perchero_ropa`, `baul`, `cofre`, `espejo_corte`,
`perchero_vestuario`; `pixellab_objetos.json` tiene `farola`, `fuente`, `jardinera`, `banco_piedra`).

## Los 40 invitados

No se colocan (los pone Codex por bandera `bnd2_fiesta_dentro`); se dejan las casillas **libres,
pisables y alcanzables**, y el generador las busca él mismo (las más cercanas al ancla, en orden de
lectura) para que sean deterministas sin escribir cuarenta coordenadas:

- **30 en el salón**, alrededor de (7,7): (7,7) (6,7) (8,7) (7,8) (7,5) (6,6) (8,6) (5,7) (9,7) (6,8)
  (8,8) (7,9) (7,4) (6,5) (8,5) (9,6) (4,7) (10,7) (5,8) (9,8) (6,9) (8,9) (7,10) (7,3) (6,4) (8,4)
  (5,5) (9,5) (4,6) (10,6).
- **10 en el jardín**, alrededor de (26,18).
- Sitios fijos: **anfitrionas en la puerta** (2,25) (3,25) · **el noble que brinda junto a la chimenea**
  (6,2) (7,2) · **el cuarteto** (8..11,3) · **los mercaderes en la escalera** (16,22) (19,22) ·
  **el seguidor con la copa** (5,6) · **el criado que sale de la cocina** (16,6).

## Verificación

- `conectividad.py`: `mansion_barrio_alto` 896/896, `mansion_barrio_alto_p1` 264/264, `ciudad_este`
  1574/1574, **las tres al 100 %** y 0 errores en el repo (las 26 farolas que bloquean no encierran
  nada).
- `validar_puertas.py`: 127 niveles, 375 puertas, **12 avisos: los de siempre, ninguno de la mansión ni
  de `ciudad_este`** (antes de corregir las dos puertas había 13).
- `validar_mundo_unido.py`: «todo se alcanza y de todo se vuelve». `validar_todo.py`: **2 errores, los
  dos heredados** (PNJ de la Barriada y la daga de Aigren).
- Guardas propias: ninguna reserva tapada ni en casilla sin suelo, ni dos objetos que bloquean en una
  casilla, ninguna puerta inalcanzable desde el inicio, ninguna objeto fuera de una casilla con suelo
  (salvo paredes, verja y setos, que van justo donde no lo hay), y las llegadas de las escaleras se
  comprueban contra los dos niveles **en memoria** (contra el disco saltaban con IndexError: el p1 viejo
  es de 22×10 y la planta baja vieja de 22×16).
- Capturas: `out/planos/mansion_barrio_alto.png` (240 objetos) y `mansion_barrio_alto_p1.png` (96).
- **No se ha jugado en el motor.**
