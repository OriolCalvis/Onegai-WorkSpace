# FRONT-01 — Tanda de E para agentes externos (29-10)

Protocolo para todas las E: `RELEVO.md`. Cada E deja su resultado en `relevos/<E>.md`.

| E | Agente | Qué | Toca código |
|---|---|---|---|
| E-L1A | Trae | Clasificar 5 fallas de ctest (pergamino, ruta_casillas, botín, economía, monstruos_acciones) | no |
| E-L1B | opencode · Big Pickle | Clasificar `test_lugares`, los 3 `demo_*` y `test_catalog_content` | no |
| E-L1C | Claude (reasignada; antes DeepSeek v4.1) | Clasificar `test_personajes_capas` (timeout), `test_kit_basico` y `test_clases_sp` | no |
| E-L1D | opencode · Beam | Clasificar `linea_base_core`, `linea_base_core_configura` y `simulador_dificultad_humo` | no |
| E-CAN1 | Antigravity (noche del 29 al 30-10; antes Nemotron) | Vault: ojos y aspecto de Venides, y borrador de ficha | no |
| E-CAN2 | opencode · GLM 5.3 free flash | Vault: C8-04b (Venides al final) y C8-04c (aviso de Xila) | no |
| E-CAN3 | opencode · MiMo v2.6 | Vault: C3-07 (reglas de aparición) y C3-02 (músicos y nobles) | no |
| E-L0 | Claude (reasignada; antes DeepSeek v4.1 con su harness) | Registro de reanudación en `fabrica.py`, con 4 pruebas | **sí**, rama `agente/claude/FRONT-L0` |

Los agentes de opencode van con esfuerzo bajo. **DeepSeek ya no tiene E en esta tanda** (29-10, 15:45: Oriol las pasa a Claude).

**Todas las E son independientes** y pueden ir en paralelo. Si un agente se queda sin tokens, cualquier otro recoge la E con el mensaje de `RELEVO.md` §6.

**Cuando estén todas:** Claude junta los cuatro `E-L1*` en `tests/CUARENTENA.md` y los tres `E-CAN*` en `orquestador/DECISIONES_PENDIENTES.md`, revisa la rama de E-L0 y escribe en `REVISAR.md` lo que tiene que mirar Oriol.
