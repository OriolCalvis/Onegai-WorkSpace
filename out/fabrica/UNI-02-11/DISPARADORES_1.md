# DISPARADORES_1 — E-DISPARADORES-1 (D0026): E1, E2b, E2 y E3 (04/11/2026)

Rama agente/claude/UNI-02-11. NO hechos (fuera de encargo): E4 combate, E5 `once:false`, E6 migrar `plaza_sermon` y demas.

## Commits
| Paso | Hash | Que |
|---|---|---|
| E1 | 50365c88 | `Cinematica::puntoDeEntrada(transitable)`: centro de la zona (o `lleva.a`); arnes (`pisaLaZona`, `pisaUnaZona`) y explorador (`cineQueEmpieza`) sin la aritmetica duplicada |
| E2b | 2883ae3e | `ambito` en `LevelDefinition`/LevelLoader + `Level/AmbitoNivel.h`; `tapaVista` en `ObjectDefinition`/ObjectCatalog; `Game/LineaDeVision.h` extraido de `TableroCombate::seVen` |
| E2 | a29aca37 | `Game/Disparador.h`: parser `disparador`, `laQueEmpieza(..., VistaDelNivel*)`, `pasosEfectivos`/`efectiva` (lleva andando o fundido + accion) |
| E3 | 1cce5ece | `validar_cinematicas.py` valida el disparador; `tools/test_validar_cinematicas.py` (22 casos) |

## tapaVista (449 objetos)
- 416 en `mobles_kit.json` (paret_* 368, estanteria 20, vitrina 12, armario 8, biombo 8) y 19 en `decorados_kit.json`
  (tanca_fusta*, empalizada*, carromato*, carro_o1..o3): los pone el GENERADOR (`tools/iso/tapavista.py`, usado por
  `exportar_mobles.py` y `exportar_decorats.py`), regenerados: las texturas salen byte a byte iguales.
- 14 en catalogos de mano via `tools/marcar_tapavista.py` (idempotente): carro, carro_aigren, huida_carro/carromato/
  carromato_lona, puerta_gurkazaal, verja_dorada, 7 `botin_*_armario*`. `verja_dorada` tambien en `tools/av19_mansion.py`.
- NO tapan: PNJ, enemigos, pickups, farolas, taburetes, mesas, cajas ni puertas (`porton_base_militar` y las `puerta_*`
  son `blocksMovement:false`, el hueco). Regla: solo `prop` que bloquea (lo comprueban el validador y un test).

## Reglas elegidas (D0026 no las fija al detalle)
- Ambito deducido: prefijos interior_, mansion_, casa_, tienda_, dependencias_, mina*, cueva*, galeria*, camara_, grieta*,
  santuario*, mazmorra*, antecova*, aldea_*_pN (lista de F-DISPARADORES_PROXIMIDAD §2); interior < 200 casillas = pequeno.
- Sin `VistaDelNivel` (arnes, explorador, pruebas viejas) un disparador empieza en su zona o con el jugador EN el foco.
- Solo vale la vision cuando se llega por el radio; la zona sigue valiendo sin vision. Interior pequeno: sin vision.
- Accion `beat`/`marca`: los `pasos` propios no se juegan (el validador avisa). `once:false` se lee pero aun no rearma (E5).

## Pruebas y mutaciones (logs en `logs_disparadores/`)
- `test_vision_y_ambito` (6), `test_cinematicas` (+8: lectura, radio por ambito, vision tapada por `tapaVista` y NO por PNJ ni
  objeto pequeno, zona o radio, lleva andando, lleva con fundido, accion), `test_validar_cinematicas` (22).
- 14 mutaciones, todas muertas: E2b 3 (lectura tapaVista, umbral 200, extremos de Bresenham), E2 8 (`>=` en el radio, radio
  exterior 5, sin vision, tapa=bloquea, zona exige vision, lleva ignora la ruta, fundido sin Coloca, accion antes de llevar),
  E3 3 (colision ignorada, sin umbral 50 %, una sola accion).

## Evidencia
- `ctest -L gl-free -j 6`: 303/303, 0 fallos (`ctest_final12.log`); `linea_base_core` OK: Cinematica.cpp 601->591 lineas, ningun
  `Application*.cpp` crece, 0 fuentes GL-free nuevas en motor_core (la cola del motor es la cabecera
  `src/Engine/ApplicationCinematicaDisparador.h`, incluida solo por ApplicationCinematica.cpp). La linea base (`--actualizar`) no se toca.
- `validar_todo.py`: 0 errores. `validar_cinematicas.py`: 113 cinematicas, 0 errores, 18 avisos (centros de zona no transitables
  de zonas antiguas: arnes y explorador los pisan igual; candidatos a `lleva.a` en E6).
- `explorar_boundington 20 1` (`explorar_20_disp.log`): prologo, dia1, dia_gorran, dia2 y ocaso 20/20, 0 atascos, salida
  identica linea a linea a `explorar_20_post.log` (antes de los cambios). Matrices, misiones y demos: dentro de la suite.

## Pendiente / decisiones
- Ninguna bloqueante. Para E4/E5/E6: D-A (de donde salen los enemigos de `combate`) sigue abierta en F-DISPARADORES.
- Aun no probado en ventana (OpenGL): el camino real `ApplicationCinematica::efectiva` (rutaAndando) solo tiene pruebas gl-free
  con escenario de mentira; ninguna cinematica actual lleva `disparador`, asi que el juego no lo ejerce hasta E6.
