# MotorGraphico Creator — Roadmap ejecutable y auditable

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el roadmap ejecutable y auditable v2, foto del 03/09/2026. Lo sustituyo `planificacion/PLA_DEFINITIU.md` + `pla/dades.py`: el plan se genera desde los seguimientos, no se escribe a mano. De las 116 citas de codigo que contiene, 70 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

> **Nota de reconciliación (28 de agosto de 2026).** Esta versión del roadmap incorpora una auditoría del código real.
> Bajo cada ficha se ha insertado un bloque **Estado real** con el avance medido, los archivos que ya existen en el
> repositorio y cómo debe reinterpretarse la ficha. Se han añadido seis paquetes de infraestructura (`INF-01`..`INF-06`)
> que el plan original daba por hechos y no lo están: no hay `tests/` ni `ctest`, Dear ImGui está descartado en el código,
> y el árbol de carpetas real no es el de la arquitectura objetivo.
>
> Las duraciones se leen como **jornadas de esfuerzo** (~6 h), no días naturales.


## 1. Propósito

Este documento convierte `motor_grafico_gantt_rpgmaker.puml` en un contrato de trabajo técnico. Cada elemento del Gantt tiene un identificador estable y una ficha que indica qué parte de la arquitectura modifica, qué archivos afecta, qué decisión debe tomarse, qué debe implementarse, cómo verificarlo y cuándo debe detenerse un agente para pedir contexto.

El objetivo final es **MotorGraphico Creator 1.0**: una aplicación que permita crear, probar y exportar un juego isométrico sin modificar código C++.

## 2. Regla de las cuatro claves

Toda tarea debe pasar, en este orden, por cuatro estados. No se puede saltar directamente a implementar.

### Especificar

Define el problema antes de tocar código:

- entrada y salida;
- comportamiento observable;
- restricciones y compatibilidad;
- casos límite;
- decisiones pendientes;
- criterio de aceptación.

Si cualquiera de estos puntos altera el formato de proyecto, la UX, la compatibilidad o el comportamiento del juego y no está decidido, el agente debe preguntar.

### Diseñar

Define la solución mínima:

- responsabilidad de cada clase;
- dependencias permitidas;
- interfaces públicas;
- propiedad y ciclo de vida de los datos;
- flujo de errores;
- pruebas necesarias.

El diseño debe evitar que el editor dependa de OpenGL para almacenar documentos y que el runtime dependa de la interfaz del editor.

### Dirigir

Ordena la ejecución:

1. crear o modificar los tipos mínimos;
2. integrar sin romper los demos existentes;
3. añadir pruebas antes de ampliar alcance;
4. ejecutar las validaciones locales;
5. entregar un diff pequeño y revisable.

### Revisar

La tarea no termina cuando compila. Debe verificarse:

- comportamiento feliz y casos de error;
- compatibilidad con datos existentes;
- propiedad de memoria y recursos;
- límites de rendimiento relevantes;
- legibilidad del diff;
- documentación pública;
- cumplimiento exacto del criterio de aceptación.

## 3. Puerta obligatoria de aclaración

Antes de implementar, el agente debe detenerse y preguntar cuando falte una decisión que afecte a cualquiera de estos aspectos:

| Área incierta | Pregunta mínima obligatoria |
| --- | --- |
| Producto | ¿Este comportamiento pertenece al editor, al runtime o a ambos? |
| Formato | ¿Debe mantenerse compatibilidad con TMX/TSX o se acepta conversión irreversible? |
| UX | ¿Cuál es el comportamiento esperado al cancelar, sobrescribir o perder foco? |
| Datos | ¿Cuál es la fuente de verdad: JSON propio, TMX o base de datos? |
| Compatibilidad | ¿Qué versiones anteriores deben abrirse y migrarse? |
| Plataforma | ¿Windows, Linux y macOS son obligatorios en esta tarea? |
| Rendimiento | ¿Qué tamaño de mapa, número de entidades y FPS forman el presupuesto? |
| Seguridad | ¿Un plugin o script puede acceder a archivos, red o procesos? |
| Jugabilidad | ¿La regla es fija o configurable por el creador? |
| Autoría | ¿El cambio debe poder realizarse sin escribir código? |

Formato recomendado para preguntar:

```text
DECISIÓN NECESARIA — <ID de tarea>
Contexto: <qué no está definido>
Opciones:
A. <opción y consecuencia>
B. <opción y consecuencia>
Recomendación: <opción recomendada y motivo>
Bloquea: <archivos o comportamiento que no deben implementarse todavía>
```

## 4. Tamaño máximo de una unidad revisable

Las líneas del Gantt son paquetes de trabajo. Dentro de cada ficha, los pasos de `Dirigir` son subtareas atómicas con ID derivado (`DOC-01.1`, `DOC-01.2`, etc.) y constituyen las unidades reales de implementación y revisión. Se aplican estas reglas:

- una unidad persigue un solo resultado observable;
- duración recomendada: entre medio día y cuatro días;
- una subtarea atómica debe cerrarse en un commit revisable o dividirse antes de continuar;
- máximo orientativo: seis archivos de producción modificados;
- debe incluir su prueba o verificación en el mismo cambio;
- no mezclar refactor, nueva funcionalidad y limpieza no relacionada;
- si el diff deja de poder explicarse en cinco minutos, dividirlo;
- cada paquete se entrega mediante varios commits pequeños, pero un único resultado final coherente.

El Gantt conserva los paquetes para mantener legible la planificación temporal; este manual es el desglose WBS que define todas sus subtareas revisables. Si una subtarea numerada todavía excede cuatro días o mezcla resultados, el agente debe crear hijos (`DOC-01.2.a`, `DOC-01.2.b`) y solicitar aprobación del nuevo desglose antes de implementar.

## 5. Definition of Ready

Una tarea está lista para comenzar cuando:

- tiene ID y título;
- sus dependencias están cerradas;
- las decisiones bloqueantes están resueltas;
- se conocen entradas, salidas y errores;
- se han identificado archivos y pruebas;
- existe un criterio de aceptación demostrable.

## 6. Definition of Done

Una tarea está terminada cuando:

- compila en Debug y Release;
- pasan pruebas y demos afectados;
- no introduce warnings nuevos;
- ASan/UBSan no detectan errores en las rutas afectadas;
- el formato y análisis estático pasan;
- el comportamiento está documentado;
- el criterio de aceptación se demuestra;
- el técnico humano ha podido revisar el diff y comprender la decisión.

## 7. Convención de trabajo

| Elemento | Convención |
| --- | --- |
| Rama | `task/<ID>-<slug>` |
| Commit | `<ID>: <resultado verificable>` |
| PR | `[<ID>] <título>` |
| Estado | `BLOCKED`, `READY`, `IN_PROGRESS`, `REVIEW`, `DONE` |
| Evidencia | comandos ejecutados, resultados, captura o fixture cuando corresponda |
| Alcance extra | se registra como otra tarea; no se introduce silenciosamente |

## 8. Arquitectura objetivo

```text
MotorGraphico/
├── include/
│   ├── Core/                 # errores, matemáticas, IDs, utilidades
│   ├── Project/              # Project, settings, documentos, esquemas
│   ├── Map/                  # MapDocument, capas, tilesets, regiones
│   ├── World/                # entidades, componentes, navegación
│   ├── Events/               # páginas, condiciones, comandos, intérprete
│   ├── Database/             # registros configurables de juego
│   ├── Engine/               # Application, escenas, input, guardado
│   └── Render/               # OpenGL, cámara, batches, render proxy
├── src/                      # implementación paralela a include/
├── editor/
│   ├── include/Editor/       # shell, paneles, herramientas, historial
│   └── src/                  # implementación del editor
├── tests/                    # pruebas unitarias e integración por dominio
├── examples/                 # vertical slices y demos manuales
├── assets/                   # recursos de prueba y juego de ejemplo
├── schemas/                  # esquemas JSON y migraciones documentadas
└── CMakeLists.txt
```

## 9. Mapa de áreas arquitectónicas

| Código | Área | Regla de dependencia |
| --- | --- | --- |
| `INF` | Infraestructura y calidad | Puede verificar todos los módulos; no contiene lógica de juego |
| `DOC` | Proyecto y documentos | No depende de OpenGL, GLFW ni Dear ImGui |
| `RUN` | Runtime y mundo | Consume documentos; no depende del editor |
| `REN` | Render | Consume proxies/datos de runtime; no decide reglas de juego |
| `EDT` | Aplicación de editor | Consume documentos y servicios; no es fuente de verdad |
| `MAP` | Autoría de mapas | Define tiles, capas, regiones y conexiones |
| `EVT` | Eventos | Ejecuta comandos tipados sobre un contexto controlado |
| `GDB` | Base de datos y juego | Contenido configurable y sistemas jugables |
| `TOOL` | Playtest, plugins y exportación | Orquesta herramientas sin contaminar el núcleo |
| `REL` | Estabilización y entrega | Pruebas, documentación, instaladores y compatibilidad |

## 10. Resumen de desglose WBS

| Fase | Paquetes del Gantt | Subtareas atómicas revisables |
| --- | ---: | ---: |
| Línea base | 2 | 6 |
| Arquitectura documental | 7 | 31 |
| Runtime jugable | 11 | 45 |
| Editor visual MVP | 15 | 64 |
| Mapas avanzados | 12 | 55 |
| Eventos | 16 | 73 |
| Base de datos y juego | 13 | 65 |
| Playtest, plugins y exportación | 12 | 59 |
| Beta y 1.0 | 10 | 50 |
| **Total** | **98** | **448** |

## 11. Plantilla de ficha

Cada ficha usa este formato:

```text
ID — Título (duración del Gantt)
Arquitectura: área atacada y frontera que debe respetar.
Dependencias: tareas que deben estar DONE.
Archivos: + crear, ~ modificar, = reutilizar sin cambiar contrato.
Especificar: decisiones y comportamiento.
Preguntar si: ambigüedad que bloquea.
Diseñar: tipos, responsabilidades y flujo.
Dirigir: unidades pequeñas de implementación.
Definición/pseudocódigo: contrato técnico orientativo.
Revisar: pruebas y riesgos concretos.
Criterio de aceptación: demostración observable para cerrar.
```

---

# Catálogo técnico de tareas

## Fase 0 — Línea base

### BASE-01 — Inventariar y congelar la base gráfica actual (2 días)

> **Estado real:** `PARCIAL` — avance **70 %** · esfuerzo restante **1 de 2 jornadas**.  
> **En el repo hoy:** ARCHITECTURE.md; AUDITORIA_CONTRATOS.md; motor_grafico_clases.puml; MEMORY_AND_INHERITANCE.md  
> **Cómo leer esta ficha:** Hay inventario disperso en varios .md, pero no una foto congelada y fechada de la línea base. Cerrar como documento único antes de tocar DOC-01.

- **Arquitectura:** INF / REN — fija el contrato de la base existente sin cambiar comportamiento.
- **Dependencias:** Ninguna.
- **Archivos:** + `docs/architecture/current-runtime.md`; ~ `MotorGraphico/README.md`; = `include/Render/*`, `src/Render/*`, `include/Core/*`.
- **Especificar:** Enumerar componentes implementados, API pública, demos, limitaciones y SHA de referencia.
- **Preguntar si:** hay cambios locales o ramas no fusionadas que deban considerarse fuente de verdad.
- **Diseñar:** Un inventario por módulo con estados `estable`, `experimental` y `stub`.
- **Dirigir:**
  1. **BASE-01.1** — **[1 d]** registrar árbol y targets CMake.
  2. **BASE-01.2** — **[0,5 d]** documentar contratos de Camera, TileMap, Entity y renderer.
  3. **BASE-01.3** — **[0,5 d]** marcar stubs y deuda sin corregirla.
- **Definición/pseudocódigo:**

```text
snapshot = inspect(public_headers, cmake_targets, demos)
freeze(snapshot, commit_sha)
assert no_source_behavior_changed
```

- **Revisar:** Comparar inventario con código y README; el diff no debe modificar `.cpp/.h` de producción.
- **Criterio de aceptación:** Un técnico puede señalar qué existe realmente y qué es solo UML usando un único documento.

### BASE-02 — Verificar CI, demos y compatibilidad multiplataforma (3 días)

> **Estado real:** `PARCIAL` — avance **75 %** · esfuerzo restante **1 de 3 jornadas**.  
> **En el repo hoy:** .github/workflows/ci.yml; .github/workflows/release.yml; .clang-format; .clang-tidy; .pre-commit-config.yaml  
> **Cómo leer esta ficha:** CI real y potente (Debug/Release x g++/clang, ASan/UBSan, clang-format, clang-tidy, cppcheck, xvfb) pero SOLO ubuntu-latest y SIN ctest: la verificación son demos que abortan. La ficha asume tests/ y multiplataforma; ver INF-01 e INF-05.

- **Arquitectura:** INF — establece la línea base de calidad para todo el roadmap.
- **Dependencias:** BASE-01.
- **Archivos:** ~ `.github/workflows/ci.yml`; ~ `MotorGraphico/CMakeLists.txt`; + `docs/testing/baseline.md`; = `examples/*`.
- **Especificar:** Definir matriz mínima Debug/Release × GCC/Clang, pruebas headless y plataformas obligatorias.
- **Preguntar si:** macOS debe ser obligatorio desde ahora o se incorpora después de la alpha.
- **Diseñar:** Un conjunto de comandos reproducible localmente y su equivalente en CI.
- **Dirigir:**
  1. **BASE-02.1** — **[1 d]** ejecutar build y demos actuales.
  2. **BASE-02.2** — **[1 d]** registrar fallos reales sin ocultarlos.
  3. **BASE-02.3** — **[1 d]** añadir checks ausentes solo si no cambian producto.
- **Definición/pseudocódigo:**

```text
for config in build_matrix:
    configure(config)
    build(config)
    run(smoke_tests + sanitizers + static_analysis)
```

- **Revisar:** Confirmar que los checks fallan cuando se introduce un error controlado y que no dependen de artefactos locales.
- **Criterio de aceptación:** La rama de referencia tiene una línea base reproducible o una lista explícita de fallos bloqueantes.

## Fase 1 — Arquitectura documental

### DOC-01 — Definir Project y ProjectSettings (5 días)

> **Estado real:** `PARCIAL` — avance **60 %** · esfuerzo restante **2 de 5 jornadas**.  
> **En el repo hoy:** include/Editor/ProjectIndex.h (struct Project); include/Editor/ProjectHub.h; src/Editor/ProjectIndex.cpp; assets/proyectos/*.json; FORMATO_PROYECTOS.md; examples/demo_proyectos.cpp  
> **Cómo leer esta ficha:** Ya existe un Project con índice y hub, pero vive bajo Editor/ y no tiene ProjectSettings ni versión de esquema. El trabajo real es MOVER y completar, no crear de cero.

- **Arquitectura:** DOC — raíz del modelo de autoría; debe ser independiente de render y editor.
- **Dependencias:** BASE-02.
- **Archivos:** + `include/Project/Project.h`; + `include/Project/ProjectSettings.h`; + `src/Project/Project.cpp`; + `tests/Project/ProjectTests.cpp`; ~ `CMakeLists.txt`.
- **Especificar:** Definir identidad, nombre, versión de esquema, rutas lógicas, mapas iniciales y ajustes globales.
- **Preguntar si:** el proyecto debe ser portable como carpeta relativa o admitir referencias absolutas externas.
- **Diseñar:** `Project` agrega documentos por ID; `ProjectSettings` contiene valores globales y no servicios.
- **Dirigir:**
  1. **DOC-01.1** — **[1,5 d]** crear tipos sin I/O.
  2. **DOC-01.2** — **[1,5 d]** añadir invariantes y validación.
  3. **DOC-01.3** — **[1 d]** integrar target `motor_project`.
  4. **DOC-01.4** — **[1 d]** probar construcción válida e inválida.
- **Definición/pseudocódigo:**

```text
struct Project { ProjectId id; SchemaVersion schema; ProjectSettings settings; Map<ProjectId, DocumentRef> docs; };
Result<void> validate(const Project&);
```

- **Revisar:** No incluir `Texture`, `Shader`, GLFW o widgets; comprobar valores por defecto deterministas.
- **Criterio de aceptación:** Se puede crear y validar un proyecto vacío en una prueba sin inicializar OpenGL.

### DOC-02 — Definir MapDocument y capas tipadas (7 días)

> **Estado real:** `PARCIAL` — avance **45 %** · esfuerzo restante **4 de 7 jornadas**.  
> **En el repo hoy:** include/Level/LevelDefinition.h; include/Editor/EditorState.h (capas de tiles); FORMATO_NIVELES.md  
> **Cómo leer esta ficha:** LevelDefinition + las capas de EditorState hacen de documento de mapa, pero solo hay capas de TILES: entidades y regiones no son capas. Falta el tipo MapDocument como fuente de verdad fuera del editor.

- **Arquitectura:** DOC / MAP — fuente de verdad editable de un mapa.
- **Dependencias:** DOC-01.
- **Archivos:** + `include/Map/MapDocument.h`; + `include/Map/MapLayer.h`; + `include/Map/TileLayerDocument.h`; + `include/Map/EntityLayerDocument.h`; + `include/Map/RegionLayerDocument.h`; + `tests/Map/MapDocumentTests.cpp`.
- **Especificar:** Definir tamaño, tile size, metadatos y capas con ID, nombre, tipo, visibilidad, bloqueo, opacidad y orden.
- **Preguntar si:** una capa puede cambiar de tipo o debe crearse otra y migrar contenido.
- **Diseñar:** Jerarquía tipada o variante cerrada; los datos no contienen punteros a GPU.
- **Dirigir:**
  1. **DOC-02.1** — **[2 d]** crear cabecera base de capa.
  2. **DOC-02.2** — **[2 d]** crear tres tipos de capa.
  3. **DOC-02.3** — **[1,5 d]** añadir orden e invariantes.
  4. **DOC-02.4** — **[1,5 d]** probar dimensiones y capas duplicadas.
- **Definición/pseudocódigo:**

```text
using LayerData = variant<TileLayerData, EntityLayerData, RegionLayerData>;
struct MapLayer { LayerId id; LayerMeta meta; LayerData data; };
```

- **Revisar:** Evitar triple vector expuesto; verificar IDs únicos, dimensiones coherentes y orden estable.
- **Criterio de aceptación:** Una prueba construye un mapa con suelo, entidades y regiones sin usar `Render::TileMap`.

### DOC-03 — Separar MapDocument de RuntimeTileMap (7 días)

> **Estado real:** `REPLANTEAR` — avance **40 %** · esfuerzo restante **5 de 7 jornadas**.  
> **En el repo hoy:** include/Render/TileMap.h; src/Render/TileMap.cpp; include/Editor/EditorState.h::exportTmx()  
> **Cómo leer esta ficha:** La frontera documento/runtime ya existe de facto (EditorState es GL-free), pero el puente es un STRING TMX: se exporta texto y TileMap lo reparsea. Sustituir ese round-trip por texto por un builder en memoria.

- **Arquitectura:** DOC → RUN → REN — crea la frontera documento/runtime.
- **Dependencias:** DOC-02.
- **Archivos:** + `include/Runtime/RuntimeTileMap.h`; + `include/Runtime/RuntimeTileMapBuilder.h`; + `src/Runtime/RuntimeTileMapBuilder.cpp`; ~ `include/Render/TileMap.h`; ~ `src/Render/TileMap.cpp`; + `tests/Runtime/RuntimeTileMapBuilderTests.cpp`.
- **Especificar:** Definir qué datos se copian, compactan o indexan al entrar en runtime.
- **Preguntar si:** el runtime puede modificar tiles de forma persistente o solo mantener overrides de partida.
- **Diseñar:** Builder puro que transforma documento validado en estructuras optimizadas.
- **Dirigir:**
  1. **DOC-03.1** — **[2 d]** renombrar conceptualmente la representación runtime.
  2. **DOC-03.2** — **[2 d]** crear builder sin cambiar render.
  3. **DOC-03.3** — **[1,5 d]** adaptar renderer al nuevo tipo.
  4. **DOC-03.4** — **[1,5 d]** mantener demo TMX mediante adaptador temporal.
- **Definición/pseudocódigo:**

```text
Result<RuntimeTileMap> build(const MapDocument& doc) { validate(doc); return compile_layers(doc.layers); }
```

- **Revisar:** La dirección de dependencia debe ser runtime consume documento; documento nunca incluye runtime.
- **Criterio de aceptación:** El mismo mapa documental se compila y renderiza con la imagen esperada.

### DOC-04 — Implementar IDs estables y referencias (4 días)

> **Estado real:** `PARCIAL` — avance **50 %** · esfuerzo restante **2 de 4 jornadas**.  
> **En el repo hoy:** CONTRATO_IDS.md; include/Core/Memory/HandlePool.h; include/Core/Resources/ICatalog.h  
> **Cómo leer esta ficha:** Hay contrato de IDs escrito y handles, pero los ids son std::string sueltos sin tipo fuerte, sin tabla de renombrado ni detección de referencias rotas centralizada.

- **Arquitectura:** Core / DOC — identidad persistente entre renombres y movimientos.
- **Dependencias:** DOC-03.
- **Archivos:** + `include/Core/Ids/StrongId.h`; + `include/Core/Ids/ProjectIds.h`; + `include/Project/DocumentRef.h`; + `tests/Core/StrongIdTests.cpp`; ~ tipos documentales.
- **Especificar:** Definir generación, serialización, comparación y representación de IDs.
- **Preguntar si:** se prefieren UUID aleatorios, UUID v7 o IDs legibles con namespace.
- **Diseñar:** Tipos fuertes distintos para proyecto, mapa, capa, asset, entidad y evento.
- **Dirigir:**
  1. **DOC-04.1** — **[1 d]** crear wrapper fuerte.
  2. **DOC-04.2** — **[1 d]** añadir generador inyectable.
  3. **DOC-04.3** — **[1 d]** sustituir strings ambiguos.
  4. **DOC-04.4** — **[1 d]** probar igualdad y round-trip.
- **Definición/pseudocódigo:**

```text
template<class Tag> class StrongId { string value; };
using MapId = StrongId<MapTag>;
using AssetId = StrongId<AssetTag>;
```

- **Revisar:** Impedir asignar accidentalmente `AssetId` a `MapId`; no regenerar IDs al renombrar.
- **Criterio de aceptación:** Mover o renombrar un documento no rompe ninguna referencia de una fixture.

### DOC-05 — Serialización JSON y versionado de esquema (6 días)

> **Estado real:** `PARCIAL` — avance **55 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** include/Core/Json/JsonValue.h; src/Core/Json/JsonValue.cpp; src/Level/LevelLoader.cpp; FORMATO_NIVELES.md; FORMATO_AVENTURAS.md; FORMATO_PROYECTOS.md  
> **Cómo leer esta ficha:** JSON propio y tres formatos documentados y en uso. Lo que falta es exactamente lo caro: campo schemaVersion, migraciones y política de compatibilidad.

- **Arquitectura:** DOC — persistencia canónica y migrable.
- **Dependencias:** DOC-04.
- **Archivos:** + `include/Project/ProjectSerializer.h`; + `src/Project/ProjectSerializer.cpp`; + `schemas/project-v1.schema.json`; + `schemas/map-v1.schema.json`; + `tests/Project/SerializationTests.cpp`; ~ `CMakeLists.txt`.
- **Especificar:** Definir formato canónico, orden estable, errores, escritura atómica y versión.
- **Preguntar si:** la librería JSON elegida y si los archivos deben ser legibles/editables por humanos.
- **Diseñar:** DTO JSON separado de clases runtime; migraciones encadenadas por versión.
- **Dirigir:**
  1. **DOC-05.1** — **[1,5 d]** elegir/adaptar biblioteca.
  2. **DOC-05.2** — **[1,5 d]** serializar proyecto y mapa.
  3. **DOC-05.3** — **[1 d]** cargar con errores localizados.
  4. **DOC-05.4** — **[1 d]** guardar vía temporal+rename.
  5. **DOC-05.5** — **[1 d]** añadir schema y fixtures.
- **Definición/pseudocódigo:**

```text
load(path): raw=parse_json(); migrated=migrate(raw.version,current); return decode(migrated)
save(doc): validate(doc); atomic_write(encode(doc))
```

- **Revisar:** Round-trip semántico, orden estable para Git, archivo original intacto si falla el guardado.
- **Criterio de aceptación:** Guardar dos veces sin cambios produce bytes idénticos y un JSON corrupto devuelve error descriptivo.

### DOC-06 — Convertir el parser TMX en importador (5 días)

> **Estado real:** `PARCIAL` — avance **40 %** · esfuerzo restante **3 de 5 jornadas**.  
> **En el repo hoy:** third_party/tinyxml2; src/Render/TileMap.cpp (loadFromFile TMX); include/Editor/EditorState.h::exportTmx()  
> **Cómo leer esta ficha:** Hoy TMX es la fuente de verdad del grid, no un formato de importación. Invertir la dirección es la tarea; el parser ya está escrito.

- **Arquitectura:** TOOL / DOC — TMX deja de ser fuente de verdad del runtime.
- **Dependencias:** DOC-05.
- **Archivos:** + `include/Import/TmxMapImporter.h`; + `src/Import/TmxMapImporter.cpp`; ~ `src/Render/TileMap.cpp`; + `tests/Import/TmxImporterTests.cpp`; = `third_party/tinyxml2/*`.
- **Especificar:** Definir subconjunto soportado, advertencias, mapeo de GID y política para datos desconocidos.
- **Preguntar si:** si una función TMX no soportada debe bloquear importación o conservarse como metadata opaca.
- **Diseñar:** Importador devuelve `ImportReport<MapDocument>` con errores y avisos.
- **Dirigir:**
  1. **DOC-06.1** — **[1 d]** extraer parsing de `TileMap`.
  2. **DOC-06.2** — **[1 d]** mapear capas y colisión.
  3. **DOC-06.3** — **[1 d]** crear informe de importación.
  4. **DOC-06.4** — **[1 d]** mantener fixture existente.
  5. **DOC-06.5** — **[1 d]** eliminar I/O TMX del renderer.
- **Definición/pseudocódigo:**

```text
report = TmxMapImporter.import(path)
if report.hasErrors(): stop
project.add(report.document)
```

- **Revisar:** No perder silenciosamente propiedades; asegurar que el renderer ya no parsea XML.
- **Criterio de aceptación:** `test_map.tmx` genera un `MapDocument` válido y las limitaciones aparecen en el informe.

### DOC-07 — Pruebas de ida y vuelta TMX-JSON-Runtime (4 días)

> **Estado real:** `PARCIAL` — avance **60 %** · esfuerzo restante **2 de 4 jornadas**.  
> **En el repo hoy:** examples/demo_editor_state.cpp  
> **Cómo leer esta ficha:** El demo ya verifica round-trip TMX->TileMap y JSON->LevelLoader. Falta añadir el eslabón MapDocument cuando exista y convertirlo en prueba de ctest.

- **Arquitectura:** INF / DOC / RUN / REN — prueba la frontera completa.
- **Dependencias:** DOC-06.
- **Archivos:** + `tests/Integration/MapRoundTripTests.cpp`; + `tests/fixtures/maps/expected-map-v1.json`; ~ `examples/demo_tilemap.cpp`; ~ CI.
- **Especificar:** Definir equivalencia semántica y visual entre origen importado y runtime.
- **Preguntar si:** la comparación visual debe ser exacta por pixel o admitir tolerancia por plataforma.
- **Diseñar:** Fixture única que recorre importación, persistencia, carga, compilación y render headless.
- **Dirigir:**
  1. **DOC-07.1** — **[1 d]** crear expected JSON revisable.
  2. **DOC-07.2** — **[1 d]** comparar documentos.
  3. **DOC-07.3** — **[1 d]** construir runtime.
  4. **DOC-07.4** — **[0,5 d]** comparar colisión y render.
  5. **DOC-07.5** — **[0,5 d]** ejecutar en CI.
- **Definición/pseudocódigo:**

```text
doc1=importTMX(tmx); save(doc1,json); doc2=load(json); rt=build(doc2); assert equivalent(doc1,doc2,rt)
```

- **Revisar:** Detectar pérdida de IDs, capas, GID, colisión y dimensiones; mensaje claro por diferencia.
- **Criterio de aceptación:** La pipeline completa pasa en CI y falla al alterar intencionadamente un tile de la fixture.

## Fase 2 — Runtime jugable mínimo

### RUN-01 — Implementar Application y bucle principal (5 días)

> **Estado real:** `PARCIAL` — avance **80 %** · esfuerzo restante **1 de 5 jornadas**.  
> **En el repo hoy:** include/Engine/Application.h; src/Engine/Application.cpp; include/Engine/Window.h  
> **Cómo leer esta ficha:** Bucle completo (run/processInput/update/render/shutdown, maxFrames para CI). Problema: Application conoce tienda, negocio y batalla — está acoplada a ESTE juego. La tarea real es adelgazarla.

- **Arquitectura:** RUN / Engine — composición raíz del juego.
- **Dependencias:** DOC-07.
- **Archivos:** + `include/Engine/Application.h`; + `src/Engine/Application.cpp`; + `tests/Engine/ApplicationTests.cpp`; ~ `CMakeLists.txt`; = `Engine/Window`.
- **Especificar:** Estados init/run/shutdown, timestep, errores y orden de subsistemas.
- **Preguntar si:** timestep fijo, variable o híbrido y FPS objetivo.
- **Diseñar:** `Application` posee servicios; el bucle delega input, fixedUpdate, update y render.
- **Dirigir:**
  1. **RUN-01.1** — **[1,5 d]** definir ciclo de vida.
  2. **RUN-01.2** — **[1,5 d]** implementar reloj y salida.
  3. **RUN-01.3** — **[1 d]** inyectar servicios mínimos.
  4. **RUN-01.4** — **[1 d]** crear smoke test sin lógica de juego.
- **Definición/pseudocódigo:**

```text
while(window.open){ input.poll(); accumulator+=dt; while(accumulator>=fixed) world.fixedUpdate(fixed); world.update(dt); renderer.render(); }
```

- **Revisar:** Shutdown idempotente, dt limitado tras pausas y recursos destruidos con contexto GL válido.
- **Criterio de aceptación:** Un ejecutable entra, procesa varios frames y sale limpiamente en CI.

### RUN-02 — Implementar InputSystem (4 días)

> **Estado real:** `PARCIAL` — avance **60 %** · esfuerzo restante **2 de 4 jornadas**.  
> **En el repo hoy:** include/Engine/InputState.h; src/Engine/Application.cpp::processInput()  
> **Cómo leer esta ficha:** Hay estado de entrada, pero el mapeo tecla->acción está incrustado en Application y en level_editor.cpp. Falta la capa de acciones configurable.

- **Arquitectura:** RUN — traduce dispositivos a acciones, sin reglas de juego.
- **Dependencias:** RUN-01.
- **Archivos:** + `include/Engine/Input/InputSystem.h`; + `include/Engine/Input/InputMap.h`; + `src/Engine/Input/InputSystem.cpp`; + `tests/Engine/InputSystemTests.cpp`; ~ `Window` callbacks.
- **Especificar:** Acciones, ejes, pressed/held/released, remapeo y foco.
- **Preguntar si:** mando debe entrar en el MVP o solo teclado/ratón.
- **Diseñar:** Snapshot inmutable por frame y bindings configurables.
- **Dirigir:**
  1. **RUN-02.1** — **[1 d]** capturar eventos GLFW.
  2. **RUN-02.2** — **[1 d]** construir snapshot.
  3. **RUN-02.3** — **[1 d]** mapear acciones.
  4. **RUN-02.4** — **[1 d]** probar transiciones de estado.
- **Definición/pseudocódigo:**

```text
poll(); current=readDevices(); actions=bindings.resolve(previous,current); previous=current;
```

- **Revisar:** Foco perdido libera teclas; tests no dependen de hardware real.
- **Criterio de aceptación:** `MoveNorth` y `Interact` pueden probarse con eventos simulados.

### RUN-03 — Implementar SceneManager y carga de mapas (6 días)

> **Estado real:** `PARCIAL` — avance **45 %** · esfuerzo restante **4 de 6 jornadas**.  
> **En el repo hoy:** src/Engine/Application.cpp::applyPendingTransition(); include/Level/LevelLoader.h; include/Level/WorldTopology.h  
> **Cómo leer esta ficha:** El cambio de escena funciona, pero no hay un SceneManager: la lógica está repartida entre Application y GameSession.

- **Arquitectura:** RUN / DOC — controla escenas y transiciones.
- **Dependencias:** RUN-02.
- **Archivos:** + `include/Engine/Scene/Scene.h`; + `include/Engine/Scene/SceneManager.h`; + `src/Engine/Scene/SceneManager.cpp`; + `tests/Engine/SceneManagerTests.cpp`.
- **Especificar:** Escena activa, carga, activación, descarga, error y transición diferida.
- **Preguntar si:** se permiten escenas apiladas para menú/pausa o una sola escena activa.
- **Diseñar:** Cambios de escena se encolan y aplican fuera de `update`.
- **Dirigir:**
  1. **RUN-03.1** — **[1,5 d]** definir interfaz Scene.
  2. **RUN-03.2** — **[1,5 d]** crear state machine.
  3. **RUN-03.3** — **[1,5 d]** integrar MapId.
  4. **RUN-03.4** — **[1,5 d]** probar fallo de carga y transición.
- **Definición/pseudocódigo:**

```text
requestChange(id); endOfFrame(){ next=loader.load(id); if(ok){ current.exit(); current=next; current.enter(); }}
```

- **Revisar:** No destruir escena mientras ejecuta callback; error conserva escena anterior.
- **Criterio de aceptación:** Dos mapas fixture alternan sin referencias colgantes.

### RUN-04 — Implementar World y ciclo de vida de entidades (6 días)

> **Estado real:** `PARCIAL` — avance **35 %** · esfuerzo restante **4 de 6 jornadas**.  
> **En el repo hoy:** include/Game/GameSession.h (WorldEnemy, spawns); include/Render/Entity.h  
> **Cómo leer esta ficha:** GameSession gestiona el ciclo de vida de enemigos y objetos del nivel, pero es específico del juego, no un World genérico con EntityId.

- **Arquitectura:** RUN / World — propietario único de entidades runtime.
- **Dependencias:** RUN-03.
- **Archivos:** + `include/World/World.h`; + `include/World/EntityHandle.h`; + `src/World/World.cpp`; + `tests/World/WorldTests.cpp`.
- **Especificar:** Alta, baja diferida, búsqueda, update y validez de handles.
- **Preguntar si:** se adopta ECS completo o composición ligera con componentes polimórficos.
- **Diseñar:** Handles generacionales; destrucciones después de iterar.
- **Dirigir:**
  1. **RUN-04.1** — **[1,5 d]** definir handle.
  2. **RUN-04.2** — **[1,5 d]** implementar almacenamiento.
  3. **RUN-04.3** — **[1,5 d]** encolar create/destroy.
  4. **RUN-04.4** — **[1,5 d]** probar reuse y handle obsoleto.
- **Definición/pseudocódigo:**

```text
handle={index,generation}; destroy(h)->pending; flush(){ generation[index]++; free(index); }
```

- **Revisar:** Ningún puntero crudo sobrevive a destrucción; orden de update determinista.
- **Criterio de aceptación:** Un handle antiguo no puede acceder a una nueva entidad que reutilice índice.

### RUN-05 — Migrar Entity a composición por componentes (7 días)

> **Estado real:** `PENDIENTE` — avance **10 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** include/Render/Entity.h; include/Render/Player.h; include/Render/Enemy.h; include/Render/AnimatedEntity.h; MEMORY_AND_INHERITANCE.md  
> **Cómo leer esta ficha:** Hoy es jerarquía por herencia y está documentada como decisión consciente. Migrar a componentes es un cambio de arquitectura: NO empezarlo sin decidirlo explícitamente (ver la puerta de aclaración).

- **Arquitectura:** RUN / REN — separa estado de mundo y representación.
- **Dependencias:** RUN-04.
- **Archivos:** + `include/World/Entity.h`; + `include/World/Components/TransformComponent.h`; + `include/World/Components/SpriteComponent.h`; + `include/World/Components/CollisionComponent.h`; + `include/Render/EntityRenderProxy.h`; ~ `Render/Entity.*`; + tests.
- **Especificar:** Componentes mínimos, propiedad, consulta y puente de render.
- **Preguntar si:** compatibilidad binaria con `Render::Entity` es necesaria o basta compatibilidad funcional de demos.
- **Diseñar:** World almacena datos; proxy produce `RenderItem` sin poseer gameplay.
- **Dirigir:**
  1. **RUN-05.1** — **[2 d]** crear Transform/Sprite.
  2. **RUN-05.2** — **[2 d]** crear registro de componentes.
  3. **RUN-05.3** — **[1,5 d]** adaptar demo con proxy.
  4. **RUN-05.4** — **[1,5 d]** retirar acoplamientos gradualmente.
- **Definición/pseudocódigo:**

```text
entity.add(Transform{grid}); entity.add(Sprite{assetId}); renderProxy.collect(world, queue);
```

- **Revisar:** World no incluye `glad`, `SpriteBatch` ni `TextureAtlas`; demo conserva orden visual.
- **Criterio de aceptación:** Una entidad sin sprite existe y actualiza; añadir sprite la hace renderizable.

### RUN-06 — Implementar animación por frames (5 días)

> **Estado real:** `DONE` — avance **90 %** · esfuerzo restante **1 de 5 jornadas**.  
> **En el repo hoy:** include/Render/AnimatedEntity.h; src/Render/AnimatedEntity.cpp; examples/demo_animated_entity.cpp  
> **Cómo leer esta ficha:** Animación por frames resuelta. Queda exponerla como dato editable desde el editor, no como código.

- **Arquitectura:** RUN / REN — estado de animación independiente del atlas.
- **Dependencias:** RUN-05.
- **Archivos:** + `include/World/Components/AnimationComponent.h`; + `include/World/Systems/AnimationSystem.h`; + `src/World/Systems/AnimationSystem.cpp`; + tests; ~ render proxy.
- **Especificar:** Clips, frame duration, loop, once, cambio y evento de fin.
- **Preguntar si:** tiempo en segundos o ticks fijos y si se requieren animaciones direccionales desde el inicio.
- **Diseñar:** Clips referencian IDs de frame; sistema actualiza frame actual.
- **Dirigir:**
  1. **RUN-06.1** — **[1,5 d]** definir clip.
  2. **RUN-06.2** — **[1,5 d]** implementar avance.
  3. **RUN-06.3** — **[1 d]** integrar sprite frame.
  4. **RUN-06.4** — **[1 d]** probar dt grande y no-loop.
- **Definición/pseudocódigo:**

```text
elapsed+=dt; while(elapsed>=frameTime){ advance(); elapsed-=frameTime; if(end) emit(AnimationFinished); }
```

- **Revisar:** Sin saltos inconsistentes; no reiniciar al pedir el clip ya activo salvo opción.
- **Criterio de aceptación:** Fixture reproduce walk en bucle y attack una vez con secuencia exacta.

### RUN-07 — Implementar movimiento y colisión dinámica (7 días)

> **Estado real:** `PARCIAL` — avance **65 %** · esfuerzo restante **3 de 7 jornadas**.  
> **En el repo hoy:** src/Render/TileMap.cpp (colisión por GID); include/Game/GameSession.h; include/Render/OcclusionRules.h  
> **Cómo leer esta ficha:** Colisión contra el mapa sí; colisión dinámica entre entidades y empuje, no.

- **Arquitectura:** RUN / MAP — reglas de ocupación y desplazamiento.
- **Dependencias:** RUN-06.
- **Archivos:** + `include/World/Systems/MovementSystem.h`; + `include/World/Systems/CollisionSystem.h`; + `src/World/Systems/MovementSystem.cpp`; + `src/World/Systems/CollisionSystem.cpp`; + tests; ~ TileDefinition runtime.
- **Especificar:** Movimiento por grid, reserva de destino, colisión tile/entidad y respuesta.
- **Preguntar si:** movimiento estrictamente por celdas o interpolación libre sobre grid isométrico.
- **Diseñar:** Posición lógica discreta y offset visual interpolado para MVP.
- **Dirigir:**
  1. **RUN-07.1** — **[1,5 d]** consultar passability.
  2. **RUN-07.2** — **[1,5 d]** resolver intento.
  3. **RUN-07.3** — **[1,5 d]** reservar celda.
  4. **RUN-07.4** — **[1,5 d]** interpolar visual.
  5. **RUN-07.5** — **[1 d]** emitir llegada/bloqueo.
- **Definición/pseudocódigo:**

```text
if(nav.canEnter(from,to) && occupancy.reserve(to,id)){ transform.logical=to; motion.interpolate(from,to); }
```

- **Revisar:** Choques simultáneos deterministas; no atravesar diagonalmente esquinas bloqueadas.
- **Criterio de aceptación:** Jugador no cruza dos tiles bloqueados y llega a un tile libre sin desincronizar visual/lógico.

### RUN-08 — Implementar cámara de seguimiento (3 días)

> **Estado real:** `DONE` — avance **90 %** · esfuerzo restante **1 de 3 jornadas**.  
> **En el repo hoy:** include/Render/Camera.h; src/Render/Camera.cpp; examples/demo_camera.cpp  
> **Cómo leer esta ficha:** Cámara isométrica con seguimiento hecha. Falta solo límites de mapa configurables por documento.

- **Arquitectura:** RUN → REN — política de cámara separada de la matriz.
- **Dependencias:** RUN-07.
- **Archivos:** + `include/World/Systems/CameraFollowSystem.h`; + `src/World/Systems/CameraFollowSystem.cpp`; ~ `Render/Camera.*`; + tests.
- **Especificar:** Objetivo, dead zone, suavizado, límites y teletransporte.
- **Preguntar si:** la cámara debe rotar en isométrico o solo desplazar/zoom.
- **Diseñar:** Sistema calcula target; `Camera` conserva interpolación y proyección.
- **Dirigir:**
  1. **RUN-08.1** — **[1 d]** añadir target por handle.
  2. **RUN-08.2** — **[1 d]** calcular centro y límites.
  3. **RUN-08.3** — **[0,5 d]** resolver snap en transición.
  4. **RUN-08.4** — **[0,5 d]** probar mapa menor que viewport.
- **Definición/pseudocódigo:**

```text
target=worldToScreen(entity.grid)+offset; camera.setTarget(clamp(target,mapBounds));
```

- **Revisar:** Sin vibración por redondeo; entidad destruida libera target.
- **Criterio de aceptación:** La cámara sigue al jugador, respeta límites y hace snap al cambiar mapa.

### RUN-09 — Implementar portales y cambio de mapa (4 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **1 de 4 jornadas**.  
> **En el repo hoy:** include/Level/WorldTopology.h; include/Game/GameSession.h (LevelTransition); EditorState::setObjectTransition(); examples/demo_mundo.cpp; examples/demo_ciudad.cpp  
> **Cómo leer esta ficha:** Portales y grafo de mundo funcionando y editables. Falta el punto de aparición por destino bien definido.

- **Arquitectura:** RUN / MAP / DOC — navegación entre documentos.
- **Dependencias:** RUN-08.
- **Archivos:** + `include/Map/MapLink.h`; + `include/World/Components/PortalComponent.h`; + `include/World/Systems/PortalSystem.h`; + `src/World/Systems/PortalSystem.cpp`; + tests; ~ SceneManager.
- **Especificar:** Destino por MapId+SpawnId, trigger, orientación y error.
- **Preguntar si:** portales automáticos al entrar o interacción configurable por portal.
- **Diseñar:** Portal emite `SceneTransitionRequest`; no carga directamente.
- **Dirigir:**
  1. **RUN-09.1** — **[1 d]** definir enlace documental.
  2. **RUN-09.2** — **[1 d]** crear trigger runtime.
  3. **RUN-09.3** — **[1 d]** resolver spawn.
  4. **RUN-09.4** — **[1 d]** manejar destino ausente.
- **Definición/pseudocódigo:**

```text
onEnter(entity,portal){ bus.emit(ChangeMap{portal.mapId,portal.spawnId}); }
```

- **Revisar:** Validar enlaces al guardar; fallo no deja jugador sin escena.
- **Criterio de aceptación:** Jugador cruza A→B y aparece exactamente en spawn configurado.

### RUN-10 — Implementar estado y guardado básico (6 días)

> **Estado real:** `PENDIENTE` — avance **15 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** include/Game/GameSession.h  
> **Cómo leer esta ficha:** GameSession tiene TODO el estado de partida en memoria pero no hay una sola función de guardado/carga a disco. Es el hueco más grande de la fase 2.

- **Arquitectura:** RUN / DOC — estado de partida separado de proyecto.
- **Dependencias:** RUN-09.
- **Archivos:** + `include/Engine/Save/GameState.h`; + `include/Engine/Save/SaveService.h`; + `src/Engine/Save/SaveService.cpp`; + `schemas/save-v1.schema.json`; + tests.
- **Especificar:** Qué persiste: mapa, posición, variables, entidades modificadas y versión.
- **Preguntar si:** número de slots, autosave y compatibilidad de partidas tras editar proyecto.
- **Diseñar:** Save guarda deltas por ID estable y referencia versión de proyecto.
- **Dirigir:**
  1. **RUN-10.1** — **[1,5 d]** definir DTO.
  2. **RUN-10.2** — **[1,5 d]** capturar/restaurar estado.
  3. **RUN-10.3** — **[1,5 d]** escritura atómica.
  4. **RUN-10.4** — **[1,5 d]** validar mismatch de versión.
- **Definición/pseudocódigo:**

```text
snapshot=GameState::capture(world,events); atomic_write(slot,encode(snapshot)); load->validateProject->restore;
```

- **Revisar:** Nunca sobrescribir slot válido con guardado fallido; errores accionables.
- **Criterio de aceptación:** Cerrar runtime y cargar restaura mapa y posición de la fixture.

### RUN-11 — Crear vertical slice jugable del runtime (5 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **1 de 5 jornadas**.  
> **En el repo hoy:** examples/juego.cpp; examples/demo_aventura.cpp; examples/demo_campana_boundington.cpp; examples/demo_prologo.cpp  
> **Cómo leer esta ficha:** Vertical slice jugable existe y es más ambicioso que lo que pedía la ficha.

- **Arquitectura:** INF / RUN / REN — validación integrada de la fase.
- **Dependencias:** RUN-10.
- **Archivos:** + `examples/vertical_slice_runtime.cpp`; + `assets/projects/runtime_slice/*`; ~ `CMakeLists.txt`; ~ CI; + `docs/testing/runtime-slice.md`.
- **Especificar:** Escenario mínimo y controles que demuestren todas las piezas.
- **Preguntar si:** asset provisional permitido y duración exacta de la demo.
- **Diseñar:** Proyecto fixture con dos mapas, obstáculos, animación, portal y save/load.
- **Dirigir:**
  1. **RUN-11.1** — **[1,5 d]** crear contenido fixture.
  2. **RUN-11.2** — **[1,5 d]** conectar subsistemas.
  3. **RUN-11.3** — **[1 d]** añadir modo headless determinista.
  4. **RUN-11.4** — **[1 d]** documentar ejecución.
- **Definición/pseudocódigo:**

```text
launch(project); move(path); assert blockedAt(wall); enter(portal); save(); reload(); assert restored;
```

- **Revisar:** La demo no contiene lógica hardcodeada por nombre de mapa salvo test driver.
- **Criterio de aceptación:** CI completa el recorrido y un humano puede jugarlo localmente.

## Fase 3 — Editor visual de mapas MVP

### EDT-01 — Crear ejecutable MotorEditor (4 días)

> **Estado real:** `PARCIAL` — avance **60 %** · esfuerzo restante **2 de 4 jornadas**.  
> **En el repo hoy:** examples/level_editor.cpp (2707 líneas); CMakeLists.txt::level_editor  
> **Cómo leer esta ficha:** El editor existe y es grande, pero es un ejemplo, no un ejecutable MotorEditor con su propio árbol editor/. Tarea real: promoverlo.

- **Arquitectura:** EDT / INF — proceso separado que reutiliza librerías núcleo.
- **Dependencias:** RUN-11.
- **Archivos:** + `editor/src/main.cpp`; + `editor/include/Editor/EditorApplication.h`; + `editor/src/EditorApplication.cpp`; ~ `CMakeLists.txt`; + `tests/Editor/EditorSmokeTests.cpp`.
- **Especificar:** Ciclo de vida del editor, argumentos, proyecto inicial y salida segura.
- **Preguntar si:** editor portable o instalado y asociación de extensión de proyecto.
- **Diseñar:** Target `motor_editor` enlaza DOC/RUN/REN, nunca al revés.
- **Dirigir:**
  1. **EDT-01.1** — **[1 d]** crear target.
  2. **EDT-01.2** — **[1 d]** iniciar ventana/contexto.
  3. **EDT-01.3** — **[1 d]** abrir proyecto vacío.
  4. **EDT-01.4** — **[1 d]** cerrar liberando recursos.
- **Definición/pseudocódigo:**

```text
EditorApplication app; app.init(args); return app.run();
```

- **Revisar:** Runtime standalone sigue compilando sin Dear ImGui.
- **Criterio de aceptación:** `motor_editor --new` abre y cierra en smoke test.

### EDT-02 — Integrar Dear ImGui y docking (5 días)

> **Estado real:** `REPLANTEAR` — avance **0 %** · esfuerzo restante **5 de 5 jornadas**.  
> **En el repo hoy:** examples/level_editor.cpp (cabecera); include/Render/HudManager.h; include/Render/HudWidgets.h  
> **Cómo leer esta ficha:** DECISIÓN YA TOMADA EN CONTRA: la UI se hizo con el HUD propio del motor porque no había red para vendorizar Dear ImGui. Esta ficha no se puede ejecutar como está escrita — hay que decidir (ver INF-02).

- **Arquitectura:** EDT — shell visual, sin lógica documental.
- **Dependencias:** EDT-01.
- **Archivos:** + `third_party/imgui/` o FetchContent; + `editor/include/Editor/EditorShell.h`; + `editor/src/EditorShell.cpp`; ~ CMake.
- **Especificar:** Versión, estrategia de dependencia, docking, multi-viewport y estilo.
- **Preguntar si:** vendorizar Dear ImGui y activar ventanas fuera de la principal.
- **Diseñar:** Un dockspace raíz con menú y registro de paneles.
- **Dirigir:**
  1. **EDT-02.1** — **[1,5 d]** integrar backend GLFW/OpenGL.
  2. **EDT-02.2** — **[1,5 d]** crear dockspace.
  3. **EDT-02.3** — **[1 d]** registrar panel demo.
  4. **EDT-02.4** — **[1 d]** persistir layout local.
- **Definición/pseudocódigo:**

```text
beginDockspace(); for(panel:registry) if(panel.visible) panel.draw(ctx); endDockspace();
```

- **Revisar:** No guardar estado de proyecto en `imgui.ini`; escala DPI comprobada.
- **Criterio de aceptación:** Paneles pueden acoplarse, ocultarse y restaurar su disposición.

### EDT-03 — Integrar viewport OpenGL del mapa (6 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **2 de 6 jornadas**.  
> **En el repo hoy:** examples/level_editor.cpp; include/Render/IsometricRenderer.h; include/Render/Framebuffer.h  
> **Cómo leer esta ficha:** El viewport del mapa dibuja con el renderer real. Falta zoom/pan formalizados y rejilla configurable.

- **Arquitectura:** EDT → REN — incrusta render sin duplicar renderer.
- **Dependencias:** EDT-02.
- **Archivos:** + `editor/include/Editor/Panels/ViewportPanel.h`; + `editor/src/Panels/ViewportPanel.cpp`; + `include/Render/RenderTarget.h`; + `src/Render/RenderTarget.cpp`; + tests.
- **Especificar:** Framebuffer, resize, coordenadas, foco y overlays.
- **Preguntar si:** viewport único o posibilidad de varias vistas simultáneas.
- **Diseñar:** Render a textura; panel presenta textura y transforma ratón a viewport.
- **Dirigir:**
  1. **EDT-03.1** — **[1,5 d]** crear FBO RAII.
  2. **EDT-03.2** — **[1,5 d]** renderizar mapa.
  3. **EDT-03.3** — **[1 d]** manejar resize diferido.
  4. **EDT-03.4** — **[1 d]** traducir coordenadas.
  5. **EDT-03.5** — **[1 d]** probar framebuffer.
- **Definición/pseudocódigo:**

```text
target.resize(panelSize); renderer.render(target); ImGui::Image(target.texture); mouse=panelToWorld(cursor);
```

- **Revisar:** No recrear FBO cada frame; comprobar DPI y viewport cero.
- **Criterio de aceptación:** El mapa se ve dentro del panel y un clic devuelve grid correcto.

### EDT-04 — Crear panel de proyecto y árbol de mapas (6 días)

> **Estado real:** `PARCIAL` — avance **50 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** include/Editor/ProjectHub.h; include/Editor/ProjectIndex.h; examples/demo_proyectos.cpp  
> **Cómo leer esta ficha:** Hub de proyectos con modos y comprobaciones. Falta el árbol de mapas navegable dentro del editor.

- **Arquitectura:** EDT / DOC — navegación sobre referencias documentales.
- **Dependencias:** EDT-03.
- **Archivos:** + `editor/include/Editor/Panels/ProjectPanel.h`; + `editor/src/Panels/ProjectPanel.cpp`; + `editor/include/Editor/Selection/DocumentSelection.h`; + tests.
- **Especificar:** Crear, abrir, seleccionar, renombrar, ordenar y filtrar mapas.
- **Preguntar si:** la jerarquía es una carpeta real o solo organización lógica.
- **Diseñar:** Panel emite comandos documentales; no modifica containers directamente.
- **Dirigir:**
  1. **EDT-04.1** — **[1,5 d]** mostrar árbol.
  2. **EDT-04.2** — **[1,5 d]** seleccionar MapId.
  3. **EDT-04.3** — **[1,5 d]** crear/renombrar vía servicio.
  4. **EDT-04.4** — **[1,5 d]** añadir filtro y estados.
- **Definición/pseudocódigo:**

```text
onRename(id,name) -> commandBus.execute(RenameDocument{id,name});
```

- **Revisar:** Renombrar no cambia ID ni rutas referenciadas; selección inválida se limpia.
- **Criterio de aceptación:** Crear y renombrar dos mapas se refleja tras recargar proyecto.

### EDT-05 — Crear navegador de assets (6 días)

> **Estado real:** `PARCIAL` — avance **45 %** · esfuerzo restante **4 de 6 jornadas**.  
> **En el repo hoy:** EditorState::setPaletteFilter(); assets/generated/authoring/asset_manifest.json; assets/objects/previews_pendientes.json  
> **Cómo leer esta ficha:** Hay filtro por texto y familias sobre miles de entradas. Falta navegador con miniaturas: previews_pendientes.json es literalmente la lista de lo que falta.

- **Arquitectura:** EDT / DOC / TOOL — exploración, no carga GPU directa.
- **Dependencias:** EDT-04.
- **Archivos:** + `editor/include/Editor/Panels/AssetBrowserPanel.h`; + `editor/src/Panels/AssetBrowserPanel.cpp`; + `include/Project/AssetCatalog.h`; + tests.
- **Especificar:** Categorías, búsqueda, miniaturas, estados missing/dirty y drag source.
- **Preguntar si:** qué formatos y carpetas se admiten en el MVP.
- **Diseñar:** Catálogo indexa metadatos; preview service resuelve miniatura.
- **Dirigir:**
  1. **EDT-05.1** — **[1,5 d]** definir AssetEntry.
  2. **EDT-05.2** — **[1,5 d]** indexar proyecto.
  3. **EDT-05.3** — **[1,5 d]** mostrar grid/lista.
  4. **EDT-05.4** — **[1,5 d]** añadir búsqueda y drag payload.
- **Definición/pseudocódigo:**

```text
results=catalog.query(filter); for(asset:results) drawThumbnail(asset.previewId);
```

- **Revisar:** Miles de assets no bloquean cada frame; missing asset visible.
- **Criterio de aceptación:** Buscar un PNG por nombre y arrastrarlo produce un `AssetId` válido.

### EDT-06 — Crear paleta de tiles (6 días)

> **Estado real:** `DONE` — avance **85 %** · esfuerzo restante **1 de 6 jornadas**.  
> **En el repo hoy:** EditorState::setTilePalette/setTilePaletteLabels/nextPaletteEntry; EDITOR_TAXONOMIA.md  
> **Cómo leer esta ficha:** Paleta con etiquetas de autor, filtro y paginación. Falta la rejilla visual del tileset.

- **Arquitectura:** EDT / MAP — selección de TileDefinition.
- **Dependencias:** EDT-05.
- **Archivos:** + `editor/include/Editor/Panels/TilePalettePanel.h`; + `editor/src/Panels/TilePalettePanel.cpp`; + `editor/include/Editor/Tools/TileSelection.h`; + tests.
- **Especificar:** Tileset activo, grid, selección simple/múltiple, zoom y categorías.
- **Preguntar si:** se permiten patrones rectangulares multi-tile desde el MVP.
- **Diseñar:** Selección almacena IDs y forma, no UVs.
- **Dirigir:**
  1. **EDT-06.1** — **[1,5 d]** mostrar atlas por definiciones.
  2. **EDT-06.2** — **[1,5 d]** implementar selección.
  3. **EDT-06.3** — **[1,5 d]** añadir tabs/filtros.
  4. **EDT-06.4** — **[1,5 d]** exponer ToolContext.
- **Definición/pseudocódigo:**

```text
selection = TilePattern{width,height,[TileId...]};
```

- **Revisar:** Tile vacío/borrador explícito; selección persiste al cambiar mapa solo si compatible.
- **Criterio de aceptación:** Seleccionar patrón 2×2 entrega cuatro IDs ordenados a la herramienta.

### EDT-07 — Implementar selección y picking isométrico (5 días)

> **Estado real:** `DONE` — avance **85 %** · esfuerzo restante **1 de 5 jornadas**.  
> **En el repo hoy:** include/Core/Math/IsoMath.h; examples/level_editor.cpp (picking bajo cursor)  
> **Cómo leer esta ficha:** Picking isométrico resuelto y en uso diario.

- **Arquitectura:** EDT / REN / MAP — traducción precisa de interacción.
- **Dependencias:** EDT-06.
- **Archivos:** + `editor/include/Editor/Viewport/ViewportPicker.h`; + `editor/src/Viewport/ViewportPicker.cpp`; + `tests/Editor/ViewportPickerTests.cpp`; ~ `IsoMath` si falta contrato.
- **Especificar:** Screen→viewport→world→grid, borde de diamante, capa y entidad.
- **Preguntar si:** al solaparse entidad y tile, qué prioridad de selección aplica.
- **Diseñar:** Picker puro devuelve candidatos ordenados y razón.
- **Dirigir:**
  1. **EDT-07.1** — **[1,5 d]** normalizar coordenadas.
  2. **EDT-07.2** — **[1,5 d]** resolver celda isométrica.
  3. **EDT-07.3** — **[1 d]** hacer hit-test de entidades.
  4. **EDT-07.4** — **[1 d]** añadir política de prioridad.
- **Definición/pseudocódigo:**

```text
PickResult pick(screen, camera, map, mode) -> {grid, layerId?, entityId?, candidates};
```

- **Revisar:** Probar vértices y bordes con zoom/DPI; sin off-by-one negativos.
- **Criterio de aceptación:** Tabla de puntos de fixture selecciona exactamente la celda esperada.

### EDT-08 — Implementar lápiz, borrador y cuentagotas (6 días)

> **Estado real:** `PARCIAL` — avance **75 %** · esfuerzo restante **2 de 6 jornadas**.  
> **En el repo hoy:** EditorState::paintTile/eraseTile/applyAt  
> **Cómo leer esta ficha:** Lápiz y borrador sí. Cuentagotas (tomar el GID de la celda bajo el cursor) NO existe y es media jornada.

- **Arquitectura:** EDT / MAP / DOC — primeras mutaciones editables.
- **Dependencias:** EDT-07.
- **Archivos:** + `editor/include/Editor/Tools/PencilTool.h`; + `EraserTool.h`; + `EyedropperTool.h`; + implementaciones; + tests.
- **Especificar:** Mouse down/drag/up, repetición, capa bloqueada y patrón.
- **Preguntar si:** el borrador elimina solo capa activa o todas con modificador.
- **Diseñar:** Herramientas generan comandos; preview no muta documento.
- **Dirigir:**
  1. **EDT-08.1** — **[1,5 d]** definir IEditorTool.
  2. **EDT-08.2** — **[1,5 d]** implementar trazo deduplicado.
  3. **EDT-08.3** — **[1,5 d]** crear preview.
  4. **EDT-08.4** — **[1,5 d]** probar cancelación.
- **Definición/pseudocódigo:**

```text
onDrag(cell){ if(newCell) pending.add(SetTilesCommand(layer,patternAt(cell))); }
```

- **Revisar:** Un trazo es una unidad de undo; capa bloqueada no cambia.
- **Criterio de aceptación:** Pintar, borrar y muestrear un tile funciona y deshacer restaura el estado.

### EDT-09 — Implementar rectángulo, relleno y selección (6 días)

> **Estado real:** `PARCIAL` — avance **80 %** · esfuerzo restante **2 de 6 jornadas**.  
> **En el repo hoy:** EditorState::fillTiles/setSelection/copySelection/pasteAt/eraseSelection  
> **Cómo leer esta ficha:** Relleno conectado, selección rectangular y portapapeles hechos, con un solo paso de historial. Falta la herramienta rectángulo/línea que PINTA.

- **Arquitectura:** EDT / MAP — operaciones por región.
- **Dependencias:** EDT-08.
- **Archivos:** + `editor/include/Editor/Tools/RectangleTool.h`; + `FillTool.h`; + `SelectionTool.h`; + implementaciones; + tests.
- **Especificar:** Contorno/relleno, flood-fill, límites, selección y preview.
- **Preguntar si:** relleno compara TileId exacto o categoría/autotile resultante.
- **Diseñar:** Calcular delta antes de aplicar; limitar flood-fill defensivamente.
- **Dirigir:**
  1. **EDT-09.1** — **[1,5 d]** rectángulo inclusivo.
  2. **EDT-09.2** — **[1,5 d]** flood-fill iterativo.
  3. **EDT-09.3** — **[1,5 d]** selección rectangular.
  4. **EDT-09.4** — **[1,5 d]** preview y cancelación.
- **Definición/pseudocódigo:**

```text
queue=[start]; while(queue&&!limit){ if(matches(cell,target)) add(cell); enqueue(neighbors); }
```

- **Revisar:** Sin recursión que desborde stack; mapa grande con límite y progreso.
- **Criterio de aceptación:** Fixtures validan áreas exactas y cancelación sin cambios.

### EDT-10 — Crear panel de capas (5 días)

> **Estado real:** `PARCIAL` — avance **65 %** · esfuerzo restante **2 de 5 jornadas**.  
> **En el repo hoy:** EditorState::addLayer/removeLayer/setActiveLayer/layerName  
> **Cómo leer esta ficha:** Capas creables, renombrables y activables. Faltan visibilidad, bloqueo, opacidad y reordenar — justo lo que hace útil el panel.

- **Arquitectura:** EDT / DOC / MAP — controla `MapLayer` tipada.
- **Dependencias:** EDT-09.
- **Archivos:** + `editor/include/Editor/Panels/LayersPanel.h`; + `editor/src/Panels/LayersPanel.cpp`; + comandos de capa; + tests.
- **Especificar:** Crear, renombrar, ordenar, mostrar, bloquear y opacidad.
- **Preguntar si:** se permite reordenar capas entre tipos con reglas de render distintas.
- **Diseñar:** Operaciones por LayerId y validación documental.
- **Dirigir:**
  1. **EDT-10.1** — **[1,5 d]** listar capas.
  2. **EDT-10.2** — **[1,5 d]** crear/eliminar con confirmación.
  3. **EDT-10.3** — **[1 d]** drag reorder.
  4. **EDT-10.4** — **[1 d]** toggle/opacity.
- **Definición/pseudocódigo:**

```text
execute(ReorderLayer{mapId,layerId,beforeLayerId});
```

- **Revisar:** No eliminar capa no vacía sin decisión; capa activa siempre válida.
- **Criterio de aceptación:** Orden, bloqueo y visibilidad sobreviven guardado/carga.

### EDT-11 — Crear inspector contextual (6 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **2 de 6 jornadas**.  
> **En el repo hoy:** include/Editor/ObjectInspector.h; src/Editor/ObjectInspector.cpp; EditorState::setObject*(); commit 22aac83, 008585f  
> **Cómo leer esta ficha:** Inspector por instancia con botín, eventos, variantes y propiedades libres. Es de lo más maduro del editor.

- **Arquitectura:** EDT — edición de propiedades de selección.
- **Dependencias:** EDT-10.
- **Archivos:** + `editor/include/Editor/Panels/InspectorPanel.h`; + `editor/include/Editor/Inspectors/*`; + implementaciones; + tests.
- **Especificar:** Inspector de mapa, capa, tile y entidad con edición validada.
- **Preguntar si:** edición aplica inmediatamente o requiere Apply/Cancel por objeto.
- **Diseñar:** Registro de inspector por tipo; cambios como comandos.
- **Dirigir:**
  1. **EDT-11.1** — **[1,5 d]** crear contexto de selección.
  2. **EDT-11.2** — **[1,5 d]** registrar inspectores.
  3. **EDT-11.3** — **[1,5 d]** editar campos básicos.
  4. **EDT-11.4** — **[1,5 d]** mostrar errores inline.
- **Definición/pseudocódigo:**

```text
inspector = registry.resolve(selection.type); inspector.draw(selection,commandBus);
```

- **Revisar:** No retener punteros a selección destruida; invalid input no muta.
- **Criterio de aceptación:** Cambiar nombre/opacidad/posición desde inspector se puede deshacer.

### EDT-12 — Implementar CommandHistory y deshacer-rehacer (7 días)

> **Estado real:** `REPLANTEAR` — avance **70 %** · esfuerzo restante **3 de 7 jornadas**.  
> **En el repo hoy:** EditorState::undo/redo/recordUndo/Snapshot  
> **Cómo leer esta ficha:** Hay deshacer/rehacer robusto pero por SNAPSHOT COMPLETO del mapa, no por comandos. Funciona; escala mal en memoria. Decidir si se migra o se documenta el límite.

- **Arquitectura:** EDT / DOC — transacciones reversibles.
- **Dependencias:** EDT-11.
- **Archivos:** + `editor/include/Editor/Commands/IEditorCommand.h`; + `CommandHistory.h`; + `editor/src/Commands/*`; + tests.
- **Especificar:** Execute/undo/redo, merge de trazos, límite, dirty state y transacción.
- **Preguntar si:** historial se persiste entre sesiones o solo sesión actual.
- **Diseñar:** Comandos poseen delta mínimo; macros agrupan operaciones.
- **Dirigir:**
  1. **EDT-12.1** — **[1,5 d]** definir interfaz.
  2. **EDT-12.2** — **[1,5 d]** crear pilas y cursor.
  3. **EDT-12.3** — **[1,5 d]** merge por gesture.
  4. **EDT-12.4** — **[1,5 d]** integrar dirty flag.
  5. **EDT-12.5** — **[1 d]** probar excepciones.
- **Definición/pseudocódigo:**

```text
execute(c){ c.apply(doc); undo.push(c); redo.clear(); } undo(){ c=pop; c.revert(doc); redo.push(c); }
```

- **Revisar:** Redo se invalida tras nueva acción; fallo deja documento consistente.
- **Criterio de aceptación:** Secuencia de diez cambios recorre undo/redo y recupera hashes esperados.

### EDT-13 — Implementar copiar, pegar y duplicar (4 días)

> **Estado real:** `DONE` — avance **85 %** · esfuerzo restante **1 de 4 jornadas**.  
> **En el repo hoy:** EditorState::copySelection/pasteAt/eraseSelection/Clipboard  
> **Cómo leer esta ficha:** Copiar, pegar y borrar selección con capas y objetos incluidos.

- **Arquitectura:** EDT / DOC — portapapeles tipado.
- **Dependencias:** EDT-12.
- **Archivos:** + `editor/include/Editor/Clipboard/EditorClipboard.h`; + `editor/src/Clipboard/EditorClipboard.cpp`; + tests; ~ herramientas.
- **Especificar:** Tiles, entidades, offsets, IDs y referencias externas.
- **Preguntar si:** copiar entre proyectos y sistema operativo forma parte del MVP.
- **Diseñar:** Payload versionado; al pegar entidades se generan IDs nuevos y remapeo interno.
- **Dirigir:**
  1. **EDT-13.1** — **[1 d]** definir payload.
  2. **EDT-13.2** — **[1 d]** copiar selección.
  3. **EDT-13.3** — **[1 d]** pegar con offset.
  4. **EDT-13.4** — **[1 d]** duplicar vía mismo pipeline.
- **Definición/pseudocódigo:**

```text
payload=serializeSelection(sel); pasted=cloneWithNewIds(payload,target); execute(InsertCommand(pasted));
```

- **Revisar:** No duplicar IDs; referencias internas apuntan a clones correctos.
- **Criterio de aceptación:** Duplicar grupo conserva disposición y cada entidad tiene ID único.

### EDT-14 — Implementar guardado, autosave y recuperación (5 días)

> **Estado real:** `PARCIAL` — avance **40 %** · esfuerzo restante **3 de 5 jornadas**.  
> **En el repo hoy:** EditorState::exportTmx/exportLevelJson; examples/level_editor.cpp (guardado)  
> **Cómo leer esta ficha:** Se guarda a petición. No hay autosave, ni archivo de recuperación, ni aviso de cambios sin guardar.

- **Arquitectura:** EDT / DOC — seguridad de trabajo del creador.
- **Dependencias:** EDT-13.
- **Archivos:** + `editor/include/Editor/Services/DocumentSaveService.h`; + `RecoveryService.h`; + implementaciones; + tests; ~ serializers.
- **Especificar:** Save, Save As, autosave, recovery, conflictos y confirmación de cierre.
- **Preguntar si:** frecuencia, número de copias y directorio de recuperación.
- **Diseñar:** Autosave separado de documento oficial; escritura atómica y manifest.
- **Dirigir:**
  1. **EDT-14.1** — **[1 d]** guardar explícito.
  2. **EDT-14.2** — **[1 d]** programar autosave por dirty.
  3. **EDT-14.3** — **[1 d]** detectar crash.
  4. **EDT-14.4** — **[1 d]** ofrecer restauración.
  5. **EDT-14.5** — **[1 d]** limpiar al guardar.
- **Definición/pseudocódigo:**

```text
if(dirty&&elapsed>interval) recovery.write(snapshot); onOpen(){ if(recovery.newer) promptRestore(); }
```

- **Revisar:** Simular interrupción durante write; nunca corromper última versión válida.
- **Criterio de aceptación:** Tras cierre forzado, editor ofrece recuperar cambios no guardados.

### EDT-15 — Validar creación de un mapa sin usar Tiled (4 días)

> **Estado real:** `PARCIAL` — avance **70 %** · esfuerzo restante **2 de 4 jornadas**.  
> **En el repo hoy:** include/Editor/EditorValidation.h; examples/demo_editor_state.cpp; level_editor_output.png  
> **Cómo leer esta ficha:** Ya se crean mapas sin Tiled. Falta el guion de validación formal y la evidencia reproducible en CI.

- **Arquitectura:** INF / EDT / DOC / REN — hito de editor MVP.
- **Dependencias:** EDT-14.
- **Archivos:** + `tests/Acceptance/EditorMapMvp.md`; + `assets/projects/editor_mvp/*`; + test automation posible; ~ CI/documentación.
- **Especificar:** Guion humano y automático para crear, guardar, reabrir y renderizar.
- **Preguntar si:** quién firma la revisión UX y resolución/tamaño mínimo de pantalla.
- **Diseñar:** Checklist de aceptación con evidencia y fixture resultante.
- **Dirigir:**
  1. **EDT-15.1** — **[1 d]** crear proyecto desde cero.
  2. **EDT-15.2** — **[1 d]** importar tileset.
  3. **EDT-15.3** — **[1 d]** pintar capas.
  4. **EDT-15.4** — **[0,5 d]** deshacer/rehacer.
  5. **EDT-15.5** — **[0,5 d]** guardar/reabrir/playtest.
- **Definición/pseudocódigo:**

```text
newProject(); createMap(); paint(); undo(); redo(); save(); reopen(); assertSameDocumentAndRender();
```

- **Revisar:** No editar JSON/TMX manualmente; registrar fricciones como tareas nuevas.
- **Criterio de aceptación:** El técnico completa el guion sin Tiled ni C++ y obtiene el mismo mapa al reabrir.

## Fase 4 — Herramientas avanzadas de mapas

### MAP-01 — Crear importador y editor visual de tilesets (8 días)

> **Estado real:** `PARCIAL` — avance **35 %** · esfuerzo restante **6 de 8 jornadas**.  
> **En el repo hoy:** include/Core/Resources/TextureAtlas.h; assets/textures/terreno_iso.json; REMAPEO_TILES.md; SPRITE_DIMENSIONS_GUIDE.md  
> **Cómo leer esta ficha:** Atlas y remapeo resueltos por JSON y por scripts. Editor visual de tilesets: no existe.

- **Arquitectura:** MAP / EDT / TOOL — incorporación controlada de arte.
- **Dependencias:** EDT-15.
- **Archivos:** + `include/Map/TilesetDocument.h`; + `include/Import/TilesetImporter.h`; + `src/Import/TilesetImporter.cpp`; + `editor/.../TilesetEditorPanel.*`; + tests.
- **Especificar:** Imagen, tile size, margen, spacing, columnas, origen y reimportación.
- **Preguntar si:** formatos, tamaños variables y compatibilidad TSX obligatoria.
- **Diseñar:** Import report crea/actualiza TilesetDocument conservando TileId.
- **Dirigir:**
  1. **MAP-01.1** — **[2 d]** definir documento.
  2. **MAP-01.2** — **[1,5 d]** detectar grid.
  3. **MAP-01.3** — **[1,5 d]** crear preview de corte.
  4. **MAP-01.4** — **[1,5 d]** guardar parámetros.
  5. **MAP-01.5** — **[1,5 d]** implementar reimportación segura.
- **Definición/pseudocódigo:**

```text
draft=detect(image); user.adjust(grid); report=import(draft); catalog.upsert(report.tileset);
```

- **Revisar:** Reimportar no cambia IDs existentes sin aviso; validar dimensiones.
- **Criterio de aceptación:** Importar una hoja, corregir grid y usar sus tiles en un mapa.

### MAP-02 — Implementar TileDefinition y metadatos (6 días)

> **Estado real:** `PARCIAL` — avance **40 %** · esfuerzo restante **4 de 6 jornadas**.  
> **En el repo hoy:** include/Render/Tile.h; TmxTilesetSettings::collisionGids; EDITOR_TAXONOMIA.md  
> **Cómo leer esta ficha:** Metadatos de tile hoy = colisión booleana por GID. Falta TileDefinition con terreno, coste, sonido y etiquetas.

- **Arquitectura:** MAP / DOC / RUN — semántica de cada tile.
- **Dependencias:** MAP-01.
- **Archivos:** + `include/Map/TileDefinition.h`; + `include/Map/NavigationData.h`; + `include/Map/TerrainTags.h`; + serializers/schemas; + `editor/.../TileInspector.*`; + tests.
- **Especificar:** Visual, colisión, direcciones, coste, tags, elevación y animación.
- **Preguntar si:** tags libres, catálogo cerrado o combinación.
- **Diseñar:** TileInstance guarda TileId; definición central resuelve propiedades.
- **Dirigir:**
  1. **MAP-02.1** — **[1,5 d]** definir metadatos.
  2. **MAP-02.2** — **[1,5 d]** serializar.
  3. **MAP-02.3** — **[1,5 d]** crear editor por lote.
  4. **MAP-02.4** — **[1,5 d]** compilar a runtime.
- **Definición/pseudocódigo:**

```text
TileDefinition{id,visual,navigation,tags,elevation,animation}; runtime.resolve(tileId);
```

- **Revisar:** Cambiar definición actualiza mapas sin reescribir celdas.
- **Criterio de aceptación:** Modificar walkable de un TileId afecta dos mapas al recompilar.

### MAP-03 — Implementar autotiles isométricos (9 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **9 de 9 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Nada hecho. Es la tarea de mayor riesgo técnico de la fase 4 en isométrico.

- **Arquitectura:** MAP / EDT / RUN — conexión visual por vecindad.
- **Dependencias:** MAP-02.
- **Archivos:** + `include/Map/AutotileRuleSet.h`; + `src/Map/AutotileResolver.cpp`; + `editor/.../AutotileEditorPanel.*`; + tests/fixtures.
- **Especificar:** Vecindad, máscaras, esquinas, prioridades, preview y override manual.
- **Preguntar si:** 4 u 8 vecinos y tipos iniciales: suelo, carretera, río, muro.
- **Diseñar:** Reglas data-driven; mapa guarda terreno lógico u override.
- **Dirigir:**
  1. **MAP-03.1** — **[2 d]** definir máscara.
  2. **MAP-03.2** — **[2 d]** resolver variantes.
  3. **MAP-03.3** — **[2 d]** integrar pincel.
  4. **MAP-03.4** — **[1,5 d]** crear editor reglas.
  5. **MAP-03.5** — **[1,5 d]** probar combinaciones.
- **Definición/pseudocódigo:**

```text
mask=neighbors.map(matchesTerrain); variant=rules.resolve(mask); if(manual) variant=override;
```

- **Revisar:** Determinismo y actualización acotada a celda+vecinos.
- **Criterio de aceptación:** Pintar río y carretera genera bordes correctos en fixture exhaustiva.

### MAP-04 — Crear editor de colisiones direccionales (7 días)

> **Estado real:** `PARCIAL` — avance **25 %** · esfuerzo restante **6 de 7 jornadas**.  
> **En el repo hoy:** include/Render/OcclusionRules.h; src/Render/TileMap.cpp  
> **Cómo leer esta ficha:** Colisión booleana + reglas de oclusión. Direccional (por lado de celda) no existe.

- **Arquitectura:** MAP / EDT / RUN — authoring de passability.
- **Dependencias:** MAP-03.
- **Archivos:** + `editor/.../CollisionEditorPanel.*`; + `editor/.../Tools/CollisionPaintTool.*`; ~ `NavigationData`; ~ `CollisionSystem`; + tests.
- **Especificar:** Bloqueo N/E/S/O, overlay, selección múltiple y presets.
- **Preguntar si:** colisión por tile definition, override por celda o ambos.
- **Diseñar:** Definición base + override opcional de mapa con precedencia explícita.
- **Dirigir:**
  1. **MAP-04.1** — **[1,5 d]** crear overlay.
  2. **MAP-04.2** — **[1,5 d]** editar flags.
  3. **MAP-04.3** — **[1,5 d]** añadir presets.
  4. **MAP-04.4** — **[1,5 d]** compilar navegación.
  5. **MAP-04.5** — **[1 d]** probar entrada/salida.
- **Definición/pseudocódigo:**

```text
effective = cell.override.value_or(tileDef.navigation); canMove(a,b)=a.exit(dir)&&b.enter(opposite);
```

- **Revisar:** Visualización distingue base/override; simetría configurable.
- **Criterio de aceptación:** Mapa prueba permite paso en un sentido y lo bloquea en contrario.

### MAP-05 — Crear capas y canales de regiones (7 días)

> **Estado real:** `PENDIENTE` — avance **10 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** include/Core/Math/GridBounds.h  
> **Cómo leer esta ficha:** Los rectángulos existen como matemática, no como regiones autorales con canal y semántica.

- **Arquitectura:** MAP / EDT / RUN — semántica espacial superpuesta.
- **Dependencias:** MAP-04.
- **Archivos:** + `include/Map/RegionChannel.h`; + `include/Map/RegionDefinition.h`; + `editor/.../RegionPanel.*`; + `RegionPaintTool.*`; + tests.
- **Especificar:** Canales bioma, localización, encuentro y trigger; múltiples valores por celda.
- **Preguntar si:** si un canal admite un valor o conjunto de valores por celda.
- **Diseñar:** RegionLayer por canal con IDs semánticos y leyenda.
- **Dirigir:**
  1. **MAP-05.1** — **[2 d]** definir canales.
  2. **MAP-05.2** — **[2 d]** pintar/borrar.
  3. **MAP-05.3** — **[1,5 d]** overlay y leyenda.
  4. **MAP-05.4** — **[1,5 d]** consulta runtime.
- **Definición/pseudocódigo:**

```text
regions.at(cell).set(channel, regionId); query(cell,channel)->RegionId;
```

- **Revisar:** Ocultar canales no borra datos; colores distinguibles.
- **Criterio de aceptación:** Una celda pertenece a bioma bosque y trigger emboscada simultáneamente.

### MAP-06 — Implementar editor de propiedades del mapa (5 días)

> **Estado real:** `PARCIAL` — avance **30 %** · esfuerzo restante **4 de 5 jornadas**.  
> **En el repo hoy:** include/Level/LevelDefinition.h; FORMATO_NIVELES.md  
> **Cómo leer esta ficha:** Hay campos de nivel en el formato pero no un panel de propiedades del mapa.

- **Arquitectura:** MAP / EDT / DOC — configuración de escena.
- **Dependencias:** MAP-05.
- **Archivos:** + `include/Map/MapSettings.h`; + serializers/schema; + `editor/.../MapPropertiesDialog.*`; + tests.
- **Especificar:** Nombre, tamaño, wrap, tilesets, ambiente, cámara y reglas.
- **Preguntar si:** qué propiedades pertenecen al mapa y cuáles al juego global.
- **Diseñar:** Secciones tipadas y validación previa a Apply.
- **Dirigir:**
  1. **MAP-06.1** — **[1,5 d]** definir settings.
  2. **MAP-06.2** — **[1,5 d]** crear diálogo.
  3. **MAP-06.3** — **[1 d]** validar resize destructivo.
  4. **MAP-06.4** — **[1 d]** emitir comando macro.
- **Definición/pseudocódigo:**

```text
draft=settings; if(validate(draft)&&confirmDestructiveDiff) execute(UpdateMapSettings{before,draft});
```

- **Revisar:** Reducir mapa muestra datos que se perderán y es deshacible.
- **Criterio de aceptación:** Cambios sobreviven reload y resize no destruye sin confirmación.

### MAP-07 — Implementar colocación de entidades (7 días)

> **Estado real:** `DONE` — avance **85 %** · esfuerzo restante **2 de 7 jornadas**.  
> **En el repo hoy:** EditorState::placeObject/placeSpawn/setObjectPatrol; include/Level/LevelDefinition.h::ObjectSpawn  
> **Cómo leer esta ficha:** Colocación de entidades con patrulla, escala, variante y destino. Muy por delante de la ficha.

- **Arquitectura:** MAP / EDT / RUN — instancias documentales en el mapa.
- **Dependencias:** MAP-06.
- **Archivos:** + `include/Map/EntityInstance.h`; + `include/Project/EntityPrototypeRef.h`; + `editor/.../EntityPlacementTool.*`; + `EntityInspector.*`; + runtime builder/tests.
- **Especificar:** Prototype, transform, layer, overrides, selección y eliminación.
- **Preguntar si:** entidad libre en píxeles, por grid o ambas.
- **Diseñar:** Instancia referencia prototype y guarda overrides mínimos.
- **Dirigir:**
  1. **MAP-07.1** — **[2 d]** definir instancia.
  2. **MAP-07.2** — **[2 d]** drag/drop al viewport.
  3. **MAP-07.3** — **[1,5 d]** seleccionar/mover.
  4. **MAP-07.4** — **[1,5 d]** compilar al World.
- **Definición/pseudocódigo:**

```text
instance={id,prototypeId,grid,elevation,overrides}; world.spawn(resolve(instance));
```

- **Revisar:** Mover es undoable; prototype ausente muestra placeholder.
- **Criterio de aceptación:** Colocar NPC, guardar y jugar lo crea en la misma celda.

### MAP-08 — Implementar prefabs y plantillas (7 días)

> **Estado real:** `PARCIAL` — avance **30 %** · esfuerzo restante **5 de 7 jornadas**.  
> **En el repo hoy:** include/Level/ObjectCatalog.h; OBJECT_VARIANTS_FOR_EDITOR.md  
> **Cómo leer esta ficha:** Variantes de objeto sí; prefabs multi-celda y plantillas de habitación, no.

- **Arquitectura:** DOC / MAP / EDT — reutilización de composiciones.
- **Dependencias:** MAP-07.
- **Archivos:** + `include/Project/PrefabDocument.h`; + `src/Project/PrefabService.cpp`; + `editor/.../PrefabPanel.*`; + tests/schemas.
- **Especificar:** Prefab de entidad y grupo, instancias vinculadas, overrides y actualización.
- **Preguntar si:** instancias se actualizan automáticamente al cambiar prefab.
- **Diseñar:** Árbol de objetos con IDs locales; instancia remapea a IDs globales.
- **Dirigir:**
  1. **MAP-08.1** — **[1,5 d]** definir formato.
  2. **MAP-08.2** — **[1,5 d]** crear desde selección.
  3. **MAP-08.3** — **[1,5 d]** instanciar.
  4. **MAP-08.4** — **[1,5 d]** editar overrides.
  5. **MAP-08.5** — **[1 d]** resolver actualización.
- **Definición/pseudocódigo:**

```text
clone=prefab.instantiate(newRootId); remapLocalIds(clone); applyOverrides(instance);
```

- **Revisar:** No romper referencias internas; prevenir ciclos prefab.
- **Criterio de aceptación:** Cambiar sprite base actualiza instancias no sobrescritas.

### MAP-09 — Implementar navegación y pathfinding (7 días)

> **Estado real:** `PARCIAL` — avance **20 %** · esfuerzo restante **6 de 7 jornadas**.  
> **En el repo hoy:** include/Game/EnemyBrain.h; src/Game/EnemyBrain.cpp  
> **Cómo leer esta ficha:** IA de enemigo con movimiento simple. Sin malla de navegación ni A*.

- **Arquitectura:** RUN / MAP — ruta sobre datos compilados.
- **Dependencias:** MAP-08.
- **Archivos:** + `include/World/Navigation/NavGrid.h`; + `Pathfinder.h`; + `src/World/Navigation/*.cpp`; + debug overlay; + tests.
- **Especificar:** A*, coste, diagonales, entidades dinámicas y límites.
- **Preguntar si:** tamaño máximo de mapa, movimiento diagonal y alturas.
- **Diseñar:** NavGrid derivado de mapa; obstáculos dinámicos como capa aparte.
- **Dirigir:**
  1. **MAP-09.1** — **[1,5 d]** compilar grid.
  2. **MAP-09.2** — **[1,5 d]** implementar A*.
  3. **MAP-09.3** — **[1,5 d]** integrar occupancy.
  4. **MAP-09.4** — **[1,5 d]** añadir overlay.
  5. **MAP-09.5** — **[1 d]** medir presupuesto.
- **Definición/pseudocódigo:**

```text
open.push(start); while(open){ n=lowestF(); if(n==goal)return reconstruct(); for(next:neighbors) relax(n,next,cost); }
```

- **Revisar:** Path imposible termina; heurística admisible; rendimiento fixture grande.
- **Criterio de aceptación:** NPC encuentra ruta alrededor de muro y replanifica al bloquearse.

### MAP-10 — Implementar conexiones y puntos de aparición (5 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **1 de 5 jornadas**.  
> **En el repo hoy:** EditorState::setObjectTargetLevel/setObjectTargetPosition; include/Level/WorldTopology.h  
> **Cómo leer esta ficha:** Conexiones y destino por instancia editables desde el mapa.

- **Arquitectura:** MAP / DOC / EDT — grafo navegable del mundo.
- **Dependencias:** MAP-09.
- **Archivos:** + `include/Map/SpawnPoint.h`; ~ `MapLink.h`; + `editor/.../WorldLinksPanel.*`; + validation/tests.
- **Especificar:** Spawns nombrados, destinos, orientación, preview y referencias inversas.
- **Preguntar si:** conexiones se muestran como grafo global en MVP.
- **Diseñar:** IDs estables y validador de destino; panel lista links entrantes/salientes.
- **Dirigir:**
  1. **MAP-10.1** — **[1,5 d]** crear spawn tool.
  2. **MAP-10.2** — **[1,5 d]** editar links.
  3. **MAP-10.3** — **[1 d]** mostrar referencias.
  4. **MAP-10.4** — **[1 d]** validar y navegar.
- **Definición/pseudocódigo:**

```text
validate(link){ exists(mapId)&&exists(spawnId)&&typesCompatible; }
```

- **Revisar:** Eliminar spawn usado bloquea o repara explícitamente.
- **Criterio de aceptación:** Panel navega desde portal a mapa/spawn destino y detecta enlace roto.

### MAP-11 — Mejorar profundidad, elevación y capas altas (8 días)

> **Estado real:** `PARCIAL` — avance **45 %** · esfuerzo restante **5 de 8 jornadas**.  
> **En el repo hoy:** include/Render/OcclusionRules.h; include/Render/BlobShadow.h; src/Render/IsometricRenderer.cpp  
> **Cómo leer esta ficha:** Orden de dibujo y oclusión resueltos. Elevación real (alturas) no existe.

- **Arquitectura:** REN / MAP — orden visual isométrico completo.
- **Dependencias:** MAP-10.
- **Archivos:** + `include/Render/RenderItem.h`; + `include/Render/DepthKey.h`; ~ `IsometricRenderer.*`; ~ render proxies; + fixtures/tests.
- **Especificar:** Intercalar tiles/entidades, elevación, puentes, techos y transparencia.
- **Preguntar si:** número de niveles de altura y política de ocultación de techos.
- **Diseñar:** Cola unificada de RenderItem ordenada por layer/elevation/grid/tie.
- **Dirigir:**
  1. **MAP-11.1** — **[2 d]** definir depth key.
  2. **MAP-11.2** — **[1,5 d]** emitir tiles como items.
  3. **MAP-11.3** — **[1,5 d]** unificar entidades.
  4. **MAP-11.4** — **[1,5 d]** añadir capas altas.
  5. **MAP-11.5** — **[1,5 d]** crear fixture puente.
- **Definición/pseudocódigo:**

```text
key=(renderLayer,elevation,gridX+gridY,gridX,tie); stable_sort(items,key);
```

- **Revisar:** No depender de dirección de memoria; sprites altos con anchor correcto.
- **Criterio de aceptación:** Personaje pasa por debajo y encima de puente con orden visual correcto.

### MAP-12 — Validar aldea, interior y mazmorra conectados (5 días)

> **Estado real:** `PARCIAL` — avance **55 %** · esfuerzo restante **3 de 5 jornadas**.  
> **En el repo hoy:** examples/demo_ciudad.cpp; examples/demo_mundo.cpp; examples/demo_este_norte.cpp; examples/demo_este_sur.cpp; assets/levels/  
> **Cómo leer esta ficha:** Hay aldea e interiores conectados; falta la mazmorra y el guion de validación conjunto.

- **Arquitectura:** INF / MAP / EDT / RUN — aceptación de autoría espacial.
- **Dependencias:** MAP-11.
- **Archivos:** + `assets/projects/map_authoring_slice/*`; + `tests/Acceptance/MapAuthoring.md`; + snapshots; ~ CI/docs.
- **Especificar:** Contenido y recorrido que cubra tilesets, autotiles, regiones, entidades y links.
- **Preguntar si:** resolución, estilo de assets y presupuesto de rendimiento objetivo.
- **Diseñar:** Tres mapas creados solo desde editor con checklist y métricas.
- **Dirigir:**
  1. **MAP-12.1** — **[1 d]** crear aldea.
  2. **MAP-12.2** — **[1 d]** crear interior.
  3. **MAP-12.3** — **[1 d]** crear mazmorra.
  4. **MAP-12.4** — **[1 d]** conectar/spawns.
  5. **MAP-12.5** — **[1 d]** ejecutar recorrido y perf.
- **Definición/pseudocódigo:**

```text
authorThreeMaps(); validateAll(); playRoute(village,interior,dungeon); assertNoBrokenRefs();
```

- **Revisar:** No corregir JSON manualmente; capturar errores UX como backlog.
- **Criterio de aceptación:** Proyecto abre sin warnings y recorrido completo funciona desde 1.ª ejecución.

## Fase 5 — Sistema de eventos tipo RPG Maker

### EVT-01 — Definir EventDefinition y EventPage (6 días)

> **Estado real:** `PARCIAL` — avance **45 %** · esfuerzo restante **4 de 6 jornadas**.  
> **En el repo hoy:** include/RPG/Definitions/RpgCoreDefinitions.h::EventDefinition; include/RPG/NarrativeEngine.h::NarrativeBeat; assets/adventures/*.json  
> **Cómo leer esta ficha:** Existe EventDefinition y el beat narrativo, con aventuras reales en JSON. Falta el concepto de PÁGINA con condiciones que selecciona qué versión del evento corre.

- **Arquitectura:** EVT / DOC — modelo persistente de comportamiento.
- **Dependencias:** MAP-12.
- **Archivos:** + `include/Events/EventDefinition.h`; + `include/Events/EventPage.h`; + serializers/schemas; + tests.
- **Especificar:** Identidad, orden de páginas, prioridad, presentación y contenido.
- **Preguntar si:** página válida de mayor índice como RPG Maker o prioridad explícita.
- **Diseñar:** Definición documental sin estado mutable de ejecución.
- **Dirigir:**
  1. **EVT-01.1** — **[1,5 d]** definir tipos.
  2. **EVT-01.2** — **[1,5 d]** añadir páginas ordenadas.
  3. **EVT-01.3** — **[1,5 d]** serializar.
  4. **EVT-01.4** — **[1,5 d]** validar IDs/prioridad.
- **Definición/pseudocódigo:**

```text
active = pages.filter(p=>conditions(p)).maxBy(priority);
```

- **Revisar:** Página sin condición válida; ninguna válida desactiva evento.
- **Criterio de aceptación:** Fixture resuelve la página esperada para tres estados distintos.

### EVT-02 — Definir Condition, Trigger y EventCommand (6 días)

> **Estado real:** `PARCIAL` — avance **55 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** NarrativeEngine.h::NarrativeCondition/BeatTrigger(Talk,Enter,Auto)/NarrativeEffect/EffectType  
> **Cómo leer esta ficha:** Condiciones, disparadores y efectos ya son datos tipados. Falta que el efecto sea un EventCommand extensible en vez de un enum cerrado.

- **Arquitectura:** EVT — contratos extensibles y tipados.
- **Dependencias:** EVT-01.
- **Archivos:** + `include/Events/Conditions/ICondition.h`; + `Triggers.h`; + `Commands/IEventCommand.h`; + registries; + tests.
- **Especificar:** Tipos iniciales, resultado, errores, serialización y extensibilidad.
- **Preguntar si:** plugins pueden registrar tipos desde 1.0 o se reserva namespace.
- **Diseñar:** Tagged variant/registry con typeId estable; comandos no llaman UI.
- **Dirigir:**
  1. **EVT-02.1** — **[1,5 d]** definir interfaces.
  2. **EVT-02.2** — **[1,5 d]** crear IDs de tipo.
  3. **EVT-02.3** — **[1,5 d]** registrar factories.
  4. **EVT-02.4** — **[1,5 d]** round-trip tipos desconocidos.
- **Definición/pseudocódigo:**

```text
ConditionResult evaluate(EventContext&); CommandStep execute(EventContext&, CommandState&);
```

- **Revisar:** Tipo desconocido se preserva o falla explícitamente, nunca se descarta.
- **Criterio de aceptación:** Condición y comando dummy se serializan, cargan y ejecutan por registry.

### EVT-03 — Implementar EventContext e EventInterpreter (8 días)

> **Estado real:** `PARCIAL` — avance **50 %** · esfuerzo restante **4 de 8 jornadas**.  
> **En el repo hoy:** include/RPG/NarrativeEngine.h::NarrativeEngine/NarrativeState; src/RPG/NarrativeEngine.cpp  
> **Cómo leer esta ficha:** Hay intérprete con estado que ejecuta beats y devuelve resultados. Falta el contexto de ejecución por evento y la pila de llamadas.

- **Arquitectura:** EVT / RUN — ejecución controlada de secuencias.
- **Dependencias:** EVT-02.
- **Archivos:** + `include/Events/EventContext.h`; + `include/Events/EventInterpreter.h`; + `src/Events/EventInterpreter.cpp`; + tests.
- **Especificar:** Stack, instruction pointer, llamadas, pausa, error y cancelación.
- **Preguntar si:** un evento fallido detiene solo instancia o juego completo.
- **Diseñar:** Intérprete stateful por instancia; contexto expone servicios limitados.
- **Dirigir:**
  1. **EVT-03.1** — **[2 d]** definir frame.
  2. **EVT-03.2** — **[1,5 d]** ejecutar step.
  3. **EVT-03.3** — **[1,5 d]** soportar pause/resume.
  4. **EVT-03.4** — **[1,5 d]** propagar error.
  5. **EVT-03.5** — **[1,5 d]** test secuencia.
- **Definición/pseudocódigo:**

```text
while(budget--&&state==Running){ result=command[ip].step(ctx); handle(result); }
```

- **Revisar:** Presupuesto por frame evita bloqueo; stack limitado.
- **Criterio de aceptación:** Secuencia dummy pausa, reanuda, llama subevento y termina determinísticamente.

### EVT-04 — Implementar variables e interruptores (5 días)

> **Estado real:** `PARCIAL` — avance **60 %** · esfuerzo restante **2 de 5 jornadas**.  
> **En el repo hoy:** NarrativeState::setFlag/clearFlag/contadores  
> **Cómo leer esta ficha:** Interruptores y contadores globales funcionan. Falta exponerlos como tabla editable con nombres.

- **Arquitectura:** EVT / RUN / SAVE — estado global tipado.
- **Dependencias:** EVT-03.
- **Archivos:** + `include/Events/State/VariableStore.h`; + `SwitchStore.h`; + implementations; + save schema/tests.
- **Especificar:** Tipos de variable, defaults, operaciones y persistencia.
- **Preguntar si:** variables dinámicas multitype o tipos declarados en proyecto.
- **Diseñar:** Definiciones en proyecto; valores en GameState.
- **Dirigir:**
  1. **EVT-04.1** — **[1,5 d]** definir declarations.
  2. **EVT-04.2** — **[1,5 d]** crear stores.
  3. **EVT-04.3** — **[1 d]** añadir get/set/op.
  4. **EVT-04.4** — **[1 d]** integrar save.
- **Definición/pseudocódigo:**

```text
set(id,value){ assert(type(value)==definition.type); values[id]=value; emitChanged(id); }
```

- **Revisar:** ID ausente y tipo incorrecto devuelven error contextual.
- **Criterio de aceptación:** Cambios sobreviven save/load y condiciones leen valores correctos.

### EVT-05 — Implementar interruptores locales por evento (3 días)

> **Estado real:** `PENDIENTE` — avance **10 %** · esfuerzo restante **3 de 3 jornadas**.  
> **En el repo hoy:** NarrativeState::markBeatFired()  
> **Cómo leer esta ficha:** Lo más parecido es marcar un beat como disparado. Interruptores locales por instancia de evento no existen.

- **Arquitectura:** EVT / SAVE — estado por instancia.
- **Dependencias:** EVT-04.
- **Archivos:** + `include/Events/State/SelfSwitchStore.h`; + implementation; ~ GameState; + tests.
- **Especificar:** Clave MapId+EntityId+nombre, defaults y limpieza.
- **Preguntar si:** instancias de prefab comparten o no estado local.
- **Diseñar:** Clave estable basada en instancia documental, no índice runtime.
- **Dirigir:**
  1. **EVT-05.1** — **[1 d]** definir clave.
  2. **EVT-05.2** — **[1 d]** implementar store.
  3. **EVT-05.3** — **[0,5 d]** persistir.
  4. **EVT-05.4** — **[0,5 d]** probar mapas duplicados.
- **Definición/pseudocódigo:**

```text
key={mapId,eventEntityId,switchName}; selfSwitches.set(key,true);
```

- **Revisar:** Dos eventos con mismo nombre no colisionan.
- **Criterio de aceptación:** Abrir un cofre cambia solo su propia página tras recargar.

### EVT-06 — Implementar scheduler, esperas y procesos paralelos (7 días)

> **Estado real:** `PARCIAL` — avance **30 %** · esfuerzo restante **5 de 7 jornadas**.  
> **En el repo hoy:** NarrativeEngine.h::TimerDefinition/TimerState; examples/demo_temporizadores.cpp  
> **Cómo leer esta ficha:** Temporizadores de misión hechos (commit f5ef2fd). Esperas dentro de un evento y procesos paralelos, no.

- **Arquitectura:** EVT / RUN — concurrencia cooperativa.
- **Dependencias:** EVT-05.
- **Archivos:** + `include/Events/EventScheduler.h`; + `src/Events/EventScheduler.cpp`; + `WaitCommand.*`; + tests.
- **Especificar:** Prioridades, autorun, parallel, wait, cancelación y presupuesto.
- **Preguntar si:** número máximo de eventos simultáneos y política de orden.
- **Diseñar:** Scheduler cooperativo actualiza instancias con budget por frame.
- **Dirigir:**
  1. **EVT-06.1** — **[2 d]** gestionar instancias.
  2. **EVT-06.2** — **[2 d]** implementar wait temporal/condición.
  3. **EVT-06.3** — **[1,5 d]** detectar autorun repetido.
  4. **EVT-06.4** — **[1,5 d]** añadir límites.
- **Definición/pseudocódigo:**

```text
for(instance:readyOrdered){ instance.step(min(perEventBudget,totalBudget)); if(waiting) reschedule; }
```

- **Revisar:** Loop sin yield produce diagnóstico, no congelación.
- **Criterio de aceptación:** Dos paralelos y un diálogo bloqueante se ejecutan con orden documentado.

### EVT-07 — Implementar comandos de diálogo y elecciones (6 días)

> **Estado real:** `PARCIAL` — avance **55 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** include/Level/ObjectCatalog.h::DialogueData; GameMode::Dialogue; assets/adventures/*.json  
> **Cómo leer esta ficha:** Diálogos con elecciones funcionan en las aventuras. Falta el comando de diálogo genérico dentro del intérprete de eventos.

- **Arquitectura:** EVT / RUN / UI — narrativa asíncrona.
- **Dependencias:** EVT-06.
- **Archivos:** + `include/Events/Commands/ShowDialogueCommand.h`; + `ChoiceCommand.h`; + runtime UI interfaces; + tests.
- **Especificar:** Speaker, retrato, texto localizado, opciones, cancelación y resultado.
- **Preguntar si:** sistema de localización entra ahora y número/layout de opciones.
- **Diseñar:** Comando abre request UI y espera respuesta por token.
- **Dirigir:**
  1. **EVT-07.1** — **[1,5 d]** definir DialogueRequest.
  2. **EVT-07.2** — **[1,5 d]** implementar show/wait.
  3. **EVT-07.3** — **[1,5 d]** implementar choices/branches.
  4. **EVT-07.4** — **[1,5 d]** test UI fake.
- **Definición/pseudocódigo:**

```text
token=ui.showDialogue(data); yield until ui.done(token); choice=ui.result(token); jump(branch[choice]);
```

- **Revisar:** Guardar durante diálogo y cancelar ventana tienen comportamiento definido.
- **Criterio de aceptación:** Conversación de fixture elige dos ramas sin bloquear loop.

### EVT-08 — Implementar comandos de estado e inventario (6 días)

> **Estado real:** `PARCIAL` — avance **55 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** include/RPG/InventoryEngine.h; include/RPG/ConditionEngine.h; NarrativeEffect  
> **Cómo leer esta ficha:** Dar/quitar objeto y aplicar estado ya son efectos. Falta cubrir dinero, experiencia y equipo desde el mismo comando.

- **Arquitectura:** EVT / GDB / RUN — mutación transaccional de juego.
- **Dependencias:** EVT-07.
- **Archivos:** + `include/Events/Commands/StateCommands.h`; + `InventoryCommands.h`; + implementations/tests; ~ services.
- **Especificar:** Set/Add/Compare, dar/quitar objeto, validación y resultado.
- **Preguntar si:** quitar más cantidad de la disponible falla, limita o permite negativo.
- **Diseñar:** Comandos llaman servicios; no acceden a containers internos.
- **Dirigir:**
  1. **EVT-08.1** — **[1,5 d]** comandos de variable/switch.
  2. **EVT-08.2** — **[1,5 d]** comandos inventario.
  3. **EVT-08.3** — **[1,5 d]** errores y mensajes.
  4. **EVT-08.4** — **[1,5 d]** tests transaccionales.
- **Definición/pseudocódigo:**

```text
result=inventory.tryRemove(itemId,qty,policy); ctx.setLastResult(result);
```

- **Revisar:** Operación fallida no deja cambio parcial.
- **Criterio de aceptación:** Evento entrega llave, puerta la consume y rama de error funciona.

### EVT-09 — Implementar movimiento, audio, cámara y teletransporte (7 días)

> **Estado real:** `PARCIAL` — avance **30 %** · esfuerzo restante **5 de 7 jornadas**.  
> **En el repo hoy:** GameSession::LevelTransition; include/Render/Camera.h  
> **Cómo leer esta ficha:** Teletransporte sí, cámara programable a medias, AUDIO NO EXISTE en el repo (ver GDB-11).

- **Arquitectura:** EVT → RUN / REN — comandos sobre servicios existentes.
- **Dependencias:** EVT-08.
- **Archivos:** + `include/Events/Commands/MovementCommands.h`; + `PresentationCommands.h`; + implementations/tests; ~ service interfaces.
- **Especificar:** Target, espera hasta fin, cancelación, mapa destino y parámetros.
- **Preguntar si:** movimiento forzado ignora colisión y qué comandos bloquean secuencia.
- **Diseñar:** Cada comando crea operación async y yield hasta resultado si `wait`.
- **Dirigir:**
  1. **EVT-09.1** — **[1,5 d]** definir target selector.
  2. **EVT-09.2** — **[1,5 d]** adaptar movement.
  3. **EVT-09.3** — **[1,5 d]** audio/camera.
  4. **EVT-09.4** — **[1,5 d]** transfer map.
  5. **EVT-09.5** — **[1 d]** test fakes.
- **Definición/pseudocódigo:**

```text
op=movement.route(target,route,flags); return wait?YieldUntil(op):Continue;
```

- **Revisar:** Entidad destruida y cambio de escena cancelan operaciones correctamente.
- **Criterio de aceptación:** Evento mueve NPC, reproduce sonido, centra cámara y transfiere mapa.

### EVT-10 — Implementar ramas, bucles y eventos comunes (7 días)

> **Estado real:** `PARCIAL` — avance **25 %** · esfuerzo restante **6 de 7 jornadas**.  
> **En el repo hoy:** NarrativeEngine (condiciones por beat)  
> **Cómo leer esta ficha:** Hay ramificación por condición de beat, pero no bucles, ni etiquetas, ni eventos comunes reutilizables.

- **Arquitectura:** EVT — control de flujo y reutilización.
- **Dependencias:** EVT-09.
- **Archivos:** + `include/Events/Commands/FlowCommands.h`; + `include/Events/CommonEvent.h`; + interpreter changes; + tests.
- **Especificar:** If/else, loop, break, call/return, argumentos y recursión.
- **Preguntar si:** eventos comunes aceptan parámetros y retornos en 1.0.
- **Diseñar:** Comandos estructurados compilados a offsets/frames validados.
- **Dirigir:**
  1. **EVT-10.1** — **[1,5 d]** condicionales.
  2. **EVT-10.2** — **[1,5 d]** bucles/break.
  3. **EVT-10.3** — **[1,5 d]** call stack.
  4. **EVT-10.4** — **[1,5 d]** límites recursión.
  5. **EVT-10.5** — **[1 d]** validator.
- **Definición/pseudocódigo:**

```text
if(cond) ip=thenOffset else ip=elseOffset; call(id,args)->pushFrame; return->popFrame;
```

- **Revisar:** Ramas mal formadas se rechazan al cargar; stack overflow controlado.
- **Criterio de aceptación:** Fixture con bucle y llamada común produce traza exacta.

### EVT-11 — Crear editor visual de páginas de evento (9 días)

> **Estado real:** `PENDIENTE` — avance **5 %** · esfuerzo restante **9 de 9 jornadas**.  
> **En el repo hoy:** EditorState::setObjectEvent()  
> **Cómo leer esta ficha:** La semilla está puesta: una instancia ya puede referenciar un eventId. Todo el editor de páginas está por hacer.

- **Arquitectura:** EDT / EVT / MAP — autoría de eventos sobre entidad.
- **Dependencias:** EVT-10.
- **Archivos:** + `editor/.../EventEditorWindow.h`; + `EventPagesPanel.*`; + `EventPageInspector.*`; + commands/tests.
- **Especificar:** Crear/copiar/reordenar/eliminar página, condiciones, trigger y apariencia.
- **Preguntar si:** ventana modal, documento acoplable o panel del inspector.
- **Diseñar:** Draft transaccional o comandos inmediatos según decisión UX.
- **Dirigir:**
  1. **EVT-11.1** — **[2 d]** abrir por EventId.
  2. **EVT-11.2** — **[2 d]** tabs de página.
  3. **EVT-11.3** — **[2 d]** editar condiciones/trigger.
  4. **EVT-11.4** — **[1,5 d]** copy/paste.
  5. **EVT-11.5** — **[1,5 d]** preview activa.
- **Definición/pseudocódigo:**

```text
selectedPage=event.pages[id]; drawConditions(selectedPage); execute(UpdateEventPage{before,after});
```

- **Revisar:** Borrar página activa y referencias; undo de operaciones.
- **Criterio de aceptación:** Crear cofre de dos páginas exclusivamente desde editor.

### EVT-12 — Crear editor vertical de comandos (10 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **10 de 10 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** El editor vertical de comandos es el paquete más caro del roadmap (10 j.) y no tiene nada hecho. Es LA pieza de 'sin tocar C++'.

- **Arquitectura:** EDT / EVT — secuencia legible y estructurada.
- **Dependencias:** EVT-11.
- **Archivos:** + `editor/.../EventCommandList.h`; + `EventCommandPalette.h`; + `CommandEditors/*`; + commands/tests.
- **Especificar:** Insertar, editar, mover, seleccionar múltiples, ramas y búsqueda.
- **Preguntar si:** drag/drop, atajos y representación gráfica de ramas requerida.
- **Diseñar:** Árbol lineal estructurado con IDs de nodo; renderer de fila por typeId.
- **Dirigir:**
  1. **EVT-12.1** — **[2 d]** modelo selección.
  2. **EVT-12.2** — **[2 d]** lista indentada.
  3. **EVT-12.3** — **[2 d]** paleta búsqueda.
  4. **EVT-12.4** — **[2 d]** editores tipos base.
  5. **EVT-12.5** — **[2 d]** clipboard/undo.
- **Definición/pseudocódigo:**

```text
for(node:flattenStructured(commands)) drawRow(node,indent(node)); onInsert(type)->factory.create(type);
```

- **Revisar:** Mover no rompe bloques; lista grande virtualizada.
- **Criterio de aceptación:** Construir la misión fixture y copiar un bloque completo con ramas.

### EVT-13 — Crear editor visual de rutas de movimiento (6 días)

> **Estado real:** `PARCIAL` — avance **15 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** EditorState::setObjectPatrol(); level_editor.cpp (tecla T)  
> **Cómo leer esta ficha:** Las patrullas se marcan con el rectángulo. Rutas con pasos ordenados no existen.

- **Arquitectura:** EDT / EVT / MAP — autoría espacial de secuencias.
- **Dependencias:** EVT-12.
- **Archivos:** + `include/Events/MovementRoute.h`; + `editor/.../MovementRouteEditor.*`; + viewport overlay/tests.
- **Especificar:** Pasos, orientación, wait, loop, preview y dibujo en mapa.
- **Preguntar si:** ruta absoluta, relativa o ambas y comportamiento ante obstáculo.
- **Diseñar:** Lista de órdenes data-driven; preview usa simulador aislado.
- **Dirigir:**
  1. **EVT-13.1** — **[1,5 d]** definir route.
  2. **EVT-13.2** — **[1,5 d]** editor lista.
  3. **EVT-13.3** — **[1 d]** dibujo overlay.
  4. **EVT-13.4** — **[1 d]** preview/cancel.
  5. **EVT-13.5** — **[1 d]** serializar.
- **Definición/pseudocódigo:**

```text
route=[Move(N),Move(E),Face(S),Wait(0.5)]; simulate(route,ghostEntity);
```

- **Revisar:** Preview no muta mundo/documento; rutas inválidas marcadas.
- **Criterio de aceptación:** Ruta dibujada reproduce exactamente la secuencia guardada.

### EVT-14 — Implementar validación estática de eventos (6 días)

> **Estado real:** `PARCIAL` — avance **35 %** · esfuerzo restante **4 de 6 jornadas**.  
> **En el repo hoy:** include/Editor/EditorValidation.h::checkRpg(); AUDITORIA_CONTRATOS.md  
> **Cómo leer esta ficha:** Ya valida referencias de skills, sprites, enemigos, pickups, NPC y transiciones. Ampliarlo a eventos es continuación, no obra nueva.

- **Arquitectura:** EVT / TOOL — diagnósticos antes de jugar.
- **Dependencias:** EVT-13.
- **Archivos:** + `include/Events/Validation/EventValidator.h`; + `src/Events/Validation/EventValidator.cpp`; + editor diagnostics/tests.
- **Especificar:** Referencias, ramas, loops, autorun, tipos y comandos desconocidos.
- **Preguntar si:** qué diagnósticos bloquean guardado/exportación frente a warning.
- **Diseñar:** Diagnóstico con código, severidad, ubicación y quick-fix opcional.
- **Dirigir:**
  1. **EVT-14.1** — **[1,5 d]** crear modelo diagnóstico.
  2. **EVT-14.2** — **[1,5 d]** validar estructura.
  3. **EVT-14.3** — **[1 d]** validar refs/tipos.
  4. **EVT-14.4** — **[1 d]** integrar panel.
  5. **EVT-14.5** — **[1 d]** fixtures.
- **Definición/pseudocódigo:**

```text
Diagnostic{code,severity,eventId,pageId,nodeId,message,fix?};
```

- **Revisar:** Sin falsos positivos en fixtures válidas; cada error navega a nodo.
- **Criterio de aceptación:** Evento defectuoso produce todos los códigos esperados y cero crashes.

### EVT-15 — Implementar depuración paso a paso de eventos (7 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Sin depurador paso a paso. Depende de EVT-03 rehecho con contexto.

- **Arquitectura:** TOOL / EVT / RUN — observabilidad del intérprete.
- **Dependencias:** EVT-14.
- **Archivos:** + `include/Events/Debug/EventDebugSession.h`; + `editor/.../EventDebuggerPanel.*`; + interpreter hooks/tests.
- **Especificar:** Pause, step, continue, breakpoints, stack, variables y traza.
- **Preguntar si:** debug remoto de juego exportado o solo playtest embebido.
- **Diseñar:** Hooks de depuración opcionales sin cambiar semántica.
- **Dirigir:**
  1. **EVT-15.1** — **[1,5 d]** añadir trace events.
  2. **EVT-15.2** — **[1,5 d]** breakpoints por nodeId.
  3. **EVT-15.3** — **[1,5 d]** panel stack/state.
  4. **EVT-15.4** — **[1,5 d]** step controls.
  5. **EVT-15.5** — **[1 d]** test traza.
- **Definición/pseudocódigo:**

```text
beforeExecute(node){ trace.emit(...); if(breakpoints.contains(node.id)) suspend(DebugBreak); }
```

- **Revisar:** Desactivar debug no penaliza significativamente; no altera timing lógico.
- **Criterio de aceptación:** Breakpoint detiene antes de un comando y Step ejecuta exactamente uno.

### EVT-16 — Validar misión de llave, puerta y cofre (5 días)

> **Estado real:** `PARCIAL` — avance **40 %** · esfuerzo restante **3 de 5 jornadas**.  
> **En el repo hoy:** assets/adventures/*.json; NarrativeEngine::ObjectiveDefinition  
> **Cómo leer esta ficha:** Hay misiones con objetivos en JSON escritas a mano; falta hacerlo íntegramente desde el editor, que es el criterio real.

- **Arquitectura:** INF / EVT / EDT / RUN — aceptación no-code.
- **Dependencias:** EVT-15.
- **Archivos:** + `assets/projects/event_slice/*`; + `tests/Acceptance/EventMission.md`; + traces/snapshots; ~ CI.
- **Especificar:** Estados inicial/final, diálogo, elección, llave, puerta, cofre y save.
- **Preguntar si:** texto/arte final o provisional y condición exacta de éxito.
- **Diseñar:** Misión construida desde editor y prueba automatizada por acciones.
- **Dirigir:**
  1. **EVT-16.1** — **[1 d]** crear NPC diálogo.
  2. **EVT-16.2** — **[1 d]** crear llave.
  3. **EVT-16.3** — **[1 d]** crear puerta multipágina.
  4. **EVT-16.4** — **[1 d]** crear cofre/recompensa.
  5. **EVT-16.5** — **[1 d]** save/load y recorrido alternativo.
- **Definición/pseudocódigo:**

```text
talk(); chooseAccept(); obtainKey(); openDoor(); openChest(); saveReload(); assertQuestStateComplete();
```

- **Revisar:** No scripts C++; probar intentar puerta sin llave y cofre repetido.
- **Criterio de aceptación:** Un creador reproduce la misión desde proyecto vacío siguiendo documentación.

## Fase 6 — Base de datos y sistemas de juego

### GDB-01 — Crear GameDatabase extensible (8 días)

> **Estado real:** `PARCIAL` — avance **65 %** · esfuerzo restante **3 de 8 jornadas**.  
> **En el repo hoy:** include/RPG/Catalogs/RpgCatalogs.h (Catalog<T>, CatalogLoader); include/Core/Resources/ICatalog.h; assets/catalogs/  
> **Cómo leer esta ficha:** Ya hay una GameDatabase de facto: catálogo genérico + 19 tipos de definición cargados de JSON. Falta el registro extensible y el índice inverso de referencias.

- **Arquitectura:** GDB / DOC — catálogo de contenido no espacial.
- **Dependencias:** EVT-16.
- **Archivos:** + `include/Database/GameDatabase.h`; + `include/Database/DataRecord.h`; + `src/Database/GameDatabase.cpp`; + serializers/schemas/tests.
- **Especificar:** Tipos de registro, IDs, CRUD, índices, referencias y extensiones.
- **Preguntar si:** base de datos por proyecto, módulos importables o ambas.
- **Diseñar:** Registros documentales tipados con registry y notas de plugin.
- **Dirigir:**
  1. **GDB-01.1** — **[2 d]** definir DataRecord.
  2. **GDB-01.2** — **[1,5 d]** crear registry de tipos.
  3. **GDB-01.3** — **[1,5 d]** CRUD e índices.
  4. **GDB-01.4** — **[1,5 d]** serializar/migrar.
  5. **GDB-01.5** — **[1,5 d]** validar refs.
- **Definición/pseudocódigo:**

```text
database.records[typeId][recordId]=record; resolve<T>(id)->Result<const T*>;
```

- **Revisar:** IDs únicos globales o por tipo según decisión; tipo desconocido preservado.
- **Criterio de aceptación:** Crear, guardar, cargar y resolver registros de dos tipos.

### GDB-02 — Crear editor maestro-detalle de registros (8 días)

> **Estado real:** `PENDIENTE` — avance **10 %** · esfuerzo restante **8 de 8 jornadas**.  
> **En el repo hoy:** include/Editor/ProjectHub.h (patrón de menús)  
> **Cómo leer esta ficha:** El editor maestro-detalle de la base de datos no existe. Hoy los catálogos se editan a mano en JSON.

- **Arquitectura:** EDT / GDB — autoría consistente de datos.
- **Dependencias:** GDB-01.
- **Archivos:** + `editor/.../DatabaseWindow.h`; + `RecordListPanel.*`; + `RecordInspectorRegistry.*`; + tests.
- **Especificar:** Categorías, lista, búsqueda, alta/baja, duplicar y detalle.
- **Preguntar si:** edición inmediata o Apply/Cancel; edición múltiple requerida.
- **Diseñar:** Inspector por record type y comandos undoables.
- **Dirigir:**
  1. **GDB-02.1** — **[2 d]** crear ventana/categorías.
  2. **GDB-02.2** — **[1,5 d]** lista virtualizada.
  3. **GDB-02.3** — **[1,5 d]** CRUD.
  4. **GDB-02.4** — **[1,5 d]** inspector registry.
  5. **GDB-02.5** — **[1,5 d]** búsqueda/diagnósticos.
- **Definición/pseudocódigo:**

```text
type=category.selected; rows=db.query(type,filter); inspector[type].draw(selected);
```

- **Revisar:** Eliminar usado muestra referencias; duplicar genera ID nuevo.
- **Criterio de aceptación:** Gestionar 1.000 registros con búsqueda fluida y reload exacto.

### GDB-03 — Implementar actores, NPC y enemigos (7 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **2 de 7 jornadas**.  
> **En el repo hoy:** RpgCoreDefinitions.h::NpcDefinition/MonsterDefinition; include/RPG/CharacterSheet.h; include/Game/EnemyBrain.h  
> **Cómo leer esta ficha:** Actores, NPC y enemigos con definición rica. Falta editarlos sin tocar JSON (GDB-02).

- **Arquitectura:** GDB / RUN / MAP — prototipos de entidades vivas.
- **Dependencias:** GDB-02.
- **Archivos:** + `include/Database/Records/ActorRecord.h`; + `NpcRecord.h`; + `EnemyRecord.h`; + inspectors; + runtime factories/tests.
- **Especificar:** Presentación, stats ref, comportamiento, facción y loot ref.
- **Preguntar si:** Actor jugador y NPC comparten schema base y qué distingue enemigo.
- **Diseñar:** Composición de record base + componentes de prototipo.
- **Dirigir:**
  1. **GDB-03.1** — **[1,5 d]** definir records.
  2. **GDB-03.2** — **[1,5 d]** crear inspectores.
  3. **GDB-03.3** — **[1,5 d]** factory World.
  4. **GDB-03.4** — **[1,5 d]** integrar placement.
  5. **GDB-03.5** — **[1 d]** tests.
- **Definición/pseudocódigo:**

```text
prototype=db.resolve<EntityPrototype>(id); entity=world.spawn(componentFactory.build(prototype));
```

- **Revisar:** No duplicar lógica entre NPC/enemy; referencias faltantes diagnosticadas.
- **Criterio de aceptación:** Tres records colocados generan entidades con componentes correctos.

### GDB-04 — Implementar atributos, estados y progresión (7 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **2 de 7 jornadas**.  
> **En el repo hoy:** include/RPG/Stat.h; include/RPG/TierRules.h; include/RPG/ConditionEngine.h; include/RPG/DicePoolEngine.h; include/Game/Morality.h  
> **Cómo leer esta ficha:** Atributos, estados, progresión por tiers y moralidad implementados y probados en demos.

- **Arquitectura:** GDB / RUN — modelo de estadísticas configurable.
- **Dependencias:** GDB-03.
- **Archivos:** + `include/Database/Records/StatDefinition.h`; + `StateRecord.h`; + `include/World/Systems/StatsSystem.h`; + implementations/editors/tests.
- **Especificar:** Stats base/derivados, modificadores, estados, duración y progresión.
- **Preguntar si:** usar sistema propio CON/DES/SAB/INT, niveles, tiers o configurar ambos.
- **Diseñar:** Definiciones data-driven y evaluador acíclico de fórmulas.
- **Dirigir:**
  1. **GDB-04.1** — **[1,5 d]** definir stats.
  2. **GDB-04.2** — **[1,5 d]** modificadores.
  3. **GDB-04.3** — **[1,5 d]** estados/duración.
  4. **GDB-04.4** — **[1,5 d]** progresión.
  5. **GDB-04.5** — **[1 d]** inspector/pruebas.
- **Definición/pseudocódigo:**

```text
effective(stat)=base + sum(flatMods); effective*=product(percentMods); derived=eval(formula,deps);
```

- **Revisar:** Detectar ciclos y división por cero; orden de modificadores documentado.
- **Criterio de aceptación:** Fixture calcula stats y aplica/expira estado con resultados exactos.

### GDB-05 — Implementar objetos, equipo e inventario (7 días)

> **Estado real:** `DONE` — avance **80 %** · esfuerzo restante **2 de 7 jornadas**.  
> **En el repo hoy:** RpgCoreDefinitions.h::EquipmentDefinition/ConsumableDefinition; include/RPG/InventoryEngine.h; include/RPG/Defense.h  
> **Cómo leer esta ficha:** Objetos, equipo e inventario con reglas de defensa.

- **Arquitectura:** GDB / RUN / SAVE — posesión y equipamiento.
- **Dependencias:** GDB-04.
- **Archivos:** + `ItemRecord.h`; + `EquipmentRecord.h`; + `InventoryComponent.h`; + `InventorySystem.*`; + inspectors/tests.
- **Especificar:** Stack, capacidad, categorías, slots, equipar y persistir.
- **Preguntar si:** inventario limitado por peso, slots o ilimitado en 1.0.
- **Diseñar:** Servicios transaccionales con resultado; record separado de instancia.
- **Dirigir:**
  1. **GDB-05.1** — **[1,5 d]** records item/equipo.
  2. **GDB-05.2** — **[1,5 d]** inventory ops.
  3. **GDB-05.3** — **[1,5 d]** equip slots.
  4. **GDB-05.4** — **[1,5 d]** modificadores stats.
  5. **GDB-05.5** — **[1 d]** save/editor.
- **Definición/pseudocódigo:**

```text
transaction.begin(); remove(item); equip(slot,itemInstance); applyModifiers(); transaction.commit();
```

- **Revisar:** Equipar fallo revierte; stacks e IDs de instancias correctos.
- **Criterio de aceptación:** Recoger, apilar, equipar, desequipar y cargar conserva estado.

### GDB-06 — Implementar habilidades y efectos (7 días)

> **Estado real:** `DONE` — avance **85 %** · esfuerzo restante **2 de 7 jornadas**.  
> **En el repo hoy:** include/RPG/Definitions/SkillDefinition.h; src/RPG/SkillExecutor.cpp; include/RPG/EnhanceHooks.h; examples/demo_skills.cpp  
> **Cómo leer esta ficha:** Habilidades y efectos con ejecutor y ganchos de mejora.

- **Arquitectura:** GDB / RUN / EVT — acciones configurables.
- **Dependencias:** GDB-05.
- **Archivos:** + `SkillRecord.h`; + `EffectDefinition.h`; + `SkillSystem.*`; + editor effect list/tests.
- **Especificar:** Coste, target, cooldown, fórmula, efectos y evento asociado.
- **Preguntar si:** sistema de cartas del proyecto Onegai es objetivo inmediato o módulo posterior.
- **Diseñar:** Skill compone efectos tipados ejecutados en contexto.
- **Dirigir:**
  1. **GDB-06.1** — **[1,5 d]** definir scope/cost.
  2. **GDB-06.2** — **[1,5 d]** effect registry.
  3. **GDB-06.3** — **[1,5 d]** resolver target.
  4. **GDB-06.4** — **[1,5 d]** aplicar efectos.
  5. **GDB-06.5** — **[1 d]** editor/tests.
- **Definición/pseudocódigo:**

```text
if(canPay&&targeting.valid){ pay(); for(effect:skill.effects) effects.execute(effect,ctx); }
```

- **Revisar:** Validar target/cost antes de mutar; RNG inyectable.
- **Criterio de aceptación:** Habilidad fixture daña, aplica estado y consume recurso determinísticamente.

### GDB-07 — Implementar combate básico configurable (10 días)

> **Estado real:** `PARCIAL` — avance **70 %** · esfuerzo restante **4 de 10 jornadas**.  
> **En el repo hoy:** include/Game/BattleState.h; src/Game/BattleState.cpp; examples/demo_battle_nd6.cpp; reglamento Ed.3 (commits b89759f, 0a2a172)  
> **Cómo leer esta ficha:** Combate enfrentado nd6 cerrado y validado. Lo que falta es que sea CONFIGURABLE por el creador, no fijo en C++.

- **Arquitectura:** RUN / GDB / EVT / UI — bucle de conflicto jugable.
- **Dependencias:** GDB-06.
- **Archivos:** + `include/Combat/CombatState.h`; + `CombatSystem.h`; + `src/Combat/*`; + records/editor/runtime UI/tests.
- **Especificar:** Inicio, turnos/tiempo, acciones, victoria, derrota y recompensas.
- **Preguntar si:** combate táctico en mapa, pantalla separada, turnos o sistema de cartas.
- **Diseñar:** Esta tarea queda BLOCKED hasta decisión; motor de estados independiente de presentación.
- **Dirigir:**
  1. **GDB-07.1** — **[2 d]** cerrar reglas en ADR.
  2. **GDB-07.2** — **[2 d]** definir state machine.
  3. **GDB-07.3** — **[2 d]** resolver iniciativa/turno.
  4. **GDB-07.4** — **[2 d]** acciones y efectos.
  5. **GDB-07.5** — **[2 d]** fin/recompensa/UI mínima.
- **Definición/pseudocódigo:**

```text
state=Start; while(!finished){ actor=initiative.next(); action=input.choose(actor); resolve(action); checkEnd(); }
```

- **Revisar:** Simulación sin render; seed reproducible; empate y huida definidos.
- **Criterio de aceptación:** Combate fixture completo se simula en test y coincide con log esperado.

### GDB-08 — Implementar misiones y objetivos (8 días)

> **Estado real:** `PARCIAL` — avance **65 %** · esfuerzo restante **3 de 8 jornadas**.  
> **En el repo hoy:** NarrativeEngine.h::ObjectiveDefinition/ObjectiveStatus/TimerDefinition  
> **Cómo leer esta ficha:** Objetivos y temporizadores hechos; falta diario de misiones y edición visual.

- **Arquitectura:** GDB / EVT / SAVE / EDT — progresión narrativa.
- **Dependencias:** GDB-07.
- **Archivos:** + `QuestRecord.h`; + `QuestState.h`; + `QuestSystem.*`; + `editor/.../QuestEditor.*`; + event commands/tests.
- **Especificar:** Estados, objetivos, condiciones, dependencias, recompensas y ocultos.
- **Preguntar si:** estructura lineal, árbol o grafo y si objetivos pueden retroceder.
- **Diseñar:** Definición documental + estado de partida; eventos emiten señales.
- **Dirigir:**
  1. **GDB-08.1** — **[2 d]** record/state.
  2. **GDB-08.2** — **[1,5 d]** objetivos tipados.
  3. **GDB-08.3** — **[1,5 d]** signal evaluation.
  4. **GDB-08.4** — **[1,5 d]** comandos/eventos.
  5. **GDB-08.5** — **[1,5 d]** editor/tests.
- **Definición/pseudocódigo:**

```text
onSignal(s){ for(q:active) for(obj:pending) if(obj.matches(s)) advance(obj); if(all) complete(q); }
```

- **Revisar:** No polling global caro; recompensa idempotente.
- **Criterio de aceptación:** Misión de tres objetivos progresa y persiste sin doble recompensa.

### GDB-09 — Implementar diálogos reutilizables (6 días)

> **Estado real:** `PARCIAL` — avance **50 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** ObjectCatalog.h::DialogueData; assets/adventures/  
> **Cómo leer esta ficha:** Diálogos por objeto y por aventura. No hay árbol de diálogo reutilizable entre NPC.

- **Arquitectura:** GDB / EVT / EDT — contenido narrativo compartido.
- **Dependencias:** GDB-08.
- **Archivos:** + `DialogueRecord.h`; + `DialogueNode.h`; + `editor/.../DialogueEditor.*`; + event adapter/tests.
- **Especificar:** Nodos, opciones, condiciones, saltos, speaker y localización.
- **Preguntar si:** editor vertical, árbol o grafo como modo principal.
- **Diseñar:** Grafo validado referenciado por comando de evento.
- **Dirigir:**
  1. **GDB-09.1** — **[1,5 d]** definir record/nodes.
  2. **GDB-09.2** — **[1,5 d]** resolver traversal.
  3. **GDB-09.3** — **[1 d]** editor elegido.
  4. **GDB-09.4** — **[1 d]** integrar comando.
  5. **GDB-09.5** — **[1 d]** validator.
- **Definición/pseudocódigo:**

```text
node=current; show(node.text); choice=select(validOptions); current=choice.targetNodeId;
```

- **Revisar:** Detectar nodos inaccesibles/ciclos no intencionales/referencias rotas.
- **Criterio de aceptación:** Dos NPC reutilizan diálogo con variables de contexto distintas.

### GDB-10 — Implementar tablas de loot y encuentros (6 días)

> **Estado real:** `PARCIAL` — avance **55 %** · esfuerzo restante **3 de 6 jornadas**.  
> **En el repo hoy:** RpgCoreDefinitions.h::LootTableDefinition; EditorState::setObjectLootTable/addObjectLoot  
> **Cómo leer esta ficha:** Tablas de botín definidas y asignables por instancia. Tablas de encuentro, no.

- **Arquitectura:** GDB / MAP / RUN — selección ponderada reproducible.
- **Dependencias:** GDB-09.
- **Archivos:** + `LootTableRecord.h`; + `EncounterTableRecord.h`; + `WeightedTableResolver.*`; + editors/tests.
- **Especificar:** Peso, cantidad, condiciones, nested tables y RNG.
- **Preguntar si:** protección contra mala suerte y escalado por región/nivel.
- **Diseñar:** Resolver puro con RNG inyectado y límite de recursión.
- **Dirigir:**
  1. **GDB-10.1** — **[1,5 d]** tabla genérica.
  2. **GDB-10.2** — **[1,5 d]** loot entries.
  3. **GDB-10.3** — **[1 d]** encounter entries.
  4. **GDB-10.4** — **[1 d]** editores.
  5. **GDB-10.5** — **[1 d]** tests estadísticos/deterministas.
- **Definición/pseudocódigo:**

```text
roll=random(0,totalWeight); entry=first(prefixWeight>roll); return resolve(entry);
```

- **Revisar:** Peso cero/negativo, tabla vacía y ciclos diagnosticados.
- **Criterio de aceptación:** Seeds conocidos producen resultados exactos y distribución smoke razonable.

### GDB-11 — Implementar audio, música y ambiente (6 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** NO HAY AUDIO en el repositorio. Requiere elegir y vendorizar una dependencia; con el entorno sin red, es una decisión de infraestructura (ver INF-04).

- **Arquitectura:** RUN / GDB / EVT / TOOL — reproducción referenciada por AssetId.
- **Dependencias:** GDB-10.
- **Archivos:** + `include/Audio/AudioSystem.h`; + `src/Audio/AudioSystem.cpp`; + audio backend config; + map settings/event commands/tests.
- **Especificar:** BGM, SFX, ambience, buses, volumen, fade y streaming.
- **Preguntar si:** backend y formatos/licencias obligatorios.
- **Diseñar:** AudioSystem interface + backend; comandos async para fades.
- **Dirigir:**
  1. **GDB-11.1** — **[1,5 d]** seleccionar backend.
  2. **GDB-11.2** — **[1,5 d]** buses y assets.
  3. **GDB-11.3** — **[1 d]** play/stop/fade.
  4. **GDB-11.4** — **[1 d]** map ambience.
  5. **GDB-11.5** — **[1 d]** tests fake.
- **Definición/pseudocódigo:**

```text
audio.play(Bus::BGM,asset,{loop:true,fadeIn}); onMapChange crossfade(next);
```

- **Revisar:** Asset ausente no crashea; volumen y dispositivos se recuperan.
- **Criterio de aceptación:** Cambiar mapas hace crossfade y evento reproduce SFX.

### GDB-12 — Implementar título, menús y carga de partida (8 días)

> **Estado real:** `PARCIAL` — avance **40 %** · esfuerzo restante **5 de 8 jornadas**.  
> **En el repo hoy:** include/RPG/CharacterCreationScreen.h; include/Render/HudManager.h; include/Editor/ProjectHub.h  
> **Cómo leer esta ficha:** Creación de personaje y menús HUD existen. Carga de partida imposible hasta RUN-10.

- **Arquitectura:** RUN / UI / SAVE / GDB — shell jugable del juego exportado.
- **Dependencias:** GDB-11.
- **Archivos:** + `include/UI/GameUi/*`; + `src/UI/*`; + `Database/UiThemeRecord.h`; + editors/tests.
- **Especificar:** Title, new/load, pause, inventory, status, options y game over.
- **Preguntar si:** UI hardcoded inicial o diseñador visual desde 1.0.
- **Diseñar:** Pantallas como escenas/UI states configuradas por theme record.
- **Dirigir:**
  1. **GDB-12.1** — **[2 d]** title/load.
  2. **GDB-12.2** — **[1,5 d]** pause/options.
  3. **GDB-12.3** — **[1,5 d]** inventory/status.
  4. **GDB-12.4** — **[1,5 d]** game over.
  5. **GDB-12.5** — **[1,5 d]** navegación input.
- **Definición/pseudocódigo:**

```text
UiStateMachine: Title -> NewGame|Load -> Gameplay <-> Pause -> GameOver;
```

- **Revisar:** Teclado/mando, foco, resoluciones y save corrupto.
- **Criterio de aceptación:** Ejecutable arranca en título, carga slot y permite usar inventario.

### GDB-13 — Validar aventura completa de treinta minutos (7 días)

> **Estado real:** `PARCIAL` — avance **50 %** · esfuerzo restante **4 de 7 jornadas**.  
> **En el repo hoy:** examples/demo_campana_boundington.cpp; examples/demo_aventura.cpp; GDD_MG.md  
> **Cómo leer esta ficha:** Ya hay campaña larga jugable, pero creada con código y JSON a mano, no desde el editor.

- **Arquitectura:** INF / GDB / EVT / MAP / RUN — alpha funcional.
- **Dependencias:** GDB-12.
- **Archivos:** + `assets/projects/creator_alpha/*`; + `tests/Acceptance/CreatorAlpha.md`; + telemetry/logs; ~ docs/CI.
- **Especificar:** Guion con exploración, diálogo, misión, combate, loot, save y final.
- **Preguntar si:** tema, reglas y assets aceptados para el juego muestra.
- **Diseñar:** Contenido creado solo con herramientas disponibles; lista de bloqueos UX.
- **Dirigir:**
  1. **GDB-13.1** — **[1,5 d]** escribir guion.
  2. **GDB-13.2** — **[1,5 d]** crear contenido.
  3. **GDB-13.3** — **[1,5 d]** playthrough feliz.
  4. **GDB-13.4** — **[1,5 d]** rutas de error.
  5. **GDB-13.5** — **[1 d]** perfil/rendimiento.
- **Definición/pseudocódigo:**

```text
newGame(); completeQuest(); winCombat(); loot(); saveReload(); reachEnding(); assertNoErrors();
```

- **Revisar:** Sesión externa con técnico; ninguna edición manual de JSON/C++.
- **Criterio de aceptación:** Aventura se crea, exporta internamente y completa en aproximadamente 30 minutos.

## Fase 7 — Playtest, extensiones y exportación

### TOOL-01 — Implementar playtest desde el editor (6 días)

> **Estado real:** `PENDIENTE` — avance **10 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** examples/level_editor.cpp; examples/juego.cpp  
> **Cómo leer esta ficha:** Editor y juego son dos ejecutables sin puente. Lanzar el juego desde el editor está por hacer.

- **Arquitectura:** TOOL / EDT / RUN — ejecución del proyecto activo.
- **Dependencias:** GDB-13.
- **Archivos:** + `editor/include/Editor/Playtest/PlaytestService.h`; + `editor/src/Playtest/PlaytestService.cpp`; + IPC/process helpers; + tests.
- **Especificar:** Guardar snapshot, lanzar, detener, logs y aislamiento.
- **Preguntar si:** playtest embebido en viewport o proceso separado.
- **Diseñar:** Proceso separado recomendado; manifest temporal y canal de control.
- **Dirigir:**
  1. **TOOL-01.1** — **[1,5 d]** crear snapshot validado.
  2. **TOOL-01.2** — **[1,5 d]** lanzar runtime.
  3. **TOOL-01.3** — **[1 d]** stream logs.
  4. **TOOL-01.4** — **[1 d]** stop/restart.
  5. **TOOL-01.5** — **[1 d]** limpiar temporales.
- **Definición/pseudocódigo:**

```text
snapshot=workspace.freeze(); process=launch(runtime,--playtest,snapshot); bridge.attach(process);
```

- **Revisar:** Crash del juego no derriba editor; cambios no guardados incluidos según decisión.
- **Criterio de aceptación:** Botón Play ejecuta proyecto actual y Stop lo termina limpiamente.

### TOOL-02 — Implementar probar desde la celda seleccionada (4 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **4 de 4 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Depende de TOOL-01.

- **Arquitectura:** TOOL / EDT / RUN — contexto de arranque temporal.
- **Dependencias:** TOOL-01.
- **Archivos:** + `include/Engine/Playtest/PlaytestOverrides.h`; ~ PlaytestService; ~ ViewportPanel; + tests.
- **Especificar:** Mapa, celda, orientación, party y estado temporal.
- **Preguntar si:** override ignora punto inicial sin modificar proyecto y qué pasa en celda bloqueada.
- **Diseñar:** Archivo de overrides efímero consumido solo en modo playtest.
- **Dirigir:**
  1. **TOOL-02.1** — **[1 d]** capturar selección.
  2. **TOOL-02.2** — **[1 d]** validar spawn temporal.
  3. **TOOL-02.3** — **[1 d]** pasar override.
  4. **TOOL-02.4** — **[1 d]** mostrar error inline.
- **Definición/pseudocódigo:**

```text
overrides.start={mapId,grid,facing}; runtime.applyOnlyIf(playtestMode);
```

- **Revisar:** Nunca persistir override en Project/GameState.
- **Criterio de aceptación:** Probar aquí inicia en celda y el proyecto permanece sin cambios.

### TOOL-03 — Implementar recarga en caliente segura (6 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** include/Core/Resources/ResourceManager.h  
> **Cómo leer esta ficha:** Sin recarga en caliente. El ResourceManager es el punto de enganche.

- **Arquitectura:** TOOL / RUN / DOC — aplicar cambios compatibles durante playtest.
- **Dependencias:** TOOL-02.
- **Archivos:** + `include/Engine/HotReload/ChangeSet.h`; + `HotReloadService.h`; + editor bridge; + tests.
- **Especificar:** Tipos de cambio hot, restart required y rollback.
- **Preguntar si:** qué documentos deben recargarse sin reiniciar en 1.0.
- **Diseñar:** Diff por IDs; cada subsystem declara capacidad de apply.
- **Dirigir:**
  1. **TOOL-03.1** — **[1,5 d]** calcular changeset.
  2. **TOOL-03.2** — **[1,5 d]** clasificar cambios.
  3. **TOOL-03.3** — **[1 d]** enviar por IPC.
  4. **TOOL-03.4** — **[1 d]** aplicar transacción.
  5. **TOOL-03.5** — **[1 d]** rollback/error.
- **Definición/pseudocódigo:**

```text
changes=diff(old,new); if(all canApply) transaction.apply(changes); else requestRestart(reasons);
```

- **Revisar:** Cambio fallido conserva runtime previo; mensaje lista motivos.
- **Criterio de aceptación:** Texto/tile se actualiza en playtest; schema change pide reinicio.

### TOOL-04 — Crear consola e inspector de estado (6 días)

> **Estado real:** `PARCIAL` — avance **20 %** · esfuerzo restante **5 de 6 jornadas**.  
> **En el repo hoy:** include/Render/HudWidgets.h; salida de consola de los demos  
> **Cómo leer esta ficha:** Los demos imprimen estado, pero no hay consola ni inspector en vivo.

- **Arquitectura:** TOOL / EDT / RUN — observabilidad unificada.
- **Dependencias:** TOOL-03.
- **Archivos:** + `include/Core/Diagnostics/LogEvent.h`; + log sink; + `editor/.../ConsolePanel.*`; + `RuntimeInspectorPanel.*`; + tests.
- **Especificar:** Niveles, categorías, búsqueda, navegación, variables, entidades y rendimiento.
- **Preguntar si:** edición de estado runtime desde inspector permitida.
- **Diseñar:** Canal estructurado, no parseo de strings; snapshots paginados.
- **Dirigir:**
  1. **TOOL-04.1** — **[1,5 d]** logging estructurado.
  2. **TOOL-04.2** — **[1,5 d]** IPC logs.
  3. **TOOL-04.3** — **[1 d]** panel filtros.
  4. **TOOL-04.4** — **[1 d]** inspector state.
  5. **TOOL-04.5** — **[1 d]** link a fuente.
- **Definición/pseudocódigo:**

```text
emit(LogEvent{time,level,category,code,context,message}); console.query(filter);
```

- **Revisar:** Volumen alto no bloquea; datos sensibles/rutas controlados.
- **Criterio de aceptación:** Error de evento aparece filtrable y navega al nodo responsable.

### TOOL-05 — Crear gestor completo de recursos y referencias (8 días)

> **Estado real:** `PARCIAL` — avance **30 %** · esfuerzo restante **6 de 8 jornadas**.  
> **En el repo hoy:** include/Core/Resources/ResourceManager.h; assets/generated/authoring/asset_manifest.json; include/Editor/EditorValidation.h  
> **Cómo leer esta ficha:** Manifiesto de assets y validación de referencias ya existen: es la base del gestor.

- **Arquitectura:** TOOL / DOC / EDT — ciclo de vida de assets.
- **Dependencias:** TOOL-04.
- **Archivos:** + `include/Project/AssetRegistry.h`; + `AssetReferenceIndex.h`; + import/reimport services; + `editor/.../ResourceManagerWindow.*`; + tests.
- **Especificar:** Importar, mover, renombrar, reemplazar, referencias, duplicados y unused.
- **Preguntar si:** assets externos enlazados o siempre copiados al proyecto.
- **Diseñar:** AssetId estable; path mutable; índice inverso incremental.
- **Dirigir:**
  1. **TOOL-05.1** — **[2 d]** registry persistente.
  2. **TOOL-05.2** — **[1,5 d]** hash/metadata.
  3. **TOOL-05.3** — **[1,5 d]** índice refs.
  4. **TOOL-05.4** — **[1,5 d]** operaciones seguras.
  5. **TOOL-05.5** — **[1,5 d]** UI/preview.
- **Definición/pseudocódigo:**

```text
replace(assetId,newFile){ importTemp; validate; swapPathKeepId; rebuildDependents; }
```

- **Revisar:** Eliminar usado bloquea y lista consumidores; operación atómica.
- **Criterio de aceptación:** Reemplazar imagen conserva referencias de todos los mapas.

### TOOL-06 — Diseñar API de plugins de runtime (7 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Sin API de plugins. Decidir antes si el objetivo 1.0 la necesita o se pospone a 1.1.

- **Arquitectura:** TOOL / RUN / EVT / GDB — extensión con frontera estable.
- **Dependencias:** TOOL-05.
- **Archivos:** + `include/Plugins/RuntimePluginApi.h`; + `PluginManifest.h`; + `docs/plugins/runtime-api.md`; + sample plugin/tests.
- **Especificar:** ABI/API, registro de componentes/comandos/records, lifecycle y compatibilidad.
- **Preguntar si:** plugins C++ nativos, scripting seguro o ambos; esta decisión bloquea implementación.
- **Diseñar:** Preferir C ABI estable o proceso/scripting; no exponer clases internas sin versión.
- **Dirigir:**
  1. **TOOL-06.1** — **[1,5 d]** ADR de tecnología.
  2. **TOOL-06.2** — **[1,5 d]** manifest/versiones.
  3. **TOOL-06.3** — **[1,5 d]** host API mínima.
  4. **TOOL-06.4** — **[1,5 d]** sample plugin.
  5. **TOOL-06.5** — **[1 d]** compat tests.
- **Definición/pseudocódigo:**

```text
plugin_init(const HostApiV1* host, PluginHandle* out); plugin_shutdown(handle);
```

- **Revisar:** Plugin incompatible falla antes de ejecutar; crash isolation según modelo.
- **Criterio de aceptación:** Plugin muestra comando de evento nuevo sin modificar motor.

### TOOL-07 — Diseñar API de extensiones de editor (7 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Ídem TOOL-06.

- **Arquitectura:** TOOL / EDT — paneles, importadores y validadores.
- **Dependencias:** TOOL-06.
- **Archivos:** + `editor/include/Editor/Plugins/EditorExtensionApi.h`; + docs; + sample extension/tests.
- **Especificar:** Registrar panel, tool, inspector, importer, validator y command editor.
- **Preguntar si:** extensión comparte proceso y Dear ImGui directo o UI declarativa.
- **Diseñar:** Contexto limitado y registros versionados; documentos vía servicios.
- **Dirigir:**
  1. **TOOL-07.1** — **[1,5 d]** definir extension points.
  2. **TOOL-07.2** — **[1,5 d]** registro/lifecycle.
  3. **TOOL-07.3** — **[1,5 d]** sample panel/importer.
  4. **TOOL-07.4** — **[1,5 d]** reload policy.
  5. **TOOL-07.5** — **[1 d]** tests.
- **Definición/pseudocódigo:**

```text
extension.registerPanel(desc,factory); extension.registerValidator(typeId,validator);
```

- **Revisar:** Extensión no obtiene puntero mutable directo a Project.
- **Criterio de aceptación:** Extensión ejemplo añade panel y validador reversible.

### TOOL-08 — Implementar permisos, versiones y migraciones de plugins (6 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Ídem TOOL-06.

- **Arquitectura:** TOOL / DOC / Security — gobernanza de extensiones.
- **Dependencias:** TOOL-07.
- **Archivos:** + `include/Plugins/PluginManager.h`; + `PluginPermissions.h`; + migration registry; + editor UI/tests.
- **Especificar:** Dependencias, rangos SemVer, permisos, enable/disable y datos desconocidos.
- **Preguntar si:** política de confianza, firma y acceso a filesystem/red.
- **Diseñar:** Manifest declara permisos; host pide consentimiento y conserva datos al faltar plugin.
- **Dirigir:**
  1. **TOOL-08.1** — **[1,5 d]** resolver dependencias.
  2. **TOOL-08.2** — **[1,5 d]** comprobar API version.
  3. **TOOL-08.3** — **[1 d]** permission gate.
  4. **TOOL-08.4** — **[1 d]** migraciones.
  5. **TOOL-08.5** — **[1 d]** missing-plugin placeholders.
- **Definición/pseudocódigo:**

```text
load(manifest){ resolveDeps(); checkApi(); requireGranted(perms); migrateData(); init(); }
```

- **Revisar:** No quitar guardas para hacer funcionar plugin; disable no pierde datos.
- **Criterio de aceptación:** Plugin sin permiso no accede a archivo y proyecto abre con placeholder si falta.

### TOOL-09 — Implementar empaquetado de recursos usados (6 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **6 de 6 jornadas**.  
> **En el repo hoy:** assets/generated/authoring/asset_manifest.json  
> **Cómo leer esta ficha:** El manifiesto permitiría calcular los recursos usados; el empaquetado no existe.

- **Arquitectura:** TOOL / DOC — build reproducible y mínimo.
- **Dependencias:** TOOL-08.
- **Archivos:** + `include/Build/ProjectPackager.h`; + `src/Build/ProjectPackager.cpp`; + build manifest/schema/tests.
- **Especificar:** Grafo de dependencias, assets dinámicos, plugins y determinismo.
- **Preguntar si:** incluir recursos no referenciados marcados keep y contenido DLC.
- **Diseñar:** Recorrido desde roots con manifest y hashes.
- **Dirigir:**
  1. **TOOL-09.1** — **[1,5 d]** construir dependency graph.
  2. **TOOL-09.2** — **[1,5 d]** marcar roots.
  3. **TOOL-09.3** — **[1 d]** resolver transitivas.
  4. **TOOL-09.4** — **[1 d]** copiar/atlas.
  5. **TOOL-09.5** — **[1 d]** emitir manifest.
- **Definición/pseudocódigo:**

```text
used=graph.reachable(project.roots + keepMarked); package.copyDeterministic(used);
```

- **Revisar:** Referencias dinámicas declaradas; dos builds idénticos tienen hashes iguales.
- **Criterio de aceptación:** Asset no usado queda fuera y todos los usados cargan en paquete.

### TOOL-10 — Implementar exportación Windows y Linux (8 días)

> **Estado real:** `PARCIAL` — avance **20 %** · esfuerzo restante **7 de 8 jornadas**.  
> **En el repo hoy:** .github/workflows/release.yml; CMakePresets.json  
> **Cómo leer esta ficha:** Hay flujo de release y presets, pero exporta el motor, no un juego del usuario.

- **Arquitectura:** TOOL / INF / RUN — producto ejecutable standalone.
- **Dependencias:** TOOL-09.
- **Archivos:** + `tools/export/`; + CMake install rules; + `.github/workflows/export-smoke.yml`; + runtime bootstrap/tests.
- **Especificar:** Layouts, runtime libs, configuración Release, iconos y licencias.
- **Preguntar si:** cross-compilation, macOS y formatos instalador/zip.
- **Diseñar:** Exporter por plataforma consume paquete neutral.
- **Dirigir:**
  1. **TOOL-10.1** — **[2 d]** definir ExportTarget.
  2. **TOOL-10.2** — **[1,5 d]** Windows bundle.
  3. **TOOL-10.3** — **[1,5 d]** Linux bundle.
  4. **TOOL-10.4** — **[1,5 d]** smoke launch.
  5. **TOOL-10.5** — **[1,5 d]** licencias.
- **Definición/pseudocódigo:**

```text
bundle=pack(project); exporter[target].emit(runtime,bundle,plugins,licenses,outDir);
```

- **Revisar:** Máquina limpia sin build tools; rutas relativas y DLL/so completas.
- **Criterio de aceptación:** Artefactos Windows/Linux arrancan y completan smoke de título→mapa.

### TOOL-11 — Implementar diagnósticos de exportación (5 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **5 de 5 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Depende de TOOL-09/TOOL-10.

- **Arquitectura:** TOOL / EDT / DOC — preflight accionable.
- **Dependencias:** TOOL-10.
- **Archivos:** + `include/Build/ExportValidator.h`; + diagnostics catalog; + `editor/.../ExportDialog.*`; + tests.
- **Especificar:** Errores bloqueantes, warnings, navegación y resumen de tamaño.
- **Preguntar si:** warnings pueden tratarse como error en modo estricto.
- **Diseñar:** Validadores por dominio agregan diagnósticos con ubicación.
- **Dirigir:**
  1. **TOOL-11.1** — **[1 d]** preflight pipeline.
  2. **TOOL-11.2** — **[1 d]** integrar validadores.
  3. **TOOL-11.3** — **[1 d]** UI progreso/resultados.
  4. **TOOL-11.4** — **[1 d]** links/quick fixes.
  5. **TOOL-11.5** — **[1 d]** tests.
- **Definición/pseudocódigo:**

```text
report=validators.run(snapshot,target); if(report.errors) blockExport(); else exporter.run();
```

- **Revisar:** Cada fallo común explica solución; no empezar export antes de validar.
- **Criterio de aceptación:** Proyecto con asset roto se bloquea y navega a referencia exacta.

### TOOL-12 — Publicar alpha del creador de juegos (3 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **3 de 3 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Hito de publicación.

- **Arquitectura:** REL / TOOL — hito controlado para testers.
- **Dependencias:** TOOL-11.
- **Archivos:** + `CHANGELOG.md`; + `docs/releases/alpha-checklist.md`; ~ release workflow; + issue templates.
- **Especificar:** Scope alpha, plataformas, datos soportados, known issues y feedback.
- **Preguntar si:** grupo de testers, canal de distribución y telemetría permitida.
- **Diseñar:** Release versionada con proyecto muestra y formulario reproducible.
- **Dirigir:**
  1. **TOOL-12.1** — **[1 d]** freeze branch.
  2. **TOOL-12.2** — **[0,5 d]** ejecutar full suite.
  3. **TOOL-12.3** — **[0,5 d]** crear artefactos.
  4. **TOOL-12.4** — **[0,5 d]** publicar notas.
  5. **TOOL-12.5** — **[0,5 d]** abrir feedback triage.
- **Definición/pseudocódigo:**

```text
if(allReleaseGatesPass) tag(v0.8.0-alpha.1); publish(artifacts,notes,sample);
```

- **Revisar:** No prometer compatibilidad 1.0; instrucciones backup visibles.
- **Criterio de aceptación:** Tester instala, abre muestra, crea cambio y exporta sin entorno dev.

## Fase 8 — Beta, documentación y versión 1.0

### REL-01 — Crear juego de ejemplo completo (10 días)

> **Estado real:** `PARCIAL` — avance **45 %** · esfuerzo restante **6 de 10 jornadas**.  
> **En el repo hoy:** assets/adventures/ (9 aventuras); GDD.md; GDD_MG.md; examples/demo_campana_boundington.cpp  
> **Cómo leer esta ficha:** El universo Boundington/Onegai está muy desarrollado como contenido. Falta rehacerlo con las herramientas públicas, que es el criterio de la ficha.

- **Arquitectura:** REL / todas — prueba de producto y documentación viva.
- **Dependencias:** TOOL-12.
- **Archivos:** + `samples/official-game/*`; + source assets/license; + build/test script.
- **Especificar:** Duración, sistemas cubiertos, licencia y criterios de calidad.
- **Preguntar si:** ambientación Onegai o universo neutro redistribuible.
- **Diseñar:** Proyecto real, no fixture, con capítulos que demuestran features.
- **Dirigir:**
  1. **REL-01.1** — **[2 d]** guion y asset list.
  2. **REL-01.2** — **[2 d]** crear mundo/eventos.
  3. **REL-01.3** — **[2 d]** integrar combate/misiones.
  4. **REL-01.4** — **[2 d]** exportar.
  5. **REL-01.5** — **[2 d]** playthrough.
- **Definición/pseudocódigo:**

```text
sample.coverage = requiredFeatures; assert(playthrough.reachesEnding && noManualCode);
```

- **Revisar:** Todo debe abrir en editor y enseñar buenas prácticas.
- **Criterio de aceptación:** Juego muestra se completa y exporta usando solo APIs públicas.

### REL-02 — Crear tutorial interactivo del editor (7 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Sin tutorial interactivo.

- **Arquitectura:** REL / EDT — onboarding sin alterar documentos silenciosamente.
- **Dependencias:** REL-01.
- **Archivos:** + `editor/include/Editor/Tutorial/*`; + `tutorials/getting-started.json`; + UI/tests.
- **Especificar:** Pasos, highlights, progreso, reanudación y proyecto sandbox.
- **Preguntar si:** tutorial obligatorio al primer inicio y localización inicial.
- **Diseñar:** Tutorial data-driven observa acciones y valida objetivos.
- **Dirigir:**
  1. **REL-02.1** — **[1,5 d]** definir schema.
  2. **REL-02.2** — **[1,5 d]** overlay/highlight.
  3. **REL-02.3** — **[1,5 d]** detectores de acción.
  4. **REL-02.4** — **[1,5 d]** contenido inicial.
  5. **REL-02.5** — **[1 d]** test recorrido.
- **Definición/pseudocódigo:**

```text
step.waitUntil(ctx=>ctx.project.hasMap&&ctx.history.contains(PaintTiles)); advance();
```

- **Revisar:** Puede saltarse/reiniciarse; no bloquea UI por estado imprevisto.
- **Criterio de aceptación:** Usuario nuevo crea mapa, NPC y diálogo siguiendo tutorial.

### REL-03 — Documentar formatos, runtime y plugins (7 días)

> **Estado real:** `PARCIAL` — avance **60 %** · esfuerzo restante **3 de 7 jornadas**.  
> **En el repo hoy:** FORMATO_NIVELES.md; FORMATO_AVENTURAS.md; FORMATO_PROYECTOS.md; ARCHITECTURE.md; CONTRATO_IDS.md; SPRITE_AUTHORING_GUIDE.md  
> **Cómo leer esta ficha:** Documentación de formatos muy por delante de lo habitual. Falta la de runtime y plugins.

- **Arquitectura:** REL / DOC / RUN / TOOL — contratos públicos 1.0.
- **Dependencias:** REL-02.
- **Archivos:** + `docs/formats/*`; + `docs/runtime/*`; + `docs/plugins/*`; + API docs config; ~ README.
- **Especificar:** Audiencias, versiones, ejemplos y política de compatibilidad.
- **Preguntar si:** idiomas de documentación y generador oficial.
- **Diseñar:** Docs por tarea, how-to y referencia; ejemplos compilados en CI.
- **Dirigir:**
  1. **REL-03.1** — **[1,5 d]** mapear contratos.
  2. **REL-03.2** — **[1,5 d]** documentar schemas.
  3. **REL-03.3** — **[1,5 d]** runtime embedding.
  4. **REL-03.4** — **[1,5 d]** plugin tutorials.
  5. **REL-03.5** — **[1 d]** enlazar API.
- **Definición/pseudocódigo:**

```text
for(publicContract) assert(documented && exampleValidated && versionPolicyDeclared);
```

- **Revisar:** Comandos ejecutables; enlaces y snippets verificados.
- **Criterio de aceptación:** Desarrollador externo crea plugin de ejemplo solo con documentación.

### REL-04 — Realizar pruebas con proyectos grandes (7 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** Sin pruebas de carga con proyectos grandes.

- **Arquitectura:** REL / Performance — límites medidos.
- **Dependencias:** REL-03.
- **Archivos:** + `tests/Performance/*`; + `tools/generate_large_project.*`; + `docs/performance/budgets.md`; ~ CI scheduled.
- **Especificar:** Tamaños de mapas, assets, entidades, eventos y presupuesto.
- **Preguntar si:** hardware mínimo y objetivos FPS/latencia/memoria.
- **Diseñar:** Generador determinista y benchmarks separados editor/runtime.
- **Dirigir:**
  1. **REL-04.1** — **[1,5 d]** cerrar budgets.
  2. **REL-04.2** — **[1,5 d]** generar small/medium/large.
  3. **REL-04.3** — **[1,5 d]** medir apertura/paint/save/play.
  4. **REL-04.4** — **[1,5 d]** registrar baseline.
  5. **REL-04.5** — **[1 d]** crear alertas.
- **Definición/pseudocódigo:**

```text
benchmark(projectSize){ measure(load,pick,paint,save,frame,memory); assert<=budget; }
```

- **Revisar:** No usar máquina única como verdad sin contexto; separar cold/warm.
- **Criterio de aceptación:** Resultados reproducibles y regresiones superiores al umbral fallan.

### REL-05 — Realizar pruebas de usabilidad con creadores (8 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **8 de 8 jornadas**.  
> **En el repo hoy:** ESTUDIO_EDITOR_RPG.md  
> **Cómo leer esta ficha:** Hay estudio de mercado, no pruebas con creadores reales.

- **Arquitectura:** REL / EDT / Product — valida flujos humanos.
- **Dependencias:** REL-04.
- **Archivos:** + `docs/usability/test-plan.md`; + anonymized findings; + backlog mapping; ~ telemetry opt-in si se aprueba.
- **Especificar:** Perfiles, tareas, métricas, privacidad y éxito.
- **Preguntar si:** participantes disponibles y consentimiento de grabación/telemetría.
- **Diseñar:** Sesiones moderadas con guion, sin enseñar solución.
- **Dirigir:**
  1. **REL-05.1** — **[2 d]** reclutar perfiles.
  2. **REL-05.2** — **[1,5 d]** ejecutar tareas.
  3. **REL-05.3** — **[1,5 d]** medir tiempo/errores.
  4. **REL-05.4** — **[1,5 d]** sintetizar hallazgos.
  5. **REL-05.5** — **[1,5 d]** crear tareas priorizadas.
- **Definición/pseudocódigo:**

```text
for(participant){ observe(taskSet); record(blockers,errors,time); } prioritize(severity*frequency);
```

- **Revisar:** Distinguir opinión de bloqueo reproducible; no incluir datos personales.
- **Criterio de aceptación:** Top problemas tiene evidencia, severidad, owner y tarea asociada.

### REL-06 — Corregir bloqueos y pérdida de datos (10 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **10 de 10 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** 

- **Arquitectura:** REL / DOC / EDT — gate de fiabilidad.
- **Dependencias:** REL-05.
- **Archivos:** Según defectos; especialmente serializers, autosave, commands, importer; + regression tests.
- **Especificar:** Lista cerrada P0/P1, reproducción y riesgo de migración.
- **Preguntar si:** cualquier fix que cambie UX o formato no cubierto por especificación.
- **Diseñar:** Un fix por causa raíz con test antes/después; no paquete de limpieza.
- **Dirigir:**
  1. **REL-06.1** — **[2 d]** triage P0/P1.
  2. **REL-06.2** — **[2 d]** fixture reproductora.
  3. **REL-06.3** — **[2 d]** fix mínimo.
  4. **REL-06.4** — **[2 d]** regresión.
  5. **REL-06.5** — **[2 d]** backport/migration.
- **Definición/pseudocódigo:**

```text
reproduce(); assert(fails); applyFix(); assert(passes); run(fullDataIntegritySuite);
```

- **Revisar:** Cada incidente tiene análisis de causa y protección futura.
- **Criterio de aceptación:** Cero P0/P1 abiertos y suite de integridad pasa repetidamente.

### REL-07 — Optimizar mapas, entidades y editor (10 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **10 de 10 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** 

- **Arquitectura:** REL / REN / RUN / EDT — optimización guiada por perfiles.
- **Dependencias:** REL-06.
- **Archivos:** Según perfiles: `SpriteBatch`, renderer, índices, panels, serializers; + benchmarks.
- **Especificar:** Regresiones concretas y presupuesto; prohibido optimizar sin métrica.
- **Preguntar si:** trade-off memoria/latencia y hardware mínimo.
- **Diseñar:** Cambios aislados con baseline y comparación.
- **Dirigir:**
  1. **REL-07.1** — **[2 d]** perf profile.
  2. **REL-07.2** — **[2 d]** seleccionar cuellos.
  3. **REL-07.3** — **[2 d]** optimizar uno por commit.
  4. **REL-07.4** — **[2 d]** comparar calidad/correctitud.
  5. **REL-07.5** — **[2 d]** documentar.
- **Definición/pseudocódigo:**

```text
before=benchmark(); optimize(hotspot); after=benchmark(); assert(after.improvesTarget && correctnessSame);
```

- **Revisar:** No degradar mapas pequeños; snapshots y sanitizers.
- **Criterio de aceptación:** Budgets medium/large se cumplen sin cambios visuales o de datos.

### REL-08 — Preparar instalador y actualización (7 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **7 de 7 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** 

- **Arquitectura:** REL / Distribution — instalación segura.
- **Dependencias:** REL-07.
- **Archivos:** + `packaging/windows/*`; + `packaging/linux/*`; + updater/manifest; + signing/release workflows/tests.
- **Especificar:** Install/uninstall, rutas, asociaciones, update, rollback y firma.
- **Preguntar si:** tecnología de instalador, certificados y actualización automática.
- **Diseñar:** Manifest firmado/versionado; descarga staging y swap/rollback.
- **Dirigir:**
  1. **REL-08.1** — **[1,5 d]** crear paquetes.
  2. **REL-08.2** — **[1,5 d]** instalar limpio.
  3. **REL-08.3** — **[1,5 d]** updater check/download.
  4. **REL-08.4** — **[1,5 d]** verificar/finalizar.
  5. **REL-08.5** — **[1 d]** rollback/uninstall.
- **Definición/pseudocódigo:**

```text
download(temp); verify(hash,signature); stage(); swapAtomic(); if(healthFail) rollback();
```

- **Revisar:** No borrar proyectos de usuario; probar sin permisos admin cuando aplique.
- **Criterio de aceptación:** Actualizar alpha→RC conserva proyectos y rollback funciona.

### REL-09 — Congelar esquema 1.0 y compatibilidad (5 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **5 de 5 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** 

- **Arquitectura:** REL / DOC / TOOL — contrato de larga duración.
- **Dependencias:** REL-08.
- **Archivos:** ~ `schemas/*`; + `docs/compatibility/1.0-policy.md`; + migration fixtures; + plugin API version headers/tests.
- **Especificar:** Qué se garantiza para proyectos, saves y plugins.
- **Preguntar si:** duración política y compatibilidad forward/backward exacta.
- **Diseñar:** Golden fixtures por versión y migraciones solo hacia delante con backup.
- **Dirigir:**
  1. **REL-09.1** — **[1 d]** auditar schemas.
  2. **REL-09.2** — **[1 d]** cerrar IDs/campos.
  3. **REL-09.3** — **[1 d]** crear fixtures golden.
  4. **REL-09.4** — **[1 d]** probar migración.
  5. **REL-09.5** — **[1 d]** publicar política.
- **Definición/pseudocódigo:**

```text
for(fixture in supportedVersions) assert(loadAndMigrate(fixture).matchesExpected);
```

- **Revisar:** No reutilizar campos/IDs retirados; unknown data policy explícita.
- **Criterio de aceptación:** Todos los fixtures soportados abren y ninguna migración pierde datos conocidos.

### REL-10 — Publicar MotorGraphico Creator 1.0 (3 días)

> **Estado real:** `PENDIENTE` — avance **0 %** · esfuerzo restante **3 de 3 jornadas**.  
> **En el repo hoy:** —  
> **Cómo leer esta ficha:** 

- **Arquitectura:** REL — cierre formal del roadmap.
- **Dependencias:** REL-09.
- **Archivos:** ~ version/changelog/release workflows; + release notes; + checksums; + support docs.
- **Especificar:** Gate de release, artefactos, canales, soporte y rollback.
- **Preguntar si:** licencia, distribución pública y nombre comercial definitivos.
- **Diseñar:** Release inmutable desde commit firmado y artefactos reproducibles.
- **Dirigir:**
  1. **REL-10.1** — **[1 d]** release candidate final.
  2. **REL-10.2** — **[0,5 d]** full matrix.
  3. **REL-10.3** — **[0,5 d]** tag firmado.
  4. **REL-10.4** — **[0,5 d]** publicar artefactos/docs.
  5. **REL-10.5** — **[0,5 d]** monitorizar incidencias.
- **Definición/pseudocódigo:**

```text
if(gates.allGreen && approvals.human){ tag(v1.0.0); publish(reproducibleArtifacts); }
```

- **Revisar:** Checksums, licencias, sample project y recuperación verificados.
- **Criterio de aceptación:** Usuario instala 1.0, crea proyecto, juega y exporta sin herramientas de desarrollo.

# Puertas de revisión por fase

| Gate | Se aprueba únicamente cuando |
| --- | --- |
| G0 | La base actual está inventariada y CI es reproducible |
| G1 | TMX → documento → JSON → runtime conserva semántica y render |
| G2 | El runtime permite mover, colisionar, cambiar mapa y guardar |
| G3 | Se crea un mapa sin Tiled ni edición manual |
| G4 | Aldea, interior y mazmorra conectan con navegación correcta |
| G5 | Una misión completa se crea sin C++ y puede depurarse |
| G6 | Una aventura de 30 minutos se crea únicamente con herramientas públicas |
| G7 | Un tester instala la alpha y exporta un juego |
| G8 | Fixtures antiguas migran, cero P0/P1 y release 1.0 reproducible |

---

## Fase 0 bis — Infraestructura exigida por el código real

Estos seis paquetes no estaban en el Gantt original. Salen de contrastar las fichas con el repositorio: cada uno bloquea criterios de aceptación que hoy no se pueden cumplir literalmente.

### INF-01 — Crear tests/ y ctest, y convertir los demos-aserción en pruebas (5 días)

- **Arquitectura:** INF — Infraestructura y calidad.
- **Dependencias:** BASE-02.
- **Por qué existe esta ficha:** El roadmap referencia tests/ en 60 fichas y el repo NO TIENE tests/ ni enable_testing(). Sin esto, ningún criterio de aceptación de las fichas se puede cumplir literalmente.
- **Dirigir:**
  1. **INF-01.1** — **[1 j]** añadir enable_testing() y add_test a los demos que ya abortan al fallar.
  2. **INF-01.2** — **[1,5 j]** crear tests/ con un runner mínimo sin dependencias externas.
  3. **INF-01.3** — **[1,5 j]** mover las aserciones de demo_editor_state a tests/Editor.
  4. **INF-01.4** — **[1 j]** ejecutar ctest en CI y hacer fallar el build si cae una prueba.
- **Criterio de aceptación:** ctest --output-on-failure pasa en CI en Debug y Release y una prueba rota tumba el pipeline.

### INF-06 — Limpiar duplicados y ruido del repositorio (1 días)

- **Arquitectura:** INF — Infraestructura y calidad.
- **Dependencias:** INF-01.
- **Por qué existe esta ficha:** Hay archivos duplicados por sincronización ('FORMATO_NIVELES 2.md', 'demo_battle 2.cpp'), .DS_Store versionados y salidas .ppm/.png en la raíz. Es barato y evita editar el archivo equivocado.
- **Dirigir:**
  1. **INF-06.1** — **[0,5 j]** resolver los duplicados ' 2' y borrar los .DS_Store.
  2. **INF-06.2** — **[0,5 j]** mover salidas de demo a build/ y ampliar .gitignore.
- **Criterio de aceptación:** git status limpio tras compilar y ejecutar los demos, sin archivos duplicados.

### INF-02 — Decidir la tecnología de UI del editor (HUD propio vs Dear ImGui) (3 días)

- **Arquitectura:** INF — Infraestructura y calidad.
- **Dependencias:** INF-06.
- **Por qué existe esta ficha:** EDT-02 planifica integrar Dear ImGui, pero level_editor.cpp ya documenta que se descartó por falta de red para vendorizar. Toda la fase 3 depende de cerrar esto.
- **Dirigir:**
  1. **INF-02.1** — **[1 j]** medir qué cuesta hoy un panel nuevo con el HUD propio.
  2. **INF-02.2** — **[1 j]** comprobar si se puede vendorizar ImGui en third_party/ como glad y stb.
  3. **INF-02.3** — **[1 j]** escribir el ADR y reescribir EDT-01..EDT-06 según la decisión.
- **Criterio de aceptación:** Existe un ADR aceptado y las fichas EDT afectadas están reescritas contra esa decisión.

### INF-04 — Decidir dependencias externas y política de vendorizado (2 días)

- **Arquitectura:** INF — Infraestructura y calidad.
- **Dependencias:** INF-02.
- **Por qué existe esta ficha:** No hay audio (GDB-11), no hay ImGui (EDT-02) y el entorno de desarrollo no tiene red. third_party/ solo tiene glad, stb y tinyxml2. Es una restricción real que atraviesa tres fases.
- **Dirigir:**
  1. **INF-04.1** — **[1 j]** listar las dependencias que exigen las fases 3, 6 y 7.
  2. **INF-04.2** — **[1 j]** fijar la política: vendorizar en third_party/ o implementar en casa.
- **Criterio de aceptación:** Documento de política aprobado y GDB-11 y EDT-02 replanificados en consecuencia.

### INF-05 — Ampliar CI a Windows y macOS (4 días)

- **Arquitectura:** INF — Infraestructura y calidad.
- **Dependencias:** INF-04.
- **Por qué existe esta ficha:** BASE-02 declara compatibilidad multiplataforma pero ci.yml solo usa ubuntu-latest; TOOL-10 promete exportación a Windows sin haberlo compilado nunca allí.
- **Dirigir:**
  1. **INF-05.1** — **[1,5 j]** añadir windows-latest a la matriz y arreglar lo que rompa.
  2. **INF-05.2** — **[1,5 j]** añadir macos-latest y aislar lo específico de GL.
  3. **INF-05.3** — **[1 j]** documentar qué demos son GL-free y cuáles necesitan pantalla.
- **Criterio de aceptación:** La matriz de CI pasa en los tres sistemas y el README dice qué se garantiza en cada uno.

### INF-03 — Reorganizar el árbol hacia la arquitectura objetivo (6 días)

- **Arquitectura:** INF — Infraestructura y calidad.
- **Dependencias:** INF-05.
- **Por qué existe esta ficha:** La arquitectura objetivo del roadmap (Project/, Map/, World/, Events/, Database/, editor/) no coincide con el árbol real (Editor/, Level/, RPG/, Game/, Render/). Cada ficha DOC/RUN empieza con rutas que no existen.
- **Dirigir:**
  1. **INF-03.1** — **[1 j]** mapear cada carpeta actual a su destino y publicar la tabla.
  2. **INF-03.2** — **[2 j]** sacar Project e índice de Editor/ a su propio módulo sin lógica de UI.
  3. **INF-03.3** — **[1,5 j]** separar el núcleo del editor de examples/ hacia editor/.
  4. **INF-03.4** — **[1,5 j]** actualizar CMake, includes y CI en un solo commit verificable.
- **Criterio de aceptación:** El proyecto compila igual tras el movimiento y ninguna cabecera de documento incluye Render ni HUD.

# Protocolo para iniciar una tarea con un agente

Usar esta solicitud, sustituyendo el ID:

```text
Trabaja únicamente en <ID> del roadmap MotorGraphico Creator.
1. Lee la ficha completa y sus dependencias.
2. Ejecuta Especificar y enumera decisiones sin resolver.
3. Si una decisión bloqueante no está contestada, pregunta y no implementes.
4. Propón el diseño mínimo y los archivos exactos.
5. Espera aprobación si cambia arquitectura, formato, UX o compatibilidad.
6. Implementa las unidades de Dirigir en commits revisables.
7. Ejecuta Revisar y demuestra el criterio de aceptación.
8. No incluyas cambios ajenos; registra cualquier alcance extra como nueva tarea.
```

# Registro de revisión humana

Para cerrar cada tarea, el técnico completa:

```text
Tarea:
Commit/PR:
Decisiones aceptadas:
Archivos revisados:
Pruebas reproducidas:
Riesgos pendientes:
Resultado del criterio de aceptación:
Veredicto: APROBADA | CAMBIOS SOLICITADOS | BLOQUEADA
```






