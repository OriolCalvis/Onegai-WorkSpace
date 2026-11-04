# Registre de captures i proves (UNI-02-11) — auditoria crítica

Auditoria de només lectura del worktree `~/onegai-ramas/UNI-02-11` (HEAD `3414b0cd`, 04/11). Cada PNG (44) de `out/fabrica/UNI-02-11/capturas/` s'ha obert amb Read i comparat amb allò que diuen `CAPTURAS.md`, `MATRIZ_DIA1/GORRAN/DIA3.md`, els guions i els missatges de commit. Les proves s'han comptat al codi i contrastat amb els `ctest_final*.log`. No s'ha compilat ni executat res.

Lectura prèvia:
- Les captures de `g_`, `s_` i `00-05` no tenen cap taula pròpia d'afirmacions. Només n'hi ha a `CAPTURAS.md` (00-05) i als missatges de commit. Per a les altres la columna «Què diem» recull la frase del guió, del commit o de la matriu.
- Dins les imatges, el HUD amaga l'or, l'objectiu i el minimapa mentre hi ha un quadre de diàleg. Per això l'or, el dia i els PV només es poden verificar a les captures sense diàleg.
- El jugador vestit o amb cos base s'ha mirat a ull. «Vestit» vol dir amb pell, cabells i roba; «cos base» vol dir el maniquí nu.

## 1. Taula de captures (44 PNG)

Llegenda: ✅ coincideix, ⚠ parcial, ❌ no coincideix. Commit d'origen via `git log` sobre la carpeta. «Guió / partida» és el que s'hi va fer servir segons el guió.

### 1.1 Mode fum (`./juego [--nivel X] N`, sense guió) — commit `e7e39281`

| Fitxer | Guió / partida | Què diem | Què es veu de debò | Veredicte |
|---|---|---|---|---|
| `00_titol_f60.png` | cap | Títol amb Continuar / Nueva partida / Opciones / Salir sobre la ciutat | Pergamí amb les 4 opcions, «No hay ninguna partida guardada», sobre el Casc Antic | ✅ Coincideix |
| `01_ciudad_centro_f90.png` | `--nivel ciudad_centro` | HUD (PV, or, objectiu), minimapa, PNJ, riu, edifici translúcid | HUD «Aventurero T1 · 150 · PV 100/100», objectiu del pròleg 2/5, minimapa «Casco Antiguo», PNJ, edifici translúcid tapant el jugador. Jugador amb **cos base** (nu), com avisa el propi doc | ✅ Coincideix (cos base reconegut al doc) |
| `02_ciudad_centro_f100.png` | idem, 10 fotogrames més | Igual que 01 | **Visualment idèntica a 01** (md5 diferent només per petit soroll): jugador quiet, res no canvia. No aporta cap informació nova | ⚠ Gairebé duplicada de 01 |
| `03_ciudad_centro_ocaso_f300.png` | `--nivel ciudad_centro_ocaso` | «Boundington al ocaso, es veu de dia» | Minimapa diu «Boundington en el ocaso (Oeste y Casco)»; escena amb llum de dia, sense tint. Cos base. Objectiu encara del pròleg | ✅ Coincideix (el doc ja diu que no hi ha tint) |
| `04_afueras_ocaso_f300.png` | `--nivel afueras_ocaso` | Afores al ocaso, de dia i molt buides | Praderia verda, 3-4 cases, riu, mur; minimapa «Las Afueras... al ocaso». Realment buit i de dia. Cos base | ✅ Coincideix |
| `05_prado_del_rio_f120.png` | `--nivel prado_del_rio` | «El nivell inicial carrega» | Prat quasi buit (herba, riu, camí), narració «Oscuridad. Caes desde el vacío...» 1/4, HUD sense or. **Sense arbres, cofre ni pont**, que sí surten a `g_prologo_dialogo` (partida normal). Cos base | ⚠ Parcial: carrega, però sense la decoració del mateix mapa en partida real; el doc no ho diu |

### 1.2 Mode guió, creador i `--nivel` — commit `2197f77c` (E-GUION-01 `78827571`)

| Fitxer | Guió | Què diem | Què es veu de debò | Veredicte |
|---|---|---|---|---|
| `g_titulo.png` | `titulo_nueva_partida.txt` f10 | Títol | Idèntic a `00_titol` (md5 diferent només per renderitzat) | ✅ Coincideix |
| `g_creador_raza.png` | idem f59 | Creador, pas 1 Raza | «Nueva partida · Paso 1 de 7 · Raza», 69 races, Aarakocra de las Montañas triat | ✅ Coincideix |
| `g_tras_raza.png` | idem f76 | Després de la raça | «Paso 2 de 7 · Clase», Almirante Yi (primera de la llista). El nom del fitxer és «tras raza» i mostra la pantalla de classe: correcte | ✅ Coincideix |
| `g_tras_aspecto.png` | idem f104 | Després de l'aspecte | «Paso 4 de 7 · **Origen**» (Mes de la Agonía). No mostra l'Aspecto sinó la pantalla següent | ⚠ Nom ambigu, contingut coherent |
| `g_nombre_elegido.png` | idem f216 | Primera suggerència de nom | «Paso 7 de 7 · Nombre», Nerea, personatge vestit amb el kit (Aarakocra) | ✅ Coincideix |
| `g_prologo_dialogo.png` | idem f260 | Pròleg amb diàleg | Narració 1/4 «Oscuridad. Caes...», HUD «Nerea T1 · PV 9/9», **vestit** (ocell blau) | ✅ Coincideix |
| `g_partida_empieza.png` | idem f533 | Després del pròleg: jugador al prat, **vestit** | Nerea, 150 d'or, PV 9/9, objectiu 0/5 «Abre el cofre», cofre i arbres, minimapa; vestit | ✅ Coincideix |
| `g_partida_paseo.png` | idem f573 | Després de caminar amb D | El jugador s'ha desplaçat uns 30 px respecte de `partida_empieza` i el marcador del minimapa canvia. Vestit | ✅ Coincideix |
| `g_nivel_quieto.png` | `nivel_paseo.txt` f5 | Amb `--nivel`, jugador **vestit** (humà per defecte) | «Aventurero T1 · 150 · PV 100/100», humà vestit amb cabell fosc, Casc Antic | ✅ Coincideix |
| `g_nivel_tras_andar.png` | idem f40 | Després d'una passa | Jugador desplaçat, vestit; objectiu 2/5 | ✅ Coincideix |
| `g_nivel_fin_paseo.png` | idem f95 | Final del passeig | Jugador darrere d'un edifici translúcid, vestit; marcador «21 PASOS» a l'objectiu | ✅ Coincideix |

Observació: el creador per defecte genera un **Aarakocra** (primera raça), no un humà. La frase de la tasca de E-GUION-01 («humà per defecte») només val per a `--nivel` (`g_nivel_*`). A la partida «normal» pel guió es veu un ocell.

### 1.3 Partides desades (`juego --guardado`) — commit `bbcb30f2`, regenerades a `8f7f1aef` (E-GUION-02 i E-DIA1-CONSEC)

Guardats `guardados/{sectarios,ocaso,combate5}.json`, heroi «Ilse» (humà segons el commit; a la imatge es veu humà vestit amb espasa).

| Fitxer | Guió | Què diem | Què es veu de debò | Veredicte |
|---|---|---|---|---|
| `s_sectarios_plaza.png` | `sectarios.txt` f30 | «Dia 1, la plaça: els tres sectaris...» | HUD complet **«Día 1 · Mañana · 125 d'or · PV 11/11»**, objectiu «Preséntate al tendero» 2/8, prompt «E Hablar con Sectario de los Perdidos». Jugador darrere d'edifici translúcid. L'or 125 és coherent amb 150 − 25 (llit) | ✅ Coincideix |
| `s_sectarios_dialogo.png` | idem f50 | Beat `b1_sectarios_seguir` | Narració «Los tres sectarios recogen sus cosas...» i opcions «1 Seguirles de lejos [Sigilo]» / «2 Dejarles ir». Prova visual que el beat de persecució ja és abastable | ✅ Coincideix |
| `s_sectarios_seguimiento.png` | idem f120 | «La persecució dels sectaris» | Només narració 1/3 «A la segunda esquina, el mayor se para y se vuelve». **No es veu cap persecució**: ni sectaris ni moviment, el jugador no s'ha mogut | ⚠ Mostra el text del beat, no la persecució |
| `s_combate5_dialogo.png` | `combate5.txt` f30 | «Diàleg» amb la nena del gat | Narració de lloc «Pico Dragón: casas de madera sin orden...» 1/3. No es veu cap diàleg amb la nena | ⚠ Nom enganyós |
| `s_combate5_pelea.png` | idem f118 | Abans de pelejar | «No te escuchan. El primero ya te está empujando.» amb «ENTER Pelear» | ✅ Coincideix |
| `s_combate5_mesa.png` | idem f170 | Mesa de combat amb **5 enemics i totes les plaques** | Ronda 1, 6 plaques (5 Devotos + Ilse), 5 barres «Devoto de los Perdidos 8/8», registre d'iniciativa amb els 5 devots. **Cada devot mostra una fletxa vermella com a retrat, no una cara/sprite** | ✅ Nombre correcte (5+1) / ⚠ Els retrats són placeholders |
| `s_ocaso_cuartel.png` | `ocaso.txt` f3 | «Ocaso, la base militar, abans de les campanes» | «La Casa de Guardia», minimapa **«Día 4 · Mañana»**, 280 d'or, PV 11/11, objectiu «Sal de Boundington con vida» 2/3, soldats. No hi ha cap campana ni text; és simplement l'arribada | ⚠ Escena correcta, però no «ocaso» (matí) ni campanes |
| `s_ocaso_arenga.png` | idem f160 | «Arenga» | **Pràcticament idèntica a `s_ocaso_cuartel`** (alguns soldats han mogut). Cap quadre de diàleg ni arenga | ❌ No mostra cap arenga |
| `s_ocaso_fundido_negro.png` | idem (no és un nom del guió) | Fosa a negre de l'Ocaso | Pantalla negra amb el HUD (or 280, «Día 4 · Mañana») i un marcador d'objectiu. El guió actual vol `campanas_1..5` i `arenga`, no `fundido_negro`: el fitxer s'ha reanomenat a mà | ⚠ No demostra campanes; origen no traçable |

Nota: dels guions, `sectarios.txt` demana `plaza, dialogo, sigilo, tirada, seguimiento`; només n'hi ha 3. `combate5.txt` demana `antes, dialogo, pelea, mesa_1/2/3`; només hi ha 3 (`mesa` sense numero). `ocaso.txt` demana `cuartel, campanas_1..5, arenga`; només 3 i un d'ells no té nom al guió. Les captures s'han triat i reanomenat sense deixar-ho escrit, així que la correspondència guió→PNG no es pot reproduir tal qual.

### 1.4 Dia 1 cerrat (E-MATRIZ-01) — commit `db93f46b` (14:01, abans de E-MATRIZ-01b)

Guió `m1_unirse.txt` sobre `dia1_edul_unirse.json` i `m1_dudas.txt` sobre `dia1_edul_dudas.json`.

| Fitxer | Què diem | Què es veu de debò | Veredicte |
|---|---|---|---|
| `m1_unirse_despierta.png` / `m1_dudas_despierta.png` | Es carrega el dia 1 tancat i canvia a «El Segundo Día: la montaña de Gorran» (0/7) | Habitació de la Taverna Humilde, HUD «Ilse · **0 d'or** · PV 11/11», minimapa **«Día 1 · Noche»**, aviso «Nuevo objetivo: Rosita... quiere su espada...» 0/7. **Les dues captures són idèntiques byte a byte** | ⚠ Coincideix amb el text; **duplicades** i el doc ho admet |
| `m1_unirse_amanece_1.png` / `m1_dudas_amanece_1.png` | Idem durant el desvaniment | Mateixa escena a mig fosa a gris | ⚠ Duplicades; no distingeixen camins |
| `m1_unirse_amanece_2.png` / `m1_dudas_amanece_2.png` | Fosa a blanc | Fons blanc amb HUD i un marcador d'objectiu. Duplicades | ⚠ Duplicades; informació nul·la |

Les 6 captures són 3 imatges repetides dues vegades (comprovat amb md5), de manera que **no demostren cap diferència entre «unirse» i «dudas»**, que és justament el que es volia mostrar. El HUD dóna «0 d'or» i «Día 1 · Noche»: l'or 0 va ser corregit més tard a `252f53ff` (E-MATRIZ-01b, 150/125 d'or real), però les `m1_` no es van regenerar (només `s_`, a `8f7f1aef`). `MATRIZ_DIA1.md:217` encara diu «el oro es 0 (el arnés no guarda oro)», mentre que `:250` diu que ja són reals: inconsistència interna. Les captures `m1_` són obsoletes pel que fa a l'or.

### 1.5 Dia de Gorran (E-MATRIZ-02 `b41a0790` i E-GORRAN-TREGUA `9628017a`)

| Fitxer | Commit | Guió / partida | Què diem | Què es veu de debò | Veredicte |
|---|---|---|---|---|---|
| `m2_subasta_menu.png` | `b41a0790` | `m2_subasta.txt` f260, `m2_subasta.json` | Saló del recinte amb la Fúria a la taula, pàgina 11/11 i 4 opcions | Saló amb cadires i faristol; narració 11/11 «Y los dos se echan encima del pujador...»; opcions 1-4 correctes (Unirte a la farsa / Llevarte la Furia / Salvarle la cara / Dejar que estalle). **La Fúria a la taula no s'hi veu**; la taula queda tallada a baix. HUD amb PV 11/11; sense or per haver-hi diàleg | ✅ Opcions i text coincideixen / ⚠ «Furia en la mesa» no verificable |
| `m2_subasta_pelea.png` | `b41a0790` | idem f375 | Ronda 1, Gorran 17/17, 2 bandits 6/6, miners aliats, «Zancadilla de la cueva» | Ronda 1, 6 plaques d'iniciativa; barres «Gorran, jefe de... 17/17», «Bandido de Gorran 6/6» i «Bandido de Gorr... 2 6/6»; registre amb «Zancadilla de la cueva» i miners de la Veta Honda | ✅ Coincideix |
| `m2_mia_balance.png` | `b41a0790` | `m2_mia_perdida.txt` f180, `m2_mia_perdida.json` | Carrer del Casc Antic, «Tinet, en casa. Mia, no.» | Carrer davant d'una taverna (rètol de cervesa), narració «Tinet, en casa. Mia, no.». El minimapa no es veu, de manera que **no es pot confirmar el nivell/dia**; és el mateix racó que `03` (nivell «Boundington en el ocaso»), no el Casc Antic de `01` | ⚠ El text coincideix; la ubicació no es pot comprovar |
| `m2_cuatro_balance.png` | `b41a0790` | `m2_cuatro_lios.txt` f180 | Mateix fotograma amb «Tinet y Mia, en casa.» | Fotograma idèntic excepte la frase «Tinet y Mia, en casa.» (contrast intencionat) | ✅ Coincideix |
| `m2_tregua_menu.png` | `9628017a` | `m2_tregua.txt` f260, `m2_tregua.json` | Subasta amb «Proponerle una tregua» en lloc de «Dejar que estalle» | Mateixa escena que `m2_subasta_menu`; opció 4 «Proponerle una tregua» | ✅ Coincideix |
| `m2_tregua_pactos.png` | `9628017a` | idem f375 | Beat `b2_subasta_tregua`: 3 opcions | Gorran (màscara blanca, vestit vermell) parla: «Hay tregua, y una tregua se cumple. Pero la sala me mira. Tu diras.»; opcions «Salvarle la cara a Gorran / Pedirle a Joel con su permiso / Traicionar la tregua» | ✅ Coincideix |
| `m2_tregua_cerrada.png` | `9628017a` | idem f460 | «Tregua tancada» | Narració 2/4 «La sala se ríe con Gorran, no de él. Gorran, rojo, se ríe también.»; no és el final (2 de 4 pàgines) | ⚠ Mig diàleg, no «tancada» |

Fitxers que la documentació descriu però **no existeixen** al directori: `m2_tregua_sin_dialogo.png` i `m2_tregua_atraviesa.png` (a `MATRIZ_GORRAN.md:191-192`; esborrats a `9628017a` quan la tregua es va moure a la subasta). La taula de la Fase C de `MATRIZ_GORRAN.md` no s'ha actualitzat: no esmenta `m2_tregua_menu/pactos/cerrada`, i segueix descrivint com a fet allò que D0024 va canviar (Gorran amagat, sense diàleg). Aquesta part del doc és falsa en l'estat actual.

### 1.6 Dia 3 (E-MATRIZ-03) — commit `3414b0cd`

| Fitxer | Guió / partida | Què diem | Què es veu de debò | Veredicte |
|---|---|---|---|---|
| `m3_fiesta_mansion.png` | `m3_fiesta.txt` f20, `m3_fiesta.json` | Sala de la mansió sense diàleg (el guardat) | «Mansion del Barrio Alto», minimapa **«Día 3 · Noche»**, **120 d'or**, PV 11/11, objectiu «Si traes la Furia de Alaro, devuélvesela a Rosita» 3/4. Sala plena de convidats | ✅ Coincideix |
| `m3_fiesta_noble.png` | idem f70 | Noble que brinda: «Tres copas de mi cuenta (6 de oro)» / «Prefiero hacerle preguntas» | Parlant «NOBLE»: «La copa esta vacia otra vez.» amb les dues opcions exactes | ✅ Coincideix |
| `m3_fiesta_preguntas.png` | idem f130 | «De vuelta a la sala tras elegir 'Prefiero hacerle preguntas'» | **Gairebé idèntica a `m3_fiesta_mansion`** (només canvien uns PNJ). No hi ha cap pista que s'hagi triat res | ⚠ No demostra l'elecció; quasi duplicada |
| `m3_informe_menu.png` | `m3_informe.txt` f50, `m3_informe.json` | Venides: «Siéntate. Hoy no bebo. Hoy cuentas tú.», només dues opcions | Parlant «VENIDES»: «Sientate. Hoy no bebo. Hoy cuentas tu.» amb «Contarselo todo» / «No contarle nada» | ✅ Coincideix |
| `m3_informe_callado.png` | idem f115 | Després de «No contarle nada»: «Se levanta.» 2/3 | Narració «Se levanta.» 2/3 | ✅ Coincideix |

### 1.7 Recompte per veredicte (44 PNG)

- ✅ **27**: 00, 01, 03, 04; `g_` 10 (titulo, creador_raza, tras_raza, nombre_elegido, prologo_dialogo, partida_empieza, partida_paseo, nivel_quieto, nivel_tras_andar, nivel_fin_paseo); `s_` 4 (sectarios_plaza, sectarios_dialogo, combate5_pelea, combate5_mesa amb matís dels retrats); `m2_` 5 (subasta_menu amb matís de la Fúria, subasta_pelea, cuatro_balance, tregua_menu, tregua_pactos); `m3_` 4 (fiesta_mansion, fiesta_noble, informe_menu, informe_callado).
- ⚠ **16**: 02, 05, `g_tras_aspecto`, `s_sectarios_seguimiento`, `s_combate5_dialogo`, `s_ocaso_cuartel`, `s_ocaso_fundido_negro`, els 6 `m1_`, `m2_mia_balance`, `m2_tregua_cerrada`, `m3_fiesta_preguntas`.
- ❌ **1**: `s_ocaso_arenga`.

Duplicats comprovats amb md5: les 3 parelles `m1_unirse_* = m1_dudas_*` (6 fitxers, 3 imatges). Quasi duplicats visuals: `01/02`, `s_ocaso_cuartel/arenga`, `m3_fiesta_mansion/preguntas`, `g_titulo/00_titol`.

## 2. Taula de proves per E

Cap `ctest_final*.log` és posterior a `ctest_final6.log` (03/11 13:43, 292/292). Els comptatges de 293, 297 i 298 de commits posteriors **no tenen log guardat**. Només hi ha mutacions amb log per a `validar_opciones_beat` (`A0_mut.log`) i per a `validar_beats_auto` (`B1_mut.log`, `B2_mut.log`).

| E (commit) | Fitxer de prova | Casos | Què comprova | Mutació declarada | Evidència que fallés | Últim resultat conegut |
|---|---|---|---|---|---|---|
| E-GUION-01 `78827571` | `tests/Engine/GuionCapturaTests.cpp` | 15 | Parseig del guió, errors amb línia, tecles per fotograma (pulsa/suelta/toca), llista de captures, execució en curs | `toca` dura 2 fotogrames → 3 casos en vermell | **Sense evidència al log**; només el missatge de commit. El cas `toca_dura_exactamente_un_fotograma` és el que la fixa | `ctest_final5.log` 291/291 (conté `test_guion_captura`), `ctest_final6.log` 292/292 i `Passed` |
| E-GUION-02 `bbcb30f2` | `tests/Engine/GuardadosDeCapturaTests.cpp` | 3 | Els guardats `sectarios`, `ocaso` i `combate5` s'escriuen, es rellegeixen i es restauren amb mapa i flags esperats | Fabricar `sectarios` un pas tard → falla | **Sense evidència al log** (commit i informe) | `ctest_final6.log`: `test_guardados_de_captura Passed 1.43 s` |
| E-GUION-02b `d113c5c9` | mateix fitxer, +4 línies | (dins dels 3) | `restoreSaveGame` distingeix per id els objectes de la mateixa casella ((6,59), etc.) | Treure la cerca per id → rojo «se esperaba aigren_carro en (6,59)» | **Sense evidència al log** | `ctest_final6.log` 292/292 (13:43, mateixa hora que el commit) |
| E-MATRIZ-01 `f9d67d0f` + `db93f46b` | `tools/test_matriz_decisiones.py` (script, 25 `comprueba(`, 0 `def test_`) i `tests/Engine/MatrizDia1Tests.cpp` | 1 test ctest (script) + 8 casos | Inventari de 206 beats / 57 decisions estable; un guardat per camí del dia 1 restaura, juga l'inici de Gorran i compara flags, variables, beats | Per camí, còpia temporal del JSON sense l'efecte → ha de fallar (`sin_el_efecto_de_la_opcion_el_camino_falla`) | **Sense evidència al log**; `MATRIZ_DIA1.md:197-199` diu «verde / mutación en rojo» (només informe) | Commit diu 293/293; **cap log** |
| E-MATRIZ-01b `252f53ff` | `MatrizDia1Tests.cpp` (+55 línies; 3 casos d'or/moral/nivell/motxilla) | (dins dels 8) | Or, moral (valor i tram), nivell i motxilla per camí | `grantGold → log` i acte de moral canviat → falla | **Sense evidència al log** | Sense log |
| E-DIA1-CONSEC `8f7f1aef` | `MatrizDia1Tests.cpp` (+173) | (dins dels 8) | 26 files llegides pel jugador, 18 mutacions en vermell, estrat exclusiu per a cada combinació | 18 mutacions (`Mutacion.dia`) | **Sense evidència al log**; commit diu «293/293, validar_beats_auto 0» | Sense log |
| GORRAN-LIOS / GORRAN-FIX `3a8bda1a`, `653ad9ff` | cap fitxer de prova; demos `demo_boundington_alternativas.cpp` | 8 escenes | Els 4 lios de la aldea es juguen, cap «lio C» doble, Tinet sol, campana a la primera visita | M1-M4c «totes en vermell, restaurat en verd» (`GORRAN_LIOS.md:57`) | **Sense evidència al log** (només informe) | Commit diu 291/291; `ctest_final5.log` 291/291 és de fet del mateix període (13:43 o abans) |
| E-MATRIZ-02 A0 `2237a501` | `tools/test_validar_opciones_beat.py` | 5 | Cap beat amb >4 opcions (llegit del .h); repo real net (502 beats, 0 problemes) | Beat amb 5 opcions → error, amb 4 no | **Sí**: `A0_mut.log`: original `0 problemas` exit 0; mutat `bgd_candela (+1 opcion)` → «5 opciones, maximo 4», exit 1 | Només aquest log; cap ctest posterior |
| E-MATRIZ-02 A `2237a501` | `tools/test_matriz_decisiones.py` (amplia el de 01) | (mateix script) | Inventari del dia de Gorran amb el diari com a lector | — | — | Sense log |
| E-MATRIZ-02 B+C `b41a0790` | `tests/Engine/MatrizGorranTests.cpp` | 12 (6 de matriu + 5 de tregua/arnés + 1 de hallazgo) | 10 camins del dia de Gorran: flags, variables, beats, or, moral, nivell, motxilla, lectors del dia 3 i de l'Ocas | Efecte, cobrament, acte → en vermell | **Sense evidència al log** | Commit: 297/297; **cap log** |
| E-GORRAN-TREGUA `9628017a` | `MatrizGorranTests.cpp` (casos 7-12) | (dins dels 12) | Tregua a la subasta, traïció porta a la baralla, gemelles de 4 opcions, arnés no parla amb Gorran ocult | «Sense l'efecte de la traïció no hi ha baralla»; «si el pacte peleja la tregua no protegeix» | **Sense evidència al log** (`MATRIZ_GORRAN.md:208` només ho afirma) | Sense log |
| E-MATRIZ-03 A0 `94f565a0` | `test_matriz_decisiones.py` (+17) | — | Inventari del dia 3 | — | — | Sense log |
| E-MATRIZ-03 B+C `3414b0cd` | `tests/Engine/MatrizDia3Tests.cpp` | 11 | 10 camins del dia 3 (des de treva i baralla), visibilitat de Xila, aviso de Xila, arnés i fiesta; hallazgo Verina | Per camí: decisió, or, moral; `cinematicas.json`/`eventos` de HEAD → falla `xila_resaca` | **Sense evidència al log** (`MATRIZ_DIA3.md:202`) | Commit: 298/298, validar_todo 0 errors; **cap log** |
| E-B1/B2 (previ) | `tools/test_validar_beats_auto.py` | 32 | Beats auto sense `once` pegajosos i beats tapats | Sense `once` a `b3_dia_pasa`/`b3_campanas`; `seguir` tornat al lloc | **Sí**: `B1_mut.log` (`b3_dia_pasa` 1 pegajós → test falla; `b3_campanas` idem) i `B2_mut.log` (`b1_sectarios_seguir` tapat → 3 tests Failed; «RESTAURADO» 0 problemas) | `ctest_final6.log`: `test_validar_beats_auto Passed` |

Recompte de proves: 13 files d'E; **evidència al log de mutació en vermell: 2** (`validar_opciones_beat`, `validar_beats_auto`); **sense evidència al log: 11**. Resultat ctest guardat només fins al 292/292.

## 3. Resum crític

**Captures.** 44 PNG revisats: 27 ✅, 16 ⚠, 1 ❌ (llistats a la secció 1.7). Les que coincideixen són sobretot interfícies (títol, creador, menús de decisió, combat) on el text és el que es veia. Les que fallen són:

1. **Tres parelles de `m1_*` són el mateix PNG** (md5 idèntic). Les 6 captures d'`E-MATRIZ-01` no demostren cap diferència entre «unirse al culto» i «sembrar dudas». El doc ho admet, però en una frase i comptant sis captures. A més l'HUD diu **or 0 i «Día 1 · Noche»** i les captures no es van regenerar després que `252f53ff` donés or real: `MATRIZ_DIA1.md:217` («el oro es 0») contradiu `:250`.
2. **`s_ocaso_arenga` no mostra cap arenga i `s_ocaso_cuartel` és gairebé igual.** `s_ocaso_fundido_negro` és negre amb HUD. **Cap de les tres demostra les campanes de l'Ocaso**, que era l'objectiu de E-GUION-02. A més l'HUD diu «Día 4 · **Mañana**», no ocàs. Els noms dels fitxers no segueixen els `captura` dels guions (`campanas_1..5`).
3. **`MATRIZ_GORRAN.md` descriu captures inexistents.** `m2_tregua_sin_dialogo.png` i `m2_tregua_atraviesa.png` van ser esborrats a `9628017a`; la taula de la Fase C continua dient que Gorran està ocult i que no hi ha diàleg, el contrari del que mostren `m2_tregua_menu/pactos/cerrada` (que no hi consten).
4. **`s_sectarios_seguimiento` no mostra la persecució** (només una narració), i **`s_combate5_dialogo`** és una descripció del lloc, no un diàleg. **`m3_fiesta_preguntas`** és gairebé idèntica a `m3_fiesta_mansion`, de manera que «de vuelta a la sala tras 'Prefiero hacerle preguntas'» no es pot veure.
5. `s_combate5_mesa`: el recompte és correcte (5 enemics + heroi = 6 plaques), però **els devots tenen una fletxa vermella com a retrat** (placeholder), a diferència dels bandits de `m2_subasta_pelea`. No es pot dir que «es veuen tots els enemics» sense matisar-ho.
6. **Jugador vestit o cos base:** `00-05` es veuen amb cos base (reconegut al doc per a `--nivel` sense creador, i corregit amb `g_nivel_*`). Totes les `g_`, `s_`, `m1-m3` són vestides. La partida «normal» pel creador dona un **Aarakocra** (primera raça), no un humà com insinua el contracte.
7. **HUD:** l'or, el dia i el nom del mapa només es poden comprovar a les captures sense diàleg; a `m2_*` i `m3_informe_*` el HUD els amaga, i la frase «oro real» dels guardats no es verifica per imatge allà. A `s_sectarios_plaza` (125), `m3_fiesta_*` (120) i `s_ocaso_*` (280) sí que es veu, i és coherent.
8. **`CAPTURAS.md` està desactualitzat**: només documenta 00-05 i encara diu que «el joc no té guió de captura» i que no es pot capturar el combat de 5. No cobreix les 38 captures posteriors.

**Proves.** De 13 files/E només 2 tenen log de mutació en vermell (A0 i B1/B2). Tot el nucli de `E-GUION-*` i `E-MATRIZ-*` (50 casos C++ i els scripts Python) depèn de missatges de commit i d'informes. L'últim `ctest` guardat és 292/292 (`ctest_final6.log`); el 293/297/298 dels commits no té log.

**Afirmacions exagerades o errònies.**
- «Es veu la persecució / les campanes / l'arenga» (s_sectarios_seguimiento, s_ocaso_*): no.
- «Les captures m1 mostren el comenzament de cada camí»: són idèntiques.
- «Fase C de MATRIZ_GORRAN revisada una a una amb la vista»: descriu dos PNG que ja no existeixen.
- «Mutacions en vermell» sense log de ctest ni de mutació per a MATRIZ-01/02/03, GUION-01/02.

**Què caldria repetir.**
1. Ocaso: guardat + guió amb captures a cada fotograma després de `comienza_ocaso`, i triar la de la campana i la de l'arenga amb text visible. Si és de dia, dir-ho.
2. `m1_`: regenerar amb els guardats actuals i fer que el guió esperi més (o triar fotogrames posteriors a la fosa) per veure la diferència de camí, o suprimir una de les dues sèries.
3. Reescriure la Fase C de `MATRIZ_GORRAN.md` amb `m2_tregua_menu/pactos/cerrada`; completar `m2_tregua_cerrada` amb l'última pàgina.
4. `m3_fiesta_preguntas`: capturar el diàleg del noble després d'escollir «Prefiero hacerle preguntas».
5. Fer córrer `ctest -R "guion|guardados|matriz"` i guardar el log, i repetir les 11 mutacions sense log capturant la sortida en vermell (`*_mut.log` com A0 i B1/B2).
6. Actualitzar `CAPTURAS.md` per a les 38 captures noves i registrar la correspondència guió → PNG.
