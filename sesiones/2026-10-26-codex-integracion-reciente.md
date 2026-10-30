# Corrección: compilar la integración reciente — 26/10/2026

Autor: Codex. Oriol señala que la compilación anterior no contenía sus mejoras recientes y autoriza todo el conjunto más reciente.

## Causa y alcance

La compilación anterior partía de claude/escenas01 (bfc5a22, contenido hasta 19/10). Aunque compilaba y arrancaba, no representaba la integración reciente. El registro 2026-10-26-codex-compilar-juego.md solo certifica aquella rama antigua.

Se crea copia aislada /tmp/onegai-reciente-20261026, rama codex/revision-reciente-20261026, base claude/integracion-pl 37ffa05, integrada con fable/pl-ropa-01 c03127b. Incluye Gorran ampliado e integración narrativa, modelos PixelLab, ropa, puertas y personalización del creador. Verificados por ascendencia e50331e, 37ffa05 y c03127b. Las copias originales y sus cambios pendientes se preservan; incluye WIP de muebles/isometría ajeno en wt_integ que no se incorpora como entrega terminada.

## Integración y corrección

- e0111b1: integración; conflictos solo en BrilloEquipoTests y PersonajesCapasTests, conservando comprobaciones recientes.
- cbc2503: faltaban 21 PNG de alas/colas/cuernos, referenciados por personajes.json pero ausentes en Git y el paquete. La prueba aleatoria falla con can't fopen alas_medio.png. Generados con pixellab_extras existente, 7 tallas por 3 grupos; herramienta reproducible tools/gen_rasgos_personalizacion.py. Diagnóstico de prueba enriquecido con apariencia y error concreto.
- Sin modificaciones de lógica de producto; no se regeneran ciudades ni se reescribe canon.

## Verificación y entrega

CMake Release y objetivo juego correctos. Arranque de título y ciudad, 3 frames, salida normal. Carpeta final comprobada también.

CTest final: test_personajes_capas (144,60 s), test_brillo_equipo y test_editor_apariencia: 3/3 pasan; incluye piel, pelo, marcas, rasgos y apariencias aleatorias. No equivale a suite completa.

Entrega permanente: MotorGraphico-main/MotorGraphico/out/juego-reciente-20261026/ (binario, assets, VERSION.txt y Obrir joc recent.command). Juego abierto desde esa carpeta. Fuente recuperable por rama; worktree temporal conservado para recompilar. Logs de verificación copiados en entrega. TABLERO original tiene cambios ajenos y solo se añade/mueve la fila propia; no se commitea completo.
