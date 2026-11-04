# Decisions pendents d'Oriol (recull a 03/11/2026)

Recull de tot el que espera una decisió o una acció teva. Fonts:
- les tandes UNI-02-11 (02–03/11), REPLICA-R1 (30–31/10) i FRONT-01 (29–30/10);
- `BANDEJA_PRODUCT_OWNER.md`, la bandeja oficial des del 14/10;
- les decisions `proposed` de `decisiones/`;
- la secció «Esperando a Oriol» de `TABLERO.md`;
- les revisions de la fàbrica nocturna (`out/fabrica/agy/revisiones/`).

Cada punt porta la meva **recomanació** i on es decideix. Les rutes de codi són de la branca `agente/claude/UNI-02-11` (worktree `~/onegai-ramas/UNI-02-11`).

**Com contestar:** posa la teva resposta al costat de cada número (n'hi ha prou amb «sí», «no», «A» o «com dius»). Jo la passo a `decisiones/` i a la bandeja.

---

## 1. Només ho pots fer tu (accions)

| # | Què | Per què | Recomanació |
|---|---|---|---|
| 1.1 | **Decidir si `agente/claude/UNI-02-11` passa a ser la nova branca d'integració** | `claude/unificacion-28-10` es va perdre amb el `.git`. La branca unifica tot el que s'ha recuperat i la suite gl-free passa **290/290** (`e43db51d`). | Revisar-la (`out/fabrica/UNI-02-11/INFORME.md`) i fer-la la base de tot el que vingui. No hi ha cap merge ni cap push fet. |
| 1.2 | **Jugar-la al Mac** (vistos visuals) | Hi ha coses que no es poden verificar sense GL. | Llista: càmera nova (U4-01, D-O5); animació de PNJ (U3-06, D-O4; guia a `out/fabrica/REPLICA-R1/U3-06.E07_verificacion.md`); pols de passos (U4-07); mesa de combat amb **5 enemics** (placa compacta de 50 px); **persecució dels sectaris** [Sigilo] al dia 1; **Ocaso** deixant sonar les campanes sense avisar; `level_editor` amb els atles restaurats. |
| 1.3 | **PO-04: permís per commitejar i publicar** (pendent des del 14/10) | Ara mateix ningú fa push. | A: fas tu `git push`. B: connectes GitHub a claude.ai. |
| 1.4 | **Renovar la clau de Qwen** (Bailian *Token Plan*) | Respon «Invalid API-key»; ara Codex en fa el paper. | Renovar-la si vols tornar a tenir Qwen d'analista. Si no, seguim amb Codex. |
| 1.5 | **Permisos d'Antigravity per a la fàbrica de nit** | 3 de 3 execucions reals tallades: no pot fer servir el terminal ni escriure. E-CAN1 està encallada per això. | Afegir a `settings.json` d'Antigravity `permissions.allow` de només lectura (`grep`, `sed -n`, `ls`, `git -c core.checkStat=minimal …`) i `write_file` al seu fitxer de relleu (proposta a `out/fabrica/agy/revisiones/2026-11-03.md`). |
| 1.6 | **Script nocturn `agy_nocturno.sh`** | Fa servir com a base `claude/unificacion-28-10`, que ja no existeix, i demana escriure `.agy_relevo.md` sense tenir permís. | Canviar la base per la branca de 1.1 i demanar «relleu només a la resposta». És un script teu: no el toco. |
| 1.7 | **Fitxers d'àudio** | `assets/audio/` no té cap so (només llicències). Sense sons, el lot de so no es pot sentir. | Baixar els primers sons (CC0 o els d'Erang, amb la seva fila a `LICENCIAS.md`). |
| 1.8 | **Sprites amb PixelLab** | Vas dir que PixelLab torna a funcionar, però no sé com vols que hi treballi. | O generes tu i jo importo amb `tools/pixellab/`, o em dius quina connexió puc fer servir. |

## 2. Lots de joc per aprovar (Pla U i altres)

Cap E de codi del Pla U està autoritzada sense el teu vist-i-plau amb els fitxers congelats.

| # | Lot | Què inclou | Recomanació |
|---|---|---|---|
| 2.1 | **So U1-01 (E01–E09)** | Mapa de cues (JSON), música per lloc, efecte per resultat, fundido, volums i tolerància a falta de dispositiu; tot gl-free. `AudioDirector` existeix però no està connectat al joc. | Aprovar-lo quan hi hagi algun so (1.7). |
| 2.2 | **Missions U3-01 (E01–E04)** | Inventari dels dos camins narratius (beats i events de mapa), normalitzar condicions i efectes, triar pàgina. | Aprovar-lo: és la base per unificar missions i events. |
| 2.3 | **Diari de missions: H3-02 o MISIONES-01** (TABLERO, sense resposta) | Dues meitats del mateix feature: un contracte explícit (`quests.json`) i una vista de les flags. | Combinar-les: H3-02 aporta les dades i MISIONES-01 la vista. S'ha de decidir abans de 2.2. |
| 2.4 | **`simular_economia.py` a 4 dies** | Encara compta 3 dies: amb dades reals descartaria el dia 4 i barrejaria Gorran amb el dia 2. | Sí; és petit. |
| 2.5 | **Cobertura real de premis de l'economia** | La prova arriba al final, però en un recorregut real només es disparen 15 de 40 premis (història) i 21 de 63 (explora/optimitza). | Una tanda perquè la prova exigeixi els premis de debò. |
| 2.6 | **El validador de beats també ha de veure cinemàtiques i events del món** | Ara no en veu els efectes, així que podria deixar passar un altre encallament com el de l'Ocaso. | Sí. |

## 3. Decisions de joc sortides d'aquesta tanda (02–03/11)

| # | Què | Recomanació |
|---|---|---|
| 3.1 | **Emblemes de Gorran** que han sortit de `lugares.json` en corregir-lo: `antecova_gorran` (funció, emblema `lore`), 6 amb emblema `mapa` i els textos `_propuesta_ui_puertas_01`. Literal a `out/fabrica/UNI-02-11/EMBLEMAS_GORRAN_PENDIENTES.md`. | Decidir-ne un per un. Si tornes la fitxa d'`antecova_gorran`, la prova dels cartells hi pot tornar. |
| 3.2 | **Dia 2 a les proves:** els 4 «líos» de la vila de Gorran, la subhasta i el «Subir a dormir» es donen per fets (`gorran_misiones=3`), tant a les demos com a l'economia. La xerrada directa amb Gorran (`b2_gorran_hablar`) no té prova. | Acceptar-ho per ara i fer més endavant una E que jugui els «líos» de debò. |
| 3.3 | **Placa compacta del combat de 60 a 50 px** (i les fitxes d'estat 6 px més amunt) perquè hi càpiguen 5 enemics. | Mirar-ho en una captura (1.2). |
| 3.4 | **Ordre de dies:** el dia 2 és ara el de Gorran i el vell «segon dia» és el dia 3 (GORRAN-DIA2, 22/10). Les proves ja ho segueixen. | Confirmar que és la decisió definitiva. |
| 3.5 | **`genera.py`** (`out/claude_dialogos02/guiones/`, fora de git) regenera `demo_boundington*.cpp` i esborraria els guions nous de les demos (H2 i H4). | Retirar-lo o posar-lo al dia. Fins aleshores, no l'executeu. |
| 3.6 | **`tools/parche_c_trama.py`** ja no funciona (`:229`) i tornaria a deixar la persecució dels sectaris darrere de «Ya hemos hablado». | Retirar-lo. |
| 3.7 | **Deute de `linea_base_core` acceptat** (53 punts: 46 fitxers que cap executable fa servir, sostres de 500 línies, membres de classe). Llista a `DEUDA_LINEA_BASE.md`. | Pagar-lo a poc a poc. Primer, decidir si els fitxers morts s'endollen o s'esborren (és el mateix que H8-06, a la secció 7). |
| 3.8 | **U3-02 D8:** un Enter/Exit d'una regió que cau al fotograma en què acaba una escena es perd. | No bloqueja. Proposo tornar-lo a la cua més endavant. |

## 4. Zones per a Fable (restaurar el que es va perdre)

Detall a `out/fabrica/UNI-02-11/ZONAS_PARA_FABLE.md` i a `copias_finder/`.

| # | Què | Recomanació |
|---|---|---|
| 4.1 | Decoració d'**afueras_ocaso** (Qwen): 34 objectes que no són a `afueras_boundington_ocaso.json`. | Aplicar-la sobre el nivell de la branca o descartar-la; no recuperar el nom vell. |
| 4.2 | **`ciudad_centro.json`** de la teva còpia: ~60 `cartel_*`, `estrado_sermon` i 2 portes que la branca no té. | Comprovar primer si `CartelesPuertas.cpp` ja els genera mentre es juga. |
| 4.3 | **12 cofres `botin_*`** de la teva còpia que no són a la branca. | Regenerar-los amb `tools/repartir_botin.py`, no copiar-los a mà. |
| 4.4 | **7 còpies de Finder « 2»** (`ApplicationPantallas 2.cpp`, `CMakeLists 2.txt`, `InterfazPergamino 2.cpp`…). Les he tret de la branca; hi ha una còpia amb el diff. | Que Fable miri si alguna conté feina perduda. |
| 4.5 | `ojos_grande.png`, `con.png` (CON és un nom reservat a Windows; està ignorat a propòsit) i `Untitled-1.psd` (3 MB). | `ojos_grande`: regenerar-lo amb `tools/capa_ojos_iris.py`. `con.png`: canviar-li el nom. `.psd`: guardar-lo fora del repo. |

## 5. Bandeja del product owner (oberta)

| # | Què | Recomanació (de la bandeja) |
|---|---|---|
| PO-52 | Botí dels dies 2 i 3: `requisitos.dia` al contenidor; sense cadàvers-contenidor. | Ja s'aplica la recomanació des del 16/10; només cal que ho confirmis. |
| PO-53 | Marges de l'economia: les aventures paguen menys del previst (història acaba amb ~330 d'or, no ~840). | Acceptar els saldos reals; si vols més bossa, premis de 60–120 als fites de la ruta principal. Ara que l'economia ja fa 4 dies (Ampliación 3), cal tornar-ho a mesurar. |
| PO-54 | Unir-se al culte el dia 1: Sandes et tracta com un dels seus i la Barriada rebutja l'aliança. | Acceptar-ho, sense obligar a quedar-se amb Sandes. |
| PO-55 | Gorran v2: barraques i plànols (qui dibuixa els 21 nivells), tancament de Gorran, connexió del complot. | Revisar primer les funcions sobre l'Excel de colors; un sol premi de fita per resoldre la Fúria. |
| PO-57 | El botí del dia 3 és als mapes de dia, però el dia 3 es juga als de l'Ocaso: no es pot obrir mai. | Que `repartir_botin.py` el reparteixi als mapes de l'Ocaso. Compte: `afueras_ocaso` no existeix a la branca unificada (vegeu 4.1). |

## 6. Cànon (FRONT-01, vault)

| # | Pregunta | Proposta |
|---|---|---|
| 6.1 | **E-CAN1:** de quin color són els ulls de Venides (blaus o verds) i esborrany de la seva fitxa. | Encallada per 1.5. La pots contestar directament. |
| 6.2 | **E-CAN2:** l'avís de Xila fa sonar les campanes, tot i que la vault només descriu l'avís del jugador. | Sí: és una ampliació teva del 29/10 (text literal teu a `b3_xila_avisa`). |
| 6.3 | **E-CAN3:** `cultista_carcelero` no existeix a la vault (al joc surt 6 vegades). | Renomenar-lo «cultista de la barricada» o lligar-lo a Verina retinguda. Decideixes tu. |
| 6.4 | **E-CAN3:** els 3 músics; la vault només dona una línia per a tot el quartet. | Repartir-la entre els 3 o deixar parlar només el violinista. |

## 7. Decisions `proposed` i temes antics que no he vist tancats

Aquests venen de setembre i d'abans del 14/10. **No he verificat un per un si ja s'han resolt d'una altra manera.**

| # | Què | On |
|---|---|---|
| 7.1 | **D0005:** vocabulari d'efectes d'habilitat (famílies i com s'executen); la recomanada és la B, per onades. | `decisiones/D0005-…` |
| 7.2 | **D0007:** tornar a congelar l'abast real (el comptador de fitxers morts puja de 42 a 50). Va lligat a 3.7. | `decisiones/D0007-…` |
| 7.3 | **D0010:** els números que falten de les set magnituds i les dues que es contradiuen. | `decisiones/D0010-…` |
| 7.4 | **D0011:** verificar `HEAD` des d'un clon net abans de tancar res. | `decisiones/D0011-…` |
| 7.5 | **Neteja:** D0003, D0004 (B), D0006, D0009 i D0012 es van acceptar a D0015 (14/10), però els fitxers encara diuen `proposed`. | Confirmar-ho i els poso a `accepted`. |
| 7.6 | **C8-02 / D0021:** disseny dels events del món (calendari tercer/quart, acció de Xila, rastre). La branca `agente/codex/F-C8` i el fitxer de `/private/tmp` es van perdre amb el `.git`. | Decidir si es refà o es dona per perdut. |
| 7.7 | **H8-06:** `buscar_sobrante.py --muertos` troba 13 funcionalitats acabades que ningú no fa servir (7.645 línies: `AssetBrowserState`, `MapEventRunner`, `TargetSelection`, `Audio`, `QuestLogState`…). | Endollar-les o esborrar-les; és el mateix que 3.7. |
| 7.8 | **BND-ESCENAS-01:** la vida de Sandes (105 PV, CA 16) i qui l'acompanya; fitxa dels guàrdies que retenen Edul i Venides; la festa del Barri Alt (primera o segona nit?); textos que falten. | TABLERO |
| 7.9 | **Reglament Ed. 4:** el cas «marge 1,5 contra 0», l'arrodoniment del crític en pools senars, la vida per tier contra `healthCap`… | TABLERO |
| 7.10 | **Les 4 decisions de les 260 classes** i les **6 preguntes de CARTAS-CLASES-01**. | TABLERO |
| 7.11 | Petits: etiquetes del combat en català o castellà (`Fracàs/Èxit`); el comptador de FPS, producte o depuració; **aixecar la veda de tildes/ñ als JSON d'aventura** (la font ja dibuixa accents); la capital de Bosmurg (Teshkorr o Tor'k Hazar); la llegenda de la Hoja 5 de l'atles; `Mapa_Egaroth.svg` substituït; el diagrama UML, viu o històric; quina numeració de fases mana; RUN-05 contra el `World/` ECS-lite; aprimar `Application.cpp`; què són `MotorGraphico-HUD-Redesign/` i `Incomplete request(.zip)`. | TABLERO (historial) |

---

### El que recomano decidir primer
1. **1.1 i 1.2:** acceptar la branca unificada i jugar-la. Desbloqueja tota la resta.
2. **1.3 (PO-04):** com es publica.
3. **1.5 i 1.6:** permisos i script d'Antigravity; sense això la fàbrica de nit no fa res.
4. **2.2 + 2.3:** missions (el que més fa avançar el joc).
5. **3.1, 3.4, 6.1–6.4:** cànon i contingut de Boundington, que bloquegen poc però s'acumulen.
