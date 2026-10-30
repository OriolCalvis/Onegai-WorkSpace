# 11/09/2026 — D-04: el catálogo de condiciones lee stackable/duration/effects

- **Commits:** MotorGraphico `795f8f6` (7 ficheros; hunks propios, sin los bloques PAQUETES DEL PLAN ajenos sin commitear).
- **Deja esperando a Oriol:** nada nuevo; CI remota pendiente como en el resto del corte B.

## Encargo y coordinación

Segunda tanda del mismo encargo («coge el Gantt y sigue programando»). Con E-01 aún bloqueado por la reserva de ProjectSerializer (codex, que además está implementando E-01 en vivo — `EventLayerDocument` ya aparece en el variant de `MapLayer`), el paquete elegido es **D-04 (Condicions)**: `RpgCatalogs.h` y los catálogos RPG siguen libres, y es dependencia directa del **D-09 que opencode tiene abierto hoy** (su fila excluye expresamente `RpgCatalogs.h`: el lado executor no puede arreglar el lado datos). Fila reclamada antes de tocar nada.

## Cambio

Las 16 cartas hablan `stackable`/`duration`/`category`/`effects[]`; el cargador esperaba `stacking`/`defaultRounds`/`damagePerRound`/`caMod`... Ningún campo coincidía y las 16 cargaban idénticas (DURATION, 3 rondas, 1 stack, 0 daño, flags apagados). Migración DEC-02:

- **stackable** → STACK_INTENSITY (fatiga, la única apilable) / NONE. `maxStacks` queda a 1: ninguna carta declara el máximo y no se inventa.
- **duration** → nuevo `Duration::UNTIL_CURED` para «hasta_curarse» (15/16): no expira por rondas — `ConditionEngine::tick_conditions` no decrementa pero sí aplica el daño por ronda declarado, y la retira `remove()`. «1_turn» → ROUNDS con 1 ronda (guardia_rota, la única que expira).
- **category** → isNegative (anclado/enfurecido son «neutro»).
- **effects[].mechanicHook** → todos crudos en el nuevo campo `mechanicHooks` (para D-11/E-14/G-13) y traducidos solo los unívocos con apoyo del GDD: `dano_por_turno:N`→damagePerRound (sangrado=1), `velocidad_mitad`→halfSpeed, `desventaja:ataque[_y_habilidad]`→grantDisadvantageToCaster, `bloquea:vision`→grantAdvantageToEnemies, `incapacitado`→preventAction. `velocidad_0` NO es halfSpeed y queda crudo.
- **Criterio «modificadores de defensa parseados e ignorados»**: `caMod`/`physicalSaveMod`/`willSaveMod` **retirados** — el cargador viejo los leía, ningún consumidor existía (verificado en src/include/tests/examples) y Ed. 3 deroga la CA como dificultad. Las cartas los expresan como hooks (`bono_ca:2`, `penal_ca:2`), que quedan crudos hasta D-11. `statMod` se conserva (recalc sí lo aplica).

## Verificación

- `test_catalog_content` **10/10** (5 de D-02 + 5 nuevos), con la prueba del plan: **≥3 combinaciones stacking×duración** (antes el conjunto tenía UN elemento). `test_condition_engine` **12/12** con el caso nuevo UNTIL_CURED (10 rondas sin expirar, daño 1/tick, `remove()` la retira). `test_catalog_contract` **7/7** con el contrato invertido: los nombres del cargador viejo son ahora los que fallan AllDeclared. `demo_catalogos_rpg` verde.
- **Falsabilidad por mutación: 4 mutantes cazados** (stackable ignorado, duration ignorado, `dano_por_turno` sin mapear, tick sin el salto UNTIL_CURED), todos revertidos **con copia de seguridad** — nunca `git checkout` (lección registrada de D-02).
- **Incidente de la sesión:** tras las mutaciones, el primer build completo abortó en `MapDocument.cpp` (E-01 de codex en obras) ANTES de recompilar mis binarios, que quedaron mutados y parecían una regresión propia. Rebuild explícito de los targets del paquete → 10/10 y 12/12. Lección: tras mutar, recompilar explícitamente cada target propio antes de leer la suite.
- **Fallos ajenos documentados (regla 2):** `demo_ciudad` aborta cruzando la puerta del mercado por la edición concurrente de `GameSession.cpp` (C-17 de opencode, 12:47); `motor_level`/`EditorState` en obras. Ninguno toca cargadores RPG.

## Coexistencia

- Codex ejecutó `tools/marcar_paquetes.py` a las 12:47 y sembró bloques «PAQUETES DEL PLAN» sin commitear en 4 de mis 6 ficheros: el commit va quirúrgico por índice (parche filtrado hunk a hunk), versionando solo mis cambios. Los bloques quedan en el árbol para su sesión.
- Deuda apuntada en el seguimiento: `tools/auditar_datos_onegai.py` aún espera nombres viejos de condiciones (herramienta de auditoría; su marco es D-10).
