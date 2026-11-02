# Registro — tanda UNI-02-11 (fábrica multiagente, modo día)

Brain: Claude Opus 5.5 (sesión Code tab, 02/11/2026). Checkout de Oriol (`Software/`, rama `recuperat/software-copia-30-10`) **no tocado**, salvo esta carpeta `out/fabrica/UNI-02-11/`.

## Encargo (Oriol, 02/11 ~20:50)
1. «Primer unificaria tot el que es pugui, mantindria els progressos que planteja cada branca i les aniria filant; demà Fable pot restaurar si deixem acotades les zones a revisar.»
2. Després, avançar el joc: integració de missions, animacions, sons; PixelLab restaurat → acabar sprites.

## Estado de partida (comprobado)
- El `.git` original se perdió la noche 29→30/10. `claude/unificacion-28-10` (9d52ae8) ya no existe.
- Familias recuperadas:
  - **Nube** (raíz `95ef783a` «base prueba c528ebb», 29/10): `nube/*`, `recuperat/G-*`, `recuperat/H-ciudad`, `recuperat/prueba-H`, `prueba/claude/integracion-I-30-10`, `agente/claude/BND-04`, `agente/claude/PLAU-U3-E01-E04` (`caa7565a`, la punta más avanzada: prueba I + U3-02/05/07 E01–E04 + arreglos REV-U3).
  - **REPLICA-R1** (raíz `a523d6aa` = foto PLAU01-R1 ≈ 9d52ae8 + U1-02.E01–E04): `replica-r1/U3-02`, `U3-05→U3-06`, `U3-07→U4-01`, `U3-07→U4-07`.
  - **Fotos sueltas** `recuperat/arbre/*` (copias de trabajo antiguas) y la foto del checkout de Oriol `recuperat/software-copia-30-10` (`22c6a7d6`).
- Agentes: codex-cli 0.165.0 y plugin activos; OpenCode con `opencode/glm-5.3-flash-free`, `opencode/deepseek-v4.1-flash-free`, `opencode/beam-free`, Qwen `bailian-token-plan/qwen3.8-max|qwen3.7-plus` (Coding/Token Plan) y `bailian-payg/*` (pago por uso).
- Disco: ~10 GB libres (98 %). Sin ccache.

## UNI-01 — unificación (`agente/claude/UNI-02-11`, worktree `~/onegai-ramas/UNI-02-11`)
Base: `caa7565a`. Qué ya estaba dentro (blob a blob, script `estado.sh`): w2-C3-08 (30/36 iguales, 6 con ediciones posteriores), G-creador (63/64), G-dia1 (27/32), H-ciudad (24/31): **todo integrado** en prueba H/I; las divergencias son ediciones posteriores de la propia línea nube.

Aplicado encima (cherry-pick `-x`, conserva mensajes y autoría):
| Paso | Origen | Commits | Conflictos y resolución |
|---|---|---|---|
| 1 | BND-04 `a68c090b` | 1 | ninguno |
| 2 | U1-02.E01–E04 (diff foto PILOTO-E2 → `a523d6aa`, 3 ficheros) | 1 (`0521058b`) | ninguno |
| 3 | U3-07 `39d1829a..628f417c` | 8 | `Interpolacion.h`/`InterpolacionTests.cpp`: nube y REPLICA arreglaron el mismo hallazgo de Codex de dos maneras. **Se queda la implementación de nube** (monótona y acotada; la de REPLICA, `a*(1-p)+b*p`, es justo la que fallaba el caso de Codex) + nota NaN de REPLICA + **unión** de pruebas (las de nube y las de `tweenVec2`). |
| 4 | U4-01 `628f417c..19817097` | 14 | `CMakeLists.txt`: unión de `include()`/fuentes (resolutor `cmake_union.py`). |
| 5 | U4-07 `628f417c..70c3e605` | 18 | `ApplicationBucle.cpp` ×2: la cámara nueva de U4-01 (`fotogramaCamara`) + las líneas de polvo de U4-07 (E07 y D-O7) justo tras `m_vida->avanza`. |
| 6 | U3-05→U3-06 `0b039f9b..e0042119` | 30 | `CursorClip.{h,cpp}` (E04 r1): **se queda REPLICA** (su `enDominio` ya rechaza `dt*1000` desbordado, el arreglo de nube) + **unión** de pruebas. |
| 7 | U3-02 `02323dac..d59e813e` | 12 | solo contratos `.md` (faltaban en nube → versión de REPLICA). |
| 8 | PIL-E1 y PIL-E2 (fotos del piloto 29-10) | 2 | ninguno |

Contratos `U1-02`, `U3-02`, `U3-05`, `U3-07`: no estaban en nube; quedan con la versión final de REPLICA.

**No incluido (decisión pendiente o zona dudosa):** fotos `recuperat/arbre/*` y foto del checkout de Oriol → `ZONAS_PARA_FABLE.md` (subagente Sonnet, en curso). Visto visual D-O4/D-O5 de REPLICA sigue pendiente de Oriol. Revisiones sin hacer de REPLICA: U3-06.E07 r1 (solo informe) y U4-07.E08 (Opus-adv + Haiku).

| 9 | r1: hallazgo de Codex (tweenEscalar desborda) | 1 (`fa3743f6`) | implementa Sonnet; mutación repetida por el Brain |
| 10 | 8 atlas `editor_*.png` desde `22c6a7d6` | 1 (`f39feaa2`) | `level_editor.cpp:403-468` los carga; no estaban en nube |

Punta: **`agente/claude/UNI-02-11` @ `f39feaa2`**.

## Evidencias (UNI-01)
**Pruebas ejecutadas** (`agente/claude/UNI-02-11` + `macos-debug`):
- Compilación completa sobre `a4d0b6d9`: 696 TU, 67 s, `juego`/`level_editor`/`motor_editor` OK (`~/onegai-ramas/UNI-02-11/out/fabrica/UNI-02-11/build.log`).
- `ctest -L gl-free -LE lenta -j 6` sobre `a4d0b6d9`: **274/287**, 193 s. Fallan 13: test_pergamino, test_ruta_casillas, test_botin_equipo, test_recompensas_economia, test_validar_monstruos_acciones, test_lugares, demo_creacion_personaje, demo_boundington, demo_campana_boundington, test_catalog_content, test_kit_basico, linea_base_core_configura, linea_base_core (`ctest_glfree.log`).
- Mismas 13 + `test_clases_sp` sobre la base **`caa7565a`** (worktree temporal, ya borrado; `base_ctest.log`): **0 fallos nuevos, 1 arreglado** (PIL-E1). Las 13 son las heredadas de los relevos E-L1A/B/C.
- Sobre `fa3743f6`: `ctest -R "interpolacion|tween|camara"` 8/8. Mutación (fórmula antigua) → `test_interpolacion` ROJO (`InterpolacionTests.cpp:229`); restaurada → verde.
**Revisión estática:** Codex (codex-cli 0.165.0, `codex exec -s read-only`) sobre `a4d0b6d9`: 4 de 5 puntos sin hallazgos; 1 medio (desbordamiento en `tweenEscalar`, heredado de nube) → **confirmado** con `-ffp-contract=off` (1.7e308 en vez de 8.5e307) → corregido en `fa3743f6`. Límite que señala: la prueba textual del polvo no vigila el orden respecto a la cámara.
**Qwen:** BLOQUEADO — `bailian-token-plan/qwen3.8-max` responde «Invalid API-key». No se probó `bailian-payg` (pago por uso) sin permiso.
**Pendiente de ejecución gráfica:** jugar `juego` de esta rama (cámara U4-01, animaciones U3-06, polvo U4-07: vistos D-O4/D-O5); `level_editor` con los atlas restaurados.

## Zonas para Fable
`ZONAS_PARA_FABLE.md` (subagente Sonnet, solo lectura). Resumen: casi todo lo de las fotos está superado; quedan por decidir la decoración de `afueras_ocaso` (Qwen), ~60 carteles/puertas de `ciudad_centro.json` de la copia de Oriol, 12 cofres `botin_*`, `ojos_grande.png`, `con.png` (nombre reservado en Windows) y un `.psd`.

## Fase 2 — tancar animacions (Oriol 21:20: «Tancar animacions»; revisor = Codex en lloc de Qwen)
Treball apilat a la mateixa branca `agente/claude/UNI-02-11` (no una per E): només hi ha 7 GB de disc lliures i cada worktree amb build en necessita ~3.

### U3-06.E07 r1 — `bc874299`
- Hallazgos d'origen: Opus-adv i Haiku (REPLICA-R1, 31/10, a `out/fabrica/REPLICA-R1/PROGRESO.md`).
- Implementa: Sonnet (9 min). Ámbito: la prova `VerificacionAnimacionTests.cpp` + informe + 2 logs; `src/` sense canvis (verificat).
- Proves ejecutadas: `ctest -R "verificacion_animacion|maquina|animacion|clip|idle_cuerpo_base"` 13/13. 8 mutacions ROJO (`U3-06.E07_mut.log`). El Brain va repetir la de doble emissió a `ClipActor.cpp:32`: ROJO a `evento_de_clip_exactamente_uno_por_cruce…` (:233) i `…no_ciclico…` (:253); restaurada → 13/13.
- Comprovat pel Brain: `juego.cpp:66` carrega `boundington_npcs.json` per defecte → cal `--catalogo` per veure el slime.
- Conclusió per a Oriol: l'idle del cos base com a clip i H2 **no es veuen al joc d'avui** (cap enemic de nivell hi cau); només per prova.

### U4-07.E08 — revisió Codex sobre `ba50c70c` (21:22–21:30)
7 hallazgos (6 mitjans, 1 baix). Consolidació:
| # | Hallazgo | Estat |
|---|---|---|
| 1 | `hashDe` omet origen/grupo/RNG/residu; comentari fals | confirmat (lectura `EmisorCargaTests.cpp:37-50`) |
| 2 | pressupost global sense frontera exacta 2048/2049 | confirmat (lectura: només `<=2048 && >1024`, :182-183) |
| 3 | quads: 8160/8194 i només contra el comptador | confirmat (lectura :218-226) |
| 4 | polvo sense mínim d'activitat ni 7/31 | confirmat (lectura :289-293) |
| 5 | retall de projecció pot alterar simulació sense control | plausible |
| 6 | log verd amb 5 casos, codi en té 6; temps de l'informe ≠ log | confirmat (`grep -c MOTOR_TEST_CASE` = 6; log «Ejecutados 5») |
| 7 | «cota alta» amb Debug | confirmat (informe :4) |
r1 llançada amb Sonnet (21:31).

## Bloqueos
- Qwen: clave del Token Plan de Bailian inválida (21:19).

## Tiempos
- 20:45–21:03 inventario de ramas y bases.
- 21:03–21:09 unificación (cherry-picks, 5 conflictos a mano) + subagente de zonas en paralelo (11 min).
- 21:09–21:10 compilación (67 s). 21:10–21:13 ctest gl-free (193 s).
- 21:09–21:14 Codex (tras un cuelgue de 2 min: esperaba stdin; relanzado con `</dev/null`).
- 21:14–21:17 corrección Sonnet (82 s) + verificación y base `caa7565a`.
- 21:18–21:19 Qwen: falla al instante (clave).

## Cuelgues
- Codex 21:09: `codex exec` en segundo plano esperaba stdin («Reading additional input from stdin»). Detectado a los 2 min, matado, relanzado con `< /dev/null`.

## Fase 3 — heredadas (Oriol 21:55: «Heretades»)
Diagnóstico: `HEREDADAS.md` (subagente Sonnet, solo lectura, sobre `08b0602d`).

### H1 — 6 pruebas desfasadas/entorno sin decisión de Oriol (Sonnet)
Ámbito congelado (rutas bajo `MotorGraphico-main/MotorGraphico/`), solo pruebas:
- `tests/Game/RutaCasillasTests.cpp` (umbral de tiempo distinto en Debug, `NDEBUG`) — test_ruta_casillas
- `tools/test_validar_monstruos_acciones.py` (id de «estado futuro» que no exista) — test_validar_monstruos_acciones
- `tests/RPG/CatalogContentTests.cpp` (16 → 20 condiciones, PO-37) — test_catalog_content
- `tests/Game/BotinEquipoTests.cpp` (26 → 43 contenedores) — test_botin_equipo
- `tests/Game/CofreKitTests.cpp:66` (`bono.empty()`, AV16-02) — test_kit_basico
- `examples/demo_creacion_personaje.cpp:43,156,172` (72 razas / 69 jugables) — demo_creacion_personaje
Nada de `src/`, `include/`, assets ni catálogos. Cada número nuevo con una mutación del dato que lo haga fallar.

### H2 — Boundington y economía (después de H1, mismo worktree)
- `examples/demo_boundington_partida.h`, `examples/demo_boundington.cpp`, `examples/demo_boundington_alternativas.cpp`, `examples/demo_campana_boundington.cpp` (ayudante `duerme()` que entra en `interior_taberna_p1`)
- `tools/rutas_economia.json` regenerado con `tools/sintetizar_rutas.py` — test_recompensas_economia

### Esperando a Oriol: test_lugares, test_pergamino (5.ª placa), «… 2.cpp» de Finder, rebase de linea_base_core.

### Avance fase 3 (22:00–22:25)
- **H1 `02477277`** VALIDADA: 6 pruebas en verde (catalog 16→20, botin 26→43, kit AV16-02, validar_monstruos id futuro, creacion 72/69, ruta_casillas umbral Debug 20 ms / Release 5 ms). Mutación por dato; el Brain repitió una (catálogo vaciado → ROJO; más burda que la del Sonnet).
- **H3a** (decisión de Oriol): 7 copias de Finder « 2» fuera de la rama, guardadas con su diff en `out/fabrica/UNI-02-11/copias_finder/` para Fable.
- **H2 `0f86c34d`** PARCIAL: `duerme()` (anochece + skilla + `interior_taberna_p1`); demo_boundington pasa prólogo + 17/17 del día 1 y cae en el día 2. Brain: el primer verde/rojo que vio era de un binario viejo (mtime tras `git checkout`); con `touch` + rebuild se confirma. `rutas_economia.json` revertido (la entrada manual no cambiaba nada y se perdería al regenerar).
- Decisiones de Oriol (22:20): lugares «corregir y guardar emblemas»; pergamino «actualizar 3 + arreglar placa»; copias « 2» «fuera y que Fable las mire»; linea_base «rebasar»; **día 2 «reescribir guiones contra boundington_dia_gorran.json sin tocar historia»**; **recorredor «que ejecute cinemáticas»**.
- H3 (lugares + pergamino) lanzada con Sonnet. Siguen en serie: H4 (guiones día 2: `examples/demo_boundington*.cpp`, `demo_boundington_partida.h`, `demo_campana_boundington.cpp`), H5 (`tools/recorrer_recompensas.cpp` + su prueba + `tools/rutas_economia*.json` regenerados), rebase de `planificacion/linea_base_core.json`, suite completa.

### Cierre fase 3 (03/11)
H2+H4 `7fb3368a` (día 2 = Gorran; PENDIENTE ORIOL temporal), H4b `45e66555` (once en 3 beats; 58/58), H3 `6bc14fe3`, H5 parcial `dc6fbb48` (BLOQUEADA: Oriol «otra tanda»), H3b `cf991f58` (rebase). Suite gl-free completa sobre `cf991f58`: 287/288. Corrección al informe de H4: el cargador SÍ lee `once` (`AdventureScriptLoader.cpp:143`); los beats no lo llevaban.
