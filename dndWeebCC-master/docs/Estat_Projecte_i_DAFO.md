# Estat del projecte ONEGAI — objectius assolits, pendents i DAFO

*dndWeebCC — Sistema de Cartes i Tiers · revisió del 19 de juliol de 2026*

Aquest document contrasta el que diuen els documents de disseny (`Sistema_Cartas_Tiers.md`,
`Plan_Cierre_y_Revision_Diseno.md`, `Mapa_Mundi_Requisitos.md`, `Ventanas_y_Utilidades.md`)
amb l'estat **real** del codi i de les dades, comptats directament del repositori. No és una
llista de desitjos: cada punt assolit té una evidència verificable, i cada pendent indica per
què encara no ho és.

---

## 1. Fotografia del projecte en xifres

| Mètrica | Valor |
|---|---:|
| Classes Java | 115 |
| Plantilles Thymeleaf | 76 |
| Controladors | 25 |
| Serveis | 21 |
| Formularis de carta (`web/form`) | 13 |
| Fulls CSS | 11 (~3.000 línies) |
| Mòduls JS | 7 (~860 línies) |
| Fitxers de test | 3 (~340 línies) |
| Cartes de catàleg en disc | 2.017 |
| Històries | 300 |
| Taules de botí | 200 |
| PNJs | 133 |
| Aventures | 52 |
| Personatges | 5 |
| Esdeveniments del món | **0** |

Detall del catàleg de cartes: habilitats 458, enemics 430, armes/equip 400, hechizos 201,
invocacions 200, clases 57, pasivas 57, monturas 50, trampes 50, races 38, consumibles 31,
condicions 15, trasfons 12, deïtats 10, dotes 6, rasgos 2.

---

## 2. Objectius assolits

### 2.1 Nucli del sistema de cartes ✅

- **13 tipus de carta amb CRUD complet** (model → form → repositori → servei → controlador →
  llistat/fitxa/formulari): clases, races, trasfons, habilitats, equip, dotes, pasivas,
  hechizos, consumibles, rasgos, invocacions, deïtats i condicions. Això tanca sencer el
  **Bloc C** del pla de tancament (ítems 14-19 + 19b), que era la bretxa més gran entre el
  GDD i el codi.
- **Sistema de piles (`recovery`)** al model i als formularis d'habilitat, substituint el cost
  per punts de l'edició 1 (Bloc B, ítems 10, 11 i 13). `Cost` queda com a llegat marcat.
- **Plantilla universal de 5 zones** aplicada de manera sistemàtica a tots els catàlegs.
- **Càlcul de vida** (`CalculadoraVida`) amb previsió des de la fitxa de classe.

### 2.2 Personatges ✅

- CRUD complet amb fitxa, exportació **PDF** i **JSON** (`PdfService`).
- **Sis validacions reals de servidor** que bloquegen el desat: slots d'equip, compatibilitat
  equip↔classe, límit de mà per tier, tier màxim de cartes, límit de dotes i límit de cartes
  divines.
- Metàfora de "taula" amb silueta d'equipament — un patró que, com apunta la revisió de
  disseny, no té equivalent directe a D&D Beyond ni Foundry.

### 2.3 Món narratiu ✅

- **Mapa Mundi editable en viu**: 19 nacions poligonals, 16 faccions jugables, 95 ciutats i
  26 zones, amb arrossegament de vèrtexs, creació d'elements, esborrany a `localStorage` i
  desat a `data/mapa/geografia.json`.
- **Llegenda 4a completa** (doc §5.2): cercador global, capes tipus GIS
  (nacions/faccions/ciutats/zones/punts/esdeveniments), llista de nacions amb "…N més" i
  **mode foc** que atenua la resta i obre un panell resum agrupat.
- **Panells modulars superposats**: rail d'icones sobre el llenç i panells acoblats
  esquerra/dreta (`paneles-overlay.js` + `mapa-paneles.js`), amb canvi de costat, plegat al
  rail i persistència. La toolbar està reorganitzada segons el doc §6.8 amb el menú
  `➕ Nuevo ▾`. La fitxa de nació/ciutat ja no bloqueja el mapa amb teló de fons.
- **Catàleg narratiu enllaçat de forma bidireccional**: història ↔ antagonista ↔ botí ↔ PNJ
  amb secret ↔ mes de trasfons.

### 2.4 Creació de contingut des de la web ✅ *(iteració recent)*

- **Crear PNJ** (`/npcs/crear`) i fitxa de mesa del GM amb agenda, palanca i secret marcat en
  groc. Abans els PNJs només naixien del script de generació en lot.
- **Crear història** (`/historias/crear`) amb antagonista i recompensa vinculats al catàleg
  real — resol l'asimetria detectada al wireframe (turn 12).
- **Històries multi-escena**: el model `Historia` accepta `escenas[]` (lloc, moment,
  repartiment, accions→conseqüències) i la fitxa les pinta com a línia de temps (turn 13a).
- **Wizard d'aventura de 4 passos** amb validació per pas, cercador de faccions i **vista
  prèvia editable** de cada tirada 🎲 amb re-tirar per element (turns 11a/11b/11c).
- **Cartes físiques derivades de l'esquema real** de cada tipus, no d'una plantilla única
  copiada de Magic (turns 6, 7 i 8), incloent-hi els casos que trenquen el motlle a
  propòsit: trasfons sense gema de poder i acció legendària com a text pla amb avís.

### 2.5 Interfície i eines transversals ✅

- **Menú hamburguesa mòbil** amb drawer (Bloc K, ítem 43) — la sidebar ja no desapareix.
- **Classes d'utilitat** `u-page-header`, `u-actions--wrap`, etc. reemplaçant els estils en
  línia repetits (Bloc K, ítem 44): 41 plantilles ja les fan servir.
- **Pantalla de diagnòstic** amb integritat de referències, mètriques HTTP i sessions.
- **Tests automatitzats** (`CalculadoraVidaTest`, `PersonatgeServiceTest`, context de Spring):
  el pla els comptava a zero; ara n'hi ha ~340 línies cobrint fórmules i validacions.
- **13 scripts generadors** que han poblat el món sencer sense escriure JSON a mà.

---

## 3. Objectius pendents

### 3.1 Prioritat alta

| # | Pendent | Per què importa |
|---|---|---|
| ~~P1~~ | ~~**Control de versions**~~ ✅ **Fet**: repositori Git amb historial, etiqueta `v0.1.0`, guia de treball i script de desat. | Era el risc més gran del projecte. |
| ~~P2~~ | ~~**Modularitzar `mapa/index.html`**~~ ✅ **Fet**: 2.073 línies de JS inline repartides en 13 mòduls a `static/js/mapa/`, amb l'estat compartit a `window.GMMapa`. La plantilla ha passat de 2.589 a 532 línies. | Desbloqueja la línia temporal, el zoom per nació i els modals per entitat. |
| P3 | **Motor de combat** (Bloc D, ítems 21-26): tirades de pool de d6, avantatge/desavantatge, torns, iniciativa, condicions en joc, mort i agonia. | Mitja secció 7 del GDD només és jugable en paper. La Zona 4 de la fitxa surt sempre a zero perquè no existeix estat de sessió. |
| P4 | **Contingut d'edició 2** (Bloc A, ítems 1-9): regenerar classes, races, habilitats, objectes i personatges d'exemple sota les regles vigents. | El propi GDD marca les seccions 14-18 com a no vàlides. |
| P5 | **Catàlegs prims**: dotes (6), rasgos (2), trasfons (12), condicions (15). | Contrasten amb 458 habilitats i 430 enemics: el desequilibri es notarà a la taula. |
| P6 | **Zero esdeveniments del món** malgrat tenir CRUD i cronologia funcionals. | Tota la maquinària de cronologia i línia temporal no té res a mostrar. |

### 3.2 Prioritat mitjana

| # | Pendent | Estat actual |
|---|---|---|
| P7 | Piles en viu durant una partida (Bloc B, ítem 12). | El camp `recovery` existeix; l'estat de sessió no. |
| P8 | Connectar invocacions i condicions actives a `Personatge` (Bloc C, ítem 18). | Les cartes existeixen; `Personatge` no té camp que les referenciï. |
| P9 | Lògica de substitució d'`evolvesInto` en pujar de tier (ítem 20). | El camp es pot omplir, però ningú l'aplica. |
| P10 | Multiclasse (Bloc F): `claseSecundariaId` i límit de mà reduït. | `Personatge` només admet `claseId`. |
| P11 | Símbols d'afinitat (✦☠✝♞⚙) i validació real de `requiredTags`/`incompatibleTags` (Bloc E). | Avui són text decoratiu sense lògica. |
| P12 | Línia temporal del món i zoom per nació (doc mapa §7.1 i §5.5). | Documentats amb decisions ja preses; sense implementar. |
| P13 | Els 5 modals per entitat en fitxers propis (criteri d'acceptació del mapa). | Hi ha un `#gm-dialogo` genèric funcional, no modularitzat. |
| P14 | Il·lustracions de carta (Bloc J): la zona ② segueix sent un emoji. | És la bretxa més gran contra qualsevol producte de cartes real. |

### 3.3 Prioritat baixa / diferits

- Bestiari i mazo d'aventura com a jerarquia Java (Blocs G i H) — avui funcionen bé com a
  JSON de només lectura.
- Migració de JSON pla a base de dades embeguda (Bloc L, ítem 46).
- Exportació PDF de cartes de catàleg i de subconjunts (Bloc I, ítems 39-40).
- Guia de "primera partida" d'extrem a extrem (Bloc M, ítem 50).
- **Neteja**: `flotantes.js`, `mapa-flotantes.js` i `flotantes.css` han quedat orfes en disc
  després de substituir-los pel sistema de panells; ja no els referencia cap plantilla.

---

## 4. DAFO

### 🟩 Fortaleses (internes, positives)

1. **Volum de contingut real, no maquetes**: 2.017 cartes, 300 històries, 200 taules de botí,
   133 PNJs i un mapa amb 140 elements geogràfics. Molts projectes d'aquest tipus moren amb
   deu cartes d'exemple.
2. **Consistència estructural superior a la mitjana**: la plantilla de 5 zones aplicada a 13
   tipus de carta és més disciplinada que D&D Beyond, que fa un layout diferent per tipus.
3. **Cartes derivades de l'esquema real de cada tipus**, no d'una plantilla forçada: un
   enemic mostra vida/atacs, una habilitat mostra recuperació/abast, un trasfons no mostra
   poder perquè per disseny no en té.
4. **Validació de regles al servidor**, no només a la interfície: sis validacions bloquegen
   el desat d'un personatge il·legal.
5. **Arquitectura repetible**: el patró Model→Form→Repositori→Servei→Controlador→3 plantilles
   ha permès afegir 7 tipus de carta nous sense sorpreses.
6. **Independència tecnològica**: JS vanilla sense framework ni build step; res es trencarà
   perquè una dependència quedi obsoleta.
7. **Documentació de disseny excepcionalment honesta**: els docs marquen explícitament el que
   no funciona, inclosos els seus propis errors. És rar i molt valuós.

### 🟥 Debilitats (internes, negatives)

1. ~~**Sense control de versions.**~~ ✅ Resolt: repositori Git inicialitzat amb historial,
   etiqueta `v0.1.0` i guia de treball (`docs/Guia_Control_Versions.md`).
2. ~~**`mapa/index.html` és un monòlit.**~~ ✅ Resolt: 13 mòduls a `static/js/mapa/` amb
   espai de noms compartit. *Queda pendent* el segon pas del doc §6.7: separar els 5
   modals per entitat en fitxers propis a partir de `mapa-dialogo.js`.
3. **Cobertura de tests mínima**: 3 fitxers per a 115 classes. Els controladors, els
   repositoris i tot el mapa no tenen cap prova.
4. **Persistència en JSON pla sense transaccions**: cap protecció davant escriptura
   concurrent; cada consulta carrega el directori sencer a memòria.
5. **Desequilibri del catàleg**: 458 habilitats contra 6 dotes i 2 rasgos.
6. **La meitat del reglament no és executable**: sense motor de combat, la secció 7 del GDD
   viu només al paper.
7. **Zona d'il·lustració buida** en totes les cartes: emojis com a marcador de posició.
8. **Microtipografia per sota del llindar pràctic** (8-11px) en badges que comuniquen regles.
9. **Deute de contingut declarat**: les seccions 14-18 del GDD segueixen en edició 1.

### 🟦 Oportunitats (externes, positives)

1. **El sistema de panells overlay és reutilitzable** fora del mapa: la mateixa mecànica
   serveix per al constructor d'aventures o la fitxa de personatge en viu.
2. **La cronologia buida és una oportunitat neta**: el motor ja hi és; escriure 20
   esdeveniments històrics activaria de cop la línia temporal i donaria profunditat al món.
3. **Els scripts generadors poden tancar els catàlegs prims** en hores, no setmanes — el
   camí ja està obert per a habilitats i enemics.
4. **La IA generativa fa viable la zona ②**: el coll d'ampolla històric de qualsevol joc de
   cartes indie (l'art) avui és assolible fins i tot amb un primer pas d'art genèric per rol.
5. **Nínxol real**: cap eina comercial ofereix un sistema de tiers per cartes sense nivells;
   D&D Beyond i Foundry assumeixen les regles de 5e.
6. **La base de dades embeguda (H2/SQLite) és una migració acotada**: els repositoris ja
   aïllen l'accés a disc darrere d'una interfície pròpia.
7. **El mapa com a producte per si sol**: l'editor geogràfic en viu té valor fins i tot fora
   d'aquest sistema de regles.

### 🟧 Amenaces (externes, negatives)

1. **Pèrdua de dades irreversible**: sense Git i amb escriptura directa a JSON, un desat
   defectuós o un `rm` accidental s'emporta contingut de mesos. *És l'amenaça número u.*
2. **El monòlit del mapa acabarà bloquejant l'evolució**: cada funció nova encareix la
   següent, fins que afegir res sigui inassumible.
3. **Sense tests, cada millora pot trencar regles en silenci**: les validacions de personatge
   són fàcils de rompre sense que ningú se n'adoni fins a la taula de joc.
4. **Risc legal/de disseny per proximitat a 5e**: cal mantenir la distància deliberada que ja
   marca el GDD i no importar text propietari als generadors.
5. **Fatiga del creador**: és un projecte d'una sola persona amb 50 tasques obertes; sense
   fites petites i visibles, el risc d'abandonament és real.
6. **Deriva entre documentació i codi**: ja n'hi ha exemples (el pla diu "zero tests" quan
   ja n'hi ha tres). Si els docs deixen de ser fiables, es perd el millor actiu del projecte.
7. **Divergència de nomenclatura CON/DES/INT/**CAR** vs **SAB** del disseny original**: com
   més contingut es genera, més cara serà la migració si mai es decideix.

---

## 5. Ordre d'atac recomanat

| Fase | Accions | Per què primer |
|---|---|---|
| **0 — Blindatge (1 dia)** | `git init` + primer commit + `.gitignore` (`target/`, `logs/`). Còpia de seguretat de `data/`. | Elimina l'amenaça número u abans de tocar res més. |
| **1 — Deute estructural (1-2 setmanes)** | Partir el JS del mapa en `static/js/mapa/*.js` per responsabilitat; retirar els fitxers orfes `flotantes.*`. | Desbloqueja tota la feina futura del mapa. |
| **2 — Contingut viu (dies)** | Escriure 15-20 esdeveniments històrics; ampliar dotes i rasgos amb els generadors. | Guany visible immediat amb motor ja construït. |
| **3 — Jugabilitat (setmanes)** | Motor de tirades (P3) + estat de piles en viu (P7): primer les tirades pures, que són testejables de manera aïllada. | Converteix l'app d'un catàleg en una eina de partida. |
| **4 — Acabat (continu)** | Il·lustracions per rol, tipografia mínima a 13px, línia temporal i zoom per nació. | Impacte visual alt un cop l'estructura és sòlida. |

---

## 6. Conclusió

El projecte està **molt més avançat en catàleg i món que en joc**. La infraestructura de
cartes, personatges, històries, aventures i mapa és sòlida, coherent i inusualment ben
documentada; el que falta és, d'una banda, la capa que converteix tot això en una partida
resolta a l'aplicació (motor de tirades i estat de sessió) i, de l'altra, dues mesures
d'higiene tècnica —control de versions i modularització del mapa— que no afegeixen cap
funcionalitat però protegeixen tot el que ja existeix.

La recomanació més important d'aquest document no és una funcionalitat: és **inicialitzar Git
avui mateix**.
