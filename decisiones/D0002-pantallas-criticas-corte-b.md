# D0002 — Integración de pantallas críticas en el plan del Corte B

- **Estado:** `accepted` — revisión e integración encargadas por Oriol en esta sesión; las estimaciones son de Codex y son revisables.
- **Fecha:** 2026-09-03 · **Autor:** Codex.
- **Encargo:** «son muchas cosas criticas, revisa su integracon en nuestro planer de actividades», referido a las dos tablas de pantallas del editor y del jugador.
- **Alcance:** planificación de actividades pendientes. No declara implementadas las pantallas ni autoriza cambiar reglas canónicas.
- **Fuente operativa:** `MotorGraphico-main/MotorGraphico/planificacion/pla/dades.py`; se generan el Markdown, el Gantt y la página. No introduce otra numeración de fases.

## Contexto

La comparación con RPG Maker MZ encontró pantallas sin paquete ejecutable:
el shell y los paneles del editor estaban descritos en ADR-001; título,
creación de personaje integrada, pausa, slots, equipo y menú de combate
dependían de sistemas ya escritos pero no accesibles desde una pantalla.
El plan tampoco concretaba el asistente de exportación ni una prueba completa
de interacción. Los informes históricos mezclaban el alcance amplio EDT/GDB
con el Corte B; no se usan sus porcentajes como medida de finalización.

## Integración

1. **E-10..E-16:** shell visual y foco; proyectos/árbol; propiedades de mapa,
   capas e instancias; recursos e importación; selectores; problemas/cerca;
   edición RPG mínima local. Se reutilizan ProjectHub, ProjectIndex,
   ObjectInspector, MapDocument y los cargadores existentes.
2. **G-08..G-14:** título; asistente de personaje integrado; menú y opciones;
   slots y recuperación de derrota; inventario/equipo/ficha; combate desde
   datos y selección de objetivos; feedback de HUD, temporizadores y comercio.
3. **G-15:** asistente que invoca el exportador de G-06. **G-16:** aceptación
   con autor y jugador nuevos, además de pruebas sobre pantallas reales.
4. **Promoción explícita de alcance:** el mínimo de base de datos pasa de 1.1
   al Corte B: objetos/consumibles, equipo, habilidades y enemigos locales de
   proyecto. Los catálogos importados son de lectura; las copias locales se
   identifican como tales y no se escriben en dndWeebCC ni en el canon. No se
   amplía Nd6. Edición de todas las familias, lotes y fórmulas sigue en 1.1.
5. Permanecen aplazados audio, autotiles, plugins, export nativo macOS/tiendas,
   generador gráfico de personajes, editor de HUD y diario avanzado. No se
   confunde el generador gráfico con el asistente de ficha ya existente.

## Dependencias y cierre

- C y D conservan su alcance; no se añaden funciones durante la consolidación.
- E-09 exige los paneles y diagnósticos además del intérprete. G-04 exige los
  menús reales; G-16 depende del tutorial, formatos, exportación y playtest.
- Cada fase espera todos los terminales de la anterior. El Gantt anterior
  tomaba el último ID topológico como cierre aunque otra rama pudiera durar más.
- G-14 integra el resultado de F1, respetando la reserva de zcode en TABLERO;
  no reclama ni implementa ahora GameSession/Application.
- Un test del modelo no basta para cerrar una pantalla: control visible,
  efecto real, cancelación/error, retorno y evidencia de interacción.

## Esfuerzo y límites de la estimación

Se añaden **16 paquetes y 66 jornadas** (~6 h por jornada). El alcance total
pasa de **54 paquetes / 177 jornadas** a **70 / 243**: C 62, D 30, E 79,
F 13, G 59. Es estimación total de alcance, no trabajo restante ni compromiso
de fecha. No se descuentan cierres parciales o casillas locales del navegador.
Los nuevos paquetes se revisarán al ejecutarse según la regla del doble de
estimación del plan; no se suman otra vez los paquetes equivalentes EDT/GDB.

## Registro y comprobación

La matriz generada incluye 28 filas: 25 pantallas/recorridos del Corte B y
3 grupos de exclusiones. Cada fila incluida apunta a paquetes y aceptación.
El registro de ejecución y las verificaciones de esta revisión viven en
`sesiones/2026-09-03-codex-plan-pantallas.md`.
