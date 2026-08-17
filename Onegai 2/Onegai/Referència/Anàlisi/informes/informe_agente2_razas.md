# Informe Agente 2 — Razas, geografía, genealogía, astrología y clases

> Corpus analizado: `THE ONEGAI PROJECT` (extracción markdown en `outputs/corpus/`).
> Contrastado contra `Onegai_Cronologia_Entendida.md` (en adelante **[CRON]**).
> Todo lo aquí escrito procede literalmente de las fuentes citadas. Lo que no está en las fuentes se marca como hueco, no se rellena.

---

## 0. Resumen ejecutivo — lo más importante que le falta a [CRON]

1. **Existe una lista canónica de 17 capitales de reino** (`Campañas/Ciudades.xlsx`, Hoja 2, bloque final). Resuelve la duda #8 de [CRON] ("13 reinos, solo 7 confirmados") y **contradice 6 de las capitales que [CRON] da por buenas**.
2. **Aparecen dos naciones que [CRON] no menciona en absoluto: `Nocturnsea` y `Ostad`** (esta última sí como topónimo, no como nación).
3. **`Ciudades y sus elementos.xlsx` (Hoja 2/3) contiene ~75 eventos fechados entre 9071 y 3059** — es decir, **rellena precisamente el "HUECO MAYOR 8487–3000 b.f."** que [CRON] declara vacío (duda #7). Pero **usa fechas incompatibles** con las de [TL] para los mismos eventos fundacionales.
4. **La Hoja 4 de ese mismo xlsx es un mapa en rejilla** (cada celda = territorio), y la Hoja 5 es la misma rejilla codificada por colores/elementos. Es la única representación geográfica real del mundo que hay en el corpus.
5. **El árbol genealógico (`Plantillas/Árboles genealogicos.xlsx`) da la dinastía completa de Ascaria y Aegroum con apellidos, años de reinado y años de vida** en calendario `d.e.` (729–900). [CRON] solo tiene 4 de esos 6 reyes y ninguno con apellido.
6. **El sistema astrológico usa 3 lunas y 7 elementos, no 4.** Y asigna una deidad regente a cada mes, incluyendo dos Creadores que [CRON] apenas nombra (**Deitrok**, **Destrucción**).

---

## 1. Catálogo de razas

Fuente base: los 18 archivos `Plantillas/Razas/*.docx`. Cada ficha tiene un patrón fijo: *Características raciales / Carácter / Maestría cultural / Estilo de vida / Conocimiento Ancestral / Lenguajes* + subrazas, y **cada subraza declara su alineación con dos Creadores** ("Diferencias religiosas"). Ese campo es la fuente más sistemática de patrocinio Creador↔raza que existe en el corpus, y **[CRON] lo desconoce por completo** (su tabla "Patrocinio Creador ↔ Raza" no coincide).

| Raza | Nación / territorio | Subrazas y Creadores patronos (según ficha) | Rasgos culturales clave | ¿En [CRON]? |
|---|---|---|---|---|
| **Humanos de Bastrea** | Bastrea (cap. Numandum) | Diplomáticos de Bastrea → **Sofía + Eros**; Guardianes del Elemento → **Chronos + Vida** | Diplomacia, cultura, magia elemental (elige fuego/agua/aire/tierra), tratados y alianzas; zona de paso de viajeros; biblioteca de Numandum y universidad de Ection. Fuentes: `Razas/Humanos de Bastrea.docx`, `INTRODUCCIÓN NACIONES Y RAZAS.docx` | Sí (incompleto) |
| **Humanos de Aegroum** | Aegroum (cap. Ranmont) | Guardianes de la Luz → **Egos + Sofía**; Alquimistas de Aegroum → **Chronos + Envidia** | Devoción a la Luz, cultura cerrada, alquimia; **exportan pociones, licores y "agua lupulada"** a todo el continente; ciudades amuralladas "preparadas para un asedio que nunca llega". Idioma propio: Aegroumiano. Fuentes: `Razas/Humanos de Aegroum.docx`, `INTRODUCCIÓN…` | Sí |
| **Humanos de Ascaria** | Ascaria (cap. Finesaux) | Nobles de Ascaria → **Sofía + Chronos**; Mercenarios de Ascaria → **Eros + Envidia** | ⚠️ La ficha los describe **tras la caída del imperio**: "orgullosos de su historia, marcada por la caída y la pérdida de su antiguo imperio… ahora defensores de la justicia". Artesanía del cuero, mercenariado. Fuentes: `Razas/Humanos de Ascaria.docx`, `INTRODUCCIÓN…` | Sí (pero [CRON] los retrata solo como imperio tiránico) |
| **Humanos (Perdidos)** | sin nación; comunidades secretas | Traidores Oscuros → **Envidia + Muerte**; Hechiceros Oscuros → **Egos + Chronos** | **Raza jugable, no solo facción.** Humanos corrompidos que "siguen los dictados de Gurkazaal"; conspiran para expandir su influencia. Lengua Oscura. Fuente: `Razas/Humanos (Perdidos).docx` | ⚠️ [CRON] los trata como facción histórica, no como raza |
| **Elfos de Ecla** | Ecla | Elfos del Bosque → **Sofía + Vida**; Elfos de las Estrellas → **Chronos + Eros** | Asentamientos en copas de árboles gigantes, invisibles para quien no conoce los caminos; magia elemental (agua/tierra/aire/fuego); resistencia al encantamiento. Universidad de Ekait. Fuentes: `Razas/Elfos de Ecla.docx`, `INTRODUCCIÓN…` | Sí |
| **Elfos Oscuros (Drow)** | Ashye / Underdark | Drow de Casa Noble → **Envidia + Muerte**; Drow Explorador → **Chronos + Muerte** | **Sociedad matriarcal**, ciudades subterráneas, intriga política, venenos. "Lengua del Inframundo". Fuente: `Razas/Elfos Oscuros (Drow).docx` | Parcial (solo como "Sagas/Drow" en Ashye) |
| **Enanos de Udrax** | Udrax (cap. Bomengrid) | Enanos Forjadores → **Egos + Chronos**; Enanos Historiadores → **Sofía + Chronos** | Ciudades subterráneas; **escritura y poesía tan valoradas como la forja**; guardianes de la historia. Universidad de Bomengrid. Fuentes: `Razas/Enanos de Udrax.docx`, `INTRODUCCIÓN…` | Sí |
| **Orcos de Gongorguma** | Gongorguma (cap. Guskuedor) | Orcos del Consejo → **Sofía + Eros**; Orcos Dramáticos → **Egos + Vida** | ⭐ **Orcos pacíficos, sensibles, amantes del teatro**, cartógrafos y cronistas; ciudades organizadas en torno a teatros y bibliotecas; gobierno por consejo con representante por ciudad. Universidad teatral de Qethatos. Fuentes: `Razas/Orcos de Gongorguma.docx`, `INTRODUCCIÓN…` | ⚠️ [CRON] los define como "sabios + guerreros"; el corpus insiste en artistas |
| **Dracónidos** | Wulcain | Dracónidos de Fuego → **Egos + Vida**; Dracónidos de **Hielo** → **Sofía + Chronos** | Descendientes de dragones; códigos estrictos de honor; ciudades fortificadas en montañas; ven la magia como legado sagrado. Fuente: `Razas/Dracónidos.docx` | Parcial — **la subraza de Hielo no aparece en [CRON]** |
| **Hombres Gato (Felinos)** | Bosmurg | Cazador Nocturno → **Vida + Envidia**; **Cara Loca** → **Sofía + Chronos** | Tribus nómadas, desconfiados, libertad por encima de todo. "Cara Loca" es explícitamente una **tribu** con nombre propio. Fuente: `Razas/Hombres Gato (Felinos).docx` | Sí |
| **Hombres Pájaro (Aarakocra)** | Choubar | Aarakocra de las Montañas → **Sofía + Eros**; Aarakocra del Viento → **Chronos + Eros** | Nidos comunales en cimas; navegación aérea y tiro con arco; fuerte sentido de justicia y libertad. Fuente: `Razas/Hombres Pájaro (Aarakocra).docx` | Sí |
| **Hombres Lagarto** | Ostad / pantanos y selvas | Del Pantano → **Vida + Egos**; De la Selva → **Sofía + Muerte** | Amorales-pragmáticos ("no ven el mundo en términos de moral sino de supervivencia"); **comunicación con criaturas reptiles**; hablan Dracónico. Fuente: `Razas/Hombres Lagarto.docx` | Sí |
| **Nagas** | Tabaxi | Nagas Venenosos → **Envidia + Muerte**; Nagas Arcanos → **Chronos + Sofía** | Mitad humano mitad serpiente; veneno + magia ilusionista; pantanos y cuevas. Hablan **Dracónico** (dato de parentesco lingüístico con Hombres Lagarto y Dracónidos). Fuente: `Razas/Nagas.docx` | Sí |
| **Minotauros** | Esmua | Del Laberinto → **Chronos + Envidia**; De la Llanura → **Egos + Vida**; **De la Roca Quemada → "Destrucció" + "Deitrok"** | Clanes tribales en llanuras o cuevas; rituales de combate; conexión con los elementos de la tierra. La subraza Roca Quemada son "descendientes de las antiguas guerras del clan, expertos en combatir el fuego". Fuente: `Razas/Minotauros.docx` | ⚠️ Roca Quemada **no está en [CRON]**, y es la única raza que nombra a **Destrucción** y **Deitrok** como patronos |
| **Sagas** | Ashye / Umbralia | Sagas de la Locura → **Muerte + Envidia**; Sagas del Abismo → **Muerte + Chronos** | Seres de magia oscura y locura, "vagan por los límites de la realidad"; viven en **los oscuros bosques de Ashye**; "Lengua del Abismo". Fuentes: `Razas/Sagas.docx`, `INTRODUCCIÓN…` | Sí |
| **Yokai** | Ayashii | Yokai de los Vientos → **Eros + Sofía**; Yokai de las Sombras → **Muerte + Envidia** | **No forman sociedades tradicionales**; habitan zonas aisladas o mágicamente ocultas; existencia ligada a elementos o espíritus. Fuente: `Razas/Yokai.docx` | Sí |
| **Medianos** | ❓ sin nación asignada | Medianos del Bosque → **Sofía + Vida**; Medianos Urbanos → **Chronos + Eros** | Comunidades rurales pequeñas y pacíficas en colinas o llanuras; resistencia al miedo. Fuente: `Razas/Medianos.docx` | ❌ **NO aparece en [CRON]** — ni raza, ni territorio, ni evento fundacional |
| **No Muertos** | ❓ sin nación; comunidades ocultas | Espectros → **Muerte + Chronos**; Revenants → **Envidia + Muerte** | Retornados por magia oscura o **maldiciones antiguas**; conservan personalidad previa; no se integran con los vivos. Hablan **Infernal**. Fuente: `Razas/No Muertos.docx` | ❌ **NO aparece como raza en [CRON]** (solo como ejército invocado en la Carga de los Muertos) |
| **Poseídos** | ❓ periferia de las sociedades | De la Sombra → **Muerte + Envidia**; **Del Vacío → Chronos + Sofía** | Habitados por entidades que "les otorgan poder pero exigen control constante"; dualidad interna; los del Vacío están "vinculados a entidades del **vacío o el abismo**". Fuente: `Razas/Poseídos.docx` | ❌ **NO aparece en [CRON]** — implica un plano/cosmología del Vacío no documentada |
| **Elfo de las Sombras** | ❓ | — | Solo existe como ficha de personaje pregenerado. Fuente: `Personajes Hombrew/Raza_ Elfo de las Sombras.docx` | ❌ no en [CRON] |
| **Gnomo del Subsuelo** | ¿Knehapnest? (no se afirma) | — | Solo ficha pregenerada; clase asociada Inventor. Fuente: `Personajes Hombrew/Raza_ Gnomo del Subsuelo.docx` | ⚠️ [CRON] menciona Knehapnest (gnomos) pero **no hay ficha de raza gnoma** en el corpus |
| **Tiflin de la Llama Oscura** | Mistarium (por inferencia; la ficha no lo dice) | — | Ficha pregenerada. **No existe ficha `Razas/Tieflings.docx`** en el corpus. Fuente: `Personajes Hombrew/Raza_ Tiflin de la Llama Oscura - Oriol.docx` | ⚠️ los Tieflings son centrales en [CRON] pero **carecen de ficha racial** |

### 1.b Razas con nación en [CRON] pero **sin ficha de raza** en el corpus
Goblins (Gliaddokx), Elementales (Aeon), Vampiros, Gnomos, Shardmind, Shadar-kai, Aguakturos. Existen como clanes/pueblos en los xlsx pero nadie escribió su plantilla racial.

### 1.c Estética (única ficha de vestuario del corpus)
`Razas/Estetiques/HUMA DE BASTREA.docx` (en catalán) define **dos castas visuales de Bastrea**:
- **Guardians de l'Element**: referencias "tradició catalana + mestres artesans venecians + Assassin's Creed"; **cinturó d'elements amb quatre medallons girables (focus actiu)** y **panells ceràmics elementals intercanviables** en el peto. Es decir: los Guardianes del Elemento llevan físicamente un selector de elemento activo. Paleta azul mar / blanco cal / verde cobre / terracota.
- **Diplomàtics de Bastrea**: repúblicas marítimas + Corts catalanes; armadura oculta, capa con solapa para documentos, collar de oficio, bastó amb *sigillum*.
- **Barretina especial (común a Bastrea)**: con nervios internos de cobre patinado, orejeras plegables y bandas de seda con tres modos de uso incluido **emmascarament** (ocultar rostro). Es el marcador cultural visual de la nación.

---

## 2. Mapa político completo

### 2.a Capitales — LISTA CANÓNICA
Fuente: `Campañas/Ciudades.xlsx`, Hoja 2, bloque "→ Capital del reino". **17 entradas.**

| Reino | Capital (según Ciudades.xlsx) | Capital que dice [CRON] | Raza dominante | Notas |
|---|---|---|---|---|
| Gongorguma | **Guskedor** | Guskuedor ✓ | Orcos | grafía alterna "Guskuedor" en `INTRODUCCIÓN…` |
| Bastrea | **Numandum** | Numandum ✓ | Humanos | |
| Ascaria | **Finesaux** | Finexsaux ⚠ | Humanos | `Ciudades y sus elementos.xlsx` escribe "Finesaux" también |
| Aegroum | **Ranmont** | Ranmont ✓ | Humanos | |
| Gliaddokx | **Havar'gruztak** | Vurn ❌ | Goblins | contradicción directa |
| Ecla | **Umedan** | Thorn ❌ | Elfos | contradicción directa |
| Udrax | **Bomengrid** | Bomengrid ✓ | Enanos | "Boomengird" en el otro xlsx |
| Tabaxi | **Naka't-ol** | "Tabaxi" ❌ | Nagas | |
| Wulcain | **Dhin Thyraxion** | ❓ | Dracónidos | **rellena hueco de [CRON]** |
| Bosmurg | **Tor'k Hazar** | Teshkorr ❌ | Hombres Gato | Teshkorr aparece como ciudad, no capital |
| Esmua | **Saif-l'sa** | Taurengrad ❌ | Minotauros | Taurengrad = fundación mítica, no capital |
| Ashye | **Zathor'aetz** | Umbralia ❌ | Drow + Sagas | Umbralia figura como *Fortaleza* y *red subterránea* |
| Choubar | **Klimnebra** | ❓ | Aarakocra | **rellena hueco** |
| Mistarium | **Venordemn** | ❓ (isla mayor oriental) | Tieflings | **rellena hueco** |
| **Nocturnsea** | **Ciudad de Grytoz** | ❌ ausente | ❓ | **NACIÓN NUEVA** |
| Aeon | **Conclave Elemental** | ❓ | Elementales | **rellena hueco** |
| Ayashii | **Himetsumota (Templo)** | (Himetsu) | Yokai | la capital es el templo |
| *(Ostad)* | ❓ sin capital listada | mencionado solo como territorio | Hombres Lagarto | aparece como territorio propio en el mapa y con eventos ("Primeros pueblos en Ostad" 7932; "Fundación del primer templo en Ostad" 6821) |

→ **Total de entidades territoriales: 18.** Si el canon exige "13 reinos", hay que decidir cuáles 5 no cuentan (candidatos: Ostad, Nocturnsea, Aeon, Ayashii, Knehapnest — este último **no aparece en ninguna de las dos hojas de capitales ni en el mapa**).

### 2.b Geografía real: el mapa en rejilla
`Ciudades y sus elementos.xlsx`, **Hoja 4** = mapa del mundo en celdas, ~26 columnas × ~25 filas, cada celda etiquetada con el nombre del reino. Lectura de la rejilla (oeste→este, norte→sur):
- **Bloque noroeste/oceánico**: Ayashii (islas), Aeon (masa grande al norte-centro), Tabaxi (costa oeste, franja vertical).
- **Continente occidental**: Ecla al norte, Udrax a su este, Gliaddokx al noreste, Bastrea al centro-sur, Ascaria al oeste-sur (con enclaves insulares aislados), Aegroum al sur.
- **Continente oriental**: Mistarium al norte (islas), Nocturnsea al norte-oeste (bloque propio de 4 celdas), Gongorguma ocupando la mayor masa central-este, Ostad como franja intermedia occidental, Choubar al sur de Gongorguma, Ashye y Bosmurg debajo de Choubar (Ashye al oeste, Bosmurg al este), Wulcain y Esmua al sur, Tabaxi reapareciendo en la costa sur-este.
- **Hoja 5** = la misma rejilla pero con celdas 🟥 🟧 🟩 🟦 + marcadores `C` e `I`. **La leyenda no está escrita en ninguna parte del corpus.** Por posición, `C` bordea siempre las masas de reino (probable costa/frontera) e `I` aparece aislada en el mar (probable isla), y los colores parecen codificar elemento/bioma. **Esto es el "Ciudades y sus elementos" prometido por el nombre del archivo, pero la clave elemento↔color falta.** Es el hueco más accionable que he encontrado: sin la leyenda no se puede cerrar el punto 2 del encargo (elemento asociado a cada ciudad).

### 2.c Estado post-Cataclismo
**No hay ni un solo dato** en mi dominio de archivos sobre qué reinos sobreviven al Cataclismo. La duda #9 de [CRON] sigue abierta.

### 2.d Ciudades secundarias documentadas (más allá de las capitales)
De `Ciudades y sus elementos.xlsx` Hoja 2 y `Campañas/Ciudades.xlsx`:
- **Ascaria**: Finesaux (Palacio Real, Mercado Principal, Fortaleza de la familia Voskezorik, Plaza Central, Puertas del Este con estatuas de héroes), **Velours** (población fronteriza; nombre antiguo **"Mal Lunar"**, cf. evento 4914 "Expansión de Ascaria hacia las Tierras Interiores → Mal Lunar (actual Velours)"), **Bescau** y **Puerto del Este** (frente naga), **Argent**, Academia de Arcania, Muralla de Ascaria (5912) y segunda Muralla (3761).
- **Aegroum**: Ranmont (Palacio Real de 6 niveles: 4 plantas + prisión + catacumbas reales), **Boundington** (Casco Antiguo, Barrio Militar, Barrios Altos, Distrito Comercial, Surysal, Pico Dragón, Barriada), **Doomsbury**, **Perishton**, Bosque de Ventus Oscuro, Santas Montañas (Templo de los Perdidos).
- **Bastrea**: Numandum, Ection (universidad), **Wolfwater** (campamento militar), Bijyll/Beijyll, Corazón de Bruma (Ciudad Flotante, 4963), Llanuras/Riberas del Bast.
- **Udrax**: Bomengrid, **Rhenor** (Biblioteca de Runas, 6623), Montes Forja.
- **Ecla**: Ekait (universidad).
- **Gongorguma**: Guskedor, **Qethatos** (ciudad costera + universidad), Monte Kurn, Montaña/Gran Montaña de la Ceniza, Plaza Umbral.
- **Ashye**: Fortaleza de Umbralia, Red Subterránea de Umbralia, Praderas del Oro.
- **Ayashii**: Himetsu, Himetsumota, Jardines de las Llamas Eternas, Cascadas Susurrantes, Costas del Ocaso.
- **Aeon**: Isla de la Convergencia, Llanuras del Eco, Monolito Cristalino, Santuario de Cristal, Cavernas de la Desolación, Faro de la Eternidad.
- **Tabaxi**: Pantanos de Nezhura, Ruta de las Escamas, Llanuras de Shaarak.
- **Mistarium**: Neblinia, Isla de la Penuria, Torre de los Vientos Eternos, Bosques de la Niebla, Costa de Aeltheris.
- **Bosmurg / Choubar / Esmua**: Teshkorr, Llanuras Septentrionales, Monte Thrynn, Bosques de Shaerak, valle de Gorukor; Montañas Althir, Bosque de Kelryth, Llanuras de Ulda, Río Ardiente, Agujas de Zhyrrak; Gran Estepa, Llanuras Ardientes, Plaza de los Cuernos.
- **Wulcain**: Monte Wulcain.

### 2.e Clanes por nación (dato inédito, `Ciudades y sus elementos.xlsx` Hojas 2–3)
El xlsx registra sistemáticamente los **clanes** implicados en cada evento. Muestra consolidada:
- **Humanos fundacionales**: Orh, Ilmar, Bastian (+ Yslur en [TL], ausente aquí).
- **Enanos de Udrax**: Martelys, Runescribe, Bronzeborn, Silvershield.
- **Goblins de Gliaddokx**: Zantox, Fliark, Gorthak, Bleek (8797) → después Zorkul, Ugrak, Morvok, Gnarz (7692); más tarde Espiral de Jade, Dedos Ágiles.
- **Yokai de Ayashii**: Shinken, Kazehana, Hikarikami, Honoun (los cuatro clanes fundacionales); después Sasayaki, Faiasamitto, Eien, Kokuta.
- **Hombres Gato de Bosmurg**: Cara Loca, Zarpa Sombría, Colmillo Dorado, Puño de Hierro, Piel Oscura, Hachadura.
- **Aarakocra de Choubar**: Ventobélico, Ventoférreo, Alas Escarlata, Viento de Sombras, Kraahs, Hierro Oscuro, Óxido Hambriento, Puño Reforzado.
- **Nagas de Tabaxi**: Diente de Niebla, **Espiral de Marfil**, Perlas Oscuras, Espiral de Jade, Colmillo de Sal, Escama Azul.
- **Orcos de Gongorguma**: Puño de Lava, Forjahierro.
- **Minotauros de Esmua**: **Gran Estepa**, Roca Partida.
- **Mistarium (tieflings)**: Neblinia, Filo Sombrío, Umbral.
- **Ashye**: Sagas, Drow, Shardmind, Shadar-kai (los 4 juntos = "Consejo de las Sombras", 4000).
- **Elementales de Aeon**: Ignífera, Aqualis, Aerium, Terranox; luego Pyrestorm, Hydrelith, Terraforge, Aerisong; luego Flamígero, Oceánico, Roca Viva, Aliento Celestial. **Tres generaciones de nombres para las mismas 4 corrientes elementales** (fuego/agua/tierra/aire).
- **Bastrea**: **Freixes de Foc, Guillemot Velmont, Sabater Caballé** (confirma 3 de los 10 apellidos nobles de [CRON], ya presentes en 6002); después Vientoblanco, Puño de Roble, Caminasol, Lanza Carmesí; Naeltharion, Aquatris, Vigilantes de Bruma.
- **Ascaria**: Mirathis, Kaelid, Eldrayne (5912); Arclight, Rivenstone, Arbolar (4914); Puños de Bronce, Guardia de la Espada Roja.
- **Dracónidos de Wulcain**: Escama Carmesí, Fuego Ancestral, Forjahierro, Kraahs.
- **Aegroum**: Bronce de Aegroum, **Forjamundo**, "Actual Perdidos" (3105) — ⭐ el xlsx identifica el clan de Aegroum que **se convertiría en los Perdidos**.

---

## 3. Genealogías y personajes

Fuente: `Plantillas/Árboles genealogicos.xlsx` (3 hojas: árbol, resumen por medio siglo, cronología detallada). **Todas las fechas en `d.e.`, rango 729–900.**

### 3.a Dinastía **Voskezorik** (Ascaria)

| Reinado | Vida | Rey | Vida | Reina |
|---|---|---|---|---|
| 732–759 | 712–759 | **Seigebert Voskezorik "El Inmortal"** | 721–770 | **Feiana Voskerozik** |
| 770–807 | 749–807 | **Lambert Voskezorik** | 743–805 | **Xeamia Matajia** |
| 807–835 | 772–835 | **Gyrid Voskezorik "El Renacido"** | 775–829 | **Senoiga Voskezorik** |
| 836–864 | 800–864 | **Nerseh Voskezorik "El Unificador"** | 805–852 | **Alda Voskezorik** |
| 865–880 | 822–882 | **Irumka Voskezorik "El Santo"** | — | — |

Ascendencia: **Gon Voskezorik × Natia (noble)** → Seigebert. Seigebert × Feiana → **Lambert**. Lambert × Xeamia → **Gyrid**. Gyrid × Senoiga → **Nerseh**. Nerseh × Alda → **Irumka**. Aparece también **Xenia** en la generación de Gyrid.

### 3.b Dinastía **Matajia** (Aegroum)

| Reinado | Vida | Rey | Vida | Reina |
|---|---|---|---|---|
| 729–762 | 709–762 | **Tanush Matajia "El Derrotado"** | 712–762 | **Arsinde Matajia** |

Hijos de Tanush × Arsinde: **Xeamia Matajia**, **Jahan Matajia**, **Gildanh Matajia**. Solo Xeamia sobrevive (762) y se casa con Lambert (770). **La dinastía Matajia se extingue como casa reinante y se funde en la Voskezorik.**

### 3.c Linaje **Venides** (héroes, no reyes)
**Venides Hijo de Nefaria × Verina Hija de Perishton** → **Villena Hijo de Finesaux** → **Edul**.
- 729: Venides salva a la familia real de Aegroum.
- 780: se casa con Verina, nace su hijo.
- 820: Venides se consagra **alto general** de Ascaria.
- Villena aparece como PJ/NPC dirigiendo el asedio de Ranmont en `Campañas/Ciudades.xlsx`, y "Edul el Herrero Reformado" figura como personaje clave del Palacio de Ranmont.
- ⭐ **Convención onomástica**: los Venides toman por "apellido" el nombre de la madre o de la ciudad (*Hijo de Nefaria*, *Hija de Perishton*, *Hijo de Finesaux*). Es un sistema de filiación distinto al de las casas reales.

### 3.d Cronología dinástica (Hoja "Sheet1", años `d.e.`)
729 Venides salva a los Matajia · 750 Seigebert "El Inmortal" · **762 caída y muerte de Tanush y Arsinde; muere toda la familia real de Aegroum en Doomsbury salvo Xeamia** · 770 Xeamia se casa con Lambert en Finesaux · 780 nace el hijo de Venides · **800 Batalla de Ranmont y Caída de Ranmont** · **801 Primera Asamblea Templaria (la celebran los Perdidos)** · 806 Gran Batalla de Doomsbury (gana Aegroum) · 814 Asedio de Argent · 817 Batalla Nocturna de Velours · 820 Venides alto general · **822 Carga de los Muertos + Gyrid "El Renacido"** · 850 Nerseh "El Unificador" · **860 surgimiento de los Adalides de Gurkazaal en Aegroum** · **870 Gendrazul** · 880 Ranmont pasa a ser segunda capital de Ascaria · 885 Segundo Asedio de Ranmont · 890 Irumka "El Santo" · **900 Templo del Sol**.

### 3.e Otros personajes del corpus no listados en [CRON]
De `Campañas/Ciudades.xlsx`: **Rey Aldon** y **Reina Lysandra** de Aegroum (padres de Xeamia en la ficha de palacio — ⚠️ contradice a Tanush/Arsinde del árbol genealógico), **Duke/Consejero Nefaros**, **Sir Galdrin**, **Capitán Rhomar**, **Xila** (alcoba oculta), **Riu'jin** (líder de los Perdidos en Ranmont, "maestro de la magia de Gurkazaal"), **Hechicera Irae**, **Capitán Joros** (resistencia de Doomsbury), **Anara la Suma Sacerdotisa** y **Eldon el Vigilante** (Santuario de Doomsbury), **General Roger de Flor** y **Sargento Torgrim** (Bastrea, Wolfwater), **Lady Helena**, **Dalgor** (mercader), **Skila** (tabernera ex-aventurera), **Madame Luisarda**, **Aigren** (herrero), **Elderath** y **Drakkin** (Bosque de Ventus Oscuro).

---

## 4. Astrología y sistema de elementos

Fuentes: `Plantillas/Astrologia.docx` ("Horòscop d'Egaroth – Les 12 Cases Astrals dels Déus", en catalán) y su versión ampliada `Personajes Hombrew/Astrologia.docx` ("Llibre de les Llunes d'Egaroth", con virtudes/defectos/propósitos vitales por mes).

### 4.a Las tres lunas
| Luna | Dominio | Ciclo |
|---|---|---|
| **Thaleia** | Agua, magia, equilibrio | 30 días |
| **Varn** | Aire, caos, guerra | 45 días |
| **Lys** | Fuego, alma, visión | 90 días |

La **Triple Unión Lunar** (las tres coincidiendo) ocurre **una sola noche al año**, idealmente en el Mes de las Estrellas (diciembre), "com a culminació astral del calendari". ⭐ Gancho narrativo de primer orden y **completamente ausente de [CRON]**.

### 4.b Los 12 meses, su deidad regente y su elemento
| Mes | Lunas | Elemento | Deidad regente | Signo |
|---|---|---|---|---|
| Enero — Escarcha | Thaleia+Varn | Vegetación | **Deitrok** (intercambio, artesanía, normas sociales) | Teixidors de l'Equilibri |
| Febrero — Siembra | Thaleia+Lys | Tierra | **Egaroth** (naturaleza, magia, creación) | Arrelats del Coneixement |
| Marzo — Agua | Thaleia | Agua | **Sofía** (conocimiento, verdad) | Savis Silents |
| Abril — Oráculo | Varn+Lys | **Rayo** | **Morfeo** (sueño, imaginación) | Somiadors del Llamp |
| Mayo — Sol | Thaleia+Varn | Vegetación | **Eros** (deseo, inspiración, atracción) | Cors Desperts |
| Junio — Cosecha | Thaleia+Lys | Tierra | **Chronos** (tiempo) | Guardians del Cicle |
| Julio — Hierro | Varn | Aire | **Destrucción** (guerra y separación) | Fills del Tall |
| Agosto — Agonía | Thaleia+Varn | Vegetación | **Envidia** (corrupción, posesión) | Corruptibles Luminosos |
| Septiembre — Tierra | Lys | **Fuego** | **Vida** (nacimiento, familia, semilla) | Fundadors |
| Octubre — Aullidos | Thaleia+Lys | Tierra | **Egos** (personalidad y **máscara**) | Doble-Rostre |
| Noviembre — Sombras | Varn+Lys | Rayo | **Muerte** (final y olvido) | Passadors dels Límits |
| Diciembre — Estrellas | las tres | **Energía Suprema** | **Sofía** (conocimiento supremo) | Nascuts de la Nit Luminosa |

### 4.c Consecuencias de sistema (relevantes para el canon)
1. **Hay 7 elementos astrales, no 4**: Agua, Aire, Tierra, Fuego, **Vegetación**, **Rayo** y **Energía Suprema**. [CRON] solo maneja Agua/Fuego/Tierra/Aire + "Aire Frío". Vegetación y Rayo son categorías elementales de pleno derecho aquí, y coinciden con las clases homebrew **Devoto del Bosque** y **Nacido del Trueno**.
2. **El elemento del mes no coincide con el nombre del mes** (Septiembre "Mes de la Tierra" tiene elemento **Fuego**; Marzo "Mes del Agua" sí es agua). Puede ser deliberado (nombre litúrgico vs. elemento real) o un error de la tabla. **Necesita decisión del autor.**
3. **Egaroth aparece como "deessa"** (femenino) de la naturaleza y la magia — ⚠️ contradice frontalmente a [CRON], donde Egaroth es el **Padre Elemental** masculino y durmiente, y donde *no rige* ningún mes porque duerme.
4. **Egos rige octubre y su dominio es "la personalitat i la màscara"** — es decir, **el sistema astrológico presupone a Egos activo**, no fracturado en Gurkazaal. Todas las fichas de raza le asignan también subrazas patrocinadas. Si la fractura de Egos es canon [CRON], el horóscopo y las fichas raciales describen un estado *anterior* a ella, o hay contradicción.
5. **"Destrucción" ocupa el lugar que [CRON] llama "Guerra"** (dios de la guerra). Probablemente el mismo Creador con dos nombres — hay que unificar.
6. **Deitrok = "Deithrok" de [CRON]**, y aquí sí tiene dominio definido: **intercambio, artesanía y normas sociales**. Deja de ser el "¿conocimiento oculto?" con interrogante.
7. **No aparece "Deseo" como Creador separado de Eros** en la astrología: Eros absorbe deseo, pasión e inspiración. Contradice la resolución #2 de [CRON] (Eros y Deseo como dos Creadores distintos).
8. **Sofía rige dos meses** (marzo y diciembre) — es el único Creador con dos casas, lo que la posiciona como la deidad estructuralmente más importante del calendario.

### 4.d Difusión cultural del sistema
"Aquestes guies s'utilitzen a les grans ciutats i temples de les cultures **humanes**, però també han estat adoptades, amb adaptacions locals, pels **Aarakocra de Choubar**, els **Nagas de Tabaxi**, els **Hombres Gato de Bosmurg**, i fins i tot pels **Dracònids de Wulcain**, que veuen en les conjuncions lunars una revelació dracònica." Los consejos espirituales los transmiten **els Oracles de Nimandun** (= Numandum, Bastrea) **i els Somiadors de Arcania** (= la Academia de Arcania de Ascaria). Fuente: `Plantillas/Astrologia.docx`.
→ Esto establece **dos instituciones religioso-astrológicas** con sede identificada: Bastrea y Ascaria son las potencias astrológicas del mundo. **Ausente en [CRON].**

---

## 5. Clases homebrew y su gancho de lore

Corpus: 24 fichas en `Plantillas/Classes/` + `Campañas/Clase_ de Feriante.docx`. Aviso importante: **la inmensa mayoría de fichas son puramente mecánicas y no citan nación ni raza**. Solo dos anclan explícitamente a una facción. El resto se conecta por implicación cosmológica.

| Clase | Anclaje narrativo explícito | Lore que presupone |
|---|---|---|
| **Bardo de la Voz Divina** | ⭐ **"Son soldados de Ascaria, donde entrenan"** — único anclaje nacional explícito del corpus | Existe una academia militar-religiosa de bardos en Ascaria |
| **Devoto del Bosque** | ⭐ **"paladín de Gaia"**, canaliza "las fuerzas de Gaia" | ❗ **"Gaia" no aparece en ningún otro documento del proyecto.** Ni en el panteón de [CRON] ni en la astrología. Deidad huérfana: o es sinónimo de Egaroth (que en la astrología sí es "deessa de la natura") o es un Creador no documentado |
| **Apóstata** (3 versiones: 1ª y 2ª iteración + subclase de Brujo en `Hoja de cálculo sin título.xlsx`) | "Traidor a la luz, **ex-guerrero sagrado** que ha sucumbido a las tentaciones oscuras"; magia de sangre, maldiciones, subyugación; paga hechizos con puntos de vida | Presupone una **orden de guerreros sagrados de la que se deserta** (¿Templarios? ¿Guardianes de la Luz de Aegroum?) y un pacto con "fuerzas oscuras" identificables. Encaja con Gurkazaal/los Perdidos pero **no se dice** |
| **Renacido** | "Ha hecho un **oscuro pacto** para obtener poder a cambio de su propia vida"; convierte vida en escudo | ⚠️ Choca con el título de **Gyrid Voskezorik "El Renacido"**, que en la genealogía gana el nombre por **sobrevivir a la Carga de los Muertos** (822 d.e.), no por un pacto. O la clase nace de él, o hay colisión de nombres |
| **Saltador de Planos** | Pícaro con pacto que "atraviesa **planos de existencia**", arrastra enemigos a "**dimensiones oscuras**", dagas malditas con daño necromántico | ❗ **Presupone cosmología multiplanar concreta y no documentada.** Concuerda con: "Elementales **cierran rutas interplanares**" (6719, `Ciudades y sus elementos.xlsx`), el **Reino de las Almas** al que entra Ascaria en [CRON], el **Reino Astral** de donde se exilia Gurkazaal, y los **Poseídos del Vacío**. Hay al menos 4 planos implícitos sin mapa cosmológico |
| **Redimido** | "Antiguo brujo que ha abandonado las artes oscuras… **sigue ciegamente los designios de su dios**" | Espejo del Apóstata. Presupone conversión religiosa institucionalizada |
| **Caballero Templario** | Guerrero de fe, "puede ser tanto un tirano como un santo"; invoca **Camaradas de Luz**; en `Hoja de cálculo sin título.xlsx` es **subclase de Paladín** con "control mágico de la Luz + habilidades de asedio" | Conecta con la **Primera Asamblea Templaria (801 d.e.)** del árbol genealógico y con el **Templo del Sol (900 d.e.)**. Es la clase institucional del culto a la Luz |
| **Señor de los Astros** | Canaliza "estrellas, sol y luna" para "**afectar el destino**", invoca luz y oscuridad, manipula el tiempo | Es la clase-avatar del sistema astrológico: las 3 lunas y los Oráculos de Numandum |
| **Bardo de las Cuatro Estaciones** (2 iteraciones) | Canaliza invierno/primavera/verano/otoño, "**altera el clima**" | Ligado a la **Marcha de las Cuatro Estaciones** de los Hombres Gato Cara Loca (Bosmurg). El PJ pregenerado que usa esta clase es un **Elfo de Ecla (Elfos de las Estrellas)**, no un felino |
| **Nacido del Trueno** | "Bendecido **o maldecido** por el poder del trueno y la tormenta" | Corresponde al elemento astral **Rayo** (Varn+Lys, meses de Abril y Noviembre, deidades Morfeo y Muerte) |
| **Furia de los Creadores** | "Bendecido por poderes divinos… **elegido por fuerzas superiores para cumplir una misión sagrada**" | Es la única clase que nombra a los Creadores como colectivo. Casa con los **"Designios de los Creadores"** que [CRON] sitúa en la Era Oscura |
| **Inventor** | "Genio caótico que combina magia e ingeniería"; máquinas de guerra, explosivos, **robots controlados**, cañones de energía | Encaja con la línea tecnológica ascariana de [CRON] (válvula de vapor 649 b.f., tanque 539, robot con núcleo de maná 459) y con los **goblins explosivos** de la 3ª Cruzada. PJ pregenerado: **Gnomo del Subsuelo** → sugiere que los gnomos son el pueblo tecnólogo |
| **Francotirador** | "Pícaro + artificiero", **armas pesadas de proyectiles mecánicas** | Misma línea tecnológica. Anacrónico para la era de las Cruzadas: implica que la tecnología ascariana ya circula |
| **Portador de Plagas** | Nigromancia + naturaleza corrupta, hordas de insectos, cadáveres | Encaja con la **Primera Gran Maldición** y con la nigromancia de Aegroum/Ascaria. PJ pregenerado: **Enano Forjador** (elección chocante) |
| **Titiritero Maldito** | Brujo con pactos oscuros que manipula cuerpos con hilos | PJ pregenerado: **Humano de Aegroum** ⚠️ — un aegroumiano (pueblo de la Luz) con pacto oscuro = los **Perdidos** en términos de clase |
| **Bardo de la Cuerda Rota** | "Ha **roto los lazos con lo divino o lo puro**" y firmado pactos oscuros | Contrapartida caída del Bardo de la Voz Divina (ascariano) |
| **Hermano de Sangre** | "Furia sagrada", "la **llama de luz que habita en ellos**" | Presupone una hermandad/orden de portadores de luz interior. Sin nación asignada |
| **Hoja Venenosa** | "**Antiguo druida** que ha dejado las formas tradicionales" para el asesinato | Presupone una tradición druídica organizada de la que se desvía. PJ pregenerado: Mediano del Bosque |
| **Cambiaformas** | Cazador que muta el cuerpo | PJ pregenerado: **Elfo de las Sombras** |
| **Furia Salvaje** | Bárbaro-druida, espíritus animales | Sin anclaje. Compatible con Hombres Gato / Hombres Lagarto |
| **Señor de las Armas** (2 iteraciones) | Invoca "**armas divinas y reliquias perdidas de tiempos antiguos**", cada una "con su propia historia" | ❗ Presupone un **catálogo de armas legendarias con historia propia** que no existe escrito. PJ pregenerado: **Tiflin de la Llama Oscura** |
| **Guerrero del Puño Cortante** | Combate dual con dos espadas grandes | Sin lore |
| **Mago del Paso Ligero** | Mago-pícaro de moralidad ambigua | Sin lore |
| **Feriante** | ⭐ "Forzudos de feria", gran bigote, espectáculo con fuego, "**Magia Confusa**" (efectos aleatorios) | Presupone una **cultura de ferias y circo ambulante**. Nota: el **Bazar del Horizonte** de Boundington se describe como "comunidad gitana y mercaderes" (`Campañas/Ciudades.xlsx`) — es el gancho cultural más probable |
| **Guardián Elemental** | Subclase de Mago (`Hoja de cálculo sin título.xlsx`): "control mágico elemental + conexión rúnica", profesiones de alquimista/boticario | ⭐ Casa exactamente con los **"Guardians de l'Element" de Bastrea** de la ficha estética, con su cinturón de cuatro medallones. Es la clase nacional de Bastrea |
| **Gladiador Arcano** | Subclase de Hechicero: esgrimista imbuidor de armas, alineamiento caótico-neutral | Sin nación |

### 5.b Estructura de clases del sistema
`Plantillas/Hoja de cálculo sin título.xlsx` (Hoja "Paladin") revela la arquitectura: **8 clases base** (Paladín, Guerrero, Mago, Hechicero, Brujo, Pícaro/Bardo, Bárbaro, Artificiero) → **subclase** → **"SUBCLASE SUPREMA: a decidir y crear en base a todas las decisiones tomadas por el personaje"**. Cada clase tiene además **MORALIDAD** y **PROFESIONES** asignadas — es decir, la clase es también un rol social, no solo de combate. Los **niveles de subclase** son 4-6-9-12-15-20 frente a 3-5-7-10-13-16-19 de las clases base.
`Niveles Clases y Razas.xlsx` documenta además un eje de progresión llamado **"Nivel Creador"** (Creador 1 a Creador 5) que suma a las habilidades — ⭐ implica que **el vínculo con un Creador se nivela mecánicamente**, lo cual es lore: la devoción es progresiva y medible.

---

## 6. Contradicciones y huecos frente a [CRON]

### 6.a Contradicciones duras (requieren decisión)

| # | Tema | [CRON] dice | El corpus dice | Fuente |
|---|---|---|---|---|
| C1 | **Capitales** | Ecla→Thorn, Gliaddokx→Vurn, Bosmurg→Teshkorr, Esmua→Taurengrad, Ashye→Umbralia, Tabaxi→Tabaxi | Ecla→**Umedan**, Gliaddokx→**Havar'gruztak**, Bosmurg→**Tor'k Hazar**, Esmua→**Saif-l'sa**, Ashye→**Zathor'aetz**, Tabaxi→**Naka't-ol** | `Campañas/Ciudades.xlsx` H2 |
| C2 | **Fechas fundacionales** | Gran Pacto 8865; Goblins 8837; Juramento Llamas Eternas 8853; Guardián del Abismo 8739; Rebelión Corrientes Oscuras 8762; Purga de las Tormentas 8786; Marcha 4 Estaciones 8790; clanes Yokai ~9070 | **8765; 8797; 8200; 8500; 8300; 7600; 5809 (¡y duplicado como 7800 en la misma celda!); 7993** | `Ciudades y sus elementos.xlsx` H2/H3 |
| C3 | **Fragmentación humana** | "Día de los Cuatro Soles" 8954, **cuatro** clanes: Orh, Yslur, Bastian, Ilmar | "Fragmentación inicial Clanes humanos" **8964**, **tres** clanes: Orh, Ilmar, Bastian (falta Yslur). Lugar: **El Campo de Thyrene** | `Ciudades y sus elementos.xlsx` H2 |
| C4 | **Calendario de las Cruzadas** | Batalla de Ranmont 1916 b.f.; 2º Asedio 1874 b.f.; Adalides 1885–1873 | Batalla de Ranmont **800 d.e.**; 2º Asedio **885 d.e.**; Adalides **860 d.e.**. El desfase b.f.↔d.e. **no es constante** (1116 vs 989 años), luego una de las dos series está mal | `Árboles genealogicos.xlsx` Sheet1 vs [CRON] Era 4 |
| C5 | **Padres de Xeamia** | (no los nombra) | **Árbol genealógico**: Tanush Matajia + Arsinde Matajia. **`Campañas/Ciudades.xlsx`**: "Rey Aldon, Reina Lysandra, Xeamia" en las habitaciones reales de Ranmont. Dos parejas distintas | `Árboles genealogicos.xlsx` vs `Campañas/Ciudades.xlsx` |
| C6 | **Gendrazul** | Demonio "de la lengua bífida" contactado por Dire Hijo de Killville | **"Gendrazul se convierte en la segunda capital de Aegroum bajo el gobierno de los Adalides"** (870 d.e.) — es un **topónimo** | `Árboles genealogicos.xlsx` Sheet1 |
| C7 | **Egaroth** | Padre Elemental, masculino, **dormido**, no rige nada | "**Egaroth — deessa** de la natura, la màgia i la creació", **regenta el mes de la Siembra** | `Plantillas/Astrologia.docx` |
| C8 | **Egos** | Fracturado en Gurkazaal; "ya no actúa como Creador activo" | Rige el mes de Aullidos como dios "de la personalitat i la màscara"; patrocina 5 subrazas vivas (Dracónidos de Fuego, Enanos Forjadores, Hombres Lagarto del Pantano, Minotauros de la Llanura, Orcos Dramáticos, Guardianes de la Luz, Hechiceros Oscuros) | `Astrologia.docx` + todas las fichas `Razas/` |
| C9 | **Eros vs Deseo** | Dos Creadores distintos (Deseo→Displicencia) | Solo **Eros**, "déu del desig, la inspiració i l'atracció". **Deseo y Displicencia no existen** en astrología ni en fichas raciales | `Astrologia.docx` |
| C10 | **Guerra vs Destrucción** | Creador "Guerra" | "**Destrucció**, déu de la guerra i la separació" | `Astrologia.docx`, `Razas/Minotauros.docx` |
| C11 | **Ascaria** | Imperio tiránico | La ficha racial la describe **post-caída**, humilde y justiciera; ciudades en mal estado de conservación | `Razas/Humanos de Ascaria.docx`, `INTRODUCCIÓN…` |
| C12 | **Orcos** | "Sabios + guerreros" | Pacíficos, sensibles, **físicamente torpes**, artistas de teatro | `Razas/Orcos de Gongorguma.docx`, `INTRODUCCIÓN…` |
| C13 | **"Marcha de las Cuatro Estaciones"** | 8790 b.f., fundación de Bosmurg | El xlsx la fecha en **5809** y la titula "**Regreso** de los Cara Loca", posterior a una "**Caída de los Cara Loca**" en **7632**. Es un ciclo caída→exilio→retorno, no una fundación | `Ciudades y sus elementos.xlsx` H2 |
| C14 | **Número de reinos** | 13 | **17 capitales listadas + Ostad = 18 territorios**. Y **Knehapnest no figura en ninguna lista ni en el mapa** | `Campañas/Ciudades.xlsx`, `Ciudades y sus elementos.xlsx` H4 |

### 6.b Huecos nuevos que abre el corpus
1. **Nocturnsea**: nación con capital (Ciudad de Grytoz) y presencia en el mapa (bloque norte del continente oriental), pero **ningún evento histórico, ninguna raza asignada, ninguna descripción**. Aparece además como topónimo de un evento de Aeon ("El azote a los hijos de las Guerras del Fuego", 3878). Es el hueco más grande.
2. **Ostad**: territorio propio en el mapa, dos eventos fechados (7932 primeros pueblos, 6821 primer templo, en "Pantanos Profundos"), asociable a los Hombres Lagarto. Sin capital, sin ficha.
3. **Leyenda de colores/elementos del mapa (Hoja 5)**: sin ella, la asociación ciudad↔elemento prometida por el nombre del archivo es irrecuperable.
4. **Knehapnest y los Gnomos**: [CRON] lo lista, el corpus no lo conoce. Y no hay ficha racial de gnomos pese a existir un PJ "Gnomo del Subsuelo".
5. **Tieflings sin ficha racial** pese a ser una de las razas más importantes del timeline.
6. **Gaia**: deidad nombrada solo en `Clase_ Devoto del Bosque.docx`.
7. **Cosmología planar**: al menos Reino Astral, Reino de las Almas, "dimensiones oscuras", el Vacío/Abismo, y "rutas interplanares" cerradas por los Elementales en 6719. Nadie ha escrito el mapa de planos, pero tres clases y una raza dependen de él.
8. **Catálogo de armas legendarias** que la clase Señor de las Armas presupone.
9. **Vampiros**: [CRON] les da fundación y linaje (Vincalp, Pacte de les Ànimes) pero **no existen ni como ficha racial ni en ningún evento del xlsx**.
10. **Elementales**: cuatro corrientes (Ignífera/Aqualis/Aerium/Terranox) con tres generaciones de nombres de clan y eventos propios (Guerras del Fuego 5961, Orden del Cristal 5662, Orden Secreta del Cristal de la Vida 3681, Faro de la Eternidad 4705), pero sin ficha racial y sin conexión con los "hijos de Shorm Aergan" de [CRON].

### 6.c Recomendación de prioridad
1. **Decidir qué serie de fechas es canónica** (C2/C3/C13): el xlsx cubre el hueco de 5500 años que [CRON] declara vacío, pero solo sirve si se acepta su cronología, que reescribe las fundaciones. Es la decisión de mayor impacto del proyecto.
2. **Fijar el desfase b.f.↔d.e.** (C4) usando el árbol genealógico como ancla, ya que es el documento más internamente consistente del corpus.
3. **Aceptar la lista de 17 capitales** y reescribir la tabla de MAPA POLÍTICO de [CRON].
4. **Reconciliar el panteón** (C7–C10): la astrología y las 40 asignaciones de subraza son un sistema completo y coherente entre sí; la cosmogonía de [CRON] es otro. Elegir cuál manda.

---
*Informe elaborado sin añadir información inventada. Toda afirmación es trazable al archivo citado.*
