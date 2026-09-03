# Revisión global del proyecto + auditoría de menús del motor

*Foto del 3 de septiembre de 2026 (tarde) sobre HEAD `50098f8` + trabajo en vuelo declarado (C-11/C-22 de codex aplicados en el workspace, migración INF-03 sin versionar). Encargo de Oriol: revisar todo el proyecto y, en particular, los menús del motor gráfico — qué flujo hacen, qué elementos contienen, qué % está finalizado/inacabado y qué es crítico frente a qué puede esperar. Complementa a `LINEA_BASE.md` (módulos, 28/08), `analisis_clases/` (clases, 31/08) y `COMPARATIVA_CODIGO_UML_PLANIFICACION.md` (planes, 03/09): aquí el eje son **las pantallas y sus flujos**.*

## 0. Titular

- **El proyecto va bien ahí donde ha decidido ir.** La Fase C (consolidación) está casi cerrada —C-09, C-10, C-11, C-20, C-21 y C-22 con núcleo hecho en solo dos días—, la campaña es jugable end-to-end y la verificación (15 suites + suite gl-free + validadores Python) es real. El objetivo vigente (corte B: que un tercero haga un juego sin tocar código) está correctamente planificado en `PLA_DEFINITIU.md`.
- **Los menús del JUEGO están más hechos de lo que parece** (~70% del flujo exploración→diálogo→combate→tienda/negocio funciona y es data-driven, salvo el menú de combate), **pero les faltan las puertas**: no hay pantalla de título, no hay pausa, y la creación de personaje (14 pasos, terminada) no está cableada al juego.
- **Los menús del EDITOR son la brecha real del corte B**: lo que existe (hub de proyectos, 7 herramientas, familias, búsqueda, capas, inspector) está sólido (~65–90% por pantalla), pero **el editor de eventos (EVT-11/12) y la base de datos visual (GDB-02) —el corazón del «ciclo de autor»— están a 0–10%** y son los paquetes más caros del plan (19 j y 8 j).
- **Hallazgo nuevo de esta revisión:** `tools/verificar_controles.py` —el guardián que impide que los menús del editor documenten teclas fantasma— **está roto por la migración INF-03** (busca `examples/level_editor.cpp`, que ahora vive en `editor/src/`). Mientras esté roto, la trampa documentada en la Parte 8.7 de la bitácora (controles anunciados sin existir) puede volver a morder sin que nada lo detecte.

---

## 1. Estado global de los cuatro proyectos

| Proyecto | Papel | Estado | Señales |
|---|---|---|---|
| `MotorGraphico-main` | Ejecución (C++) | **Fase C casi cerrada**; 101 cabeceras, ~23,8k LOC `include+src`; suite gl-free 52–54/54; rama `feature/editor-s1-contratos` | 6 paquetes C tocados en 2 días; **INF-03 sin commitear** (5 módulos + `editor/` + `tests/` untracked, ~25 M) |
| `dndWeebCC-master` | Reglamento y mapa | Estable; Java 25 en rama de upgrade | 1 fichero M (pom del upgrade); Ed.3 aún sin reflejar en el GDD §5 (bandeja antigua) |
| `Onegai 2` | Canon (LORE) | Estable desde el volcado del atlas (28/08) | 229 ficheros sin commitear (capítulos/canvas en curso de Oriol) — no tocar |
| `Onegai-Core` | Puente | **Ya tiene git** (la bitácora 4.3 decía que no; se inicializó con los seguimientos de C-22) | 169 ficheros sin commitear — pendiente de cierre de sesión |

**Riesgos transversales** (detalle y preguntas en `COMPARATIVA_CODIGO_UML_PLANIFICACION.md` §4–5): INF-03 sin versionar es el nº1 de colisión; el UML y el flujo .puml congelados desde el commit inicial; tres numeraciones de plan conviviendo; los docs de estado (ESTADO_REAL 28/08) subestiman lo hecho desde entonces.

**La cifra de proyecto** (del plan reconciliado): 1.0 completo al 32%; corte B = 177 jornadas ≈ 118 semanas a ritmo real. La Fase C (62 j) está consumiéndose a buen ritmo esta semana.

---

## 2. Los flujos de menú del motor

### Flujo A — Editor (`level_editor`): arranque → proyecto → edición → prueba

```
arranque
  └─ ProjectHub (GL-free, probado en demo_proyectos)
       modo List ── W/S mueve, ENTER abre ──────────┐
       modo NewProject ─ N: teclear id [a-z0-9_] ───┤
       modo Message ── B: build_proyecto.py ─────────┤
       ESC ──> salir                                 │
                                                     v
  pantalla de EDICIÓN (HUD propio: paneles+texto, teclas)
       herramientas 1..7 (pintar/borrar tile, poner/quitar objeto,
                          inicio jugador, rellenar, enlazar nivel)
       paletas      Q/E navegar · TAB familias · [ ] paginar · B buscar
       capas        C siguiente · SHIFT+C anterior · CTRL+N nueva
       selección    R marca rectángulo · CTRL+C/V/X portapapeles
       instancia    T patrulla · H escala · click-der INSPECTOR
       historial    CTRL+Z / CTRL+Y (snapshot completo)
       ficheros     G guardar · V validar · P probar (lanza ./juego aparte)
       navegación   , / . escenario anterior/siguiente · M/ESC volver al hub
                     v
  ./juego --nivel <este> --catalogo <el del proyecto>  (proceso aparte;
  guarda ANTES de lanzar; a la vuelta KeyEdge::prime() evita el ESC fantasma)
```

El inspector (click derecho sobre objeto) es un menú modal propio: W/S navega campos, ENTER edita/elige, X quita, ESC cierra. Campos por instancia: nombre, botín (picker ElegirLoot con filtro), evento, transición (picker ElegirSalida), variante, escala, propiedades libres, texto editable.

### Flujo B — Juego (`juego`): arranque directo, sin título

```
arranque → NIVEL (argv --nivel/--catalogo; por defecto interior_vacio)
  │   [NO HAY pantalla de título, NI menú principal, NI "continuar"]
  │
  ├─ EXPLORACIÓN (HUD: status, PV/PM, minimapa, moral, inventario TAB,
  │               hints, FPS?, timer-bar [creada, invisible, sin alimentar])
  │     WASD/flechas mover (celda a celda) · E hablar/cruzar puerta
  │     TAB inventario (solo lectura) · F5/F9 guardar/cargar (atómico)
  │
  ├─ DIÁLOGO (modal): HudDialogueBox envuelve texto; con opciones
  │     W/S+ENTER (mismo lenguaje de menú que el combate); sin opciones,
  │     cualquier tecla de confirmación cierra
  ├─ COMBATE (modal): menú Atacar/Tajo/Cura/Pocion/Huir — ¡HARDCODEADO
  │     en Application.cpp:573! (comentario: "a mano y no desde JSON");
  │     log Nd6 con grados Fracàs/Èxit (bandeja) en HudDialogueBox
  ├─ TIENDA (modal): SÍ data-driven — ofertas del catálogo + VENDER
  │     agrupado con precios; "(NADA QUE COMERCIAR)" si vacío
  ├─ NEGOCIO (modal): ficha EN VENTA/PRECIO/RENTA + comprar o fijar
  │     alquiler (moral sube/baja según exprimas a los inquilinos)
  └─ GAME OVER: una línea de hint ("FIN DE LA PARTIDA ESC SALIR")
```

Todos los modos modales se cierran con `closeInteraction()` y vuelven a exploración; el estado (PV, oro, inventario, moral, negocios) sobrevive.

### Flujo C — Creación de personaje: terminado y **desconectado**

`CharacterCreationScreen` (GL-free, 14 pasos: Raza→Clase→Trasfondo→Reparto de stats→Vínculo→Miedo→Defecto→Meta→Ideal→Personalidad→Virtud→Deidad (opcional)→Nombre→Resumen) con filtro por texto, puntos libres, avisos y validación final. La ejercitan `examples/creacion_personaje.cpp` (huérfano de compilación) y `demo_creacion_personaje`. **Ninguna ruta del `juego` pasa por ella.**

---

## 3. Auditoría elemento a elemento

Leyenda de criticalidad: **CRÍTICO** = bloquea el corte B o es riesgo activo ahora · **IMPORTE** = dentro del corte B pero no bloquea hoy · **ESPERA** = 1.1 o decisión de Oriol. Los % combinan mi lectura de código con las fichas de `ESTADO_REAL_vs_ROADMAP.md` (28/08).

### 3.1 Editor — pantalla de proyectos (ProjectHub)

| Elemento | Estado | % | Criticalidad |
|---|---|---:|---|
| Lista navegable con estado jugable por proyecto | hecho, probado (demo_proyectos) | 90 | — |
| Crear proyecto (valida id/prefijo, no pisa manifiesto suelto) | hecho | 90 | — |
| Build desde la pantalla (llama a tools/) | hecho (idempotente) | 85 | — |
| Panel de detalle (niveles/aventuras/huérfanos) | hecho | 80 | — |
| Borrar proyecto desde la UI | `ProjectIndex::remove()` existe pero el hub no expuse tecla | 40 | ESPERA |
| Árbol de mapas navegable (EDT-04) | solo ciclo `,`/`.` entre escenarios | 40 | IMPORTE |

### 3.2 Editor — pantalla de edición

| Elemento | Estado | % | Criticalidad |
|---|---|---:|---|
| 7 herramientas (pintar/borrar tile, poner/quitar objeto, inicio, rellenar, enlazar nivel) | funcionan; faltan **cuentagotas** y **rectángulo que pinta** (EDT-08/09) | 75 | IMPORTE |
| Paleta de tiles con etiquetas/familias/filtro/paginación (EDT-05/06) | sólida; sin miniaturas visuales | 75 | miniaturas ESPERA; resto — |
| Búsqueda de objetos (B) con filtro por texto | hecho | 90 | — |
| Capas: crear/renombrar/activar (EDT-10) | faltan visibilidad, bloqueo, opacidad, reordenar | 60 | IMPORTE |
| Selección rect. + portapapeles (CTRL+C/V/X, 1 paso de historial) | hecho (EDT-13) | 85 | — |
| Deshacer/rehacer (EDT-12) | snapshot completo: funciona, escala mal en memoria | 70 | ESPERA (decisión REPLANTEAR ya registrada) |
| Inspector de instancia (EDT-11) | botín/evento/transición/variante/escala/props libres con pickers | 80 | — |
| Guardar (G) + validar (V) | guardar a petición; **sin autosave ni recuperación** (EDT-14) | 45 | **CRÍTICO** (un cuelgue pierde horas del autor) |
| Probar (P): guarda y lanza ./juego aparte (TOOL-01) | hecho — la auditoría 28/08 lo daba al 10%; **está más hecho que documentado** | 70 | — |
| Probar desde la celda seleccionada (TOOL-02) | no existe (siempre arranca en el inicio del jugador) | 0 | IMPORTE |
| Zoom/pan/picking isométrico | hechos y en uso diario | 85 | — |
| HUD de estado del editor (herramienta activa, capa, coordenadas) | barra de estado al día (verificar_controles la vigilaba… ver §6) | 80 | — |

### 3.3 Editor — menús que NO existen (y su peso en el corte B)

| Menú | Ficha | % | Criticalidad |
|---|---|---:|---|
| **Editor de páginas de evento** (EVT-11) | 9 j | 5 (solo `setObjectEvent()` como semilla) | **CRÍTICO** — sin él no hay ciclo de autor |
| **Editor vertical de comandos de evento** (EVT-12) | 10 j — el paquete más caro del roadmap | 0 | **CRÍTICO** — es LA pieza «sin tocar C++» |
| **Base de datos visual maestro-detalle** (GDB-02) | 8 j | 10 (patrón de menús del hub) | **CRÍTICO** para corte B (los 21 catálogos se editan a mano en JSON) |
| Depurador paso a paso de eventos (EVT-15) | 7 j | 0 | IMPORTE (la fase E del corte B lo pide) |
| Panel de propiedades del MAPA (MAP-06) | 5 j | 30 (campos en el formato) | IMPORTE |
| Editor visual de tilesets (MAP-01) / autotiles (MAP-03) | 8+9 j | 35 / 0 | ESPERA (1.1 explícito) |

### 3.4 Juego — HUD de exploración

| Elemento | Estado | % | Criticalidad |
|---|---|---:|---|
| Status del jugador + barras PV/PM | hecho, envuelto/recortado (F0 cerrado el 28/08) | 90 | — |
| Minimapa | hecho (HudMinimap, con test) | 85 | — |
| Inventario (TAB, agrupado, solo lectura) | hecho como panel; **no se puede usar/equipar desde él** | 60 | IMPORTE (equipo/gestión: GDB-05 tiene motor, no menú) |
| Moral (etiqueta + barra villano↔héroe) | hecho | 85 | — |
| Diálogo/aviso por umbrales de timer | el beat llega; **la barra `m_timerBar` está creada, invisible y nadie la alimenta** — F1 cojo de HUD (mi propia fila del TABLERO) | 50 | IMPORTE (cierra F1) |
| Contador FPS | en pantalla sin decisión de producto | — | ESPERA (en bandeja desde el 28/08) |
| Acentos en etiquetas | la fuente los dibuja; quedan decisiones sueltas (veda JSONs, `Fracàs/Èxit`) | — | ESPERA (bandeja) |

### 3.5 Juego — modos modales

| Modo | Estado | % | Criticalidad |
|---|---|---:|---|
| Diálogo con elecciones | beats con opciones W/S+ENTER, wrap del texto | 85 | — |
| Combate: bucle Nd6 Ed.3 + log | completo y validado (campaña 72/72) | 85 | — |
| Combate: **menú de comandos** | `{"Atacar","Tajo","Cura","Pocion","Huir"}` **hardcodeado** en Application.cpp:573 (el comentario lo confiesa) | 30 | **CRÍTICO** — GDB-7 «combate configurable» pasa por aquí; hoy las habilidades reales del PJ no aparecen en SU menú |
| Combate: selección de objetivo | no hay menú de objetivo (índices fijos del flujo actual) | 20 | IMPORTE |
| Tienda | data-driven (ofertas + VENDER agrupado, misiones no vendibles) | 80 | — |
| Negocio (comprar + fijar alquiler con moral) | jugable; ficha + menú | 75 | — |
| Game Over | una línea de hint; sin opciones (¿cargar partida?) | 20 | IMPORTE (F9 existe pero nada lo dice) |

### 3.6 Juego — pantallas que NO existen

| Pantalla | % | Criticalidad |
|---|---:|---|
| **Título / menú principal (nueva partida·continuar·salir)** | 0 | **CRÍTICO** para corte B: un tercero arranca dentro de un interior sin contexto ni forma de elegir su guardado (GDB-12) |
| **Pausa** (reanudar/guardar/cargar/salir) | 0 | **CRÍTICO**: F5/F9 disparan sin confirmación desde cualquier frame de exploración |
| Integración de la creación de personaje en el arranque de partida | 0 (pantalla misma: 85) | **CRÍTICO**: el 14-pasos está terminado y desconectado |
| Menú de equipo/stats del PJ (diario de misiones incluido, GDB-08) | 0–10 | IMPORTE |

---

## 4. Resumen de porcentajes

| Área | % aproximado | Base del cálculo |
|---|---:|---|
| Menús del editor QUE EXISTEN (hub + edición + inspector) | **~70%** | elementos de §3.1–3.2 |
| Menús del editor NECESARIOS para corte B (añade eventos+BD+deps) | **~35%** | §3.1–3.3 ponderado por esfuerzo de las fichas |
| Menús del juego QUE EXISTEN (HUD + 5 modos) | **~70%** | §3.4–3.5 |
| Menús del juego NECESARIOS para corte B (añade título/pausa/creación/equipo) | **~50%** | §3.4–3.6 |
| Proyecto entero (motor, no solo menús) | 32% al 1.0 · Fase C casi cerrada | `ESTADO_REAL` + progreso 28/08→03/09 |

Lectura: **el déficit no es de calidad sino de cobertura** — lo hecho funciona y está probado; lo que falta son justamente las pantallas que convierten el motor en herramienta de autor (editor) y en producto arrancable (juego).

## 5. Qué es crítico y qué puede esperar

**CRÍTICO — bloquea el corte B o es riesgo activo HOY**
1. **Commitear INF-03 y reactivar `verificar_controles.py`** (path `examples/`→`editor/src/`). El guardián de los menús del editor lleva roto desde la migración; la trampa de los controles fantasma (bitácora 8.7) ya mordió una vez sin red.
2. **Editor de eventos EVT-11+EVT-12** (19 j, 0–5%): sin él no existe el «ciclo de autor completo» que da nombre al corte.
3. **Puertas del juego**: título con nueva/continuar, pausa con confirmación, y cablear la creación de personaje al arranque (la pantalla ya está hecha).
4. **Menú de combate data-driven** (Application.cpp:573): hoy un PJ con otras habilidades no las ve en su menú.
5. **Autosave/recuperación del editor** (EDT-14): el resto del editor está demasiado bueno para perder partidas de trabajo por un cuelgue.

**IMPORTANTE — en el corte B, no bloquea hoy**: GDB-02 base de datos visual · TOOL-02 probar desde celda · panel de capas completo · cerrar F1 (barra de timer) · menú de objetivo en combate · Game Over con opción de cargar · inventario gestionable · adelgazar Application (tienda/negocio/UI de combate viven allí).

**PUEDE ESPERAR — 1.1 o decisión de Oriol**: editor visual de tilesets y autotiles · miniaturas del navegador · consola/inspector en vivo (TOOL-04) · audio (GDB-11, requiere decisión de dependencia) · exportación multiplataforma (TOOL-10) · borrar proyectos desde el hub · FPS/etiquetas catalán/veda de tildes (ya en bandeja).

## 6. Hallazgos nuevos de esta revisión

1. `tools/verificar_controles.py` **roto por INF-03** (FileNotFoundError contra `examples/level_editor.cpp`) — registrado también en el TABLERO (Pendiente) para quien cierre la migración.
2. **Menú de combate hardcodeado** (Application.cpp:573) frente a una tienda que SÍ es data-driven — el contraste delata que fue un atajo puntual que se quedó.
3. **TOOL-01 (playtest) está más hecho de lo que dice la auditoría del 28/08** (10% → ~70% en mi lectura + Parte 8 de la bitácora): segunda prueba de que `ESTADO_REAL` necesita nota de foto.
4. **Onegai-Core ya tiene git** (bitácora §4.3 dice «sin git»): inicializado con los seguimientos de C-22.
5. `m_timerBar`: creada, `setVisible(false)` y sin consumidor — confirma el punto 5 del INDEX de analisis_clases; el cierre pasa por mi fila F1 del TABLERO.
6. Game Over es hoy una línea de texto; la carga (F9) existe pero el juego no la ofrece en ese momento.

## 7. Verificación

- Fuentes de código leídas: `ProjectHub.h`, `CharacterCreationScreen.h` (enum Paso completo), `GameSession.h` (GameMode + OwnedBusiness), `HudWidgets/HudTextWidgets/HudMinimap/HudManager.h`, cabecera de `editor/src/level_editor.cpp` (controles completos), `Application.cpp` (init del HUD completo, refreshHud, processInput por modo, confirmBattleCommand, tienda/negocio/gameover), `ObjectInspector.h`, `juego.cpp`, `BattleState.cpp` (degreeLabel).
- `python3 tools/verificar_controles.py` ejecutado: falla con FileNotFoundError (hallazgo §6.1, evidencia en la salida).
- Estado de los 4 repos con `git log/status`; % de fichas cruzados con `ESTADO_REAL_vs_ROADMAP.md` y `PLA_DEFINITIU.md`.
- Sin cambios de código; trabajo concurrente (C-11/C-22 de codex aplicado al workspace, INF-03 sin versionar) intacto.
