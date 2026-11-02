# Las 13 pruebas heredadas de UNI-02-11 (diagnostico de solo lectura)

Worktree `/Users/admin/onegai-ramas/UNI-02-11` (rama `agente/claude/UNI-02-11` @ `08b0602d`). Motor: `MotorGraphico-main/MotorGraphico/`. Build: `out/build/macos-debug` (Debug, `-g` sin optimizar). Todas las rutas de abajo son relativas al motor salvo que se indique.
Nada del repo se ha tocado. Las comprobaciones de "arreglo" se hicieron en copias del scratchpad de la sesion (binarios/JSON parcheados fuera del arbol).
Contexto comun: el repo solo tiene un commit de base (`95ef783a` "base prueba c528ebb", 29/10). Es una foto mezclada de varias lineas de trabajo; por eso hay datos de una rama y pruebas de otra.

## Tabla

| prueba | causa | clase | arreglo minimo | ficheros | decision de Oriol | min |
|---|---|---|---|---|---|---|
| test_ruta_casillas | Dijkstra 100x100 en Debug (-O0) tarda 4,6-5,9 ms; umbral `ms < 5.0`. Con -O2 son 0,8 ms | entorno (prueba de rendimiento en Debug) | umbral segun `NDEBUG` (5 ms Release / 50 ms Debug) o tomar el minimo de 3 pasadas | `tests/Game/RutaCasillasTests.cpp:90` | no | 10 |
| test_validar_monstruos_acciones | usa `quemado` como "estado futuro desconocido", pero `quemado` ya esta en `conditions.json` (PO-37, 15/10) | prueba desfasada | cambiar `quemado` por un id inexistente (`petrificado`) | `tools/test_validar_monstruos_acciones.py:28` | no (PO-37 ya lo decidio) | 5 |
| test_catalog_content | `cat.size() == 16` pero hay 20 condiciones (+quemado, congelado, aturdido, silenciado, PO-37) | prueba desfasada | `== 20` | `tests/RPG/CatalogContentTests.cpp:223` | no | 5 |
| test_botin_equipo | `comprobados == 26`; el catalogo de botin tiene 43 contenedores (+17: variantes `botin_afueras_*`, `botin_cueva_*`, AV19) | prueba desfasada | `== 43` (el resto de comprobaciones de las 43 pasan) | `tests/Game/BotinEquipoTests.cpp:92` | no | 5 |
| test_kit_basico | espera `bono == "+1 CAR"`; el codigo lo deja vacio a proposito (AV16-02: todo empieza a 0, Oriol 15/10) | prueba desfasada | `p->piezas[1].bono.empty()` | `tests/Game/CofreKitTests.cpp:66` | no | 5 |
| demo_creacion_personaje | 3 constantes viejas: `races().size()==45` (hay 72), `opciones().size()==43` y `totalSinFiltrar()==43` (hay 69 jugables) | prueba desfasada | 45->72, 43->69, 43->69 (verificado: la demo corre hasta el final) | `examples/demo_creacion_personaje.cpp:43,156,172` (y comentarios 154, 172) | solo confirmar que `espectros` no se juega (`playable:false` en races.json) | 10 |
| demo_boundington | el cierre del dia 1 (`cierre_creadores`, `cierre_espera`...) exige `bnd1_en_cuarto`, que da `b1_sube_al_cuarto` al ENTRAR en `interior_taberna_p1` (Oriol 22/10). Los guiones no entran | prueba desfasada | en cada escena que cierra el dia 1, `e.j.entra("interior_taberna_p1")` antes del `ve({"bnd_dia_cerrado"})` (mejor: un helper `duerme()` en la partida) | `examples/demo_boundington.cpp:~83` y escenas siguientes, `examples/demo_boundington_alternativas.cpp` (~11 escenas), `examples/demo_boundington_partida.h` | no | 45-60 |
| demo_campana_boundington | misma causa (`demo_campana_boundington.cpp:92`, `hasFlag("bnd_dia_cerrado")`) | prueba desfasada | mismo helper | `examples/demo_campana_boundington.cpp`, `demo_boundington_partida.h` | no | 20 (tras el helper) |
| test_recompensas_economia | `recorrer_recompensas` dice "Ruta incompleta: falta bnd3_fuera": las 3 rutas de `tools/rutas_economia.json` no tienen ninguna accion `entrar interior_taberna_p1`, el dia 1 no cierra y la cadena no llega al final | dato desfasado (rutas sintetizadas antes del 22/10) | regenerar con `tools/sintetizar_rutas.py` (ya soporta triggers `enter`) y repasar a mano | `tools/rutas_economia.json` (generado), `tools/sintetizar_rutas.py` si falla | no | 30-45 |
| test_lugares (5 fallos) | `assets/catalogs/lugares.json` de esta base es la variante de la rama ui-puertas-01: 31 salidas (`salir`) estan en `lugares` en vez de `calles`; faltan en `calles` `base_militar` y `afueras_ocaso`. Ademas el test cuenta puertas solo en 3 ciudades pero `interior_cuartel` cuelga ahora de `base_militar` (AV19-11, 20/10) | dato mal puesto + prueba desfasada | mover `lugares[18:]` a `calles` (solo nivel, nombre, salir), anadir `base_militar` ("el patio de armas", "el patio") y `afueras_ocaso` ("las Afueras", "los bancales"); y en el test anadir `base_militar` a la lista de niveles que se recorren. Verificado con copia: 9/9 salvo el del test (queda 1 que se arregla con la linea del test) | `assets/catalogs/lugares.json`, `tests/Game/LugaresTests.cpp:~83` | si: los emblemas propuestos para Gorran (`_propuesta_ui_puertas_01`: libro de secretos, mapa) se perderian al mover; "pendiente de Oriol" | 20 |
| test_pergamino (4 fallos MesaCombate) | AV16-11 cambio el layout del combate a proposito (retratos 80 px, paso 92 en vez de 74; tira mas alta; el rotulo "Turno de X" esconde mano y barra). 3 de 4 son pruebas con numeros viejos. El 4o (`p5.size()==5`) es real: solo se pintan 4 placas | 3 prueba desfasada + 1 defecto real de layout | actualizar 136 (rival: cuantos() menor), 214 (chips del heroe y 130-150), 359 (paso 92); el de 5 enemigos: alto de placa compacta 60->50 (170+5*56-6=444<=446) | `tests/HUD/MesaCombateTests.cpp:136,214,299,359`, `src/HUD/Pergamino/MesaCombateRetales.cpp:~291-296` | si, mirar captura: con 5 enemigos el quinto no tiene placa (no se puede apuntar) | 40 |
| linea_base_core_configura | 49x "SIMBOLO DUPLICADO": 4 copias Finder " 2.cpp" versionadas en la base (trampa " 2" de la bitacora) | entorno (basura de macOS en el commit) | retirar los 4 `* 2.cpp` | `src/Engine/ApplicationPantallas 2.cpp`, `src/Game/CartasJugar 2.cpp`, `src/HUD/Pergamino/InterfazPergamino 2.cpp`, `editor/src/LevelEditorPaleta 2.cpp` | no (pero git rm: confirmar con Oriol segun regla de AGENTS.md; no son de esta sesion) | 5 |
| linea_base_core | lo anterior + 53 "DEUDA NUEVA" que persisten sin los " 2" (huerfanos que no alcanza ningun ejecutable, techos de 500 lineas, miembros) | defecto real / deuda sobre la linea base del 28/09 | no hay arreglo minimo de codigo: o se paga la deuda o se rebasa (`--actualizar`, "no puede subir") | `planificacion/linea_base_core.json` | si (rebasar la linea base es decision de proceso) | 5 si se rebasa; dias si se paga |

Notas: "CHECK FALLO" de las demo y el abort son el mismo mecanismo (`require` aborta). Sin tildes/ñ nuevas: ninguno de los arreglos propuestos toca `lines/speaker/text` de JSON de aventura.

## Grupo A. Condiciones: 16 -> 20 (test_catalog_content, test_validar_monstruos_acciones)

- `assets/catalogs/conditions.json` tiene 20 entradas: agarrado, anclado, asustado, aturdido, caido, cegado, congelado, encantado, endeudado, enfurecido, envenenado, fatiga, guardia_rota, inconsciente, marcado, paralizado, quemado, ralentizado, sangrado, silenciado.
- `BANDEJA_PRODUCT_OWNER.md:27`: "PO-37 (quemado, congelado, aturdido, silenciado; dados extra si)" (15/10). `PENDIENTE_DE_REVISAR.md:265` era la pregunta ("hoy hay 16"). `sesiones/2026-10-16-claude-av18-cierre.md:14` ya arreglo otro test por lo mismo ("`quemado` ya existe desde el 15/10").
- Fallo 1: `tests/RPG/CatalogContentTests.cpp:223` `MOTOR_CHECK(cat.size() == 16)`.
- Fallo 2: `tools/test_validar_monstruos_acciones.py:28` mete `condicion:quemado:1` esperando "Estado desconocido"; `tools/validar_monstruos_acciones.py:56` mira `d['condiciones']` y `quemado` esta, asi que no lanza. Reproducido: `AssertionError: ValueError not raised`. Los otros 9 tests del fichero pasan.

## Grupo B. Catalogos que crecieron con tests con el numero viejo (test_botin_equipo, demo_creacion_personaje)

- botin: `assets/proyectos/boundington.json` -> `botin_boundington.json` tiene 43 contenedores. `sesiones/2026-10-16-fable-d-ciudad-integrada.md:19`: repartir_botin con variantes nuevas `botin_afueras_*`; las `botin_cueva_*` vienen de AV19. Solo falla el contador de `tests/Game/BotinEquipoTests.cpp:92` (`== 26`); el bucle de 43 x 32 semillas respeta presupuesto (no hay mas FAIL).
- razas: `assets/catalogs/races.json` tiene 72 (69 jugables; `playable:false`: espectros, yokai_de_las_sombras, yokai_de_los_vientos). `tests/Game/PersonajesCapasTests.cpp:99` ya espera 72 ("+27 del 2000 b.f.") y pasa. Reproducido compilando una copia de la demo fuera del arbol con 45->72, 43->69, 43->69: termina sin mas fallos.
- `examples/demo_creacion_personaje.cpp:43` `== 45`; `:156` `p.opciones().size() == 43`; `:172` `p.totalSinFiltrar() == 43`.

## Grupo C. Kit sin bono (test_kit_basico)

- `src/Game/CofreKit.cpp:53` `Pieza p{e->name, ranuraLegible(*e), ""};  // AV16-02: las prendas del kit no dicen que sumen`.
- `src/RPG/CharacterCreation.cpp:144-145`: "AV16-02 — Oriol (15/10): todo empieza a 0". `tests/Game/CofreKitTests.cpp:65` ya espera `bono.empty()` para el arma; `:66` aun espera `"+1 CAR"` para la cabeza. Fallo unico de 16.

## Grupo D. Cierre del dia 1 y `b1_sube_al_cuarto` (demo_boundington, demo_campana_boundington, test_recompensas_economia)

- `assets/adventures/boundington_primer_dia.json` ~27356: beat `b1_sube_al_cuarto`, nota "Oriol (22/10): la noche del dia 1 acaba arriba, en la cama", trigger `enter assets/levels/interior_taberna_p1.json`, requiere `bnd_noche`, da `bnd1_en_cuarto`. `cierre_creadores` (~27382) y los otros `cierre_*` requieren `bnd1_en_cuarto`.
- Salida real: `demo_boundington`: "MAL DIA 1: el forastero que pregunta / falta bnd_dia_cerrado / no se vio cierre_creadores" (`demo_boundington_partida.h:375`). `demo_campana_boundington`: `demo_campana_boundington.cpp:92` `j.s.hasFlag("bnd_dia_cerrado")`.
- Ningun guion entra en `interior_taberna_p1` (grep de `examples/` = 0 hits); los tests `BoundingtonDia1Tests.cpp:209,266` si lo hacen y pasan.
- Prueba del arreglo: copia de `demo_boundington.cpp` con `e.j.entra("interior_taberna_p1")` antes del `ve({"bnd_dia_cerrado"})` -> la primera escena pasa; la siguiente ("el forastero que oye rumores") falla igual con `cierre_espera`. Es el mismo patron en todas las escenas que cierran el dia 1, por eso se propone un helper unico.
- test_recompensas_economia: `tools/recorrer_recompensas.cpp` ("Ruta incompleta: falta bnd3_fuera"); `tools/rutas_economia.json` no contiene `taberna_p1`. Cuidado: tras regenerar, el recorrido del dia 2 y 3 puede destapar otros cambios posteriores al 22/10 (no verificado).

## Grupo E. lugares.json (test_lugares)

- Fallos: `LugaresTests.cpp:55` (campo `funcion` no textual en la entrada 18), `:92`, `:112`, `:143` (`todos().size()==18`, hay 49), `:189` ("la salida de interior_cuartel.json lleva a assets/levels/base_militar.json, sin nombre").
- Causa dato: `lugares` tiene 49 entradas; de la 18 en adelante son salidas (`nivel`, `nombre`, `salir`) que el cargador (`src/Game/Lugares.cpp`) lee como `calles` aparte (`calles` tiene 42 y faltan `base_militar`, `afueras_ocaso`). El fichero coincide con el de `.ui-puertas-01` (49/42), no con el de `Software/` vivo (18/45).
- Causa test: `LugaresTests.cpp:~83` solo busca puertas en `ciudad_centro/este/oeste`; `sesiones/2026-10-20-qwen-av19-11-base-militar.md:51`: `puerta_cuartel` ya entra en `base_militar`, que es la que lleva a `interior_cuartel`.
- Prueba: con `lugares` = primeras 18 + `calles` = 42 + 31 movidas + 2 anadidas: 8 de 9 pasan; el noveno (`:92`) queda solo por lo del cuartel (`base_militar` apunta solo a `interior_cuartel`).
- Decision: las propuestas de emblema de Gorran ("libro de secretos" en `antecova_gorran`, "mapa" en 6 pasos) son de `sesiones/2026-10-25-codex-ui-puertas-01.md`, marcadas "pendiente de Oriol". El esquema `calles` no tiene `emblema`: al moverlas se pierden hasta que decida.

## Grupo F. MesaCombate (test_pergamino)

Valores medidos con una copia de `MesaCombateTests.cpp` con trazas (fuera del arbol):
- `:136` rival pendiente: 109 elementos pintados vs 536 con barra -> `src/HUD/Pergamino/MesaCombateHud.cpp:366-372` "Su mano y su barra se esconden... el turno es suyo". Prueba desfasada.
- `:214` chip de estado del heroe: y=130, abajo=150 (la prueba pide 98..138). `MesaCombateRetales.cpp:109-113` "AV16-11: retratos mas grandes (80)... kPaso = 92". Prueba desfasada.
- `:359` cajas del orden de turno a x=504, 596, 688, 780 (paso 92, no 74). Mismo comentario.
- `:299` 5 enemigos: solo 4 placas compactas (y=170,236,302,368; alto 60). `MesaCombateRetales.cpp:~291-296`: `compacta` empieza en 170 ("la tira es mas alta; la compacta empieza en 170") y `if (y + alto > hasta) break;` con `hasta=446` descarta la 5a (170+5*66-6=494). Los 4 fallos ya estaban antes del encargo ui-puertas (`sesiones/2026-10-25-codex-ui-puertas-01.md:23`, `2026-10-26-codex-creador-corregido.md:27`: lineas 136, 214, 299, 359).
- Este ultimo es defecto real de producto: el quinto enemigo de un grupo no tiene placa ni es apuntable. Arreglo propuesto (no verificado): alto 50 y hueco 6.

## Grupo G. linea_base_core

- `linea_base_core_configura`: 49 "SIMBOLO DUPLICADO" (`tools/linea_base_core.py:378-420`, recorre todo `src/` y `editor/src/`, este compilado o no). Las cuatro copias " 2.cpp" estan versionadas desde `95ef783a`. En copia del scratchpad sin ellas: "OK — configuracion y estructura instantanea correctas". Tambien versionados y sin efecto en esta guardia: `examples/demo_boundington_partida 2.h`, `include/Render/PergaminoGL 2.h`, `CMakeLists 2.txt`. La trampa esta en `BITACORA_DEL_PROYECTO.md:489`.
- `linea_base_core`: sin los " 2", sigue "0 instantaneos, 53 de deuda nueva" frente a la linea base del 28/09 (`planificacion/linea_base_core.json`): 46 ficheros "no alcanza ningun ejecutable" (ShopEngine, Companeros, TitleScreen, LevelSerializer...), `Application.h` 519->526 lineas, `GameSession.h` 546->554, `ApplicationNivel.cpp` 517, `Cinematica.cpp` 601, `EditorApariencia.cpp` 714, 3 clases con mas miembros, `motor_core` de 17 a 22 fuentes GL-free. `sesiones/2026-10-25-codex-ui-puertas-01.md`: "49 instantaneos y 51 de deuda ... exactamente iguales" en la base. Ya fallaba antes: no es regresion de esta rama.
- Nota: el AVISO "alcance_real.json es 0.1 dias mas viejo" no cambia el resultado; regenerarlo en la copia dio un conjunto distinto (mas huerfanos), no menos. No usarlo para rebajar la deuda sin mirar.

## Orden sugerido (de menos a mas coste)

1. Un solo commit de pruebas/datos sin decision: A, B, C, `test_ruta_casillas` (umbral). ~35 min, 6 pruebas.
2. " 2.cpp" (configura): 5 min, tras OK de Oriol para retirar copias.
3. lugares + test (E): 20 min; dejar la pregunta de emblemas Gorran en la bandeja.
4. Helper `duerme()` en `demo_boundington_partida.h` (D): 1-1,5 h, mas regenerar rutas.
5. MesaCombate (F): 40 min con captura para Oriol.
6. `linea_base_core` completo: decision de proceso.
