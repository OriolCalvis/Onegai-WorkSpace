# 12/09/2026 — D-03 reclamado y liberado: WIP huérfano en los loaders; auditoría alineada con D-04

- **Commits:** MotorGraphico `84a7f26` (herramienta de auditoría). D-03 **sin código propio**: bloqueado por WIP huérfano (ver abajo).
- **Deja esperando a Oriol:** autorización para versionar el WIP huérfano de los loaders D-03/D-05 y cerrar ambos con sus tests (precedente: rescate del 03/09).

## Qué pasó con D-03

Encargo: seguir el Gantt usando `MAPA_DEL_CODIGO.md` (nueva herramienta de coordinación de codex: ficheros↔paquetes, con los puntos de cruce señalados). Elegido **D-03 (Monstres)** — dependencia directa del D-08 vivo de opencode, cuyo puente `EnemyCatalogBridge` lee un MonsterCatalog que en HEAD carga con `maxHealth = 0`. Fila reclamada antes de estudiar el asset.

Al estudiarlo, el cargador del árbol de trabajo **ya estaba migrado a `defenses{}`/`derived{}`** — con comentarios «D-03 —» — pero **sin commitear y sin fila**: huérfano total (sin sesión, sin rama, sin TABLERO). El lote huérfano es mayor: loaders de D-03 (monstruos) + D-05 (equipo: `compatibilitySymbols` fusionado, `requiredStats`, `penalties`, `linkedSkill`; consumibles: `flavorText`, usos, tipo de acción) + sus campos en `RpgCoreDefinitions.h`. Estilo y mediciones («13 cartas», «31/31», «100/100») idénticos al estilo de qwen en D-01 — y opencode documentó ayer «Fuera a propósito: loaders `RpgCatalogs.h` (zcode)», así que no es suyo. Hoy no hay fila viva de qwen.

**Decisión (regla 2 del protocolo):** no se commitea ni se deshace. El árbol CON todo dentro está verificado verde (`test_catalog_content` **28/28** tras build). Fila D-03 liberada con documentación en Pendiente; lo que falta para cerrar D-03 es solo la prueba del plan (recorrer los 432 y afirmar `maxHealth > 0` + stats no todas cero, más 2-3 cartas concretas) — **quien versionee el loader debe meterla en el mismo commit**, o HEAD queda roto. Añadida fila a «Esperando a Oriol» pidiendo la autorización de versionado (mismo mecanismo que el rescate del 03/09).

Datos del asset medidos para quien lo cierre: `defenses{ca,defMental,maxHealth,movement,precMag,resFis}` en 432/432 con `maxHealth` SIEMPRE > 0; `derived{}` duplica los valores en castellano; `stats{CAR,CON,DES,INT}` 432/432; `skillIds` VACÍO en los 432 (se leen, quedan vacíos: DEC-02, no se inventan); `attacks[].defense` ∈ {CA 562, defensa_mental 187, resistencia_fisica 151} — mismo vocabulario que skills (D-02); tiers 1..5.

## Qué se entregó

`tools/auditar_datos_onegai.py`: su sección «condiciones» seguía pidiendo `stacking`/`statModifier` — desde D-04 (`795f8f6`) nadie los espera; ahora pide `stackable`/`duration`/`category`/`effects`. Era deuda anotada en el seguimiento D-04. Verificado con `py_compile` y ejecutada completa.

**Observación lateral (no perseguida):** la auditoría reporta `castingStat` 0/483 en las habilidades ORIGINALES de dndWeebCC aunque el 11/09 la primera carta (`abrazo_de_brasas.json`) lo llevaba — posible edición concurrente aguas arriba. Registrada aquí; el flujo canon→cartas→motor prohíbe tocarlo desde el motor.

## Coordinación (el tablero está saturado)

Mapa de hoy: codex → D-05/DEC-05 (fixtures) + C-23 (render/visuales, ProjectSerializer solo visualSettings); opencode → D-10 (CI) + D-07 resto + D-08 (esperando que GameSession use `EnemyCatalog::createEnemy()`); otra instancia zcode → D-10 activación CI (commits `f79e4c7`, `1b099bf`). C-24 completado por opencode (6 suites GL-free). Con D-03 fantasma, D-05-loaders fantasma, D-06/D-09 sin commitear y E-01 en manos de codex, **no queda ningún paquete del corte B libre y limpio a la vez** — la fila de Oriol desbloquea dos (D-03 y la mitad loader de D-05) de un golpe.
