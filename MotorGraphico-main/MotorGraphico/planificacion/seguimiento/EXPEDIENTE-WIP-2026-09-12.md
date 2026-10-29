# EXPEDIENTE WIP HUÉRFANO — 12/09/2026 (zcode)

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el expediente de WIP huerfano de zcode, foto del 12/09/2026. Lo que quedaba por decidir se decidio: las piezas aparecen hoy versionadas o con fila propia en `TABLERO.md`. De las 25 citas de codigo que contiene, 23 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

**Qué es esto.** Siete paquetes del corte B están implementados (con su
`seguimiento/*.json` escrito por sus autores) pero su código sigue **sin
commitear** en el árbol de trabajo, sin fila de TABLERO que los ampare.
Regla 2 del protocolo: no se commitea ni se deshace trabajo ajeno sin
autorización de Oriol. Este expediente deja el rescate listo para que esa
autorización se ejecute en un commit por paquete. Patrón: el
`C-11-integracion.md` del rescate del 03/09.

**Regenerar el triage:** `python3 tools/detectar_wip_huerfano.py` (nuevo).
Nivel 1 = ficheros nuevos sin versionar (seguro). Nivel 2 = solo ficheros
compartidos reformados; la atribución se hace verificando el símbolo
distintivo contra HEAD (el grep por id solo anota: los commits citan ids
ajenos en prosa y algunos versionan sin nombrar el id — `05edf18` =
C-13/C-15 sin decirlo).

## Los siete huérfanos verificados + un lote anexo

| Paquete | Autor (su seguimiento) | Verificado por zcode | Falta para cerrar |
|---|---|---|---|
| D-03 Monstres | TRAE (Cowork) — «implementación preexistente + tests nuevos» | `defenses{}`/`derived{}` NO están en HEAD; árbol con el lote dentro verde el 11/09 (content 28/28) | Nada: `MonsterCatalogTests.cpp` ya existe. Commit loader+tests juntos |
| D-05-loaders (anexo a D-03, sin seguimiento propio) | ¿TRAE/qwen? («preexistente» para TRAE) | `compatibilitySymbols`/`requiredStats`/`penalties`/`linkedSkill` no en HEAD | Commit junto al D-03 (mismo fichero) + sus tests de content |
| D-08 Enemics del catàleg | opencode | `EnemyCatalogBridge.h` untracked; su seguimiento dice «GameSession crida createEnemy()» | Commit puente+GameSession+tests; **cruza con E-04 (GameSession) — rescatar en orden** |
| E-01 Evento en el documento | Claude (Cowork) | `EventLayerDocument.*` untracked; **su test NO ENLAZA hoy** (falta link a la lib de Project en su cmake) | Arreglar el link del target y verde → commit |
| E-02 Interruptors locals | TRAE (Cowork) | 0 vs 25 apariciones de interruptores/variables en HEAD:vivo | Commit NarrativeEngine+SaveGame+`NarrativeVariablesTests` |
| E-03 Intèrpret | TRAE (Cowork) | `EventInterpreter.*` untracked | Commit + sus tests |
| E-04 Joc de comandes | Claude (Cowork) | `EventCommand.h`/`EventCommands.cpp` untrackeds; su seguimiento dice ABIERTO (faltan familias) | Rescatar lo hecho; el paquete sigue abierto |
| E-07 Validació (rodaja) | Claude (Cowork) | `AdventureValidation.*` untrackeds; ABIERTO (solo eventos) | Ídem |

C-04/C-07/C-08/C-12/C-13/C-15/C-18 aparecen en el nivel 2 «sin commit que
los nombre» pero **están versionados** (rescates del 03/09 y `05edf18`
sin nombrar ids; C-18 verificado por símbolo): sus ficheros compartidos
los reforma el WIP de arriba. Falso positivo del triage, no rescatar.

## Exclusiones — sesiones VIVAS ahora mismo (no tocar sus ficheros)

- **codex, D-05/DEC-05:** `ProjectCatalog.*`, `assets/ejemplos_*`,
  `assets/proyectos/*`, `tests/Project/Project*Tests.cpp`, bloques de
  catálogo en `level_editor.cpp`.
- **opencode, D-11-O2:** `SkillEffect.*`, `EnhanceHooks.h`,
  `SkillExecutor.cpp`, `Skill/BattleState`, `tests/RPG/SkillEffectTests.cpp`.
- **Origen desconocido (acaba de aparecer, compila mal):**
  `tests/Render/PlayerPopulateTests.cpp` — hoy rompe el build; esperar a
  su autor, no arreglarlo en el rescate.
- opencode D-10 ya commiteado (`f79e4c7`, `1b099bf`).

## Estado del árbol combinado (verificación zcode, 12/09)

Build completa: **2 roturas**, ambas ajenas a los huérfanos cerrados:
`test_event_layer_document` no enlaza (E-01, ve arriba) y
`test_player_populate` no compila (sesión viva). El resto de la suite
gl-free que llega a correr está en verde. Ayer 11/09 el mismo árbol dio
content 28/28 y gl-free 61/66 (5 Not Run por el EditorState de codex en
obras, ya versionado en `05edf18`).

## Orden de rescate propuesto (dependencias + menos cruce primero)

1. **D-03 + D-05-loaders** (solo `RpgCatalogs.h`/`RpgCoreDefinitions.h` +
   tests RPG) — cero cruce con vivas.
2. **E-02** (NarrativeEngine/SaveGame — cruza solo con E-04 en GameSession:
   no lo toca) y **E-03** (EventInterpreter, ficheros nuevos).
3. **E-01** tras arreglar el link de su test; arrastra MapDocument/
   ProjectSerializer/MapLayer (nadie vivo los reclama hoy).
4. **E-04** y **E-07** (GameSession compartido con D-08).
5. **D-08** el último (GameSession + EnemyBrain tras E-04).

Cada commit: loader/código + SU test en el mismo commit, mensaje nombrando
el id, verificación local antes (build del target + suite gl-free).
Excluir SIEMPRE los ficheros de las vivas (pathspec/hunks).

## Huellas SHA-256 (prefijo 64→16) de los ficheros clave

```
3cb8ad98619971e4  include/Map/EventLayerDocument.h
9908117b7fe38711  src/Map/EventLayerDocument.cpp
4279a30f3f1c68a6  tests/Map/EventLayerDocumentTests.cpp
fc7a100b5769cc8e  include/RPG/EventCommand.h
605c2e520c9075cd  src/RPG/EventCommands.cpp
e398f59cb051e53b  include/RPG/EventInterpreter.h
27cc393fcba0452b  src/RPG/EventInterpreter.cpp
0e81bb3b8d4f115b  include/RPG/AdventureValidation.h
bb14b9c24f212317  src/RPG/AdventureValidation.cpp
90555a7b92da0f9c  include/Game/EnemyCatalogBridge.h
14b27e8948d7a91a  include/RPG/NarrativeEngine.h
75e1d572966a7d34  include/RPG/Catalogs/RpgCatalogs.h
60eb5d9e1cc0e137  tests/RPG/MonsterCatalogTests.cpp
795ab9c0d2148124  tests/RPG/NarrativeVariablesTests.cpp
bb0d7cdad8f47ca9  tests/RPG/EventCommandsTests.cpp
d4185d3ca3912b50  tests/RPG/EventInterpreterTests.cpp
a63b1ded1a06c1b6  tests/Game/EnemyCatalogBridgeTests.cpp
```

Si alguna huella no coincide al ejecutar el rescate, el fichero cambió
desde este expediente: re-verificar antes de commitear (alguien siguió
trabajando).

**Nota de protocolo:** han aparecido dos familias de agentes (Claude
Cowork, TRAE Cowork) que escriben seguimientos pero no usan TABLERO ni
commitean. Proponerles (o exigirles vía D0001) el commit con git-safe al
terminar. También hay un `MotorGraphico/sesiones/` untracked: el registro
de sesiones vive en `Software/sesiones/` (D0001), no dentro del repo del
motor.
