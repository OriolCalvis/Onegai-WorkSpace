# CONTROLES-02 y rodaja G12-INVENTARIO-VISTA — 29/09/2026

Encargo de Oriol: continuar game feel y Gantt con MAPA_DEL_CODIGO, respetando a los agentes simultáneos. Commits MotorGraphico: `c4b939e` y `315957f`. Continúa CONTROLES-01 (`0e2b2b4`).

- Jugador: reposo orientado, fase de animación conservada entre casillas y profundidad visual interpolada. Cadencia de 140 ms también al alternar direcciones, cola máxima de un tap y cancelación del paso automático al soltar. Foco, modal y teletransporte limpian la cola. El rebote de Claude se conserva en el árbol compartido, fuera del commit propio.
- Inventario: pantalla real conectada a InventoryMenuState y GameSession, cantidades agrupadas, equipar/desequipar, errores visibles, refresco por identidad y ventana de texto que sigue al cursor. Consumibles piden destinatario; cancelar o cambiar pestaña no los consume. InventoryMenuState pasa a motor_level, retirando su compilación suelta del test.
- README, mapa y seguimiento actualizados. G-12 global permanece abierto: faltan la vista previa y otros criterios, además del playtest manual completo.
- Integración concurrente: no se modifican GameSession ni reglas RPG. La prueba obtiene InventoryRules mediante inventoryRules() para admitir su extracción. Se conservan FichaVista, DiarioMisiones, Códice e iconos. ApplicationMenuJugador.cpp, aún ajeno y sin versionar, conserva el pequeño hunk que muestra el detalle/resultado; no se añadió el fichero completo al commit.

Validación final en el árbol compartido: juego compila, humo `./juego --nivel assets/levels/ciudad_oeste.json 3` sale 0 y framebuffer inspeccionado. CTest 2/2: Player 63 casos (61 propios y 2 del rebote ajeno), InventoryMenu 13 (incluyen cuatro integraciones con pantalla/sesión reales). Build: `/private/tmp/motor-est36-player-build`.

En worktree aislado: juego y humo pasan, CTest 5/5 (Player, populate, inventario, pantallas GDB12, límite de equipo). La validación inicial sobre 4fc8c00 necesitó copiar cuatro fuentes BattleState/BattleMenuActions del WIP compartido, solo para resolver implementaciones que HEAD aún no incluía; quedaron fuera del commit. Build: `/private/tmp/motor-controles-inventario-build`. No se afirma compilación de un clon limpio ni suite global completa.

Guardia rápida y diff-check pasan. Se corrigió el pre-commit para exportar GIT_DIR/GIT_WORK_TREE absolutos antes de cambiar de directorio; los commits pasaron el hook normal, sin saltarlo. El hook sigue avisando de tests FichaVista ajenos ausentes del índice del worktree. El índice principal quedó sin cambios staged; el resto del trabajo concurrente permanece en disco. Un index.lock vacío generado por lectura desde sandbox se conservó renombrado tras comprobar antigüedad y ausencia de escritor, siguiendo la trampa documentada; siguientes lecturas usan GIT_OPTIONAL_LOCKS=0.

CUA no reconoció el binario nativo como app; queda pendiente el playtest manual completo. Las dos reservas propias del TABLERO quedan liberadas. Worktree `codex/controles-inventario` disponible para reutilizar, sin procesos nuestros en ejecución.
