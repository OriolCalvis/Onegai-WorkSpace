# GUÍA DEL PROYECTO ONEGAI — dónde está cada cosa

> **Para qué sirve esto.** El proyecto son ~18.500 archivos repartidos en cinco carpetas que hacen cosas muy distintas. Esta guía te dice **dónde mirar según lo que busques**, no cómo está organizado el disco. Si buscas por carpeta, salta al mapa del final.
>
> **Lo primero que hay que saber:** el material narrativo real son **unos 200 archivos**. Todo lo demás (17.000 y pico) es código de las dos herramientas y recursos gráficos de terceros.

---

# PARTE 1 — BUSCO UNA COSA CONCRETA

## "¿Qué pasó en el año X?"

**→ `Onegai_Cronologia_Maestra.md`** (raíz)

El esqueleto completo: 285 entradas, 17 eras, de la Cosmogonía al 300 a.f. Cada evento tiene su definición, sus fuentes y un ID estable. Es el único sitio donde toda la historia está en un mismo orden. Empieza siempre aquí.

Si el evento tiene ➡️, el documento que lo desarrolla está indicado en la propia entrada. Si tiene ⬜, aún no existe.

## "¿De dónde salen los dioses? ¿Cómo se creó el mundo?"

**→ `Onegai/THE ONEGAI PROJECT/THE ONEGAI PROJECT/Gran Compendio de los Creadores.docx`**

La biblia cosmológica. Nacimiento de Egaroth, Chronos, Sofía, Vida y Muerte, Egos, Deseo y Envidia, y la corrupción que produce a Gurkazaal. También el Gran Pacto de 8865 b.f. y la mayoría de fundaciones raciales.

**Junto a él, dos documentos que lo completan:**

- **`Estructura narrativa del Gran Compendio.docx`** — la versión esquematizada, con los eventos separados por fichas. Más fácil de consultar, menos matizada.
- **`Campañas/Ayashii.docx`** — ⭐ **el documento más importante que nadie estaba leyendo.** Es el único sitio donde se cuenta el asesinato de **Mirathis** y la escisión de Egos en *Egos el Justo* + *Gurkazaal*. También las seis eras yokai fechadas y el origen del volcán Himetsu.
- **`Plantillas/Astrologia.docx`** — el horóscopo de Egaroth: doce meses, doce deidades regentes, las tres lunas (Thaleia, Varn, Lys) y la Triple Unión Lunar. Es la fuente del panteón ampliado (Deitrok, Morpheo, Destrucción).

## "¿Qué razas hay y de dónde vienen?"

Tres capas, y conviene ir en este orden:

| Qué quieres | Dónde |
|---|---|
| El resumen de todas las naciones y razas de golpe | `infologia/DOCUMENTOS PARA JUGAR/INTRODUCCIÓN NACIONES Y RAZAS.pdf` |
| La ficha detallada de una raza concreta | `THE ONEGAI PROJECT/Plantillas/Razas/` — 21 fichas `.docx`, una por raza |
| Personajes de ejemplo ya construidos con esa raza | `THE ONEGAI PROJECT/Plantillas/Personajes Hombrew/` — 13 fichas de nivel 5 |

**Ojo:** faltan fichas de **Tieflings, Gnomos, Elementales y Vampiros**, que son centrales en el lore. Y **Medianos, No Muertos y Poseídos** tienen ficha pero no aparecen en la cronología.

## "¿Dónde está el mapa? ¿Qué ciudades hay?"

**El mapa real está escondido dentro de una hoja de cálculo.**

**→ `THE ONEGAI PROJECT/Ciudades y sus elementos.xlsx`**

- **Hoja 4** = el mapa del mundo, una rejilla de 26×25 celdas con el nombre del reino en cada celda.
- **Hoja 5** = la misma rejilla con colores (elementos/biomas) y marcadores `C` e `I`. ⚠️ **La leyenda de colores no está escrita en ninguna parte** — solo tú sabes qué color es qué elemento.
- **Hojas 2–3** = las capitales, las ciudades secundarias y los **clanes por nación**, evento a evento.

**→ `Campañas/Ciudades.xlsx`** — la lista canónica de las 17 capitales. Es la que manda cuando hay discrepancia.

**Mapas dibujados (imágenes):** `infologia/Mapas/` — 30 mapas generados, más los tres *trivu* humanos (Orh, Ilmar, Bastian, Yslur). Los mapas tácticos de mesa están en `PARTIDA 1/` (Boundington: casco antiguo, base militar, las afueras, con versiones GM y jugador, más los `.psd` editables) y `PARTIDA 2/` (Catacumbas, Templo de Sofía, Templo de Chronos).

## "¿Quién es este personaje? ¿De qué familia?"

**→ `THE ONEGAI PROJECT/Plantillas/Árboles genealogicos.xlsx`**

Las dos dinastías humanas completas con años de reinado:
- **Casa Voskezorik (Ascaria):** Gon → Seigebert *el Inmortal* → Lambert → Gyrid *el Renacido* → Nerseh *el Unificador* → Irumka *el Santo*
- **Casa Matajia (Aegroum):** Tanush *el Derrotado* + Arsinde → Xeamia, Jahan, Gildanh

Para personajes sueltos, la cronología maestra los cita en la entrada del evento donde aparecen.

## "¿Qué campañas hay listas para jugar?"

**→ `THE ONEGAI PROJECT/Campañas/`**

| Carpeta | Qué contiene |
|---|---|
| `LongSide Quests/1 Primera Cruzada/` | *Huida de Ranmont Xeamia* (26.000 palabras, la más desarrollada del proyecto) y *El Asedio de Ranmont* |
| `LongSide Quests/5 Quinta Cruzada/` | *Expedición a la Isla Maldita* ⚠️ **mal archivada** — no tiene nada que ver con la Quinta Cruzada canónica |
| `Campañas One Shoot/Primera Cruzada/` | *Azotamentes en Perishton*, *Corrupción en Ecla*, *El Infierno en Argent* |
| `Campañas One Shoot/Tercera Cruzada/` | *Campaña 01 Goblin* |
| `Campañas One Shoot/Eventos historicos/` | *Sombras del Gran Pacto* (I y II), *Guerra de los Clanes: Sangre en Thyrene* |
| `Canon/` | *La Matanza de Boundington* — la campaña canónica de Aegroum |

**Las carpetas `2 Segunda Cruzada`, `3 Tercera Cruzada` y `4 Quarta Cruzada` están vacías.** Y las cruzadas 9ª a 14ª no tienen ni una línea en todo el proyecto.

También en el vault de Obsidian, ya troceadas en fichas jugables: `Onegai/OnegaiTimeLine/Campanyes/` — *Campanya de Itron* (8495 b.f.), *Tren hacia el Himetsu* (4592 b.f., con puzles y eventos día a día) y *Boundington*.

## "¿Cómo funcionan las clases y el sistema?"

**→ `THE ONEGAI PROJECT/Plantillas/Classes/`** — 24 clases homebrew en `.docx`.

Hay dos iteraciones de balanceo: la carpeta `Classes Balanceadas 2nd Iteraccio` **sustituye** a la 1ª para Apóstata, Bardo de las Cuatro Estaciones y Señor de las Armas. (⚠️ La 2ª del Señor de las Armas tiene el encabezado corrompido con texto tecleado por accidente — revísala antes de descartar la 1ª.)

**Hojas de datos:**
- `Plantillas/Hombrew Class.xlsx` — tabla de todas las clases
- `Plantillas/Niveles Clases y Razas.xlsx` — ⚠️ **roto**, buena parte de las celdas son `#REF!` / `#N/A`
- `Plantillas/5th Edition Spells.xlsx` — la base de conjuros de D&D 5e importada tal cual. **54.521 palabras, cero contenido de Onegai.** Es una quinta parte del volumen del corpus y no aporta nada al mundo.

**Fuera de sitio:** `Campañas/Clase_ de Feriante.docx` es una clase, no una campaña. Y es la más larga.

## "¿Dónde está la tabla que relaciona razas con Creadores?"

Hay **dos**, y dicen cosas distintas:

1. **`TL_OCR_completo.md`, diapositiva 270** — la tabla de *patrón*: Minotauros→Sofía, Elfos→Vida, Humanos→Guerra, Enanos→Forja, Goblins→Deithrok, H. Gato→Morfeo, Dracónidos→Chronos.
2. **`Plantillas/Campañas.xlsx`, hoja *Clases Razas*** — la tabla de *afinidad* SI/NO/NEUTRAL de cada raza con siete Creadores.

No se contradicen (una es patrocinio, otra afinidad) pero conviene no confundirlas.

## "¿Cómo funciona el calendario?"

**→ `Plantillas/Campañas.xlsx`, hoja *Años***

No son dos calendarios sino **siete corriendo en paralelo**: Before Forcast, Despertar Elemental, La Gran Alianza, Gongorguma, Neo Gongorguma, Resistencia, Ascaria, After Forcast.

⚠️ Es el único sitio donde puedes resolver el desfase de 100 años entre 2738 y 2838 — **y solo tú puedes abrirlo y leer las columnas bien alineadas.** La extracción a texto colapsa las celdas vacías y desalinea.

---

# PARTE 2 — LOS DOCUMENTOS QUE MANDAN

Si alguien llega nuevo al proyecto, esto es lo que tiene que leer y en este orden:

| # | Documento | Por qué |
|---|---|---|
| 1 | **`Onegai_Cronologia_Maestra.md`** | El mapa de todo. 285 eventos ordenados. |
| 2 | `Gran Compendio de los Creadores.docx` | La cosmología y las fundaciones raciales. |
| 3 | `Campañas/Ayashii.docx` | El mito de Egos y Mirathis. Nada más lo cuenta. |
| 4 | `TL_OCR_completo.md` | Las 274 diapositivas del timeline, en texto. La Era Oscura entera está aquí. |
| 5 | `Campañas/Los Reinos del Oeste.docx` | 45.680 palabras: la historia del continente occidental. |
| 6 | `Campañas/Lore Reinos del Este.docx` | Lo mismo para el oriental. Discrepa del Compendio en fechas. |
| 7 | `Partida de Rol avec Uri.docx` | La única fuente de las Cruzadas, el Imperio y el Cataclismo. |
| 8 | `Ciudades y sus elementos.xlsx` | El mapa y los clanes. |
| 9 | `INTRODUCCIÓN NACIONES Y RAZAS.pdf` | El resumen para jugadores. |
| 10 | `Plantillas/Astrologia.docx` | El panteón completo y el calendario ritual. |

**Los dos índices de nombres propios** (lo primero que hay que consultar para no duplicar nombres):

- **`indice_personajes.md`** — 322 personajes, entidades y cargos. 41 desarrollados, 96 mencionados, **173 son solo un nombre**, 12 contradictorios. 38 pilares del mundo (aparecen en 3+ fuentes).
- **`indice_lugares.md`** — 248 lugares: 21 territorios, 18 capitales, 74 ciudades, 79 accidentes geográficos, 45 edificios singulares, 11 lugares de otro plano. Incluye el cruce de nombres antiguos y modernos de las ciudades humanas.

**Los análisis que ya existen** (léelos antes de rehacer trabajo):

- `Onegai_Addendum_THE_ONEGAI_PROJECT.md` — las contradicciones detectadas, tus decisiones y la rectificación de la sección E6
- `Hallazgos_Nuevos.md` — el mapa reconstruido, los 7 calendarios y el vault
- `Hallazgos_Nuevos_2.md` — los trasfondos astrológicos, las tres fases del mapa humano y la hoja `Progreso`
- `Mapa_Egaroth.svg` — el mapa político del mundo, reconstruido desde la hoja de cálculo
- `informes/informe_agente1_lore.md` — panteón, cronología, cruzadas, con citas literales
- `informes/informe_agente2_razas.md` — razas, mapa político, genealogías
- `informes/catalogo_documentos.md` — ficha de cada uno de los 95 documentos
- `informes/informe_vault.md` — el vault de Obsidian
- `informes/informe_hojas_calculo.md` — las hojas de cálculo descifradas

---

# PARTE 3 — MAPA DE CARPETAS

```
Onegai 2/
├── Onegai_Cronologia_Maestra.md      ⭐ el esqueleto: 285 eventos, 17 eras
├── Onegai_Addendum_THE_ONEGAI_PROJECT.md   contradicciones + tus decisiones
├── Onegai_Cronologia_Entendida.md    el primer análisis, con tus respuestas
├── TL_OCR_completo.md                ⭐ las 274 diapositivas del timeline, en texto
├── GUIA_DEL_PROYECTO.md              este archivo
├── informes/                         los dos análisis largos con citas
├── corpus_extraido/                  los 95 .docx/.xlsx convertidos a texto legible
│
├── onegai timeline.docx / .pdf / .pptx    192 MB · las MISMAS 274 diapositivas
│                                          en tres formatos. Son imágenes puras:
│                                          cero texto seleccionable. Usa el OCR.
│
├── Onegai/                           ⭐ TODO EL MATERIAL NARRATIVO (1.041 archivos)
│   ├── OnegaiTimeLine/               el vault de Obsidian: 158 fichas + 19 canvas
│   │   ├── TML/                      la línea temporal por eras y por raza
│   │   ├── Campanyes/                campañas troceadas en fichas jugables
│   │   └── Descripciones/            astrología
│   ├── THE ONEGAI PROJECT/
│   │   ├── THE ONEGAI PROJECT/       ⭐ los 88 .docx y 8 .xlsx de lore
│   │   │   ├── (raíz)                Gran Compendio, Reinos, Partida de Rol…
│   │   │   ├── Plantillas/           razas · clases · hojas de datos
│   │   │   └── Campañas/             campañas jugables + Ayashii + los Reinos
│   │   └── infologia/                recursos gráficos (2,2 GB)
│   │       ├── Mapas/                30 mapas + tácticos de mesa con .psd
│   │       ├── Fichas personajes/    7 retratos
│   │       ├── DOCUMENTOS PARA JUGAR/  el PDF de naciones y razas
│   │       ├── INFO/                 manuales de D&D 5e (896 MB, de terceros)
│   │       └── WEL-Resources/        694 mapas de MCDM (842 MB, de terceros)
│   └── ONEGAI_LORE_MIXED/            vacía
│
├── AuthorAgent/                      herramienta 1 · dashboard web (Node)
│   ├── workspace/library/            lo único con contenido tuyo:
│   │   └── onegai_world/             las fichas del vault copiadas para el agente
│   └── skills/                       write · outline · continuity-check ·
│                                     beta-reader · style-clone · revise…
│
├── StoryCraftr/                      herramienta 2 · CLI Python (worldbuilding,
│                                     outline, generación de capítulos, chat)
│
└── PanelOnegai/                      ⭐ el panel que une las dos: marcas qué
                                      documentos debe leer el agente y le mandas
                                      el encargo. Doble clic en
                                      "Abrir Panel Onegai.command"
```

**Proporciones reales:** de los ~18.500 archivos, **StoryCraftr son 14.067** (casi todo librerías de Python) y **AuthorAgent 3.384** (casi todo `node_modules`). Tu material son unos 200 archivos, y el 95% del peso en disco son los manuales de D&D y los mapas de MCDM que ni siquiera son tuyos.

---

# PARTE 4 — LO QUE ESTÁ ROTO, DUPLICADO O SOBRA

**Se puede archivar sin perder nada:**

- `Día 4, Mes de los Aullidos año 800_.docx` — son literalmente las primeras 121 líneas de `La Batalla De Ranmount 800 d.e_.docx`. Comprobado con `diff`.
- `Plan De Salida.docx` — **completamente vacío**, cero párrafos.
- `Onegai/ONEGAI_LORE_MIXED/` — carpeta vacía.
- `Onegai/2025-12-23.md`, `2026-02-27.md`, `Dr Oren Lir "Punt de Sutura".md` — notas diarias de Obsidian, vacías.
- Las tres carpetas de cruzadas vacías: `2 Segunda`, `3 Tercera`, `4 Quarta`.

**Roto o inservible:**

- `Plantillas/Niveles Clases y Razas.xlsx` — lleno de `#REF!` y `#N/A`.
- `Campañas/Ciudades.xlsx` hoja 3 — tabla de porcentajes (N)/(P) sin cabecera. Indescifrable sin ti.
- `Plantillas/5th Edition Spells.xlsx` — 54.521 palabras de D&D 5e sin una sola línea de Onegai.

**Mal ubicado (mover, no borrar):**

- `Campañas/Clase_ de Feriante.docx` → debería estar en `Plantillas/Classes/`
- `Plantillas/Personajes Hombrew/Astrologia.docx` → es lore, no una ficha de personaje
- `Plantillas/Razas/Estetiques/HUMA DE BASTREA.docx` → única superviviente de una carpeta de diseño visual nunca terminada. Contiene la paleta de colores oficial de Bastrea.

**Borradores con datos únicos que hay que rescatar antes de tirarlos:**

- `Untitled document(1).docx` — único sitio donde aparecen **Varaxx, el Maestro de las Sombras** (regente de los Perdidos) y el **Puente de la Traición**.
- `Documento sin título.docx` — propone un **culto a Egos** (no a Gurkazaal) y contiene la fecha más tardía de todo el corpus: **855 d.e.**
- `Untitled document.docx` — el escalado completo de "Sello de Luz" y "Redención de los Caídos", que falta en la ficha de la clase Redimido.

---

# PARTE 5 — LOS HUECOS, ORDENADOS POR LO QUE CUESTA CERRARLOS

**Cinco minutos, y solo puedes hacerlo tú:**

1. **La leyenda de colores del mapa** (`Ciudades y sus elementos.xlsx`, hoja 5). Sin ella no se puede asociar cada ciudad con su elemento, que es justo lo que promete el nombre del archivo.
2. **Los dos culpables de la Corrupción de Nuragapóken.** La diapositiva 261 tiene los rótulos "Saga importante:" y "Hombre del este importante:" **en blanco**. Es el evento bisagra de toda la Era Oscura.
3. **El offset del calendario: 2738 o 2838.** Arrastra un siglo en toda la Era de las Cruzadas.

**Una tarde:**

4. Las diapositivas **257–260** ("El Conocimiento tras la Guerra") y **267–269** ("Consecuencias físicas de la corrupción") son solo rótulos. La 268 se corta a media frase: *"Los Vampiros, que antaño"*.
5. **Eros / Deseo / Displicencia.** Tu distinción es clara pero no tiene ni una línea de soporte en 463.000 palabras. Hay que escribirla desde cero.

**Trabajo de verdad:**

6. **Las Cruzadas 9ª a 14ª** (1630–1058 b.f.) — cero líneas en todo el proyecto.
7. **Todo lo posterior a 972 b.f.** — Ascenso del Mal, Gran Guerra, Cataclismo y mundo flotante existen solo como apuntes en `Partida de Rol avec Uri.docx`.
8. **La cosmología multiplanar.** Las clases Apóstata, Saltador de Planos, Renacido y Poseído del Vacío presuponen Reino Astral, Reino de las Almas, el Vacío y unas "rutas interplanares cerradas por los Elementales en 6719". Nadie ha escrito ese sistema.

**Contradicciones que siguen abiertas** (detalle en el addendum):

- **Artrenax el Velo Negro asciende dos veces**, con 5.739 años de diferencia y en versiones incompatibles: en una funda el Consejo de las Sombras, en la otra lo derroca.
- **Azhira Espiral de Marfil aparece viva a lo largo de 6.755 años.** O es inmortal, o *Azhira* es un cargo hereditario como Nuragapóken.
- **Gyrid Voskezorik muere en dos sitios distintos y en dos bandos distintos.**
- **`Los Reinos del Oeste` duplica once secuencias enteras** de eventos con desfases de 2.000 a 3.943 años. Si es error de edición, sobran doce entradas de la cronología.
- **Kortarium** aparece en la lista de los 13 reinos de `Partida de Rol` y en ningún otro sitio de las 463.000 palabras. Candidato a residuo, como lo era Knehapnest.
