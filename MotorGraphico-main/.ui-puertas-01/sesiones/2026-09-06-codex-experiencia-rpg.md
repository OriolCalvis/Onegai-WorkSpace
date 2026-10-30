# 06/09/2026 — Codex — experiencia completa frente a RPG Maker

## Encargo y entrega
Analizar MotorGraphico, Gantt y planning al detalle de menús, objetos, descripciones, tooltips, arranque y ampliación mediante datos/código.

Entregado [ESPECIFICACION_EXPERIENCIA_RPG.md](../MotorGraphico-main/MotorGraphico/planificacion/ESPECIFICACION_EXPERIENCIA_RPG.md), 22 secciones y 24 requisitos UX trazados al Corte B. Commit del motor: `f48ebee`. Solo documentación; no se modifica planificación generada, código, canon ni decisiones aceptadas.

## Hallazgos
- Paneles generales sin exclusión de input de mapa en rama de ratón del editor; el hover actual es información de celda, no sistema de tooltip. Hallazgo por lectura, no reproducción visual.
- Guardado del editor anuncia éxito sin verificar streams TMX/JSON. Probar/cambiar mapa no recibe un resultado de fallo.
- Menú de combate y habilidades de demostración hardcodeados; objetivos fijos. Título, pausa y asistente integrado pendientes del plan.
- F-01 pide mismo proceso, en tensión con BITACORA §8.4. Se registra discrepancia en la entrega sin cambiar la decisión.
- G-01 queda retrasado por puerta de fase G. Propuesta de revisar ubicación, no replanificación aplicada.
- Grafo vigente: 70 paquetes / 243 jornadas / 28 filas de cobertura; ruta crítica ideal con puertas de fase de 109 unidades bajo paralelismo sin límite. No son estimaciones de trabajo restante.

## Verificación
Código de arranque/editor/HUD/inspector/entrada leído; fuentes oficiales RPG Maker MZ consultadas y enlazadas. DAG recalculado con entradas dades.py y política de build.py, sin regenerar salidas. IDs de paquetes citados válidos. 22 secciones y 24 UX comprobados. No build, ejecución UI ni pruebas del motor en esta sesión; guiones de aceptación son requisitos futuros, no resultados.

## Coexistencia
Se preservan cambios ajenos en CMake, MapDocument/EntityLayerDocument, ProjectSerializer, tests, LevelDocumentBridge, seguimientos, plan y renders; también temporales preexistentes. Reserva F1 GameSession/Application respetada. TABLERO modificado únicamente para reclamar y cerrar esta tarea; no se incluye su contenido ajeno en el commit del motor. No se edita BITACORA congelada.
