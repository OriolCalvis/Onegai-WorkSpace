# F-REST 4.2–4.4 — cierre de la restauración de la foto del 30/10

- **Fecha:** 2026-11-04 (noche, tras la integración de UNI-02-11)
- **Autor:** Claude (Code tab, Fable 5.1) + un subagente Sonnet de solo lectura para las copias « 2»
- **Encargo:** Oriol, «Acabar restauració»: las tres zonas de `out/fabrica/UNI-02-11/ZONAS_PARA_FABLE.md` que quedaban abiertas tras F-REST.1 (decorado de afueras al ocaso) y F-REST.5 (icono CON). Su criterio del 03/11: «més que restaurar, adaptar la idea a les funcionalitats pactades actualment».
- **Base:** checkout de Oriol, `recuperat/software-copia-30-10` @ `dabdb015` (ya con UNI-02-11 y UNI-RESTAURA dentro).
- **Resultado en una línea:** **no queda nada que restaurar.** Las tres zonas están superadas por lo que ya hay en la rama; el único cambio de ficheros es documental.
- **Deja esperando a Oriol:** nada bloqueante; dos hallazgos abajo.

## 4.2 — `ciudad_centro.json`: carteles, `estrado_sermon`, puertas

Comparación por instancias (objectId + casilla) entre la foto (`5ee2be05`, 281 objetos) y la rama (248 objetos): 119 instancias solo en la foto, 86 solo en la rama, **0 con el mismo id en la misma casilla movido**.

- **Los ~60 `cartel_*` de la foto** (4–5 casillas por tienda: banco, baños, herrería, joyería, mercado, ópera, posada, sastrería, taberna…) son el cartel **estático** anterior a AV15-02. Hoy los pinta el motor en tiempo de ejecución: `src/HUD/Pergamino/CartelesPuertas.cpp` (D0015 §14) pone sobre cada puerta con sitio en `lugares.json` un retal con emblema, nombre y función. Copiarlos duplicaría el cartel y, además, en casillas de **otra ciudad**: no se restauran.
- **Las 22 puertas «en otra posición»**: el `ciudad_centro.tmx` de la foto difiere en 30 líneas del actual; la foto es la ciudad anterior a la regeneración (AV16-09b y posteriores). Las puertas de la rama son las que validan `validar_puertas` y `validar_mundo_unido` (143 niveles alcanzables, 0 fuera del mundo). No se tocan.
- **`puerta_coliseo` + `cartel_coliseo`**: en la rama el coliseo (`interior_coliseo.json`, `_s1`) cuelga de `ciudad_oeste.json` y es alcanzable; la puerta en el centro era de la ciudad vieja. No se restaura.
- **`estrado_sermon`** (3 casillas en (39–41,52), junto a la iglesia vieja en (42,50)): en el catálogo actual es `"Estrado de tablas (marcador)"`, `spriteId 3`, un **marcador de relleno**, no arte. En la rama la escena del sermón vive en los actores (`verina_estrado` (55,53), `captor_estrado` ×5 en y=54, `sandes_plaza`, `edul_plaza`) y el disparador `plaza_sermon` lleva al jugador a (55,55) «delante del estrado». Poner el marcador debajo de los actores no aporta nada visible. **Cuando haya sprite real de estrado**, su sitio es (54–56,53), bajo Verina y los captores; se deja anotado, no se coloca.
- `arbol_seco`, `botin_ciudad_saco_4`, antorchas, flores, farolas y PNJ de raza de la foto: decoración de la ciudad vieja, ya colocada de otra forma en la nueva.
- Otros niveles «REVISAR A MANO» de ZONAS, comprobados por instancias: `interior_taberna` (la rama es superconjunto; `tranca` → `tranca_taberna`), `interior_castillo_p2` (0 diferencias), `interior_herreria` (foto `armero_vacio` / rama `estanteria_frascos`: decorado distinto, no pérdida), `tienda_abandonada` (`cajas_almacen` y `barriles_rotos` **ya están** en la rama en las mismas casillas), `ciudad_surysal` (solo posiciones de puertas de la ciudad vieja).

## 4.3 — cofres `botin_*`

- La foto tenía 58 contenedores en los niveles comunes, la rama 46 (+ variantes). ZONAS pedía regenerar con `tools/repartir_botin.py` antes de copiar a mano.
- `repartir_botin.py --seco` sobre la rama: presupuesto de los tres días cuadrado en las cinco regiones (15 OK), 57 contenedores. `validar_botin.py` sobre el estado commiteado: **pasa** (50 reutilizables, 12 únicos, 30 tablas, 55 instancias). Es decir, el reparto commiteado es el validado; los 12 cofres extra de la foto son el reparto antiguo de AV18-05b en la ciudad vieja. **No se copian.**
- **Hallazgo (no arreglado, fuera de encargo):** ejecutar `repartir_botin.py` de verdad reescribe **25 niveles** (545+/444−) y el resultado **rompe su propio validador**: `validar_botin.py` → `ERROR Reparto supera presupuesto: 2/cueva/entorno 40`. La herramienta ya no es idempotente respecto a lo commiteado (última edición: base del 29/10). Revertido con `git checkout -- assets/levels`; el árbol queda como estaba. Toca mirarlo en la tanda de economía (2.4/2.5 de las respuestas del 03/11), **no ejecutar `repartir_botin.py` sin `--seco` hasta entonces.**

## 4.4 — las 7 copias « 2» del Finder

Revisadas por un subagente de solo lectura a partir de `out/fabrica/UNI-02-11/copias_finder/*.diff` (1.273 líneas), buscando en el árbol actual cada identificador que la copia tuviera de más. **Las 7 son versiones viejas; nada perdido** (clase A). Comprobado a mano después: `pintaDerrota` está en `InterfazDerrota.cpp:14`, `golpes()`/`dibujaSuelo` en `PergaminoGL.h:72/82`, `BattleMenuState.cpp` no existe (retirado por AV13-35/D0016).

| Copia | Qué tenía de más | Dónde está hoy |
|---|---|---|
| `CMakeLists 2.txt` | `BattleMenuState`, `BattleMenuActions`, `TargetSelection`, `QuestLogState`, `demo_battle` y sus `.cmake` | retirados a propósito (AV13-35, AV14-22/D0016; comentarios en `CMakeLists.txt:238/318/434`) |
| `LevelEditorPaleta 2.cpp` | ids `sagas_de_la_locura/abismo` | renombrados a `drow_de_la_locura/abismo` |
| `demo_boundington_partida 2.h` | nada | a la copia le faltan `empiezaElDesayuno`, `subeConRosita`, FANTASMA-01 |
| `PergaminoGL 2.h` | nada | a la copia le faltan `golpes()`, `suelo()`, `dibujaSuelo` |
| `ApplicationPantallas 2.cpp` | `vistaDeBatalla(battle, &catalog)`, `MoveWest/East` en menús, `turnoRival` | firma de un argumento; `SideLeft/Right`; `rivalesAndan/rivalesPegan` (AV13-12) |
| `CartasJugar 2.cpp` | `m_turno.abierto = !principal`, `cerrarTurno`, mapa por enemigo | `sigueAbierto()`, `gastarPrincipal` (AV13-28), clave `(lado, i)` |
| `InterfazPergamino 2.cpp` | `pintaDerrota`, «Entrar en», «EN PAUSA», «COFRE» | `InterfazDerrota.cpp`; `Lugares`, `GuiaMapa`, `MapaGrande` |

Las copias siguen en la historia de la foto (`5ee2be05`) y en `copias_finder/` por si alguien quiere mirarlas; no hace falta conservarlas en el árbol.

## Limpieza

- Worktree `~/onegai-ramas/UNI-RESTAURA` retirado (`git worktree remove`): estaba limpio, sin ficheros sin versionar ni imágenes, sin `out/build` (516 MB de árbol). La rama `agente/fable/UNI-RESTAURA` se queda, ya fusionada en `6189fc7f`. Norma de Oriol del 04/11: tanda acabada e integrada → fuera sus copias.
- `UNI-02-11` **no** se toca: otra sesión trabaja ahí.

## Para Oriol

1. `repartir_botin.py` desincronizado de su validador (arriba, 4.3). Hasta la tanda de economía, solo `--seco`.
2. Si quieres el estrado del sermón con arte de verdad, pídelo como sprite; su casilla ya está decidida (54–56,53).
3. ZONAS_PARA_FABLE queda cerrado entero: 4.1 y 4.5 en F-REST.1/.5, 4.2–4.4 aquí.
