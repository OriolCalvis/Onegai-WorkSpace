# Integración de UNI-02-11 y UNI-RESTAURA en el checkout de Software

- **Fecha:** 2026-11-04 (noche)
- **Autor:** Claude (Code tab, Fable 5.1)
- **Encargo:** Oriol, tras el repaso de estado: «Integrar UNI-02-11». Aplica su decisión 1.1 del 03/11 (`out/fabrica/UNI-02-11/DECISIONS_PENDENTS_ORIOL_RESPOSTES.md`): la rama unificada pasa a ser la base de todo.
- **Commits (rama `recuperat/software-copia-30-10`, el checkout de Oriol):** `351d7fa1` (fusión de `agente/claude/UNI-02-11` @ `916e75b0`), `6189fc7f` (fusión de `agente/fable/UNI-RESTAURA` @ `92b3c0a5`) + el commit de cierre con esta nota.
- **Deja esperando a Oriol:** nada bloqueante. Ver «Para Oriol».

## Situación de partida

- El checkout de Oriol (`Software/`) estaba en `recuperat/software-copia-30-10`: la foto del 30/10 tras perder el `.git`, más la foto de trabajo del 04/11 y los dos commits de ECC. Cuatro commits, raíz `22c6a7d6`.
- La feina viva estaba en `agente/claude/UNI-02-11` (167 commits, linaje nube, raíz `95ef783a`): unificación de todo lo recuperado, suite gl-free 290/290, beats del Ocaso, economía a 4 días y, el 04/11, los disparadores de D0026 y la galería de capturas.
- `agente/fable/UNI-RESTAURA` (2 commits sobre `e43db51d`): zonas 4.1 y 4.5 de la restauración.
- **Las dos historias no comparten ningún ancestro** (`git merge-base` vacío). Entre los árboles: 388 ficheros distintos, 812 solo en UNI, 13.481 solo en la foto (casi todos la copia anidada `MotorGraphico-main/.ui-puertas-01/` y `MotorGraphico-main/Claude outputs/`).
- El worktree `~/onegai-ramas/UNI-02-11` tenía 5 ficheros modificados sin commitear con mtime de hace minutos (`quests.json`, `boundington_ocaso.json`, `eventos/boundington.json`, `explorar_boundington.cpp`, `fabricar_guardados.h`): **otra sesión trabaja ahí en vivo**. Regla 2 de la bitácora: no se toca ni se commitea. Por eso no se fusionó UNI-RESTAURA *dentro* de UNI-02-11 (habría movido su HEAD bajo los pies de esa sesión) sino que las dos ramas se fusionan en el checkout de Oriol.

## Qué se ha hecho

1. `git stash` de `.obsidian/workspace.json` (modificado en el árbol; Obsidian lo reescribe solo).
2. `git merge --no-commit --no-ff --allow-unrelated-histories -X theirs agente/claude/UNI-02-11`: sin conflictos pendientes. Con base vacía, todo fichero común se resuelve a favor de UNI-02-11.
3. Recuperados «los nuestros» (más nuevos en la foto que en la rama): `TABLERO.md` (la fila de ECC), `sesiones/INDICE.md`, `.obsidian/workspace.json` y todo `out/` que ya existía (informes de UNI-02-11 con las **respuestas de Oriol** del 03/11, `PROGRESO.md` de REPLICA-R1, `agy/consumo.jsonl` y `nocturno.log`). Los 251 ficheros nuevos de `out/fabrica/UNI-02-11` de la rama entran igual.
4. Fuera las 8 copias « 2» del Finder que UNI-02-11 ya había retirado (rompían `linea_base_core_configura`): `CMakeLists 2.txt`, `ApplicationPantallas 2.cpp`, `InterfazPergamino 2.cpp`, `CartasJugar 2.cpp`, `PergaminoGL 2.h`, `LevelEditorPaleta 2.cpp`, `demo_boundington_partida 2.h`, `props 2.json`. Siguen en la historia de la foto y, con su diff, en `out/fabrica/UNI-02-11/copias_finder/` (zona 4.4, pendiente de Fable).
5. Comprobación: `git diff --cached agente/claude/UNI-02-11 -- MotorGraphico-main/MotorGraphico` → **0 modificados, 226 solo añadidos**. Es decir, el motor es exactamente el de UNI-02-11 más lo que solo tenía la foto y se conserva a propósito: `Claude outputs/` (121), `monumentos/` y `race_npc_sprites/` (los 84 que NET-E3+E4 borró como no usados; ZONAS_PARA_FABLE §2), `planificacion/replica/`, `.claude_tmp/`, `assets.zip`, `Untitled-1.psd`, `ojos_grande.png`, `tools/av19_ocaso.py` y `tools/probar_cambios.sh`.
6. Commit `351d7fa1`. Después `git merge --no-ff agente/fable/UNI-RESTAURA` → `6189fc7f` (4 ficheros, sin solapamiento con los commits nuevos de UNI).
7. El `stash pop` no pudo aplicarse: Obsidian había vuelto a escribir `workspace.json` entre medias. El árbol conserva la versión viva; la de antes queda en `stash@{0}` («workspace.json abans de la integracio UNI-02-11»). Es solo estado del editor; se puede tirar (`git stash drop`) cuando Oriol quiera.

8. **Primera pasada de la suite: 300/303.** Los 3 rojos (`test_aventuras_monsterid`, `catalog_content_validation`, `test_validar_todo`) venían de 5 assets que solo tenía la foto y que UNI-02-11 había retirado a propósito: `assets/levels/afueras_ocaso.json` + `assets/maps/afueras_ocaso.tmx` (el nombre viejo del nivel; 10 puertas sin vuelta y «fuera del mundo» en `validar_mundo_unido`; ZONAS_PARA_FABLE 4.1: «no recuperar el nombre viejo») y `ojos_grande.png`, `capa_medio_sin_piernas.png`, `torso_medio_sin_piernas.png` (PNG sin su `.filas`; `trocejar_atlas_personajes.py --comprobar` los daba por faltantes). Retirados con `git rm` en el commit de cierre; siguen en la historia de la foto (`5ee2be05`). `ojos_grande` se regenera con `tools/capa_ojos_iris.py` (zona 4.5) si hace falta.

## Verificación

En el checkout de Oriol (`Software/`), preset `macos-debug` (`out/build/macos-debug`, nuevo; `build/` y `build-release/` viejos no se han tocado):

- Configuración + compilación completa (`juego`, `level_editor`, `motor_editor`): **74 s**, 0 errores (21:14–21:16).
- `ctest -L gl-free -j 6` (con las `lenta`), primera pasada sobre `6189fc7f`: **300/303 en 207 s**; los 3 rojos, arriba.
- Tras retirar los 5 assets y resincronizar `copiar_assets`: los 3 pasan (`test_aventuras_monsterid` 3,0 s, `catalog_content_validation` 1,1 s, `test_validar_todo` 3,0 s).
- Suite completa de nuevo tras la limpieza (`ctest -L gl-free -j 6`): **303/303 en 152 s**.
- No se ha jugado (ventana GL): los vistos visuales de 1.2 siguen pendientes.

## Segundo merge (más tarde, misma noche): `f45edf58`

La sesión que trabaja en el worktree `UNI-02-11` commiteó **E-HALLAZGOS-01** (bucle del explorador `b2_sectarios` y diario del Ocaso: salir, final, quedarse, avisar; 23 ficheros, 7 de motor/assets/tests y el resto evidencias). Oriol dijo «Sí, fusiona»:

- `git merge --no-ff agente/claude/UNI-02-11` → **`0aa57918`**. Primer merge normal tras la integración: ya hay ancestro común y nada solapa con los commits locales.
- Un conflicto, `out/fabrica/UNI-02-11/capturas/CAPTURAS.md`: la foto del 04/11 iba 36 líneas por detrás de la rama (le faltaba la sección de E-DISPARADORES-2). Se toma la versión de la rama entera.
- Verificación: recompilación incremental sin errores (18 s). `ctest -L gl-free -j 6` sobre `0aa57918`: **301/303 en 1.337 s**, con 2 **Timeout** (`test_montaje_filas`, `linea_base_core`) porque la otra sesión pasaba a la vez sus propias pruebas en el mismo Mac (carga media 11–13; esa sesión relanzó justo esos dos tests). Repetidos aparte: `linea_base_core` **pasa en 38 s**; `test_montaje_filas` **pasa en 177 s** (en las pasadas verdes anteriores tardaba 135–152 s). Ningún fichero de la fusión toca atlas ni línea base: fueron timeouts de carga, no regresiones.
- El worktree `UNI-02-11` sigue con trabajo sin commitear de esa sesión (`CMakeLists.txt`, `actos.json`, `moral.json`, `demo_boundington_partida.h`, `demo_game_session.cpp`): no se toca.

## Para Oriol

1. **Tu rama `recuperat/software-copia-30-10` es ahora la base de integración** (contiene UNI-02-11 y UNI-RESTAURA). Lo que venga nuevo en `agente/claude/UNI-02-11` desde `916e75b0` (la sesión que trabaja ahí ahora) ya tiene ancestro común y se fusiona con un merge normal.
2. `agy_nocturno.sh` (punto 1.6 de tus respuestas) debería apuntar a esta rama como base.
3. Sigue sin hacerse push a ningún sitio (PO-04).
4. `stash@{0}` con tu `workspace.json` de antes: se puede borrar.
5. Zonas 4.2–4.4 de la restauración siguen abiertas en `agente/fable/UNI-RESTAURA`; su worktree y el de UNI-02-11 **no se han borrado** (norma de limpieza: solo tandas acabadas e integradas).
