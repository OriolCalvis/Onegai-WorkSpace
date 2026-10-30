# 06/09/2026 — Codex — diseño de autoría por bloques

Encargo: explorar con precisión creación mediante bloques encajables y estándares, reutilizando combate.

Entrega: [DISENO_AUTORIA_BLOQUES.md](../MotorGraphico-main/MotorGraphico/planificacion/DISENO_AUTORIA_BLOQUES.md), commit MG `f7b0c55`. Propuesta, sin implementación ni modificación automática de decisiones/plan.

Define plantilla y composición como vistas del mismo documento, árbol tipado, registro de bloques, intérprete/adaptadores, 14 bloques piloto y cinco puertas de prototipo. Contrato de combate indivisible: validar/aceptar/cobrar/resolver una vez, sin exponer restar PM como bloque libre. Constata coexistencia Skill/SkillDefinition y necesidad de conciliar consumidores. Mantiene prioridad legacy de primer beat y registra límites de guardado de ejecuciones. Evalúa interfaz nativa/Blockly/lista sin instalar dependencias ni cambiar ADR-001.

Verificación: lectura de FORMATO_AVENTURAS, Skill.h, SkillDefinition.h, BattleState.h, ADR-001 y datos del plan; referencias oficiales Blockly. 14 IDs de bloques únicos; IDs de paquetes y rutas citadas comprobados. Sin build ni ejecución UI. No se implementan transacciones o contratos que el diseño propone.

Coexistencia: no se toca reserva F1 ni título/Application de qwen, ni archivos ajenos sin commitear. TABLERO solo reclama/cierra la fila propia. BITACORA congelada sin cambios.
