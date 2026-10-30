# Evidència de planificació i discrepàncies

Inspecció de lectura el 28/10/2026. Base observada:
`522347660cf4ba9c4693781f21effa6748f35b1c`, branca compartida `claude/escenas01`.
El repo real és **Software/**, amb el motor a **MotorGraphico-main/MotorGraphico/**.
El treball nou viu exclusivament a `planificacion/pla_unity/descomposicio/`, més
els registres obligatoris de coordinació. No s’han modificat els encàrrecs adjunts.

La font del pla s’ha llegit del ZIP com a dades literals AST. No s’ha importat
ni executat cap generador o eina rebuda. El hash del ZIP i la taula original
queden a [referencia_original.json](referencia_original.json). Els hash dels
fitxers de codi efectivament llegits queden a `fonts_verificades` de [pla.json](pla.json).
Quan un fitxer té WIP, es distingeix el hash de l’arbre del commit base.

## Canvis que el pla ha d’absorbir

| Porta | Evidència llegida | Proposta i efecte al pla |
|---|---|---|
| `POOL_REFERENCIES` | `World.h` usa deque; `HandlePool.h` usa vector; handles generacionals ja existeixen. | Caracteritzar abans de substituir. Cap segon pool. Preservar referències durant callbacks amb altes. |
| `ZONA_GAMESESSION` | `GameSession::update` actualitza brains i rebutja la casella del jugador. | Afegir explícitament adapter de percepció/contacte i zona de sessió al pla aprovat; no donar-ho per cobert per l’adjunt. |
| `DETECCIO` | `EnemyBrain::update` assigna kPatrol en cada actualització; kChase existeix. | Separar percepció, temps, ruta, transició, retorn i petició de combat. Acordar mètrica i límits 4/8/3. |
| `RANURES` | `SaveSlots::kSlotCount=6`, `kQuickSlot=0`; ja hi ha servei i recuperació. | Conservar sis com a recomanació fins que Oriol decideixi; no crear servei duplicat ni reduir ranures silenciosament. |
| `AUTOSAVE_RENAME` | Hi ha escriptura de SaveGame i comentari de substitució atòmica. | Verificar implementació i comportament Windows; no confondre rename atòmic amb persistència davant apagada. |
| `AUDIO_ASSETS` | Recompte d’extensions wav/ogg/mp3/flac dins assets: **0**. | Cablejat i acceptació audible separats. El paquet audible no acaba sense recursos i prova real. |
| `AUDIO_FORMATS` | AudioDirector ja té bind, cues, playMusic i applyVolumes; declara backend miniaudio. | Auditar backend/build; usar el director actual. Formats i disponibilitat multiplataforma pendents de verificació d’execució. |
| `FRANGES` | `FranjasDelDia.h`: kFranjasPorDia=3, manana/tarde/noche. | No persistir una quarta franja a partir del text adjunt. Capvespre visual com a proposta de transició. |
| `PNJ_NOCTURNS` | RutinasPnj ja es munta al nivell i avança des de GameSession. | Afegir selecció d’horari; respectar diàleg, altura i origenDe. Identitat entre mapes necessita contracte. |
| `TONS` | La llum és acceptació visual i no hi ha paleta aprovada al paquet. | Colors en dades, captures amb mateixa càmera i revisió d’Oriol abans de tancar. |
| `APPLICATION_WIP` | Application.cpp té 1.251 línies a l’arbre llegit i WIP aliè; Application.h 518; GameSession.h 545. | Cap creixement dels fitxers per sobre del sostre. Relleu anunciat amb origen més curt; propietari únic d’integració. |
| `FABRICA_ADOPCIO` | L’eina és als ZIP, encara no instal·lada ni validada aquí. | PREP revisa abans d’executar. Regex ajustades sobre sortida íntegra i base immutable. |
| `HOOK_MONOREPO` | hooksPath apunta al hook nested, però el script busca `$RAIZ/MotorGraphico/tools/linea_base_core.py`. Aquest camí no existeix des de Software. | El hook pot sortir 0 abans d’executar la guardia. PREP proposa una correcció i prova del camí real; aquesta sessió no canvia configuració ni hooks. |
| `GUARDIA_HERETADA` | Executar explícitament `python3 tools/linea_base_core.py --rapido --indice` dona **49 símbols duplicats**; els quatre fitxers «2.cpp» implicats ja estan versionats. | Base vermella que cal registrar i resoldre en zona pròpia abans del producte; el pla no toca aquestes fonts ni presenta la guardia com a verda. |
| `PRESETS` | CMakePresets.json: macos-debug i windows-vs2022-debug. No té preset Linux declarat. | La suposició windows-debug de fabrica no és vàlida. Per Linux cal configurar amb ordre comprovada o nou preset revisat. |
| `EDITOR_UNIC` | El front antic és `editor/src/level_editor.cpp`, no `examples/level_editor.cpp` com diu documentació antiga. | Matriu de paritat i retirada condicionada. No planificar una edició sobre ruta absent. |
| `DEC10` | MapEventRunner ja té política provisional de pàgines i automàtics; EventInterpreter té stepOnce/trace. | Decidir prioritat i rearmament abans de normalitzar events; completar panell debug, no duplicar runtime. |
| `DEC06` | La bitàcora documenta playtest en procés separat. | Conservar la decisió vigent mentre no s’aprovi canviar-la; comparativa de cost abans d’incrustar joc. |

Altres portes d’U2–U4 (`LIMIT_MODUL`, `ENTRADA_PROJECTE`, `COMPAT_EVENTS`,
`TEMPS_TWEEN`, `CLIPS_FORMAT`, `RELOAD_ESTAT`, `TIMELINE_CONFLICTES`, `SKIP_EFECTES`,
`PREFAB_OVERRIDE`, `PRESSUPOST_PARTICULES`) són **incerteses de disseny registrades**,
no bugs confirmats. La primera E corresponent les concreta sobre l’API real.

## Treball aliè preservat

En començar hi havia canvis a Application, ShaderManager, AuraJugador/AuraMoral,
demo_brillo_equipo, proves de HUD/sprites, assets d’ales i arxius de coordinació,
entre d’altres; també sessions no versionades i directoris temporals. No s’han
desfet, inclòs al commit ni utilitzat com a aprovació per canviar producte.
`TABLERO.md` i `sesiones/INDICE.md` ja contenien canvis aliens: la traça d’aquesta
sessió es manté separada i el commit només ha d’incloure els fragments propis.

La llei activa de 500 línies i 15 membres preval sobre el sostre antic de 600
de l’adjunt i de documentació històrica. No es modifica la bitàcora congelada.

## Què acredita aquesta entrega

Les eines locals comproven cobertura dels 37 paquets, preservació de les 128
jornades i dependències originals, IDs únics, absència de cicles incloent ports
entre paquets, dependència del final de totes les seves E, sostres A–E, S sense
durada compromesa i derivats reproduïbles. La prova d’integritat també comprova
que una E no pugui passar a preparada sense rutes i contracte v1.

Sí s’ha executat la part ràpida de `linea_base_core` explícitament: vermell heretat
amb 49 problemes. Els fitxers duplicats són `ApplicationPantallas 2.cpp`,
`CartasJugar 2.cpp`, `InterfazPergamino 2.cpp` i `LevelEditorPaleta 2.cpp`.
S’ha contrastat que estan versionats; no s’han esborrat com si fossin temporals.
Les nou fonts implicades en els missatges de duplicació coincideixen byte a
byte amb el commit base: aquesta fallada ja és a HEAD, no l’ha creat el pla.

No s’ha executat `validar_todo`, build, CTest del motor, `fabrica` ni cap prova
jugable, perquè aquest lliurament no canvia producte. La validació del pla no
és una prova de compilació de les signatures proposades. La reproducció del
generador i la validació des d’un clon net del commit es registren a la sessió.

Reproducció feta sobre `d296ccf45fb1cd1ad8cb5fab69f518ed5810c17f`: clon temporal
amb checkout dels fitxers de planificació del commit; `valida.py` i
`renderitza.py --comprovar` passen i el clon continua net. No s’hi han copiat
arxius sense commit. Els 51 fitxers del pla i els fragments propis de coordinació
estan en una branca independent; la branca compartida continua al commit base.
