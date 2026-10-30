# 2026-10-20 · qwen · AV19-10 las Afueras al ocaso, encima del dibujo de Oriol

Cola del orquestador (`orquestador/qwen.md`, tarea 2 de 6; tercera en ejecutarse por el orden
1 → 3 → 2 → 4 → 5 → 6). Rama **`agente/qwen/AV19-10`**, encadenada sobre `agente/qwen/AV19-11`
(`ec5aa22`). Mismo worktree y mismo build (disco al 95 %; ver la sesión de AV19-09).

## La regla que ha mandado en toda la tarea

Oriol (21/10): **«respectem els mapes que em vaig passar molt temps modificant-los en el excel»**.
Así que el bloque `afueras_boundington_ocaso` de la hoja «Otros» es intocable: **ni una casilla de
suelo repintada y ni un objeto suyo movido**. Todo lo que pide la ficha de AV19-10 va encima, en
casillas que estaban libres, y cada añadido se marca en la copia del Excel que se le deja.

El nivel ya existía desde AV19-09 (se construyó entonces porque las puertas del Casco y de la huida
apuntan a él y sin el nivel la rama quedaba roja). Esta tarea le pone la capa del Ocaso: pasa de
**70 objetos a 102**. Las 10 casillas de suelo que cambian respecto de `afueras_boundington` son las
del Excel (roca → pradera/sendero en el borde oeste, por donde sale el paso a la huida), no mías.

## Lo que se ha añadido (32 casillas nuevas, todas marcadas `+AV19-10` en el Excel)

- **El molino parado** (6,5) — marcador `molino`. Hoy no hay molino en las Afueras; la ficha lo pide.
  Sitio para **Venides de rodillas** (7,6) y **tres guardias** (8,6), (7,7), (8,7).
- **La manzana medio destrozada, cerrada por la barricada en llamas**: las casas son las suyas
  (x 32..35, y 18..20), sin repintar; lo que la dice «destrozada» son los escombros de las casillas
  libres de al lado (`runa` (31,18), `columna_rota` (36,18), `runa_b` (31,19)) y la barricada
  (carro, tres fuegos, sacos, carro girado) en y=17, x 31..36.
- **La plaza interior con el abrevadero y el árbol grande**: `arbol_grande` (33,26) y `abrevadero`
  (31,28) —el abrevadero es el marcador que ya creó AV19-11, se reutiliza— con **7×7 libres**
  (x 30..36, y 23..29) comprobados vacíos *después* de plantarlo todo. Ahí caben **Edul retenido**
  (33,25), **tres guardias** (32,25) (34,25) (33,24), **Sandes para el combate final** (33,27) y
  **seis casillas para la compañía de Rosita** (30..36,29).
- **Cuatro barricadas cruzando los caminos, dos con el fuego blanco de Edul**: (19-21,8) y (19-21,25)
  con `fuego_blanco`; (5-7,23) y (30-32,16) con `fuego`. Carros, sacos, barriles, un baúl y cajas.
  No cortan el mapa en dos: la pradera está abierta y se rodea, que es lo que quiere la ficha
  («cruzando las calles»).
- **Los huertos: el pasillo por donde entran Zarela y los guerreros de la Barriada**. Es el hueco que
  ya hay entre los bancales de Oriol (x 17..23) y el hueco del borde sur (x 16..20, y 31): **no se
  toca ni un bancal**. Se comprueba en cada pasada que el pasillo (18,31)→(18,29)→(20,27) está libre.
  Casillas: **Zarela a caballo** (18,30) y **cinco guerreros** (17,30) (19,30) (20,30) (18,29) (17,29).
- **El costado por donde cargan los bandidos de Gorran**: el este, cinco casillas (36-38,19) y (37-38,20).
- **La gente desaparecida o tirada**: seis casillas para los cuerpos de la orden `tumba` —(5,15),
  (10,23), (22,26), (28,16), (13,30), (3,29)—. Las pone Codex; yo solo las dejo libres.
- **Tres rondas de Perdidos, rectas y en grupo** (`patrolMin`/`patrolMax`, el formato de
  `cueva_gorran`): 3 saqueadores por (10,10)-(30,10), 2 fanáticos por (3,20)-(3,25), 3 devotos por
  (8,26)-(18,26). Cada casilla de cada ronda se comprueba andable, sin objeto y sin puerta: **ninguna
  ronda pisa fuego ni barricada**, que es lo que pedirá AV19-14.

## Tres cosas que la ficha pedía y NO se han hecho (con motivo)

1. **La salida a la huida sigue al OESTE** (1,17-21), no al norte. La ficha dice «salida al norte»,
   pero el bloque de la huida que dibujó Oriol tiene sus cinco `P` en el borde **este** (39,18-22) y
   el final en (1,13): la huida se recorre de este a oeste. Mover el paso al norte de las Afueras
   dejaría la huida colgando de un borde que Oriol no dibujó. **Excel manda** (ya pasó igual en
   AV19-09 con el paso del Casco, que está al sur y no al oeste).
2. **No hay puerta a `ciudad_surysal`.** La ficha dice «y desde `ciudad_surysal` si se llega por el
   oeste» (condicional), pero en el Excel el borde oeste ya es la salida a la huida, y el norte es la
   `P` del Casco. Añadir el par de puertas habría tocado `ciudad_surysal`, que es el nivel que Claude
   acaba de reconstruir con todos los personajes en escena (ESCENAS-01). **Se pregunta en el INFORME**
   en vez de inventarlo: la entrada de la Barriada ya existe como pasillo de los huertos, que es por
   donde la ficha hace entrar a Zarela.
3. **Los bancales no se han repintado de «pisoteados».** Cambiarlos sería repintar su suelo. Se quedan
   como `bancal_huerto` y queda dicho aquí y en el Excel, para que lo decida él.

## El Excel para Oriol

`MotorGraphico/out/qwen_av19_10/boundington_ciudad8_av19-10.xlsx` — **copia**: su original
(`out/fable_ciudad02/boundington_ciudad8.xlsx`) no se toca, ni falta que hace. Mismas seis hojas y
misma geometría del bloque (título en la fila 97, columna 46), con el título corregido a
`afueras_ocaso  (40x32)` y **las 32 casillas nuevas** con su letra, su comentario
(`objectId (x,y)` + `→ destino` cuando es paso) y la marca `+AV19-10 (Qwen)`. Los colores de suelo
que él pintó se conservan (comprobado: (6,5) sigue siendo `007DBE6B`, Hierba).

Se puede corregir ahí y volver a meter con
`python3 tools/av19_ocaso.py --xlsx out/qwen_av19_10/boundington_ciudad8_av19-10.xlsx`.
Aviso: la copia pasa por openpyxl, que conserva rellenos, comentarios, anchos y celdas combinadas,
pero perdería imágenes o gráficos incrustados si los hubiera.

## Marcadores nuevos (3)

**`fuego_blanco`** (el de Edul; la ficha lo pide si AV17-11 no trae variante, y hoy `fuego` sigue
siendo un marcador, así que tampoco), **`molino`** y **`arbol_grande`**, los tres con spriteId 3
provisional y a la lista de Pixel. Se reutilizan `abrevadero` y `caballo` de AV19-11 en vez de crear
otros.

## Verificación

- `conectividad.py`: `afueras_ocaso` **1021/1021 (100 %)** y las 102 puertas alcanzables; 0 errores en
  todo el repo. Las cuatro barricadas no encierran nada: la comprobación del generador abre fuego y
  carros y vuelve a inundar.
- `validar_puertas.py`: 127 niveles, 371 puertas, 12 avisos, **ninguno nuevo**.
  `validar_mundo_unido.py`: «todo se alcanza y de todo se vuelve».
- `validar_todo.py`: **2 errores, los dos heredados** (los PNJ de la Barriada sin fila propia en
  `pnj_*`, y la daga de Aigren), igual que en la base `bfc5a22`.
- Guardas propias del generador (fallan en vez de escribir): ni una reserva tapada, ni un objeto en
  casilla ocupada, el 7×7 de la plaza vacío después de plantar todo, el pasillo de los huertos libre,
  y cada casilla de cada ronda andable y sin fuego ni puerta.
- Captura: `out/planos/afueras_ocaso.png` (102 objetos). **No se ha jugado en el motor**: la
  comprobación es la de los validadores y el flood fill.
