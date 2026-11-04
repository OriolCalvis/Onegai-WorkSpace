# ZONAS DUDOSAS PARA FABLE - fotos recuperat/arbre/* y checkout de Oriol vs caa7565a

Generado el 2026-11-02, solo lectura de git. Alcance: `MotorGraphico-main/MotorGraphico/{src,include,tests,cmake,CMakeLists.txt,assets,tools,examples,editor}`, ignorando rutas con `/out/` o `/build`. Referencia: `caa7565a`. Bases viejas para detectar 'version antigua': `a523d6aa`, `95ef783a` y los 25 ancestros de caa (un blob de la foto igual a cualquier version historica de ese mismo fichero = 'antigua, superada').

## Lectura rapida (lo que mas importa)

1. **El resto de ficheros 'nuevos' es casi siempre el mismo puñado de 12 assets** (ver 'Assets comunes'): estan en las fotos del 14-22/10 y en la copia de Oriol, y NO en el linaje 95ef783a->caa. Dos de ellos estan referenciados por codigo/JSON de caa y faltan (editor_*.png, lote2/con.png).
2. **Las 84 'faltas' constantes** (assets/textures/race_npc_sprites 43, monumentos 38, race_npc_idle.{png,json}, gen_race_npc_sprites.py, 1 de personajes) NO son perdida: las borro a proposito el commit `NET-E3+E4` (3ff6888d, 29/10, 84 D). Estan en todas las fotos; NO restaurar.
3. `BattleMenu*`, `TargetSelection*`, `QuestLogState*`, `demo_battle.cpp` (solo en las 3 fotos del 14/10: AV13-08, AV15-02a/b) tambien son borrados deliberados: AV13-35 'el combate antiguo, medido y borrado' (14/10) y D0016/AV14-22 (QuestLogState retirado; ver comentario en cmake/QuestLogTests.cmake de caa). NO restaurar.
4. Las fotos intermedias (14-22/10) son **versiones anteriores** de ~250-340 ficheros que caa tiene ya mas nuevos (caa viene de main c528ebb del 29/10 con todo integrado). Su 'revisar' es ruido salvo los ficheros 'propios' (blob que no aparece en ninguna otra foto ni en la historia), que se listan por foto.
5. En `caa7565a` hay un `CMakeLists 2.txt` (duplicado de Finder/iCloud) rastreado; y las fotos traen `pixellab/props 2.json` (otro duplicado). Ignorar/limpiar.

## Tabla resumen

Columnas: **nuevos** = ficheros en la foto que no existen en caa (ya descontados los 84 borrados a proposito por NET-E3+E4 y los que caa tiene con otro nombre) · **mod-antiguos** = blob de la foto igual a una version historica de ese fichero (ya superada) · **mod-revisar** = existe en ambos, contenido distinto, blob no historico · **propios** = de esos 'revisar', los que no aparecen en ninguna otra foto (trabajo especifico de esa tarea).

| foto | fecha tarea | nuevos | mod-antiguos | mod-revisar (propios) | veredicto |
|---|---|---|---|---|---|
| PILOTO-29-10-E1 | 29/10 | 0 | 129 | 1 (1) | rescatar fichero modificado |
| PILOTO-29-10-E2 | 29/10 | 0 | 128 | 1 (1) | rescatar fichero modificado |
| PLAU01-R1 | 29/10 | 0 | 131 | 0 (0) | nada que rescatar |
| agy-E-00-SIMULACRO | 29-31/10 | 0 | 129 | 0 (0) | nada que rescatar |
| REPLICA-R1-U3-02 | 30/10 | 0 | 128 | 1 (1) | nada que rescatar |
| REPLICA-R1-U3-05 | 30/10 | 0 | 128 | 4 (4) | nada que rescatar |
| REPLICA-R1-U3-07 | 30/10 | 0 | 128 | 4 (4) | nada que rescatar |
| REPLICA-R1-revisiones | 30/10 | - | - | - | fuera de alcance |
| codex-AV13-08 | 14/10 | 27 | 15 | 395 (1) | rescatar ficheros nuevos (solo assets sueltos) |
| codex-AV15-02a | 14/10 | 27 | 15 | 395 (1) | rescatar ficheros nuevos (solo assets sueltos) |
| codex-AV15-02b | 14/10 | 27 | 15 | 398 (3) | rescatar ficheros nuevos (solo assets sueltos) |
| codex-AV16-08 | 15/10 | 14 | 22 | 332 (9) | nada que rescatar (salvo assets comunes) |
| codex-AV18-01 | 15/10 | 12 | 22 | 333 (0) | nada que rescatar (salvo assets comunes) |
| codex-AV18-02 | 15/10 | 12 | 22 | 335 (4) | nada que rescatar (salvo assets comunes) |
| codex-AV18-03 | 15/10 | 12 | 23 | 333 (1) | nada que rescatar (salvo assets comunes) |
| codex-AV18-04 | 15/10 | 12 | 23 | 335 (5) | nada que rescatar (salvo assets comunes) |
| codex-AV18-06 | 15/10 | 12 | 23 | 337 (11) | nada que rescatar (salvo assets comunes) |
| codex-ESCENAS-02 | 19/10 | 12 | 34 | 282 (9) | revisar a mano (bajo) |
| codex-ESCENAS-03 | 19/10 | 12 | 34 | 282 (11) | revisar a mano (bajo) |
| codex-ESCENAS-04 | 21/10 | 3 | 36 | 252 (10) | revisar a mano (bajo) |
| codex-ESCENAS-05 | 21/10 | 3 | 36 | 252 (11) | revisar a mano (bajo) |
| codex-ESCENAS-06 | 21/10 | 3 | 36 | 251 (15) | revisar a mano (bajo) |
| codex-ESCENAS-07 | 19/10 | 12 | 34 | 282 (17) | revisar a mano (bajo) |
| codex-GORRAN-MAPAS-02 | 22/10 | 3 | 35 | 276 (35) | revisar a mano (bajo) |
| codex-GORRAN-V2-PLAN | 22/10 | 3 | 35 | 251 (1) | nada que rescatar |
| qwen-AV16-09 | 15/10 | 12 | 22 | 331 (1) | nada que rescatar (salvo assets comunes) |
| qwen-AV16-09b | 15/10 | 12 | 22 | 342 (16) | nada que rescatar (salvo assets comunes) |
| qwen-AV19-05 | 15/10 | 12 | 22 | 344 (10) | nada que rescatar (salvo assets comunes) |
| qwen-AV19-09 | 20/10 | 16 | 34 | 281 (1) | revisar a mano |
| zcode-AV17-01 | 15/10 | 12 | 23 | 331 (3) | nada que rescatar (salvo assets comunes) |
| zcode-AV17-01.viejo | 15/10 | 13 | 22 | 333 (2) | nada que rescatar (salvo assets comunes) |
| software-copia-30-10 | 30/10 07:24 (rama agente/qwen/integracion-av19-28-10 + sin commitear) | 16 | 35 | 275 (17) | rescatar ficheros nuevos + revisar a mano |

## Assets comunes que faltan en caa (aparecen en casi todas las fotos)

| asset | tamaño | donde esta | estado en caa | veredicto |
|---|---|---|---|---|
| `assets/textures/editor_{biomes_creatures_equipment,construction_tiles,enemy_library_v1,object_icons,race_npcs_01..04}.png` (8) | 1,7-2,1 MB c/u (~15 MB) | 19 fotos (14-20/10) + copia de Oriol; NO en ESCENAS-04..06/GORRAN/PILOTO | caa tiene los `.md` descriptivos (`editor_race_npcs.md`, ...) y `editor/src/level_editor.cpp:465` carga `editor_race_npcs_01.png`; el PNG no existe en ningun commit del linaje (ni en a523d6aa/95ef783a) | **RESTAURAR** desde `recuperat/software-copia-30-10` (hash unico compartido por las 19 fotos: misma version) |
| `assets/textures/pixellab/lote2/con.png` | pequeño | codex-AV13-08, AV15-02a/b, zcode-AV17-01.viejo (NO esta en la copia de Oriol) | `pixellab/iconos.json:33` de caa referencia `lote2/con.png` y el fichero no existe (referencia colgante) | **RESTAURAR** desde `recuperat/arbre/codex-AV13-08`; comprobar que la version coincide con el estilo del resto de lote2 |
| `assets/textures/personajes/cuerpos/ojos_grande.png` | 5 KB | todas las fotos 14/10-30/10 | caa tiene ojos_{bajo,medio,pequeno}.png y `tools/capa_ojos_iris.py` los genera por talla (`ojos_<talla>.png`); falta la talla `grande` | **REVISAR**: probar a regenerar con `capa_ojos_iris.py`; si no, tomar la de la foto |
| `personajes/equipo/{torso,capa}_medio_sin_piernas.png` | 519 KB / 15 KB | todas | caa los trata en `tools/actores_cuerpos.py` (`_sin_piernas`) pero no tiene estos PNG; el `personajes.json` de la foto los enumera, el de caa ya no | **REVISAR (prioridad baja)**: puede ser el esquema antiguo (pre CAR-2 / .filas) |
| `assets/textures/monumentos/Untitled-1.psd` | 3,0 MB | solo copia de Oriol | no existe | fuente Photoshop de monumentos; **conservar** (decide Oriol; no es para el repo si pesa) |
| `assets/textures/pixellab/props 2.json` | 7 KB | casi todas | caa tiene `props.json` | duplicado de Finder: **ignorar** |

## Secciones por foto

### PILOTO-29-10-E1  (`recuperat/arbre/PILOTO-29-10-E1`)
- Commit foto: ver `git log -1 recuperat/arbre/PILOTO-29-10-E1` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 29/10.
- Pista TABLERO/sesiones/out: out/fabrica/PILOTO-29-10/INFORME.md: PIL-E1 VALIDADA (rama agente/claude-sonnet/PIL-E1, 1beb819), `entregar` parado; NO integrada en caa.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4771 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 129 'version antigua, superada'; 1 'revisar' (de los cuales 1 propios).
  - De los 1 'revisar': 0 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 0 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: tests/Game 1
  - Modificados 'revisar':
    - `tests/Game/ClasesSpTests.cpp` [8, 1]
- Nota: tests/Game/ClasesSpTests.cpp: +8 lineas (arregla el caso `siempre:velocidad_0`). Es el resultado real del piloto.
- **Veredicto: rescatar fichero modificado**

### PILOTO-29-10-E2  (`recuperat/arbre/PILOTO-29-10-E2`)
- Commit foto: ver `git log -1 recuperat/arbre/PILOTO-29-10-E2` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 29/10.
- Pista TABLERO/sesiones/out: INFORME.md: PIL-E2 HECHA (agente/opencode/PIL-E2, e64da06); NO integrada en caa.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4771 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 128 'version antigua, superada'; 1 'revisar' (de los cuales 1 propios).
  - De los 1 'revisar': 0 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 0 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: cmake 1
  - Modificados 'revisar':
    - `cmake/AparienciaTests.cmake` [2, 55]
- Nota: cmake/AparienciaTests.cmake: solo las 2 lineas `LABELS "unit;gl-free;lenta"` + `TIMEOUT 600` de test_personajes_capas. OJO: el resto del fichero de la foto es MAS VIEJO que caa (le faltan test_editor_apariencia_razas/colores): tomar solo esas 2 lineas.
- **Veredicto: rescatar fichero modificado**

### PLAU01-R1  (`recuperat/arbre/PLAU01-R1`)
- Commit foto: ver `git log -1 recuperat/arbre/PLAU01-R1` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 29/10.
- Pista TABLERO/sesiones/out: out/fabrica/PLAU01-R1/INFORME.md (analisis; sin cambios de motor).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4771 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 131 'version antigua, superada'; 0 'revisar' (de los cuales 0 propios).
- Nota: Arbol de MotorGraphico identico a caa salvo ficheros ya borrados a proposito.
- **Veredicto: nada que rescatar**

### agy-E-00-SIMULACRO  (`recuperat/arbre/agy-E-00-SIMULACRO`)
- Commit foto: ver `git log -1 recuperat/arbre/agy-E-00-SIMULACRO` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 29-31/10.
- Pista TABLERO/sesiones/out: out/fabrica/agy/nocturno.log: `fin rc=0 ... commit=ninguno`; simulacro sin cambios.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4771 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 129 'version antigua, superada'; 0 'revisar' (de los cuales 0 propios).
- Nota: Arbol identico a caa en MotorGraphico.
- **Veredicto: nada que rescatar**

### REPLICA-R1-U3-02  (`recuperat/arbre/REPLICA-R1-U3-02`)
- Commit foto: ver `git log -1 recuperat/arbre/REPLICA-R1-U3-02` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 30/10.
- Pista TABLERO/sesiones/out: Pla U U3-02 integrado en caa (0d376a5d) + replica-r1/U3-02 (gestionada por ti).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4775 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 128 'version antigua, superada'; 1 'revisar' (de los cuales 1 propios).
  - De los 1 'revisar': 1 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 0 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: CMakeLists.txt 1
  - Modificados 'revisar':
    - `CMakeLists.txt` [0, 20]
- Nota: 1 fichero (CMakeLists.txt) cuyo contenido es subconjunto del de caa.
- **Veredicto: nada que rescatar**

### REPLICA-R1-U3-05  (`recuperat/arbre/REPLICA-R1-U3-05`)
- Commit foto: ver `git log -1 recuperat/arbre/REPLICA-R1-U3-05` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 30/10.
- Pista TABLERO/sesiones/out: U3-05 integrado en caa + replica-r1/U3-05.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4778 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 128 'version antigua, superada'; 4 'revisar' (de los cuales 4 propios).
  - De los 4 'revisar': 2 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 0 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: CMakeLists.txt 1, include/Game 1, src/Game 1, tests/Game 1
  - Modificados 'revisar':
    - `CMakeLists.txt` [1, 22]
    - `include/Game/CursorClip.h` [1, 1]
    - `src/Game/CursorClip.cpp` [0, 1]
    - `tests/Game/CursorClipTests.cpp` [0, 33]
- Nota: 4 ficheros: CMakeLists.txt, CursorClip.h/.cpp, CursorClipTests.cpp = version previa a los arreglos de revision (caa tiene la mas nueva).
- **Veredicto: nada que rescatar**

### REPLICA-R1-U3-07  (`recuperat/arbre/REPLICA-R1-U3-07`)
- Commit foto: ver `git log -1 recuperat/arbre/REPLICA-R1-U3-07` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 30/10.
- Pista TABLERO/sesiones/out: U3-07 integrado en caa + replica-r1/U3-07.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4775 ficheros en el alcance.
- Nuevos: 0.
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 85.
- Modificados: 128 'version antigua, superada'; 4 'revisar' (de los cuales 4 propios).
  - De los 4 'revisar': 1 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 0 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: CMakeLists.txt 1, include/Core 1, src/Core 1, tests/Core 1
  - Modificados 'revisar':
    - `CMakeLists.txt` [1, 22]
    - `include/Core/Math/Interpolacion.h` [1, 2]
    - `src/Core/Math/Interpolacion.cpp` [1, 9]
    - `tests/Core/InterpolacionTests.cpp` [0, 35]
- Nota: Interpolacion.h/.cpp/test: la foto trae `a*(1-p)+b*p` (version simple); caa trae la version endurecida (guardas de p, monotonia, mitades si b-a desborda). Caa es la buena.
- **Veredicto: nada que rescatar**

### REPLICA-R1-revisiones  (`recuperat/arbre/REPLICA-R1-revisiones`)
- Commit foto: ver `git log -1 recuperat/arbre/REPLICA-R1-revisiones` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 30/10.
- Pista TABLERO/sesiones/out: No es un arbol de codigo.
- 11 .md de revision codex (U3-02.E01..U3-07.E04). Existen tambien en ~/onegai-ramas/REPLICA-R1-revisiones/. Sin MotorGraphico.
- **Veredicto: fuera de alcance**

### codex-AV13-08  (`recuperat/arbre/codex-AV13-08`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV13-08` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 14/10.
- Pista TABLERO/sesiones/out: TABLERO l.24/71: AV13-08 plantillas de area; sesiones/2026-10-14-codex-av13-08-av15-02a.md. Lo posterior (AV13-35, 14/10) borro el combate viejo; D0016 retiro QuestLogState (AV14-22).
- Layout de la foto: raiz `MotorGraphico/`; 4323 ficheros en el alcance.
- Nuevos: 27. De ellos 11 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
  - Distintos de los comunes:
    - `assets/textures/`: race_npc_idle.json
    - `assets/textures/pixellab/lote2/`: con.png
    - `cmake/`: BattleMenuTests.cmake, TargetSelectionTests.cmake
    - `examples/`: demo_battle.cpp
    - `include/Game/`: BattleMenuActions.h, BattleMenuState.h, QuestLogState.h, TargetSelection.h
    - `src/Game/`: BattleMenuActions.cpp, BattleMenuState.cpp, QuestLogState.cpp, TargetSelection.cpp
    - `tests/Game/`: BattleMenuTests.cpp, TargetSelectionTests.cpp
    - `tools/`: gen_race_npc_sprites.py
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 82.
- Modificados: 15 'version antigua, superada'; 395 'revisar' (de los cuales 1 propios).
  - De los 395 'revisar': 65 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 59 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 75, assets/levels 52, src/Game 31, tools 31, include/Game 23, tests/Game 20, src/HUD 16, examples 14
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `CMakeLists.txt` [25, 108]
- Nota: Layout antiguo: la raiz de la foto es `MotorGraphico/` (no `MotorGraphico-main/MotorGraphico`). Se comparo igualmente.
- **Veredicto: rescatar ficheros nuevos (solo assets sueltos)**

### codex-AV15-02a  (`recuperat/arbre/codex-AV15-02a`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV15-02a` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 14/10.
- Pista TABLERO/sesiones/out: TABLERO l.200: AV15-02a tabla de 17 lugares aprobada; sesiones/2026-10-14-codex-av13-08-av15-02a.md.
- Layout de la foto: raiz `MotorGraphico/`; 4319 ficheros en el alcance.
- Nuevos: 27. De ellos 11 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
  - Distintos de los comunes:
    - `assets/textures/`: race_npc_idle.json
    - `assets/textures/pixellab/lote2/`: con.png
    - `cmake/`: BattleMenuTests.cmake, TargetSelectionTests.cmake
    - `examples/`: demo_battle.cpp
    - `include/Game/`: BattleMenuActions.h, BattleMenuState.h, QuestLogState.h, TargetSelection.h
    - `src/Game/`: BattleMenuActions.cpp, BattleMenuState.cpp, QuestLogState.cpp, TargetSelection.cpp
    - `tests/Game/`: BattleMenuTests.cpp, TargetSelectionTests.cpp
    - `tools/`: gen_race_npc_sprites.py
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 82.
- Modificados: 15 'version antigua, superada'; 395 'revisar' (de los cuales 1 propios).
  - De los 395 'revisar': 65 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 59 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 75, assets/levels 52, src/Game 31, tools 31, include/Game 23, tests/Game 20, src/HUD 16, examples 14
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `CMakeLists.txt` [23, 108]
- Nota: Layout antiguo, igual que AV13-08.
- **Veredicto: rescatar ficheros nuevos (solo assets sueltos)**

### codex-AV15-02b  (`recuperat/arbre/codex-AV15-02b`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV15-02b` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 14/10.
- Pista TABLERO/sesiones/out: TABLERO l.197: AV15-02b terminada e incluida en claude_dialogos04.
- Layout de la foto: raiz `MotorGraphico/`; 4326 ficheros en el alcance.
- Nuevos: 27. De ellos 11 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
  - Distintos de los comunes:
    - `assets/textures/`: race_npc_idle.json
    - `assets/textures/pixellab/lote2/`: con.png
    - `cmake/`: BattleMenuTests.cmake, TargetSelectionTests.cmake
    - `examples/`: demo_battle.cpp
    - `include/Game/`: BattleMenuActions.h, BattleMenuState.h, QuestLogState.h, TargetSelection.h
    - `src/Game/`: BattleMenuActions.cpp, BattleMenuState.cpp, QuestLogState.cpp, TargetSelection.cpp
    - `tests/Game/`: BattleMenuTests.cpp, TargetSelectionTests.cpp
    - `tools/`: gen_race_npc_sprites.py
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 82.
- Modificados: 15 'version antigua, superada'; 398 'revisar' (de los cuales 3 propios).
  - De los 398 'revisar': 66 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 59 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 75, assets/levels 52, src/Game 31, tools 31, include/Game 23, tests/Game 21, src/HUD 16, examples 14
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `CMakeLists.txt` [26, 108]
    - `cmake/Lugares.cmake` [2, 10]
    - `tests/Game/LugaresTests.cpp` [1, 96]
- Nota: Layout antiguo. Integrada segun TABLERO.
- **Veredicto: rescatar ficheros nuevos (solo assets sueltos)**

### codex-AV16-08  (`recuperat/arbre/codex-AV16-08`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV16-08` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.185: AV16-08 terminado (65365fc + 43fa13d), El Zurron del Vado. sesiones/2026-10-15-codex-av16-08-rama.md. Integracion de Fable (l.188) `agente/fable/ciudad`.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4464 ficheros en el alcance.
- Nuevos: 14. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
  - Distintos de los comunes:
    - `assets/textures/`: pnj_pesca_caza.json, pnj_pesca_caza.png
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 332 'revisar' (de los cuales 9 propios).
  - De los 332 'revisar': 55 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 58, tools 35, src/Game 27, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 11
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/sprites.json` [82, 100]
    - `assets/objects/ciudad_objetos.json` [16, 16]
    - `assets/proyectos/boundington.json` [3, 52]
    - `assets/levels/interior_pesca_caza.json` [1, 9]
    - `assets/textures/iconos.json` [1, 29]
    - `tools/gen_iconos.py` [1, 67]
    - `assets/catalogs/lugares.json` [0, 382]
    - `assets/textures/iconos.png` [binario]
    - `tests/Game/LugaresTests.cpp` [0, 38]
- Nota: pnj_pesca_caza.{json,png}: superado, caa ya mete `pesca_caza` dentro de pnj_boundington.json. 'props 2.json' = duplicado de Finder.
- **Veredicto: nada que rescatar (salvo assets comunes)**

### codex-AV18-01  (`recuperat/arbre/codex-AV18-01`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV18-01` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.196: AV18-01 economia terminado (rama 9e6c25b). AV18 cerrado por Fable 16/10 (l.190).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4458 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 333 'revisar' (de los cuales 0 propios).
  - De los 333 'revisar': 54 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 37, src/Game 27, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 11
- **Veredicto: nada que rescatar (salvo assets comunes)**

### codex-AV18-02  (`recuperat/arbre/codex-AV18-02`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV18-02` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.19/191/195: AV18-02/03 fusionados en main 4b5ae22.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4467 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 335 'revisar' (de los cuales 4 propios).
  - De los 335 'revisar': 57 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 37, src/Game 28, include/Game 19, tests/Game 18, src/HUD 14, assets/catalogs 12
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `CMakeLists.txt` [2, 44]
    - `src/Game/ContenedoresSesion.cpp` [1, 7]
    - `src/Game/GameSessionInteraccion.cpp` [1, 3]
    - `assets/catalogs/contenedores_tablas.json` [0, 5220]
- **Veredicto: nada que rescatar (salvo assets comunes)**

### codex-AV18-03  (`recuperat/arbre/codex-AV18-03`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV18-03` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.19/192: AV18-03 integrado en main 4b5ae22.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4475 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 23 'version antigua, superada'; 333 'revisar' (de los cuales 1 propios).
  - De los 333 'revisar': 57 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 39, src/Game 27, include/Game 19, tests/Game 18, src/HUD 14, assets/catalogs 10
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `CMakeLists.txt` [2, 43]
- **Veredicto: nada que rescatar (salvo assets comunes)**

### codex-AV18-04  (`recuperat/arbre/codex-AV18-04`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV18-04` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.193: AV18-04 98369be; l.190 Fable: simulador a 0 (AV18-04b).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4483 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 23 'version antigua, superada'; 335 'revisar' (de los cuales 5 propios).
  - De los 335 'revisar': 57 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 41, src/Game 27, include/Game 19, tests/Game 18, src/HUD 14, assets/catalogs 10
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `tools/rutas_economia.json` [103, 2111]
    - `tools/simular_economia.py` [5, 50]
    - `CMakeLists.txt` [2, 42]
    - `tools/test_validar_botin.py` [1, 5]
    - `tools/validar_botin.py` [0, 3]
- Nota: tools/rutas_economia.json y simular_economia.py: la foto trae el recorrido de rutas de codex; caa trae el reescrito por Fable (AV18-04b).
- **Veredicto: nada que rescatar (salvo assets comunes)**

### codex-AV18-06  (`recuperat/arbre/codex-AV18-06`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-AV18-06` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.194: AV18-06 ff64e5c montaje nativo; l.190 cierre AV18 por Fable.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4487 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 23 'version antigua, superada'; 337 'revisar' (de los cuales 11 propios).
  - De los 337 'revisar': 61 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 42, src/Game 27, include/Game 19, tests/Game 19, src/HUD 14, assets/catalogs 10
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `tools/simular_economia.py` [23, 75]
    - `tools/validar_todo.py` [9, 64]
    - `tests/Game/MonstruosAccionesTests.cpp` [3, 3]
    - `CMakeLists.txt` [1, 39]
    - `src/Game/CartasTiradas.cpp` [1, 2]
    - `assets/catalogs/monstruos_boundington_cartas.json` [0, 79]
    - `assets/catalogs/skills.json` [0, 598]
    - `assets/objects/boundington_enemigos.json` [0, 127]
    - `cmake/SimularEconomia.cmake` [0, 11]
    - `include/Game/CartasEnCombate.h` [0, 6]
    - `tools/generar_acciones_monstruos.py` [0, 1]
- **Veredicto: nada que rescatar (salvo assets comunes)**

### codex-ESCENAS-02  (`recuperat/arbre/codex-ESCENAS-02`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-ESCENAS-02` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 19/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-19-codex-escenas02.md. Sin linea de integracion en TABLERO; narrativa integrada despues (sesion 26/10 codex-integracion-reciente: 'integracion narrativa', claude/integracion-pl).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4532 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 34 'version antigua, superada'; 282 'revisar' (de los cuales 9 propios).
  - De los 282 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 64, tools 36, src/Game 13, include/Game 12, assets/maps 10, tests/Game 10, src/Engine 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/pnj_boundington.json` [1924, 4822]
    - `assets/adventures/boundington_segundo_dia.json` [674, 3136]
    - `assets/textures/pnj_boundington_2.json` [460, 9002]
    - `assets/cinematicas/cinematicas.json` [10, 4557]
    - `assets/levels/ciudad_este.json` [9, 521]
    - `tools/actores_pnj.py` [8, 583]
    - `assets/objects/boundington_npcs.json` [0, 2046]
    - `assets/textures/pnj_boundington.png` [binario]
    - `assets/textures/pnj_boundington_2.png` [binario]
- Nota: Los guiones/PNJ (pnj_boundington.*, boundington_segundo_dia.json, cinematicas.json) existen en caa en version mas nueva (pnj_boundington.png pesa 2,4 MB en caa vs 0,86 MB en la foto).
- **Veredicto: revisar a mano (bajo)**

### codex-ESCENAS-03  (`recuperat/arbre/codex-ESCENAS-03`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-ESCENAS-03` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 19/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-19-codex-escenas03.md.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4532 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 34 'version antigua, superada'; 282 'revisar' (de los cuales 11 propios).
  - De los 282 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 64, tools 36, src/Game 13, include/Game 12, assets/maps 10, tests/Game 10, src/Engine 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/pnj_boundington.json` [1876, 4802]
    - `assets/textures/pnj_boundington_2.json` [993, 7469]
    - `assets/adventures/boundington_segundo_dia.json` [642, 1446]
    - `assets/levels/ciudad_centro.json` [332, 125]
    - `assets/adventures/boundington_ocaso.json` [233, 1696]
    - `assets/cinematicas/cinematicas.json` [26, 4256]
    - `tools/actores_pnj.py` [8, 512]
    - `assets/objects/boundington_npcs.json` [0, 1887]
    - `assets/rules/actos.json` [0, 7]
    - `assets/textures/pnj_boundington.png` [binario]
    - `assets/textures/pnj_boundington_2.png` [binario]
- Nota: Idem ESCENAS-02. ciudad_centro.json: 332 lineas propias.
- **Veredicto: revisar a mano (bajo)**

### codex-ESCENAS-04  (`recuperat/arbre/codex-ESCENAS-04`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-ESCENAS-04` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 21/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-21-codex-escenas04.md.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4561 ficheros en el alcance.
- Nuevos: 3. De ellos 3 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 36 'version antigua, superada'; 252 'revisar' (de los cuales 10 propios).
  - De los 252 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 46, tools 33, src/Game 12, include/Game 11, tests/Game 10, src/Engine 9, src/HUD 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/pnj_boundington_2.json` [1980, 4633]
    - `assets/textures/pnj_boundington.json` [1758, 4611]
    - `assets/adventures/boundington_ocaso.json` [152, 1094]
    - `assets/textures/pnj_boundington_3.json` [135, 10142]
    - `assets/cinematicas/cinematicas.json` [39, 3096]
    - `tools/actores_pnj.py` [8, 390]
    - `assets/objects/boundington_npcs.json` [0, 1548]
    - `assets/textures/pnj_boundington.png` [binario]
    - `assets/textures/pnj_boundington_2.png` [binario]
    - `assets/textures/pnj_boundington_3.png` [binario]
- Nota: Idem. Sin los atlas editor_*.png.
- **Veredicto: revisar a mano (bajo)**

### codex-ESCENAS-05  (`recuperat/arbre/codex-ESCENAS-05`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-ESCENAS-05` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 21/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-21-codex-escenas05.md.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4561 ficheros en el alcance.
- Nuevos: 3. De ellos 3 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 36 'version antigua, superada'; 252 'revisar' (de los cuales 11 propios).
  - De los 252 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 46, tools 33, src/Game 12, include/Game 11, tests/Game 10, src/Engine 9, src/HUD 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/pnj_boundington_2.json` [2002, 4622]
    - `assets/textures/pnj_boundington.json` [1763, 4502]
    - `assets/textures/pnj_boundington_3.json` [335, 9288]
    - `assets/adventures/boundington_ocaso.json` [72, 902]
    - `assets/cinematicas/cinematicas.json` [39, 2726]
    - `tools/actores_pnj.py` [8, 312]
    - `assets/levels/ciudad_centro_ocaso.json` [5, 68]
    - `assets/objects/boundington_npcs.json` [0, 1396]
    - `assets/textures/pnj_boundington.png` [binario]
    - `assets/textures/pnj_boundington_2.png` [binario]
    - `assets/textures/pnj_boundington_3.png` [binario]
- Nota: Idem.
- **Veredicto: revisar a mano (bajo)**

### codex-ESCENAS-06  (`recuperat/arbre/codex-ESCENAS-06`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-ESCENAS-06` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 21/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-21-codex-escenas06.md: 'no fusionar la base antigua escenas01'. Final/huida de las Afueras.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4561 ficheros en el alcance.
- Nuevos: 3. De ellos 3 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 36 'version antigua, superada'; 251 'revisar' (de los cuales 15 propios).
  - De los 251 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 45, tools 33, src/Game 12, include/Game 11, tests/Game 10, src/Engine 9, src/HUD 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/pnj_boundington_2.json` [1749, 4691]
    - `assets/textures/pnj_boundington.json` [1618, 4644]
    - `assets/textures/pnj_boundington_3.json` [777, 7369]
    - `assets/textures/sprites.json` [80, 89]
    - `assets/cinematicas/cinematicas.json` [25, 1981]
    - `assets/adventures/boundington_ocaso.json` [23, 545]
    - `tools/actores_pnj.py` [8, 197]
    - `assets/levels/ciudad_centro_ocaso.json` [4, 4]
    - `assets/levels/ciudad_oeste.json` [4, 16]
    - `assets/levels/interior_cuartel.json` [3, 3]
    - `assets/levels/afueras_boundington_ocaso.json` [0, 0]
    - `assets/objects/boundington_npcs.json` [0, 1124]
    - `assets/textures/pnj_boundington.png` [binario]
    - `assets/textures/pnj_boundington_2.png` [binario]
    - `assets/textures/pnj_boundington_3.png` [binario]
- Nota: Idem. Trae afueras_boundington_ocaso.json propio (version de codex).
- **Veredicto: revisar a mano (bajo)**

### codex-ESCENAS-07  (`recuperat/arbre/codex-ESCENAS-07`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-ESCENAS-07` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 19/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-19-codex-escenas07.md (rama sobre fa1dd54=ESCENAS-03).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4532 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 34 'version antigua, superada'; 282 'revisar' (de los cuales 17 propios).
  - De los 282 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 64, tools 36, src/Game 13, include/Game 12, assets/maps 10, tests/Game 10, src/Engine 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/pnj_boundington.json` [1933, 4663]
    - `assets/textures/pnj_boundington_2.json` [1085, 7208]
    - `assets/adventures/boundington_segundo_dia.json` [892, 615]
    - `assets/levels/ciudad_centro.json` [369, 113]
    - `assets/adventures/boundington_ocaso.json` [263, 1516]
    - `assets/textures/sprites.json` [78, 94]
    - `assets/levels/ciudad_surysal.json` [29, 105]
    - `assets/cinematicas/cinematicas.json` [23, 4022]
    - `assets/levels/interior_herreria.json` [12, 11]
    - `tools/actores_pnj.py` [8, 472]
    - `assets/levels/interior_sastreria.json` [7, 0]
    - `assets/levels/interior_iglesia_s1.json` [4, 42]
    - `assets/levels/tienda_abandonada.json` [3, 91]
    - `assets/objects/boundington_npcs.json` [0, 1756]
    - `assets/rules/actos.json` [0, 0]
    - `assets/textures/pnj_boundington.png` [binario]
    - `assets/textures/pnj_boundington_2.png` [binario]
- Nota: Idem; ultimo de la cadena 02->03->07.
- **Veredicto: revisar a mano (bajo)**

### codex-GORRAN-MAPAS-02  (`recuperat/arbre/codex-GORRAN-MAPAS-02`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-GORRAN-MAPAS-02` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 22/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-22-codex-gorran-mapas02.md (commit e50331e, base claude/gorran-v2 6b6368b). Gorran ampliado integrado en claude/integracion-pl (sesion 26/10).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4618 ficheros en el alcance.
- Nuevos: 3. De ellos 3 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 35 'version antigua, superada'; 276 'revisar' (de los cuales 35 propios).
  - De los 276 'revisar': 57 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 57 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 70, assets/levels 69, tools 36, src/Game 12, include/Game 11, tests/Game 10, src/Engine 9, src/HUD 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/sprites.json` [78, 41]
    - `assets/levels/mina_gorran.json` [41, 482]
    - `assets/maps/mina_gorran.tmx` [41, 41]
    - `assets/catalogs/lugares.json` [7, 22]
    - `assets/levels/ciudad_oeste.json` [2, 261]
    - `assets/proyectos/boundington.json` [2, 9]
    - `tools/gen_gorran_mapas02.py` [2, 10]
    - `assets/levels/aldea_cojo_p1.json` [1, 1]
    - `assets/levels/aldea_herreria_p1.json` [1, 8]
    - `assets/levels/aldea_joel_p1.json` [1, 1]
    - `assets/levels/aldea_tajo_p1.json` [1, 1]
    - `assets/levels/galerias_gorran.json` [1, 564]
    - `assets/levels/mina2_gorran.json` [1, 553]
    - `assets/levels/recinto_gorran.json` [1, 142]
    - `tools/gen_gorran_v2.py` [1, 7]
    - `assets/levels/aldea_almacen_p0.json` [0, 7]
    - `assets/levels/aldea_almacen_p1.json` [0, 7]
    - `assets/levels/aldea_cojo_p0.json` [0, 14]
    - `assets/levels/aldea_cuartel_p0.json` [0, 14]
    - `assets/levels/aldea_familias_p0.json` [0, 14]
    - `assets/levels/aldea_familias_p1.json` [0, 7]
    - `assets/levels/aldea_herreria_p0.json` [0, 7]
    - `assets/levels/aldea_joel_p0.json` [0, 7]
    - `assets/levels/aldea_joel_p2.json` [0, 7]
    - `assets/levels/aldea_serra_p0.json` [0, 28]
    - `assets/levels/aldea_serra_p1.json` [0, 14]
    - `assets/levels/aldea_tajo_p0.json` [0, 56]
    - `assets/levels/antecova_gorran.json` [0, 524]
    - `assets/levels/cueva_gorran.json` [0, 3897]
    - `assets/levels/galeria_minas_cojo.json` [0, 399]
    - `assets/levels/grieta_cojo.json` [0, 112]
    - `assets/levels/rio_gorran.json` [0, 1822]
    - `assets/objects/boundington_av19_objetos.json` [0, 379]
    - `tools/ciudad_a_excel.py` [0, 37]
    - `tools/excel_a_ciudad.py` [0, 77]
- Nota: 35 ficheros propios (aldea_*, mina_gorran, galerias, tools/gen_gorran_*): TODOS existen en caa; 20 son subconjunto de caa, el resto tiene 1-78 lineas distintas (mina_gorran.json/tmx 41, sprites.json 78).
- **Veredicto: revisar a mano (bajo)**

### codex-GORRAN-V2-PLAN  (`recuperat/arbre/codex-GORRAN-V2-PLAN`)
- Commit foto: ver `git log -1 recuperat/arbre/codex-GORRAN-V2-PLAN` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 22/10.
- Pista TABLERO/sesiones/out: sesiones/2026-10-22-codex-gorran-v2-plan.md (commit 25fac97; solo planificacion).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4559 ficheros en el alcance.
- Nuevos: 3. De ellos 3 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 35 'version antigua, superada'; 251 'revisar' (de los cuales 1 propios).
  - De los 251 'revisar': 39 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 57 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 70, assets/levels 46, tools 34, src/Game 12, include/Game 11, tests/Game 10, src/Engine 9, src/HUD 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/textures/sprites.json` [78, 90]
- Nota: Solo sprites.json (78 lineas) de diferencia.
- **Veredicto: nada que rescatar**

### qwen-AV16-09  (`recuperat/arbre/qwen-AV16-09`)
- Commit foto: ver `git log -1 recuperat/arbre/qwen-AV16-09` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.20: AV16-09 hecha; integrada por Fable en agente/fable/ciudad (l.188).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4457 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 331 'revisar' (de los cuales 1 propios).
  - De los 331 'revisar': 54 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 34, src/Game 27, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 11
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/levels/ciudad_centro.json` [2897, 3509]
- **Veredicto: nada que rescatar (salvo assets comunes)**

### qwen-AV16-09b  (`recuperat/arbre/qwen-AV16-09b`)
- Commit foto: ver `git log -1 recuperat/arbre/qwen-AV16-09b` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.20: AV16-09b hecha; integrada (l.188).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4474 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 342 'revisar' (de los cuales 16 propios).
  - De los 342 'revisar': 53 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 62, tools 35, src/Game 27, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 11
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/levels/ciudad_oeste.json` [797, 1394]
    - `assets/levels/interior_castillo.json` [575, 596]
    - `assets/levels/interior_banco.json` [356, 344]
    - `assets/levels/interior_iglesia_s1.json` [337, 443]
    - `assets/levels/interior_armeria.json` [313, 304]
    - `assets/levels/interior_banco_s1.json` [297, 291]
    - `assets/levels/prado_del_rio.json` [191, 175]
    - `tools/repartir_botin.py` [188, 268]
    - `assets/levels/ciudad_centro.json` [183, 435]
    - `assets/levels/montana_gorran.json` [29, 3505]
    - `assets/levels/afueras_boundington.json` [14, 216]
    - `assets/levels/ciudad_este.json` [7, 677]
    - `tools/av19_niveles.py` [4, 142]
    - `assets/proyectos/boundington.json` [3, 48]
    - `tools/gen_niveles_av19.py` [2, 2]
    - `assets/objects/boundington_av19_objetos.json` [1, 951]
- Nota: 16 niveles propios: version de ciudad_oeste, interiores... mas vieja que caa.
- **Veredicto: nada que rescatar (salvo assets comunes)**

### qwen-AV19-05  (`recuperat/arbre/qwen-AV19-05`)
- Commit foto: ver `git log -1 recuperat/arbre/qwen-AV19-05` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.20: AV19-05 hecha (tienda/mansion).
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4471 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 344 'revisar' (de los cuales 10 propios).
  - De los 344 'revisar': 55 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 62, tools 37, src/Game 27, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 11
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `tools/repartir_botin.py` [143, 401]
    - `assets/levels/ciudad_centro.json` [106, 697]
    - `assets/maps/ciudad_centro_ocaso.tmx` [66, 106]
    - `assets/levels/ciudad_centro_ocaso.json` [31, 11451]
    - `assets/levels/afueras_boundington.json` [12, 219]
    - `assets/proyectos/boundington.json` [3, 49]
    - `tools/gen_niveles_av19.py` [3, 46]
    - `assets/levels/ciudad_este.json` [2, 753]
    - `tools/av19_niveles.py` [1, 274]
    - `assets/objects/boundington_av19_objetos.json` [0, 967]
- **Veredicto: nada que rescatar (salvo assets comunes)**

### qwen-AV19-09  (`recuperat/arbre/qwen-AV19-09`)
- Commit foto: ver `git log -1 recuperat/arbre/qwen-AV19-09` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 20/10.
- Pista TABLERO/sesiones/out: TABLERO l.21: AV19-09/10/11/12 hechas (Qwen). sesiones/2026-10-20-qwen-av19-09-ocaso-huida.md, ...av19-10-afueras-ocaso.md.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4541 ficheros en el alcance.
- Nuevos: 16. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
  - Distintos de los comunes:
    - `assets/levels/`: afueras_ocaso.json
    - `assets/maps/`: afueras_ocaso.tmx
    - `tools/`: av19_decorado.py, av19_ocaso.py
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 34 'version antigua, superada'; 281 'revisar' (de los cuales 1 propios).
  - De los 281 'revisar': 35 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 66, tools 34, src/Game 13, include/Game 12, tests/Game 10, assets/maps 9, src/Engine 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/objects/boundington_av19_objetos.json` [34, 666]
- Nota: NUEVOS reales: afueras_ocaso.json/.tmx, tools/av19_ocaso.py, tools/av19_decorado.py (ver seccion Afueras al ocaso).
- **Veredicto: revisar a mano**

### zcode-AV17-01  (`recuperat/arbre/zcode-AV17-01`)
- Commit foto: ver `git log -1 recuperat/arbre/zcode-AV17-01` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: TABLERO l.23: AV17-01 y AV17-02 HECHOS; sesiones/2026-10-15-zcode-av17-01-segundas-oportunidades.md.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4456 ficheros en el alcance.
- Nuevos: 12. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 23 'version antigua, superada'; 331 'revisar' (de los cuales 3 propios).
  - De los 331 'revisar': 55 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 35, src/Game 27, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 10
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/adventures/boundington_primer_dia.json` [18432, 27788]
    - `assets/adventures/boundington_segundo_dia.json` [881, 7661]
    - `assets/adventures/boundington_ocaso.json` [240, 1852]
- Nota: 3 aventuras propias (primer_dia 18.432 lineas no presentes en caa = versiones antiguas de los guiones; caa tiene las posteriores de ESCENAS/Fable).
- **Veredicto: nada que rescatar (salvo assets comunes)**

### zcode-AV17-01.viejo  (`recuperat/arbre/zcode-AV17-01.viejo`)
- Commit foto: ver `git log -1 recuperat/arbre/zcode-AV17-01.viejo` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 15/10.
- Pista TABLERO/sesiones/out: Idem; foto del worktree previo descolgado (layout raiz MotorGraphico).
- Layout de la foto: raiz `MotorGraphico/`; 4457 ficheros en el alcance.
- Nuevos: 13. De ellos 12 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json).
  - Distintos de los comunes:
    - `assets/textures/pixellab/lote2/`: con.png
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 22 'version antigua, superada'; 333 'revisar' (de los cuales 2 propios).
  - De los 333 'revisar': 55 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 57, tools 35, src/Game 28, include/Game 20, tests/Game 17, src/HUD 14, assets/catalogs 11
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/adventures/boundington_primer_dia.json` [18343, 27788]
    - `assets/adventures/boundington_segundo_dia.json` [883, 7762]
- **Veredicto: nada que rescatar (salvo assets comunes)**

### software-copia-30-10  (`recuperat/software-copia-30-10`)
- Commit foto: ver `git log -1 recuperat/software-copia-30-10` (fecha de la foto: 30/10; todas son 'Recuperacio 30-10'). Fecha real de la tarea: 30/10 07:24 (rama agente/qwen/integracion-av19-28-10 + sin commitear).
- Pista TABLERO/sesiones/out: commit raiz 22c6a7d6 sin padres.
- Layout de la foto: raiz `MotorGraphico-main/MotorGraphico/`; 4552 ficheros en el alcance.
- Nuevos: 16. De ellos 13 son los 'Assets comunes' (editor_*.png, ojos_grande, *_sin_piernas, props 2.json, Untitled-1.psd).
  - Distintos de los comunes:
    - `assets/levels/`: afueras_ocaso.json
    - `assets/maps/`: afueras_ocaso.tmx
    - `tools/`: av19_ocaso.py
- Borrados a proposito en caa (NET-E3+E4, no restaurar): 84.
- Modificados: 35 'version antigua, superada'; 275 'revisar' (de los cuales 17 propios).
  - De los 275 'revisar': 36 tienen todas sus lineas ya en caa (caa es superconjunto -> superado), 58 binarios (PNG), resto con lineas que caa no tiene.
  - Por carpeta: assets/textures 72, assets/levels 61, tools 34, src/Game 13, include/Game 12, tests/Game 10, assets/maps 9, src/HUD 9
  - Propios (trabajo especifico de esta foto), `[lineas-solo-foto, lineas-solo-caa]`:
    - `assets/levels/interior_taberna.json` [603, 640]
    - `assets/levels/ciudad_centro.json` [367, 118]
    - `assets/levels/interior_castillo_p2.json` [325, 325]
    - `assets/levels/interior_herreria.json` [305, 290]
    - `assets/objects/boundington_av19_objetos.json` [40, 528]
    - `assets/levels/ciudad_surysal.json` [30, 36]
    - `src/Engine/Application.cpp` [12, 43]
    - `tools/test_validar_sprites.py` [5, 75]
    - `assets/levels/tienda_abandonada.json` [4, 29]
    - `src/Game/AuraMoral.cpp` [4, 20]
    - `tools/validar_puertas.py` [3, 99]
    - `src/Render/AuraJugador.cpp` [2, 10]
    - `include/Game/AuraMoral.h` [1, 17]
    - `src/Core/Resources/ShaderManager.cpp` [1, 1]
    - `include/Render/AuraJugador.h` [0, 1]
    - `tests/HUD/PergaminoTests.cpp` [0, 28]
    - `tools/test_validar_puertas.py` [0, 20]
- Nota: Ver seccion propia.
- **Veredicto: rescatar ficheros nuevos + revisar a mano**


## Seccion propia: `recuperat/software-copia-30-10` (22c6a7d6) = checkout de Oriol del 30/10

**Procedencia medida, no supuesta:** el arbol de MotorGraphico de esta foto es el de `qwen-AV19-09` (20/10) mas 12 ficheros solo suyos y 24 modificados. Es decir, la rama `agente/qwen/integracion-av19-28-10` partia de esa base y lo posterior (28-30/10, incluido lo sin commitear) es SOLO ese delta. Comparado con caa: 4.552 ficheros en el alcance (caa tiene 16 src + 15 include + 22 tests + 90 tools + 30 niveles que la foto no tiene: la foto es MAS VIEJA y mas pequeña, no mas rica).

### Delta de Oriol sobre qwen-AV19-09 (36 ficheros) y su estado en caa
| fichero | estado vs caa |
|---|---|
| assets/audio/LICENCIAS.md, licencias/erang_dungeon_synth.md, textures/alas_moral.{json,png}, tools/{explorar_aventura,gen_alas_moral,inventari_arbre,validar_audio}.py y sus test_*.py, examples/demo_brillo_equipo.cpp, levels/mazmorra_64x64.json, mundi_landmass_{2,4,7,8}.json | **identicos a caa** (ya estan) |
| src/Engine/ApplicationCierre.cpp | version antigua presente en el historial de caa |
| src/Engine/Application.cpp (+12/-43), include/Game/AuraMoral.h (+1/-17), src/Game/AuraMoral.cpp (+4/-20), src/Render/AuraJugador.cpp (+2/-10), include/Render/AuraJugador.h (0/-1), src/Core/Resources/ShaderManager.cpp (+1/-1), tests/HUD/PergaminoTests.cpp (0/-28), tools/validar_puertas.py (+3/-99), tools/test_validar_puertas.py (0/-20), tools/test_validar_sprites.py (+5/-75) | **caa es superconjunto o casi**: alasEnSprite, ratonEnPapiro, mueveCursor, kPechoX existen ya en caa. Las pocas lineas solo-foto son formas antiguas (p.ej. `throw ResourceLoadException(path)` en ShaderManager; la version de `alasEnSprite` sin el 5.o parametro). Superado. |
| **assets/textures/monumentos/Untitled-1.psd** (3,0 MB) | **NO esta en caa** (fuente de arte) |
| assets/levels/ciudad_centro.json (+367/-118), interior_taberna.json (+603/-640), interior_castillo_p2.json (+325/-325), interior_herreria.json (+305/-290), ciudad_surysal.json (+30/-36), tienda_abandonada.json (+4/-29), assets/objects/boundington_av19_objetos.json (+40/-528) | **REVISAR A MANO** (ver abajo) |

### Niveles: que tiene la foto que caa no tiene (comparado por objetos, no por lineas)
- `ciudad_centro.json`: la foto tiene 281 objetos, caa 248. Solo en foto: ~60 `cartel_*` de tienda (79 `"cartel_` en los niveles de la foto vs 17 en los de caa), `estrado_sermon` (ESCENAS-04), `puerta_coliseo`, `puerta_plaza_de_las_razas`, `botin_ciudad_saco`, `botin_ciudad_saco_4`, 22 puertas con otra posicion. Solo en caa: los actores de la historia (verina_estrado, captor_estrado x5, edul_plaza, sandes_plaza, aigren_carro...) y arboles/pinos. => Son DOS evoluciones paralelas del mismo mapa; no es un 'superconjunto'. Fable debe decidir carteles y puertas a mano.
- Cofres `botin_*` (reparto de botin AV18-05b): la foto tiene 58 en los niveles comunes, caa 46. Solo en foto: botin_ciudad_escondite (surysal), botin_ciudad_barril (oeste), botin_interiores_escondite_20 + armero_vacio (herreria), botin_interiores_{armario,armario_12,cofre,cofre_20,escondite} (armeria, biblioteca_p1, casa_te, castillo, castillo_p1, coliseo), botin_cueva_* y cofre_tesoro (cueva_gorran), botin_afueras_{escondite,cofre} (afueras, montana_gorran). Puede ser reparto antiguo que `repartir_botin.py` regenera: **comprobar regenerando antes de copiar**.
- `interior_taberna.json`: foto 84 objetos / caa 88 (caa añade vecina_ventana, xila_noche, velas nocturnas...). `interior_castillo_p2.json`: mismos objetos, solo cambian posiciones/formato (0 objetos distintos). `boundington_av19_objetos.json`: caa tiene 133 entradas, la foto 70 (caa mucho mas completo).
- Niveles solo en la foto: **`afueras_ocaso.json` + `maps/afueras_ocaso.tmx`** (ver siguiente seccion).

### Assets (especial cuidado)
- **PixelLab**: la foto tiene 1.568 ficheros bajo `assets/textures/pixellab/`, caa 1.567. Unica diferencia de contenido: `props 2.json` (duplicado Finder) y `bloques.json` (1 fichero modificado). **No se pierde ningun sprite PixelLab**: caa los tiene todos y es superconjunto.
- **Sprites de PNJ/personajes**: ninguno nuevo respecto a caa salvo `ojos_grande.png`, `torso_medio_sin_piernas.png`, `capa_medio_sin_piernas.png` (ver 'Assets comunes'). Los PNG modificados (pnj_boundington*.png, razas_andando.png, pnj_mundo_*.png, personajes/*.png: 58 binarios) son versiones **anteriores** (mas pequeñas) que las de caa; no restaurar.
- **Niveles de Qwen**: unico nivel que no existe en caa = `afueras_ocaso`. Los demas (ciudad_centro_ocaso, huida_boundington, base_militar, mansion_*) existen en caa con contenido distinto (caa mas nuevo; base_militar.json foto +26/-2 lineas).
- **Atlas del editor**: los 8 `editor_*.png` (ver 'Assets comunes') son lo mas valioso de la foto que caa no tiene.
- Fuera de alcance pero visible: la foto contiene una copia anidada `MotorGraphico-main/.ui-puertas-01/` (otro arbol completo de MotorGraphico+Onegai). No se ha comparado.

## Afueras al ocaso (qwen AV19-09/10 vs caa)
- Foto (qwen-AV19-09 y copia de Oriol): `assets/levels/afueras_ocaso.json` (102 objetos, 13.392 B) + `assets/maps/afueras_ocaso.tmx`, generados por `tools/av19_ocaso.py` (pipeline Excel de Qwen; en qwen-AV19-09 tambien `tools/av19_decorado.py`). Sesion: `sesiones/2026-10-20-qwen-av19-10-afueras-ocaso.md` ('+32 casillas nuevas', marcadas +AV19-10).
- caa: `afueras_boundington_ocaso.json` (87 objetos, 11.343 B), generado por `tools/gen_ocaso_huida.py` (`afueras_ocaso()`), con los actores de la historia (edul_afueras, verina_abrazo, guardia_venides, gorran_afueras, ...).
- Comparacion por (objectId, posicion): 34 objetos solo en la foto, 19 solo en caa. Solo en la foto: la decoracion de Qwen (molino, abrevadero, runas/columna_rota, carros, fuegos, sacos, juncos, establo, cofre (16,2), perdido_devoto/fanatico/saqueador). Solo en caa: los personajes/botin de la historia.
- `caa` solo referencia el nombre viejo `afueras_ocaso` en el propio `gen_ocaso_huida.py` (comentario). En la foto lo referencian `catalogs/lugares.json`, `proyectos/boundington.json`, `tools/validar_mundo_unido.py`, `huida_boundington.json`, `ciudad_centro_ocaso.json`.
- Veredicto: **revisar a mano**: o bien aplicar la decoracion de Qwen (34 objetos) sobre `afueras_boundington_ocaso.json`, o descartarla. No restaurar el nivel con el nombre viejo (duplicaria el nivel).

## Base comun 14-22/10: codigo con lineas que caa no tiene (muestra)
Todas las fotos intermedias comparten la misma base de ~250-340 ficheros 'revisar'. Es la version de `main` del 14-16/10: **superada**. Para que nadie lo vuelva a mirar, los 12 mayores (lineas solo-foto / solo-caa): `examples/demo_boundington.cpp` 634/363, `tests/Game/G13CombatCommandsTests.cpp` 503/173, `examples/demo_campana_boundington.cpp` 359/205, `demo_boundington_alternativas.cpp` 335/974, `tests/Game/BattleStateTests.cpp` 315/3, `src/Game/BattleState.cpp` 207/56, `tests/Game/QuestLogTests.cpp` 206/52 (solo en las 3 del 14/10: combate viejo y QuestLog), `tools/repartir_botin.py` 188/268 (AV16-09b) y 143/401 (AV19-05), `src/Game/AuraMoral.cpp` 79/246, `src/Game/Personajes/MontajePersonajes.cpp` 74/206, `tools/gen_ciudad.py` 88/297. Ninguno aparece como 'propio' de una tarea con integracion pendiente.

## Lista priorizada: que deberia restaurar Fable primero
1. **8 atlas `assets/textures/editor_*.png`** (~15 MB) desde `recuperat/software-copia-30-10`. El editor de niveles de caa los carga (`editor/src/level_editor.cpp:465`) y no existen en ningun commit del linaje. Certeza alta.
2. **`assets/textures/pixellab/lote2/con.png`** desde `recuperat/arbre/codex-AV13-08` (referencia colgante en `pixellab/iconos.json:33` de caa). Certeza alta, trivial.
3. **PIL-E1: `tests/Game/ClasesSpTests.cpp`** (+8 lineas, `recuperat/arbre/PILOTO-29-10-E1`) y **PIL-E2: dos lineas `LABELS ...;lenta` + `TIMEOUT 600` en `cmake/AparienciaTests.cmake`** (`recuperat/arbre/PILOTO-29-10-E2`, tomar SOLO esas 2 lineas: el resto del fichero es mas viejo que caa). Son los unicos entregables del piloto 29/10 que no estan en caa (informe: PIL-E1 validada 1beb819, PIL-E2 e64da06). Verificar con `ctest -R ^test_clases_sp$`.
4. **`ojos_grande.png`** (5 KB): primero regenerar con `tools/capa_ojos_iris.py`; si no cuadra, tomar la de cualquier foto 14-30/10. Luego decidir si hacen falta `torso_medio_sin_piernas.png` y `capa_medio_sin_piernas.png` (prioridad baja; pueden ser del esquema pre-.filas).
5. **Afueras al ocaso de Qwen**: decidir sobre los 34 objetos decorativos de `afueras_ocaso.json` (seccion arriba). Medio valor, decision de Oriol/Fable.
6. **`ciudad_centro.json`**: carteles de tienda (`cartel_*` ~60), `estrado_sermon`, `puerta_coliseo`, `puerta_plaza_de_las_razas` de la foto de Oriol (y ESCENAS-04/07) frente a los actores de caa. Mezcla manual; comprobar primero si `CartelesPuertas.cpp` ya los genera en runtime.
7. **Cofres `botin_*` de la foto de Oriol** que caa no tiene (lista en su seccion): regenerar con `tools/repartir_botin.py` antes de copiar a mano.
8. `assets/textures/monumentos/Untitled-1.psd` (3 MB): conservar fuera del repo si Oriol lo quiere (fuente de arte).
9. **NO restaurar**: las 84 borradas por NET-E3+E4 (race_npc_sprites, monumentos, race_npc_idle, gen_race_npc_sprites.py), BattleMenu*/TargetSelection*/QuestLogState*/demo_battle, `pnj_pesca_caza.*` (absorbido en pnj_boundington), `props 2.json`, ni ningun 'modificado' de las fotos 14-22/10 (versiones antiguas superadas por caa), ni las fotos PLAU01-R1, E-00-SIMULACRO y REPLICA-R1-U3-* (identicas o previas a los arreglos de revision).

## Metodo y limites
- Comparacion por hash de blob en `git ls-tree -r` (sin checkout). 'Version antigua' = blob igual a cualquier version previa del mismo path en los 25 ancestros de caa o en a523d6aa/95ef783a. Las bases viejas son ancestros 'finos' (25 commits), asi que una version antigua propia de las fotos 14-22/10 que no pasara por esos commits aparece como 'revisar'; por eso se añadio la medida de lineas (solo-foto / solo-caa por multiconjunto de lineas): `solo-foto = 0` equivale a 'caa es superconjunto'.
- 4 fotos (codex-AV13-08, AV15-02a/b, zcode-AV17-01.viejo) tienen otro layout (raiz = `MotorGraphico/`); se normalizo la ruta.
- No se comparo nada fuera de `MotorGraphico-main/MotorGraphico/` (ni `Onegai 2`, ni `out/`, ni docs). Scripts de apoyo en el scratchpad de la sesion (`cmp.py`, `stats.py`, `own.py`, `delta.py`).
