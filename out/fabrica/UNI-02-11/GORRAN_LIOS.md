# GORRAN-LIOS: el dia 2 jugado de verdad (D0022 3.2)
Commit `3a8bda1a` en `agente/claude/UNI-02-11` (~/onegai-ramas/UNI-02-11). Trailer `Fabrica-E: GORRAN-LIOS`.
Rutas JSON = `MG/assets/adventures/boundington_dia_gorran.json` (J). Ayudas en `MG/examples/demo_boundington_partida.h` (P).

## Inventario de los 4 lios (reloj `aldea_tiempo`: solo avanza con acciones `bnd2_t_*`)
| Lio | Se abre | Se cierra (suma 1 a gorran_misiones) | Jugable |
|---|---|---|---|
| B Veta Honda | hablar `minera_comedor`/`herido_aldea` o reentrar en cueva_gorran (bgb_gancho) | zona `bgb_golpes` (mina_gorran) -> «Apuntalar con las vigas» (vigas con `carpintero_galeria`, +1 reloj) -> bgb_salen_todos | si |
| A Cal Serra | hablar `serra` (bga_gancho); con reloj>=2 llegan (bga_llegan) | cine `bga_entran` -> «Hablarle a la Garsa» -> bga_garsa_exito; (mala suerte: «Dejar que se la lleven») | si |
| C Galerias | hablar `aduanero_galeria`/Bet (bgc_gancho) | cine `bgc_asalto` (zona) -> «Hundirlas en el rio» (o pelea con Bet/Fumat); si no, `bgc_barca_sale` a reloj>=5 | si |
| D Mina vieja | `bgd_rapto` auto a reloj>=2 (bgd_ninos_fuera) | cine `bgd_velas` (zona) -> «Sacar a los crios en silencio»; si no, `bgd_fin_tarde` a reloj>=7 | si |
Al 3.o cerrado: `b2_subasta_aviso` -> `bnd2_subasta_ya` -> zona `recinto_subasta`. Orden jugado B, A, C, D: coste de reloj 2+0+2+1 = 5; caben los 4 antes del tope (7).
Ninguno queda sin poder cerrarse jugando. Sin hallazgo de bloqueo; si de incoherencias (abajo).

## Que se juega ahora (P: losLiosDeLaAldea, pisaLaZona, pasaElDiaDeGorranJugando)
- `laEspadaDeRosita` ya no hace `setVariable("gorran_misiones",3)`: juega los 4 lios; cada uno suma exactamente 1 (se comprueba lio a lio) y `gorran_misiones == cerrados`. Lo usan campana A y 5 escenas del dia 2.
- Escena nueva «los cuatro lios, la subasta y a dormir»: lios + cuerno + porton + subasta + `bg_cierre_furia` (sin setFlag de `bnd2_subasta_vista` ni `bndg_a_dormir`).
- Escena nueva «hablar con Gorran de frente»: `b2_gorran_hablar` -> `b2_gorran` (3 temas, «Dejarlo para esta noche» cierra la charla; se repite).
- Escena nueva «lios con mala suerte»: B y A cierran, C acaba en calabozo (`bgc_abrir_pifia`), D atado (`bgd_atado`); no suena el cuerno.
- Campana A exige ahora los 4 `*_cerrada` + `bnd2_subasta_ya`. El helper fuerza tiradas buenas en los lios aunque la escena tenga Mala/Critica (param `tiradasBuenas`); la escena de mala suerte lo desactiva.

## Que NO se juega, y por que
- `pasaElDiaDeGorran` (atajo; ~30 escenas del dia 3 en `demo_boundington_alternativas.cpp`) sigue con `setFlag(bnd2_subasta_vista/bndg_a_dormir)`: sustituirlo por `pasaElDiaDeGorranJugando` rompe «DIA 3: la fiesta, por la puerta y hablando» (la primera charla con Rosita en el dia 3 cambia: «Decirle que no la encontraste», y `anochece()` no hace la suya). Hace falta reescribir esas escenas; no lo hago aqui.
- Viajes entre niveles: `entra()` es un salto; no se comprueba que las salidas conecten (P:entra).
- Reintentos tras fallo (calabozo, atado): no jugados. Rutas de pelea de C/D/A, capucha, Candela, Bru, carta falsa, Gorran-contra-gris: sin cubrir.

## Hallazgos (no tocados; son de historia)
1. J:5001 `bgb_campana` (enter cueva_gorran) exige `bg_aldea_problemas_visto`, que pone J:16398 (posterior en el fichero): en la 1.a entrada no salta; hay que reentrar o hablar con la minera.
2. J:2366 `bgd_mia` («Llegas tarde: Mia ya no está», reloj>=5) no cuenta en J:2833 `bgd_sacar_exito`/`bgd_ganada`: igualmente cierra «Tinet y Mia, en casa». Incoherente.
3. J:4294 `bgc_barca_sale` + J:4667 `bgc_hundir_exito`: con el asalto a reloj>=5 (orden D antes que C) saltaron los dos: C sumo dos veces (en esa corrida el tope `max 4` lo escondio). `bgc_asalto` se enciende al acabar la cine, tras la eleccion.
4. Subasta (J:16731): «Dejar que estalle» abre pelea con Gorran y el unico desenlace sin tocar el default es «Acabar con el» (mata a Gorran); el helper pide «Suelta a los presos...».
5. `b2_gorran_hablar` (J:16053) solo exige estar en recinto_gorran: sin subasta_ya ni encargo; el dialogo da pistas (`bnd2_pista_gorran`, `bnd2_gorran_asustado`) y nada mas.

## Comandos y salidas
`cd MG && cmake --build out/build/macos-debug --target demo_boundington demo_campana_boundington`
`ctest --test-dir out/build/macos-debug -R "boundington|campana" --output-on-failure` -> 6/6 passed (demo_boundington 1.12 s, campana 0.86 s). Beats en escenas: 405 -> 425 de 1196.

## Mutacion
Quitar `pisaLaZona("bgd_velas")` (lio D): `demo_boundington` falla («MAL ... falta bgd_cerrada, falta bgd_ninos_salvados») y `demo_campana_boundington` aborta (campana:103). Restaurado (cp + touch + recompilar): 6/6 verde.

## Preguntas para Oriol (que queremos de Gorran)
1. Cuantos lios caben en un dia? A) Los 4 caben (coste 5 de 7): dejarlo asi y que 3 sea solo el umbral de la subasta. B) Que no quepan (subir coste de alguna accion) para obligar a elegir 3.
2. Mia tarde (hallazgo 2)? A) Que `bgd_mia_perdida` haga que solo se salve Tinet. B) Quitar el aviso «Mia ya no esta».
3. Hablar con Gorran de frente (`b2_gorran_hablar`)? A) Que lleve a algo: tregua/aliado si hay lios cerrados y aval. B) Dejarlo como charla de ambiente (hoy).

## GORRAN-FIX (D0023 puntos 2 y 3 + los dos errores de arriba)
Trailer `Fabrica-E: GORRAN-FIX`. Todo en `boundington_dia_gorran.json` (J) y las demos `demo_boundington_{partida.h,alternativas.cpp}`.
1. **Lio C doble** (hallazgo 3): `bgc_barca_sale` lleva ahora `bgc_asalto_visto` en notFlags (se pone al empezar el asalto, antes de la eleccion; `bgc_asalto` se pone despues y no servia). Escena: partiendo de 0, `gorran_misiones` acaba en 1 exacto con el reloj en 5.
2. **`bgb_campana`**: pasa de `enter` a `auto` con `lugar` cueva_gorran: salta justo despues de `bg_aldea_problemas` en la PRIMERA entrada (el aviso de la plaza antes, luego la campana). Texto intacto. (Con `enter` solo cabia mover el orden, y entonces el aviso de la plaza se iria a la 2.a entrada.)
3. **D0023.2**: `bgd_sacar_exito` exige no `bgd_mia_perdida`; nuevo `bgd_sacar_exito_tinet` (cierra el lio, +1 una vez, flags `bgd_tinet_salvado`+`bgd_solo_tinet`, NO `bgd_ninos_salvados`) y variantes `bg_balance_bgd_tinet_salvado` («Tinet, en casa. Mia, no.») y `bgd_plaza_tinet`. Mismo grantLevel 0.3 (sin decision nueva).
4. **D0023.3**: nuevo `b2_gorran_tregua` (talk gorran, antes de `b2_gorran_hablar`): pide `bnd2_aval_serra` (la unica flag de aval, la da Serra) + `gorran_misiones >= 3` (el mismo umbral de «aldea en calma»; elegido por mi). «Proponerle una tregua» -> `bnd2_gorran_tregua` (+ sospecha -1); «Dejarlo para luego» -> `bnd2_gorran_tregua_ofrecida` y sigue con la charla de siempre.
5. **Subasta**: sin cambios. En el juego real NO hay opcion por defecto: con eleccion pendiente no se puede cerrar el dialogo (`GameSession::closeInteraction`). El «Acabar con el» por defecto es solo el relleno de `Jugador::contesta` (la ultima opcion salida). No hay nada que reordenar.
## Decisiones que quedan para Oriol (no tocadas)
- `bnd2_gorran_tregua` solo se enciende y se registra: no activa nada despues. Si la tregua debe dar aliados (`bnd2_tiene_aliados`, `bnd2_gorran_perdonado`) o quitar «Dejar que estalle» de la subasta, hay que decidirlo (hoy matar a Gorran tras una tregua es posible).
- D0023.2 solo cubre el cierre de `bgd_sacar_exito`; los otros cierres del lio D (`bgd_ganada`, Candela, «¡Gorran! ¡Ahora!») siguen poniendo `bgd_ninos_salvados` aunque Mia este perdida. Y la cinematica `bgd_velas` sigue diciendo «Tinet y Mia, atados» con Mia perdida.
- El orden de los lios en las demos pasa a B, A, D, C (antes B, A, C, D): con el antiguo la mina vieja llega con reloj 5 y Mia se pierde; `orden` es parametro (`"BACD"` = tarde).
## Pruebas y mutaciones
8 escenas nuevas en `demo_boundington_alternativas.cpp`; mutaciones (todas fallan, restaurado + touch + recompilado en verde): M1 quitar `bgc_asalto_visto`, M2 `bgb_campana` a enter, M3 quitar `bgd_mia_perdida` de exito, M4a sin aval, M4b sin umbral de lios, M4c sin la flag de tregua. `validar_todo` 0 errores (729 autos, 0 pegajosos/tapados); boundington|campana 6/6; gl-free 291/291.
