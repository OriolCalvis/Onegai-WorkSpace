# Atlas de Egaroth — mapamundi jugable y volcado al proyecto documental

*Addendum de `ONEGAI_CORE_Analisis_y_Plan.md` · 11 de agosto de 2026 · revisión 2*
*Todo lo que hay aquí está ejecutado y verificado, no propuesto: los scripts están en `bridge/` y su salida en `atlas/salida/`.*

> ## ⏳ El mapa está fechado: **2000 b.f.**
>
> Todo este documento describe **una instantánea**, no el mundo eterno. Y la fecha resultó no ser una cualquiera: **2000 b.f. es el primer año de la ERA XIII — LAS CATORCE GRANDES CRUZADAS (2000 – 1058 b.f.)**, la era más extensa de la cronología con 59 entradas. El mapa retrata Egaroth el año en que empieza la Primera Cruzada.
>
> Eso obligó a **corregir la validación de la revisión 1** (sección 1) y añadió una sección nueva sobre qué significa que el mapa tenga fecha (sección 1b).

---

## 0. La conclusión, primero

**El mapamundi no hay que diseñarlo. Ya está dibujado, desde hace tiempo, dentro de una hoja de cálculo.**

`THE ONEGAI PROJECT/Ciudades y sus elementos.xlsx`, Hoja 4, es una rejilla de **26×26 celdas donde cada celda lleva el nombre del reino** y `~` marca el mar. Eso no es una tabla de datos: es un mapa de tiles. Es, literalmente, el formato de mapamundi de un Dragon Quest, autorizado por ti y guardado en Excel.

243 celdas de tierra, 433 de mar, **19 naciones** — 18 de la hoja más los Gnomos, que añadiste a mano (ver 0b).

Y tu intuición sobre dividir por partes de un continente resulta ser lo que el mundo **ya hace solo**. Al calcular las masas de tierra conexas salen **14**, y se reparten así:

```
   ..........................      A Aegroum    G Bosmurg     M Mistarium
   ....BBBBBBB....NNNN..M.M..      B Aeon       H Choubar     N Nocturnsea
   ....BBBBBBB....NNNN....MM.      C Ascaria    I Ecla        O Ostad
   .EE.BBBBBBB..........M....      D Ashye      J Esmua       P Tabaxi
   .EE.............LL...M..L.      E Ayashii    K Gliaddokx   Q Udrax
   ................LL.....LL.      F Bastrea    L Gongorguma  R Wulcain
   ............O...LLLLLLLLL.      g Gnomos — añadido a mano (ver 0b)
   ........OOOOO....LLLLLLL..
   ...I....OOOOO.....LLLLL...
   .P...IIIQKKKK......LLLL...
   .P..IIIIQQKKK......HHHH...
   .PP.IIIIQQQQK......DDHH...
   ....IIIIQQQQQ......DGGG...
   .....IIQQQQQF......DGGG...
   ..........FFF......PPGG...
   ....CCCCCFFF.......PPGG...
   ..C.CCCCCFFF......PPPGJ...
   ....AAAAAFFF.....PPRRJJ...
   .C..AAAAAFFF.....PPRRJJ...
   ....AAAAAFF..g...PPPRJJ...
   ..C.AAAAA........PPPPPPP..
   .....AAA..........PPP.PP..
   ..........................
```

| Masa | Celdas | Naciones |
|---|---:|---|
| **Continente occidental** | 104 | Ostad · Udrax · Ecla · Gliaddokx · Bastrea · Ascaria · Aegroum |
| **Continente oriental** | 91 | Gongorguma · Choubar · Bosmurg · Ashye · Tabaxi · Esmua · Wulcain |
| Aeon | 21 | isla propia — coherente con que sea el reino elemental |
| Nocturnsea | 8 | isla |
| Ayashii · Tabaxi (isla) · Mistarium ×2 | 4·4·3·2 | archipiélagos |
| **Gnomos** | 1 | islote en el mar entre los dos continentes — añadido a mano |

Dos continentes de siete naciones cada uno, más un rosario de islas. El mundo **ya viene troceado** y el troceo tiene sentido narrativo — mucho más de lo que parecía, como se ve en la sección 1.

## 0b. Los Gnomos, y por qué existe `atlas/overlays/`

Añadiste a mano una celda de **Gnomos** en el bloque de arriba, en `(fila 19, columna 13)`. Es una aportación real al canon y merece dos comentarios.

**Uno: encaja con un hueco ya documentado.** Tu propia `GUIA_DEL_PROYECTO.md` avisa de que *"faltan fichas de Tieflings, Gnomos, Elementales y Vampiros, que son centrales en el lore"*. Los Gnomos no estaban en la Hoja 4 ni en la cronología. Ahora tienen sitio: una **isla de una celda en el mar entre los dos continentes** (sus cuatro vecinos son agua). Estar equidistante de los dos frentes de las Cruzadas es, narrativamente, un sitio interesante para un pueblo que no participa en ellas.

**Dos: esa edición se habría perdido.** La hiciste sobre el bloque de este documento, que es salida generada — la siguiente ejecución del script lo habría sobrescrito. Es exactamente el fallo que la regla *"no editar a mano, se regenera"* pretende evitar, y el que a mí me tocaba prever.

Así que ahora existe **`atlas/overlays/correcciones.json`**: el sitio donde las correcciones manuales sobreviven a la regeneración.

```json
{ "epoca": { "anio": 2000, "sufijo": "b.f.",
             "era": "ERA XIII — LAS CATORCE GRANDES CRUZADAS (2000 – 1058 b.f.)" },
  "celdas": [
    { "fila": 19, "columna": 13, "nacion": "Gnomos",
      "motivo": "Añadido a mano por Oriol. Los Gnomos son centrales en el lore y
                 no figuraban en la Hoja 4 ni en la cronología." } ],
  "capitales_canon": { "Bastrea": "Numandum", "Mistarium": "Venordemn", … },
  "frentes_cruzada": { "occidental": [...], "oriental": [...] } }
```

`extraer_rejilla.py` lo aplica encima de la hoja de cálculo. La hoja sigue siendo la fuente; el overlay es la corrección, con su motivo escrito al lado. Ahí es donde van las 20 ciudades por colocar, el conflicto de Gliaddokx y cualquier nación futura.

*(Los Gnomos ocupan 1 celda, y el generador salta las masas de menos de 2 porque un nivel de 8×8 tiles no es un mapamundi. Si quieres que sea visitable, o le das otra celda o se hace como nivel de isla a mano.)*

---

## 1. Por qué se puede confiar en esta rejilla

De la rejilla se pueden derivar las **18 fronteras terrestres** del mundo: qué nación toca con cuál y por cuántas celdas. Eso permite contrastarla contra la cronología.

> **Corrección de la revisión 1.** Validé la rejilla con tres eventos de los años 8964, 8400 y 7816 b.f. Encajaban, pero **eran mala prueba**: ocurren entre 6.000 y 7.000 años *antes* de la fecha del mapa. Que Bastrea y Udrax se toquen en 2000 b.f. no dice nada sobre si se tocaban en 7816. Sabiendo la fecha, la validación correcta usa eventos **contemporáneos** — y es mucho más fuerte.

### La prueba de verdad: los dos frentes son dos continentes

La cabecera de la ERA XIII, en la cronología maestra, dice:

> *"Las Cruzadas corren en **dos frentes paralelos** —uno occidental (Ascaria, Aegroum, Bastrea, Ecla, Udrax) y uno oriental (Choubar, Bosmurg, Esmua, Tabaxi, Gongorguma)— que `Partida de Rol avec Uri` numera por separado hasta la Cuarta, cuando ambos se funden en uno solo."*

Y las masas de tierra que salen de la rejilla, calculadas sin saber nada de esto:

| | Frente según la cronología | Masa de tierra calculada | |
|---|---|---|---|
| **Occidental** | Ascaria, Aegroum, Bastrea, Ecla, Udrax | `landmass_1` — las 5, más Ostad y Gliaddokx | ✅ contenido entero |
| **Oriental** | Choubar, Bosmurg, Esmua, Tabaxi, Gongorguma | `landmass_2` — las 5, más Ashye y Wulcain | ✅ contenido entero |

**Ningún frente cruza de masa. Son disjuntos.**

Eso no es una coincidencia: **es la explicación geográfica de la estructura narrativa de la era**. Las Cruzadas corrieron en dos frentes paralelos porque estaban en dos continentes separados, y por eso se numeraban por separado — hasta la Cuarta, cuando se funden. La rejilla acaba de dar el *por qué* de un hecho que la cronología solo constataba.

Y en el propio año del mapa:

| Evento datado en 2000 b.f. | La rejilla |
|---|---|
| `EV-2000-CRUZADA01E` — *Primera Cruzada (frente Este): **Choubar contra Bosmurg*** | Bosmurg—Choubar ✅ frontera de 2 celdas — una guerra entre vecinos directos |
| `EV-2400-CUERNOZARPA` — *"Esmua y Bosmurg formalizan la alianza"* (400 años antes) | Esmua—Bosmurg ✅ frontera de 3 celdas |

Esto sí es canon verificado contra su propia época.

Y da algo que el proyecto documental no tenía: **el grafo de vecindad del mundo**. Hasta ahora, "quién limita con quién" solo estaba implícito en la prosa.

---

## 1b. Qué significa que el mapa tenga fecha

Es la consecuencia de más alcance de todo este addendum, y conviene verla antes de construir nada encima.

**El atlas deja de ser "el mapa de Egaroth" y pasa a ser "Egaroth en 2000 b.f."** La cronología abarca de la Cosmogonía al 300 a.f. — 17 eras, más de 9.000 años. Un solo mapa no puede describir todo eso, y de hecho no lo describe:

- Naciones que **aún no existían**: la cronología fecha fundaciones a lo largo de 7.000 años (Wulcain en 7930, Ayashii en 7932, los clanes goblin de Gliaddokx en 7692…). En un mapa del 9000 b.f. media rejilla estaría vacía.
- Naciones que **cambian de nombre o caen**: el índice de lugares recoge *Neo Gongorguma*, *Ranmont / Tierra Santa*, *Velours (antes Mal Lunar, antes Veloru)*, *Killville (antes Killollie)*. Son la misma tierra en momentos distintos.
- Lo que viene **después**: la ERA XIV es *el ascenso del mal y el Imperio de Ascaria*, y la XV *la Gran Guerra y el Gran Cataclismo*. Un cataclismo, casi por definición, redibuja el mapa.

**Qué se hace con eso ahora:** nada, salvo etiquetar. Ya está hecho — `world_grid.json` lleva un bloque `epoca`, el SVG lo lleva en el título, las 19 fichas de nación lo dicen en la cabecera, y los TMX y niveles del motor lo llevan en un comentario y en un campo. El coste de etiquetar hoy es cero; el coste de descubrir dentro de un año que un mapa sin fecha se usó para tres eras distintas, no.

**Lo que queda abierto para el futuro**, y no hay que resolver ahora: si algún día quieres un segundo mapa —pongamos el del Imperio de Ascaria, o el posterior al Cataclismo— la estructura ya lo admite (`atlas/2000bf/`, `atlas/103bf/`…). El diseño correcto es guardar **mapas por época completos**, no diffs: son 26×26 celdas, ocupan nada, y un diff entre dos mapas se calcula cuando se necesita. Lo que no hay que hacer es tener un mapa sin fecha y parchearlo.

Una consecuencia práctica e inmediata: **el mapa acota qué contenido puede situar**. Una aventura de la Novena Cruzada o una carta que hable del Imperio de Ascaria no se pueden colocar en este mapa sin comprobar antes que sus lugares existían en 2000 b.f. El validador del atlas debería avisar de eso.

---

## 2. El mapamundi, ya generado

Decisiones tomadas: **proyección isométrica** (la nativa del motor) y **8 tiles caminables por celda** de la rejilla.

`bridge/generar_mapamundi.py` expande la rejilla a TMX + nivel JSON, un mapamundi por masa de tierra. Ejecutado:

| Nivel | TMX | Tiles caminables | Ciudades | Naciones |
|---|---|---:|---:|---:|
| `mundi_landmass_1` | **72×128** | 6.656 | 46 | 7 |
| `mundi_landmass_2` | **72×144** | 5.824 | 28 | 7 |
| `mundi_landmass_3` (Aeon) | 56×24 | 1.344 | — | 1 |
| `mundi_landmass_4` (Nocturnsea) | 32×16 | 512 | — | 1 |
| + 4 islas menores | | 832 | 1 | 4 |
| **Total** | | **15.168** | **75 / 95** | 18 |

*(Los Gnomos no generan nivel: 1 celda = 8×8 tiles, demasiado poco para un mapamundi.)*

**La escala es la correcta y esto importa:** el mayor mapa que el motor maneja hoy es 64×64 (4.096 tiles). El continente occidental es 72×128 = 9.216. Es **2,3× el mapa más grande que ya funciona** — dentro de lo que el `SpriteBatch` mueve sin tocar nada. A 16 tiles/celda habrían sido 144×256 y habría hecho falta trocear y medir rendimiento. A 8 no.

Y el resultado pasa la prueba de conectividad del propio motor (`tools/conectividad.py`: *"que toda puerta sea alcanzable andando; un umbral rodeado de muros carga sin error pero es contenido muerto"*):

```
mundi_landmass_1     6656/6656 celdas alcanzables     46/46 ciudades
mundi_landmass_2     5824/5824 celdas alcanzables     28/28 ciudades
...
TOTAL ciudades alcanzables andando: 75/75
```

Cero contenido muerto.

### Cómo encaja con lo que el motor ya tiene

Un mapamundi **no es un subsistema nuevo**. Es un nivel más:

```
mundi_landmass_1.tmx  ──LevelTransition──▶  ciudad_centro.tmx  ──▶  interior_taberna.tmx
   (72×128, exterior)                        (64×64, ya existe)      (13×11, ya existe)
```

`GameSession::enterLevelNarrative()` y `LevelTransition` ya hacen esa navegación; hay 3 ciudades y 18 interiores construidos. Lo único que falta es **el escalón de arriba**, el exterior, y es el que acaba de generarse. Los `EnemySpawn` del formato de nivel dan los encuentros aleatorios de camino sin código nuevo.

### Sobre la proyección

El motor es isométrico puro: `IsoMath` proyecta 2:1 y todos los niveles usan `tilewidth=64 tileheight=32`. El mapamundi generado usa exactamente eso, así que **el mismo tileset, la misma cámara, la misma colisión y las mismas sombras** valen para el exterior y para las ciudades. Cero código nuevo.

El precio, dicho claro: no se verá como el Dragon Quest original, que era cenital. Se verá como un isométrico de mundo abierto — Landstalker, Ultima VII. Si en algún momento quieres el cenital de verdad, el cambio no es imposible pero sí caro: obliga a una segunda proyección junto a `IsoMath`, un segundo set de tiles, y a que cámara y sombras conozcan ambos modos. Merece la pena verlo funcionando en isométrico antes de decidir eso.

---

## 3. Las tres representaciones del mapa, y cómo se reconcilian

Existen tres, y ninguna sabe de las otras:

| Fuente | Qué aporta | Qué le falta |
|---|---|---|
| `Ciudades y sus elementos.xlsx` Hoja 4 | **Topología**: rejilla 26×26, fronteras, islas, qué es mar | Resolución; ninguna ciudad |
| `dndWeebCC/data/mapa/geografia.json` | **Forma y detalle**: 19 polígonos (204 vértices), 95 ciudades con población y gobernante, 26 zonas | Los nombres canónicos; no sabe de fronteras |
| `Onegai 2/Mapa_Egaroth.svg` | **Los nombres canónicos** de las 18 capitales | Toda la geometría: 74 etiquetas, cero polígonos |

Al ajustar por mínimos cuadrados los centroides de nación entre la rejilla y los polígonos sale una transformación limpia:

```
celda_x = 0,01513 · px + 1,16        (1 celda ≈ 66 × 53 px)
celda_y = 0,01891 · py + 0,34
```

Error medio **1,39 celdas**, y con ella **88 de 95 ciudades caen dentro o al lado de su propia nación (93%)**. La peor es Tabaxi (3,8 celdas), y por una razón entendible: Tabaxi está partida en dos masas de tierra y el ajuste usa un único centroide — que es precisamente por lo que `geografia.json` ya distingue *Tabaxi Continental* de *Tabaxi Occidental*. **Las dos fuentes llegaron por su cuenta a la misma conclusión.**

Conclusión práctica: los polígonos se dibujaron a mano, no derivados de la rejilla, así que **no comparten proyección exacta y no hay que forzarla**. El reparto correcto es:

- La **rejilla** manda en topología (fronteras, islas, mar).
- Los **polígonos** mandan en forma y en el detalle de ciudades.
- El **SVG canónico** manda en los nombres.
- La unión se hace por **nación**, no por píxel.

Las 20 ciudades que el generador no colocó son las que caen en mar o fuera del *bounding box* tras el ajuste. Se resuelven a mano una vez y se congelan en el atlas; no es un problema recurrente.

---

## 4. El volcado al proyecto documental

Aquí hay un riesgo que conviene nombrar: volcar el mapa al proyecto documental **puede crear una segunda fuente de verdad**, que es justo lo que el plan intenta eliminar.

La salida es tratar todo lo volcado como **artefacto generado, nunca editado a mano** — igual que un informe compilado. El lore sigue viviendo en el corpus; lo volcado solo lo *sitúa*.

### 4.1 `mapa_egaroth_geometrico.svg`

Sustituye a `Mapa_Egaroth.svg`. Mismo viewBox, pero con los 19 polígonos de nación, las 95 ciudades (capitales destacadas), los 5 santuarios, **la fecha en el título** y **los nombres de capital tomados del canon, no de las cartas** — así que Bastrea aparece como *Numandum* y no *Nimandum*, y Mistarium como *Venordemn* en vez de `Capital`.

Es la primera vez que el proyecto documental tiene un mapa con fronteras.

### 4.2 Fichas de nación para el vault (19 notas)

Una nota por nación, con wikilinks para que Obsidian teja el grafo. Ejemplo real generado:

```markdown
# Udrax

> [!info] Ficha generada automáticamente desde el atlas.
> No editar a mano: se regenera. El lore va en el corpus; esto solo lo sitúa.

## Situación
- **Capital:** [[Bomengrid]]
- **Superficie:** 17 celdas de la rejilla canónica (7% de la tierra emergida)
- **Masa de tierra:** landmass_1

## Fronteras terrestres
- [[Gliaddokx]] — 7 celdas de contacto
- [[Ecla]] — 6 celdas de contacto
- [[Bastrea]] — 4 celdas de contacto
- [[Ostad]] — 1 celda de contacto

## Ciudades (8)
- **Bomengrid** (capital) · más de 30.000 · gobierna Baronesa Nara
- **Eottetika** (ciudad) · ~12.000 · gobierna Alcalde Ferran
- **Aclosos** (ciudad) · ~6.000 · gobierna Prior Ferran
  …

## Zonas (3)
- **Montaña**, **Sierra**, **Montaña I** — montana
```

Fíjate en lo que aparece solo: la ficha de Udrax muestra frontera con Bastrea, y la cronología dice que en 7816 hubo *"primer pacto humano-enano en Udrax"* entre esas dos. **La nota hace visible una conexión que estaba enterrada en una hoja de cálculo.** Multiplicado por 19 naciones y 285 eventos, eso es worldbuilding gratis.

Las fichas marcan además con ⬜ los asentamientos sin nombre canónico, convirtiendo el mapa en **cola de trabajo narrativo**: 47 sitios que existen, tienen habitantes y gobernante, y esperan un nombre.

---

## 5. Lo que queda abierto

**La capa de biomas está a medias.** La Hoja 5 del xlsx es la misma rejilla con colores y marcadores `C`/`I`, pero solo llega a 24×16 y buena parte es relleno. Los símbolos 🟦🟩🟧🟥 sí son legibles por máquina — **la "leyenda perdida" que la guía del proyecto daba por irrecuperable no lo está**, solo hace falta que digas qué elemento es cada color. Sin ella, el mapamundi generado es tierra/mar plano: se camina, pero no hay bosque ni desierto ni montaña.

**El conflicto de Gliaddokx** sigue sin resolver: el canon dice que su capital es Havar'gruztak, las cartas dicen Grahgan. Hay que elegir.

**Las 20 ciudades sin colocar** necesitan una pasada manual.

**Los santuarios no son todavía niveles.** Himetsumota, el Templo de Sofía y el Templo de Chronos están en el mapa como zonas; convertirlos en mazmorras es contenido, no fontanería.

---

## 6. Orden sugerido

1. **Decir qué es cada color de la Hoja 5.** Es una frase tuya y desbloquea toda la capa de terreno.
2. Resolver Gliaddokx y colocar a mano las 20 ciudades.
3. Copiar el mapamundi generado a `MotorGraphico/assets/` y engancharlo a `LevelTransition` con las 3 ciudades que ya existen. **Ahí ya se puede caminar por Egaroth.**
4. Volcar el SVG y las 19 fichas al vault.
5. Poblar la capa de biomas y los encuentros aleatorios por zona.

---

### Anexo — Qué hay en `Onegai-Core/`

```
bridge/
  extraer_rejilla.py        xlsx Hoja 4 → world_grid.json (rejilla, masas, fronteras)
  generar_mapamundi.py      world_grid + geografia → TMX + niveles del motor
  volcar_a_documental.py    atlas → SVG geométrico + 19 fichas de nación

atlas/
  world_grid.json           la rejilla canónica como datos, por primera vez
  salida/maps/              8 TMX de mapamundi
  salida/levels/            8 niveles con las 75 ciudades colocadas
  salida/naciones/          19 fichas para el vault
  overlays/correcciones.json época, celdas añadidas a mano, capitales canónicas
  salida/mapa_egaroth_geometrico.svg
```

Los tres scripts son deterministas y sin estado: se vuelven a ejecutar y regeneran todo. Ninguno modifica los proyectos originales.
