# Análisis DAFO — Motor Gráfico Isométrico Pixel Art (C++)

## Fortalezas (interno / positivo)

- C++17 con templates y RAII: rendimiento predecible y gestión de memoria segura sin recolector de basura.
- Arquitectura modular clara (Errores, Recursos, Render, Motor) que facilita añadir nuevos sistemas sin romper los existentes.
- OpenGL 3.3 Core es ampliamente compatible (Windows, Linux, y macOS hasta su deprecación) y bien documentado.
- El estilo pixel art con paleta limitada reduce drásticamente el coste de producción artística frente a 3D o 2D de alta resolución.
- Al ser un motor propio, es reutilizable como base para múltiples juegos roguelike futuros, amortizando la inversión inicial.
- `ResourceManager<T>` genérico evita duplicar código entre gestores de texturas, shaders u otros recursos futuros.

## Debilidades (interno / negativo)

- Curva de aprendizaje alta en programación gráfica de bajo nivel (OpenGL, shaders, gestión manual de buffers).
- Dependencia de herramientas externas para edición de mapas (Tiled) al no existir editor propio en las primeras fases.
- Equipo probablemente pequeño (uno o pocos desarrolladores), lo que alarga los tiempos frente a las estimaciones optimistas del Gantt.
- Alcance inicial sin sistema de físicas, red o audio: cualquier juego real necesitará estos sistemas más adelante.
- Los diagramas de render y el propio motor son difíciles de testear con pruebas unitarias automáticas (mucho es visual).
- El sistema mixto de errores (excepciones + `Result<T>`) exige disciplina para no mezclar criterios de cuándo usar cada uno.

## Oportunidades (externo / positivo)

- Comunidad activa e ilusionada con roguelikes y pixel art, con nichos de jugadores fieles (r/roguelikes, itch.io, etc.).
- Bibliotecas maduras y gratuitas (GLFW, GLAD, stb_image, glm) que cubren gran parte de la infraestructura de bajo nivel.
- Posibilidad de publicar el motor como open source, atrayendo colaboradores y feedback técnico externo.
- Reutilización directa en futuras game jams o prototipos, reduciendo el coste marginal de cada nuevo juego.
- Formato de mapas TMX (Tiled) es un estándar de facto, lo que facilita herramientas y assets de terceros.

## Amenazas (externo / negativo)

- Motores generalistas ya maduros (Godot, LÖVE2D, Unity 2D) resuelven gran parte de este problema con mucho menos esfuerzo de desarrollo.
- OpenGL está deprecado en macOS y su soporte a largo plazo en plataformas Apple es incierto (alternativa: MoltenVK/Metal).
- "Feature creep": la fase 4 (iluminación, niebla de guerra, paletizado) puede expandirse indefinidamente y retrasar un prototipo jugable.
- Dependencia de mantenimiento de bibliotecas de terceros que podrían quedar obsoletas o sin soporte.
- Riesgo de sobre-ingeniería: la combinación de templates + excepciones + `Result<T>` puede complicar el código más de lo necesario para un motor pequeño.

## Lectura estratégica rápida

|             | Ayuda                                                                                  | Perjudica                                                                               |
| ----------- | -------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| **Interno** | Fortalezas → aprovechar la modularidad y los templates como argumento de venta técnica | Debilidades → mitigar con documentación y tests de integración tempranos (Fase 5)       |
| **Externo** | Oportunidades → publicar como open source y apoyarse en Tiled/glm                      | Amenazas → fijar un alcance mínimo viable (Fases 1-3) antes de tocar la Fase 4 opcional |
