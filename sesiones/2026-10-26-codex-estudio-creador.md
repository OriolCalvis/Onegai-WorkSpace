# Estudio visual del creador — 26/10/2026

Autor: Codex. Encargo: estudiar alas y colas poco visibles y defectos raciales, especialmente minotauros. Se usa el flujo engineering:debug; sin cambios de producto ni de ramas ajenas.

Base: codex/revision-reciente-20261026, producto cbc2503. Original y wt_integ con WIP ajeno preservados. Reclamo AUDIT-CREADOR-26 en TABLERO antes de crear instrumentación.

Informe: MotorGraphico-main/MotorGraphico/planificacion/ESTUDIO_CREADOR_VISUAL_2026-10-26.md. Entrega visual permanente: MotorGraphico-main/MotorGraphico/out/estudio-creador-20261026/index.html, ESTUDI.md, láminas PNG, matriz TSV y fuentes de instrumentación.

Método: enlace de un harness temporal a las bibliotecas CMake existentes (ningún src compilado suelto). 72 razas × (base + 15 complementos) = 1152 montajes, 4608 muestras de orientación quieta. 975 complementos ofertados, 105 forzados para diagnóstico. 0 errores de carga. MapaDeCapas mide visibilidad; Pillow solo recorta y amplia las salidas C++.

Hallazgos: minotauro.json ausente y fallback a familia orco en dos minotauros y aeronautas; vista inicial quieta SW con oclusión de alas/colas; 4 razas con alas naturales y 17 con cola sin sustitución desde UI; cuernos naturales en rasgos permiten duplicación; cuernos_diablo del enano deepforge totalmente tapados por bacinete en las 4 orientaciones; tinte solo en cuerpo; reglas genéricas sin perspectiva propia para cada apéndice.

Límite explícito: no se reproduce desaparición general de alas/colas en el montador actual; no es una captura de la interacción exacta de Oriol. No se han auditado todos los frames de animación ni todas las ropas/combinaciones. No se han realizado cambios de producto por tratarse de un estudio.

Verificación: las tres láminas principales se inspeccionan visualmente; métricas y rutas de código contrastadas. Informe propone orden de reparación y criterios visuales nuevos; no equivale a cierre de defectos.
