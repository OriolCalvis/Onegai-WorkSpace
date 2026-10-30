# Relevo de Claude · 08/10/2026, tarde (BND-ESCENAS-01)

Para la siguiente ventana. Sigue al de la mañana (`claude/relevo-2026-10-08.md`, cuyas §0 y §3 siguen valiendo).
Sesión: `sesiones/2026-10-08-claude-bnd-escenas.md`. Seguimiento: `FASE1-BND-01.json` y `BND-ESCENAS-01.json`.

## 1. Dónde está cada cosa

- **El canon de Boundington**: `planificacion/CANON_BOUNDINGTON_CAMPANA01.md`. §1-§6, las fichas de escena del
  vault; §7, qué pide al motor; §8, las 12 preguntas para Oriol; §9 y §10, cómo quedó en el juego y qué decidí.
- **Las fuentes**: en el vault («Onegai 2»), `corpus_extraido/THE ONEGAI PROJECT__Campan_as__Canon__Campan_a 01_
  La Matanza de Boundington _Aventura de Venides_.docx.md` [C01] y `THE ONEGAI PROJECT__PARTIDAS.docx.md` [PART].
- **Las aventuras**: `boundington_primer_dia.json` (78 beats), `boundington_segundo_dia.json` (47),
  `boundington_ocaso.json` (27). `demo_boundington` las juega por muchas rutas y exige que no quede ningún beat
  sin disparar: si añades uno, añade su ruta.

## 2. Montar el entorno (lo que funcionó hoy)

La subida del Mac al contenedor solo es fiable con ficheros pequeños (≈150 KB, uno por llamada), así que **se
compila y se juega en la VM del Mac** (`device_bash`: Ubuntu 22.04 aarch64, 4 núcleos):

1. En la VM: `~/work/base/MotorGraphico` (copia del Mac con git: base `28c9ea3`, y los commits de la sesión),
   `~/work/build` (Ninja), `~/work/kit` (cmake 3.28 y ninja de GitHub, GLFW 3.4 X11 con el guion, cabeceras X11
   de github.com/allyourcodebase/x11-headers). El kit también está en `out/claude_traspaso1008/kit_vm.tgz`.
2. Los cambios se editan en el contenedor y se pasan a la VM en un tgz por la carpeta del Mac
   (`out/claude_traspaso1008/sync/`, `device_commit_files`; en la VM, `aplica_sync.sh NOMBRE`).
3. ctest con pantalla: `LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a -s "-screen 0 1600x900x24" ctest -j4`.
   `test_application_scenes` falla a veces con -j4 bajo Xvfb y pasa solo; `linea_base_core` sigue en 44.
4. **Jugar**: `~/work/arnes/run_vm.py` (Xvfb + GLFW X11 con el guion de capturas; si la VM ya no lo tiene, está
   en `out/claude_arnes_vm/arnes_vm.tgz`, con su `LEEME.md`). Diferencias con la plataforma
   nula: el guion le da el foco a la ventana, limita a 60 fps (`GLFW_SCRIPT_FPS`) y `move` mueve el puntero de
   verdad. **Empieza siempre con `move 1500 850`**: el puntero de Xvfb nace en el centro, encima del menú del
   título, y lo selecciona. Ejemplos: `llegar.py` (crear personaje, prólogo, ciudad y F5), `skilla_noche.py`,
   `aigren.py`, `dia2.py` (cargan un guardado y juegan una escena). Rutas por la rejilla con
   `arnes/guiones/nav.py` (`nav.ASSETS` a la copia de la VM). Las hojas de contactos, con `mosaico_vm.py`, y se
   miran subiendo un jpg pequeño al contenedor.
5. Los guardados para saltar a una escena: el rápido (`saves/boundington/rapido.json`) se copia y se le tocan las
   flags y las variables del bloque `narrative`.

## 3. Preguntas abiertas para Oriol (no las decidas tú)

Las 12 del canon, §8. Las que paran el trabajo siguiente: la vida de **Sandes** (su ficha es de 105 PV), la ficha
de los **guardias**, **en qué noche es la fiesta** y **quién escribe los textos que faltan**.

## 4. Lo siguiente

1. La segunda parte del Ocaso: Sandes (opciones de [PART] y combate con `huyen`), Edul y Venides retenidos, el
   fuego de la barricada. Necesita las respuestas de §3.
2. Lo que el motor aún no hace: refuerzos, dejar las armas de verdad, las monedas (pp, pb, po), los mapas del
   Ocaso.
3. La Taberna Rica y las submisiones del plan del 05/10 (que son propuestas de Claude, no del vault).
