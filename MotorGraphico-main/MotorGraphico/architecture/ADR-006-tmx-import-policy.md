# ADR-006: TMX se importa con informe explícito

**Estado:** Aceptado  
**Fecha:** 2026-08-28  
**Decisor:** Proyecto MotorGrafico

## Contexto

TMX fue la fuente directa de `TileMap`. El modelo documental nuevo debe ser la
fuente de autoría y no puede perder en silencio datos importados: los mapas
TMX reales contienen tilesets, colisión, capas y, potencialmente, grupos u
objetos aún no representados por `MapDocument`.

## Decisión

`TmxMapImporter` acepta TMX finito con capas de tiles CSV y tilesets embebidos.
Convierte GIDs, visibilidad, opacidad y colisión a `MapDocument` y devuelve un
`ImportReport`. Las estructuras que impiden un resultado correcto —CSV no
soportado, dimensiones incoherentes o mapa infinito— son errores. Elementos no
representables, como grupos u object layers, son advertencias explícitas.

`TileMap` queda como adaptador de compatibilidad: usa importador + builder para
sus capas de runtime y solo conserva la lectura de metadatos de tileset que su
API heredada expone.

## Consecuencias

- Renderer y runtime dejan de conocer el parser CSV/TMX.
- Los autores ven las limitaciones antes de guardar un documento canónico.
- Los tilesets externos y elementos TMX avanzados requieren una futura
  extensión documental o una decisión de preservación opaca; no se ignoran.
