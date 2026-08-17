# Estudio comparado del editor RPG y propuesta multi-título

*Encargo de Oriol (17/08/2026): convertir MotorGraphico en una aplicación
para crear videojuegos separados — varios títulos con una misma forma de
funcionar, historias cortas o largas, a buen coste y calidad. Primero
estudiar y comparar con herramientas del mercado; después adaptar.*

---

## 1. Qué pedimos del editor

1. **Multi-título:** un mismo motor publica juegos separados (cada título
   con sus assets, sus reglas de contenido y su ejecutable), sin
   reescribir el motor por juego.
2. **Autoría completa sin tocar código:** NPCs, diálogos, historias,
   eventos (por zona, por conversación, en segundo plano), temporizadores
   que decidan misiones por TIEMPO y no por calidad, objetos, enemigos.
3. **UI/UX a la altura:** hoy "suspende por todos lados"; menús con
   objetos mal etiquetados (auditoría en marcha).
4. **Productividad de autor:** plantillas, duplicar, buscar, deshacer,
   validar y playtestear sin salir del editor. «Sacar títulos como
   churros.»
5. **Lo que NO hace falta:** IA sofisticada de movimiento (basta
   patrullas y persecución simple); físicas complejas; un motor nuevo.

## 2. Herramientas del mercado: paneles y lecciones

### 2.1 RPG Maker MV/MZ (la referencia directa del género)

Paneles: **editor de mapas** (capas de tiles + eventos colocados en las
casillas), **base de datos** (Actores/Clases/Habilidades/Objetos/Enemigos/
Tropas/Estados/Tilesets/**Eventos comunes**), **editor de eventos** por
casilla, **playtest** integrado. Flujo oficial: `Database → Map → Event →
Playtest`.

Lecciones que copiar:
- **El evento es DATO, no código**: disparador + condición + lista de
  comandos, con páginas que cambian según flags. Nuestro `NarrativeBeat`
  es exactamente este modelo (trigger + requires + effects) — ya tenemos
  el corazón conceptual.
- **Eventos comunes**: lógica reutilizable por id (una "puerta con
  llave", un "cofre") que cualquier mapa invoca. Nosotros: `ObjectCatalog`
  + beats compartidos por aventura.
- **Interruptores y variables globales** como estado del mundo. Nosotros:
  las flags (ya son la única fuente de verdad, bitácora).
- **Disparadores**: botón de acción, tocar al jugador, **autorun** y
  **parallel** (segundo plano). Nuestros Talk/Enter/Auto cubren tres; el
  "parallel" real (beats de fondo que corren sin interacción) lo cubre
  `tick()`, pero le faltan **temporizadores**.
- Debilidades que NO copiar: formato cerrado, motor rígido, editor con
  estética de los 2000.

### 2.2 Godot 4

Docks: Escena (árbol de nodos), Inspector (propiedades), FileSystem
(assets), editor de scripts. Potente y abierto, pero **no trae base de
datos RPG**: hay que construirla o tirar de addons (Dialogic para
diálogo, addons de quests). Lección: un inspector de propiedades genérico
es barato de mantener y sirve para TODO; pero para un autor no
programador, los formularios tipados (como la DB de RPG Maker) rinden más
que un inspector libre.

### 2.3 Unity + plugins de contenido

El patrón de mercado: el editor general + **editores de contenido
específicos** como asset (Dialogue System for Unity, Quest Machine).
Lección: los paneles de quests y diálogo son productos en sí mismos —
señal de que son caros de hacer y valiosos; los nuestros deben salir de
nuestros propios formatos (FORMATO_AVENTURAS), no de formatos ajenos.

### 2.4 Especialistas de narrativa: Yarn Spinner e Ink

Diálogos como **ficheros de texto editables fuera del motor** (con
herramienta propia), compilados a datos que el juego consume. Lección:
el diálogo se escribe mejor en una vista de texto/grafo dedicada que
dentro de JSON a mano; y la separación texto→dato facilita traducir y
revisar. Nuestros diálogos hoy viven en `DialogueData.lines` (lineales) y
en beats; el salto es una vista de autoría, no un formato nuevo.

### 2.5 Tiled

Estándar de mapas por casillas (tilesets, capas, objetos con
propiedades). Nuestros TMX ya son de su familia; el level_editor ya
edita este formato. Lección: no reinventar el formato de mapa — el
nuestro (TMX + JSON de nivel) es compatible con el ecosistema.

### 2.6 RPG in a Box / GB Studio

Editores pequeños con scripting visual y **export a juego standalone**.
Lección para el multi-título: el valor está en que "File → Export"
produce algo jugable y distribuible sin llevarse el editor detrás.

### 2.7 Patrones comunes de las herramientas que SÍ sacan juegos

1. Base de datos de entidades tipadas con formularios (no JSON a mano).
2. Editor visual de mapas con eventos colocados encima.
3. Eventos = datos: disparador + condición + efectos, editables.
4. Estado del mundo = variables/flags, visibles y depurables.
5. Reutilización: prefabs/eventos comunes/plantillas.
6. Playtest inmediato y vuelta al punto del error.
7. Validación que explica (fichero, entidad, coordenada).
8. Undo/redo + autoguardado + export estable en git.

Nosotros tenemos semilla de 2, 3, 4, 5, 7 y 8 (level_editor, beats,
flags, ObjectCatalog, validar_todo.py, EditorState round-trip). **Faltan
en serio: 1 (paneles de base de datos), 6 (playtest contextual) y la
superficie visual de todo lo demás.**

## 3. Lo que YA tenemos (y no se reescribe)

| Semilla | Qué es | Hacia dónde crece |
|---|---|---|
| `NarrativeEngine` (beats: Talk/Enter/Auto + flags + efectos) | El motor de eventos, GL-free y testeado | El panel de **eventos/historias** edita ESTO |
| `ObjectCatalog` + `ObjectSpawn` | Base de datos de objetos e instancias | Panel de **objetos/NPCs/enemigos** |
| `assets/proyectos/` + `build_proyecto.py` + `ProjectHub` | Packs de contenido con build jugable | El **multi-título**: cada proyecto = un título exportable |
| `level_editor` + `EditorState` (undo/redo, export) | Editor de mapas | Crece con paletas de eventos/NPCs |
| `EnemyBrain` (patrullas) | Movimiento enemigo simple | Suficiente; se queda |
| `tools/validar_todo.py` + `verificar_controles.py` | Validación que explica | El "panel de errores" del editor |
| `CONTRATO_IDS.md` + `renombrar_id.py` | Ids estables con migración | Requisito de toda base de datos seria |
| `architecture/MotorGraphico_World_Platform.mdj` (StarUML) | Propuesta ya dibujada de plataforma multi-juego (Game/Campaign Definition) | Se alinea con esta propuesta |

Regla: el editor crece ENCIMA del núcleo GL-free existente (criterio
ARCHITECTURE.md), nunca un segundo motor paralelo.

## 4. Necesidades del encargo → huecos concretos

| Necesidad de Oriol | Estado | Hueco |
|---|---|---|
| Crear NPCs | `ObjectDefinition` Npc existe (diálogo lineal, tienda) | Panel de NPC; diálogo ramificado; horarios/estado |
| Historias | Beats + objetivos (diario) | Panel de historia: beats como lista/grafo, vista de flags |
| Líneas de diálogo | `DialogueData.lines` + beats | Vista de autoría de diálogo (texto primero, JSON después) |
| Evento al entrar en zona | Trigger `Enter` existe | Falta editor + señalización en el mapa |
| Evento al hablar con algo | Trigger `Talk` existe | Ídem |
| Eventos en segundo plano (sorpresas) | Trigger `Auto` + `tick()` | Falta editor + documentación del patrón |
| Temporizadores (misión por tiempo) | **No existe** | Contrato nuevo: deadline con flag al expirar (ver §5 F1) |
| Menús mal etiquetados | — | Auditoría UI en marcha (inventario separado) |
| Varios títulos | Proyectos + build | Empaquetado de título: config, export standalone, saves propios |

## 5. Propuesta: paneles y fases

### F0 — Auditoría y arreglo de la UI existente (base de todo)
Inventario de todas las cadenas visibles (en curso), catálogo de
etiquetas con corrección de nombres, idiomas y hotkeys anunciadas.
**Terminado cuando:** cada control visible hace lo que dice, y
`verificar_controles.py` pasa en ambas direcciones sin avisos.

### F1 — Eventos y temporizadores (el motor de las historias)
- Contratos nuevos (GL-free + tests): `TriggerType::Pickup`, `Defeat`,
  `Timer`; `TimerDefinition` (deadline en ticks de juego/días, flag al
  expirar, flag de aviso opcional) — la misión que sale bien o mal por
  TIEMPO.
- `AdventureScript` gana timers y el editor de mapas señala dónde hay
  eventos.
**Terminado cuando:** una aventura demo tenga un evento de zona, uno de
conversación, uno de fondo y una misión con deadline, todo testeado sin
ventana.

### F2 — Autoría de diálogo y NPCs
- `DialogueData` crece a diálogo con condiciones por rama (sin motor
  nuevo: ramas = beats con `Talk`).
- Panel de NPC: ficha, diálogo, tienda, facción.
**Terminado cuando:** un NPC del editor habla, vende y cambia de
respuesta según flags, sin tocar JSON.

### F3 — Base de datos visual (objetos/enemigos/skills)
Paneles con búsqueda, filtros, duplicar, plantillas y edición por lotes
sobre los catálogos existentes (el roadmap los pide en S3).
**Terminado cuando:** crear enemigo/objeto/NPC no requiere tocar C++ ni
JSON (hito H1 del roadmap).

### F4 — Multi-título (el objetivo de negocio)
- `GameDefinition`: manifiesto de título (nombre, época, resolución,
  paleta/LUT, fuente, controles, proyecto raíz) — crece desde
  `assets/proyectos/`.
- `juego --titulo X` arranca un título; saves por título; export a
  carpeta standalone (extiende `build_proyecto.py`).
**Terminado cuando:** dos títulos distintos salen del mismo motor con
sus propios assets, config y build, y cada uno guarda su partida.

### F5 — Playtest y depuración contextual
Lanzar el juego desde el editor y volver al mismo punto; el panel de
errores (validar_todo) navega al fichero/entidad (H2 del roadmap).

Orden propuesto: **F0 → F1 → F2 → F3 → F4 → F5** (F0 y F1 ya pueden
corrían en paralelo con otras tareas del roadmap; F4 puede adelantarse
si el multi-título urge más que los paneles).

## 6. Decisiones tomadas por Oriol (17/08/2026, bitácora 2.18)

1. **Universo único:** todos los títulos ocurren en el universo de
   Onegai — el multi-título son historias dentro del MISMO mundo.
2. **Estética:** estilo RPG Maker (paneles jerárquicos, flujo claro),
   sin copiar su interfaz literal.
3. **Orden confirmado:** F0 → F1 → F2 → F3 → F4 → F5.
4. **Temporizadores = tiempo narrativo de misión** (no calendario ni
   segundos crudos): no avanzan en combate, se pausan con el jugador
   quieto (AFK), y requieren feedback visible. Mecanismo fino pendiente
   de negociación (F1). Ejemplos del encargo: la casa se quema si no
   superas la zona antes de irte; tiempo para rescatar a alguien; tiempo
   hasta que el asesino mate a otro.
