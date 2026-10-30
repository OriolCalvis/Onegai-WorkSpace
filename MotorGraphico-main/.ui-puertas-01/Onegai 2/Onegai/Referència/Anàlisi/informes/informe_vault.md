# INFORME DEL VAULT DE OBSIDIAN — Onegai / Egaroth

**Objeto analizado:** `/Onegai 2/Onegai/OnegaiTimeLine/` — 133 archivos `.md` + 19 `.canvas`.
**Contrastado contra:** `Onegai_Cronologia_Maestra.md` (285 eventos), `TL_OCR_completo.md` (274 diapositivas) y los 95 documentos de `corpus_extraido/`.

> **Dos correcciones al encargo, comprobadas.**
> 1. El vault tiene **133** archivos `.md`, no 158. El recuento de 158 de la `GUIA_DEL_PROYECTO.md` está inflado.
> 2. **La raíz real del vault de Obsidian es `/Onegai 2/Onegai/`, no `OnegaiTimeLine/`** — la carpeta `.obsidian/` está en `Onegai/`. Fuera de `OnegaiTimeLine/` hay **seis notas sueltas y un canvas número 20** que el encargo no contemplaba y que sí importan (§4).

**Volumen real de texto:** 18.835 palabras en `OnegaiTimeLine/`, repartidas así:

| Carpeta | Palabras | % |
|---|---:|---:|
| `Campanyes/` | 23.860* | — |
| `TML/` | 8.343 | 44 % del lore |
| `Descripciones/Astrologia/` | 4.936 | duplicado (ver §2) |
| `Plantilles/` | 1.696 | plantillas vacías |

\* de las cuales **21.028 son solo de *Tren hacia el Himetsu***. La cifra de ~38.900 palabras del encargo cuadra con la suma total; lo que no cuadra es la distribución: **el 54 % del vault entero es una sola one-shot**, y el lore histórico propiamente dicho son 8.343 palabras.

---

## 1. INVENTARIO ESTRUCTURADO

### 1.1 `TML/Creadores/` — 2 fichas, ambas VACÍAS

| Archivo | Estado |
|---|---|
| `TML/Creadores/8865 b.f. - Envidia.md` | **plantilla sin rellenar** (`H - Fitxa D'intervenció divina`, con `{{miracle}}` literal) |
| `TML/Creadores/8954 b.f. - Guerra.md` | **plantilla sin rellenar** (`H - Fitxa Esdeveniment Històric`, con `{{esdeveniment}}` y `year: {{any}}`) |

**No contienen una sola línea de contenido.** Su único valor informativo es el *encuadre*: alguien decidió que Guerra interviene en 8954 b.f. y Envidia en 8865 b.f., y los conectó con una flecha en `Onegai/Sense títol.canvas` (Guerra → Envidia). Eso sí es un dato canónico nuevo (§2.6).

### 1.2 `TML/9100b.f. -9071b.f. - EDAD DE LOS DIOSES/` — 6 fichas, 5 con contenido

| Archivo | Palabras | Estado |
|---|---:|---|
| `00 - TML - Nacimiento de Egaroth y las Voluntades Creadoras.md` | 378 | ⭐ **contenido exclusivo del vault** |
| `01 - TML - CHRONOS.md` | 114 | ⭐ parcialmente exclusivo |
| `02 - TML - SOFIA.md` | 88 | ⭐ parcialmente exclusivo (nombre **Aegor**) |
| `03 - TML - VIDA Y MUERTE.md` | 342 | mezcla: 1er párrafo verbatim del Gran Compendio + ampliación exclusiva |
| `04 - TML - Nacimiento de Egos y Eros.md` | 79 | verbatim del Gran Compendio. **El título promete "Egos y Eros" pero el cuerpo solo contiene "EL DÍA DE LOS NOMBRES"** |
| `La gran migracion.md` | 31 | dos frases sueltas, prácticamente vacío |

⚠️ `03 - TML - VIDA Y MUERTE.md` enlaza a `[[05 - TML - CAMBIOS EN EGAROTH -]]`, **archivo que no existe**. El canvas, en cambio, enlaza 03 → 04. La cadena está rota en el texto y remendada en el canvas.

### 1.3 `TML/9071b.f. -8578 b.f. - ERA DEL DESPERTAR DE EGAROTH/` — 17 subcarpetas de raza

| Raza | Canvas (fecha del rótulo) | Fichas `EVENTOS/` | Estado del contenido |
|---|---|---:|---|
| **Elfos** | `9049 b.f. - La Fundació d'Itron i la Caiguda de Vincalp` | 9 | ⭐ **la carpeta mejor desarrollada del vault** (4.343 palabras). 7 con texto, 2 vacías |
| **Elementales** | `8845 b.f. - Las aldeas de los Elementales` | 11 | todas con texto, pero cortas (20–220 pal.). Copia del Gran Compendio |
| **Dracónidos** | `8735 b.f. - La Fundación del Congreso de Wulcain` | 7 | todas con texto (42–475 pal.). Copia de `Estructura narrativa del Gran Compendio` |
| **Hombres Pájaro** | `8786 b.f. - La Purga de las Tormentas` | 1 (`Contexto.md`, 207 pal.) | copia **truncada y con la fecha alterada** del Gran Compendio |
| Enanos | `8898 b.f. - Descubren la primera veta de mithril.` | 1 | **VACÍA** |
| Goblin | `8837 b.f - Primeras tribus goblins unifican Gliaddokx.` | 1 | **VACÍA** |
| Hombres Gato | `8790 b.f. - La Marcha de las Cuatro Estaciones` | 1 | **VACÍA** |
| Hombres Lagarto | `8742 b.f. - El Asentamiento… Península de Orun` | **0** | el canvas apunta a un `EVENTOS/Contexto.md` **que no existe** |
| Orcos | `8853 b.f. - Juramento de las Llamas Eternas` | 1 | **VACÍA** |
| Nagas | `8762 b.f. - La Rebelión de las Corrientes Oscuras` | 1 (`Context.md`) | **VACÍA**. Además el canvas es un fichero de **2 bytes** (`{}`), sin nodos |
| Sagas | `8739 b.f. - La Ascensión del Guardián del Abismo` | 1 | **VACÍA** |
| Yokai | `8729 b.f. - La Fundación de los Clanes Yokai` | 1 (en `EVENTOS 01/`) | **VACÍA** |
| Minotauro | `8739-8722 b.f. - La Fundación de Taurengrad` | 1 | **VACÍA** |
| Humanos de Aegroum | `CLAN ORH` (sin fecha) | 1 | **VACÍA** |
| Humanos de Ascaria | `CLAN YSLUR` (sin fecha) | 1 | **VACÍA** |
| Humanos de Bastrea | `CLAN BASTIAN y CLAN ILMAR` (sin fecha) | 1 | **VACÍA** |
| Humanos del Este | `8865 b.f. - El Gran Pacto` (sin fecha en carpeta) | 1 | **VACÍA** |

**Balance brutal: de 17 razas, solo 4 tienen texto.** Las otras 13 son un canvas con un único nodo que apunta a un `Contexto.md` de 0 bytes. La estructura está montada; el contenido, no.

### 1.4 `Campanyes/`

| Campaña | Archivos | Palabras | Estado |
|---|---:|---:|---|
| `OneShot/4592b.f. - Tren hacia el Himetsu/` | 45 | **21.028** | ⭐ **completa y jugable**, 5 días, 22 fichas de personaje, 8 puzles, 8 eventos, 5 enemigos |
| `OneShot/8495b.f. - Campanya de Itron/` | 4 | 1.400 | 1 esqueleto narrativo real (950 pal.) + **3 plantillas sin rellenar con nombres tecleados al azar** (`ghghf.md`, `ffghfh.md`, `Sense títol.md`) |
| `Canon/Aegroum/Campanya 1 Boundinghton/` | 21 | 1.432 | **esqueleto a medias**: 5 capítulos (2 con texto, 3 solo con encabezados), **13 de 21 archivos vacíos** |

### 1.5 `Descripciones/Astrologia/` — 1 archivo

`📖 Llibre de les Llunes d'Egaroth.md` (4.936 palabras). **Es un duplicado casi exacto** de `corpus_extraido/THE ONEGAI PROJECT__Plantillas__Personajes Hombrew__Astrologia.docx.md` (4.663 palabras). Mismo autor ficticio (*Ortheon d'Elghëmar, Astromag de la Torre Silenciosa*), misma tabla de compatibilidades, misma *Nit del Fil Diví*, mismo dios *Desgarro*. **No aporta nada nuevo.** ⚠️ Ojo: es **otro** documento que `Plantillas/Astrologia.docx` (2.232 pal.), que asigna deidades regentes distintas a cada mes — la contradicción astrológica está en el corpus, no en el vault.

### 1.6 `Plantilles/` — 12 plantillas, todas sin rellenar (correcto: son plantillas)

`C - Fitxa artefacte` · `C - Fitxa de Campanya` · `C - Fitxa lloc` · `C - Fitxa subquest` · `E - Fitxa esdeveniment` · `E - Fitxa puzzle` · `H - Fitxa Conflicte bèlic` · `H - Fitxa Conseqüències d'un fet històric` · `H - Fitxa D'intervenció divina` · `H - Fitxa Entitat divina` · `H - Fitxa Esdeveniment Històric` · `N - Fitxa d'NPC Enemic` · `N - Fitxa d'NPC important` · `N - Fitxa d'NPC General` · `N - Fitxa monstre`.

Sistema de tipos coherente y bien pensado (`C`=campaña, `E`=escena, `H`=historia, `N`=NPC), con frontmatter `type:` y `tags:`. Es infraestructura útil, no lore.

### 1.7 Fuera de `OnegaiTimeLine/` — la raíz del vault (`Onegai/`)

| Archivo | Contenido |
|---|---|
| `Onegai/Sense títol.canvas` | **el canvas nº 20**: conecta las dos fichas de `TML/Creadores/`. Ver §4.4 |
| `Onegai/Sense títol.md` | plantilla de NPC enemigo sin rellenar |
| `Onegai/Rin'mel.md` | contiene literalmente `hola, buenos día`. Basura |
| `Onegai/2025-12-23.md`, `Onegai/2026-02-27.md` | notas diarias vacías (0 bytes) |
| `Onegai/Dr Oren Lir "Punt de Sutura".md` | **0 bytes** — duplicado extraviado del NPC del Himetsu |
| `Onegai 2/Sense títol 2.canvas` | 2 bytes (`{}`), canvas vacío en la raíz del proyecto |

---

## 2. ⭐ MATERIAL QUE SOLO ESTÁ EN EL VAULT

Método: cada término y giro característico se cruzó contra `TL_OCR_completo.md` y contra los 95 ficheros de `corpus_extraido/`. Lo que sigue tiene **cero coincidencias** en ambos.

### 2.1 ⭐⭐⭐ El Pacto de las Almas — el texto íntegro
**`TML/…/Elfos/EVENTOS/El Pacte de les Ànimes - El Naixement dels Elfs.md`** — 2.502 palabras.

**Esta es la joya.** En `TL_OCR_completo.md` la diapositiva 236 contiene **únicamente el título**:

> `## [slide 0236]`
> `El Pacte de les Animes: El Naixement`
> `dels Elfs`

Nada más. Ni una línea de cuerpo. La maestra lo registra como `EV-SF-PACTEANIMES` con una definición de dos frases y **Fuentes: [TL slide 236]**. El vault tiene la narración completa, en catalán, y **contradice o corrige varios puntos de la entrada actual de la maestra**:

**(a) Vincalp NO muere: queda sellado.**
> «El sòl tremolà, l'aire s'esquinçà, i un crit sense veu escapà del seu cos mutat mentre era **arrossegat cap a les profunditats d'Itron, segellat en un calabós d'obsidiana i memòria**.»

El ritual es *«una cadena rúnica de lligam d'ànimes… El seu propòsit no era vèncer, sinó contenir»*, ejecutado por supervivientes Tierrakteros y Vienturkos con la sangre de los caídos. **El primer vampiro de Egaroth sigue vivo y encerrado bajo Itron.** Es el gancho de campaña más grande del proyecto y no está en ningún otro sitio.

**(b) El destino de cada subraza, que la maestra tiene mal.**
La maestra dice: *«los Aguakturos derivarán en los Nagas y Azhira Espiral de Marfil será su única superviviente»*. El texto del vault dice otra cosa:
- **Aguakturos**: su ejército es arrasado; Azhira huye herida a las orillas, salvada por dos compañeros que mueren (uno, **Nothuun**, se sacrifica abriendo una grieta con aire comprimido).
- **Chimenyorik**: extinción total. **Shalira Escorça Lluminosa** canaliza *«l'última cançó de foc»* y *«quan la llum s'apagà, el clan havia desaparegut per complet del camp de batalla»*.
- **Tierrakteros**: *«Només un grapat… fugiren en embarcacions precàries fins arribar a les costes d'Ashye. Allà… foren rebuts pels **Sagas**, que veieren en ells un instrument per al seu propi destí.»* ⭐ **Vínculo Tierrakteros → Sagas, inédito.**
- **Vienturkos**: *«destruïren la biblioteca de coneixement rúnic, cremaren documents, trencaren relíquies i esborraren qualsevol rastre de Vincalp»*; algunos desaparecen esa misma noche, *«segons alguns per suïcidi, segons d'altres, absorbits per l'eco de la criatura»*. ⭐ **Explica por qué no hay fuentes élficas sobre Vincalp: fue una damnatio memoriae deliberada.**

**(c) Las almas no transitan — el bloqueo que fuerza la intervención divina.**
> «Les ànimes dels caiguts, centenars, potser milers, no havien abandonat els seus cossos. No flotaven, no ascendien… Estaven allà, suspeses com si la mateixa realitat hagués oblidat com deixar-les marxar.»

**(d) El diálogo teológico Muerte / Vida / Guerra / Egos.** ~1.400 palabras de diálogo directo. Es **el único texto de todo el proyecto donde los Creadores hablan entre sí**. Caracteriza a las cuatro voluntades de forma incompatible con la lectura plana que da el Gran Compendio:
- **Muerte** es *«la més jove dels Primigenis»*, llora, huele a musgo húmedo, y es **la que defiende la redención**: *«És inversemblant! Que sigui jo, precisament jo, la que cregui en la redempció i les segones oportunitats!»*
- **Vida** es fría y utilitarista: *«Jo no dono vida perquè juguin a ser poetes de la tragèdia. La dono perquè s'utilitzi bé. I quan s'abusa, la retiro. Això no és crueltat; és responsabilitat envers el futur.»*
- **Guerra** llega riendo (*«HAHAHA!»*), aplaudiendo la masacre, y **es él quien propone crear a los elfos**: *«Què us semblaria si d'aquest desastre… en féssim néixer quelcom nou? … Una raça que neixi de la mort mateixa.»*
- **Egos** es convocado por Guerra y necesita permiso de las dos: *«Mort, jo crec en els nostres fills… Els donaré un vel nou perquè puguin caminar de nou.»*

**(e) El nacimiento de los elfos por canto.** Los tres entonan una melodía, los cuerpos se elevan y se funden en *«una amalgama impressionant, una dansa grotesca i alhora bellíssima de carn, ossos i sang»*, que explota en lluvia; cada gota al tocar el suelo forma un elfo. *«No els éssers dividits en elements, no les antigues subespècies; sinó una raça única, nova, sencera.»*

### 2.2 ⭐⭐ La prehistoria de Egaroth — los Deaps y el Hechicero
**`TML/9100b.f. -9071b.f. - EDAD DE LOS DIOSES/00 - TML - Nacimiento de Egaroth y las Voluntades Creadoras.md`**

Términos con **0 coincidencias** en OCR y corpus: `Deap`, `Gran Espacio Vacío`, `agujero negro`, `Hechicero` (en este sentido), `abeja`.

El Gran Compendio despacha el origen de Egaroth en una frase: *«surgieron de la nada unas voluntades… Tornándose un ser gigantesco que flota en medio del Vacío Eterno… cayó en un largo sueño»*. El vault da una cosmogonía completamente distinta y mucho más ambiciosa:

> «Egaroth pertenece a una especie que lleva viviendo en este universo desde casi el origen de la creación… Estos gigantescos seres eran conocidos como **Deaps**. Vivían en armonía flotando por el Gran Espacio Vacío… Estos seres se agrupaban alrededor de **puntos de energía** que flotaban, como ellos, en medio de la oscuridad, que les daban luz y calor.»

> «Pero algo ocurrió en uno de ellos, un **Hechicero**, sediento de poder y destrucción, creó una oscuridad que engulló a toda su población. Descubrió que habían más Deaps y quiso explorarlos y conquistarlos todos. Su camino llevó casi a la extinción a toda la población de Deaps… hasta que el destino se interpuso en su camino. **Perdió el control de su magia, lo que supuso la creación de un agujero negro que engullía toda esa magia corrupta.**»

> «Egaroth, que flotaba algo más alejado que los demás no fue detectado por el Hechicero… pero le conmocionó hasta tal punto que se sumió en un profundo sueño… los restos de la destrucción… Con el paso del tiempo se volvieron pequeños puntos de luz, que adornaron para siempre esa gran oscuridad.»

Tres consecuencias canónicas de golpe:
1. **Egaroth es una especie, no un individuo**, y tiene hermanos muertos.
2. **Las estrellas del cielo de Egaroth son los restos de los Deaps asesinados.**
3. **Existe un agujero negro** con magia corrompida dentro, y un genocida cósmico anterior a la Creación.
4. **El sueño de Egaroth no es voluntario: es un trauma.** *«se sumió en un profundo sueño»* por conmoción. Eso reescribe el sentido del Despertar Elemental de 2838/2738 b.f.

⚠️ El archivo contiene un `...` literal en mitad del texto (un párrafo elidido entre la llegada del "punto de energía" y el despertar por Chronos). **Falta material aquí.**

### 2.3 ⭐ Chronos nace de la unidad mínima de tiempo
**`01 - TML - CHRONOS.md`** — sin equivalente en corpus ni OCR:
> «En ese pequeño destello, que duró entre menos y aproximadamente, aún menos que la **molécula de segundo, que tarda una abeja en batir sus alas**, nacido Chronos… Es irónico pensar que el mismísimo creador del tiempo naciera de la menor unidad en la que el tiempo es capaz de ser mesurado.»

Además: **Chronos nace del *primer intento fallido* de Egaroth por despertar**, *«ya que las magulladuras que le habían dejado la explosión de corrupción aún pasaban factura»*. Conecta directamente con §2.2.

### 2.4 ⭐ Aegor — el nombre de la Voluntad Única de Vida y Muerte
**`02 - TML - SOFIA.md`**. `Aegor` tiene **0 coincidencias** en corpus, OCR y maestra.
> «Y de ahí nació **Aegor**. Este ser componía la vida y la muerte en un mismo ser y con él trajo a más seres inferiores a él por supuesto. Pero esos fueron **los primeros habitantes de Egaroth**.»

La maestra (`EV-SF-VIDAMUERTE`) habla de «Vida y Muerte, las Voluntades Gemelas» pero **no tiene nombre para el ser fusionado previo a la separación**. El vault lo bautiza. Además añade que Aegor trajo consigo a los primeros habitantes del mundo — dato ausente en el Compendio.

### 2.5 ⭐ La Campaña de Itron (8495 b.f.) — el frente norte de las Guerras del Ocaso
Nombres con 0 coincidencias fuera del vault: **K-2**, **Jotdik**, **Quezados**, **Noraga Pôken**.

Aporta el **orden de batalla de las Guerras del Ocaso**, que la maestra no tiene (`EV-8578-OCASO` solo dice «las alianzas del Norte y del Sur entran en guerra abierta»):
> «Posar en joc la **guerra "Hòmens Gat + Minotaures + Chimenyorik" vs "Orcs + Homes Pàjaro + Sagas + Vienturkos"**.»

⚠️ Nótese que ese bando incluye **Chimenyorik y Vienturkos como facciones vivas y enfrentadas en 8495 b.f.** — es decir, después del Pacto de las Almas las cuatro subrazas siguen existiendo como entidades políticas, no fueron absorbidas del todo por «los Elfos». Contradice la lectura de fusión total de §2.1.

Otros datos exclusivos:
- La maldición se llama aquí **«la maledicció per la desaparició del Noraga Pôken»** (variante ortográfica de *Nuragapóken*), y la causa que se revela es que **«El Noraga Pôken ha estat segellat»**, no simplemente corrompido. La maestra tiene `EV-8577-NURAGAPOKEN` (corrupción) y `EV-SF-PRIMERAMALDICION`; el vault añade el **sellado** como mecanismo.
- Efectos físicos de la maldición: *«La llum del sol s'apaga / El fred augmenta / La vida es marceix / Pèrdua d'esperança col·lectiva»*.
- **Los Sagas se retiran del frente por orden divina** (*«Els Sagas s'han retirat per ordre divina»*), y su retirada hunde el frente.
- **El Laberinto Minotauro de Vosmurg** contiene «las claves de la Edad Oscura», un **mercado interior vivo** con maestros y comerciantes, y las **pruebas de Sofía y Deitrog**.
- Existe **«una arma llegendària, i és múltiple»**.
- Sylphera Aergan aparece como PNJ aliado y su ira causa *«marees de màgia salvatge al cel»*.
- **La dona encadenada**, entidad de horror corporal sin identificar.

### 2.6 ⭐ El encuadre Guerra 8954 / Envidia 8865
Las dos fichas de `TML/Creadores/` están vacías, pero su **existencia y sus fechas** son un dato: alguien decidió que en **8954 b.f.** (Día de los Cuatro Soles) intervino **Guerra**, y en **8865 b.f.** (Gran Pacto / nacimiento de Mirathis) intervino **Envidia**. El canvas `Onegai/Sense títol.canvas` los encadena **Guerra → Envidia**, sugiriendo causalidad. La maestra atribuye 8865 a Envidia/Mirathis (coherente) pero **no atribuye 8954 a Guerra** en ningún sitio.

### 2.7 ⭐ Toda la one-shot del Himetsu (21.028 palabras, 45 fichas)
**Todos** los topónimos, facciones y personajes de esta campaña dan 0 coincidencias en corpus y OCR:
- **`Etriath`** (Ecla), **`Ugrotar`** (sierra), el **Puerto Subterráneo Udrax–Ecla**.
- El **Tren Abisal de la Alianza «Nereida de Ferro»**: convoy de vapor rúnico, túnel abisal sellado, 5 días bajo el océano, construido por **Bomengrid (Udrax) + Umedan/Etriath (Ecla) + Ection (Bastrea)**. 14 zonas descritas.
- La facción **«els Horitzons»**: célula que *«captura infraestructures i ven estabilitat com si fos or»*, fundada por **Kael Velombre "Clau Fosca"**, exjefe de rutas de Bastrea.
- 13 NPC, 4 PJ y 5 enemigos con ficha completa D&D 5e.
- Estética tricultural codificada: *«Udrax (robusto y visible) vs Ecla (ritual y sutil) vs Bastrea (papel y registro)»*.
- El **Vagó Santuari** que *«estabilitza el flux espiritual durant el trajecte»* y la superstición *«Himetsu no 'cura' gratis. Si vas, algo te pide.»*

⚠️ **Todo este material choca de frente con la cronología maestra** por la fecha (§3.2). Como *worldbuilding* es excelente; como *evento fechado* es insostenible.

### 2.8 Lo que el encargo sospechaba y NO es exclusivo del vault
Hay que decirlo con claridad para no duplicar trabajo:

| Sospecha | Veredicto |
|---|---|
| Crònica dels Elfs (Fundació d'Itron) | ❌ está íntegra en `TL_OCR_completo.md` diapositiva 228 |
| El Torneo de los Fuertes | ❌ está íntegro en OCR (diapositiva ~235), líneas 2073–2106 |
| Las cuatro subrazas (Aguakturos, Vienturkos, Tierrakteros, Chimenyorik) | ❌ están en OCR diapositivas 233-234, con los mismos nombres propios (Azhira, Taloen, Vincalp, Oraleia, Dorak Irmsol, Thirra, Kaelyn Brasa, Zharuk) |
| Fichas de Dracónidos (Zorvhan, Tharkos, Shayrith, Varyx, niveles de la ciudad-caldera) | ❌ copia de `Estructura narrativa del Gran Compendio.docx` y OCR |
| Fichas de Elementales (Shorm, Aequor, Terrus, Zephyrus, los cinco hijos) | ❌ copia de `Gran Compendio de los Creadores.docx` |
| `Hombres Pajaro/Contexto.md` | ❌ copia **verbatim y truncada** del Gran Compendio, con la fecha cambiada |
| `Llibre de les Llunes d'Egaroth` | ❌ duplicado de `Plantillas/Personajes Hombrew/Astrologia.docx` |

**El vault-exclusivo real son §2.1 a §2.7: unas 25.000 palabras, de las cuales solo ~3.500 son lore histórico y ~22.000 material de campaña.**

---

## 3. LAS DOS CAMPAÑAS TROCEADAS

### 3.1 `8495b.f. - Campanya de Itron` — esqueleto, no partida

**Qué es:** el guion global de una campaña oscura ambientada en el frente norte durante las Guerras del Ocaso. 11 escenas numeradas, con propósito dramatúrgico, acciones posibles, tiradas y consecuencias; lista de PNJ (aliados / antagonistas / misteriosos); tres líneas de decisión; tres finales alternativos (Restauración del Equilibrio / Victoria Militar / Dulce-amargo con sacrificio de un PJ).

**Estado:** **el esqueleto está completo (950 palabras) pero no hay ni una ficha rellenada.** Los tres archivos restantes de la carpeta son plantillas vírgenes con el nombre tecleado al azar: `ghghf.md`, `ffghfh.md`, `Sense títol.md`. No hay ficha de K-2, ni de Jotdik, ni del Laberinto, ni de la dona encadenada. **Ratio de completitud: ~20 %.**

**¿Cuadra la fecha de 8495 b.f.?** **Sí.** La maestra sitúa las Guerras del Ocaso entre `EV-8578-OCASO` (8578 b.f.) y `EV-8487-FINOCASO` (8487 b.f.). 8495 cae dentro, cerca del final — coherente con el tono de agotamiento («guerres centenàries», «guerra infinita», refugiados) y con `EV-SF-PRIMERSIGLO` (los asedios del Oeste, desde 8577). **La ambientación es canónicamente impecable.**

⚠️ **Pero el archivo se contradice a sí mismo.** El frontmatter dice:
```yaml
title: 8565b.f. - Campanya de Itron
```
mientras la carpeta y el nombre de archivo dicen **8495**. Discrepancia de 70 años dentro del mismo fichero. **8565 b.f. también caería dentro de las Guerras del Ocaso** (justo después del estallido), así que ambas son defendibles; hay que elegir una. Recomiendo **8495**, que es lo que aparece dos veces (carpeta + nombre) frente a una (frontmatter, campo que además Obsidian rellena automáticamente y suele quedar desactualizado).

### 3.2 `4592b.f. - Tren hacia el Himetsu` — completa, y cronológicamente imposible

**Qué es:** la pieza más terminada de todo el vault. Thriller cerrado de 5 días: los PJ viajan en un tren submarino de la Alianza hacia la isla de Himetsu; un grupo de polizones ("els Horitzons") intenta tomar el convoy. Estructura por día con eventos, puzles, relojes de tensión, checklist de pistas y un QTE final de desactivación de bomba.

**Estado:** **Día 1 escrito en prosa completa; Días 2–5 desglosados en fichas de evento y puzle** (3 eventos + 3 puzles Día 2, 2+2 Día 3, 2+2 Día 4, 3+1 Día 5) + `DIA5_OVERVIEW_Rellotge.md` y `DIA5_CHECKLIST_Pistes_i_Compte_Enrere.md`. 13 NPC, 4 PJ y 5 enemigos con estadísticas 5e. **Ratio de completitud: ~85 %.** Lo único abierto es el orden definitivo de los vagones (*«Cuando tengas el diseño del tren pactado… el Día 2 te lo monto con el puzzle de la válvula»* — el archivo termina en una nota al autor, no al jugador).

**¿Cuadra la fecha de 4592 b.f.? NO. Rotundamente no.** Tres colisiones:

1. **🔴 Anacronismo tecnológico de 3.943 años.** El tren es un *«convoy de vapor rúnico»* con *«caldera, reguladores… manómetros visibles»* y *«protocolos anti-explosión»*. La maestra fecha:
   - `EV-700-ALQUIMIA` (700 b.f.) — «se construyen los primeros mecanismos y tuberías»
   - `EV-649-VALVULA` (649 b.f.) — **la primera válvula de vapor**
   - `EV-527-VEHICULOS` (527 b.f.) — «el transporte civil a vapor globaliza el Imperio»

   Un tren de vapor en 4592 b.f. es como un ferrocarril en el Neolítico. El puzle central del Día 2 es literalmente *«Vàlvula Δ-7: El Retorn Roent»*, es decir, la mecánica de la campaña depende de la tecnología que el canon inventa 3.943 años después.

2. **🟠 La «Alianza» no existe todavía.** En 4592 b.f. la maestra está en la Era IX (Las Sombras Latentes). La primera alianza militar entre reinos del Oeste es `EV-4003-INCURSIONES` (4003 b.f., Ascaria + Bastrea + Udrax) y la **Primera Gran Alianza** institucional es `EV-1925-NUMANDUM` (1925 b.f., Ascaria + Ecla + Udrax + Bastrea). El tren está patrocinado por una «Alianza» que reúne exactamente a **Udrax + Ecla + Bastrea**: es la composición de 1925 b.f. **La campaña describe la Primera Gran Alianza, no algo de 4592.**

3. **🟡 Hueco cronológico sospechoso.** 4592 b.f. cae en el único vacío grande de la Era IX: entre `EV-4705-RESTAURACION` y `EV-4003-INCURSIONES` no hay nada. Es probable que la fecha se eligiera precisamente por estar libre, sin comprobar la tecnología.

**Recomendación:** **reubicar la campaña entre 1900 y 1700 b.f.** (posterior a Numandum, dentro del arco de las Cruzadas, con la Alianza funcionando) **o**, si se quiere conservar el vapor rúnico, después de **527 b.f.** La alternativa es declararla no canónica. **No debe entrar en la cronología maestra con la fecha 4592.**

### 3.3 Bonus: `Canon/Aegroum/Campanya 1 Boundinghton` — la clave del calendario

No la pedías, pero contiene el dato más valioso del vault para la cronología. `Capítols/Cap02_El_Relato_del_Pasado.md`:
> «una taberna en el año **756 del Despertar Elemental (1982 aC)**»

`Capítols/Cap03_Boundington_y_Los_Perdidos.md`:
> «Boundington en el año **757 del Despertar Elemental (1981 aC)**»

**2738 − 756 = 1982. 2738 − 757 = 1981.** Ver §5.4.

Estado del resto: **13 de sus 21 archivos están vacíos** (los 7 personajes, los 2 lugares, las 4 subquests). Los capítulos 4 y 5 son solo encabezados. `Campanya.md.md` (doble extensión, error de guardado) contiene un índice de archivos de los que **la mitad no existen** (`Pico_Dragon`, `Casco_Antiguo`, `Base_Militar`, `Venides`, `Skilla_Legado_Perdido`, toda la carpeta `Enemigos/`). Es una versión **más pobre** que `corpus_extraido/…Campaña 01: La Matanza de Boundington (Aventura de Venides).docx.md`.

---

## 4. LOS CANVAS: LA ESTRUCTURA NARRATIVA OCULTA

**Recuento real: 19 canvas en `OnegaiTimeLine/` + 1 en `Onegai/` = 20.** De los 19, **doce tienen un único nodo y cero aristas** (son marcadores de posición). Solo **cinco** contienen estructura.

### 4.1 ¿Hay canvas maestro? Sí: dos, encadenados

**Nivel 1 — `TML/9100b.f. -9071b.f. - EDAD DE LOS DIOSES/TML - Canvas.canvas`** (6 nodos, 5 aristas). Es una **cadena lineal pura** con una cabecera de texto:
```
[# TimeLine] → 00 Nacimiento de Egaroth → 01 CHRONOS → 02 SOFIA → 03 VIDA Y MUERTE → 04 Nacimiento de Egos y Eros
```
Esto es una **secuencia de causalidad, no de cronología**: cada Voluntad nace de una necesidad de la anterior. Es exactamente el orden de la Era 0 de la maestra (`EV-SF-EGAROTH` → `EV-SF-CHRONOS` → `EV-SF-SOFIA` → `EV-SF-VIDAMUERTE` → `EV-SF-EGOS`). **La maestra ya lo respeta.** ⚠️ La cadena **muere en el 04**: no enlaza con Deseo, ni con el Panteón, ni con la Gran Quebración, ni con la migración. `La gran migracion.md` está huérfano en la misma carpeta, sin ningún nodo que lo referencie.

**Nivel 2 — `TML/9071b.f. -8578 b.f. - ERA DEL DESPERTAR DE EGAROTH/9071b.f. -8578 b.f. - ERA DEL DESPERTAR DE EGAROTH.canvas`** (20 nodos, 6 aristas). Es el **índice de razas de la era**: 17 nodos apuntan a los canvas de raza, colocados **en una banda horizontal a la misma altura (y = −156)**, es decir, presentados como paralelos simultáneos, no como secuencia.

**La estructura significativa está en las 6 aristas, y revela un arco humano que ningún texto escribe:**
```
        [Humanos]  (nodo de texto, x=−4885)
           ├──→ CLAN ORH.canvas          (Aegroum)
           ├──→ CLAN YSLUR.canvas        (Ascaria)
           └──→ CLAN BASTIAN y CLAN ILMAR.canvas  (Bastrea)
                        │
                        ├──→ [EL DÍA DE LOS CUATRO SOLES 8954 B.F.]
                        ├──→ [EL DÍA DE LOS CUATRO SOLES 8954 B.F.]
                        └──→ [EL DÍA DE LOS CUATRO SOLES 8954 B.F.]
```
Es decir: **los tres clanes humanos convergen en el Día de los Cuatro Soles**. Y los tres canvas de clan están colocados **más abajo que las razas (y = 44 en vez de −156)**, en una franja propia — se los trata como sub-nivel de una única entidad "Humanos", no como razas independientes. Esto **valida la arquitectura de la maestra** (`EV-8964-FRAGMENTACION` → `EV-8954-CUATROSOLES`) y aporta un matiz: **son cuatro clanes, no tres** (Orh, Yslur, Bastian **e Ilmar**), y **Bastian e Ilmar comparten canvas**, coherente con que Kaelar Ilmar sea el que rechaza la tregua y sea derrotado. **Los canvas de clan están todos vacíos por dentro.**

⚠️ El canvas de era también tiene un **nodo de texto vacío** en `x=−3012, y=808`, y un **nodo duplicado**: el canvas de los Enanos (`8898 b.f. - mithril`) aparece **dos veces**, en `x=−3200` y en `x=−4080`.

### 4.2 `Elfos/9049 b.f. - La Fundació d'Itron i la Caiguda de Vincalp.canvas` — el arco élfico completo
10 nodos, 9 aristas. **Es el único canvas que traza un arco narrativo cerrado de principio a fin**, y define un orden que la maestra solo puede inferir:

```
Crònica dels Elfs (9049 b.f.)
   ├──→ Les Quatre subraçes ──┬──→ Aguakturos (Aigua)
   │                          ├──→ Vienturkos (Aire)
   │                          ├──→ Tierrakteros (Terra)
   │                          └──→ Chimenyorik (Foc)
   └──→ El Torneo de los Fuertes
              └──→ El Pacte de les Ànimes
                       └──→ FORTALECIMIENTO CIVILIZACIÓN ÉLFICA / PRIMERA GRAN ARBOLEDA (8765 b.f.)
                                 └──→ DEFENSA DE LAS COSTAS OESTES (8749 b.f.)
```
**Esto resuelve la duda que la maestra deja abierta.** `EV-SF-TORNEO` está marcada como *«s.f. (posterior a 8960 b.f.)… ⚠️ sin fecha explícita»*. El canvas establece sin ambigüedad que la secuencia **Torneo → Pacto de las Almas → Primera Gran Arboleda (8765) → Defensa Costas Oestes (8749)** es lineal e inmediata. Por tanto **el Torneo y el Pacto de las Almas caen entre 8950/8960 y 8765 b.f.**, y muy probablemente cerca del extremo tardío, porque la Primera Gran Arboleda es presentada como *consecuencia directa* del Pacto (la civilización que se refunda tras la catástrofe). **La maestra puede estrechar el rango de `EV-SF-TORNEO` y `EV-SF-PACTEANIMES` a `s.f. (entre 8950 y 8765 b.f.)`.**

⚠️ Dos nodos apuntan a archivos rotos o vacíos: `FORTALECIMIENTO… 8765 B.F..md` **no existe**, y `Les Quatre subraçes.md` y `DEFENSA DE LAS COSTAS OESTES 8749 b.f..md` existen pero están **a 0 bytes**. El arco está dibujado; tres de sus ocho estaciones no tienen texto.

### 4.3 Los otros dos canvas con estructura

**`Draconidos/8735 b.f. - La Fundación del Congreso de Wulcain.canvas`** (8 nodos, 7 aristas) — cadena lineal:
`Contexto → La Dificultad de Aislarse → El Primer Rompeaislamiento → ⚠️La Pérdida de Himetsumota → La Orden del Dragón Durmiente → El Congreso de la Caldera → Niveles de la Ciudad-Caldera → Congreso y la Consolidación de Wulcain`
⚠️ El orden es **narrativamente incoherente**: el aislamiento y su ruptura se cuentan *antes* de la fundación del Congreso que lo decreta. Y **`La Pérdida de Himetsumota.md` no existe** — es un eslabón perdido en medio de la cadena que sería el evento fundacional dracónido.

**`Elementales/8845 b.f. - Las aldeas de los Elementales.canvas`** (15 nodos, 19 aristas) — **el único canvas con topología de grafo, no de cadena**. Modela un árbol genealógico divino:
```
Contexto ──┬──→ Terrus (Tierra) ────┐
           ├──→ Ignis (Fuego) ──────┤
           ├──→ Aequor (Agua) ──┐   ├──→ Alianza ──┐
           └──→ Zephyrus (Aire)─┤   │              │
                                └──→ Los dos primeros hijos ←── Shorm Aergan
                                        ├──→ Sylphera ─┐
                                        └──→ Kaelis ───┼──→ Primer Conflicto
                                                       │
                                          Alianza ─────┘
                                                 ├──→ Orun ──┐
                                                 └──→ Anwen ─┼──→ Eryon ──→ La misión de Shorm Aergan
```
Lectura: **Aequor y Zephyrus (los dos leales) engendran a los dos primeros hijos con Shorm; Terrus e Ignis (los rebeldes) desembocan en "Alianza" y luego en el "Primer Conflicto"; de la rendición de Shorm nacen Orun y Anwen (uno para cada clan rebelde); y de los cuatro converge Eryon, el quinto, nacido solo de Shorm.** Eryon es el punto de convergencia de todo el grafo — es el único nodo con dos entradas y una sola salida hacia la misión final. **La maestra (`EV-8845-AERGAN`) enumera los cinco hijos pero no expresa esta lógica de dos + dos + uno.**
⚠️ Cinco nodos rotos en este canvas: `Contexto.md`, `Terrus`, `Zephyrus`, `Los dos primeros hijos` **no existen** como archivos.

### 4.4 ⭐ El canvas número 20: `Onegai/Sense títol.canvas`
Fuera de `OnegaiTimeLine/`, en la raíz del vault. Dos nodos, una arista:
```
TML/Creadores/8954 b.f. - Guerra.md  ──→  TML/Creadores/8865 b.f. - Envidia.md
```
Es **el embrión de un tercer nivel de canvas maestro: el de las intervenciones divinas a lo largo de la línea temporal**, paralelo al de razas. Solo tiene dos fichas y ambas están vacías, pero deja establecido que **Guerra actúa en 8954 y Envidia en 8865, en ese orden y con relación causal.**

### 4.5 Enlaces rotos en los canvas (7, verificados con normalización Unicode NFC)

| Canvas | Archivo que no existe |
|---|---|
| `Hombres Lagarto/8742 b.f. - El Asentamiento…canvas` | `Hombres Lagarto/EVENTOS/Contexto.md` (la carpeta `EVENTOS/` ni siquiera existe) |
| `Elfos/9049 b.f. …canvas` | `Elfos/EVENTOS/FORTALECIMIENTO DE LA CIVILIZACION ELFICA, PRIMERA GRAN ARBOLEDA 8765 B.F..md` |
| `Draconidos/8735 b.f. …canvas` | `Draconidos/EVENTOS/La Pérdida de Himetsumota.md` |
| `Elementales/8845 b.f. …canvas` | `Elementales/EVENTOS/Contexto.md` |
| `Elementales/8845 b.f. …canvas` | `Elementales/EVENTOS/Terrus, el Gigante Inamovible.md` |
| `Elementales/8845 b.f. …canvas` | `Elementales/EVENTOS/Zephyrus, el Espíritu del Viento.md` |
| `Elementales/8845 b.f. …canvas` | `Elementales/EVENTOS/Los dos primeros hijos.md` |

Además, `Nagas/8762 b.f. - La Rebelión de las Corrientes Oscuras.canvas` es un archivo de **2 bytes** (`{}`) y `Onegai 2/Sense títol 2.canvas` también.

---

## 5. FECHAS DEL VAULT vs CRONOLOGÍA MAESTRA

### (a) Fechas que COINCIDEN — 13

| Vault | Maestra | ID |
|---:|---|---|
| 9100–9071 b.f. (rótulo de era) | Era 0 + Era I | — |
| 9071–8578 b.f. (rótulo de era) | Eras I–IV | — |
| 9049 b.f. Fundació d'Itron | 9049 b.f. La Fundación de Itron | `EV-9049-ITRON` |
| 8954 b.f. Día de los Cuatro Soles | 8954 b.f. | `EV-8954-CUATROSOLES` |
| 8898 b.f. Primera veta de mithril | 8898 b.f. El Mithril y la Fundación de Udrax | `EV-8898-MITHRIL` |
| 8865 b.f. El Gran Pacto | 8865 b.f. | `EV-8865-GRANPACTO` |
| 8853 b.f. Juramento de las Llamas Eternas | 8853 b.f. | `EV-8853-LLAMASETERNAS` |
| 8845 b.f. Aldeas de los Elementales | 8845 b.f. | `EV-8845-AEON` / `EV-8845-AERGAN` |
| 8790 b.f. Marcha de las Cuatro Estaciones | 8790 b.f. | `EV-8790-MARCHA` |
| 8765 b.f. Primera Gran Arboleda | 8765 b.f. | `EV-8765-ARBOLEDA` |
| 8749 b.f. Defensa de las Costas Oestes | 8749 b.f. | `EV-8749-COSTASOESTE` |
| 8742 b.f. Asentamiento Hombres Lagarto | 8742 b.f. | `EV-8742-ORUN` |
| 8739 b.f. Ascensión del Guardián del Abismo | 8739 b.f. | `EV-8739-ARTRENAX` |
| 8735 b.f. Congreso de Wulcain | 8735 b.f. | `EV-8735-WULCAIN` |
| 8729 b.f. Fundación Clanes Yokai | 8729 b.f. | `EV-8729-CLANESYOKAI` |
| 1981 b.f. (757 d.e.) Boundington | 1981 b.f. | `EV-1981-BOUNDINGTON` |

### (b) Fechas que DISCREPAN — 5

| Evento | Vault | Maestra | Veredicto |
|---|---:|---:|---|
| **Fundación de Taurengrad** | **8739–8722** | 8739–8716 | 🟢 **el vault tiene razón.** `TL_OCR` diapositiva 193 dice literalmente «*La Fundacion de Taurengrad - 8739-8722 b.f.*». La maestra ya avisa de la variante («*el propio TL da además 8736 b.f. en el cuerpo y 8722 en el rótulo*») pero se quedó con 8716, que no aparece en ninguna fuente localizable. **Corregir a 8739–8722.** |
| **Plenitud de Itron** | **≈8950** | 8960 | 🟡 **probablemente el vault.** El fichero del vault es texto digital limpio (*«Cap al voltant de l'any 8950 abans de la Fractura (b.f.)»*); la maestra tomó 8960 del OCR de una imagen (*«8960 abans de la Fractura (b¢.)»*, línea 1947), que es exactamente el tipo de dígito que un OCR confunde. **Verificar en el `.pptx` original antes de decidir.** |
| **Unificación de Gliaddokx** | 8837 | **8797** | 🔴 **el vault está mal.** 8797 está confirmado por **cinco** fuentes independientes: `Ayashii.docx`, `Los Reinos del Oeste.docx` (dos veces), `Ciudades y sus elementos.xlsx` (dos hojas), `Estructura narrativa del Gran Compendio.docx` y `Gran Compendio de los Creadores.docx`. 8837 no aparece en ninguna. **Corregir el nombre del canvas.** |
| **La Purga de las Tormentas** | 8786 | **8760** | 🔴 **el vault está mal.** `Hombres Pajaro/EVENTOS/Contexto.md` es una **copia verbatim** del Gran Compendio (línea 813) con un solo cambio: el compendio dice «En el año **8760** b.f., los Aarakocra en las Selvas Fronterizas de Ulda…», el vault dice «En el año **8786** b.f., los Aarakocra en las Selvas Fronterizas de Ulda…». Todo lo demás, palabra por palabra, idéntico. Es un error de transcripción. **Corregir el canvas y el `Contexto.md`.** |
| **Rebelión de las Corrientes Oscuras** | 8762 | **8642** | 🔴 **el vault está mal.** 8642 lo dan el TL (diapositivas 138-144), el Gran Compendio y la Estructura narrativa. La maestra ya registra las variantes conocidas (8542, 8300); **8762 es una cuarta variante que no existe en ninguna fuente**. La ficha del vault además está vacía. **No adoptar.** |

### (c) Eventos del vault que NO están en la maestra — 8

1. **La prehistoria de los Deaps y el Hechicero** (s.f., anterior a `EV-SF-EGAROTH`). §2.2
2. **Aegor, la Voluntad Única** (s.f., entre `EV-SF-SOFIA` y `EV-SF-SEPARACION`). §2.4
3. **El Sellado de Vincalp bajo Itron** (s.f., durante el Pacto de las Almas). §2.1a
4. **La Diáspora Tierraktero a Ashye y su acogida por los Sagas** (s.f., ídem). §2.1b
5. **La Damnatio Memoriae Vienturko** (s.f., ídem). §2.1b
6. **La Extinción de los Chimenyorik** (s.f., ídem). §2.1b
7. **La Campaña de Itron / el frente norte de las Guerras del Ocaso** (8495 b.f.). §2.5
8. **El Juramento de Venides en Ceaseton** (1982 b.f. / 756 d.e.). §3.3 — nota: el texto también está en `corpus_extraido/…PARTIDAS.docx.md` y en el `.docx` de la Matanza de Boundington, así que **no es vault-exclusivo, pero sí falta en la maestra.**

Y un noveno **con reserva**: **El Tren Abisal de la Alianza** (fecha 4592 b.f. insostenible, §3.2).

### (d) Eventos de la maestra que el vault fecha distinto
Ver tabla (b). Son los cinco casos. Fuera de eso, **el vault no contiene ni una sola fecha posterior a 4592 b.f. salvo las dos de Boundington (1982 y 1981)**. Todo el arco de las 14 Cruzadas, el Imperio de Ascaria, la Era del Vapor y el Gran Cataclismo — es decir, **de 8487 b.f. a 300 a.f., el 80 % de la historia del mundo — no existe en el vault.**

### (e) ⭐ El vault resuelve la decisión abierta A3 (offset del calendario)

`Onegai_Addendum_THE_ONEGAI_PROJECT.md` deja A3 marcada como *«🔴 Abierta — el TL no aporta nada. Es la única decisión dura que queda»*: 2838 vs 2738 como epoch del Despertar Elemental.

**El vault aporta dos pares nuevos, ambos con offset 2738:**
- `Cap02_El_Relato_del_Pasado.md`: «año **756** del Despertar Elemental (**1982** aC)» → 2738 − 756 = 1982 ✔
- `Cap03_Boundington_y_Los_Perdidos.md`: «año **757** del Despertar Elemental (**1981** aC)» → 2738 − 757 = 1981 ✔

No son independientes de la prosa del corpus (el mismo texto aparece en `PARTIDAS.docx` y en `Campaña 01: La Matanza de Boundington.docx`), así que **no zanjan la duda por sí solos** — pero elevan a **29 los pares consecutivos con offset 2738 y a cero los pares con 2838**, y añaden un tercer documento independiente que usa la misma conversión. **La única fuente que implica 2838 sigue siendo la hoja *Años* del xlsx, que nadie ha abierto.** Recomendación firme: **adoptar 2738** y renombrar `EV-2838-DESPERTARELEMENTAL` → `EV-2738-DESPERTARELEMENTAL`.

---

## 6. CALIDAD Y ESTADO DEL VAULT

### 6.1 ¿Está al día? No. Es una obra parada hace cuatro meses.

Fechas de última modificación (hoy: 2026-08-11):

| Periodo | Archivos | Qué se tocó |
|---|---:|---|
| dic-2025 | 96 | Boundington, Campanya de Itron, todos los `Contexto.md` vacíos, las 12 plantillas |
| feb-2026 | 44 | *Tren hacia el Himetsu* completo |
| mar-2026 | 2 | Astrología, canvas de era |
| **abr-2026** | **10** | **Carpeta Elfos entera** — 25/04/2026, entre las 08:26 y las 08:34 |

**El último trabajo real fue el 25 de abril de 2026: nueve minutos volcando la carpeta de Elfos.** Desde entonces, nada. El vault **es anterior a todo el análisis del proyecto** (`Onegai_Cronologia_Maestra.md` y `GUIA_DEL_PROYECTO.md` son de agosto de 2026) y **no incorpora ninguna de sus decisiones**.

### 6.2 Diagnóstico: es un índice, no un corpus

- **27 de 133 archivos `.md` (20 %) están literalmente a 0 bytes.**
- **32 archivos contienen placeholders `{{...}}` sin sustituir** — de los cuales 12 son plantillas legítimas y **20 son fichas de contenido con el frontmatter sin rellenar**, incluidas todas las del Himetsu y las dos de `Creadores/`.
- **12 de los 19 canvas tienen un solo nodo y ninguna arista.**
- **7 enlaces de canvas apuntan a archivos inexistentes**; 2 canvas son ficheros de 2 bytes.
- **Al menos 1 enlace `[[...]]` roto en texto** (`[[05 - TML - CAMBIOS EN EGAROTH -]]`).
- **1 archivo con doble extensión** (`Campanya.md.md`), **3 archivos con nombre tecleado al azar** (`ghghf`, `ffghfh`, `Sense títol`), **1 archivo con el texto «hola, buenos día»**.
- **1 nodo duplicado** en el canvas de era (Enanos aparece dos veces).

### 6.3 Dónde está la calidad
Cuando el vault escribe, escribe muy bien. Los tres mejores textos del proyecto entero están aquí:
1. **`El Pacte de les Ànimes`** — 2.502 palabras de prosa mitológica con diálogo teológico. Sin equivalente.
2. **`Tren Abisal de la Alianza`** — 1.310 palabras de diseño de escenario con 14 zonas, ambiente sensorial (luz/sonido/olor/clima), conflictos internos, rumores y secretos. Es un modelo de ficha de lugar.
3. **`00 - TML - Nacimiento de Egaroth`** — cosmogonía original que mejora la del Gran Compendio.

### 6.4 Lo que el vault NO es
No es una versión antigua *de la cronología maestra* — es un **proyecto paralelo abandonado a medio construir**. Su estructura por era → raza → evento es **mejor** que la del corpus (que es un montón de `.docx`), y sus canvas contienen decisiones estructurales que ningún texto explicita (§4). Pero como fuente de datos es **derivativo en el 85 % de su volumen** y **contiene cuatro fechas erróneas que no deben propagarse**.

**Veredicto: el vault se saquea, no se sincroniza.** Extraer §2.1–§2.7 y §4, corregir Taurengrad y verificar la Plenitud de Itron, ignorar el resto.

---

## LO QUE HAY QUE INCORPORAR A LA CRONOLOGÍA MAESTRA

### A. Entradas nuevas (9)

### s.f. (anterior a toda la Cosmogonía) — La Especie de los Deaps y la Guerra del Hechicero   `EV-SF-DEAPS`
Egaroth pertenece a los **Deaps**, una especie de seres gigantescos que flotaban en armonía por el Gran Espacio Vacío agrupados alrededor de puntos de energía; un Hechicero surgido dentro de uno de ellos los llevó casi a la extinción hasta que perdió el control de su magia y creó un agujero negro que engulló su corrupción. Egaroth, no detectado por estar más apartado, quedó conmocionado y cayó en su sueño; los restos de los Deaps muertos se convirtieron con el tiempo en las estrellas.
**Fuentes:** `Onegai/OnegaiTimeLine/TML/9100b.f. -9071b.f. - EDAD DE LOS DIOSES/00 - TML - Nacimiento de Egaroth y las Voluntades Creadoras.md` (⭐ única fuente en todo el proyecto) · **Desarrollo:** ⬜ pendiente
⚠️ El archivo original contiene una elipsis (`...`) que indica un párrafo perdido entre la llegada del punto de energía y el despertar por Chronos.

### s.f. (inmediatamente antes de `EV-SF-CHRONOS`) — El Primer Intento de Despertar   `EV-SF-PRIMERINTENTO`
Chronos nace del primer intento fallido de Egaroth por despertar, frustrado porque «las magulladuras que le había dejado la explosión de corrupción aún pasaban factura»; el destello dura menos que la fracción de segundo que tarda una abeja en batir las alas, y de esa unidad mínima de tiempo nace su creador.
**Fuentes:** `TML/9100b.f. -9071b.f. - EDAD DE LOS DIOSES/01 - TML - CHRONOS.md` (⭐ única fuente) · **Desarrollo:** ⬜ pendiente

### s.f. (entre `EV-SF-SOFIA` y `EV-SF-SEPARACION`) — Aegor, la Voluntad Única   `EV-SF-AEGOR`
Antes de escindirse en las Voluntades Gemelas, Vida y Muerte fueron un solo ser llamado **Aegor**, nacido de la necesidad de definir principio y final; con él llegaron seres menores que fueron **los primeros habitantes de Egaroth**.
**Fuentes:** `TML/9100b.f. -9071b.f. - EDAD DE LOS DIOSES/02 - TML - SOFIA.md` (⭐ única fuente; el nombre *Aegor* no aparece en el Gran Compendio, el TL ni ningún otro documento) · **Desarrollo:** ⬜ pendiente

### s.f. (durante el Pacto de las Almas) — El Sellado de Vincalp en el Calabozo de Obsidiana   `EV-SF-SEGELLVINCALP`
Un grupo reducido de Tierrakteros y Vienturkos supervivientes traza con la sangre de los caídos una cadena rúnica de ligamen de almas cuyo propósito no es vencer sino contener: Vincalp, ya primer vampiro e inmortal, es arrastrado a las profundidades de Itron y **sellado, no destruido**, en «un calabós d'obsidiana i memòria».
**Fuentes:** `TML/…/Elfos/EVENTOS/El Pacte de les Ànimes - El Naixement dels Elfs.md` (⭐ el TL solo conserva el título de este episodio, diapositiva 236) · **Desarrollo:** ➡️ misma ficha

### s.f. (mismo momento) — La Diáspora Tierraktero y la Acogida de los Sagas   `EV-SF-DIASPORATIERRA`
Solo un puñado de Tierrakteros sobrevive al Torneo; huyen en embarcaciones precarias hasta las costas de **Ashye**, donde los reciben los **Sagas**, «que veieren en ells un instrument per al seu propi destí».
**Fuentes:** `TML/…/Elfos/EVENTOS/El Pacte de les Ànimes…md` (⭐ vínculo Tierrakteros–Sagas inédito) · **Desarrollo:** ➡️ misma ficha
⚠️ **Corrige la entrada actual `EV-SF-PACTEANIMES`**, que atribuye la supervivencia a los Aguakturos y su deriva hacia los Nagas.

### s.f. (mismo momento) — La Damnatio Memoriae Vienturko   `EV-SF-MEMORIAVIENTURKO`
Los Vienturkos supervivientes destruyen la biblioteca de conocimiento rúnico, queman documentos, rompen reliquias y borran todo rastro de Vincalp; algunos desaparecen la noche siguiente al ritual, «segons alguns per suïcidi, segons d'altres, absorbits per l'eco de la criatura». Explica la ausencia de fuentes élficas primarias sobre el Torneo.
**Fuentes:** `TML/…/Elfos/EVENTOS/El Pacte de les Ànimes…md` (⭐ única fuente) · **Desarrollo:** ➡️ misma ficha

### s.f. (mismo momento) — La Extinción de los Chimenyorik   `EV-SF-FICHIMENYORIK`
**Shalira Escorça Lluminosa** canaliza «l'última cançó de foc», se envuelve en llamas puras y carga contra Vincalp abriendo un pasillo para las tropas; sus llamas se transforman en animales luminosos efímeros. Cuando la luz se apaga, «el clan havia desaparegut per complet del camp de batalla».
**Fuentes:** `TML/…/Elfos/EVENTOS/El Pacte de les Ànimes…md` (⭐ única fuente) · **Desarrollo:** ➡️ misma ficha

### 8495 b.f. — El Frente Norte de las Guerras del Ocaso   `EV-8495-FRENTENORTE`
Con el norte del continente devastado y el Nuragapóken sellado, la guerra enfrenta a **Hombres Gato + Minotauros + Chimenyorik** contra **Orcos + Hombres Pájaro + Sagas + Vienturkos**; el último campo de batalla es **Quezados**, los líderes enemigos son **K-2** (orcos) y **Jotdik** (hombres pájaro), y la retirada de los Sagas por orden divina hunde el frente aliado.
**Fuentes:** `Onegai/OnegaiTimeLine/Campanyes/OneShot/8495b.f. - Campanya de Itron/Campanya/8495b.f. - Campanya de Itron.md` (⭐ único orden de batalla de las Guerras del Ocaso en todo el proyecto) · **Desarrollo:** ➡️ misma ficha (escenas 1-8, esqueleto sin fichas de PNJ)
⚠️ El frontmatter del propio archivo dice `8565b.f.`; carpeta y nombre de archivo dicen 8495. Ambas caen dentro de las Guerras del Ocaso (8578–8487). **Se propone fijar 8495.**
⚠️ Implica que **Chimenyorik y Vienturkos siguen siendo facciones políticas vivas y enfrentadas** cuatro siglos después del Pacto de las Almas, lo que matiza la idea de fusión total en «los Elfos».

### 1982 b.f. (756 d.e.) — El Juramento de Venides en Ceaseton   `EV-1982-CEASETON`
Venides hijo de Nefaria es nombrado caballero y lo celebra en Ceaseton, ciudad costera al oeste de Nefaria, con Sandes hijo de Killville y los hermanos Edul y Verina de Perishton; la excursión que planean a las Santas Montañas termina revelando la adhesión de sus tres amigos a la secta en auge en Perishton, Venides rompe con ellos y es enviado a investigar los rumores sobre los Perdidos en Boundington.
**Fuentes:** `Campanyes/Canon/Aegroum/Campanya 1 Boundinghton/Capítols/Cap02_El_Relato_del_Pasado.md` · `corpus_extraido/THE ONEGAI PROJECT__PARTIDAS.docx.md` · `corpus_extraido/…Campaña 01_ La Matanza de Boundington.docx.md` · **Desarrollo:** ➡️ `Campañas/Canon/Campaña 01: La Matanza de Boundington (Aventura de Venides).docx`
Es el prólogo directo de `EV-1981-BOUNDINGTON` y el eslabón que faltaba entre `EV-1987-TEMPLOPERDIDOS` y la Matanza.

### B. Correcciones a entradas existentes (5)

**1. `EV-8739-TAURENGRAD` — cambiar el rango a 8739 – 8722 b.f.**
El rótulo de la diapositiva 193 del TL dice literalmente «8739-8722 b.f.» y el canvas del vault (`Minotauro/8739-8722 b.f. - La Fundación de Taurengrad.canvas`) lo confirma. **El año 8716 no aparece en ninguna fuente localizable.** Mantener la nota sobre las variantes 8736 (cuerpo del TL) y 8490 (`Estructura narrativa`).

**2. `EV-8960-PLENITUD` — verificar 8960 vs 8950 b.f.**
La maestra tomó 8960 del OCR de una imagen; el fichero digital limpio del vault (`Elfos/EVENTOS/Crònica dels Elfs…md`) dice «Cap al voltant de l'any **8950** abans de la Fractura (b.f.)». **Comprobar la diapositiva 228 en `onegai timeline.pptx`** antes de decidir; si el `.pptx` no permite leerlo, prevalece el texto digital (8950), porque el vault es la fuente y la diapositiva la copia.

**3. `EV-SF-TORNEO` y `EV-SF-PACTEANIMES` — estrechar el rango a «s.f. (entre 8950/8960 y 8765 b.f.)».**
El canvas `Elfos/9049 b.f. - La Fundació d'Itron i la Caiguda de Vincalp.canvas` establece la cadena **Torneo → Pacto de las Almas → Primera Gran Arboleda (8765) → Defensa de las Costas Oestes (8749)** como secuencia lineal e inmediata. La Primera Gran Arboleda es la refundación *posterior a* la catástrofe, luego el Pacto es anterior a 8765 b.f. Elimina la incertidumbre abierta de «s.f. (posterior a 8960)».

**4. `EV-SF-PACTEANIMES` — reescribir la definición.**
La actual dice: *«los Aguakturos derivarán en los Nagas y Azhira Espiral de Marfil será su única superviviente»*. El texto íntegro del vault dice otra cosa: los Chimenyorik se extinguen por completo, los Tierrakteros huyen a Ashye y son acogidos por los Sagas, los Vienturkos borran su propia memoria, y Azhira huye herida a las orillas salvada por dos compañeros que mueren. Los elfos nacen por un canto conjunto de **Vida, Muerte y Egos** —propuesto por **Guerra**— que funde los cuerpos en una amalgama y la hace estallar en lluvia; cada gota forma un elfo. **Reescribir y añadir `Desarrollo: ➡️ TML/…/Elfos/EVENTOS/El Pacte de les Ànimes - El Naixement dels Elfs.md`, que pasa de ⬜ a ➡️.**

**5. `EV-2838-DESPERTARELEMENTAL` — cerrar A3 y renombrar a `EV-2738-DESPERTARELEMENTAL`.**
El vault añade dos pares más con offset 2738 (756 d.e. = 1982 b.f.; 757 d.e. = 1981 b.f.) desde un tercer documento independiente. Total: **29 pares consecutivos con 2738, cero con 2838.** La única fuente que implica 2838 es la hoja *Años* del xlsx, no verificada.

### C. Fechas del vault que NO deben adoptarse (3)

| Vault | Correcto | Motivo |
|---:|---:|---|
| 8837 b.f. (Gliaddokx) | **8797 b.f.** | 8797 confirmado por 5 fuentes; 8837 por ninguna |
| 8786 b.f. (Purga de las Tormentas) | **8760 b.f.** | el `Contexto.md` del vault es copia verbatim del Gran Compendio con el año alterado |
| 8762 b.f. (Corrientes Oscuras) | **8642 b.f.** | cuarta variante inexistente en toda fuente; la ficha del vault además está vacía |

### D. Material a NO incorporar por ahora (1)

**El Tren Abisal de la Alianza, `4592 b.f.`** — 21.028 palabras de campaña excelente con una fecha insostenible: un convoy de vapor rúnico 3.943 años antes de `EV-649-VALVULA` (la primera válvula de vapor) y 2.667 años antes de la Primera Gran Alianza `EV-1925-NUMANDUM`, cuya composición exacta (Udrax + Ecla + Bastrea) es la que patrocina el tren.
**Antes de incorporarlo hay que decidir una de tres:** (a) reubicarlo entre **1900 y 1700 b.f.**, con la Alianza ya funcionando pero manteniendo el vapor como anacronismo asumido; (b) reubicarlo **después de 527 b.f.**, con la tecnología ya inventada; (c) declararlo material no canónico. **Mi recomendación es (b): posterior a 527 b.f.**, porque el tren, los protocolos anti-explosión y la válvula Δ-7 son el corazón mecánico de la campaña y no se pueden quitar, mientras que la identidad concreta de «la Alianza» sí es negociable.
