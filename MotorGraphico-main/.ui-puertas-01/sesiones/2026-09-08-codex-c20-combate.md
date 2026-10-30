# 08/09/2026 — C-20: daño recibido integrado en combate

- **Commits:** MotorGraphico `14f5e8a` (11 archivos propios).
- **Deja esperando a Oriol:** nada nuevo; CI remota pendiente.

## Alcance y coordinación

Continuación autorizada del Gantt tras C-16, con Application/GameSession liberados por Oriol. Fila reclamada el 07/09 antes de editar; finalización el 08/09. Se continúa localmente tras el límite de uso de los agentes de la sesión anterior, sin consumir créditos de reinicio. Se preserva el trabajo concurrente C-05 y Map/Project.

## Cambio

BattleState compara la vida del receptor antes y después de resolver ataques básicos y habilidades, tanto aliados como enemigos. Solo la pérdida real positiva notifica a ConditionEngine: cura, fallo y daño bloqueado conservan condiciones. La notificación no se añade a takeDamage: restaurar los PV de un guardado no es daño de combate.

ICombatant expone CharacterSheet opcional y Player/EnemyBrain implementan el contrato. Application posee y carga el catálogo de condiciones y lo transmite a cada GameSession; esta propaga el catálogo a batallas nuevas y activas. El catálogo debe sobrevivir a la sesión/batalla y las fichas siguen siendo externas.

## Verificación

Build completa Debug correcta. Suite 63/63 GL-free y 2/2 ejecutables GL (ApplicationSceneTests y ApplicationShutdownTests) correctos. BattleStateTests pasa 8 casos, incluidas variantes para ataques básicos/habilidades de ambos bandos, persistencia de condiciones del atacante, recálculo del receptor, curación, bloqueo, fallo, ausencia de catálogo/ficha y propagación real mediante GameSession/EnemyBrain antes y durante el combate. BattleStateTests y los 11 casos ConditionEngineTests pasan también con ASan/UBSan. Diff sin errores de whitespace.

Pruebas ejecutadas sobre el workspace vivo en out/build/codex-c16 y out/build/codex-c16-asan: no equivalen a CI remota sobre checkout limpio. No se ha hecho push. Seguimiento C-20 actualizado, pendiente CI para cierre formal.

## Límites y trabajo preservado

La ruptura opera cuando el combatiente tiene CharacterSheet vinculada. No se crean fichas ni se corrige aquí la interpretación del catálogo canónico (D-04) o la aplicación de condiciones desde cartas (D-09). La ruta legacy sin ficha sigue funcionando.

No se incluyen los cambios ajenos de CMake, workflow ci.yml, tests/módulo C-05, Map/Project, assets ciudad_centro ni archivos sin seguimiento. Los documentos/renders generados del Gantt ya estaban modificados: no se regeneran ni se incorporan; el seguimiento está en planificacion/seguimiento/C-20.json. Se regenera el índice de sesiones pero no se versionan sus cambios ajenos previos.
