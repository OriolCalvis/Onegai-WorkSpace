# Arquitectura para múltiples juegos

Abre `MotorGraphico_World_Platform.mdj` con StarUML. El modelo presenta
la plataforma como seis ámbitos: base técnica, motor reutilizable,
framework de juego, herramientas de creación, contenido del mundo de
Egaroth y juegos concretos.

Las clases con estereotipo `existing` representan código o datos ya
presentes. `proposed` señala los límites que conviene crear antes de que
coexistan varias campañas: `Game Definition`, `Campaign Definition`,
`Content Pack`, `Save Game Service`, `Game Module API` y el entorno de
creación visual completo.

La regla central es que un juego no referencia archivos globales de modo
implícito. Cada juego declara un `Game Definition`, que selecciona su
campaña, packs de contenido, nivel inicial, tema y espacio de guardado.
Así Boundington y futuros juegos pueden compartir Egaroth, mapas o
reglas Nd6 sin mezclar sus partidas, historias ni dependencias.

Los tres diagramas visibles son:

- `01 - Platform Landscape`: separación entre plataforma, mundo y juegos.
- `02 - Reusable Runtime`: lo que debe seguir siendo independiente de una
  campaña concreta.
- `03 - Creator Toolchain`: el camino para editar, validar y probar
  contenido sin depender de comandos.
