# Mapa del codigo <-> paquetes del Gantt

Generado por `tools/marcar_paquetes.py --mapa` desde `planificacion/pla/dades.py`. **No se edita a mano: se regenera.**

Para que sirve: que cualquier agente que abra un fichero sepa a que paquete del corte B pertenece, y que cualquiera que coja un paquete sepa que ficheros va a tocar — y con quien se va a cruzar.

## 1. Los ficheros de cruce

Ordenados por numero de paquetes que los tocan. **Son los sitios donde dos agentes se pisan.** Antes de una refactorizacion grande en uno de estos, mira `git status` por si hay trabajo sin commitear.

| Fichero | Paquetes | Cuales |
| --- | ---: | --- |
| `src/Engine/Application.cpp` | 14 | C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14 |
| `editor/src/level_editor.cpp` | 10 | C-14, C-15, D-05, E-07, E-10, F-01, F-02, G-01, G-02, G-05 |
| `include/RPG/Catalogs/RpgCatalogs.h` | 8 | D-01, D-02, D-03, D-04, D-05, D-06, D-07, E-16 |
| `src/Game/GameSession.cpp` | 6 | C-04, C-17, D-08, E-04, G-12, G-14 |
| `src/Project/ProjectSerializer.cpp` | 6 | C-11, C-13, E-01, E-11, E-13, E-16 |
| `src/Editor/EditorState.cpp` | 6 | C-14, C-15, E-12, G-01, G-02, G-03 |
| `src/Game/SaveGame.cpp` | 5 | C-03, C-04, C-08, E-02, G-11 |
| `src/Game/BattleState.cpp` | 4 | C-01, D-09, D-11, G-13 |
| `src/Project/ProjectIndex.cpp` | 3 | C-11, C-15, E-11 |
| `FORMATO_PROYECTOS.md` | 3 | C-11, E-16, G-07 |
| `include/Map/MapDocument.h` | 3 | C-12, E-01, F-03 |
| `include/Editor/EditorState.h` | 3 | C-14, G-02, G-03 |
| `editor/src/event_editor.cpp` | 3 | E-05, E-06, E-08 |
| `include/Game/GameSession.h` | 2 | C-03, E-04 |
| `tests/Game/SaveGameTests.cpp` | 2 | C-04, C-08 |
| `include/Engine/Application.h` | 2 | C-05, C-07 |
| `tests/Render/BitmapFontTests.cpp` | 2 | C-06, C-24 |
| `src/RPG/NarrativeEngine.cpp` | 2 | C-08, E-02 |
| `include/Runtime/RuntimeTileMap.h` | 2 | C-09, C-22 |
| `src/Map/MapDocument.cpp` | 2 | C-12, C-13 |
| `tests/Map/MapDocumentTests.cpp` | 2 | C-12, C-13 |
| `src/World/World.cpp` | 2 | C-17, C-18 |
| `include/Render/IsometricRenderer.h` | 2 | C-19, C-23 |
| `src/Render/IsometricRenderer.cpp` | 2 | C-19, C-23 |
| `include/RPG/CharacterSheet.h` | 2 | C-20, D-06 |
| `include/RPG/EnhanceHooks.h` | 2 | C-22, D-09 |
| `tests/Game/GameSessionTests.cpp` | 2 | C-24, D-08 |
| `tests/RPG/CatalogContentTests.cpp` | 2 | D-02, D-10 |
| `src/RPG/InventoryEngine.cpp` | 2 | D-05, G-12 |
| `src/RPG/SkillExecutor.cpp` | 2 | D-09, D-11 |
| `../.github/workflows/ci.yml` | 2 | D-10, G-16 |
| `src/RPG/EventInterpreter.cpp` | 2 | E-03, E-08 |
| `include/Editor/CommandListState.h` | 2 | E-06, E-14 |
| `assets/levels/` | 2 | E-09, G-04 |
| `editor/src/EditorApplication.cpp` | 2 | E-10, E-12 |
| `CMakeLists.txt` | 2 | E-10, G-06 |
| `include/Editor/EditorDocumentSession.h` | 2 | E-10, E-12 |
| `src/Editor/EditorDocumentSession.cpp` | 2 | E-10, E-12 |
| `tests/Editor/test_motor_editor_smoke.py` | 2 | E-10, E-12 |
| `cmake/MotorEditor.cmake` | 2 | E-10, E-12 |
| `include/Editor/ObjectInspector.h` | 2 | E-12, E-14 |
| `include/Editor/ObjectInspectorData.h` | 2 | E-12, E-14 |
| `include/Editor/ObjectInspectorInput.h` | 2 | E-12, E-14 |
| `tools/exportar.py` | 2 | G-06, G-15 |

## 2. Que ficheros toca cada paquete

### Fase C — Consolidació

**C-01 — La curació cura qui toca** (2 j)

- `src/Game/BattleState.cpp`  — compartido con D-09, D-11, G-13
- `include/Game/Skill.h`
- `tests/Game/BattleStateTests.cpp`

*Cerrado cuando:* Una habilitat amb `target = Ally` aplica el seu efecte sobre el jugador; una amb `target = Enemy`, sobre l'enemic seleccionat. El PM es gasta una sola vegada i només si l'efecte s'ha pogut aplicar.

**C-02 — El minimapa deixa de llegir memòria alliberada** (1 j)

- `include/Render/HudMinimap.h`
- `src/Render/HudMinimap.cpp`
- `src/Engine/Application.cpp`  — compartido con C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14

*Cerrado cuando:* El minimapa rep la sessió per `setSession()` i `loadLevel` l'hi torna a passar. Amb sessió nul·la es dibuixa buit en comptes de llegir.

**C-03 — L'estat del jugador sobreviu el canvi de nivell** (2 j)

- `src/Engine/Application.cpp`  — compartido con C-02, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14
- `include/Game/GameSession.h`  — compartido con E-04
- `src/Game/SaveGame.cpp`  — compartido con C-04, C-08, E-02, G-11

*Cerrado cuando:* Moral, negocis, passos des de l'últim cobrament i or es transfereixen sencers entre sessions. El traspàs és una sola funció, no una llista de camps escampada per `loadLevel`.

**C-04 — Un enemic invocat per narrativa no trenca el guardat** (2 j)  ·  depende de: C-03

- `src/Game/GameSession.cpp`  — compartido con C-17, D-08, E-04, G-12, G-14
- `src/Game/SaveGame.cpp`  — compartido con C-03, C-08, E-02, G-11
- `tests/Game/SaveGameTests.cpp`  — compartido con C-08

*Cerrado cuando:* Els enemics que no venen del nivell reben identitat pròpia i el guardat els sap distingir dels del mapa. Restaurar una partida amb un enemic invocat viu el torna a col·locar amb la seva vida.

**C-05 — Tancament ordenat dels recursos GL** (1 j)

- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14
- `include/Engine/Application.h`  — compartido con C-07

*Cerrado cuando:* Textures i shaders s'alliberen mentre el context és viu. Sortir del joc no deixa cap crida GL després de `glfwTerminate()`.

**C-06 — L'ajust de text no es penja** (1 j)

- `src/Render/BitmapFont.cpp`
- `tests/Render/BitmapFontTests.cpp`  — compartido con C-24

*Cerrado cuando:* Amb una amplada menor que un glifu, `wrapText` retorna el text tallat sense inventar guions i sense bucle.

**C-07 — Les aventures autònomes disparen els seus beats** (2 j)

- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14
- `include/Engine/Application.h`  — compartido con C-05

*Cerrado cuando:* Qualsevol aventura carregada s'enganxa a la sessió. La bandera de campanya només decideix si es fan les transicions de la cronologia, no si la narrativa existeix.

**C-08 — Un guardat antic no escriu fora de rang** (1 j)

- `src/RPG/NarrativeEngine.cpp`  — compartido con E-02
- `src/Game/SaveGame.cpp`  — compartido con C-03, C-04, E-02, G-11
- `tests/Game/SaveGameTests.cpp`  — compartido con C-04

*Cerrado cuando:* En restaurar, el vector d'avisos es redimensiona a la mida de la definició actual, conservant els que ja s'havien donat.

**C-09 — Propietat de recursos i destrucció polimòrfica** (1 j)

- `include/Core/Resources/Texture.h`
- `include/Runtime/RuntimeTileMap.h`  — compartido con C-22

*Cerrado cuando:* L'assignació per moviment allibera el recurs de destí abans de robar el d'origen. `RuntimeTileMap` té destructor virtual.

**C-10 — El JSON que escrivim el podem tornar a llegir** (1 j)

- `include/Core/Json/JsonWriter.h`
- `src/Core/Json/JsonValue.cpp`
- `tests/Core/JsonTests.cpp`

*Cerrado cuando:* El parser entén `\uXXXX` i el codifica a UTF-8. Tot el que surt de `JsonWriter` torna a entrar per `JsonValue::parse` sense pèrdua.

**C-11 — Un sol format de projecte** (4 j)

- `src/Project/ProjectSerializer.cpp`  — compartido con C-13, E-01, E-11, E-13, E-16
- `src/Project/ProjectIndex.cpp`  — compartido con C-15, E-11
- `FORMATO_PROYECTOS.md`  — compartido con E-16, G-07
- `tools/migrar_proyectos.py`

*Cerrado cuando:* Un únic format v1, amb `schemaVersion`, que `ProjectSerializer` llegeix i escriu i que `ProjectIndex` consumeix. Els cinc manifests d'`assets/proyectos/` queden migrats i el format vell es rebutja amb un missatge que diu com migrar.

**C-12 — MapDocument es pot editar** (5 j)

- `include/Map/MapDocument.h`  — compartido con E-01, F-03
- `src/Map/MapDocument.cpp`  — compartido con C-13
- `include/Map/MapLayer.h`
- `tests/Map/MapDocumentTests.cpp`  — compartido con C-13

*Cerrado cuando:* API de mutació completa: pintar cel·les, afegir i treure entitats i regions, esborrar capes, reordenar-les i canviar-ne els metadades. Cada mutació manté les invariants que avui només comprova `validate()`.

**C-13 — Les entitats del document no perden res** (3 j)  ·  depende de: C-12

- `include/Map/EntityLayerDocument.h`
- `src/Map/MapDocument.cpp`  — compartido con C-12
- `src/Project/ProjectSerializer.cpp`  — compartido con C-11, E-01, E-11, E-13, E-16
- `tests/Map/MapDocumentTests.cpp`  — compartido con C-12

*Cerrado cuando:* `MapEntityDocument` arriba a la paritat completa amb `ObjectSpawn`, i la serialització hi va i en torna sense pèrdua.

**C-14 — L'editor treballa sobre MapDocument** (6 j)  ·  depende de: C-12, C-13

- `include/Editor/EditorState.h`  — compartido con G-02, G-03
- `src/Editor/EditorState.cpp`  — compartido con C-15, E-12, G-01, G-02, G-03
- `editor/src/level_editor.cpp`  — compartido con C-15, D-05, E-07, E-10, F-01, F-02, G-01, G-02, G-05
- `tests/Editor/EditorStateTests.cpp`

*Cerrado cuando:* `EditorState` és una vista d'edició sobre un `MapDocument`. L'exportació TMX passa a ser un exportador d'interoperabilitat, no el format de treball. El pont per cadena de text desapareix.

**C-15 — Desar deixa el projecte coherent** (2 j)  ·  depende de: C-11, C-14

- `editor/src/level_editor.cpp`  — compartido con C-14, D-05, E-07, E-10, F-01, F-02, G-01, G-02, G-05
- `src/Project/ProjectIndex.cpp`  — compartido con C-11, E-11
- `src/Editor/EditorState.cpp`  — compartido con C-14, E-12, G-01, G-02, G-03

*Cerrado cuando:* Desar un nivell l'inscriu al manifest del projecte. L'editor porta indicador de document brut i demana confirmació abans de sortir amb canvis pendents.

**C-16 — L'Application delega en SceneManager** (4 j)

- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14
- `include/Engine/Scene/SceneManager.h`
- `tests/Engine/SceneManagerTests.cpp`

*Cerrado cuando:* El bucle principal empeny i treu escenes en comptes de mirar banderes. El canvi de nivell i el pas menú↔joc són transicions d'escena.

**C-17 — World és el propietari de les entitats** (6 j)  ·  depende de: C-16

- `include/World/World.h`
- `src/World/World.cpp`  — compartido con C-18
- `src/Game/GameSession.cpp`  — compartido con C-04, D-08, E-04, G-12, G-14
- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-23, F-01, F-03, G-08, G-09, G-10, G-13, G-14

*Cerrado cuando:* Enemics, objectes del món i els seus marcadors de render viuen a `World` amb handles estables. `syncWorldMarkers` desapareix: no hi ha res a reconciliar.

**C-18 — HandlePool sosté World** (2 j)  ·  depende de: C-17

- `include/Core/Memory/HandlePool.h`
- `src/World/World.cpp`  — compartido con C-17
- `tests/Core/HandlePoolTests.cpp`

*Cerrado cuando:* `World` guarda les seves entitats en un `HandlePool`. El bug de moviment queda arreglat i cobert.

**C-19 — Un sol culling i un sol contracte d'actualització** (2 j)

- `include/Render/IsometricRenderer.h`  — compartido con C-23
- `include/Render/IsometricSceneData.h`
- `src/Render/IsometricRenderer.cpp`  — compartido con C-23
- `include/Render/TileMap.h`
- `include/Render/IUpdatable.h`
- `include/Render/HudElement.h`

*Cerrado cuando:* Una sola funció de culling, amb un sol propietari. Un sol contracte d'actualització per frame, o dos noms diferents si de veritat són coses diferents.

**C-20 — Un sol escriptor de condicions** (2 j)

- `include/RPG/CharacterSheet.h`  — compartido con D-06
- `src/RPG/CharacterSheet.cpp`
- `src/RPG/ConditionEngine.cpp`
- `tests/RPG/ConditionEngineTests.cpp`

*Cerrado cuando:* `ConditionEngine` és l'únic camí per aplicar, apilar i retirar condicions. La fitxa n'és el magatzem. A més, `purge_expired` purga de veritat i `breaksOnDamage` es dispara pel dany rebut, no per l'infligit.

**C-21 — Un sol generador d'atzar** (1 j)

- `include/RPG/Rng.h`
- `include/RPG/RandomEngine.h`

*Cerrado cuando:* Una sola interfície d'atzar al motor. La que no es fa servir surt del repositori.

**C-22 — Retirada de l'API morta** (2 j)  ·  depende de: C-21

- `include/RPG/DicePoolEngine.h`
- `include/RPG/TierRules.h`
- `include/RPG/EnhanceHooks.h`  — compartido con D-09
- `include/Runtime/RuntimeTileMap.h`  — compartido con C-09

*Cerrado cuando:* El que no es fa servir surt. El que s'ha de fer servir es defineix. Cap declaració del motor es queda sense cos.

**C-23 — El post-processat arriba al joc** (4 j)  ·  depende de: C-16

- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, F-01, F-03, G-08, G-09, G-10, G-13, G-14
- `src/Render/FogOfWar.cpp`
- `src/Render/DynamicLightMap.cpp`
- `include/Render/IsometricRenderer.h`  — compartido con C-19
- `include/Render/IsometricPostProcessData.h`
- `src/Render/IsometricRenderer.cpp`  — compartido con C-19
- `assets/shaders/lightmap.vert`
- `assets/shaders/fog.vert`
- `assets/shaders/lut.vert`
- `tests/Render/PostProcessingTests.cpp`
- `cmake/PostProcessingTests.cmake`

*Cerrado cuando:* La boira de guerra i la llum dinàmica es poden activar per nivell des del document de mapa. El que no s'activi no costa cap crida GL.

**C-24 — Xarxa de seguretat al camí que es juga** (5 j)  ·  depende de: C-03, C-16, C-17

- `tests/Engine/ApplicationTests.cpp`
- `tests/Game/GameSessionTests.cpp`  — compartido con D-08
- `tests/Render/BitmapFontTests.cpp`  — compartido con C-06

*Cerrado cuando:* La lògica GL-free d'Application (`refreshHud`, `loadLevel`, el guardat, el director de campanya), la màquina d'estats de `GameMode`, l'economia de negocis i la moral tenen tests unitaris.

### Fase D — Dades reals

**D-01 — Un carregador que no troba res, falla** (3 j)

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-02, D-03, D-04, D-05, D-06, D-07, E-16
- `tests/RPG/CatalogContractTests.cpp`
- `tools/comprobar_contrato_cargadores.py`
- `tests/RPG/CatalogLoaderRegressionTests.cpp`
- `cmake/CatalogLoaderContractCheck.cmake`

*Cerrado cuando:* Cada `CatalogLoader` declara els camps que espera. Si una entrada no en porta cap, la càrrega és un error amb el nom del camp i el de l'entrada. El `catch(...)` mut desapareix. <strong>DEC-02:</strong> el nom canònic d'un camp és el de l'asset; quan no coincideix amb el carregador, s'adapta el C++. Els camps declarats sense presència es classifiquen com a opcionals o es retiren, no es demanen com a contingut.

**D-02 — Habilitats i encanteris** (4 j)  ·  depende de: D-01

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-03, D-04, D-05, D-06, D-07, E-16
- `assets/catalogs/skills.json`
- `tests/RPG/CatalogContentTests.cpp`  — compartido con D-10

*Cerrado cuando:* Magnitud per grau, `defenseStat`, cost, recuperació, abast i condició aplicada es llegeixen de l'entrada. Dues cartes diferents donen números diferents.

**D-03 — Monstres** (3 j)  ·  depende de: D-01

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-02, D-04, D-05, D-06, D-07, E-16
- `assets/catalogs/monsters.json`

*Cerrado cuando:* Vida, defenses, estadístiques, tier i habilitats es llegeixen d'on són. Cap monstre del catàleg es carrega amb vida zero.

**D-04 — Condicions** (2 j)  ·  depende de: D-01

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-02, D-03, D-05, D-06, D-07, E-16
- `assets/catalogs/conditions.json`

*Cerrado cuando:* Apilament, durada i efectes per ronda es llegeixen. Els modificadors de defensa que avui es parsegen i s'ignoren, o s'apliquen o s'esborren. <strong>DEC-02:</strong> els noms bons són els de l'asset (`stackable`/`duration`/`effects`); és el carregador qui es renombra.

**D-05 — Objectes, equipament i consumibles** (3 j)  ·  depende de: D-01

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-02, D-03, D-04, D-06, D-07, E-16
- `src/RPG/InventoryEngine.cpp`  — compartido con G-12
- `tests/RPG/InventoryEngineTests.cpp`
- `include/Project/ProjectCatalog.h`
- `src/Project/ProjectCatalog.cpp`
- `tests/Project/ProjectCatalogTests.cpp`
- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, E-07, E-10, F-01, F-02, G-01, G-02, G-05
- `assets/proyectos/boundington.json`
- `assets/proyectos/ejemplos.json`
- `assets/levels/ejemplos_objetos.json`
- `assets/objects/ejemplos_interacciones.json`

*Cerrado cuando:* Els tres catàlegs llegeixen els seus camps i `InventoryEngine` aplica cada bonificador una sola vegada, distingint escut de qualsevol altre objecte a la mà esquerra. <strong>DEC-05:</strong> `test_objects.json` i la masmorra de prova surten del projecte que es publica i van a un projecte d'exemples, amb un exemplar de cada tipus comú i sense col·lisions d'ID. Res no s'esborra: així deixa de trepitjar els preus de producció.

**D-06 — Classes, races i trasfons** (3 j)  ·  depende de: D-01

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-02, D-03, D-04, D-05, D-07, E-16
- `src/RPG/CharacterCreation.cpp`
- `include/RPG/CharacterSheet.h`  — compartido con C-20

*Cerrado cuando:* Bonificadors, passives i trets de classe, raça i trasfons arriben a la fitxa. Vincle, por, defecte, meta, ideal, personalitat i virtut es guarden.

**D-07 — Els catàlegs restants** (5 j)  ·  depende de: D-01

- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-02, D-03, D-04, D-05, D-06, E-16
- `assets/catalogs/`

*Cerrado cuando:* Els 21 catàlegs llegeixen tots els seus camps. Cap entrada del repositori es carrega amb tots els valors per defecte.

**D-08 — Els enemics del joc són els del catàleg** (3 j)  ·  depende de: D-03, C-17

- `src/Game/GameSession.cpp`  — compartido con C-04, C-17, E-04, G-12, G-14
- `include/Game/EnemyBrain.h`
- `tests/Game/GameSessionTests.cpp`  — compartido con C-24

*Cerrado cuando:* Un enemic del món es construeix des de la seva `MonsterDefinition`: estadístiques, defenses, tier i habilitats. El combat Nd6 té números diferents per a monstres diferents.

**D-09 — Les cartes apliquen les seves condicions** (2 j)  ·  depende de: D-04, C-20

- `src/RPG/SkillExecutor.cpp`  — compartido con D-11
- `include/RPG/EnhanceHooks.h`  — compartido con C-22
- `src/Game/BattleState.cpp`  — compartido con C-01, D-11, G-13

*Cerrado cuando:* Una carta que enverina, enverina. El resultat de l'execució diu quina condició cal aplicar i amb quina durada, i el combat l'aplica. <strong>DEC-28:</strong> quan el resultat del motor i el de les cartes Java divergeixin, es prepara un banc de casos amb resultat esperat i el valida Oriol; aquest banc es converteix després en prova automàtica. Les famílies d'efecte que van més enllà de les condicions són D-11.

**D-10 — CI valida els assets reals** (2 j)  ·  depende de: D-01, D-07

- `../.github/workflows/ci.yml`  — compartido con G-16
- `tools/validar_catalogos.py`
- `tools/linea_base_catalogos.json`
- `tools/validar_catalogos_ci.py`
- `tests/RPG/CatalogContentTests.cpp`  — compartido con D-02

*Cerrado cuando:* CI carrega els catàlegs reals d'`assets/` a cada commit i falla si una entrada perd camps mecànics respecte de la línia base registrada.

**D-11 — Vocabulari d'efectes d'habilitat** (12 j)  ·  depende de: D-02, D-04, D-09

- `include/RPG/SkillEffect.h`
- `src/RPG/SkillExecutor.cpp`  — compartido con D-09
- `src/Game/BattleState.cpp`  — compartido con C-01, D-09, G-13
- `tests/RPG/SkillEffectTests.cpp`

*Cerrado cuando:* Cada família del vocabulari té contracte tipat, executor i prova pròpia. L'estructura d'una habilitat és sempre la mateixa i el que canvia és l'efecte que dispara. Un efecte no suportat es rebutja amb diagnòstic, sense gastar cost ni torn, en comptes d'aplicar-se a mitges. Cap regla nova de Nd6: el reglament de 3a edició continua manant i DEC-28 diu qui arbitra.

### Fase E — Autoria visual i esdeveniments

**E-01 — L'esdeveniment és part del document de mapa** (3 j)  ·  depende de: C-13

- `include/Map/EventLayerDocument.h`
- `include/Map/MapDocument.h`  — compartido con C-12, F-03
- `src/Project/ProjectSerializer.cpp`  — compartido con C-11, C-13, E-11, E-13, E-16

*Cerrado cuando:* Un esdeveniment és una entitat del mapa amb pàgines, cada pàgina amb condicions, disparador i llista de comandes. Es desa amb el mapa i es valida amb ell.

**E-02 — Interruptors locals i variables** (3 j)  ·  depende de: E-01

- `include/RPG/NarrativeEngine.h`
- `src/RPG/NarrativeEngine.cpp`  — compartido con C-08
- `src/Game/SaveGame.cpp`  — compartido con C-03, C-04, C-08, G-11

*Cerrado cuando:* Cada esdeveniment té els seus interruptors locals A–D i el joc té variables numèriques amb nom. Tot va al guardat.

**E-03 — Planificador, esperes i processos paral·lels** (4 j)  ·  depende de: E-02

- `include/RPG/EventInterpreter.h`
- `src/RPG/EventInterpreter.cpp`  — compartido con E-08

*Cerrado cuando:* L'intèrpret executa comandes al llarg de diversos frames, amb esperes per temps i per condició, i admet processos paral·lels que no bloquegen el jugador. <strong>DEC-09:</strong> no es pot desar enmig d'una acció important; en sortir i tornar es reprèn des de l'últim punt segur, i cap recompensa s'entrega dues vegades. <strong>DEC-30:</strong> canviar de mapa acaba l'esdeveniment local; la continuació, si cal, s'expressa com a esdeveniment d'entrada del destí.

**E-04 — El joc de comandes** (7 j)  ·  depende de: E-03, D-09

- `src/RPG/EventCommands.cpp`
- `include/RPG/EventCommand.h`
- `src/Game/GameSession.cpp`  — compartido con C-04, C-17, D-08, G-12, G-14
- `include/Game/GameSession.h`  — compartido con C-03
- `src/RPG/SessionCommandHost.cpp`
- `tests/RPG/SessionCommandHostTests.cpp`

*Cerrado cuando:* Les comandes de les cinc famílies estan implementades i documentades a `FORMATO_AVENTURAS.md`, amb el seu efecte sobre la sessió. <strong>DEC-30:</strong> la comanda d'anar a un altre mapa és terminal per a l'esdeveniment local; el que hi hagi a sota rep diagnòstic d'inabastable.

**E-05 — Editor de pàgines d'esdeveniment** (8 j)  ·  depende de: E-01, C-14, E-12, E-14

- `editor/src/event_editor.cpp`  — compartido con E-06, E-08
- `include/Editor/EventEditorState.h`
- `tests/Editor/EventEditorTests.cpp`

*Cerrado cuando:* Des de l'editor es crea un esdeveniment sobre una casella, se li afegeixen pàgines, i cada pàgina té disparador, condicions i aparença. Tot GL-free sota `EventEditorState`. <strong>Pendent de DEC-10:</strong> quina pàgina guanya si se'n compleixen diverses, què passa si una condició canvia amb l'esdeveniment en marxa, i com conviuen amb els beats de les aventures antigues. Es pot implementar l'editor, però no fixar la política de selecció.

**E-06 — Editor vertical de comandes** (9 j)  ·  depende de: E-04, E-05

- `editor/src/event_editor.cpp`  — compartido con E-05, E-08
- `include/Editor/CommandListState.h`  — compartido con E-14

*Cerrado cuando:* La llista de comandes s'edita amb teclat: inserir, esborrar, moure, entrar en una branca. Les comandes amb paràmetres obren el seu formulari. <strong>DEC-12:</strong> aplicació instal·lable que funciona sense connexió; cap dependència de xarxa en temps d'execució, i no es converteix l'editor en una web. <strong>Pendent de DEC-07:</strong> si la primera entrega és la llista amb formularis o ja el gest d'encaixar peces. Les 9 jornades són anteriors al disseny de blocs i s'han de tornar a estimar quan es respongui.

**E-07 — Validació estàtica d'aventures** (4 j)  ·  depende de: E-04, E-16

- `include/RPG/AdventureValidation.h`
- `src/RPG/AdventureValidation.cpp`
- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-10, F-01, F-02, G-01, G-02, G-05

*Cerrado cuando:* Beats i esdeveniments es validen: flags que ningú encén, beats inabastables, ids duplicats, referències a NPC, enemics o skills inexistents, temporitzadors que no arrenca ningú. L'editor la crida abans de desar i abans del playtest.

**E-08 — Depuració pas a pas** (5 j)  ·  depende de: E-03, E-06

- `editor/src/event_editor.cpp`  — compartido con E-05, E-06
- `src/RPG/EventInterpreter.cpp`  — compartido con E-03

*Cerrado cuando:* Es pot executar un esdeveniment comanda a comanda, veure l'estat de flags i variables a cada pas i posar punts d'aturada.

**E-09 — La missió de la clau, la porta i el cofre** (3 j)  ·  depende de: E-07, E-08, E-15

- `assets/misiones/mision_llave.json`
- `examples/demo_mision_llave.cpp`
- `assets/levels/`  — compartido con G-04

*Cerrado cuando:* Des d'un projecte nou, la missió s'escriu sencera des de l'editor: cofre amb clau, porta tancada que la demana, NPC que en parla, i final. Es desa i reobre abans de provar-la; cap pas exigeix editar JSON o escriure un ID.

**E-10 — Espai de treball visual i controls coherents** (6 j)  ·  depende de: C-14, C-16

- `editor/src/EditorApplication.cpp`  — compartido con E-12
- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-07, F-01, F-02, G-01, G-02, G-05
- `third_party/imgui/`
- `CMakeLists.txt`  — compartido con G-06
- `tools/verificar_controles.py`
- `editor/include/EditorApplication.h`
- `editor/src/main.cpp`
- `editor/include/EditorGui.h`
- `editor/src/EditorGui.cpp`
- `editor/include/EditorVistaDeMapa.h`
- `editor/src/EditorVistaDeMapa.cpp`
- `include/Editor/MapFraming.h`
- `src/Editor/MapFraming.cpp`
- `tests/Editor/MapFramingTests.cpp`
- `cmake/EditorMapFraming.cmake`
- `include/Editor/EditorDocumentSession.h`  — compartido con E-12
- `src/Editor/EditorDocumentSession.cpp`  — compartido con E-12
- `tests/Editor/EditorDocumentSessionTests.cpp`
- `tests/Editor/test_motor_editor_smoke.py`  — compartido con E-12
- `cmake/MotorEditor.cmake`  — compartido con E-12
- `../.github/workflows/editor-shell.yml`
- `include/Editor/EditorActions.h`
- `src/Editor/EditorActions.cpp`
- `tests/Editor/EditorActionsTests.cpp`
- `tools/test_verificar_controles.py`

*Cerrado cuando:* Aplicar ADR-001: ImGui privat del motor_editor, un viewport amb docking, mapa central, menús i eines visibles. Ratolí i teclat invoquen les mateixes accions. Escriure en un camp no pinta el mapa ni dispara dreceres. Hi ha indicador de canvis i Desar/Descartar/Cancel·lar abans de tancar o canviar de document. El HUD del jugador conserva la seva tecnologia.

**E-11 — Projectes i arbre de mapes** (5 j)  ·  depende de: E-10, C-11

- `editor/src/ProjectPanel.cpp`
- `include/Editor/ProjectHub.h`
- `include/Project/ProjectIndex.h`
- `src/Project/ProjectIndex.cpp`  — compartido con C-11, C-15
- `include/Project/ProjectSettings.h`
- `src/Project/ProjectSerializer.cpp`  — compartido con C-11, C-13, E-01, E-13, E-16
- `tests/Project/ProjectNamingTests.cpp`

*Cerrado cuando:* Reutilitzar ProjectHub/ProjectIndex en un assistent de nom, carpeta i plantilla; IDs generats i noms visibles separats. Arbre amb crear, obrir, duplicar i canviar nom visible de mapa. Triar mapa/casella inicials i títol del joc. Duplicar remapeja IDs locals; no sobreescriu altres projectes. Cancel·lar o fallar una operació no deixa un projecte mig registrat.

**E-12 — Propietats de mapa, capes i instàncies** (4 j)  ·  depende de: E-11, C-13

- `editor/src/MapPropertiesPanel.cpp`
- `editor/src/LayerPanel.cpp`
- `include/Editor/ObjectInspector.h`  — compartido con E-14
- `include/Editor/ObjectInspectorData.h`  — compartido con E-14
- `include/Editor/ObjectInspectorInput.h`  — compartido con E-14
- `src/Editor/EditorState.cpp`  — compartido con C-14, C-15, G-01, G-02, G-03
- `editor/include/LayerPanel.h`
- `editor/src/EditorApplication.cpp`  — compartido con E-10
- `include/Editor/EditorDocumentSession.h`  — compartido con E-10
- `src/Editor/EditorDocumentSession.cpp`  — compartido con E-10
- `tests/Editor/EditorLayerTests.cpp`
- `tests/Editor/test_motor_editor_smoke.py`  — compartido con E-10
- `cmake/MotorEditor.cmake`  — compartido con E-10

*Cerrado cuando:* Editar nom, dimensions, tileset, entrada i opcions de llum/boira ja suportades. Capes: crear, renombrar, ordenar, mostrar/ocultar i bloquejar. Reduir dimensions avisa de les entitats afectades i permet cancel·lar. L'inspector manté nom, escala, variant, botí, patrulla i sortida; tot passa per MapDocument i l'historial. Opacitat arbitrària queda per a 1.1.

**E-13 — Navegador visual i importació de recursos** (4 j)  ·  depende de: E-11

- `editor/src/AssetBrowserPanel.cpp`
- `include/Editor/AssetBrowserState.h`
- `src/Editor/AssetBrowserState.cpp`
- `tests/Editor/AssetBrowserStateTests.cpp`
- `src/Project/ProjectSerializer.cpp`  — compartido con C-11, C-13, E-01, E-11, E-16

*Cerrado cuando:* Cerca per nom/tipus, miniatures de tiles i sprites i previsualització de l'element seleccionat. Importar PNG i spritesheets compatibles cap al projecte amb selector de fitxer, revisió de mides/frames i avís de col·lisió; reutilitzar o cancel·lar sense sobreescriure. Es desa una referència relativa estable. No inclou autotiles ni editor gràfic.

**E-14 — Selectors de dades i referències** (3 j)  ·  depende de: E-13, D-10

- `editor/src/CatalogPicker.cpp`
- `include/Editor/CatalogPickerState.h`
- `include/Editor/ObjectInspector.h`  — compartido con E-12
- `include/Editor/ObjectInspectorData.h`  — compartido con E-12
- `include/Editor/ObjectInspectorInput.h`  — compartido con E-12
- `include/Editor/CommandListState.h`  — compartido con E-06

*Cerrado cuando:* Selectors amb nom, tipus, cerca, descripció i ID intern per a objectes, habilitats, enemics, condicions, mapes i esdeveniments. Integrats als inspectors actuals; E-05/06 els incorporen als formularis d'esdeveniment. Amb Obrir definició i avisos de referències inexistents. Catàleg importat de Onegai en lectura; només referències del tipus correcte, sense haver d'escriure IDs per completar el tutorial.

**E-15 — Problemes i cerca d'esdeveniments** (3 j)  ·  depende de: E-07, E-06

- `editor/src/ProblemsPanel.cpp`
- `include/Editor/ProblemsState.h`
- `src/Editor/ProblemsState.cpp`
- `tests/Editor/ProblemsStateTests.cpp`

*Cerrado cuando:* Llista de problemes amb severitat, mapa, entitat/pàgina/comanda i acció Anar-hi. Cercar esdeveniments per nom i usos de variables/interruptors. Errors bloquegen playtest/export; es pot desar feina incompleta amb diagnòstics visibles. Actualitzar un diagnòstic conserva el focus quan existeix.

**E-16 — Edició RPG mínima del projecte** (8 j)  ·  depende de: E-14, D-02, D-03, D-05

- `editor/src/ProjectDatabasePanel.cpp`
- `include/Editor/ProjectDatabaseState.h`
- `src/Project/ProjectSerializer.cpp`  — compartido con C-11, C-13, E-01, E-11, E-13
- `include/RPG/Catalogs/RpgCatalogs.h`  — compartido con D-01, D-02, D-03, D-04, D-05, D-06, D-07
- `FORMATO_PROYECTOS.md`  — compartido con C-11, G-07

*Cerrado cuando:* Formularis mínims per a objectes/consumibles, equip, habilitats i enemics: crear, duplicar a projecte, editar, validar i desar els camps mecànics suportats. Assignar habilitats a la configuració del personatge/enemic. Imports canònics de lectura; les còpies locals tenen IDs de projecte, origen explícit i s'exporten amb ell. Cap escriptura a cartes/canon ni regles noves de Nd6. La resta de famílies es consulten amb E-14; editar les 21 famílies, lots i dissenyar fórmules es manté a 1.1.

### Fase F — Playtest des de l'editor

**F-01 — Playtest des de l'editor** (4 j)  ·  depende de: C-16, E-09

- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-07, E-10, F-02, G-01, G-02, G-05
- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-03, G-08, G-09, G-10, G-13, G-14

*Cerrado cuando:* <strong>BLOQUEJAT per DEC-06.</strong> Aquest criteri diu «mateix procés, document en memòria», i la bitàcora §8.4 en prescriu un de contradictori —procés separat amb desat previ—. Les dues formulacions són vigents i implementar-ne una abans de resoldre-ho arrisca fer la feina dues vegades. El requisit observable que no canvia: no haver de desar a mà, que el que es faci jugant no toqui el projecte, i que en sortir es torni exactament a l'estat d'edició. F-02, F-03 i F-04 hereten el bloqueig.

**F-02 — Provar des de la casella seleccionada** (2 j)  ·  depende de: F-01

- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-07, E-10, F-01, G-01, G-02, G-05

*Cerrado cuando:* El playtest arrenca amb el jugador a la casella marcada, amb l'estat de partida que l'autor esculli (flags i objectes preparats).

**F-03 — Recàrrega en calent** (4 j)  ·  depende de: F-01

- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, G-08, G-09, G-10, G-13, G-14
- `include/Map/MapDocument.h`  — compartido con C-12, E-01

*Cerrado cuando:* Modificar el mapa o un esdeveniment mentre el playtest corre l'actualitza sense reiniciar la partida. El que no es pot recarregar en calent ho diu clar en comptes de fallar a mitges. <strong>Pendent de DEC-06 i DEC-11:</strong> què ha de passar amb un esdeveniment que s'està executant quan es recarrega —acabar amb la versió activa i aplicar la nova a la següent activació, o reiniciar-lo—. Barrejar meitats de dues versions no és una opció.

**F-04 — Consola i inspector d'estat** (3 j)  ·  depende de: F-01

- `editor/src/console.cpp`
- `include/Editor/ConsoleState.h`

*Cerrado cuando:* Durant el playtest es poden veure i modificar flags, variables, or i inventari, i llegir el registre d'esdeveniments disparats.

### Fase G — Tancament del corte B

**G-01 — Autosave i recuperació** (2 j)  ·  depende de: C-15, E-10

- `src/Editor/EditorState.cpp`  — compartido con C-14, C-15, E-12, G-02, G-03
- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-07, E-10, F-01, F-02, G-02, G-05

*Cerrado cuando:* El document es desa sol cada N canvis en un fitxer de recuperació. En obrir, si n'hi ha un de més nou que el desat, l'editor ho ofereix. <strong>Pendent de DEC-13:</strong> quanta feina és acceptable perdre —fixa la N— i si s'avança una protecció mínima abans de les sessions llargues d'autoria. Mentre no es respongui, el paquet es queda al final de G, que és on està planificat avui.

**G-02 — Les eines d'edició que falten** (3 j)  ·  depende de: C-14, E-12

- `include/Editor/EditorState.h`  — compartido con C-14, G-03
- `src/Editor/EditorState.cpp`  — compartido con C-14, C-15, E-12, G-01, G-03
- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-07, E-10, F-01, F-02, G-01, G-05
- `tests/Editor/EditorToolsTests.cpp`
- `cmake/EditorToolsTests.cmake`

*Cerrado cuando:* Comptagotes, rectangle i línia, cadascuna com una sola operació d'historial.

**G-03 — Historial per comandes** (5 j)  ·  depende de: C-14, G-02

- `include/Editor/EditorState.h`  — compartido con C-14, G-02
- `src/Editor/EditorState.cpp`  — compartido con C-14, C-15, E-12, G-01, G-02

*Cerrado cuando:* L'historial guarda comandes reversibles, no fotografies. Un mapa de 200×200 amb quatre capes admet mil operacions sense creixement notable de memòria. <strong>Pendent de DEC-29:</strong> «creixement notable» no és mesurable. Falta la màquina de referència —quin ordinador i a quina resolució— per convertir-ho en un límit comprovable; «PC» no permet mesurar.

**G-04 — Joc d'exemple de trenta minuts** (5 j)  ·  depende de: E-09, F-03, G-01, G-03, G-11, G-12, G-13, G-14

- `assets/proyectos/ejemplo/`
- `assets/levels/`  — compartido con E-09
- `assets/adventures/`

*Cerrado cuando:* Un joc de trenta minuts amb tres mapes, sis esdeveniments, combat i final, creat des de les pantalles. Inclou nova partida, personatge, menú de pausa, inventari/equip, habilitat local, compra/venda, guardat i derrota.

**G-05 — Tutorial de l'editor** (4 j)  ·  depende de: G-04

- `docs/tutorial/`
- `editor/src/level_editor.cpp`  — compartido con C-14, C-15, D-05, E-07, E-10, F-01, F-02, G-01, G-02

*Cerrado cuando:* Un tutorial guiat dins de l'editor que porta de zero a un mapa jugable amb un esdeveniment.

**G-06 — Empaquetat i exportació** (5 j)  ·  depende de: G-04

- `tools/exportar.py`  — compartido con G-15
- `CMakeLists.txt`  — compartido con E-10

*Cerrado cuando:* Un projecte s'exporta a un executable autocontingut per a Linux i Windows, amb només els assets que fa servir.

**G-07 — Congelació dels esquemes** (2 j)  ·  depende de: G-04, G-06

- `FORMATO_PROYECTOS.md`  — compartido con C-11, E-16
- `FORMATO_NIVELES.md`
- `FORMATO_AVENTURAS.md`
- `tests/Integration/SchemaFixtureTests.cpp`
- `tests/fixtures/esquemas/`

*Cerrado cuando:* Els esquemes de projecte, mapa, aventura i guardat es congelen a la versió 1.0-B, inclosos configuració de títol, dades RPG locals i metadades de slots, documentats, amb ruta de migració per a cada canvi futur.

**G-08 — Títol i arrencada del projecte** (4 j)  ·  depende de: C-16, E-11

- `src/Engine/Scene/TitleScene.cpp`
- `examples/juego.cpp`
- `include/Game/TitleMenuState.h`
- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-09, G-10, G-13, G-14

*Cerrado cuando:* Arrencar des del manifest del projecte; mostrar nom/imatge disponibles i Nova partida, Continuar, Opcions i Sortir. Continuar desactivat amb motiu si no hi ha guardat vàlid; G-11 connecta el selector. Dades absents mostren un error comprensible. Playtest contextual continua saltant el títol.

**G-09 — Creació de personatge dins de Nova partida** (3 j)  ·  depende de: G-08, D-06, E-16

- `src/Engine/Scene/CharacterCreationScene.cpp`
- `include/RPG/CharacterCreationScreen.h`
- `examples/creacion_personaje.cpp`
- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-10, G-13, G-14

*Cerrado cuando:* Nova partida obre l'assistent existent, amb tornar/cancel·lar i resum. Confirmar crea la sessió real amb fitxa, trets, equip i habilitats del projecte; continuar una partida no el repeteix. Sense fitxers intermedis que el jugador hagi de copiar a mà.

**G-10 — Menú de partida, opcions i focus** (3 j)  ·  depende de: G-08

- `src/Engine/Scene/PauseScene.cpp`
- `include/Game/PauseMenuState.h`
- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-13, G-14

*Cerrado cuando:* Escape obre el menú amb Reprendre, Personatge, Inventari/Equip, Desar, Carregar, Opcions, Tornar al títol i Sortir, connectats en G-11/12. Confirmació per abandonar progrés. Opcions mínimes: finestra/pantalla completa, escala de text i ajuda de controls persistents per usuari. El món i temporitzadors no avancen pausats; el focus no filtra tecles. <strong>DEC-16:</strong> el comptador d'FPS queda ocult a l'experiència normal, activable en depuració i amb una opció a ajustos per mostrar-lo. <strong>DEC-14:</strong> Opcions porta el selector d'idioma que implementa G-17.

**G-11 — Selector de guardats i recuperació de derrota** (4 j)  ·  depende de: G-09, G-10, C-24

- `include/Game/SaveSlotState.h`
- `src/Game/SaveSlotState.cpp`
- `include/Game/SaveGameValidation.h`
- `src/Game/SaveGameFormat.cpp`
- `include/Game/SaveGame.h`
- `include/Project/SerializerHelpers.h`
- `src/Project/SerializerHelpers.cpp`
- `tests/Project/SerializationTests.cpp`
- `src/Game/SaveGame.cpp`  — compartido con C-03, C-04, C-08, E-02
- `tests/Game/SaveSlotsTests.cpp`
- `tests/Game/SaveGameTests.cpp`  — compartido con C-04, C-08
- `src/Engine/Scene/SaveLoadScene.cpp`
- `src/Engine/Scene/GameOverScene.cpp`

*Cerrado cuando:* Slots separats per projecte amb personatge, mapa i data; seleccionar, desar amb confirmació de sobreescriptura i carregar. Corruptes/incompatibles mostren motiu i no destrueixen la sessió. Derrota ofereix Carregar o Tornar al títol; Continuar usa el mateix selector. F5/F9 reutilitzen el servei. <strong>DEC-09:</strong> desar només en punts segurs; enmig d'un diàleg, una espera o un combat es bloqueja indicant el motiu, i en carregar es torna a l'últim punt segur sense repetir recompenses ja entregades.

**G-12 — Inventari, equip i fitxa utilitzables** (5 j)  ·  depende de: G-11, D-05

- `include/Game/InventoryMenuState.h`
- `src/Game/InventoryMenuState.cpp`
- `include/Game/InventoryScreen.h`
- `src/Game/InventoryScreen.cpp`
- `src/Engine/ApplicationPantallas.cpp`
- `src/Engine/ApplicationHud.cpp`
- `tests/Game/InventoryMenuTests.cpp`
- `src/Engine/Scene/InventoryScene.cpp`
- `src/RPG/InventoryEngine.cpp`  — compartido con D-05
- `src/Game/GameSession.cpp`  — compartido con C-04, C-17, D-08, E-04, G-14
- `include/Render/Player.h`
- `include/Render/PlayerData.h`
- `include/Game/PlayerControls.h`
- `include/Game/PlayerControlsData.h`
- `include/Render/PlayerMotionData.h`
- `include/Render/AnimatedEntity.h`
- `src/Render/AnimatedEntity.cpp`
- `src/Render/Player.cpp`
- `tests/Render/PlayerTests.cpp`

*Cerrado cuando:* Llistes per categories, quantitats, descripció i accions Usar/Equipar/Desequipar; triar destinatari quan pertoqui. Fitxa amb estadístiques, trets i condicions; previsualitzar canvis d'equip. Accions impossibles expliquen el motiu i no gasten res. Reutilitzar les regles existents; els objectes de missió no es poden perdre amb una acció de gestió.

**G-13 — Comandes de combat i objectius des de dades** (5 j)  ·  depende de: G-09, D-02, D-09, E-16

- `include/Game/BattleMenuState.h`
- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-14
- `src/Game/BattleState.cpp`  — compartido con C-01, D-09, D-11

*Cerrado cuando:* Construir accions des de fitxa i inventari; mostrar nom, cost, disponibilitat i motiu de bloqueig. Triar objectius vàlids segons abast i bàndol, amb cancel·lació anterior a gastar recursos. Mostrar resultat Nd6, consum i condicions coherents amb la sessió. Cap taula de noms d'habilitats al HUD.

**G-14 — HUD, diàlegs i comerç amb accions visibles** (3 j)  ·  depende de: G-10, E-04, D-05

- `src/Engine/Application.cpp`  — compartido con C-02, C-03, C-05, C-07, C-16, C-17, C-23, F-01, F-03, G-08, G-09, G-10, G-13
- `src/Game/GameSession.cpp`  — compartido con C-04, C-17, D-08, E-04, G-12
- `include/Render/HudWidgets.h`
- `include/Render/HudWidgetsData.h`
- `src/Render/HudWidgets.cpp`
- `include/Render/HudTextWidgets.h`
- `include/Render/HudTextWidgetsData.h`
- `src/Render/HudTextWidgets.cpp`
- `examples/demo_timers_partida.cpp`

*Cerrado cuando:* Mantenir PV/PM, minimapa i moral; temporitzadors visibles segons Contador/Barra/Secret i temps actiu. Diàlegs i eleccions navegables amb ratolí/teclat; botiga mostra preu, quantitat i resultat, negoci compra/renda i impacte moral. Errors expliquen manca d'or/requisits. Coordinar amb F1 del TABLERO i reutilitzar-ne el resultat, sense una segona implementació. <strong>REV-02:</strong> el panell de conversa s'amaga en sortir de la interacció i el text narratiu té prioritat sobre el log; l'exploració recupera el focus i no queda cap quadre buit. <strong>REV-03 i DEC-05:</strong> el botiguer mostra els articles previstos amb preus vàlids un cop aïllat el contingut de prova. Tots dos són arreglos ja autoritzats i no esperen cap decisió.

**G-15 — Assistent d'exportació des de l'editor** (3 j)  ·  depende de: G-06, E-11, E-15

- `editor/src/ExportPanel.cpp`
- `include/Editor/ExportState.h`
- `tools/exportar.py`  — compartido con G-06

*Cerrado cuando:* Triar projecte, plataforma Linux/Windows disponible i carpeta de sortida; validar, mostrar progrés/errors i obrir la carpeta del resultat. El motor d'exportació de G-06 s'invoca una sola vegada. Si l'host no té el runtime objectiu disponible, s'explica i no es presenta com a exportació correcta. No cal terminal ni tenir el repositori per jugar el resultat.

**G-16 — Acceptació de pantalles d'autor i jugador** (3 j)  ·  depende de: G-05, G-07, G-15, F-02, F-04

- `tests/UI/`
- `docs/acceptance/corte-b-pantalles.md`
- `../.github/workflows/ci.yml`  — compartido con D-10

*Cerrado cuando:* Prova amb autor nou: projecte, mapa, recurs, habilitat local, NPC/missió, diagnòstic, desar/reobrir, playtest i exportar, sense C++/JSON/terminal. Un altre jugador obre l'export: Nova partida, personatge, diàleg, equip, combat, botiga, desar, reiniciar, continuar i recuperar derrota. Cap control inert, text essencial tallat o error sense sortida. Registrar plataforma, passos, captures, incidències i resultat; tanca G.

**G-17 — Idioma seleccionable de la interfície** (7 j)  ·  depende de: G-10, G-14, E-10

- `include/Engine/Locale.h`
- `src/Engine/Locale.cpp`
- `assets/i18n/`
- `src/Engine/Scene/OptionsScene.cpp`
- `tests/Engine/LocaleTests.cpp`

*Cerrado cuando:* Els textos visibles surten d'una taula per idioma amb clau estable i reserva a l'idioma de referència. El selector viu a Opcions (G-10) i el canvi s'aplica sense reiniciar. <strong>DEC-15:</strong> el text visible admet accents i ñ amb cobertura de font comprovada; els identificadors interns no es tradueixen ni porten accents. <strong>Pendent de DEC-14:</strong> quin és l'idioma de referència i si els graus de combat conserven el nom propi ONEGAI en els tres idiomes o es tradueixen.

## 3. Ficheros sin paquete asignado

364 de 517 ficheros `.cpp`/`.h` del repositorio **no los cita ningun paquete del plan**. No significa que sobren: significa que nadie sabe, mirandolos, dentro de que trabajo caen. Un agente que los modifique no tiene criterio de cierre contra el que medirse.

| Carpeta | Ficheros sin paquete |
| --- | ---: |
| `examples/` | 54 |
| `tests/Game/` | 23 |
| `tests/RPG/` | 22 |
| `include/Render/` | 18 |
| `src/Game/` | 18 |
| `include/Game/` | 15 |
| `src/Render/` | 12 |
| `tests/Render/` | 12 |
| `include/RPG/` | 11 |
| `src/Editor/` | 10 |
| `tests/Editor/` | 10 |
| `src/RPG/` | 9 |
| `editor/src/` | 6 |
| `include/Core/Log/` | 8 |
| `include/RPG/Catalogs/` | 8 |
| `include/RPG/Definitions/` | 8 |
| `tests/Engine/` | 8 |
| `include/Core/Math/` | 6 |
| `include/Core/Resources/` | 6 |
| `include/Editor/` | 6 |
| `include/Level/` | 6 |
| `src/Project/` | 6 |
| `src/Core/Log/` | 5 |
| `src/Core/Resources/` | 5 |
| `src/Engine/` | 5 |
| `src/Map/` | 5 |
| `tests/Project/` | 5 |
| `include/Engine/` | 4 |
| `include/Project/` | 4 |
| `src/Level/` | 4 |
| `tests/Core/` | 4 |
| `include/Core/Errors/` | 3 |
| `include/RPG/Narrative/` | 3 |
| `src/RPG/Eventos/` | 3 |
| `tests/Level/` | 3 |
| `include/Core/Ids/` | 2 |
| `include/Engine/Input/` | 2 |
| `include/Map/` | 2 |
| `include/Runtime/` | 2 |
| `src/RPG/Narrative/` | 2 |
| `src/Runtime/` | 2 |
| `tests/` | 2 |
| `include/Core/` | 1 |
| `include/Core/Json/` | 1 |
| `include/Engine/Scene/` | 1 |
| `include/HUD/` | 1 |
| `include/Import/` | 1 |
| `include/World/` | 1 |
| `src/Engine/Input/` | 1 |
| `src/Engine/Scene/` | 1 |
| `src/HUD/` | 1 |
| `src/Import/` | 1 |
| `tests/Import/` | 1 |
| `tests/Integration/` | 1 |
| `tests/Map/` | 1 |
| `tests/Runtime/` | 1 |
| `tests/World/` | 1 |

<details><summary>La lista completa</summary>

- `editor/src/EditorOpciones.cpp`
- `editor/src/LevelEditorArranque.cpp`
- `editor/src/LevelEditorEditState.cpp`
- `editor/src/LevelEditorHub.cpp`
- `editor/src/LevelEditorOpciones.cpp`
- `editor/src/LevelEditorPaleta.cpp`
- `examples/NarrativeCheckSession.h`
- `examples/ScriptedRng.h`
- `examples/TextAsset.h`
- `examples/TextAssetManager.cpp`
- `examples/TextAssetManager.h`
- `examples/demo_animated_entity.cpp`
- `examples/demo_aventura.cpp`
- `examples/demo_battle.cpp`
- `examples/demo_battle_hud.cpp`
- `examples/demo_battle_nd6.cpp`
- `examples/demo_bitmap_font.cpp`
- `examples/demo_boundington.cpp`
- `examples/demo_camera.cpp`
- `examples/demo_campana_boundington.cpp`
- `examples/demo_catalogos_rpg.cpp`
- `examples/demo_ciudad.cpp`
- `examples/demo_congelacio.cpp`
- `examples/demo_creacion_personaje.cpp`
- `examples/demo_dice_engine.cpp`
- `examples/demo_dynamic_lights.cpp`
- `examples/demo_editor_state.cpp`
- `examples/demo_enemy_brain.cpp`
- `examples/demo_enemy_sprite.cpp`
- `examples/demo_este_norte.cpp`
- `examples/demo_este_sur.cpp`
- `examples/demo_fog.cpp`
- `examples/demo_game_session.cpp`
- `examples/demo_handle_pool.cpp`
- `examples/demo_hud.cpp`
- `examples/demo_hud_update.cpp`
- `examples/demo_isometric_renderer.cpp`
- `examples/demo_level_loader.cpp`
- `examples/demo_lighting.cpp`
- `examples/demo_lut.cpp`
- `examples/demo_mundo.cpp`
- `examples/demo_narrativa_combate.cpp`
- `examples/demo_narrativa_nd6.cpp`
- `examples/demo_nd6_distribution.cpp`
- `examples/demo_nova_partida.cpp`
- `examples/demo_object_catalog.cpp`
- `examples/demo_object_inspector.cpp`
- `examples/demo_occlusion_rules.cpp`
- `examples/demo_plaza_razas.cpp`
- `examples/demo_precuela_boundington.cpp`
- `examples/demo_prologo.cpp`
- `examples/demo_proyectos.cpp`
- `examples/demo_resource_manager.cpp`
- `examples/demo_senal_ceniza.cpp`
- `examples/demo_skills.cpp`
- `examples/demo_temporizadores.cpp`
- `examples/demo_textured_quad.cpp`
- `examples/demo_tilemap.cpp`
- `examples/demo_title_menu.cpp`
- `examples/sandbox_window.cpp`
- `include/Core/Check.h`
- `include/Core/Errors/EngineException.h`
- `include/Core/Errors/Result.h`
- `include/Core/Errors/ScopeGuard.h`
- `include/Core/Ids/ProjectIds.h`
- `include/Core/Ids/StrongId.h`
- `include/Core/Json/JsonValue.h`
- `include/Core/Log/ConsoleSink.h`
- `include/Core/Log/FileSink.h`
- `include/Core/Log/ILogSink.h`
- `include/Core/Log/LogConfig.h`
- `include/Core/Log/LogFormat.h`
- `include/Core/Log/LogRecord.h`
- `include/Core/Log/Logger.h`
- `include/Core/Log/MemorySink.h`
- `include/Core/Math/GridBounds.h`
- `include/Core/Math/GridCoord.h`
- `include/Core/Math/IsoMath.h`
- `include/Core/Math/UVRect.h`
- `include/Core/Math/Vector2.h`
- `include/Core/Math/Vector4.h`
- `include/Core/Resources/ICatalog.h`
- `include/Core/Resources/ResourceManager.h`
- `include/Core/Resources/Shader.h`
- `include/Core/Resources/ShaderManager.h`
- `include/Core/Resources/TextureAtlas.h`
- `include/Core/Resources/TextureManager.h`
- `include/Editor/AssetBrowserPanel.h`
- `include/Editor/EditorDocumentIO.h`
- `include/Editor/EditorStateHelpers.h`
- `include/Editor/EditorValidation.h`
- `include/Editor/MapPropertiesPanel.h`
- `include/Editor/ProblemsInternal.h`
- `include/Engine/ApplicationInternal.h`
- `include/Engine/ApplicationLoop.h`
- `include/Engine/Input/InputMap.h`
- `include/Engine/Input/InputSystem.h`
- `include/Engine/InputState.h`
- `include/Engine/Scene/Scene.h`
- `include/Engine/Window.h`
- `include/Game/BattleMenuActions.h`
- `include/Game/BattleState.h`
- `include/Game/EnemyBrainData.h`
- `include/Game/EnemyCatalogBridge.h`
- `include/Game/GameOverScreen.h`
- `include/Game/MenuLogic.h`
- `include/Game/Morality.h`
- `include/Game/NewGameFlow.h`
- `include/Game/OptionsLocaleBridge.h`
- `include/Game/OptionsState.h`
- `include/Game/PauseMenu.h`
- `include/Game/SaveGame.h`
- `include/Game/SessionTipos.h`
- `include/Game/ShopEngine.h`
- `include/Game/TitleScreen.h`
- `include/HUD/HudTimer.h`
- `include/Import/TmxMapImporter.h`
- `include/Level/LevelDefinition.h`
- `include/Level/LevelDocumentBridge.h`
- `include/Level/LevelLoader.h`
- `include/Level/LevelSerializer.h`
- `include/Level/ObjectCatalog.h`
- `include/Level/WorldTopology.h`
- `include/Map/RegionLayerDocument.h`
- `include/Map/TileLayerDocument.h`
- `include/Project/DocumentRef.h`
- `include/Project/Project.h`
- `include/Project/ProjectSerializer.h`
- `include/Project/SerializerHelpers.h`
- `include/RPG/AdventureSerializer.h`
- `include/RPG/Audio.h`
- `include/RPG/Catalogs/CatalogLoader.h`
- `include/RPG/Catalogs/CatalogLoadersContenido.h`
- `include/RPG/Catalogs/CatalogLoadersCriaturas.h`
- `include/RPG/Catalogs/CatalogLoadersHabilidades.h`
- `include/RPG/Catalogs/CatalogLoadersMundo.h`
- `include/RPG/Catalogs/CatalogLoadersObjetos.h`
- `include/RPG/Catalogs/CatalogLoadersPersonaje.h`
- `include/RPG/Catalogs/CatalogLoadersRasgos.h`
- `include/RPG/CharacterCreation.h`
- `include/RPG/ConditionEngine.h`
- `include/RPG/Defense.h`
- `include/RPG/Definitions/DefinicionesContenido.h`
- `include/RPG/Definitions/DefinicionesCriaturas.h`
- `include/RPG/Definitions/DefinicionesMundo.h`
- `include/RPG/Definitions/DefinicionesObjetos.h`
- `include/RPG/Definitions/DefinicionesPersonaje.h`
- `include/RPG/Definitions/DefinicionesRasgos.h`
- `include/RPG/Definitions/RpgCoreDefinitions.h`
- `include/RPG/Definitions/SkillDefinition.h`
- `include/RPG/EventosInterno.h`
- `include/RPG/InventoryEngine.h`
- `include/RPG/LootResolver.h`
- `include/RPG/Narrative/AdventureScript.h`
- `include/RPG/Narrative/NarrativeBeat.h`
- `include/RPG/Narrative/NarrativeState.h`
- `include/RPG/SessionCommandHost.h`
- `include/RPG/SkillTarget.h`
- `include/RPG/Stat.h`
- `include/Render/BitmapFont.h`
- `include/Render/BlobShadow.h`
- `include/Render/Camera.h`
- `include/Render/CapturaGL.h`
- `include/Render/ColorLUT.h`
- `include/Render/Culling.h`
- `include/Render/DynamicLightMap.h`
- `include/Render/Enemy.h`
- `include/Render/Entity.h`
- `include/Render/FogOfWar.h`
- `include/Render/FormatoPPM.h`
- `include/Render/Framebuffer.h`
- `include/Render/HudManager.h`
- `include/Render/ICombatant.h`
- `include/Render/IRenderable.h`
- `include/Render/OcclusionRules.h`
- `include/Render/SpriteBatch.h`
- `include/Render/StaticEntity.h`
- `include/Runtime/RuntimeTile.h`
- `include/Runtime/RuntimeTileMapBuilder.h`
- `include/World/EntityHandle.h`
- `src/Core/Log/ConsoleSink.cpp`
- `src/Core/Log/FileSink.cpp`
- `src/Core/Log/LogConfig.cpp`
- `src/Core/Log/LogFormat.cpp`
- `src/Core/Log/Logger.cpp`
- `src/Core/Resources/Shader.cpp`
- `src/Core/Resources/ShaderManager.cpp`
- `src/Core/Resources/Texture.cpp`
- `src/Core/Resources/TextureAtlas.cpp`
- `src/Core/Resources/TextureManager.cpp`
- `src/Editor/CatalogPickerState.cpp`
- `src/Editor/EditorDocumentIO.cpp`
- `src/Editor/EditorStateExportar.cpp`
- `src/Editor/EditorStateHelpers.cpp`
- `src/Editor/EditorStateObjetos.cpp`
- `src/Editor/EditorStatePintar.cpp`
- `src/Editor/ObjectInspector.cpp`
- `src/Editor/ProblemsNavigation.cpp`
- `src/Editor/ProblemsSearch.cpp`
- `src/Editor/ProjectHub.cpp`
- `src/Engine/ApplicationBucle.cpp`
- `src/Engine/ApplicationCierre.cpp`
- `src/Engine/ApplicationLoop.cpp`
- `src/Engine/ApplicationNivel.cpp`
- `src/Engine/Input/InputSystem.cpp`
- `src/Engine/Scene/SceneManager.cpp`
- `src/Engine/Window.cpp`
- `src/Game/BattleMenuActions.cpp`
- `src/Game/BattleMenuState.cpp`
- `src/Game/EnemyBrain.cpp`
- `src/Game/GameOverScreen.cpp`
- `src/Game/GameSessionComercio.cpp`
- `src/Game/GameSessionInteraccion.cpp`
- `src/Game/GameSessionInventario.cpp`
- `src/Game/GameSessionNarrativa.cpp`
- `src/Game/Morality.cpp`
- `src/Game/NewGameFlow.cpp`
- `src/Game/OptionsLocaleBridge.cpp`
- `src/Game/OptionsState.cpp`
- `src/Game/PauseMenu.cpp`
- `src/Game/SaveGameFormat.cpp`
- `src/Game/ShopEngine.cpp`
- `src/Game/Skill.cpp`
- `src/Game/TitleMenuState.cpp`
- `src/Game/TitleScreen.cpp`
- `src/HUD/HudTimer.cpp`
- `src/Import/TmxMapImporter.cpp`
- `src/Level/LevelDocumentBridge.cpp`
- `src/Level/LevelLoader.cpp`
- `src/Level/LevelSerializer.cpp`
- `src/Level/ObjectCatalog.cpp`
- `src/Map/EntityLayerDocument.cpp`
- `src/Map/EventLayerDocument.cpp`
- `src/Map/MapDocumentEdits.cpp`
- `src/Map/RegionLayerDocument.cpp`
- `src/Map/TileLayerDocument.cpp`
- `src/Project/MapDeserializer.cpp`
- `src/Project/MapSerializer.cpp`
- `src/Project/Project.cpp`
- `src/Project/ProjectDatabaseState.cpp`
- `src/Project/ProjectIndexDuplicate.cpp`
- `src/Project/SerializerHelpers.cpp`
- `src/RPG/AdventureBeatValidation.cpp`
- `src/RPG/AdventureSerializer.cpp`
- `src/RPG/Audio.cpp`
- `src/RPG/CharacterCreationScreen.cpp`
- `src/RPG/DicePoolEngine.cpp`
- `src/RPG/Eventos/AnfitrionNarrativo.cpp`
- `src/RPG/Eventos/CondicionesYPaginas.cpp`
- `src/RPG/Eventos/ValidacionDeComandas.cpp`
- `src/RPG/LootResolver.cpp`
- `src/RPG/Narrative/AdventureScriptLoader.cpp`
- `src/RPG/Narrative/NarrativeState.cpp`
- `src/RPG/SkillDefinitionEnums.cpp`
- `src/RPG/SkillEffect.cpp`
- `src/RPG/TierRules.cpp`
- `src/Render/BlobShadow.cpp`
- `src/Render/Camera.cpp`
- `src/Render/CapturaGL.cpp`
- `src/Render/ColorLUT.cpp`
- `src/Render/Enemy.cpp`
- `src/Render/Entity.cpp`
- `src/Render/FormatoPPM.cpp`
- `src/Render/Framebuffer.cpp`
- `src/Render/HudElement.cpp`
- `src/Render/HudManager.cpp`
- `src/Render/SpriteBatch.cpp`
- `src/Render/TileMap.cpp`
- `src/Runtime/RuntimeTileMap.cpp`
- `src/Runtime/RuntimeTileMapBuilder.cpp`
- `tests/CharacterCreationFlowTests.cpp`
- `tests/Core/LogTests.cpp`
- `tests/Core/ScopeGuardTests.cpp`
- `tests/Core/StrongIdTests.cpp`
- `tests/Core/TextureTests.cpp`
- `tests/Editor/AssetBrowserTests.cpp`
- `tests/Editor/CatalogPickerTests.cpp`
- `tests/Editor/CommandListTests.cpp`
- `tests/Editor/E09BridgeTests.cpp`
- `tests/Editor/EditorDocumentIOTests.cpp`
- `tests/Editor/EditorOpcionesTests.cpp`
- `tests/Editor/EditorStateHistoryTests.cpp`
- `tests/Editor/LevelEditorNucleoTests.cpp`
- `tests/Editor/MapPropertiesTests.cpp`
- `tests/Editor/ProjectDatabaseTests.cpp`
- `tests/Engine/ApplicationCableadoTests.cpp`
- `tests/Engine/ApplicationLoopTests.cpp`
- `tests/Engine/ApplicationNarrativeWiringTests.cpp`
- `tests/Engine/ApplicationSceneTests.cpp`
- `tests/Engine/ApplicationShutdownTests.cpp`
- `tests/Engine/CameraTransitionTests.cpp`
- `tests/Engine/InputSystemTests.cpp`
- `tests/Engine/LocaleExtensionTests.cpp`
- `tests/Game/BattleMenuTests.cpp`
- `tests/Game/BusinessEconomyTests.cpp`
- `tests/Game/BusinessModeTests.cpp`
- `tests/Game/CarryOverTests.cpp`
- `tests/Game/EnemyBrainTests.cpp`
- `tests/Game/EnemyCatalogBridgeTests.cpp`
- `tests/Game/EnemyCatalogRealAssetsTests.cpp`
- `tests/Game/G10PauseAndHudTests.cpp`
- `tests/Game/G13CombatCommandsTests.cpp`
- `tests/Game/G14HudTimersAndCommerceTests.cpp`
- `tests/Game/GDB12ScreenTests.cpp`
- `tests/Game/GameModeTests.cpp`
- `tests/Game/MoralityTests.cpp`
- `tests/Game/NewGameFlowTests.cpp`
- `tests/Game/OptionsLocaleBridgeTests.cpp`
- `tests/Game/OptionsStateTests.cpp`
- `tests/Game/PlayerPathLogTests.cpp`
- `tests/Game/ShopFlowTests.cpp`
- `tests/Game/SkillTests.cpp`
- `tests/Game/StubsGLG14.cpp`
- `tests/Game/StubsSkillEffectG13.cpp`
- `tests/Game/TitleFlowTests.cpp`
- `tests/Game/TitleScreenTests.cpp`
- `tests/Import/TmxImporterTests.cpp`
- `tests/Integration/MapRoundTripTests.cpp`
- `tests/Level/LevelDocumentBridgeTests.cpp`
- `tests/Level/ObjectCatalogTests.cpp`
- `tests/Level/WorldTopologyTests.cpp`
- `tests/Map/EventLayerDocumentTests.cpp`
- `tests/Project/ManifestTests.cpp`
- `tests/Project/ProjectDatabaseTests.cpp`
- `tests/Project/ProjectLevelsTests.cpp`
- `tests/Project/ProjectTests.cpp`
- `tests/Project/SerializationTests.cpp`
- `tests/RPG/AdventureValidationTests.cpp`
- `tests/RPG/AudioTests.cpp`
- `tests/RPG/BoundingtonTutorialTests.cpp`
- `tests/RPG/CharacterCreationFlowTests.cpp`
- `tests/RPG/CharacterCreationTests.cpp`
- `tests/RPG/ClassesRacesBackgroundsCatalogTests.cpp`
- `tests/RPG/ConditionsRealAssetsTests.cpp`
- `tests/RPG/EncounterTableTests.cpp`
- `tests/RPG/EventCommandsTests.cpp`
- `tests/RPG/EventDebugTests.cpp`
- `tests/RPG/EventInterpreterTests.cpp`
- `tests/RPG/LootResolverTests.cpp`
- `tests/RPG/MonsterCatalogTests.cpp`
- `tests/RPG/MonsterPlanProofTests.cpp`
- `tests/RPG/NarrativeVariablesTests.cpp`
- `tests/RPG/ObjectAndLootCatalogTests.cpp`
- `tests/RPG/RestOfCatalogsTests.cpp`
- `tests/RPG/SheetEquipmentCapTests.cpp`
- `tests/RPG/SheetLifeCapTests.cpp`
- `tests/RPG/SkillEffectRealAssetsTests.cpp`
- `tests/RPG/SkillExecutorTests.cpp`
- `tests/RPG/SpeedZeroFlagTests.cpp`
- `tests/Render/AnimatedEntityTests.cpp`
- `tests/Render/CameraTests.cpp`
- `tests/Render/CullingTests.cpp`
- `tests/Render/EnemyTests.cpp`
- `tests/Render/EntityTests.cpp`
- `tests/Render/FormatoPPMTests.cpp`
- `tests/Render/HudMinimapTests.cpp`
- `tests/Render/HudTransformTests.cpp`
- `tests/Render/OcclusionRulesTests.cpp`
- `tests/Render/OcclusionTests.cpp`
- `tests/Render/PlayerPopulateTests.cpp`
- `tests/Render/StaticEntityTests.cpp`
- `tests/Runtime/RuntimeTileMapBuilderTests.cpp`
- `tests/TestHarness.h`
- `tests/World/WorldTests.cpp`

</details>
