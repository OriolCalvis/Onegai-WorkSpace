# AV18-04 — recompensas de escenas y misiones (15/10/2026)

- **Autor:** Codex.
- **Rama:** agente/codex/AV18-04, worktree ~/onegai-ramas/codex-AV18-04, sobre f0a91b7.
- **Commits:** `98369be` (herramientas y guía de recorrido).
- **Deja esperando a Oriol:** nada nuevo; calibración pendiente del contrato AV18-05 y de cobertura/horarios/PO-49.

Continuación autorizada después de verificar/cerrar el montaje de Claude en AV18-06. Bitácora, protocolo, plan y handoff leídos; fila reclamada antes de crear ficheros. No se modifica ninguna aventura, misión, nivel ni fuente de Claude. Se usa el worktree propio existente, que estaba limpio.

El CLI recorrer_recompensas usa NarrativeEngine real y enlaza motor_level. Los datos de herramientas declaran tres escenarios con acciones/preferencias y resultados de tiradas/combates. Los tres alcanzan bnd3_fuera/bnd3_epilogo; la traza lista los pagos y objetos del camino. La auditoría lee las seis aventuras y quests: 29 efectos (19 grantGold, diez darObjeto), incluidos los de opciones. Las seis misiones tienen premio cero. El premio de 30 de Silbido de Medianoche queda fuera del proyecto. SHA-256 de las fuentes en la salida; alternativas no recorridas no se cobran.

Ledger: cobros ordenados, clamp de oro igual a GameSession, cama sin duplicar la reserva y premios narrativos conservados por unidad. Tiradas exitosas, combates ganados, moral neutral/nivel 1, tiempo y movimiento ausentes son hipótesis explícitas. «Explora todo» nombra la política del documento, no cobertura exhaustiva; «optimiza» tampoco acredita un óptimo global.

Comandos desde MotorGraphico-main/MotorGraphico:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j8
ctest --test-dir build-release -L gl-free --output-on-failure
ctest --test-dir build-release -R '^(test_simular_economia|test_recompensas_economia)$' --output-on-failure
python3 tools/simular_economia.py --json
python3 tools/simular_economia.py --json --ajuste -25
python3 tools/simular_economia.py --json --ajuste 50
python3 tools/simular_economia.py --json --reparto /Users/admin/onegai-ramas/qwen-AV16-09b/MotorGraphico-main/MotorGraphico
```

Release completa. Suite 234/236 (39,82 s): solo linea_base_core_configura y linea_base_core, 49 instantáneos/44 de deuda, como en la base. 13 casos previos y 13 nuevos en verde; se corrigieron fixtures incompletos y la ruta de fiesta durante el desarrollo. Última comprobación específica en verde. Las cuatro simulaciones salen 2: ejecutadas, sin calibración. Neutral: historia 5/5/5, explora todo 5/2/2, optimiza 37/14/6. Nueve márgenes fuera; no se tocan precios/presupuestos.

Qwen entregó AV18-05 da120b1 durante esta continuación. Se leen sus niveles sin copiarlos ni fusionarlos: 17 lotes loot fijos y cero contenedores del contrato nativo. Hay identidad botin_av18_05 por triplicado en tres niveles; faltan botin{dia,region,canal,orden}, tablas/prefabs e identidad por instancia para poder recalibrar con AV18-02/03. Solo reparto estático del día 1. Su aviso del mínimo 40 corresponde al documento antiguo: vigente 20 desde AV18-03. Se registra en el informe propio; no se modifica trabajo de Qwen ni se le envía mensaje.

Guía revisable y tabla completa en planificacion_alcance_y_vision/RECORRIDO_RECOMPENSAS_AV18-04.md. AV18-04 sigue abierto. La continuación 98369be se entrega aparte: no se ha aplicado a la punta AV18-06. Informe y tablero compartidos actualizados por secciones sin sustituir filas ajenas. Logs /tmp/av18-04-{configure,build-full,ctest-full}.log y JSON neutral/descuento/recargo/qwen en /tmp. No hay interfaz gráfica nueva que capturar.
