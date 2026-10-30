# 2026-10-20 · qwen · AV19-09 el Casco en llamas (96x64), la huida y las Afueras al ocaso

Cola del orquestador (`orquestador/qwen.md`, tarea 1 de 6). Rama `agente/qwen/AV19-09`, base
`claude/escenas01` en **`bfc5a22`** (comprobado con `git log -1`: HEAD se movió de `d37d1f6` a
`bfc5a22` mientras leía la bitácora, así que la base es la que dice Oriol, no la que vi al entrar).
Worktree `~/onegai-ramas/qwen-AV19-09`.

## Qué se hizo

Tres niveles nuevos/rehechos con **`tools/av19_ocaso.py`** (nuevo, determinista y reejecutable), que
lee la hoja «Otros» de `out/fable_ciudad02/boundington_ciudad8.xlsx`:

| nivel | tamaño | objetos | inicio | suelo |
|---|---|---|---|---|
| `ciudad_centro_ocaso` | 96x64 | 1145 | (48,59) | 169 casillas del Excel sobre los mapas vivos |
| `afueras_ocaso` | 40x32 | 70 | (18,2) | 10 casillas del Excel |
| `huida_boundington` | 40x40 | 21 | (37,22) | pintado entero desde el Excel (529 casillas) |

**El suelo no se repinta desde los colores.** Se compone de los TMX vivos (`ciudad_oeste` 32x64 a la
izquierda + `ciudad_centro` 64x64 a la derecha; 32+64=96, que es justo el ancho del bloque de Oriol) y
solo se cambia la casilla donde el Excel dice otra **clase**. Así se conservan las variantes reales de
cada edificio (`edifici_o0..o3`, `rio_muro_izq/der/ambos`, `puente_pretil_*`, `adoquin_ajedrez`), que
el color del Excel no distingue. La primera pasada comparaba gids en vez de clases y «arreglaba» 917
casillas: aplanaba `edifici_o3` a `edifici_o0` y se cargaba el dibujo de los edificios.

Los objetos salen de los **comentarios** de las celdas (1122 en el Casco: 852 `fuego`, 72 carros, 51
Perdidos, toneles, cajas, tendederos, runas, `verina_joven` en (90,19), `edul` en (79,33),
`huida_a_egaroth` en (64,62), el cofre en (33,41)). Si Oriol borró la letra de la celda, el objeto no
está (regla de la leyenda).

`afueras_ocaso` se construye ahora (aunque sea la tarea 2) porque las puertas del Casco y de la huida
apuntan a él: sin el nivel, `validar_puertas` y `validar_mundo_unido` no pueden pasar y la rama queda
roja. La tarea 2 lo amplía con lo que pide la ficha (molino, fuego blanco, plaza 7x7 del árbol,
huertos) y lo deja en Excel para Oriol.

## Las cinco decisiones que cambian lo que pedía la ficha (Excel manda)

1. **El paso a las Afueras está al SUR, no al oeste.** Oriol pone las cinco `P` en (62..66,63) →
   `afueras_ocaso` (16..20,1), o sea la puerta sur del Casco contra el borde norte de las Afueras. La
   ficha decía «por el oeste»; se aplica el Excel y queda dicho aquí.
2. **La salida de la huida está al OESTE, no al norte.** `fin_de_la_huida` en (1,13); la llegada desde
   las Afueras entra por el borde **este** (39,18..22). La ruta forzada real es: Casco → sur →
   Afueras → oeste de las Afueras → este de la huida → oeste → Egaroth.
3. **El id es `afueras_ocaso`**, no `afueras_boundington_ocaso` (que es como lo llama el Excel y el
   trabajo anterior de Fable). Lo dicen los encargos de Codex y la ficha: los ids de nivel no se
   cambian. El generador traduce el alias.
4. **`fin_de_la_huida` lleva el `eventId` `huida_a_egaroth`**, que es «donde hoy lleva
   `huida_a_egaroth`»: ese marcador nunca tuvo `targetLevel`, solo evento. Así los beats que ya existen
   siguen saltando y no hace falta inventar un destino nuevo.
5. **La ficha de la huida va en `calles`, no en `lugares`.** `tests/Game/LugaresTests.cpp` exige que
   toda ficha de `lugares` sea un nivel `interior_*` y que haya exactamente una por puerta de interior
   de las tres ciudades de día, sin sobrantes: poner la huida ahí rompía dos comprobaciones (y lo hizo
   en la primera pasada). En `calles` sí, que es donde ya está el Casco en llamas.

## Los cuatro sitios de escena (con casilla aunque Oriol no los marcara)

Celdas comprobadas libres y alcanzables por el generador (si no lo están, falla en vez de escribirlas):

- **Estrado del sermón**, sobre lo que queda de las escaleras de la iglesia: `estrado_sermon` en
  (70,49)-(70,50), `tela_gris_ardiendo` en (69,48) y (69,51). Sandes (71,50); dos fanáticos en lo alto
  de las escaleras (70,48) y (70,51); dos sectarios en los portales (71,47) y (71,52). Combate: la
  plaza de la iglesia, x 66..76, y 46..54.
- **Esquina de la Taberna Humilde** (la taberna está en x 46..50, y 56..58, con la puerta en (48,58);
  el jugador empieza en (48,59), delante): Rosita en medio de la calle (48,60) y doce Perdidos en
  (45..50, 59..61). Combate: x 44..55, y 59..62.
- **Patio de los toneles** (Pulga): el patio detrás de la taberna, x 44..51, y 52..55, con
  `barriles_pila` en (44..46,52) y `barril` en (47,53); la boca del pasillo trasero lleva
  `puerta_sin_cerrojo` en (44,56) y el pasillo (44,57)-(44,59) se comprueba libre en cada pasada.
  Pulga (45,54), el jugador (46,54) y dos huecos más.
- **Barricada de Verina**: Oriol la pone en (90,19) (no en «la salida oeste» de la ficha), con el fuego
  cerrando la calle justo al este. Añado `carro` (87,19) y `carro_o1` (87,20) para que sea una
  barricada de verdad; dos cultistas en (89,19) y (90,20) y **tres casillas libres delante de ella**:
  (88,19), (88,20), (89,20). Combate: x 86..90, y 18..21.
- **Velas y cruces**: `vela_ventana` en seis ventanas (46,56), (47,56), (49,56), (50,56), (72,47),
  (73,48) y `cruz_hollin` en seis puertas (46,58), (50,58), (52,55), (53,56), (72,49), (73,52). Van
  **sobre casilla de edificio** para no estrechar ninguna calle, y por eso llevan `flotante: true`:
  `conectividad.py` usa esa marca para lo que está donde el jugador no puede pisar a propósito
  (BLOQUES-04, los barcos sobre el agua). Sin ella, los doce daban «puertas inalcanzables».
  No uso `candelabro` como sugería la ficha: está en `mobles_kit.json` (catálogo ajeno), bloquea el
  paso y no es flotante.

## Marcadores nuevos para Pixel (10 ids en `boundington_av19_objetos.json`)

`paso_ocaso_a_afueras`, `paso_afueras_a_ocaso`, `paso_afueras_a_huida`, `paso_huida_a_afueras` (los
cuatro invisibles, `spriteId -1`, como los `paso_*` de `boundington_pasos.json`) · `fin_de_la_huida`
(spriteId 23, `eventId huida_a_egaroth`) · **`estrado_sermon`** y **`tela_gris_ardiendo`** (spriteId 3
provisional) · **`puerta_sin_cerrojo`** (spriteId 23) · **`vela_ventana`** y **`cruz_hollin`**
(`spriteId -1`, flotantes: van en la pared). Los cuatro en negrita más los dos últimos son los que
esperan arte.

## Botín: cinco objetos pasan a decorado, registrado como PO-57

El Excel marca botín (un cofre en (33,41) del Casco; barril, saco, cofre y escondite en las Afueras).
**No cabe:** el presupuesto del día 3 ya está entero comprometido por las instancias con
`disponibleDia: 3` que `repartir_botin.py` puso en los mapas DE DÍA (3/ciudad entorno 20/20 y
cofres_visibles 40/40). Con botín nativo, `validar_botin.py` dice «Reparto supera presupuesto». Y hay
un efecto que los números no muestran: el botín del día 3 de ciudad y Afueras está en mapas que el
día 3 no se pisan. Se pasan a su equivalente de decorado (`barril`, `sacos`, `cofre`, `juncos`; mismo
`blocksMovement`, ningún paso cambia) y queda en **PO-57** con la recomendación de que
`repartir_botin.py` reparta el día 3 en los mapas del Ocaso. No ejecuto `repartir_botin.py`: es la
herramienta de Codex y redistribuiría los 51 niveles.

## Diferencias con el Excel (las 169 del Casco, agrupadas)

Todas son de suelo y casi todas caen en dos sitios:

- **La iglesia ya no está.** De las 77 casillas de `viejo_iglesia` del mapa de día, Oriol deja 10
  sueltas entre adoquín, casas, grava y ruinas: `viejo_iglesia -> adoquin_limpio` 30, `-> edifici_o0`
  28, `-> tierra_piedras` 9, más `ciudad_ruinas -> adoquin_limpio` 10 y `adoquin_limpio -> ciudad_ruinas`
  8. Es la zona (x 69..93, y 42..58) y explica que el estrado del sermón vaya delante de una ruina.
- **La muralla sur se abre donde está la `P`**: `viejo_muralla -> sendero_tierra` en (12..16,63) y
  (66,63), y `adoquin_gastado -> sendero_tierra` en (62..65,63). Sin eso las cinco puertas caen dentro
  de la muralla.
- El resto son ajustes finos: `tierra_piedras -> adoquin_limpio` 14 (la columna (93,43..53)),
  `adoquin_limpio -> edifici_o0` 13, `marmol_real -> adoquin_limpio` 8 (el patio delante de la
  iglesia), el muelle en (48,59) donde Oriol pone el ★, y el puente/muro del río en (1,46..48) que
  pasa a tierra (el puente a Surysal ya no es salida en el Ocaso: el bloque no lleva ninguna puerta a
  `ciudad_surysal`).
- **Afueras**: 10 casillas, todas `roca -> pradera`/`-> sendero_tierra` en el borde oeste, que es por
  donde sale el paso a la huida (1,17..21). Sin eso el paso cae en roca.
- **Huida**: 529 casillas pintadas enteras desde el Excel (`hierba_seca -> sendero_tierra` 274,
  `-> roca` 215, `-> pradera` 40), porque no hay mapa anterior.

## Dos trampas que han mordido de verdad

1. **La colisión no se puede sacar de la unión de todos los TMX.** `test_map.tmx`,
   `mazmorra_64x64.tmx` y `ejemplo_mapa.tmx` declaran `hierba_seca` como infranqueable. Con la unión
   de `assets/maps/` entero, los Campos del Otoño salían con 274 casillas andables de 1600: un
   laberinto de hierba seca, y las cinco puertas de la huida inalcanzables. Ahora la tabla se hace con
   `terreno_iso.json` + **solo los TMX del manifiesto de boundington** (más los tres de los que se
   compone el suelo).
2. **Un catálogo que solo añade no se corrige nunca.** La primera pasada escribió `cruz_hollin` sin
   `flotante`; cuando lo añadí, `escribir_catalogo` vio que el id ya estaba y no lo tocó, así que
   `conectividad.py` siguió en rojo dos pasadas más. Ahora reconcilia: añade lo que falta **y**
   sustituye lo que ha cambiado, y dice cuántos de cada.

## Verificación

- `validar_todo.py`: **2 errores, los dos heredados** y ninguno de esta tarea. `validar_pnj` (seis PNJ
  de la Barriada sin fila propia en `pnj_*`: zarela, guerrero_gitano, vecina_surysal…) se comprueba
  **en el árbol principal en `bfc5a22`**, que da lo mismo; y la daga de Aigren es el que la ficha ya
  daba por conocido. Los 22 avisos son los de siempre (deidades y `mundi_landmass_*`).
- `validar_puertas.py`: 126 niveles, 365 puertas, **0 errores**, 12 avisos (todos ajenos:
  `ejemplos_objetos`, `mundi_landmass_*`, `recuerdo_taberna`).
- `conectividad.py`: `ciudad_centro_ocaso` 4179/4181 (99 %), `afueras_ocaso` 1021/1021,
  `huida_boundington` 1385/1385, **0 errores**. Las dos casillas que faltan son (75,49) y (76,50),
  dos huecos entre la ruina de la iglesia: se andan pero no se llega ni quitando el fuego, y por eso
  los sectarios van a los portales de la plaza.
- `validar_mundo_unido.py`: en verde tras añadir los dos niveles nuevos a EXCLUIDOS con su motivo
  (cuelgan del Casco, que ya estaba excluido desde AV19-06).
- `validar_botin.py`: 50 reutilizables, 12 únicos, 30 tablas, 58 instancias, **0 errores**.
- **`ctest -L gl-free`: 222 pasan, 14 fallan, 0 «Not Run».** Los 14 son **exactamente los mismos en la
  línea base**: revertí mis seis ficheros, aparté los cinco nuevos, resincronicé `copiar_assets` y volví
  a ejecutar solo esos 14 en el mismo build (`/tmp/q1910_base.log`) — `test_pergamino`,
  `test_ruta_casillas`, `test_botin_equipo`, `test_recompensas_economia`, `test_lugares`,
  `test_catalog_content`, `test_aventuras_monsterid`, `catalog_content_validation`, `test_nivel_excel`,
  `test_kit_basico`, `linea_base_core`, `linea_base_core_configura`, `test_validar_todo`,
  `test_clases_sp`. **Ninguno es mío.** Dos apuntes del entorno: `test_nivel_excel`,
  `catalog_content_validation` y `test_recompensas_economia` son Python y ctest los lanza con
  `/opt/homebrew/bin/python3`, que no tiene `openpyxl` (PEP 668 no deja instalárselo sin
  `--break-system-packages`, y no lo he hecho); y `test_lugares` pide 17 fichas cuando la base ya tiene
  18 (`git show HEAD:…/lugares.json`).
- Ruta forzada: el generador abre el fuego y los carros y vuelve a inundar. En el Casco, 1006 casillas
  alcanzables de 1021 andables en las Afueras y 274 → 1385 en la huida; lo que queda cerrado lo cierra
  el fuego o la barricada, no el borde del mapa (las dos excepciones son los huecos de la ruina).
- Capturas: `out/planos/ciudad_centro_ocaso.png` (1145 objetos), `afueras_ocaso.png` (70),
  `huida_boundington.png` (21). **No las he jugado en el motor**: son datos nuevos y el binario que
  compila en este worktree es el de la base; la comprobación es la de los validadores y el flood fill.
- `tools/av19_ocaso.py --seco` dos veces seguidas: mismo resultado (idempotente).

## Trabajo ajeno encontrado (no tocado, documentado)

En `assets/levels/` hay `.bak` sin versionar de todos los niveles, y entre ellos
`huida_boundington.json.bak`, `afueras_boundington_ocaso.json.bak` y un `ciudad_centro_ocaso.json.bak`
de **1122 objetos** (contra 188 del versionado): es el mismo bloque del Excel aplicado por alguien el
19/10 a las 13:53, con los objetos en coordenadas de 96 columnas sobre un TMX que seguía siendo de
64x64 (el primer objeto cae en x=81, fuera del mapa). Por eso no carga. No lo he deshecho ni
commiteado; mi versión parte del Excel, no de esos `.bak`. También sigue en pie `_to_delete/`
(`bak_excel_18-10`, `finder_dupes_19-10`, `niveles_duplicados_18-10`), que es de donde salen esas
copias.

## Desviación de protocolo: un worktree para toda la cola

La ficha pide un worktree por tarea. El repo pesa **17 GB**, un checkout son ~1,4 GB y un build
~0,5-1 GB, y el disco está al **95 % (23 GB libres)**: seis worktrees con su build no caben. Así que
uso **uno** (`~/onegai-ramas/qwen-AV19-09`) con una rama por tarea **encadenadas** en el orden de la
cola (AV19-11 saldrá de `agente/qwen/AV19-09`, etc.), y un solo build (`out/build/qwen-19-10`) que se
reutiliza: como no toco C++, las tareas siguientes no recompilan. Se fusionan en el orden de la cola.

## Entorno

`openpyxl` y `numpy` instalados con `pip --user` para `/Library/Developer/CommandLineTools/usr/bin/python3`
(el `python3` del PATH mínimo del agente): las `tools/` que leen el Excel y `validar_pnj.py` no arrancan
sin ellos. Reversible con `pip uninstall`. Al `python3` de Homebrew no se los he podido instalar (PEP
668) y es el que usa ctest para los tests Python.
