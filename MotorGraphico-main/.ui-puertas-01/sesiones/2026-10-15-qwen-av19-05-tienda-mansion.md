# 2026-10-15 · qwen · AV19-05 la tienda abandonada y la mansión del Barrio Alto

Cola del orquestador (`orquestador/qwen.md`, tarea 2 de 7). Rama `agente/qwen/AV19-05` (sobre
`agente/qwen/AV16-09`, porque la tienda se conecta a la ciudad nueva), worktree
`~/onegai-ramas/qwen-AV19-05`, commit `3f7cf14`.

## Qué se hizo

- `tienda_abandonada` (12x9), `mansion_barrio_alto` (22x16) y `mansion_barrio_alto_p1` (22x10),
  vacíos de PNJ (los pone Z.AI en AV17-06).
- Conexiones ida y vuelta: `ciudad_centro` + `puerta_tienda_abandonada` (callejón del Comercial,
  (10,23)) y `puerta_cerrada_tienda` ((10,25), prop con `eventId` para la aventura); `ciudad_este` +
  `puerta_mansion_barrio_alto` ((24,21), portillo del seto) y `ventana_mansion_barrio_alto`
  ((17,16), la ventana accesible del jardín).
- `tools/gen_niveles_av19.py` + `tools/av19_niveles.py`: generador reejecutable con el convenio de
  los interiores de HEAD (suelo en TMX de `terreno_mundo` sin colisión; paredes/muebles como objetos
  bloqueantes sobre celdas vacías; alfombras como tiles `int_catifa_*`; muros solo en fila 0 y
  columna 0, las dos caras que ve la isométrica). Planta las puertas en las ciudades comprobando que
  sus celdas siguen andables y libres, y registra niveles y catálogo en el manifiesto.
- `assets/objects/boundington_av19_objetos.json`: 9 ids nuevos (puertas, salidas, escaleras,
  ventana). El resto del mobiliario son ids existentes de los kits.

## Verificación

- `conectividad.py` exit 0: 108/108, 352/352 y 220/220 celdas, todos los objetos alcanzables.
- `validar_puertas.py` 12 problemas = HEAD; `validar_enlaces.py` 0 problemas; `validar_todo.py`
  1 error + 22 avisos = HEAD, y el proyecto boundington valida 44 niveles contra sus catálogos.
- `juego --nivel <nivel> 3` carga los tres y vuelca frame.

## Hallazgos para el registro

1. **Los interiores no dibujan muebles ni paredes en el juego, tampoco en HEAD.** Las regiones de
   sprite de los kits (`assets/textures/kit_mobles.json`, `kit_parets.json`) no las consume nadie del
   C++ (`sprites.json` no las lista); solo se ven en el minimapa y en los planos. Comprobado
   arrancando el juego con `interior_banco.json` de HEAD: mismo aspecto sin muebles. Deuda de
   arte/motor, no de estos niveles.
2. `gen_interiores.py` (paso 3 de `regenerar_ciudad.sh`) escribe un TMX con el tileset `ciudad` y
   muros por tile, mientras los interiores de HEAD usan `terreno_mundo` y muros por objeto: el script
   quedó obsoleto y, si se ejecuta, degrada los interiores vivos. Ver también la sesión de AV16-09.
3. El jardín de la mansión queda dentro del nivel; la casa noble de la parcela (18,12,12,10) de
   `ciudad_este` sigue en pie detrás de la mansión. Si Oriol quiere que la mansión reemplace la
   parcela, toca `gen_ciudad.este()` (decisión pendiente, dicha en el INFORME).
