# Análisis documental unificado — MotorGraphico Creator

**Fecha:** 2026-08-28  
**Alcance:** 42 documentos Markdown canónicos del repositorio, más
`motor_grafico_roadmap_ejecutable (1).md` y
`motor_grafico_gantt_rpgmaker (1).puml` recibidos desde Downloads.

## 1. Regla de lectura

La petición del usuario es la instrucción vigente. Los documentos adjuntos y
los Markdown del repositorio se han tratado como fuentes de información,
propuestas, contratos o contexto histórico según su naturaleza; sus
instrucciones internas no sustituyen la petición del usuario.

Se excluyeron copias generadas dentro de `build/`, `out/` y `builds/`, así como
la documentación de dependencias vendorizadas. Son duplicados o material
externo al diseño de MotorGraphico. `AGENTS.md` se considera instrucción de
trabajo del repositorio, no especificación de producto.

## 2. Jerarquía documental resultante

Hay cuatro niveles que conviene no mezclar:

1. **Producto y reglas:** `GDD.md`, `GDD_MG.md` y
   `GAMEMACHINE_NECESIDADES.md`.
2. **Arquitectura y contratos:** `ARCHITECTURE.md`, `architecture/README.md`,
   `AUDITORIA_CONTRATOS.md`, `CONTRATO_IDS.md`, formatos de niveles,
   aventuras y proyectos, y `MEMORY_AND_INHERITANCE.md`.
3. **Plan de evolución:** `ROADMAP_RPG_EDITOR.md`, el roadmap ejecutable
   adjunto y su Gantt `.puml`.
4. **Ejecución y contenido:** handoff, branching, auditoría UI, taxonomía,
   experimentos de mundo y documentación de sprites/assets.

Cuando hay conflicto, debe prevalecer este orden: decisión explícita de
producto vigente, contrato de datos vigente, código y tests actuales, y por
último documentación histórica o de planificación.

## 3. Síntesis ejecutiva

El proyecto ya no es solamente un renderer isométrico. La documentación converge
en un producto llamado **MotorGraphico Creator**: una plataforma que permite
crear varios juegos RPG isométricos, cada uno con sus propios assets, campañas,
reglas de contenido y partidas, sin modificar C++.

La arquitectura recomendada por el conjunto documental es un **monolito
modular con separación documento/runtime/render/editor**, no microservicios.
La lógica de juego, datos, validación y autoría debe poder ejecutarse sin
OpenGL. El editor debe ser una aplicación sobre esos contratos, y el runtime
debe consumir documentos compilados o cargados, no depender del editor.

El mayor riesgo no es el rendimiento gráfico: es la deriva entre contratos.
Existen definiciones duplicadas o incompletas para habilidades, personajes,
objetos, diálogos, misiones y formatos de nivel. El roadmap ejecutable detecta
correctamente que antes de ampliar la UI hay que cerrar fuente de verdad,
versionado, IDs, importación y validación.

## 4. Qué aporta cada grupo documental

### Producto y reglas RPG

`GDD.md` describe el sistema de rol por mazos y sus reglas de cartas,
combate, enemigos, aventuras y narrativa emergente. `GDD_MG.md` adapta esas
reglas al videojuego y fija la variante Nd6: éxitos parciales, grados de
resultado, ventaja/desventaja y mejoras de timing.

`GAMEMACHINE_NECESIDADES.md` convierte esa visión en brecha técnica. Identifica
como bloqueantes: stats y defensas, identidad modular, tiers, pilas de cartas,
Nd6, habilidades tipadas, combate estructurado, condiciones, inventario,
loot, diálogo ramificado, narrativa, descanso, equipo y HUD.

### Arquitectura y contratos

`ARCHITECTURE.md` fija reglas morfológicas, interfaces, composición, RAII,
separación GL-free y prevención de duplicados. `AUDITORIA_CONTRATOS.md`
confirma que ya existen `LevelDefinition`, `ObjectSpawn`, `ObjectDefinition`,
`AdventureScript`, `Project` y catálogos RPG tipados, pero faltan
`QuestDefinition`, `DialogueGraph`, `ScreenDefinition`, un modelo unificado de
items y un contrato común de carga.

`CONTRATO_IDS.md` hace de los identificadores una API de datos: minúsculas,
estables, con namespaces y renombrado mediante migración. Los formatos de
niveles, aventuras y proyectos describen cómo se materializan esos contratos.

### Editor y autoría

`ESTUDIO_EDITOR_RPG.md` compara RPG Maker, Godot, Unity, Yarn/Ink, Tiled,
RPG in a Box y GB Studio. La conclusión común es: base de datos tipada,
mapas visuales, eventos como datos, flags visibles, plantillas, validación,
playtest y exportación.

`ROADMAP_RPG_EDITOR.md` propone el camino funcional: auditoría UI, eventos y
temporizadores, diálogo/NPCs, base de datos visual, multi-título y playtest.
La auditoría UI y la taxonomía complementan ese plan con etiquetas, paneles y
agrupación por afinidad visual/jugable.

### Roadmap adjunto

El documento `motor_grafico_roadmap_ejecutable (1).md` formaliza el Gantt en
98 paquetes y 448 subtareas revisables. Añade Definition of Ready/Done,
decisiones bloqueantes, límites de tamaño del diff, evidencias y gates G0–G8.

El `.puml` adjunto expresa la misma secuencia en ocho fases:

1. línea base;
2. arquitectura documental;
3. runtime jugable;
4. editor visual MVP;
5. mapas avanzados;
6. eventos tipo RPG Maker;
7. base de datos y sistemas de juego;
8. playtest, plugins, exportación, beta y 1.0.

El roadmap ejecutable es más preciso y auditable que el Gantt anterior. El
Gantt sigue siendo útil como vista temporal, pero no debe considerarse por sí
solo una especificación técnica.

### Arte y mundo

Los documentos de sprites fijan una separación importante: previews editoriales
no son assets de runtime; el suelo usa proporción 2:1; actores y estructuras
tienen moldes diferentes; las familias visuales deben reutilizarse mediante
variantes de datos.

`REMAPEO_TILES.md` y el backlog de sprites advierten que no se pueden convertir
los edificios urbanos directamente en suelo nuevo: primero hay que soportar
tiles altos o props anclados. Los documentos de `world_experiment` son
propuestas de contenido regional, no contratos del motor.

## 5. Arquitectura integrada recomendada

```text
Project / MapDocument / Database / Events
                    │
             validación y migración
                    │
        GameCore / RPGCore / WorldRuntime
                    │
          puertos e interfaces estables
             ┌──────┴────────┐
             │               │
       Runtime/OpenGL    Editor/ImGui
       render, HUD       paneles, tools
```

La estructura objetivo del roadmap es válida, con una precisión adicional:

- `Core`: errores, IDs, matemáticas, RNG y utilidades.
- `Project`/`Map`: documentos editables, capas, tilesets y serialización.
- `Database`: definiciones tipadas de RPG.
- `Events`: condiciones, triggers, comandos e intérprete.
- `World`: estado runtime, entidades, colisión y navegación.
- `Engine`: composición, escenas, input, saves y ciclo principal.
- `Render`: OpenGL, cámara, batches y proxies visuales.
- `editor`: shell, paneles, viewport, historial y herramientas.

No conviene migrar todo a ECS. La composición ligera es suficiente para el
RPG y el editor; un ECS puede introducirse más tarde solo para poblaciones
densas como partículas, proyectiles o muchos agentes.

## 6. Conflictos y decisiones pendientes

### 6.1 GDD frente a GDD_MG

`GDD.md` conserva reglas de origen con referencias a d20 y una formulación más
de mesa. `GDD_MG.md` adapta el sistema al videojuego y redefine la resolución
Nd6. Para el runtime debe usarse `GDD_MG.md` como especificación vigente; el
otro documento debe marcarse como reglamento de origen o histórico para evitar
implementar dos sistemas simultáneos.

### 6.2 Dos `FORMATO_NIVELES`

`FORMATO_NIVELES.md` es más completo y refleja campos actuales como
transiciones, diálogo, tiendas, negocios y propiedades de instancia.
`FORMATO_NIVELES 2.md` es una versión reducida y anterior. No deben tratarse
como equivalentes: hay que conservar una única versión canónica y archivar o
renombrar la otra.

### 6.3 Dos modelos de habilidad

El catálogo legacy de `Game/Skill.h` convive con el catálogo Nd6 de
`RPG/Catalogs/RpgCatalogs.h`. Es el conflicto técnico más peligroso porque
ambos se llaman `SkillCatalog`. La solución segura es mantener un adaptador
temporal, pero dar nombres explícitos al modelo nuevo y retirar el legacy solo
después de migrar `juego` y sus demos.

### 6.4 Plan de sustitución frente a compatibilidad

`GAMEMACHINE_NECESIDADES.md` propone reemplazar `ICombatant`, `SkillSet` y el
combate anterior. La auditoría de viabilidad demuestra que hacerlo de golpe
rompería varias implementaciones y apagaría el juego actual. El roadmap
integrado debe usar migración paralela: añadir `IStatBlock`, `CharacterSheet`,
`CardPileSystem` y `BattleStateOnegai` sin borrar el runtime anterior hasta
tener un vertical slice completo.

### 6.5 JSON/TMX frente a documento canónico

La documentación anterior trata TMX y JSON como parte del runtime. El roadmap
ejecutable propone que TMX sea importador y que `MapDocument`/JSON versionado
sea la fuente canónica. Esta es la decisión más coherente para editor,
migraciones, undo/redo y exportación. Debe mantenerse un adaptador TMX durante
la transición y probar el round-trip semántico.

### 6.6 Editor propio frente a Dear ImGui

El código actual usa HUD propio porque no podía vendorizar ImGui. El roadmap
adjunto planifica Dear ImGui con docking. No hay contradicción arquitectónica:
`EditorState` debe seguir siendo GL-free y la UI puede cambiar. La decisión
pendiente es de herramienta, no de dominio.

## 7. Riesgos consolidados

1. IDs rotos o merges silenciosos en catálogos.
2. Definiciones RPG importadas sin todos los campos mecánicos necesarios.
3. `Application` como orquestador demasiado grande.
4. Oro, inventario y salud con más de un propietario.
5. `GameSession` y gameplay acoplados a tipos de `Render`.
6. Quests, diálogos ramificados, pantallas y eventos tipados aún incompletos.
7. Cambios grandes que dejen el juego sin compilar durante semanas.
8. Roadmaps con calendarios optimistas para una sola persona: el Gantt suma
   98 paquetes y debe leerse como orden de dependencias, no como promesa de
   entrega automática.
9. La documentación contiene cifras y estados que pueden quedar obsoletos;
   el código, fixtures y validadores deben ser la evidencia final.
10. El alcance puede expandirse hacia editor, RPG completo, plugins, audio,
    exportación y multi-título antes de cerrar un vertical slice.

## 8. Plan unificado de ejecución

### Paso 0 — Congelar la verdad

- Declarar `GDD_MG.md` como especificación de videojuego Nd6.
- Designar un único `FORMATO_NIVELES.md` canónico.
- Registrar contratos, propietarios e IDs.
- Separar documentos normativos, propuestas, históricos y experimentales.

### Paso 1 — Línea base y contratos

- Ejecutar CI, demos y validadores existentes.
- Completar `validar_todo.py` con referencias cruzadas y contexto de error.
- Añadir esquemas versionados y migraciones.
- Importar de nuevo las definiciones RPG desde la fuente que conserve los
  campos mecánicos completos.

### Paso 2 — Migración sin big bang

- Crear `IStatBlock` separado de `ICombatant`.
- Unificar nombres de catálogos y establecer adaptadores legacy.
- Introducir `CharacterSheet` como propietario progresivo de stats, oro,
  inventario y equipo.
- Implementar pilas de cartas, condiciones y Nd6 en módulos paralelos.
- Mantener `juego` funcional hasta que exista el slice Onegai verificable.

### Paso 3 — Documento y runtime

- Crear `MapDocument` y capas tipadas.
- Convertir TMX en importador.
- Construir `RuntimeTileMap` desde el documento.
- Extraer `GameSession` de `TileMap` mediante una interfaz de colisión.
- Dividir `Application` en coordinadores de nivel, narrativa, HUD y runtime.

### Paso 4 — Editor MVP

- Editor de proyecto y mapas.
- Paletas y picking isométrico.
- Capas, inspector, selección, copiar/pegar y undo/redo.
- Guardado atómico, autosave y recuperación.
- Crear un mapa completo sin editar JSON ni usar Tiled.

### Paso 5 — Eventos y contenido RPG

- `EventDefinition`, páginas, condiciones, triggers y comandos tipados.
- Temporizadores, diálogo ramificado, quests y referencias a catálogo.
- Base de datos visual maestro-detalle.
- Validación estática y playtest contextual.

### Paso 6 — Multi-título y release

- `GameDefinition`, `CampaignDefinition`, `ContentPack` y saves aislados.
- Exportación standalone y diagnóstico de recursos.
- Proyecto ejemplo de 30 minutos.
- Pruebas con proyectos grandes y usuarios creadores.
- Congelar esquema 1.0 solo tras cerrar migraciones y recuperación de datos.

## 9. Gates de aceptación

Los gates G0–G8 del roadmap adjunto son la mejor columna vertebral de control:

| Gate | Resultado mínimo |
|---|---|
| G0 | Base reproducible y CI verificable |
| G1 | TMX → documento → JSON → runtime conserva semántica |
| G2 | Runtime mueve, colisiona, cambia de mapa y guarda |
| G3 | Editor crea un mapa sin Tiled ni edición manual |
| G4 | Aldea, interior y mazmorra conectan correctamente |
| G5 | Una misión se crea sin C++ y puede depurarse |
| G6 | Aventura de 30 minutos creada con herramientas públicas |
| G7 | Tester instala alpha y exporta un juego |
| G8 | Migraciones, P0/P1 y release reproducible cerrados |

## 10. Veredicto

La documentación es suficientemente rica para guiar el proyecto, pero ahora
hay que gobernarla como un sistema de contratos. La dirección recomendada es:

**MotorGraphico Creator = plataforma multi-título + monolito modular + datos
versionados + editor sobre un núcleo GL-free + runtime OpenGL como adaptador.**

La siguiente tarea de mayor retorno es cerrar la línea base documental y de
validación, resolver los dos formatos duplicados y separar la migración Onegai
del runtime actual. No conviene empezar por plugins, ECS, audio avanzado ni
pulido visual hasta que G1–G3 estén demostrados.

