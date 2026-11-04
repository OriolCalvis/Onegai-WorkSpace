# E-OCASO-ATASCO — por que 0/20 acababan el Ocaso (04/11/2026)

**Veredicto: error del EXPLORADOR, no del juego.** Un jugador real no se encalla en ninguno de los 3 casos.
No se ha tocado historia, aventuras, eventos ni cinematicas: solo `MotorGraphico-main/MotorGraphico/examples/explorar_boundington.cpp`.

## Causas (archivo:linea del explorador, tras el arreglo)
1. **No evaluaba los eventos del mundo** (:51-60, :354). El juego llama a `GameSession::avaluaEventosDelMundo()`
   una vez por fotograma (`src/Engine/ApplicationBucle.cpp:340`); el explorador nunca. `bnd3_campanas`, `evt_xila_avisa_a_venides`,
   `evt_comienza_ocaso` no se ponian y el explorador los daba por «el juego pone X» y se quedaba sin nada que hacer (18/20).
   Ahora llama a `RPG::eventos::evalua` cuando la historia se queda quieta (como `evaluaEventos()` del arnes) y esas flags
   ya no figuran como externas (:299).
2. **Cinematicas con zona solo en la casilla (12,9)** (:498). El sermon de la plaza (zona 49..61 x 49..58) nunca arrancaba,
   asi que `bnd3_esc04_sermon` / `b3_plaza` no saltaban. Ahora el jugador va al centro de la zona (el arnes ya hacia (54,54)).
3. **Sin «caminar a otro nivel»** (:395-405, :509). Dia 3: con `bnd2_a_dormir` puesto, la cinematica `taberna_velas` y los
   `b2_cierre*` (auto, `lugar: interior_taberna`) exigen estar en la taberna; el explorador solo viajaba a donde habia un
   PNJ con beat. Los 2 encallamientos del dia 3 («tras b2_ancianas_s0», «tras b2_guardian_charla_s1») eran ese: el jugador
   estaba en ciudad_surysal de noche y habia que volver a la taberna. Igual con `b3_xila_avisa` (pide base_militar/cuartel).
   Ahora, sin nada mas que hacer, va a un nivel de `requires.lugar`/cinematica donde algo arranca (`llegarServe`).
Extra: `describeAtasco` imprime nivel/dia/franja y `EXPLORAR_TRAZA=1` lista que dispara cada accion al encallarse.

## Confirmacion en el juego real
`juego --guardado guardados/ocaso.json --guion tools/guiones/ocaso.txt` (guardado con el evento comienza_ocaso sin evaluar):
al cargar, el juego evalua el evento solo y salen las campanas, el explorador y la arenga sin ninguna accion del jugador.
Capturas: `capturas_atasco/ocaso_{cuartel_antes,campanas_explorador,arenga_1}.png`. Las matrices (MatrizOcasoTests) usan
la misma `evalua`. No se ha reproducido el dia 3 con guardado en el juego: es deduccion del codigo (los beats/cinematicas
piden nivel y el jugador siempre puede caminar a la taberna) — queda sin verificar visualmente.

## Evidencia
- Mutaciones (rojo/verde, `3 1`) en `logs_atasco/`: `mut1_sin_eventos` (ocaso 0/3, atasco «b3_rosita_ocaso» dia 4),
  `mut2_sin_zona` (0/3 en el prologo), `mut3_sin_viaje` (dia2 2/3, ocaso 0/3); los tres VERDE: 3/3, 0 atascos.
- `explorar_20_post.log` (`explorar_boundington 20 1`): 20/20 en las 5 fases; finales solos 15, oeste 4, caido 1; 0 atascos.
  Antes: ocaso 0/20, 18 atascos «evt_xila_avisa_a_venides» + 2 del dia 3 (`explorar_20.log`).
- `ctest_final9.log` (gl-free): 299/299. Y `python3 tools/validar_todo.py`: 0 errores (20 avisos previos).
- Limite: el explorador sigue suponiendo que cualquier nivel es alcanzable a pie; solo 3 de los 9 finales salen al azar.
