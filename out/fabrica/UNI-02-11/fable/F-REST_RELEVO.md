# F-REST — Relevo (Fable, 03/11)

Worktree `~/onegai-ramas/UNI-RESTAURA`, rama `agente/fable/UNI-RESTAURA`, base `e43db51d`. Sin push ni merge.
Comprobacion: `python3 tools/validar_todo.py` antes y despues = **0 errores, 20 avisos** (identicos). `validar_botin.py` en HEAD: sin ERROR.

## 1. afueras_ocaso — HECHA · commit `01faa552`
- Comparado por (objectId, x, y) `OR/assets/levels/afueras_ocaso.json` (102 obj.) vs `MG/assets/levels/afueras_boundington_ocaso.json` (87): hoy solo 18 objetos son solo-foto (no 34: caa habia evolucionado).
- Aplicados **13 props** (molino, baul, barriles_pila, fuego_blanco, carro_o1, carro, fuego, cajas_almacen, runa, runa_b, columna_rota, arbol_grande, abrevadero): todos existen en los catalogos de caa, caen en casilla transitable (gid 1/3) y sin colision con otro objeto.
- Fuera (5): `cofre` (16,2) y `juncos` (24,9) = casillas que hoy ocupan `botin_afueras_cofre_40` / `botin_afueras_escondite`; los 3 `perdido_*` con patrulla no son decorado y pisarian la escena de los actores (decision de Oriol si entran: estan listados en el comentario del generador).
- Tambien en el generador `tools/gen_ocaso_huida.py` (`DECORADO_QWEN_OCASO`, usado por `afueras_ocaso()`), para que un regenerado no los pierda. `gen_ocaso_huida.py --seco` corre bien. Nombre viejo `afueras_ocaso` NO restaurado.

## 2. ciudad_centro — DESCARTADA (sin commit)
- `src/HUD/Pergamino/CartelesPuertas.cpp` dibuja en juego el cartel de cada puerta que lleva a un lugar con nombre. En la foto cada tienda tiene 5 `cartel_*`; caa dejo 1 por tienda a proposito (15 ids, todos presentes). Los ~60 extra son duplicados viejos.
- `cartel_coliseo`/`puerta_coliseo` (39,39 -> interior_coliseo): en caa el coliseo se entra desde `ciudad_oeste.json`; ponerlo tambien en el centro duplicaria el acceso (validar_todo ya avisa de interiores en 2 edificios).
- `estrado_sermon` x3 (39..41,52): en caa la iglesia esta en (56,52) y (40..41,52) son casillas de edificio (gid 89, colision); ademas `tools/parche_secta.py:140` cambia «Sandes, desde el estrado» por «desde la escalera»: el estrado se quito a proposito (ESCENAS-04 en caa = verina_estrado + captor_estrado en 52..56,54).
- `puerta_plaza_de_las_razas`: ya existe en caa (6 casillas, fila 0). Los 4 `botin_*` solo-foto pertenecen a la tarea 3.

## 3. Botin — BLOQUEADA (sin commit; cambios revertidos con git-safe checkout)
- `python3 tools/repartir_botin.py` (semilla fija) sobre HEAD **modifica 25 niveles** (+545/-444) y despues `validar_botin.py` da `ERROR Reparto supera presupuesto: 2/cueva/entorno 40`. En HEAD sin tocar, validar_botin pasa.
- Es decir: los niveles de caa NO son la salida del repartir_botin.py actual (hay reparto a mano o posterior: PO-57, E5 economia a 4 dias, rutas H5). Regenerar hoy rompe la validacion y mueve cofres (p.ej. `botin_afueras_cofre_40` del ocaso de (16,2) a (21,1); quita `botin_afueras_escondite`).
- Los 12 cofres solo-foto (surysal escondite, oeste barril, herreria escondite_20+armero_vacio, armeria/biblioteca_p1/casa_te/castillo/castillo_p1/coliseo, cueva_gorran `botin_cueva_*`+cofre_tesoro, afueras/montana) son el reparto viejo AV18-05b, anterior a PO-57: no se copian a mano (regla de la tarea).
- Para desbloquear: que quien lleve `repartir_botin.py` (Codex, PO-57) lo ponga al dia con la economia a 4 dias y el ocaso; entonces regenerar y comparar.

## 4. Copias de Finder « 2» — DESCARTADA (sin trabajo perdido, sin parche)
- Los 7 `.diff` van `--- original (caa)` / `+++ copia « 2»`. En los 7 la copia es **mas vieja**:
  - `CMakeLists 2.txt`: +27/-183; la copia aun lista BattleMenuState/TargetSelection/QuestLogState/demo_battle (borrados a proposito por NET-E3+E4) y le faltan 26 tags de caa (PART-02d, ESCENAS-01, ROSITA-01, ...).
  - `ApplicationPantallas 2.cpp` (+12/-201), `CartasJugar 2.cpp` (+15/-49, sin EH-16), `InterfazPergamino 2.cpp` (+55/-55, sin MAPA-01/02, PERF-2, PNJ-02, AV15-02/AV16-06: version anterior de misionSeguida y textoDePuerta).
  - `PergaminoGL 2.h`: sin COMBATE-01 (GolpesEnPantalla) ni AV13-10 (SueloCombate). `demo_boundington_partida 2.h`: sin H-DIA1-1, G4-02, FANTASMA-01.
  - `LevelEditorPaleta 2.cpp`: `sagas_de_la_locura/sagas_del_abismo` vs caa `drow_*` (races.json y deities.json de caa usan `drow_*`): nombre viejo.
- Se pueden borrar las 7 copias « 2» del Finder sin perder nada.

## 5. Imagenes — HECHA (parcial) · commit `92b3c0a5`
- `con.png`: copiado de OR como `assets/textures/pixellab/lote2/constitucion.png` (16x16 RGBA) y `pixellab/iconos.json` `"con"` apunta al nuevo nombre (unica referencia; `tests/Game/RegistroSpritesTests.cpp:188` es un fixture temporal, no toca). `tools/pixellab/importar_iconos.py` -> `iconos.png` identico byte a byte (la celda ya estaba pintada).
- `ojos_grande.png`: **no se regenera, descartada**: `tools/capa_ojos_iris.py` excluye la talla `grande` a proposito (`TALLAS_SIN_OJOS`: la mascara coge piel de la mejilla) y nada en caa referencia `ojos_grande` (las capas viven en `personajes/cuerpos/ojos_<talla>.png`: medio, bajo, pequeno). Nota: en este Mac falta `scipy` para ese script.
- `Untitled-1.psd` (3,0 MB): no esta en el repo del worktree (solo en la copia de Oriol): nada que hacer; conservarlo fuera como fuente de arte. Si se quiere, `monumentos/tienda4direcciones.psd` y `universidad.psd` SI estan trackeados hoy.

## Pendiente de Oriol
- Tarea 3 (repartir_botin.py desincronizado) y los 3 `perdido_*` del ocaso (tarea 1). No se ha tocado `sesiones/` ni `TABLERO.md` (el encargo limitaba las escrituras al worktree y a este relevo).
