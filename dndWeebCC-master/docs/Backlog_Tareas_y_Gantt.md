# Backlog exhaustivo + Gantt del proyecto ONEGAI

*Consolidación de todo lo pendiente en tareas pequeñas y programables, cruzando
`Plan_Cierre_y_Revision_Diseno.md` (50 tareas, bloques A–M), `Estat_Projecte_i_DAFO.md`
(pendientes P3–P14) y el workstream nuevo de Aventuras por Actos
(`Plan_Implementacion_Constructor_Aventuras.md`, GDD §20). Revisión: 23 jul 2026.*

## Cómo leer esto

- **ID** = workstream + número (`D3`). Los workstreams son las 12 líneas del Gantt.
- **Est.** = días de trabajo ideales para un dev en solitario (0,5–3 d por tarea; nada más
  grande, para que todo sea una "tasca pequeña" cerrable en una sentada).
- **Dep.** = qué debe estar hecho antes.
- **Prio.**: 🔴 alta · 🟠 media · 🔵 baja/diferible.
- **Estado**: ⬜ pendiente · 🟡 parcial · ✅ hecho (se listan los parciales para no perderlos).

Total pendiente estimado: **~118 tareas · ~180 días de trabajo** (≈ 20 semanas a ritmo
sostenido en solitario). El Gantt visual acompaña este documento en
`Gantt_Proyecto_ONEGAI.html`.

---

## WS-Z · Blindaje e higiene técnica — ✅ COMPLETADO 23 jul 2026

*Al contrastar el backlog con el código real, cuatro de las cinco tareas ya estaban hechas
(la deriva docs↔código que avisaba el DAFO). Solo faltaba el backup; ya implementado.*

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| Z1 | Verificar `.gitignore` y commit base | 0,5 | — | 🔴 | ✅ `.gitignore` completo (`target/`,`logs/`,`data_backup/`,`*.bak`) + Git con historial |
| Z2 | Backup automático de `data/` en cada arranque | 0,5 | — | 🔴 | ✅ `config/CopiaSeguridadDatos.java` (zip a `data_backup/` + rotación 10, props `onegai.backup.*`) |
| Z3 | Retirar ficheros huérfanos `flotantes.*` | 0,5 | — | 🟠 | ✅ ya eliminados en commit previo (refs restantes = solo comentarios del sistema de paneles) |
| Z4 | Páginas de error 404/500 personalizadas | 1 | — | 🟠 | ✅ `templates/error.html` |
| Z5 | Manejo de excepciones en controladores | 1 | Z4 | 🟠 | ✅ `config/ManejadorErroresGlobal.java` (@ControllerAdvice: `IllegalArgumentException`→404 · `Exception`→500, con logging) |

> **Verificación pendiente en máquina real:** `./mvnw spring-boot:run` para confirmar que
> `CopiaSeguridadDatos` genera el primer `data_backup/onegai-data-*.zip` sin afectar al arranque
> (no compilable en el sandbox: sin Maven/JDK de build).

## WS-A · Contenido edición 2 (Bloque A · P4) — ✅ COMPLETADO 27 jul 2026

*Al contrastar el backlog con el código real, cuatro de las once tareas ya estaban hechas
(A1, A4, A10, A11) — deriva docs↔código. Las 7 restantes se han cerrado en una pasada
con dos scripts reproducibles y verificación en navegador.*

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| A1 | Decidir con el usuario qué aporta mecánicamente cada uno de los 12 meses (solo sabor vs. bono estacional) | 0,5 | — | 🔴 | ✅ Decisión: **narrativo puro** (GDD §16: virtud/defecto/objetivo + recompensa por interpretar) |
| A2 | Regenerar 5 clases iniciales a ed. 2 (SAB→CAR + pilas) | 2 | — | 🔴 | ✅ `class_guardian_iron`, `class_wandering_blade`, `class_road_shadow`, `class_arcanist`, `class_dawn_voice` (esquema híbrido Java+GDD) |
| A3 | Regenerar 5 razas iniciales (bonos CON/DES/INT/CAR) | 1 | — | 🔴 | ✅ `race_human_marches`, `race_elf_canopy`, `race_dwarf_deepforge`, `race_orc_gongorguma`, `race_ascaria_fey_blood` (statBonuses + racialTrait + activeTrait) |
| A4 | Escribir los 12 trasfondos-mes finales | 2 | A1 | 🔴 | ✅ 12/12 archivos en `data/cartas/transfondos/` al esquema §4b (characterCreation completo, sin mecánica) |
| A5 | Regenerar 10 habilidades físicas de ejemplo (con `recovery`) | 1 | — | 🔴 | ✅ 10 del GDD §17.1 + 4 "como ejercicio" = 14 habilidades canónicas (ids `skill_*`) |
| A6 | Regenerar 10 hechizos de ejemplo | 1 | — | 🔴 | ✅ 10 hechizos del GDD §17.2 (ids `spell_*`) |
| A7 | Regenerar 10 pasivas de ejemplo | 1 | — | 🔴 | ✅ 10 pasivas del GDD §17.3 (ids `passive_*`) |
| A8 | Regenerar 10 objetos de equipo de ejemplo (símbolos de compatibilidad) | 1 | — | 🔴 | ✅ 10 equipo canónico del GDD §17.4 + 5 extras iniciales (`item_parry_bracer`, `item_tool_kit`, `item_light_robes`, `item_simple_dagger`, `item_light_gear`) para que las clases puedan equipar su startingEquipment completo |
| A9 | Regenerar 5 personajes de ejemplo (fórmula vida ed. 2: CON×3 + CAR×1) | 1,5 | A2,A3 | 🔴 | ✅ 5 personajes con historia + trasfondo-mes: Borkun (1001), Liesel (1002), Renn (1003), Aine (1004), Grosh (1005) |
| A10 | Actualizar tabla comparativa de personajes (GDD §18) | 0,5 | A9 | 🟠 | ✅ Tabla en GDD §18 ya usa fórmula nueva (CON×3 + CAR×1 + techo por tier) |
| A11 | Retirar avisos "edición 1" del GDD §14–18 (Bloque M-49) | 0,5 | A2–A10 | 🟠 | ✅ GDD §14–18 limpio (sin rastros de SAB ni esquemas viejos) |

> **Implementación:** `scripts/generar_canonicos_ed2.py` (genera 59 cartas + 5 personas a
> partir de los JSON inline del GDD) y `scripts/renombrar_canonicos_es_en.py` (renombra 5
> cartas existentes ES→EN y actualiza 64 referencias cruzadas en 50 archivos). Verificación
> en navegador: 18/18 URLs canónicas HTTP 200 con nombre correcto, 0 errores de plantilla.
>
> **Deuda preexistente detectada (no culpa de WS-A):** `data/cartas/enemigos/itx_centinela_de_salmuera.json`
> referencia la condición `ralentizado` que no existe. Fuera del alcance de este WS.

## WS-B · Contenido vivo y catálogos delgados (P5 · P6)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| B1 | Ampliar dotes 6 → ~30 con el generador | 1 | — | 🟠 | ⬜ |
| B2 | Ampliar rasgos 2 → ~20 con el generador | 1 | — | 🟠 | ⬜ |
| B3 | Completar condiciones: las 10 del manual como cartas + variantes | 1 | — | 🟠 | ⬜ |
| B4 | Escribir 15–20 eventos históricos del mundo (activa cronología) | 2 | — | 🔴 | ⬜ |
| B5 | Ajuste fino de `recovery` carta a carta (Bloque B-11 pendiente) | 1 | A5,A6 | 🟠 | 🟡 |

## WS-C · Conexiones de personaje y reglas de servidor (Bloques C·E·F)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| C1 | Conectar invocaciones activas a `Personatge` (campo + Zona 5 de la ficha) | 1,5 | — | 🟠 | 🟡 |
| C2 | Conectar condiciones/estados activos a `Personatge` (Zona 5) | 1 | C1 | 🟠 | ⬜ |
| C3 | Lógica de sustitución de `evolvesInto` al subir de tier (`PersonatgeService`) | 1,5 | — | 🟠 | 🟡 |
| C4 | Símbolos de afinidad ✦☠✝♞⚙ en `TierEquipment`/`TierClass` | 1,5 | — | 🟠 | ⬜ |
| C5 | Validación real de `requiredTags`/`incompatibleTags` en servidor | 1,5 | C4 | 🟠 | ⬜ |
| C6 | Límite de 3 consumibles simultáneos (`PersonatgeService`) | 0,5 | — | 🟠 | ⬜ |
| C7 | Límite de invocaciones activas según INT | 0,5 | C1 | 🟠 | ⬜ |
| C8 | Multiclase: `claseSecundariaId` + tabla de mano reducida (§11.1) | 1,5 | — | 🟠 | ⬜ |
| C9 | Multiclase: formulario + ficha con reparto de habilidades entre clases | 1,5 | C8 | 🟠 | ⬜ |

## WS-D · Motor de combate y estado de sesión (Bloque D · B12 · P3 · P7)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| D1 | Servicio de tirada: pool `Nd6`, éxitos/medios éxitos, CD, crítico y pifia | 2 | — | 🔴 | ✅ `combat/motor/TiradaD6.java` (`tirar`, `superarCD`, constantes CD, `ResultatTirada`) |
| D2 | Tests unitarios del motor de tiradas (aislados, deterministas con seed) | 1 | D1 | 🔴 | ✅ `test/combat/motor/TiradaD6Test.java` (13 tests: tablas, crítico, pifia, seed, validación) |
| D3 | Ventaja/desventaja como tablas de conversión + anulación mutua | 1 | D1 | 🟠 | ✅ `Avantatge.resoldre()` cableado en `TiradaD6.tirar(numDaus, nAvant, nDesavant, rng)` + `AvantatgeTest.java` |
| D4 | Estado de turno transitorio (principal/movimiento/reacción/canalización) | 2 | — | 🟠 | ✅ `combat/motor/TipusAccio.java` (mapeo de `actionType`) + `combat/motor/EstatTorn.java` (consumo/recuperación de recursos) |
| D5 | Iniciativa determinista (`DES` + mods, sin tirada) | 0,5 | D4 | 🟠 | ✅ `combat/motor/OrdreIniciativa.java` (ordena; el cálculo del valor ya existía en `PersonatgeService.calcularFicha()`) |
| D6 | Catálogo de las 10 condiciones aplicables en juego (Sangrado, Caído…) | 1,5 | D4 | 🟠 | ✅ `combat/motor/Condicio.java` (catálogo + descripción) + `combat/motor/NivellFatiga.java` (Fatiga, la única con grados) |
| D7 | Muerte y agonía: Moribundo, salvación `CON`d6, Carga del Destino | 1,5 | D1,D4 | 🟠 | ✅ `combat/motor/SalvacioPerMort.java` (tirada + mort instantània) + `combat/motor/CargaDelDesti.java` (ús únic) |
| D8 | Sesión de combate en vivo → estado real de pilas Activa/D.corto/D.largo | 2 | D4 | 🔴 | ✅ `combat/motor/Recovery.java`+`Pila.java`+`EstatPiles.java` (motor puro) + `combat/SessioCombatService.java` (`@Service` en memoria, un `EstatPiles` por personatge, sin persistencia a disco — es transitorio de verdad) |
| D9 | Zona 4 (Recuperación) de la ficha leyendo el estado real de pilas | 1 | D8 | 🟠 | ✅ `PersonatgeController.detall()` + `personatges/detall.html`: contadores reales de Activa/Descanso Corto/Descanso Largo. **Decisión tomada sin confirmación del usuario**: se retiró la caja "Exiliadas" (sin base mecánica en GDD §6, solo definía 3 pilas) — revisar si se quiere. **Falta**: D10/D11 (botón para jugar carta y moverla de pila desde la UI; de momento todo se ve en Activa) |
| D10 | UI DJ en combate (wireframe 6a/6b): iniciativa fija + escena + dock NPCs | 3 | D4,D5,D6 | 🟠 | ✅ `combat/SessioCombat.java` (dominio, ordre torn + `EstatTorn` por participante) + `combat/MesaCombatService.java` (una sesión activa) + `CombatController` (`/combat/iniciar`, `/combat/mesa`) + `templates/combat/{iniciar,mesa}.html`. **Nota importante**: los códigos "wireframe 6a/6b" de esta fila no existen para combate — son wireframes de diseño de carta (Enemigo/Habilidad) ya reutilizados en `cartas/enemigos/detalle.html`. Diseño hecho desde cero, sin wireframe previo, con confirmación del usuario. **Simplificación deliberada**: el dock de NPCs es manual (nombre+Iniciativa+DES escritos por el DJ), no resuelve todavía contra el catálogo real de enemigos (`Aventura.Combate`/JSON) — quedaría como mejora futura |
| D11 | UI jugador móvil en combate (7a/7b): jugar carta → pila de descanso | 2 | D8 | 🟠 | ✅ `CombatController` (`/combat/jugador/{id}`, jugar/descansar) + `templates/combat/jugador.html`: tarjetas de habilidad con botón "Jugar" que mueve la carta de pila en vivo (`EstatPiles.jugar`). Mismo aviso que D10 sobre el wireframe "7a/7b" (es de carta de Raza/Trasfondo, no de combate). **Falta**: hechizos/divinas (`FichaPersonatge` todavía no los resuelve, solo habilidades) |

## WS-E · Aventuras por Actos / Constructor (GDD §20 · plan propio)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| E1 | **Fase 0** · Modelo `CartaAventura` + enums `CardKind`/`FichaEstado` | 1,5 | — | 🔴 | ✅ `model/CartaAventura.java` (+ Cadena/Requisito/Rama/Referencias/FilaBalance) · `CardKind` · `FichaEstado` |
| E2 | Campos aditivos en `Aventura` + métodos (`cartasDeActo`, `porcentajeBase`) | 1 | E1 | 🔴 | ✅ `cartasHistoria`,`tema`,`estado`,`arquitectura` + `esPorActos`/`cartasDeActo`/`porcentajeBase` |
| E3 | Test de compatibilidad: 52 aventuras legacy siguen cargando | 0,5 | E2 | 🔴 | ✅ `test/model/CartaAventuraTest.java` (legacy plano + actos con claves §18b + enums) |
| E4 | Actualizar `Arquitectura_Datos_Onegai §5.15` al mazo por actos | 0,5 | E1 | 🟠 | ✅ §5.15 con nota de actualización + esquema de Carta de Historia |
| E5 | **Fase 1** · `CartaAventuraForm` (DTO Thymeleaf, patrón de los TierForm) | 1 | E2 | 🔴 | ✅ `web/form/CartaAventuraForm.java` |
| E6 | `ConstructorAventuraService` (añadir/editar/mover/código/pickers) | 2 | E5 | 🔴 | ✅ `service/ConstructorAventuraService.java` (form↔modelo, `siguienteCodigo` A1-/B2-/C3-) |
| E7 | Endpoints de constructor en `AventuraController` | 1 | E6 | 🔴 | ✅ GET `/constructor` (+`?edit`) · POST cartas add/editar/mover/borrar |
| E8 | Plantilla `constructor.html` (editor 3 paneles 12a / kanban 12b) | 2,5 | E7 | 🔴 | 🟡 v1 funcional (columna de actos + contadores %Base + tablero + form). Falta drag&drop 12b (JS) |
| E9 | Enriquecer `llista.html` con chips estado/tema + barra de progreso (11a/11b) | 1 | E2 | 🟠 | ✅ chips estado/tema + %Base por acto en lista; botón 🛠 Constructor en el detalle |
| E10 | **Fase 2** · `ValidadorAventuraService` (reglas §20.4 / §18c) | 2 | E2 | 🔴 | ✅ `service/ValidadorAventuraService.java`: filtro sano, efecto mariposa, válvula de escape (% Base + simulación de peor caso todo-Rojas), cadenas (numeración/total/sin requisito), Balance Final (exactamente 1 + filas), referencias (npc/enemigo/loot vía `CatalogoAventuraRepository`, historia vía nuevo `HistoriaService.existe()`). `localizacionId` fuera de alcance (sin catálogo de ubicaciones, ver WS-J). 22 tests en `ValidadorAventuraServiceTest.java` (Mockito, sin contexto Spring) |
| E11 | Plantilla `dependencias.html`: grafo + sandbox (13a) + validación (13b) | 2 | E10 | 🟠 | ✅ `GET /aventuras/{id}/dependencias` + `templates/aventuras/dependencias.html` + `static/css/dependencias.css` + `static/js/dependencias.js`. Grafo = 3 columnas de actos (versión "barata" prevista en el plan; SVG con líneas queda como mejora futura). Sandbox de fichas Verde/Roja **en cliente** (JS replica la semántica de `simularSupervivientes` de E10: Base/Inyectada/Cadena siempre entran, Condicional solo si coincide su requisito), con botones "Todo Verdes/Todo Rojas/Reiniciar" y saltos desde cada incidencia a su carta. **Decisión**: no se comparte todavía motor con `FiltroActoService` (E15, aún no existe) — cuando llegue Fase 3, valorar si el sandbox pasa a llamarlo por AJAX o se queda en cliente. Enlaces añadidos en `detall.html` y `constructor.html` (solo si `esPorActos`). Verificación de compilación pendiente en máquina real (sandbox sin JDK de build) |
| E12 | Export JSON de aventura (endpoint de descarga) | 0,5 | E2 | 🟠 | ⬜ |
| E13 | Export PDF doble cara (extender `imprimir` + `CartaImpresion` cara Director) | 1,5 | E2 | 🟠 | ⬜ |
| E14 | **Fase 3** · Modelo `SesionAventura` transitorio | 1 | E2 | 🟠 | ⬜ |
| E15 | `FiltroActoService` (motor de filtrado + inyección + Balance Final) | 2 | E14 | 🔴 | ⬜ |
| E16 | Tests de `FiltroActoService` (peor/mejor caso de fichas) | 1 | E15 | 🔴 | ⬜ |
| E17 | `PartidaAventuraController` (empezar/robar/marcar/siguiente-acto) | 1,5 | E15 | 🟠 | ⬜ |
| E18 | Plantilla `mesa-director.html` (10a) | 2 | E17 | 🟠 | ⬜ |
| E19 | Plantilla `mesa-compartida.html` (10b) | 2 | E17 | 🟠 | ⬜ |
| E20 | Snapshot de sesión a disco (para "▶ continuar" del dashboard 8a) | 1 | E14 | 🔵 | ⬜ |

## WS-F · Bestiario y estancias como jerarquía Java (Bloques G·H — diferible)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| F1 | Árbol `Enemigo`→`Criatura`/`Elite`/`Jefe` (+`FaseJefe`) según UML | 2 | — | 🔵 | ⬜ |
| F2 | CRUD web `/cartas/enemigos` (patrón Modelo→…→plantilla 5 zonas) | 2,5 | F1 | 🔵 | ⬜ |
| F3 | Poblar bestiario de ejemplo desde una campaña estudiada | 1 | F2 | 🔵 | ⬜ |
| F4 | `Estancia` + `Aventura` como contenedor de estancias (Crawler §20.5) | 2 | E2 | 🔵 | ⬜ |
| F5 | Migrar aventura de ejemplo (100 botín / 20 estancias) al sistema | 1,5 | F4 | 🔵 | ⬜ |

## WS-G · Exportación e impresión (Bloque I)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| G1 | Verificar `PdfService` en máquina real (`./mvnw`) y abrir en lector real | 0,5 | — | 🔴 | 🟡 |
| G2 | Export PDF de cartas de catálogo (mazo completo de un tipo) | 1,5 | G1 | 🟠 | ⬜ |
| G3 | Export de subconjunto elegido (p. ej. solo habilidades preparadas) | 1 | G2 | 🔵 | ⬜ |

## WS-H · Ilustración — zona ② (Bloque J)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| H1 | Sistema de subida/gestión de una imagen por carta (backend + zona ②) | 2,5 | — | 🟠 | ⬜ |
| H2 | Arte genérico por rol/tipo para las cartas base (5 clases, 5 razas) | 2 | H1 | 🟠 | ⬜ |

## WS-I · Interfaz, accesibilidad y pulido visual (Bloque K + revisión de diseño) — EN CURSO 28 jul 2026

*Layout decorator (thymeleaf-layout-dialect) implementado: 58/73 plantillas migradas a
`Blocks/layout-gm.html` (shell común con header+sidebar+footer+scripts). `image-slot.js`
(componente custom element production-ready del prototipo) integrado en el layout y como
avatar en `personatges/detall.html`. Toast "✓ guardado" tras crear/editar personaje.*

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| I1 | Menú hamburguesa / drawer en móvil (la sidebar hoy desaparece) | 1,5 | — | 🔴 | ✅ Drawer ya funciona (`panels.js` + `base.css:218-233`, `aria-expanded` dinámico) |
| I2 | Subir badges de regla de `--fs-micro` a `--fs-small`; micro solo para el ID | 0,5 | — | 🔴 | ⬜ |
| I3 | Quitar el acordeón en fichas de detalle de una sola carta (expandir todo) | 1 | — | 🟠 | ⬜ |
| I4 | Extraer cabeceras de página repetidas a `.gm-page-header` en `components.css` | 1 | — | 🟠 | ⬜ |
| I5 | Auditoría de accesibilidad (contraste, `aria-*`, zonas táctiles) | 1,5 | I1 | 🟠 | 🟡 50 ARIA ya presentes; falta skip-link, foco-trap en modales, alt en imagen del mapa |
| I6 | Revisar contraste tema Noche vs. paleta fija de carta (violeta→dorado) | 0,5 | — | 🔵 | ⬜ |
| I7 | Indicador "✓ guardado" tras crear/editar personaje | 0,5 | — | 🔵 | ✅ Toast verde con `?guardado=1` en `PersonatgeController` + JS en `layout-gm.html` |

> **Layout decorator (no es una tarea del backlog original, pero es la base de todo WS-I):**
> ✅ 58/73 plantillas migradas a `layout:decorate`. Pendientes: `personatges/formulari.html`
> (HUD 322 líneas JS), `historias/formulario.html`, `npcs/formulario.html` (editor de escenas),
> `aventuras/constructor.html` (WS-E), `mapa/index.html` (singularidad, 13 JS modulares),
> `error.html` (sin panels.js), `configuracio.html`/`exportaciones` (legacy Bootstrap).
>
> **`image-slot.js` integrado** (WS-H prácticamente cerrado): componente custom element
> copiado a `static/js/image-slot.js`, cargado en ambos layouts (`layout-gm.html` +
> `layout-gm-print.html`), con avatar `<image-slot>` en `personatges/detall.html`. Degrada
> limpiamente sin `window.omelette` (no crashea, no ensucia consola).

## WS-J · Mapa del mundo (P12 · P13)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| J1 | Separar los 5 modals por entidad en ficheros propios (desde `mapa-dialogo.js`) | 1,5 | — | 🟠 | ⬜ |
| J2 | Línea temporal del mundo sobre el mapa (§7.1) — se alimenta de B4 | 2 | B4 | 🟠 | ⬜ |
| J3 | Zoom por nación (§5.5) | 1,5 | — | 🔵 | ⬜ |

## WS-K · Robustez técnica y tests (Bloque L)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| K1 | Tests de controladores y repositorios (hoy 3 ficheros para 115 clases) | 3 | — | 🔴 | ⬜ |
| K2 | Evaluar/prototipar migración de JSON plano a BD embebida (H2/SQLite) | 3 | — | 🔵 | ⬜ |

## WS-L · Documentación y cierre (Bloque M)

| ID | Tarea | Est. | Dep. | Prio | Estado |
|---|---|---:|---|:--:|:--:|
| L1 | Guía de "primera partida" end-to-end | 2 | D1–D9 | 🟠 | ⬜ |
| L2 | Pase de sincronización docs↔código (cerrar deriva declarada en el DAFO) | 1 | — | 🟠 | ⬜ |

---

## Hitos (milestones)

| Hito | Significa | Depende de |
|---|---|---|
| **M1 · Blindaje + quick wins** (sem. 1) | Backups, errores, móvil usable, tipografía de reglas legible | Z1–Z5, I1, I2 |
| **M2 · GDD §14–18 válido** (sem. 4) | Contenido de ejemplo regenerado a edición 2 | WS-A completo |
| **M3 · Constructor usable** (sem. 7) | Se diseña, valida e imprime una aventura por actos (sin motor de partida) | E1–E13 |
| **M4 · Partida jugable** (sem. 12) | Motor de tiradas + pilas en vivo + mazo por actos en mesa digital | WS-D núcleo, E14–E19 |
| **M5 · Mundo completo** (sem. 16) | Bestiario Java, export PDF, ilustración base, mapa con cronología | WS-F, G, H, J |
| **M6 · v1.0** (sem. 18) | Guía de primera partida + docs sincronizados | L1, L2 |

## Ruta crítica

`Z1 → A2/A3 → A9 → (M2)` corre en paralelo a `E1 → E2 → E6 → E8 (M3)`. El cuello real es
**WS-D** (motor de combate), del que cuelgan D8→D9 (pilas en vivo), D10/D11 (UI de partida)
y E14–E19 comparten con él el patrón de "estado de sesión transitorio": conviene construir D4
(estado de turno) y E14 (`SesionAventura`) con una base común para no duplicar. WS-F/G/H/J son
diferibles sin bloquear la jugabilidad.
