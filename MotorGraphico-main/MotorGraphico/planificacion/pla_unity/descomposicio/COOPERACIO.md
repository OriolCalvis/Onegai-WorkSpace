# Cooperació guiada i qualitat comprovable

## Un contracte abans de diverses implementacions

El coordinador comprova el codi una vegada, defineix els tipus compartits i
els publica amb versió i hash. Cada agent rep només el contracte, els fragments
que consumeix, els fitxers autoritzats i els casos d’acceptació. Un pressupost
orientatiu és 600–1.200 paraules de context tècnic específic; les normes
obligatòries de repositori són addicionals. No és un límit que justifiqui ometre
un contracte de propietat o una dependència real.

Es registra **qui congela la interfície**, **qui implementa**, **qui revisa** i
**qui integra**. Poden ser la mateixa persona en una tasca petita; el que no pot
faltar és la responsabilitat. Aquesta proposta no ha llançat agents executors.

## Estats i criteris de pas

| Estat | Condició observable |
|---|---|
| `definida_pendent_contracte` | Intenció, entrades, retorn i acceptació escrits; encara hi pot haver API proposada. |
| `preparada` | Pla aprovat, contracte v1 exacte, fitxers assignats, base vàlida i prerequisits acceptats. |
| `en_curs` | Fila reclamada i una sola persona escriu la zona. |
| `produida` | Funció i prova local revisables, amb diff acotat i ordre reproduïble. |
| `pendent_integracio` | Peça produïda però encara no la consumeix el recorregut real. |
| `integrada` | El consumidor de producte la crida i passa la prova del seu escenari. |
| `verificada` | Commit exacte validat en clon net; prova visual/audible/jugable feta quan toca. |
| `acceptada` | Resultat i evidència contrastats amb el contracte; paquet pare pot avançar. |
| `bloquejada` | S’indica dependència, prova del bloqueig i mínim canvi necessari. |

«Prova local verda» no passa directament a «paquet acabat». El paquet només es
tanca quan l’assoleix `juego`, `motor_editor` o `level_editor` mentre aquest
executable continuï sent oficial, i quan passa la porta de resultat corresponent.

## Paral·lelisme sense contenció

Les dependències de paquet del ZIP es conserven. Les de cada E són explícites
a les fitxes. Per defecte es manté ordre conservador dins d’un paquet; les
branques pures de percepció, ruta i temps d’U1-03 només es poden paral·lelitzar
després de congelar els DTOs i assignar zones disjuntes.

- Un responsable per fitxer compartit i sessió. `Application`, `GameSession`,
  el host de l’editor i CMake són carrils d’integració reservats.
- Dues E sobre el mateix mòdul poden passar consecutivament al mateix agent,
  conservant el context. Reiniciar un agent per cada funció pot gastar més que
  donar-li una D formada per dues E precises.
- No fer un fitxer per funció per crear un paral·lelisme artificial. Agrupar
  funcions cohesionades en mòduls petits, respectant 500 línies i 15 membres.
- Les peces ja separades amb contractes estables sí que són carrils diferents.
  Els canvis de contracte passen pel coordinador i invaliden les dependents;
  cap agent amplia la seva API unilateralment.
- Una font GL-free va en una biblioteca GL-free. El consumidor i la prova
  enllacen aquesta biblioteca; no compilen `src/*.cpp` sueltos dins del test.
- `include(cmake/X.cmake)` i `X.cmake` viatgen junts. No s’amaga integració com
  una «última línia» que ningú té encarregada.

L’ordre del primer lot és identitat → magatzem → percepció/ruta/temps → contacte
de combat → integració real. Un canvi sobre pools no es barreja amb una regla
nova de persecució en el mateix relleu.

## Quatre portes per a cada resultat

1. **Contracte:** exemples entrada-sortida, unitats, errors, propietat,
   efectes, límits i consumidor concret. Cap API proposada es tracta com a existent.
2. **Peça:** compilació al target corresponent; proves positives, negatives i
   de frontera que detectin una equivocació plausible. La regla de producte
   viu a `src/include`, no dins de la prova.
3. **Integració:** escenari C i recorregut B reals. Compatibilitat del desat,
   punts segurs, cancel·lació, input i canvi de mapa quan la funció hi intervé.
4. **Lliurament:** commit concret en clon net, validació de contingut i línia
   base, GL-free i smoke visual/sonor quan correspon; logs i hashes. Un check
   no executat continua pendent, no es converteix en verd.

La regressió es compara amb una base immutable del mateix entorn. Les
signatures i multiplicitats identifiquen fallades: mateix nombre d’avisos no
vol dir mateix conjunt. Els comptadors sumaris utilitzen l’última xifra només
quan la semàntica de la sortida ho confirma. `depurar` prioritza errors nous.

`fabrica` encara s’ha de revisar i validar al repo real. Les seves ordres de
clonatge, regex, variables de `cwd`, preset Windows i lliurament són objectes
de [PREP](paquets/PREP.md), no garanties adoptades per llegir un ZIP.

## Temps i millora del tall

Les caixes E són provisionals. Les cinc primeres E mesuren treball, revisió,
integració, espera i context requerit per separat. Si una E necessita més
de 90 minuts o tres arxius compartits, es revisa el contracte i es descompon
abans de continuar. Si una revisió detecta una incompatibilitat, s’afegeix
una E amb ID nou; no es reescriu la història d’una tasca acceptada.

Els 417 h identificats no són «el cost nou del Pla U». Cada fase S queda sense
durada compromesa fins a elaborar les seves incerteses. El pressupost i el
calendari només es recalculen amb els resultats del pilot i capacitat real.

Els problemes de qualitat tenen responsable i sortida: regressió → tornar a
la peça; contracte incomplet → tornar a preparació; consumidor absent →
integració oberta; recurs extern absent → acceptació pendent; discrepància de
producte → decisió d’Oriol. No es declara perfecció perquè tots els tests d’una
peça aïllada passin.
