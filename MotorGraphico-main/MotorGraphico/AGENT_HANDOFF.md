# Handoff para agentes — MotorGraphico

## Qué es este proyecto

**MotorGraphico** es un motor isométrico pixel-art en C++17/OpenGL que incluye un RPG de fantasía y un editor visual de niveles. Su campaña de referencia es **Boundington**; el estado de prototipado publicado es `0.00.01v`.

El objetivo no es hacer un único juego rígido: el motor debe permitir crear varias historias, mapas, misiones y campañas sin tener que programar cada contenido a mano.

## Mapa mental rápido

```text
assets (datos y arte)
  └─ cargadores/parsers GL-free
       └─ GameSession / EditorState
            └─ Application + IsometricRenderer + HUD
                 ├─ juego         (runtime)
                 └─ level_editor  (autoría visual)
```

## Dónde está cada cosa

| Ruta | Responsabilidad |
|---|---|
| `include/Core`, `src/Core` | JSON, matemáticas, recursos y código sin OpenGL cuando es posible. |
| `include/Render`, `src/Render` | mapa isométrico, entidades, combate, HUD, catálogo y estado del editor. |
| `include/Engine`, `src/Engine` | ventana, ciclo de aplicación y unión de sistemas. |
| `examples/juego.cpp` | ejecutable del juego. |
| `examples/level_editor.cpp` | editor de proyectos y niveles. |
| `assets/maps` | mapas TMX de Tiled. |
| `assets/levels` | niveles JSON: spawns, enlaces, jugador y metadatos. |
| `assets/objects` | catálogos de props, PNJ, enemigos, pickups y sus variantes. |
| `assets/proyectos` | manifiestos de campaña; `boundington` es la referencia principal. |
| `assets/adventures` | contenido narrativo/misiones. |
| `assets/textures` | arte de runtime; no confundir con los atlas `editor_*`, que son previsualizaciones. |
| `tools` | validadores de enlaces, conectividad y catálogos. |

## Flujo de contenido

1. Define o amplía el objeto en un catálogo de `assets/objects` con un ID estable.
2. Incluye ese catálogo, mapa y nivel en el manifiesto de `assets/proyectos`.
3. Colócalo desde `level_editor`; los niveles guardan instancias, posición, escala, variante, nombre visible, efecto y propiedades personalizadas.
4. Conecta los niveles mediante enlaces/salidas y valida antes de probarlo.

Los identificadores son datos de contrato: usar minúsculas y guiones bajos, y no renombrarlos sin actualizar todas sus referencias.

## Ejecutar y verificar

```bash
cd MotorGraphico
cmake -S . -B build
cmake --build build --target juego level_editor demo_editor_state -j4
cd build
./level_editor --proyecto boundington
./juego 1
./demo_editor_state
python3 ../tools/validar_enlaces.py
python3 ../tools/conectividad.py
```

`assets/` se copia a `build/assets` durante la compilación: editar siempre la fuente en `assets/`, no la copia generada.

## Reglas de implementación

- Mantener la lógica de datos, validación y reglas de juego libre de OpenGL; `GameSession` y `EditorState` deben seguir siendo testeables sin ventana.
- Favorecer composición y datos sobre herencias profundas. Las herencias de entidades deben ser pequeñas, con propiedad clara y sin estado duplicado.
- Usar RAII y `ResourceManager` para recursos GPU; no crear recursos por frame ni guardar punteros crudos con propiedad ambigua.
- Para colecciones temporales de alta frecuencia, reutilizar capacidad/pools con límites claros y perfiles antes de optimizar. No introducir pools globales que oculten fugas, ciclos de vida o invalidación.
- Mantener las instancias de nivel como datos: la variante o el tamaño de una poción/PNJ debe configurarse desde el editor, no mediante nuevas subclases.
- No editar `build/` ni artefactos generados. Evitar refactors masivos que rompan catálogos, mapas y contenido existente.

## Convenciones visuales esenciales

- Suelo isométrico: fuente 128×64 px, render a 64×32 px (relación 2:1).
- Personajes y objetos altos: 64×64 px, con los pies alineados al borde inferior; nunca girar sprites verticalmente para adaptarlos al mapa.
- Consultar `SPRITE_AUTHORING_GUIDE.md`, `SPRITE_DIMENSIONS_GUIDE.md` y `SPRITE_BACKLOG.md` antes de añadir arte o nuevos moldes.

## Prioridad actual

Consolidar el vertical slice de Boundington: misiones de la taberna, bosque, desagües y los Perdidos; mejorar el editor para que esas cadenas se creen y prueben visualmente; y mantener los mapas, catálogos y representaciones de runtime coherentes entre sí.
