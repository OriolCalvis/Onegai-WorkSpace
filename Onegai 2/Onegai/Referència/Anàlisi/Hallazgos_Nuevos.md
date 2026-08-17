# HALLAZGOS NUEVOS — tercera pasada

> Cuatro cosas que no se habían mirado: las hojas de cálculo abiertas de verdad (no su texto extraído), el mapa que estaba dentro de una de ellas, el sistema de calendarios completo, y el vault de Obsidian.
>
> **Dos de estas correcciones son a afirmaciones mías anteriores.** Van marcadas.

---

# 1. ⭐ EL MAPA — ya no hay que imaginárselo

**→ `Mapa_Egaroth.svg`** (raíz del proyecto)

He reconstruido el mapa del mundo desde `Ciudades y sus elementos.xlsx`, Hoja 4. No era una tabla de nombres: cada celda **tiene un color de relleno asignado por reino**, y esos colores son tu paleta. La rejilla de 26×26 es un mapa político completo, con 18 territorios y el mar en azul pálido.

El SVG se abre en Obsidian y en cualquier navegador, con las capitales y la raza dominante de cada reino.

## La paleta oficial (nunca estuvo escrita en ninguna parte)

| Reino | Color | Hex | Celdas | Capital |
|---|---|---|---|---|
| Gongorguma | verde claro | `#B6D7A8` | 32 | Guskedor |
| Tabaxi | azul claro | `#A4C2F4` | 30 | Naka't-ol |
| Aegroum | crema | `#FFF2CC` | 23 | Ranmont |
| Aeon | amarillo puro | `#FFFF00` | 21 | Conclave Elemental |
| Ecla | verde muy claro | `#D9EAD3` | 18 | Umedan |
| Bastrea | azul | `#4A86E8` | 18 | Numandum |
| Udrax | naranja | `#FF9900` | 17 | Bomengrid |
| Ascaria | rosa | `#F4CCCC` | 13 | Finesaux |
| Ostad | amarillo pálido | `#FFE599` | 11 | *(sin capital)* |
| Bosmurg | marrón oscuro | `#5B0F00` | 11 | Tor'k Hazar |
| Nocturnsea | gris claro | `#D9D9D9` | 8 | Ciudad de Grytoz |
| Gliaddokx | verde puro | `#00FF00` | 8 | Havar'gruztak |
| Esmua | mostaza | `#BF9000` | 7 | Saif-l'sa |
| Mistarium | lila | `#B4A7D6` | 6 | Venordemn |
| Choubar | granate | `#741B47` | 6 | Klimnebra |
| Wulcain | verde medio | `#6AA84F` | 5 | Dhin Thyraxion |
| Ayashii | crema | `#FFF2CC` | 4 | Himetsumota |
| Ashye | gris | `#999999` | 4 | Zathor'aetz |
| *(mar / vacío)* | azul pálido | `#D0E0E3` | 434 | — |

**Dos cosas que salta a la vista al verlo dibujado:**

1. **Ayashii y Aegroum comparten exactamente el mismo color** (`#FFF2CC`). En el mapa no se distinguen. Están lejos, así que no molesta, pero conviene cambiar uno.
2. **Gongorguma es enorme** — 32 celdas, casi el doble que cualquier reino humano, y domina todo el centro-este. Tabaxi es el segundo con 30, pero repartidas en dos fragmentos muy separados (costa oeste y todo el sureste). Ashye, Wulcain y Ayashii son diminutos: 4 o 5 celdas.

**Y lo que confirma:** **Knehapnest no está en el mapa.** Ni una celda. Tu decisión de tratarlo como residuo era correcta. **Nocturnsea sí está**, con un bloque compacto de 8 celdas al norte del continente oriental — es una nación real, no un error de transcripción.

---

# 2. ❌ CORRECCIÓN — la "leyenda de colores que faltaba" no existe

En la guía y en el addendum dije que la Hoja 5 era el mapa coloreado por elementos y que **solo tú podías darme la leyenda**. Me equivoqué al leerlo desde el texto extraído.

**Lo que es en realidad la Hoja 5:** un **mapa de detalle de Ascaria**, dibujado dos veces (filas 1–11 y filas 13–24, la segunda con los nombres de reino escritos enteros). Todo el fondo es rosa Ascaria (`#F4CCCC`), con Ecla y Udrax asomando arriba en sus colores.

Los cuadraditos 🟥🟧🟩🟦 **no son colores de celda: son caracteres emoji tecleados dentro de la celda.** Junto a ellos hay `C` (26–32 apariciones) e `I` (3). Reparto:

| Símbolo | Bloque 1 | Bloque 2 |
|---|---|---|
| `C` | 26 | 32 |
| 🟦 | 10 | 8 |
| 🟩 | 6 | 5 |
| 🟧 | 5 | 6 |
| 🟥 | 4 | 0 |
| `I` | 3 | 3 |

**Sigue sin haber leyenda** —eso no cambia— pero la pregunta es mucho más pequeña de lo que yo decía: no es "descifrar los elementos de todo el mundo", es **"¿qué significan C, I y los cuatro cuadrados de colores en el mapa de Ascaria?"**. Por posición, `C` parece costa o frontera (siempre bordea) e `I` isla (siempre aislada en el mar). Los cuatro colores parecen biomas o elementos.

Que 🟥 aparezca 4 veces en el primer dibujo y **cero** en el segundo sugiere que el bloque 2 es una revisión donde se eliminó ese terreno.

---

# 3. ⭐ EL SISTEMA DE CALENDARIOS, RESUELTO

Abrí la hoja *Años* de `Plantillas/Campañas.xlsx` celda a celda (130 columnas × 2015 filas). **Confirmo los siete calendarios paralelos y ahora tengo la época exacta de cada uno**, deducida de cientos de pares año-a-año:

| Calendario | Año 1 (o 0) | Fórmula | Empieza en | Comprobado con |
|---|---|---|---|---|
| **Before Forcast** | — | eje base | — | 1.003 filas |
| **Despertar Elemental** | 2838 b.f. | `d.e. = 2838 − b.f.` | año **1** | 10 bloques, 218 pares, **sin una sola excepción** |
| **La Gran Alianza** | 1815 b.f. | `GA = 1815 − b.f.` | año **1** | 7 bloques |
| **Gongorguma** | 1805 b.f. | `G = 1805 − b.f.` | año **0** | 2 bloques |
| **Neo Gongorguma** | 1665 b.f. | `NG = 1665 − b.f.` | año **1** | 3 bloques |
| **Resistencia** | 1395 b.f. | `R = 1395 − b.f.` | año **0** | 13 bloques |
| **Ascaria** | 1159 b.f. | `A = 1159 − b.f.` | año **0** | 11 bloques |
| **After Forcast** | 0 b.f. | `a.f. = −b.f.` | año **0** | 1 bloque |

Fíjate en un detalle de diseño que ya está en tus datos: **unos calendarios empiezan en el año 0 y otros en el año 1.** Gongorguma, Resistencia, Ascaria y After Forcast cuentan desde 0; Despertar Elemental, la Gran Alianza y Neo Gongorguma desde 1. Si es intencionado, es un detalle cultural precioso (los pueblos que cuentan desde el suceso frente a los que cuentan desde el año siguiente). Si no, hay que unificarlo.

## Lo que esto significa para la decisión del offset

**La hoja es 100% consistente en 2838.** No hay ni una fila que dé 2738. Y la prosa es 100% consistente en 2738 — el vault aporta ahora **dos pares más desde un tercer documento, ambos 2738**, con lo que van **29 pares contra 0**.

Ya no es "una fuente está mal tecleada": son **dos sistemas enteros y coherentes, desfasados exactamente 100 años**. Elegir uno obliga a rehacer el otro entero.

Mi recomendación sigue siendo **2738** (la prosa manda: es donde están las cruzadas, las batallas y los reinados), pero ahora sabes el coste: hay que desplazar 100 años **las siete columnas** de la hoja *Años*, no solo una.

## Un error real que encontré en la hoja

El calendario **Resistencia se descuadra a sí mismo**: mantiene offset 1395 hasta el año ~635 b.f. y a partir de 630 b.f. pasa a 1390. **Se pierden cinco años a mitad de tabla** — es un salto de fila al copiar. Afecta a todos los eventos de la Resistencia posteriores a 630 b.f.

## Una pregunta que se abre

La época de **La Gran Alianza es 1815 b.f.**, pero `Partida de Rol` narra su fundación en **1925 b.f.** Son 110 años de diferencia, y no se arregla con el ajuste de 100. O el calendario no empieza en la fundación (sino en algún hito posterior), o una de las dos fechas está mal.

---

# 4. ⭐ EL VAULT DE OBSIDIAN — 25.000 palabras que no están en ningún otro sitio

Informe completo en **`informes/informe_vault.md`**. Lo esencial:

## Lo exclusivo del vault

**`El Pacte de les Ànimes` (2.502 palabras).** En el timeline OCR, la diapositiva 236 tiene **solo el título**. El vault tiene la narración entera — y **corrige la cronología maestra**:

- **Vincalp no muere.** Queda *«segellat en un calabós d'obsidiana i memòria»* bajo Itron.
- Los **Chimenyorik** se extinguen.
- Los **Tierrakteros** huyen a Ashye y **los acogen los Sagas** — vínculo élfico-saga que no aparece en ninguna otra fuente.
- Los **Vienturkos se borran su propia memoria**.
- Contiene el **único diálogo entre Creadores de todo el proyecto**, y es **Guerra** quien propone crear a los elfos.

**La prehistoria de Egaroth.** Los **Deaps**, el Hechicero que los extermina, el agujero negro. Y una idea que no está en ningún otro documento: **las estrellas son los restos de los Deaps muertos**, y el sueño de Egaroth no es una elección sino un trauma.

**`Aegor`** — el nombre del ser fusionado Vida+Muerte antes de separarse. No existe en ningún otro documento del proyecto.

**El orden de batalla de las Guerras del Ocaso**: Hombres Gato + Minotauros + Chimenyorik **contra** Orcos + Hombres Pájaro + Sagas + Vienturkos.

## Lo que yo daba por exclusivo y NO lo es

El material élfico que en la guía marqué como "solo en el vault" —la Crònica dels Elfs, el Torneo de los Fuertes, las cuatro subrazas, los Aguakturos— **está íntegro en el OCR del timeline, diapositivas 228–235**. Las fichas de Dracónidos y Elementales son copia del Gran Compendio, y el Llibre de les Llunes es duplicado de `Astrologia.docx`.

## Dos problemas de las campañas del vault

- **Campanya de Itron** encaja bien en 8495 b.f., pero **su propio frontmatter dice 8565**. Contradicción interna.
- **Tren hacia el Himetsu (4592 b.f.) es cronológicamente imposible**: un tren de vapor rúnico **3.943 años antes** de que se invente la válvula, y patrocinado por una "Alianza" cuya composición (Udrax + Ecla + Bastrea) es la de 1925 b.f. O se refecha, o es una campaña de otra era.

## El estado real del vault

El último trabajo de verdad fue el **25 de abril de 2026, y duró nueve minutos** (la carpeta de Elfos). **El 20% de los archivos están a 0 bytes**, 12 de los 19 canvas son marcadores vacíos y hay 7 enlaces rotos.

Dicho claro: **el vault se saquea, no se mantiene.** Es una cantera de la que has ido sacando material, no una base de datos viva. Lo cual está bien —pero significa que hay que rescatar de ahí lo bueno (el Pacte, los Deaps, Aegor) y no confiar en sus fechas.

## Una corrección del vault a la maestra que sí hay que aplicar

**Taurengrad: el vault tiene razón.** 8739–8722, confirmado por la diapositiva 193. El 8716 que puse en la maestra **no aparece en ninguna fuente**; lo deduje mal de las fases solapadas.

En cambio, tres fechas del vault son demostrablemente erróneas y no deben propagarse. La de la Purga de las Tormentas es flagrante: el `Contexto.md` es copia **literal** del Gran Compendio **con el año cambiado** de 8760 a 8786.

## Y algo que los canvas revelan sin escribirlo

Los `.canvas` son grafos de relaciones, y contienen estructura narrativa que ningún texto pone por escrito:

- El **arco élfico** fija que Torneo → Pacto → Arboleda (8765) es una secuencia lineal. Eso **estrecha el rango de `EV-SF-TORNEO`**, que en la maestra está sin fechar.
- El de **Elementales** es un grafo genealógico: dos hijos leales, dos rebeldes, y **Eryon como punto de convergencia**.
- El de era demuestra que **los cuatro clanes humanos convergen en el Día de los Cuatro Soles**.

---

# 5. Qué haría ahora

**Aplicar ya, sin decisión tuya:**

1. Corregir Taurengrad en la maestra a **8739–8722**.
2. Incorporar el **Pacte de les Ànimes** entero como desarrollo de la entrada élfica (hoy ⬜ pendiente).
3. Añadir **Aegor** y los **Deaps** a la Era 0.
4. Cambiar el color de Ayashii o Aegroum para que no colisionen.

**Necesita que decidas:**

5. **El offset**: 2738 o 2838. Ya no hay más datos que buscar — hay 29 pares en la prosa contra una hoja de cálculo internamente perfecta. Es una decisión, no un hallazgo.
6. **Qué son `C`, `I` y los cuatro cuadrados de colores** en el mapa de Ascaria.
7. **Tren hacia el Himetsu**: ¿a qué era pertenece de verdad?
8. **La Gran Alianza**: ¿1815 o 1925 b.f.?
9. El **descuadre de 5 años** del calendario Resistencia a partir de 630 b.f.
