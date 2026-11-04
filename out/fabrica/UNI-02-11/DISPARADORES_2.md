# DISPARADORES_2 — E-DISPARADORES-2 (D0026): A, E4, E5 y E6 (04/11/2026)

Rama agente/claude/UNI-02-11. Sigue a DISPARADORES_1.md (E1, E2b, E2, E3). Evidencia visual: `capturas/CAPTURAS.md` (prefijo `d_`).

## Commits
| Paso | Hash | Que |
|---|---|---|
| A | b91dcfb5 | Aigren (decision de Oriol): `b1_aigren_papel` solo con `c_parcial`, `b1_aigren_discurso` solo con `c_pifia`. Una condicion por beat, texto intacto. |
| E4 | 9fd9e6b4 | `accion: {"combate": "<enemigo>"}` por `GameSession::startNarrativeBattle` (la puerta del `startBattle` de los beats). |
| E5 | 6f5ec420 | `once:false`: `CatalogoCinematicas::rearmadas`. |
| E6 | (este commit) | `plaza_sermon`, `bgd_velas`, `ambito` en 2 niveles, guardados + guiones + capturas, informe. |

## A — Aigren
- Escenas de `MisionesDia1Tests`: `aigren_discurso` (parcial: papel, orden pedida, NO discurso) y la nueva `aigren_pifia_discurso` (pifia: discurso, cierra, sin orden). Mutaciones en `logs_disparadores2/A_mut.log`: 2 muertas + 1 equivalente (papel salta antes y enciende `orden_pedida`, que es el `notFlags` de discurso).
- `tools/matriz_misiones.py`: 25/25 cerrables; el discurso ya no es codigo muerto (nota 1 de `MISIONES_DIA1_notas.md` cerrada). Efecto: `b2_capitan_orden` (pide `orden_pedida`) solo se abre ya por la ruta parcial; el guardado `aigren_convence_pifia` se ajusto.

## E4 — combate
- Campo `accion.combate`: lleva al jugador, lanza el encuentro y la escena espera a que acabe; el motor pone `cine_<id>_ganado` / `cine_<id>_perdido` (huir = perder). Paso `Combate`, `EscenarioCinematica::combate` (vacio por defecto), `ApplicationCinematica::lanzaCombate` (en la cabecera interna). Validador: el id es un `enemy` del catalogo.
- Prueba gl-free `combate_lleva_lanza_el_encuentro_y_espera_a_que_acabe`; 6 mutaciones muertas (`E4_mut.log`). Ninguna escena real lo usa aun; en ventana no probado.

## E5 — once:false
- La zona se rearma cuando el jugador sale de ella y de su radio (o cambia de nivel): se apaga `cine_<id>_vista`. Sin estado nuevo: la flag viaja en el guardado. Las `once` no cambian. Prueba + 6 mutaciones muertas (`E5_mut.log`). Ninguna escena real lo usa (ni `bgd_velas`: repetir la escena no aporta nada).

## E6 — migracion
- `plaza_sermon`: `lleva.a` (55,55), delante del estrado, mira arriba. `bgd_velas`: `lleva.a` (21,30), mira arriba. `ambito` explicito en `ciudad_centro` (exterior) y `mina2_gorran` (interior_grande). `once` se queda en true.
- Avisos de `validar_cinematicas.py`: 18 -> 17 (plaza_sermon). Los 17 restantes NO se tocan: en 14 el centro lo ocupa un PNJ o es agua/roca y no esta claro donde debe ponerse el jugador (candidatos: sandes_casco (55,49), rosita_calle (26,58), final_salida_* (13,61)/(14,61), edul_*, venides_de_rodillas (9,22), cuartel_exterior_llegada (8,11), rio_* orillas); poner `lleva` ahi cambiaria cada escena.
- DECISION PARA ORIOL, `base_ruta_{oeste,norte,quedarse}` (ciudad_oeste): su zona esta casi toda en casillas con colision. Un BFS desde el inicio: oeste solo (3,12); quedarse (12,12),(13,12); norte NINGUNA. Opcion 1 (recomendada): moverlas a la calle (y=12) o a un hueco de la fachada, con `radio:0`, para que se elija pisando. Opcion 2: dejarlas y llevarlas por dialogo. No se migraron: con radio 10 las tres se disputarian la misma plaza.

## Evidencia
- Juego real: ver CAPTURAS.md. `plaza_sermon` y `bgd_velas`: fuera de la zona, dentro del radio y con vision, el juego lleva al jugador andando (8 y 5 pasos, traza) y arranca la escena (dialogo de Sandes; narracion de las velas). Con 3 objetos `tapaVista` entre el jugador y el foco, no arranca.
- `ctest -L gl-free -j 6`: 303/303, 0 fallos (`ctest_final13.log`); `linea_base_core` OK, `ApplicationCinematica.cpp` 477 -> 476; la linea base no se toca. `validar_todo.py`: 0 errores.
- `explorar_boundington 20 1` (`explorar_20_disp2.log`): prologo, dia1, dia_gorran, dia2 y ocaso 20/20, 0 atascos. Cobertura (beats sin ver, de 1216): total 301 -> 303; dia1 95 -> 94, dia2 56 -> 54, ocaso 72 -> 73, gorran 77 -> 81. Cambia por A: el explorador ya ve `b1_aigren_discurso` (pifia) y no ve `b1_aigren_papel` (solo parcial). Finales: solos 17, oeste 3, caido 0 (antes 15 / 4 / 1): el azar sigue otro camino, no hay regla nueva.
- Nada de esto cambia la linea base (`--actualizar` no se ejecuto). Diseno propio de E4 (no estaba en D0026): las flags del combate son `cine_<id>_ganado/_perdido`; si Oriol prefiere flags propias, se anaden a `accion`.
