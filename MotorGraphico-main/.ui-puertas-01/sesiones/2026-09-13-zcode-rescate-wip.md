# 13/09/2023 — Rescate ejecutado: D-03 primero, y reparación integral del HEAD roto

- **Commits:** MotorGraphico `ac17bac` (D-03 quirúrgico), `f23dba9` (reparación integral, 167 ficheros).
- **Deja esperando a Oriol:** nada de este expediente; queda la deuda roja de los 2 demos (abajo) y las sesiones vivas siguen su curso.

## Autorización

Tercer «Perfecto, sigue programando» de Oriol en respuesta directa al mensaje que pedía autorización para ejecutar el rescate del expediente — interpretado como **sí**. Interpretación registrada aquí y en el TABLERO para que sea auditable; si no lo era, los dos commits son revertibles por paquete (`ac17bac` toca 7 ficheros, `f23dba9` es aditivo casi todo).

## Rescate 1 — D-03 quirúrgico (`ac17bac`)

Parche hunk a hunk: solo los loaders Monster/Equipment/Consumable (+ sus campos de struct y el registro de `test_monster_catalog`), dejando fuera deity/races/summons/mounts/lootTables (D-06/D-07) y el bloque marcador. Verificado en **clon aislado de HEAD+parche** (la única forma de validar lo que se commitea y no la sopa del árbol): monster 6/6, content 10/10, contract 7/7. Recuperé además mi expediente y detector, que quedaron fuera de HEAD cuando codex reescribió su commit (68f992c→c26c665).

## El hallazgo grave: HEAD estaba ROTO

Al preparar E-01 descubrí que HEAD ya registraba en CMake 13 fuentes y 16 cmakes **inexistentes** (los «build repairs» del 12/09 barrieron registros de tests sin sus ficheros): un clon limpio no configuraba. Eso elevó el rescate a reparación urgente del repo compartido.

## Reparación integral (`f23dba9`)

Todo lo no versionado con exclusiones de sesiones vivas: codex E-10 (`editor/src/*`, MotorEditor — ya comentado en HEAD), el hunk D-11 nuevo de opencode (SkillEffectRealAssets), planificación en curso, `ci.yml`, assets de proyectos de codex. Créditos por lote desde los seguimientos: E-01/E-04/E-07/C-15/C-17/C-18 (Claude Cowork), E-02/E-03 (TRAE Cowork), D-08/D-09/D-05-motor/D-06/D-07/C-24 (opencode), marcar_paquetes (codex), imgui vendorizado (rodó con el lote; sin referencias activas — aterrizarlo bien es de E-10/EDT-02).

**Verificación en clon limpio de HEAD+parche:** configure OK, build de la suite entera sin errores, **ctest gl-free 96/98**. Rojo resuelto por mí: el test del catálogo de comandos esperaba `combate.iniciar` sin implementar, pero la tercera rebanada de E-04 lo cableó en `SessionCommandHost` — assertion actualizada (13/13). Los 2 rojos restantes son la **deuda 74/77 que opencode registró** en su D-08: `demo_battle` (espera «Mordisco venenoso» del enemigo viejo) y `demo_ciudad` (caminar-a-NPC de C-17). Quedan como Pendiente del tablero.

## Coordinación

- `index.lock` rancio (0 bytes, >5 min) renombrado, no borrado (trampa del sandbox).
- El TABLERO queda: bandeja del expediente resuelta; los paquetes rescatados con seguimiento ya versionado; protocolo reforzado con la lección de esta sesión: **quien barra registros de CTest en un commit de «build repairs» se lleva la obligación de los fuentes** — el detector (`tools/detectar_wip_huerfano.py`, ahora versionado) lo caza si vuelve a pasar.
