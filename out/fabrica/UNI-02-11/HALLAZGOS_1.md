# E-HALLAZGOS-01 — bucle del explorador y diario del Ocaso (04/11/2026)

Rama `agente/claude/UNI-02-11`, base `916e75b0`. Motor: `MotorGraphico-main/MotorGraphico`. Commit con trailer `Fabrica-E: E-HALLAZGOS-01` (el hash va en el informe del agente).

## Parte A — el bucle «dia2 · bucle (900 pasos) tras b2_sectarios»
**Veredicto: fallo del EXPLORADOR; un jugador real no se atasca.**
- Nota de reproduccion: el log `explorar_20_disp3.log` es de la **semilla 1** (cabecera «semilla 1»), no de las 2 y 3. Reproducido en HEAD: semilla 1 → 19/20, semilla 3 → 18/20 (2 atascos), semilla 2 → 20/20 (no se reproduce).
- Causa (con `EXPLORAR_TRAZA=1` y un trazado temporal): la partida atascada es de estilo **bondadoso**, en modo rescate, en `ciudad_centro`, dia 3 franja 2. Lo unico que quedaba «util» es hablar con `sectario_perdido` (`b2_sectarios`, repetible y sin efectos): su opcion «La Barriada se arma…» cambia la historia (pone `bnd2_barriada_delatada`), asi que `util()` la cuenta como util, pero el estilo bondadoso nunca la elige (pesa negativa). Resultado: 900 pasos hablando con el sectario y «Seguir tu camino», sin ir nunca a la taberna, que es donde `b2_cierre_*` (lugar `interior_taberna`, con `bnd2_a_dormir`) cierra el dia. Esa parte del «sin nada que hacer, ve a otro nivel» (E-OCASO-ATASCO) no se alcanzaba porque el pool nunca quedaba vacio.
- Un jugador real puede irse de la charla y caminar a la taberna: no hay atasco en el juego. No se toca la historia.
- Arreglo (en el explorador, `examples/explorar_boundington.cpp`, +26 lineas: `kMaxVacias`, `m_vacias`, `identico`, el bloque al inicio de `paso()` y el filtro en la seleccion de acciones): una accion tras la que la historia queda **identica** (flags y variables, incluida la contabilidad de paginas) 8 veces seguidas deja de ser candidata; al empezar el rescate la cuenta se pone a cero. Primer intento con `cambia()` (ignora la contabilidad de charla) dejo un atasco nuevo en el prologo (`p_barquero_p0`, semilla 3); por eso se compara por igualdad exacta.
- Mutacion: `logs_hallazgos1/explorador_bucle_mut.log` — ROJO con el explorador de HEAD (semilla 1: dia2 19/20; semilla 3: dia2 18/20, atascos «tras b2_sectarios») y VERDE con el arreglo (semillas 1, 2, 3: 20/20 en las 5 fases, 0 atascos; logs completos `explorador_bucle_verde_s{1,2,3}.log`).
- `explorar_20_h1.log` (`explorar_boundington 20`, semilla 1): 5 fases 20/20, 0 atascos, finales caido 1 / oeste 5 / solos 14.

## Parte B — diario del Ocaso (hallazgos 2, 3, 4, 6 de `MATRIZ_OCASO.md`)
| Nº | Cambio | Donde |
|---|---|---|
| 2 | `obj3_salir` ya no usa `bnd3_fuera` (que `b3_caida`/`b3_rodeo_caido` tambien ponen) sino `bnd3_salio_vivo`, nueva flag que ponen `b3_huida_gitanos`, `b3_huida_sola` y `b3_aliado_fuera` (las tres salidas vivas). | `assets/adventures/boundington_ocaso.json` (doneFlag + 3 efectos) |
| 3 | Decision nueva `bnd3_final` («Cómo acabó la noche») con una salida por final: `bnd3_final_oeste` / `_solos` / `_caido`. | `assets/catalogs/quests.json` |
| 4 | La salida «oeste» de `bnd3_ruta` lleva `notFlags: [bnd3_ruta_quedarse]`: quien se quedo ve «Te quedaste a luchar en la base», aunque `b3_base_cae` ponga tambien `bnd3_ruta_oeste` (el juego sigue poniendo las dos). | `assets/catalogs/quests.json` |
| 6 | `obj3_avisar` usa `bnd3_aviso_resuelto`, que ponen `b3_avisar_venides` y `b3_aviso_en_base` (junto a `bnd3_avisado`, sin tocarlo) y el evento `venides_retenido` (junto a `bnd3_sin_venides`, es decir con `bnd2_venides_sin_aviso` al sonar las campanas). | adventure + `assets/eventos/boundington.json` |
No cambia que finales existen ni como se llega a ellos; `bnd3_avisado` y `bnd3_fuera` siguen igual. Las flags nuevas son de lectura del diario.

Pruebas (`tests/Engine/MatrizOcasoTests.cpp`): la antigua `hallazgo_el_diario_da_por_hecha_la_salida_aunque_el_heroe_caiga` (que fijaba el fallo) pasa a `el_diario_no_da_por_hecha_la_salida_si_el_heroe_cae_y_cuenta_como_acabo_la_noche`; nuevas `quedarse_en_la_base_se_ve_en_el_diario_como_quedarse_y_no_como_ir_al_oeste` y `con_venides_sin_aviso_el_diario_cierra_el_objetivo_de_avisar_al_sonar_las_campanas` (esta ultima juega el evento real). La tabla de caminos (`examples/fabricar_guardados.h`) ahora espera la decision `bnd3_final` en los nueve caminos, «quedaste» en `sandes_aliado`, `obj3_avisar` hecho en los tres caminos con Venides sin aviso y `obj3_salir` pendiente en `caido`.
Mutaciones (rojo con los assets mutados, verde con los del arbol; se ejecutan desde un cwd temporal con una copia de `assets/` para no tocar el build): `logs_hallazgos1/salida_mut.log`, `final_mut.log`, `quedarse_mut.log`, `aviso_mut.log`.

## Parte C — final «caido»
Sigue alcanzable: `MatrizOcasoTests` lo recorre (camino `caido`, verde) y el explorador lo da 1/20 con la semilla 1 (`explorar_20_h1.log`), 3/20 con la semilla 3 y 0/20 con la 2. El «0 de 20» era probabilidad (solo 1 de cada 4 combates de la salida lo pierde y el explorador gana el 80 %), no un camino roto. No se fuerza.

## Resultados
- `ctest -L gl-free -j 6`: **303/303** — `ctest_final15.log` (fecha y hash base al principio).
- `python3 tools/validar_todo.py`: **0 errores**, 20 avisos previos — `logs_hallazgos1/validar_todo.log`.
- Captura real del diario (`juego --guion`): `capturas/m4_diario_caido_{misiones,final}.png`, descrita en `capturas/CAPTURAS.md`.

## Para Oriol (decision, no tocada)
1. Si el heroe cae, la mision del Ocaso queda «seguida» con 2/3 y «Cómo acabó la noche: Caíste en Boundington», no «fallida» (`failures` con `bnd3_final_caido` la marcaria asi). Antes salia 3/3 «hecha». No lo he decidido yo: cambia el icono y el cierre de la mision.
2. El objetivo «Avisa a Venides» se da por cerrado (no «hecho» en sentido estricto) cuando Venides ya salio solo y suenan las campanas; sigue pendiente hasta ese momento.
3. Guardados anteriores al cambio: un guardado de un Ocaso ya acabado vivo no tiene `bnd3_salio_vivo`/`bnd3_aviso_resuelto`; su diario mostrara «Salir» 0/1 (los de `fabricar_guardados --matriz-ocaso` se regeneran).
## No comprobado
Solo se ha capturado el camino «caido»; el diario de «oeste/solos» y de «quedarse» se prueba con las pruebas, no con captura. No se ha repasado `sesiones/` ni `TABLERO.md` (instruccion de no leerlos enteros).
