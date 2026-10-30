# 2026-09-17 · Claude (Cowork) · Revisión del reglamento Ed. 4

Oriol pasó el texto del sistema de combate nuevo (pool, críticos, pifias, hechizos, tiers, vida,
menús, cartas de hechizo) y la moral de dos ejes estilo Fable II, y pidió revisarlo.

## Colisión: la fila ya existe
**qwen** tiene fila viva en el TABLERO («Combate Ed. 4») con el mismo encargo y con el GDD +
`decisiones/D0007` como zona. No he tocado nada de su zona ni del motor: sólo he escrito la
revisión y le he dejado un comentario fechado en su fila.

## Lo hecho
- `planificacion/REVISION_REGLAS_ED4_2026-09-17.md`: reglamento tal como queda, comprobado contra
  los cinco ejemplos numéricos de Oriol, más el coste en código regla por regla.
- Cuatro decisiones nuevas de Oriol (respondidas en sesión): crítico = mitad del pool en seises;
  hechizos por tirada enfrentada (**descarta el umbral**, que es justo lo que dice la fila de qwen);
  fallar la salvación = daño entero y superarla = mitad; daño físico = suma de los d6 del margen.
- Cinco cabos sueltos a la bandeja de Oriol (§3 del documento).

## Hallazgos que importan
- Cuatro de los cinco ejemplos de Oriol cuadran con «margen de éxitos redondeado, 0,5 sube y el
  resto de X,5 baja»; el quinto (1,5 contra 0 → él dice 2 dados) sólo cuadra con una resta por
  caras, que a su vez rompe el tercero. Hay que elegir.
- Ed. 3 había **eliminado** las CD; el texto de Oriol las reintroducía para hechizos. Al elegir
  enfrentada, el reglamento vuelve a tener una sola forma de resolver.
- Con Ed. 4, ganar la salvación ya no anula el daño (hoy `opposed_degree` devuelve BOTCH = 0).
- La vida doblada por tier choca con `healthCap` de `tier_rules.json` (tier 5: 128/192 vs 125) y
  `tier_rules.json` no tiene nivel mínimo por tier.
- Menú del jugador, objetivos múltiples, cartas de hechizo e iconos de estado: no hay nada en el
  repo; el menú depende además de que el juego ejecute eventos de mapa.
