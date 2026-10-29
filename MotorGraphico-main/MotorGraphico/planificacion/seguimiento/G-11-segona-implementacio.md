# G-11 — dues implementacions simultànies. Expedient per decidir

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe el expediente para elegir entre dos implementaciones simultaneas de G-11, foto del 14/09/2026. La decision esta tomada y el codigo elegido es `include/Game/SaveSlotState.h` + la libreria `motor_saves` (H0-05); DEC-09 la contesto Oriol. De las 9 citas de codigo que contiene, 8 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

**Data:** 14/09/2026 · **Autor de l'expedient:** Claude (Cowork), sessió d'Oriol
**Estat:** cap de les dues està committejada. **No s'integra res sense que l'Oriol triï.**

## Què ha passat

Dues sessions han obert G-11 amb minuts de diferència i sense saber-ho:

| | Sessió A (l'altra) | Sessió B (aquesta) |
|---|---|---|
| Primers fitxers | 07:03–07:04 | ~07:05 (agent llançat a les 07:00) |
| Fitxers | `include/Game/SaveSlotState.h`, `src/Game/SaveSlotState.cpp`, `tests/Game/SaveSlotsTests.cpp`, `cmake/SaveSlotsTests.cmake` | **els mateixos noms** + `tests/Game/SaveSlotTests.cpp`, `cmake/SaveSlotTests.cmake` |
| On és | escrita a la carpeta del projecte (sense versionar) | **NO escrita**: al pedaç que acompanya aquest expedient |
| Registre | `seguimiento/G-11.json` ja existeix | cap (no se n'ha escrit per no duplicar) |

És exactament la col·lisió que `PARALLEL_DEVELOPMENT_BOARD.md` descriu amb
les dues Places de les Races del 16/08, i per la mateixa causa que allà
s'apuntava: **ningú marca la fila abans de crear el primer fitxer.** Un
mecanisme que ho hauria evitat existeix des d'aleshores i no s'ha fet
servir en cap de les dues sessions.

En descobrir-ho, aquesta sessió **no ha escrit els seus fitxers** a la
carpeta del projecte, per no fer el que es va fer al 16/08 (dues coses
vives amb el mateix nom i algú havent de triar-ne una a posteriori).

## Com es comparen

Totes dues resolen el nucli sense pantalla i coincideixen en el que
importa: ranures per projecte, estat `Corrupt` distingit d'`Incompatible`
amb el seu motiu, escriptura atòmica, càrrega tot-o-res que no toca la
sessió en curs, i DEC-09 derivat de l'estat que ja existeix en comptes
d'una bandera nova. Les diferències reals:

| | Sessió A | Sessió B |
|---|---|---|
| On viuen les ranures | **una carpeta per projecte** (`directory()`) | **prefix al nom del fitxer** (`partida_<projecte>_<ranura>.json`), per no necessitar `mkdir` — `ProjectHub.h` diu que el motor no en té cap |
| Nombre de ranures | 6 numerades + ràpida (índex 0) | 3 numerades + ràpida |
| Esborrar una ranura | sí (`remove()`, falla si no hi ha res) | no |
| Confirmació de sobreescriptura | la decideix qui crida | viu al servei (`FailIfOccupied` + `needsOverwriteConfirm`) |
| Guardat d'un altre projecte | — | es detecta (`ForeignProject`) |
| Rellotge | del sistema | injectable, perquè la prova no depengui de l'hora |
| Estat de pantalla | fora d'abast (diu que va amb G-10) | `SaveSlotScreen` gl-free, patró `Game::TitleScreen` |
| Cablejat a `Application` | no | sí: F5/F9 passen pel servei, i Continuar / Carregar de la pausa / Carregar del Game Over obren el mateix selector |
| Prova | `test_save_slots`, 88 comprovacions | `test_save_slots`, 10 casos / 173 comprovacions |

## Les tres sortides

1. **Quedar-se la de la sessió A** i llençar el pedaç. És la que ja és a
   la carpeta i la que té registre escrit. Queda pendent el cablejat a
   `Application` (F5/F9 i les tres entrades al selector), que és el que
   converteix el servei en una funcionalitat que el jugador veu.
2. **Quedar-se la de la sessió B** (aplicar el pedaç per sobre). Arriba
   més lluny, però substitueix feina que ja és a la carpeta i el
   `G-11.json` de l'altra sessió s'hauria de reescriure.
3. **Quedar-se la A i portar-hi de la B el que li falta**: el cablejat a
   `Application`, `ForeignProject`, el rellotge injectable i els casos de
   prova que la A no cobreix. És més feina que triar, però és l'única
   sortida que no llença res verificat.

La recomanació d'aquesta sessió és la **3**, i si s'ha de decidir ràpid,
la **1**: el criteri del pla per a G-11 inclou el recorregut de pantalla,
i aquest recorregut s'haurà d'escriure igualment sobre el servei que
quedi.

## Què hi ha al pedaç

`G-11-segona-implementacio.patch`, relatiu a l'arrel de `MotorGraphico/`,
sobre l'arbre del 14/09 a les 06:58. **No s'aplica tal qual**: crearia
`include/Game/SaveSlotState.h` i `src/Game/SaveSlotState.cpp`, que ja
existeixen amb el contingut de la sessió A.

- **Creats:** `include/Game/SaveSlotState.h`, `src/Game/SaveSlotState.cpp`,
  `tests/Game/SaveSlotTests.cpp`, `cmake/SaveSlotTests.cmake`
- **Editats:** `include/Game/SaveGame.h` i `src/Game/SaveGame.cpp` (tres
  camps additius `projectId`/`characterName`/`savedAt`, **sense pujar
  `schemaVersion`**, de manera que un guardat anterior reescrit dona el
  mateix text byte a byte), `include/Engine/Application.h`,
  `src/Engine/Application.cpp`, `CMakeLists.txt`
- **Verificació:** `ctest` 98/105 sobre l'arbre de les 06:58 (línia base
  97/104); `test_save_slots` 10/10 amb 173 comprovacions, determinista i
  sense deixar fitxers; `test_save_game` 19/19 intacte. El codi GL es va
  validar amb `g++ -fsyntax-only` perquè el contenidor no construeix
  `motor_core`.

## El forat que totes dues comparteixen, i que no és de G-11

`RPG::CharacterSheet` (classe, raça, estadístiques, `equippedIds`) **no
se serialitza enlloc del motor**: la construeix qui crea el personatge i
viu a l'`Application`. El criteri de G-11 demana comprovar «fitxa, equip,
inventari, moral i flags» en tornar d'un guardat. Inventari, moral i
flags hi són; **fitxa i equipament no**, amb cap de les dues
implementacions. Tancar-ho és ampliar el format de guardat i toca G-07
(congelació dels esquemes). Convé decidir-ho abans de declarar G-11 tancat,
o el recorregut d'acceptació de G-16 hi topará.
