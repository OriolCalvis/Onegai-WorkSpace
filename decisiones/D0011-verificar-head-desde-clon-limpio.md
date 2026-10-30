# D0011 — Verificar `HEAD` desde un clon limpio antes de cerrar nada

- **Estado:** `proposed` — **espera a Oriol.** No se ha tocado ningún gancho, ni
  el `pre-commit`, ni el `pre-push`, ni CI: la propuesta necesita su visto bueno
  porque toca la puerta por la que pasan los commits de todos.
- **Fecha:** 2026-10-01 · **Autor:** opencode.
- **Alcance:** el criterio de cierre de cualquier tarea del Gantt, y —si se
  acepta— un `pre-push` que configure y construya `juego` desde un clon limpio.
  **No cambia** el plan de estabilización ni las leyes del repo.

## Contexto: lo que pasó el 1 de octubre

Un clon limpio de `HEAD` **no compilaba**:

```
src/Engine/Application.cpp:867: use of undeclared identifier 'publishSheetSkills'
src/Engine/Application.cpp:905: no member named 'tickRound' in 'BattleState'
```

Los dos llamadores estaban commiteados; las declaraciones y definiciones
vivían solo en árboles de trabajo. La causa, en el orden en que se
descubrió, la encontraron **tres sesiones distintas el mismo día** —
`78cbc84` (H0-06), `8b338a4` y `9eeb27b` (H1-01), más `0476bb6` — y cada una
creyó que era suya.

Dos cosas lo hicieron posible, y las dos importan por separado:

1. **Se verificaba el árbol, no `HEAD`.** Todo el mundo construye sobre su
   árbol de trabajo, donde el código está. El fallo solo se ve en el árbol
   del otro. Agravante: los 111 objetivos de la suite GL-free **no enlazan
   `motor_core`**, que es donde vive `Application.cpp`, así que la rutina
   diaria de «construir y pasar las pruebas» no toca ni una línea del
   ejecutable real.
2. **El índice git es compartido.** El `commit` de cualquiera recoge lo que
   otro dejó *staged*. Pasó dos veces en una hora: `5504b63` se llevó los 4
   ficheros de H2-02, `78cbc84` se llevó 127 líneas mías con un mensaje que
   solo hablaba de H0-06. En los dos casos el contenido era idéntico al que
   el otro committer tenía en su árbol, así que no hubo conflicto ni trabajo
   perdido — pero la **atribución quedó mal** y la duplicación se pagó en
   tiempo de CPU, no en romper nada.

## Lo que hay que decidir

**A. El criterio de cierre.** «La tarea está verde» pasa a significar «verde
en un clon limpio de lo que se va a commitear», no «verde en mi árbol». Es
una frase en el protocolo y un renglón en el `CONTRATO` de la fila del
TABLERO.

**B. La guarda permanente.** Un `pre-push` (o un paso de CI) que haga:

```
git archive HEAD | tar -x -C $(mktemp -d)
cmake -S . -B out/build/head -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/head --target juego
```

Coste: unos minutos por push. Beneficio: ningún `HEAD` roto llega a existir, y
el fallo aparece en la sesión que lo introdujo, que es la única que puede
arreglarlo. **Justificación del coste:** hoy el mismo fallo lo encontramos
tres sesiones después, y el arreglo fue de 3 minutos — pero solo porque las
piezas seguían en los árboles de trabajo. Si un tercer agente hubiera
cerrado su terminal, `HEAD` necesitaría a 5 personas para deshacerlo.

**C. El índice compartido.** Hoy la única norma es «des-stagea lo ajeno justo
antes de commitear», y es frágil: entre el `add` y el `commit` cabe otra
sesión (me pasó). Opciones: (i) un `git commit <rutas>` explícito siempre,
(ii) un lock por fichero staged, o (iii) aceptarlo y escribir el rastro de cada
sesión para poder reconciliar atribuciones. **No se propone nada todavía:**
esto decide Oriol o se decide con la experiencia de los dos incidentes.

## Lo que NO se propone

- No se toca el gancho actual: `pre-commit` ya mira el índice
  (`ddad0d7`) y funciona; el problema es que el índice no es de nadie.
- No se actualiza `linea_base_core.json` ni se re-congela nada: eso es D0007.
- No se reescribe historia. `78cbc84` y `9eeb27b` quedan como están, con la
  atribución corregida **en el registro** (`sesiones/` + TABLERO), que es
  donde se busca cuando algo no cuadra.

## Cómo se comprueba

- Con el arreglo dentro: `git archive HEAD` a `/tmp`, configurar y construir
  `juego` → **3 de 3 ejecutables** y 207 de 210 objetivos (los 3 que faltan
  son tests que van por delante del motor, de sesiones vivas, documentados en
  la sesión del 01/10).
- Sin el arreglo: los dos `error:` de arriba. Está medido en los dos
  sentidos.
