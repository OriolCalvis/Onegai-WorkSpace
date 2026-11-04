# Estudi visual del creador de personatges

26/10/2026 · Codex · Integració recent `codex/revision-reciente-20261026`, base de producte `cbc2503`.

## Resultat

Hi ha problemes reals de coherència visual. Els PNG nous existeixen i el muntador els carrega, però això no garanteix que un tret es vegi bé, sigui anatòmicament coherent o es pugui modificar des del creador. La família pròpia dels minotaures falta; les ales i cues afegides són petites i sovint tapades de cara; els trets naturals de cada raça segueixen un circuit diferent dels complements del creador.

No s'ha reproduït la desaparició total generalitzada d'ales o cues en la còpia auditada. Sí que s'ha reproduït una opció de banyes invisible en les quatre orientacions quietes. Si al joc obert no apareix cap canvi en posar qualsevol ala, cal reproduir també aquell estat concret de la interfície: aquesta auditoria mesura el muntador real i les opcions del controlador, no una gravació de la finestra de l'usuari.

## Abast i mètode

- 72 races; aparença de raça de partida i 15 complements: 5 ales, 5 cues i 5 banyes.
- 1.152 muntatges C++ reals amb `Game::MontajePersonajes`, sense reimplementar la composició en Python.
- 4.608 mostres de les quatre orientacions quietes (frames 25, 29, 33 i 37). No són una auditoria de cadascun dels 40 frames animats.
- 975 combinacions de complement són ofertades pel controlador; les 105 restants es forcen només per estudiar les capes. No es compten com a opcions accessibles.
- 0 errors de càrrega en aquests muntatges. No s'han provat totes les combinacions de roba, pell i complements simultanis.
- `MapaDeCapas` identifica els píxels que queden al damunt. Les proporcions es comparen amb el canal alfa del PNG original; no són una mesura de bellesa ni de contrast perceptiu.
- Les làmines són sortides del muntador del joc ampliades sense suavitzar; Python només retalla, amplia i etiqueta.
- Sense canvis de producte: estudi, instrumentació temporal i documentació. Les proves verdes de la sessió anterior verificaven càrrega i regles, no aquesta qualitat visual.

## 1. Minotaures: falta la família pròpia i s'aplica el substitut d'orco — prioritat alta

`tools/pixellab_cuerpo.py` declara una família `minotauro`, i `pixellab_razas.familia_de()` l'assigna als dos minotaures i als aeronautes d'Esmua. Però `tools/pixellab/familias/minotauro.json` no existeix. `gen_personajes.talla_de_raza()` substitueix aquesta família per `grande`, que correspon a `orco`.

El manifest generat confirma `talla=grande` i `tallaRopa=grande` per als dos minotaures. El cos, la roba i els complements nous reben les regles de la família substituta, mentre morro, banyes i altres fragments provenen de les capes racials. A les làmines es veuen fragments superposats, apèndixs allargats i siluetes poc coherents entre orientacions. La fila «només cos» permet separar el problema de base del problema de vestir.

Afectació de la ruta de substitució: `minotauros_de_la_llanura`, `minotauros_del_laberinto` i `aeronautas_de_esmua`. El símptoma visual dels aeronautes queda pendent de revisió ampliada pròpia.

**Acció proposada:** disposar d'una família de minotaure aprovada, amb esquelets i quatre orientacions, i regenerar les seves peces. Si s'ha de mantenir temporalment l'orco, cal adaptar explícitament els trets i la roba al substitut i declarar-ho; no donar per resolta l'anatomia perquè els fitxers carreguin.

## 2. Ales i cues: la vista inicial amaga part del que s'acaba de seleccionar — prioritat alta d'experiència

`pintaVista()` fixa la vista quieta SW per defecte, i només gira amb el control «Girar». No mostra la batuda de les ales ni el moviment de la cua mentre es personalitzen.

Les ales tenen ordre 6 davant del personatge i 56 d'esquena; el cos té ordre 10 i la roba de tors 40. Les cues tenen ordre 5 davant i 45 d'esquena. Per tant, de cara es dibuixen al darrere del cos: és una oclusió prevista, però la mida i la silueta actuals donen poc retorn visual.

Exemples al frame quiet SW:

| Cas | Píxels visibles del complement | Píxels del dibuix original | Proporció visible |
|---|---:|---:|---:|
| Humà, ales de ratpenat | 95 | 159 | 59,7 % |
| Minotaure de la Plana, ales de ratpenat | 78 | 215 | 36,3 % |
| Minotaure de la Plana, ales de libèl·lula | 98 | 368 | 26,6 % |
| Humà, cua de llop | 61 | 83 | 73,5 % |

La cua de llangardaix del nigromant orc deixa 21 píxels visibles en aquesta vista. Una peça pot existir i quedar pràcticament reduïda a un detall de pocs píxels al costat del cos. En les 340 opcions d'ales i 275 de cua ofertades no s'ha trobat visibilitat zero en SW amb la roba inicial de raça.

**Acció proposada:** mostrar simultàniament davant i darrere, o orientar temporalment la previsualització per veure el tret seleccionat; oferir animació i comparar sense/amb. Revisar escala i silueta per família. No posar globalment les ales per damunt del tors: taparia el personatge i el problema tornaria en altres angles.

## 3. Trets de naixement i complements del creador no es poden editar igual — prioritat alta

`EditorApariencia::rehacerFilas()` amaga ALAS o COLA si el nom de la ranura ja figura a `raza.capas`. Afecta 4 races amb ales naturals i 17 amb cua natural. No hi ha una opció de substitució ni «sense» per a aquest tret natural.

En canvi, les banyes naturals acostumen a viure dins de `rasgos`, no dins d'una capa anomenada `cuernos`. Això permet oferir banyes noves sobre un minotaure que ja en porta: s'afegeixen, no substitueixen les originals. El mateix concepte anatòmic està modelat de dues maneres.

**Acció proposada:** separar els trets naturals editables per tipus (ales, cua, banyes, orelles, etc.), amb una política explícita de conservar, substituir o treure. Cal validar amb Oriol quins canvis respecten la identitat de cada raça abans d'ampliar opcions.

## 4. Opcions escollibles que la roba tapa completament — prioritat mitjana

Cas reproduït: `race_dwarf_deepforge` amb `cuernos_diablo`. El creador ofereix les banyes, però el seu `bacinete` inicial, dibuixat a ordre 70, tapa els 8 píxels de la peça d'ordre 66. Resultat: 0 píxels visibles a NE, SE, SW i NW. Les banyes de carner del mateix nan deixen només 1 píxel visible de cara.

A més, l'aparença inicial copia l'equip de raça encara que després els filtres del creador no ofereixin aquell casc com a roba inicial. No s'ha d'interpretar com un error de càrrega ni de selecció.

**Acció proposada:** indicar «ocult pel casc», poder ocultar visualment el casc durant l'edició i revisar la roba inicial respecte de les opcions que realment es permeten escollir.

## 5. El canvi de pell només tenyeix la capa `cuerpo` — prioritat mitjana

`MontajePersonajes::capas()` aplica el mapa de pell només quan `capa == cuerpo`. Les capes `rasgos`, `morro`, `cola` i `alas` es conserven. Per materials diferenciats (banya, membrana, ploma) pot ser correcte; per fragments de pell de la mateixa anatomia pot deixar dos colors incongruents.

La làmina dels minotaures compara la pell canviada amb els trets originals. La prova automatitzada actual fins i tot espera que els trets no canviïn, de manera que no detecta aquesta possible incoherència visual.

**Acció proposada:** etiquetar els materials anatòmics que han de seguir la pell i els que han de conservar color propi. No tenyir indiscriminadament tota la imatge.

## 6. Regles de dibuix genèriques amb poca adaptació a la perspectiva — prioritat mitjana

`pixellab_extras.alas_frame()` dibuixa dues ales de la mateixa extensió al voltant dels muscles; la mida depèn de l'amplada del cap. La cua parteix dels malucs, amb longitud també derivada del cap. Les formes no tenen una silueta específica per raça i no separen ala propera/llunyana com a peces amb oclusió independent.

Les comparatives mostren que «tenir set talles» no garanteix una anatomia coherent a les quatre diagonals. Els espectres, nagues i aarakocres també reben aquests complements genèrics.

**Acció proposada:** punts d'ancoratge i silueta per família, amb perspectiva, escala corporal i oclusió de costat proper/llunyà. Les ales naturals i les afegides haurien de compartir el mateix contracte.

## Ordre recomanat de reparació

1. Reproduir a la finestra exacta qualsevol cas on seleccionar una ala no canviï absolutament res; registrar raça, complement, roba i orientació. En el muntador auditat les ales carreguen i són visibles.
2. Resoldre el substitut dels minotaures i revisar separació de trets; és la base de roba i complements.
3. Unificar trets naturals i personalització, especialment substitució de banyes/cues/ales.
4. Millorar la previsualització: davant/darrere, animació i avís d'oclusió.
5. Ajustar silueta i ancoratges per família; després polir materials i colors.

## Criteris de verificació

- Les opcions visibles del creador han de tenir recursos, muntar-se i produir un canvi visible o un avís explícit d'oclusió.
- Comparació de les quatre diagonals i dels 40 frames, sense roba, amb roba i amb casc/capa; comprovació específica de fragments desconnectats i unions al cos.
- Casos mínims: dos minotaures, aeronauta, humà, nan de les Fragues, aarakocra, naga, espectre i dracònid.
- Les banyes/cues/ales naturals no s'han de duplicar en substituir-les. Canviar pell ha de respectar els materials definits.
- Afegir proves de visibilitat i d'interacció amb roba. Mantenir proves de càrrega, però no usar-les com a substitut de la revisió visual.

## Evidència adjunta

- `ales-cues.png`: quatre orientacions, aspecte base, ales i cua; la fila del minotaure mostra les ales.
- `minotaures.png`: vestits, sense roba, només cos i pell canviada, per als dos minotaures.
- `comparativa.png`: comparació exploratòria de set races. La columna de cua d'una raça que ja en té inclou una cua forçada per diagnòstic, no una opció que el creador ofereixi.
- `medidas.tsv` i `visibilidad.tsv`: matriu completa i proporcions. Els trets forçats sobre una raça que ja en té poden comptar també la capa natural; s'exclouen dels percentatges d'opcions ofertades.
- `auditar.cpp`: instrumentació C++ temporal d'exemples; la matriu completa es va obtenir amb el mateix recorregut estès a les 72 races i als 15 complements.

Rutes de codi referides a `MotorGraphico-main/MotorGraphico/` de la branca auditada: `src/Game/Personajes/MontajePersonajes.cpp`, `src/Game/Personajes/EditorApariencia.cpp`, `src/HUD/Pergamino/PaginasCreacion.cpp`, `tools/pixellab_cuerpo.py`, `tools/pixellab_razas.py`, `tools/gen_personajes.py`, `tools/pixellab_extras.py` i `assets/textures/personajes.json`.
