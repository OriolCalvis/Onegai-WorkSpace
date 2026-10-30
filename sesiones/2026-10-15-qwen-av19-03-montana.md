# 2026-10-15 · qwen · AV19-03 la montaña de Gorran

Cola del orquestador (`orquestador/qwen.md`, tarea 6 de 7). Rama `agente/qwen/AV19-03` (sobre
`agente/qwen/AV18-05`), worktree `~/onegai-ramas/qwen-AV16-09b`, commit `9d8d177`.

## La dependencia, con precisión

La ficha dice que esta tarea necesita la altura en el motor (AV19-02, Claude) y que si no está se deje
para el final. El 15/10 por la noche **AV19-02 existe pero no está commiteado**: `FORMATO_NIVELES.md` e
`include/Game/TableroCombate.h` aparecen sucios en el árbol principal, sin rama ni commit
(`git log --all --grep=AV19-02` no lo encuentra). El FORMATO sucio documenta el contrato completo
(campo opcional `"alturas"`, regiones rectangulares con `nivel` 0..9, rampas con `hasta` y `eje`, ±1
nivel por paso, 16 px de subida por nivel).

Decisión: **escribir los datos contra ese contrato** y comprobar que no rompen nada. Prueba empírica con
el binario de hoy: un nivel con `"alturas"` carga y juega 3 frames igual (`juego --nivel …`), o sea que
el parser actual lo ignora. La montaña se ve plana hasta que AV19-02 entre, y entonces sube sola. Las
rampas son siempre de dos casillas y de un nivel, que son las únicas cuyo reparto intermedio no depende
del redondeo del motor (con tres casillas, `nivel → hasta` repartido «por igual» admite dos lecturas).

## Qué se hizo

- `montana_gorran` (28x36) en `tools/av19_niveles.py::montana()`: entrada al sur, sendero de 4 casillas
  (x 12-15) de `sendero_tierra`, tres terrazas (grava y `piedra_antigua`) y macizo y cantos de `roca`.
- Alturas: terraza baja nivel 1 (5,22,18x8), media nivel 2 (7,14,14x7), cima nivel 3 (9,5,10x7), y tres
  rampas de dos casillas (0→1 en y=30-31, 1→2 en y=21-22, 2→3 en y=12-13).
- 20 objetos: la bajada a las afueras, la boca de la cueva como marcador, pinos, árboles secos, seis
  antorchas, banco, brasero, columna rota y una runa. Sin PNJ.
- Conexión ida y vuelta con `afueras_boundington`: `sendero_a_montana_gorran` deja de ser marcador y
  pasa a puerta (se llega a (13,33), la casilla del inicio), y `salida_montana_gorran` vuelve a (20,29).
- `Nivel` gana `altura()` y `escribir()` vuelca el campo `alturas` solo si existe.

## Dos guardas nuevas, las dos por fallos de esta pasada

1. `andar_con_altura()`: flood fill que cuenta la altura (más de un nivel de diferencia = acantilado) y
   los objetos que bloquean. `conectividad.py` no sirve aquí: solo mira la colisión de tiles y daría
   100 % con la montaña entera inescalable. Comprueba el inicio al nivel 0, la boca al 3, que ningún
   objeto quede tras un acantilado y que la subida siga abierta con los objetos puestos.
   Primera versión mal planteada: exigía que la casilla de cada objeto fuera alcanzable **con** los
   objetos puestos, y un pino bloquea su propia casilla (el mismo error de concepto que los Perdidos del
   ocaso). Separado en dos pasadas: terreno+altura para los objetos, y con objetos para la subida.
2. `Nivel.salida()` comprueba la casilla de llegada en el nivel de destino (ni colisión ni objeto que
   bloquee) antes de escribir. Lo enseñó un fallo real: la primera versión volvía a (20,31), que es la
   fila del muro sur de roca de las afueras; `validar_puertas` lo decía, pero con el nivel ya escrito.

## Verificación

- `gen_niveles_av19.py` → «montana: subida abierta de (13, 33) (nivel 0) a la boca (13, 5) (nivel 3);
  608/620 casillas andables con los objetos puestos, 12 tras un acantilado o un canto (adrede)».
- `conectividad.py` EXIT=0: `montana_gorran 620/620 (100 %)`, 20 objetos alcanzables.
- `validar_enlaces.py` 188 puertas, 1 problema (el conocido de `interior_pesca_caza`, rama de Codex);
  `validar_puertas.py` 13 (12 de HEAD + ese); `validar_mundo_unido.py` 1 (el mismo); `validar_todo.py`
  3 errores y 23 avisos, los mismos que antes.
- `juego --nivel assets/levels/montana_gorran.json 3` carga y vuelca frame.
- Plano `out/planos/montana_gorran.png` y captura `out/planos/captura_juego_montana.png`. **El plano es
  de planta: no dibuja la altura.**

## Pendiente encadenado

- AV19-04 (la cueva): la boca es el marcador `boca_cueva_gorran` en (13,5); al crear `cueva_gorran.json`
  pasa a puerta con una línea, y `salida()` comprobará sola la llegada. El cofre protegido quiere
  AV18-02; mientras, `loot` declarado como en AV18-05.
- Si Claude cambia el contrato de `alturas` al commitear, hay que ajustar las 6 llamadas a `n.altura(...)`.
