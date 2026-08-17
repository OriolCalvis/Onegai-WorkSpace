# INFORME — Las hojas de cálculo que nadie había abierto

> **Método.** Todas las hojas se han abierto con `openpyxl` celda a celda, respetando filas y columnas, con `data_only=True` para valores y `data_only=False` para fórmulas, y leyendo los rellenos (`cell.fill.fgColor.rgb`) donde el contenido parecía codificado por color. También se ha descomprimido el `.xlsx` de `Ciudades.xlsx` para buscar comentarios, nombres definidos y dibujos ocultos (no hay).
>
> **Aviso de tamaño.** Las dimensiones que reporta Excel (`244 × 14`, `1000 × 8`, `390 × 28`…) son **formato, no datos**. Casi todas estas hojas tienen entre 5 y 90 filas reales. Lo indico en cada sección.

---

# A. `Plantillas/Campañas.xlsx`

Es el **libro maestro del sistema de juego**: 21 hojas. Índice, tablas de raza/clase, el sistema de Creadores, el mapa de campañas, los finales, y las calculadoras de personaje. La hoja *Años* (los siete calendarios) ya se descifró en `Hallazgos_Nuevos.md` y no se repite aquí.

## A.1 `Index` (B2:F9) — la clave de lectura del libro

**Qué es:** el sumario del propio archivo, escrito a mano. Dos columnas.

| Columna B — hojas de sistema | Columna D–F — hojas de campaña |
|---|---|
| Index, Campañas, Loots, Enemigos, Secretos, Finales, Objetos, Años | Campañas → Campaña 01 → Campaña 02 (+ 02.2, 02.4) → Campaña 3 |

**Qué aporta:** confirma que el libro se concibió como **motor de una serie**, no como una hoja suelta: la columna D es el árbol de ramificación (una campaña troncal que se bifurca en 02, 02.2 y 02.4 según el final obtenido). También confirma que **`Campaña 3` estaba prevista y nunca se escribió**: aparece en el índice y no existe como hoja.

**Estado:** completa (como índice), pero **desactualizada**: no lista `Clases Razas`, `LVL`, `Niveles de Creador`, `Progreso` ni las cuatro hojas de Bonificadores, que sí existen.

## A.2 `LVL` (B2:N10) — la piedra Rosetta de las cuatro hojas de Bonificadores

**Qué es:** cuatro fichas resumidas de personaje a **nivel 30**, en dos bloques de dos. Columnas: `Fue / Des / Con / Int / Sab / Car`, en dos rejillas paralelas (B–G = Bastrea, I–N = Ascaria).

| Personaje nivel 30 | Fue | Des | Con | Int | Sab | Car |
|---|---|---|---|---|---|---|
| **Humano de Bastrea, Guerrero** | **76** | 44 | 9 | 10 | 40 | 11 |
| **Humano de Ascaria, Guerrero** | 44 | 12 | 10 | **42** | **41** | 11 |
| **Humano de Bastrea, Paladín** | 44 | **43** | 39 | 10 | 42 | 12 |
| **Humano de Ascaria, Paladín** | 14 | 11 | 40 | **42** | **41** | 12 |

**Qué aporta al mundo:** es la formulación numérica de una diferencia cultural que el corpus solo enunciaba en prosa. **Bastrea es cuerpo (Fuerza y Destreza), Ascaria es mente (Inteligencia y Sabiduría)**, y la brecha es enorme: un guerrero bastreo de nivel 30 tiene **76 de Fuerza contra los 14 del paladín ascario**. Encaja con las fichas raciales (`Diplomáticos de Bastrea` / `Guardianes del Elemento` frente a `Nobles de Ascaria` / `Mercenarios`) y con la hoja `Clases Razas`, que da a Bastrea ventajas en FUE+SAB+DES y a Ascaria en INT+SAB.

**Estado:** completa y coherente. Es la única hoja que existe para leer las cuatro de Bonificadores.

## A.3 ⭐ `Niveles de Creador` (rango real: **filas 1–44**, no 244)

**Qué es:** el sistema de progresión de poder divino. **No es un nivel de personaje**: es un canal de poder que se abre con **un Creador concreto**, y cada nivel abierto da **tres dones simultáneos**, uno por vía.

**Columnas reales:**

| Col | Contenido |
|---|---|
| A | `Lvl` — nivel de Creador (1 a 5) |
| B | `Tipo` — el Creador (fórmula: lee los nombres de `Clases Razas`!B1:H1; **Gurkazaal está escrito a mano porque no figura en esa tabla**) |
| C / D / E | Nombre del don de **Apoyo** / **Físico** / **Magia** |
| G+H, J+K, M+N | Los mismos tres dones repetidos, ahora con su **descripción de efecto** |
| C43 / C44 | Las cadenas `" de Nivel 2"` y `" de Nivel 3"` que las fórmulas concatenan |

**Cómo funciona.** Ocho Creadores × cinco niveles = 40 filas. Cada fila da tres dones: uno de **Apoyo** (afecta a aliados), uno **Físico** (afecta al combate propio) y uno de **Magia**. Los niveles 2 y 3 **no están escritos**: son fórmulas que añaden `" de Nivel 2"` / `" de Nivel 3"` al nombre del don de nivel 1. El nivel 4 está entero a `-`. El nivel 5 tiene nombres nuevos, escritos a mano, y **sin una sola descripción**.

### La tabla completa de los ocho Creadores (nivel 1)

| Creador | Apoyo | Físico | Magia |
|---|---|---|---|
| **Gurkazaal** | Sacrificio | Destructor | Susurro Demoníaco |
| **Chronos** | Mano de luz | Fuerza interna | Destello luminiscente |
| **Sofía** | Tiempo radiante | Confianza Extrema | Ayuda inconmensurable |
| **Envidia** | Manos largas | Desarme | Ilusionista |
| **Vida** | Guía divina | Golpe sanador | Potencia celestial |
| **Muerte** | Bloqueo necrótico | Ataque maldito | Invocación decrépita |
| **Egos** | Aliento moral | Crítico asegurado | Renovación mágica |
| **Eros** | Seducción mortífera | Amor tóxico | Succión de espíritu |

### ⭐ El nivel 5 — cada Creador tiene un **lugar del mundo** como poder

Esto es lo más valioso de la hoja y no está en ningún otro sitio del proyecto. En el nivel 5 los dones dejan de ser genéricos y **se nombran por ciudades, naciones y monumentos reales de Egaroth**:

| Creador | Apoyo | Físico | Magia |
|---|---|---|---|
| Gurkazaal | Castigo | Pulverizador | Purgatorio |
| Chronos | Canción del tiempo | Volver a empezar | Infinita confusión |
| **Sofía** | **Saber de Ekait** | Hermandad del conocimiento | **Estandarte de Ranmont** |
| **Envidia** | **Ley de Tabaxi** | Azote de filos | **Guardia de Ashye** |
| **Vida** | **El mesías de Numandum** | **Brisa de Argent** | Conocimiento inequívoco |
| **Muerte** | **Murallas de Perishton** | Necrosis | **Batallón de Nocturnsea** |
| **Egos** | **El grito de Bomengrid** | Precisión devastadora | Llamada de los ancestros |
| **Eros** | **Melodía de Ayashii** | Suicidio emocional | Rosario sinuoso |

Leído como geografía sagrada, esto asigna **once lugares a cinco Creadores**:
- **Sofía** ↔ Ekait (universidad élfica de Ecla) y Ranmont (capital de Aegroum).
- **Envidia** ↔ Tabaxi y Ashye — las dos naciones que el corpus asocia a nagas, drow y sagas.
- **Vida** ↔ Numandum (capital de Bastrea) y Argent (ciudad de Ascaria).
- **Muerte** ↔ Perishton (Aegroum) y **Nocturnsea** — otra confirmación de que Nocturnsea es una nación real, y la primera pista de **qué** es: el sitio del que sale un batallón cuando invocas a Muerte.
- **Eros** ↔ Ayashii (los yokai).

Gurkazaal y Chronos son los únicos que **no tienen ningún lugar**: sus dones de nivel 5 son abstractos (Castigo, Purgatorio, Canción del tiempo). Si es intencionado, dice algo: el corrupto y el señor del tiempo no están anclados a ninguna tierra.

### ⭐ Gurkazaal no está en la tabla de afinidades

La columna B se alimenta de `Clases Razas`!B1:H1, que solo tiene **siete** Creadores (Egos, Eros, Chronos, Sofía, Envidia, Vida, Muerte). Gurkazaal hubo que teclearlo a mano. Es decir: **ninguna raza declara afinidad con Gurkazaal**, pero sí se le puede abrir un canal de poder. Coherente con el mito (es la escisión corrupta de Egos) y con la mecánica (Sacrificio: *«pierdes la mitad de la vida, pero curas el doble»*; Destructor: *«te atacan con ventaja y atacas con ventaja»* — todos sus dones tienen precio).

### La paleta de colores de los Creadores (nunca escrita)

Cada fila tiene relleno propio, constante en las cinco repeticiones:

| Creador | Hex |
|---|---|
| Gurkazaal | `#F4CCCC` |
| Chronos | `#FCE5CD` |
| Sofía | `#FFF2CC` |
| Envidia | `#D9EAD3` |
| Vida | `#D0E0E3` |
| Muerte | `#C9DAF8` |
| Egos | `#D9D2E9` |
| Eros | `#E6B8AF` |

Ojo: **Gurkazaal usa el mismo rosa que Ascaria en el mapa** (`#F4CCCC`), y **Sofía el mismo crema que Aegroum y Ayashii** (`#FFF2CC`). Son paletas de archivos distintos, pero si algún día se dibujan juntas colisionan.

**Relación con razas y clases.** La hoja **no** la establece: `Niveles de Creador` solo se conecta con `Clases Razas` para leer los nombres. El puente raza↔Creador vive en la otra hoja (afinidad SI/NO/NEUTRAL) y en las fichas `.docx` de raza (patrocinio por subraza). **No hay ninguna regla que restrinja qué Creador puede tomar cada raza o clase.** Es un hueco real, no un dato perdido.

**Estado: media hoja.** Niveles 1 y 5 escritos; niveles 2 y 3 son relleno automático por concatenación de texto (no hay contenido nuevo); **nivel 4 vacío (`-`)**; **el nivel 5 no tiene ni una descripción de efecto**. De 120 descripciones posibles (8 Creadores × 5 niveles × 3 vías) hay **24**.

## A.4 `Clases Razas` (filas 1–34)

**Qué es:** dos tablas apiladas. Arriba (filas 1–20) la **afinidad de 19 razas/pueblos con siete Creadores** en SI/NO/NEUTRAL, más ventajas y desventajas de atributo. Abajo (filas 22–34) la **ficha comparada de 12 clases** con columnas: Ventajas, Desventajas, Competencias marciales, Competencias mágicas, Debilidades, Fortalezas, Salvaciones, Perdiciones, Niveles, Especialización, Profesiones, **Moralidad**, Atributos.

**Qué aporta al mundo (no mecánica):**
- La columna **`PROFESIONES`** es un dato social que no está en las fichas `.docx`: dice a qué se dedica en la vida civil quien sigue cada camino. *Paladín: «cualquier oficio noble y respetuoso»*; *Guerrero: mercenario, caballero, herrero*; *Pícaro: ladrón, espía, **actor**, mercenario*; *Bardo: artista, vendedor*; *Druida: **boticario, leñador***; *Chamán: **monje**, escritor, comerciante*; *Artificiero: **profesor**, vendedor, investigador*.
- La columna **`MORALIDAD`** fija el alineamiento cultural por clase: Paladín LEGAL, Guerrero NEUTRAL, Brujo y Pícaro **CAÓTICO NEUTRAL**, Artificiero CAÓTICO NEUTRAL, Cazador LEGAL NEUTRAL.
- Debilidades/fortalezas describen una **cosmología de oposición Luz/Sombra**: el Paladín es fuerte contra la Luz y débil ante las Sombras; el **Brujo exactamente al revés** (fuerte en Oscuridad, débil ante la Luz). Es la única formulación sistemática de esa polaridad en todo el corpus.
- Aparece un pueblo llamado **`Perdidos`** como fila de raza, con afinidad SI a Egos, Envidia y Muerte y NO a Chronos y Vida. Confirma lo que ya apuntaba `informe_agente2_razas.md`: los Perdidos son raza jugable, no solo facción.

**Estado:** la tabla de razas está **completa (19 filas)**. La de clases está **a la mitad: 12 clases de las 24** que existen en `Plantillas/Classes/`. Faltan todas las homebrew (Apóstata, Redimido, Saltador de Planos, Feriante, Señor de los Astros…).

## A.5 `Campañas` (rango real: filas 1–16)

**Qué es:** el **plan de la serie entera**, 16 líneas. Columnas: `Campañas | Misiones | Mapas | DLC | NPCs | Oro | EXP | Enfrentamientos | Objetos | Año | Party | Total`. Solo se rellenaron Campaña, Misión, Mapa, DLC, Año y Party (4 jugadores siempre).

| # | Misión | Mapa | "DLC" (lo que desbloquea) | Año |
|---|---|---|---|---|
| 1 | Matanza de Boundington | Boundington | 1 Raza (Sagas) | 757 d.e. |
| 2 | Segundo asedio a Ranmont | Tierra Santa | 2 Subrazas (Sagas) | 864 d.e. |
| **Sho-Noco** | **Primer nivel Creador** | — | — | — |
| Bonus 2 | Escapada de Boundington | Bosque, Perishton | 2 Clases | 757 d.e. |
| Bonus 2.2 | Incursión en las Santas Montañas | Santas Montañas | 2 Cambios de Raza | — |
| Bonus 2.4 | La lágrima de Perishton | Templo de los Perdidos | Objetos y Hechizos | — |
| **Sho-Noco** | **Segundo nivel Creador** | — | — | — |
| 5 | Adalides en Ceaseton | — | Bestiario ampliado | — |
| 6 | Escape de Tierra Santa | — | **Historia sobre Gurkazaal ampliada** | — |
| **Sho-Noco** | **Entreguerras — Tercer nivel Creador** | — | — | — |
| 7 | Adalides en Boundington | — | **Historia sobre Sho-Noco** | — |
| 8 | Doomsburry | — | **Primera Danza** | — |
| 9 | Batalla nocturna de Velours | — | 2 Clases | — |
| 10 | Asedio de Argent | — | 2 Cambios de Raza | — |
| 11 | La Carga de los Muertos | — | 1 Raza (Yokai) | — |

**⭐ Lo que aporta al mundo:** las tres filas amarillas (`#FFD966`) llamadas **Sho-Noco** son **interludios en los que se sube de nivel de Creador**. Cruzando con `PARTIDAS.docx` —donde Sho-Noco es la entidad que materializa a los aventureros en la taberna, les manda misiones, les muestra visiones de gente que no han visto y les narra lo que se pierden— queda claro que **Sho-Noco es quien concede los niveles de Creador**. Es el vínculo entre la voz narradora meta del juego y el sistema de poder divino, y no está escrito en ninguna otra parte.

También: la campaña 7 promete **«Historia sobre Sho-Noco»** como contenido desbloqueable — es decir, el autor tenía previsto explicar qué es Sho-Noco y nunca lo hizo. Y la campaña 8 desbloquea **«Primera Danza»**, que enlaza con el evento *Primera Gran Danza Ayashii* (5707) de `Ciudades y sus elementos.xlsx`.

**Estado:** esqueleto. 16 de 229 filas; seis de doce columnas sin un solo dato; solo dos campañas tienen año.

## A.6 ⭐ `Finales` (rango real: filas 1–13)

**Qué es:** el **árbol de finales ramificados**. Columnas: `Campaña | Finales | Desenlace | Obtener | Perder | Descripción`. Solo se usan las tres primeras. Rellenos: verde = Bueno, rosa = Malo, crema = Neutro.

**El árbol reconstruido:**

| Campaña | Final | Adónde lleva |
|---|---|---|
| **1** | Bueno | Ganas la confianza de Venides → **Campaña 2** (Venides logra volver a Nefaria) |
| 1 | Malo | Traicionas a Venides, no salvas a Verina ni a Edul → **Campaña 2.2** (puedes recuperar su confianza a medio camino) |
| 1 | Neutro | Traicionas a Venides, **matas a Verina y Edul**, te alías con **Sandes** → **Campaña 2.4** |
| **2** | Bueno | → Campaña 2.2 (Venides llega a **Perishton**) |
| 2 | Malo | Alianza con Sandes → Campaña 2.4 |
| 2 | Neutro | → Campaña 2.2 |
| **2.2** | Bueno | Te infiltras en el culto de los Perdidos **en las montañas**; Venides y Verina huyen de Perishton; **Edul te acompaña disfrazado de perdido** → Campaña 2.4 Bueno |
| 2.2 | Malo | **`#REF!` — el texto se perdió** |
| 2.2 | Neutro | Te unes al culto y descubres que todo era **una mentira de Sandes para matarte y satisfacer sus experimentos** → Campaña 2.4 Malo |
| **2.4** | Bueno | Boicoteas los planes → Campaña 3 |
| 2.4 | Malo | **Mueres como sacrificio.** «La campaña seguirá pero con otros personajes… no afecta en nada a la vida de los jugadores, **ni su curso con Sho-noco**» → Campaña 3 |
| 2.4 | Neutro | Si muere Edul, final medio; pierdes la confianza de Venira y Venides → Campaña 3 |

**⭐ Lo que aporta al mundo:** dos cosas de peso.

1. **Sandes es el villano real de la primera cruzada, no los Perdidos genéricos.** El texto lo dice sin rodeos: el culto de las montañas es *«una mentira de Sandes para matarte y satisfacer sus experimentos»*. Sandes, Venides, Edul y Verina son los cuatro amigos de la taberna de Ceaseton de `PARTIDAS.docx` — o sea, **la primera cruzada empieza con la traición de uno de los cuatro amigos de la infancia**, y esta hoja es el único sitio donde eso se declara como estructura y no como escena suelta.
2. **La muerte de los personajes no rompe el ciclo de Sho-Noco.** El final malo de 2.4 dice que morir sacrificado no afecta *«su curso con Sho-noco»*: implica que **el vínculo con Sho-Noco pertenece a los jugadores, no a los personajes**. Eso convierte a Sho-Noco en algo estructural del mundo (una entidad que recluta almas más allá de un cuerpo concreto), no en un recurso narrativo de una partida.

**Estado: roto en un punto y truncado.** El final Malo de 2.2 es `#REF!` (texto perdido para siempre). Las filas 14–16 son "Campaña 3" **sin ningún final escrito**. Y hay una **corrupción de datos grave: Excel convirtió las etiquetas de campaña `2.2` y `2.4` en fechas** (`2024-02-02` y `2024-04-02`). Cualquiera que reabra el archivo verá fechas donde debería ver números de campaña.

## A.7 `Objetos` (filas 1–35), `Enemigos` (1–24), `Loots` (1–7)

**Qué son:** las tres tablas de recompensa de la Campaña 1 y nada más.

- **`Objetos`**: `Nombre | Stats | Precio | Descripción | Dif Obtención | Enemigo drop | Rangos 1-10 | 11-15 | 16-19 | 20`. Seis equipos (Cultista, Venides, Edul, Venira, Sandes, Soldado), cada uno con cuatro escalones de botín según la tirada. Después, un bloque `INVENTARIO DE OBJETOS` con **exactamente dos objetos**: Armadura Sencilla (10 CA) y Túnica Sencilla (7 CA).
- **`Enemigos`**: `Enemigos | Cantidad | Experiencia Total | Grupos de 1|3|5 | Oro | Campaña | Referencia`. Diez filas. La columna `Referencia` son **enlaces a un Google Doc externo** con los bloques de estadísticas — es decir, **el bestiario nunca estuvo en este archivo**.
- **`Loots`**: seis filas, todas de la Campaña 1, calculadas desde `Enemigos`.

**Qué aportan al mundo:** poco, y es honesto decirlo. El único dato no mecánico: **los Perdidos de Boundington son pobres**. Un cultista deja *«ropas rotas y arma pequeña + 1 pieza de oro»*; 29 cultistas suman 319 piezas de oro entre todos. **Sandes solo lleva 50 de oro** pero tiene el doble de experiencia que nadie (450). Es un culto sin financiación, lo que refuerza que Sandes actúa por sus propios experimentos y no por un poder organizado detrás.

**Estado:** las tres son **plantillas con un solo caso de prueba**. `Loots` declara 1000 filas y tiene 6. `Enemigos` tiene tres `#REF!` en la columna de campaña. `Objetos` tiene el inventario general sin empezar.

## A.8 `Secretos` (una sola fila)

**Qué es:** los encabezados `Secreto | Objetivo | Recompensa` y **nada más**. **Plantilla vacía al 100%.**

## A.9 ⭐ `Progreso` (rango real: filas 1–190) — la hoja más importante del archivo después de *Niveles de Creador*

**Qué es:** el **estado geopolítico del mundo al principio y al final de cada una de las catorce Cruzadas**. Catorce bloques idénticos, cada uno con dos rejillas espejo:

| Bloque | Columnas |
|---|---|
| **Inicio** | B Nación · C Personalidad · D Arquitectura · E Leyendas y mitos · F Enemigos · G Aliados · H Calendario · I Annexo |
| **Final** | K Nación · L Personalidad · M Arquitectura · N Leyendas y mitos · O Enemigos · P Aliados · Q Calendario · R Annexo |

Las columnas S–AB están vacías.

### ⭐ La lista de naciones **encoge cruzada a cruzada**, y ahí está la historia

| Cruzada | Nº naciones | Qué cambia |
|---|---|---|
| 1ª | **16** | Bastrea, Ecla, Udrax, Mistarium, **Aegroum**, Ascaria, Gongorguma, Gliaddokx, Aeon, **Knehapnest**, Ayashii, Tabaxi, **Choubar**, **Esmua**, Bosmurg, **Sharium** |
| 2ª | 13 | **desaparecen Aegroum, Choubar y Esmua** |
| 3ª | 13 | — |
| 4ª | 11 | **desaparecen Gliaddokx y Sharium** |
| 5ª | 10 | desaparece Bosmurg |
| 6ª | 10 | desaparece Tabaxi; **vuelve Bosmurg** |
| 7ª | 9 | desaparece **Gongorguma** |
| 8ª | 8 | desaparece Bosmurg (definitivo) |
| 9ª | 7 | desaparece **Aeon** |
| 10ª | 8 | **aparece Neo Gongorguma** |
| 11ª | 7 | desaparece Mistarium |
| 12ª–14ª | 7 | Bastrea, Ecla, Udrax, Ascaria, Ayashii, Knehapnest, Neo Gongorguma |

De dieciséis naciones a siete. **Esto es un mapa político en movimiento a lo largo de las catorce Cruzadas, y es exactamente el periodo que la guía del proyecto declaraba vacío** («las cruzadas 9ª a 14ª no tienen ni una línea en todo el proyecto»). Sí la tienen: la tienen aquí, en forma de tabla.

### ⭐ Las cinco cosas concretas que la hoja afirma

1. **Aegroum se convierte en Mortuarium y luego desaparece.** Fila 9, 1ª Cruzada: *Inicio* = Aegroum, enemigo **Los Perdidos**, anexo Mortuarium. *Final* = **Mortuarium**, enemigo **Aegroum**, aliado **Gurkazaal**, anexo **Ascaria**. Es decir: los Perdidos toman Aegroum, la nación pasa a llamarse **Mortuarium** y se alía con Gurkazaal, y acaba absorbida por Ascaria. Después de la 1ª Cruzada **ni Aegroum ni Mortuarium vuelven a aparecer en ninguna de las trece cruzadas restantes**. `Partida de Rol avec Uri.docx` confirma el nombre doble («Aegroum/Mortuarium»), pero no la secuencia.
2. **Aeon se vuelve enemigo de la humanidad entera.** 8ª Cruzada, fila 120: Aeon acaba con enemigo **«Todos los mortales»** y aliado **Gurkazaal**. Desaparece en la 9ª. Los Elementales —los que cerraron las rutas interplanares en 6719— terminan del lado del corrupto.
3. **Ayashii repite el mismo destino**, tres veces: en las cruzadas 10ª, 11ª, 12ª y 13ª su estado final es **enemigo «Todos los mortales», aliado Gurkazaal**. Los yokai son la segunda nación que cae.
4. **Gongorguma cae y renace como Neo Gongorguma.** Gongorguma desaparece tras la 6ª Cruzada; **Neo Gongorguma** aparece en la 10ª y desde la 11ª es el enemigo declarado de la alianza occidental (`Enemigos: Udrax, Ecla, Bastrea`). Encaja con los calendarios: el corpus tiene un calendario *Gongorguma* y otro *Neo Gongorguma* separados por 140 años.
5. **La Gran Alianza tiene fecha de nacimiento en esta hoja.** 5ª Cruzada, fila 76 (Udrax): la columna *Leyendas y mitos* pasa de **«La arremetida de los Sagas»** (inicio) a **«La gran alianza»** (final), y en esa misma cruzada Bastrea, Ecla, Udrax y Ascaria tienen todos como enemigo **«Sagas»** y como aliados a los otros tres. **La Gran Alianza nace en la 5ª Cruzada como respuesta a una invasión saga.** Es la primera vez que el proyecto explica *por qué* existe la Gran Alianza.

Otros datos sueltos: **Ascaria se alía con los Dracónidos** a partir de la 12ª Cruzada; **Knehapnest** aparece en las catorce cruzadas (aunque no está en el mapa de la Hoja 4 —lo que obliga a matizar la conclusión de que era un residuo: será un reino sin territorio dibujado, pero es un actor político constante—); y **Sharium** es la nación de Shardmind, Shadar-kai y Drow según `Partida de Rol avec Uri.docx`, o sea **otro nombre de Ashye** — no una decimonovena nación.

⚠️ **Precaución sobre la columna `Annexo`.** Es ambigua. En seis casos (Choubar, Esmua, Tabaxi, Sharium, Gliaddokx, Bosmurg → todos con `Annexo = Gongorguma` o `Udrax`) la nación anotada **desaparece en la cruzada siguiente**, lo que sugiere «anexionada por». Pero en otros dos (Neo Gongorguma con `Annexo = Udrax`, Ascaria con `Annexo = Neo Gongorguma`) la nación anotada sigue existiendo. **No afirmo la lectura de esa columna**; sí afirmo el patrón de desaparición, que es un dato duro contable.

**Estado:** **el esqueleto está y el relleno no.** De ocho columnas por bloque, tres (`Personalidad`, `Arquitectura`, `Leyendas y mitos`) tienen **cuatro datos en toda la hoja**: «Curiosos y respetuosos» (Bastrea), «Sabios y delicados» (Gongorguma), «La arremetida de los Sagas» y «La gran alianza» (Udrax). La columna `Calendario` dice **«Despertar Elemental» en las 190 filas**, incluidas las cruzadas 10ª a 14ª, lo cual es imposible si el mundo cambia de era. Las catorce cruzadas repiten literalmente las mismas frases porque el bloque se copió y pegó.

## A.10 Las cuatro hojas `Bonificadores` (301 × 19 declaradas; contenido real hasta la fila ~72, más una columna de escala hasta la 250)

**Qué son:** cuatro calculadoras de personaje idénticas. Columnas: `Tiradas | Bonificadores` (tabla de conversión de puntuación a modificador, de 0-1 → −7 hasta 70-71 → +14), un bloque de ficha (`Fue Des Con Int Sab Car`), tres bloques de suma (`Resultado final`, `Bonificación por raza`, `Bonificación por clase`) y una columna `LVL / Bonus` que escala del nivel 1 al 250.

**He comparado las cuatro celda a celda: difieren en 46 celdas de 5.719, y las 46 son la ficha de atributos.** El andamiaje matemático es idéntico. Es decir: **no hay ninguna diferencia mecánica entre Bastrea y Ascaria más allá del reparto de puntos.**

Ese reparto sí dice algo:

| Bonificación por raza (a nivel 30) | Fue | Des | Con | Int | Sab | Car |
|---|---|---|---|---|---|---|
| **Bastrea** | **+32** | **+31** | −1 | 0 | **+30** | 0 |
| **Ascaria** | 0 | −1 | 0 | **+32** | **+31** | 0 |

**Qué aporta al mundo:** confirma numéricamente el contraste cultural — **Bastrea entrena el cuerpo, Ascaria entrena la mente, y las dos comparten la Sabiduría**. Ese solapamiento en Sabiduría es interesante: son dos formas distintas de la misma virtud (el bastreo la aplica al elemento, el ascario al arcano). También hay una **discrepancia interna que conviene resolver**: `Clases Razas` da a Bastrea `FUE+SAB+DES = +3, +2, +1` y aquí se aplica `FUE+3, DES+2, SAB+1`. Sabiduría y Destreza están intercambiadas entre las dos hojas.

**Estado:** funcionales las cuatro, con el mismo defecto cosmético que `Finales`: **Excel convirtió los rangos de tirada `2-3`, `4-5`, `6-7`, `8-9`, `10-11` en fechas** (`2024-03-02`, `2024-05-04`…). Las cinco primeras filas de la tabla de bonificadores son ilegibles en pantalla.

---

# B. `Ciudades y sus elementos.xlsx`

## B.1 `Hoja 2` (rango real: filas 1–91) — **la tabla de eventos, ahora con sus columnas**

**Qué es:** la cronología estructurada del proyecto, **89 eventos fechados de 9071 a 3059 b.f.** Es la fuente que rellena el «hueco mayor 8487–3000» de la cronología.

**Columnas exactas (20):**

| Col | Cabecera | Cuántas filas la usan |
|---|---|---|
| A | `Año` | 90 |
| B | `Evento` | 90 |
| C | `Nación` | 90 |
| D | `Ubicacion #1` | 67 |
| E | `Ubicacion #2` | 21 |
| F | `Ubicacion #3` | 5 |
| G–T | `Razas #1` … `Razas #14` | 60, 54, 39, 26, y ≤4 el resto |

**⭐ El hallazgo estructural:** la cabecera dice `Razas` pero **el contenido son clanes**. `Razas #1..#4` casi nunca contienen razas: contienen *Martelys, Runescribe, Bronzeborn, Silvershield* (enanos), *Zantox, Fliark, Gorthak, Bleek* (goblins), *Shinken, Kazehana, Hikarikami, Honoun* (yokai), *Ignífera, Aqualis, Aerium, Terranox* (elementales), *Cara Loca, Zarpa Sombría, Colmillo Dorado, Puño de Hierro* (hombres gato). Es decir: **la hoja registra sistemáticamente qué clanes participaron en cada evento**, y lo hace en cuatro huecos por evento. Solo la fila 2 (*La Primera Gran Migración*) usa las catorce columnas, y ahí sí son razas: las **catorce razas que migraron juntas** (Yokai, Humanos, Goblin, Orcos, Hombres Lagarto, Hombres Pájaro, Sagas, Nagas, Dracónidos, Enanos, Minotauros, Elementales, Hombres Gato, Elfos). Esa fila es el censo racial fundacional del mundo, y explica por qué la tabla tiene catorce columnas.

**Datos únicos que aporta (una selección de lo que no está en otras fuentes):**
- **Los cuatro clanes elementales de Aeon tienen nombre**: *Ignífera, Aqualis, Aerium, Terranox* (5961, *Inicio de las Guerras del Fuego*), y una segunda generación en 4705: *Pyrestorm, Hydrelith, Terraforge, Aerisong*, y una tercera en 3909: *Flamígero, Oceánico, Roca Viva, Aliento Celestial*. Son fuego/agua/tierra/aire en tres capas históricas.
- **Mirathis aparece como clan de Ascaria** (5912, Construcción de la Muralla, junto a *Kaelid* y *Eldrayne*). Mirathis es el nombre del ser asesinado por Egos en `Ayashii.docx`. O es homónimo o hay un linaje ascario que lleva ese nombre.
- **Tres clanes de Bastrea con nombre catalán**: *Freixes de Foc, Guillemot Velmont, Sabater Caballé* (6002, Guerra del Pantano). Encaja con la estética catalana documentada de Bastrea.
- **«Actual Perdidos»** figura como clan de Aegroum en 3105, junto a *Forjamundo*. Es la primera datación del origen de los Perdidos: eran un clan de contacto pacífico entre goblins y humanos.
- **Nocturnsea aparece como *ubicación* de un evento de Aeon** (3878, *El azote a los hijos de las Guerras del Fuego*), no como nación. Es el único evento fechado que la nombra.

**Estado:** completa y utilizable, con dos defectos: **la fila 27 está vacía en medio de la secuencia**, y la **fila 50 tiene un duplicado desplazado** (*La Marcha de las Cuatro Estaciones* repetida en las columnas L–T con el año **7800** en lugar de **5809**; alguien pegó la fila en las columnas equivocadas). Las filas 2–9 tienen relleno de color, una por color, y a partir de la 10 no; **es un formateo empezado y abandonado, no un código** (Udrax y Choubar comparten color, así que no puede ser un código de nación).

## B.2 `Hoja 3` (rango real: filas 1–87 con datos; 88–251 son basura de fórmula)

**Qué es:** **una vista derivada de la Hoja 2, no una fuente**. Las columnas A, B y G son referencias directas (`='Hoja 2'!A2`) y las C–F son concatenaciones (`="Clanes de los " & 'Hoja 2'!G2`). Columnas: `Año | Evento | Clanes | Clanes | Clanes | Clanes | Ubicación | Pais`.

**Qué aporta:** una sola cosa, y es real. **La fila 2 contiene un evento que no existe en la Hoja 2 ni en ninguna otra fuente estructurada:**

| Año | Evento | Ubicación | País |
|---|---|---|---|
| **9XXX** | **Gran Quebración** | **Egaroth** | (Ayashii — erróneo) |

Está escrita a mano, por encima de la primera fila de fórmulas, con el año sin determinar (`9XXX`) y **ubicación «Egaroth»**, es decir, el mundo entero. Es el evento que abre la tabla: **la Gran Quebración es el suceso anterior a la Primera Gran Migración de 9071**. La cronología maestra menciona una «Quebración»; esta hoja es la única que la fecha (aproximadamente) y la localiza.

**Estado: rota.** Tres problemas:
1. La columna **`Pais` está tecleada a mano y desalineada una fila**, y a partir de la fila 6 **dice «Ayashii» en las 246 filas restantes**, incluidas todas las que hablan de Udrax, Ascaria o Aeon. Es literalmente falsa en el 95 % de la hoja.
2. La hoja **solo copia 4 de las 14 columnas de clanes** de la Hoja 2: los clanes 5º a 14º de cada evento se pierden.
3. Hay **`#REF!` en las filas 34, 53, 54, 55 y 72**, y **164 filas de relleno** con el texto `"Clanes de los "` sin nada detrás.

**Recomendación:** la Hoja 3 no aporta nada que la Hoja 2 no tenga, salvo la Gran Quebración. Rescatar esa fila y archivar la hoja.

---

# C. `Campañas/Ciudades.xlsx`

## C.1 `Hoja 1` (rango real: filas 1–62) — el **gazetteer de la Primera Cruzada**

**Qué es:** no es una lista de ciudades: es un **plano funcional de nueve localizaciones jugables**, cada una desglosada en seis o siete zonas. Estructura repetida por bloque:

| Fila del bloque | Contenido |
|---|---|
| cabecera | Nombre del lugar → sus 6-7 zonas como cabeceras de columna |
| `Ubicación Principal` / `Secundaria 1` / `Secundaria 2` | qué hay en cada zona |
| **`Ubicación Secreta / Oculta`** | lo que solo se encuentra buscando |
| `Personajes Clave` | quién está allí |
| `Encuentro Importante / Combate Clave` | qué pasa |

**Los nueve lugares:** Boundington (756 d.e., Aegroum) · **Palacio Real de Ranmont** · Bosque de Ventus Oscuro · Santuario de Doomsbury · **Finesaux** (capital de Ascaria) · **Velours** · Doomsbury asediado · **Wolfwater** (campamento de Bastrea) · **Santas Montañas** · **Ranmont bajo los Perdidos**.

**⭐ Datos únicos que aporta al mundo:**
- **El Palacio Real de Ranmont tiene seis niveles con nombre**: 1ª Planta (entrada y salón de banquetes), 2ª (habitaciones reales), 3ª (sala del consejo), 4ª (torre del vigía), Subterránea 1 (prisión), **Subterránea 2 (catacumbas reales)** — y en las catacumbas están la **Sala de las Estatuas de los Reyes**, la **Tumba del Primer Rey**, la **Cámara de los Secretos del Reino** y una **Sala del Ritual Oscuro**.
- **La corte de Aegroum al completo**: **Rey Aldon**, **Reina Lysandra**, **Princesa Xeamia** (la protagonista de *Huida de Ranmont*), el **Consejero / Duke Nefaros**, **Sir Galdrin**, el **Capitán Rhomar**, y una **«Alcoba Oculta de Xila»** en el piso de las habitaciones reales. Nefaros aparece a la vez como *Duke* en la entrada y como *Consejero* en la sala del consejo: es el traidor de palacio.
- **Riu'jin**, líder final de los Perdidos y «maestro de la magia de Gurkazaal», con base en el **Templo de los Perdidos** de las Santas Montañas y después en el **Palacio de los Perdidos** de Ranmont. Es el antagonista con nombre de toda la Primera Cruzada.
- **La Hechicera Irae**, aliada de las Santas Montañas; **Anara la Suma Sacerdotisa** y **Eldon el Vigilante** en Doomsbury; el **Capitán Joros** de la resistencia; **General Roger de Flor** y **Sargento Torgrim** en Wolfwater; **Villena, hijo de Venides**, que dirige el asedio final; **Lady Helena** y **Dalgor** en Finesaux; **Skila**, tabernera y antigua aventurera, y **Aigren** el herrero, en Boundington.
- Un detalle teológico: en el Bosque de Ventus Oscuro hay un **Altar de los Creadores** junto al **Árbol Ancestral** y un **Manantial de la Vida**; en Doomsbury hay una **Estatua de los Creadores** y **frescos de los Creadores**. Es la única descripción física de culto a los Creadores en todo el corpus.
- La estructura de Boundington en **siete barrios** —Casco Antiguo, Barrio Militar, Barrios Altos, Distrito Comercial, Surysal, Pico Dragón, Barriada— coincide con los mapas tácticos de `PARTIDA 1/`.

**Estado:** **completa y de calidad**, la mejor hoja de los tres archivos. 62 filas densas, sin errores. Está infrautilizada: es material listo para jugar y para escribir.

## C.2 `Hoja 2` (33 × 2) — la lista canónica de capitales

**Qué es:** dos bloques en la misma tabla `Sitios | Descripcion`. Filas 2–16: los **quince lugares de Boundington** con su descripción de una línea. Filas 17–33: las **17 capitales de reino**, con la fórmula `Capital -> Reino` y la etiqueta «Capital del reino».

Ya está recogida en `informe_agente2_razas.md`; la confirmo sin cambios (Guskedor, Numandum, Finesaux, Ranmont, Havar'gruztak, Umedan, Bomengrid, Naka't-ol, Dhin Thyraxion, Tor'k Hazar, Saif-l'sa, Zathor'aetz, Klimnebra, Venordemn, **Ciudad de Grytoz → Nocturnsea**, Conclave Elemental → Aeon, Himetsumota (Templo) → Ayashii).

**Dato de Boundington que sí es nuevo:** la **Iglesia de Santa Sofía** es *«antigua iglesia usada por los Perdidos, dedicada a Sofía, **patrona local amante de la cerveza**»*, y bajo ella está **La Sima Oscura**, la bóveda oculta que es la guarida del culto. Que Sofía —la Creadora del conocimiento— sea venerada localmente como patrona de la cerveza es el mejor ejemplo del proyecto de **cómo un culto universal se deforma al bajar a una ciudad concreta**. Y encaja con que Aegroum exporte «agua lupulada» a todo el continente.

**Estado:** completa.

## C.3 ⭐ `Hoja 3` (17 × 87) — **descifrada**

Un análisis anterior la declaró «indescifrable: tabla de porcentajes (N)/(P) sin cabecera». **Sí es descifrable, y la respuesta es prosaica: no es una tabla de la hoja de cálculo, es una tabla de texto pegada.**

**Qué pasó.** Alguien copió una tabla escrita con barras verticales (formato ASCII/Markdown, `| 11 | 60 (N) | 40 (P) |`) y la pegó en Excel. Excel **partió el texto por cada `|`** y repartió cada trozo en una columna distinta. De ahí las 87 columnas: no son 87 campos, son los fragmentos de **una sola línea de texto por fila**. Las columnas `A, C, F, I, J, L…` contienen literalmente el carácter `|`.

**La tabla original, reconstruida.** Filas impares 1–17 = los nueve escalones de decena (10, 20, 30 … 90). Cada fila contiene los diez resultados de esa decena:

| Tirada | (N) | (P) |
|---|---|---|
| x0 | *ver abajo* | *ver abajo* |
| x1 | 60 | 40 |
| x2 | 50 | 50 |
| x3 | 50 | 50 |
| x4 | 40 | 60 |
| x5 | 40 | 60 |
| x6 | 30 | 70 |
| x7 | 30 | 70 |
| x8 | 20 | 80 |
| x9 | **entre** 20 / 80 **y** 0 / 100 |

Y los nueve valores de decena, que son los únicos que cambian de fila a fila y están expresados **sobre 10, no sobre 100**:

| 10 | 20 | 30 | 40 | 50 | 60 | 70 | 80 | 90 |
|---|---|---|---|---|---|---|---|---|
| 7 / 3 | 8 / 2 | 9 / 1 | 7 / 3 | 9 / 1 | 8 / 2 | 9 / 1 | 8 / 2 | **10 / 0** |

**Qué es, entonces:** una **tabla de d100** que reparte un resultado entre dos categorías, **N** y **P**. Cuanto más alta la tirada dentro de la decena, más se inclina hacia **P** (de 60/40 en x1 a 0/100 en x9). Es una tabla de gradiente, no de resultado binario.

**Lo que he probado y he descartado.** Crucé el 17 y el 87 con todas las listas conocidas: 17 capitales, 18 territorios, 21 razas, 12 meses, 8 Creadores, 24 clases, 7 elementos, 3 lunas, 14 cruzadas. **Ninguna encaja**, y ahora se sabe por qué: **el 87 y el 17 no son números de contenido, son artefactos del pegado** (87 fragmentos por línea, 9 líneas con una fila en blanco entre cada una = 17). El intento de cruce estaba condenado desde el principio.

**Lo que sigue sin saberse: qué son N y P.** He buscado la leyenda en el propio archivo (no hay cabecera, ni comentarios, ni nombres definidos, ni texto en los tres `drawing.xml`, todos vacíos) y en las 463.000 palabras del corpus extraído (`grep` de `(N)`, `(P)`, `60 (N)` y variantes: la única aparición en todo el proyecto es esta misma tabla, reexportada). **No está escrito en ninguna parte.** Por contexto —las otras dos hojas del archivo son íntegramente material de Boundington y de los Perdidos— las dos lecturas plausibles son `N/P = Negativo/Positivo` (una tabla de reacción o de persuasión) o `N/P = Neutrales/Perdidos` (qué proporción de una población está ya captada por el culto). **No afirmo ninguna de las dos**; es una pregunta de una línea para el autor.

**Estado: descifrada estructuralmente, ilegible semánticamente.** Además, **ocho de las nueve filas son duplicados literales**: el patrón de las unidades (60/40, 50/50, 50/50, 40/60, 40/60, 30/70, 30/70, 20/80) es **idéntico en las nueve decenas**. Solo cambia la etiqueta de la fila y el valor de la decena. Y la fila del 90 tiene un error: donde todas las demás dicen `x1 → 60/40`, la del 91 dice **`0 (N) | 10 (P)`**, que no suma 100 ni sigue el patrón. La tabla útil de verdad cabe en **diez líneas**.

---

# LO QUE APORTA AL MUNDO

Solo lore. Nada de dados.

1. **Los ocho Creadores tienen, en el quinto nivel de comunión, un poder ligado a un lugar real de Egaroth.** Sofía ↔ **Ekait** y **Ranmont**. Envidia ↔ **Tabaxi** y **Ashye**. Vida ↔ **Numandum** y **Argent**. Muerte ↔ **Perishton** y **Nocturnsea**. Egos ↔ **Bomengrid**. Eros ↔ **Ayashii**. Es un mapa de geografía sagrada que no existe en ningún otro documento. **Gurkazaal y Chronos son los únicos sin lugar**: sus poderes máximos son abstractos (Castigo, Purgatorio, Canción del tiempo). El corrupto y el señor del tiempo no tienen tierra.

2. **Ninguna raza declara afinidad con Gurkazaal.** La tabla de afinidades tiene siete Creadores; Gurkazaal hubo que teclearlo aparte en la tabla de niveles. Se le puede servir, pero no se nace suyo.

3. **Sho-Noco concede los niveles de Creador.** Las tres filas amarillas del plan de campañas son interludios llamados «Sho-Noco» y su contenido es *Primer / Segundo / Tercer nivel Creador*. La entidad que materializa a los aventureros y les muestra visiones es la misma que abre el canal con los dioses. Y en el final malo de la campaña 2.4 se dice que morir *«no afecta en nada… ni su curso con Sho-noco»*: **el vínculo con Sho-Noco sobrevive a la muerte del personaje.**

4. **La Primera Cruzada empieza con la traición de un amigo de la infancia.** El árbol de finales identifica a **Sandes** —uno de los cuatro de la taberna de Ceaseton— como el autor del engaño: el culto de las Santas Montañas es *«una mentira de Sandes para matarte y satisfacer sus experimentos»*. Los Perdidos son el decorado; Sandes es el móvil.

5. **Aegroum cae, se llama Mortuarium y desaparece del mapa.** En la 1ª Cruzada los Perdidos la toman, la nación pasa a llamarse **Mortuarium** y se alía con **Gurkazaal**; después es absorbida por **Ascaria**. En las trece cruzadas siguientes **ni Aegroum ni Mortuarium vuelven a existir**.

6. **La Gran Alianza nace en la 5ª Cruzada contra una invasión saga.** La columna de mitos de Udrax pasa de *«La arremetida de los Sagas»* a *«La gran alianza»* dentro de la misma cruzada, y Bastrea, Ecla, Udrax y Ascaria comparten enemigo (Sagas) y se declaran aliados entre sí. Es la primera explicación causal de la Alianza en todo el proyecto.

7. **Aeon y Ayashii acaban del lado de Gurkazaal.** Aeon en la 8ª Cruzada y Ayashii de la 10ª a la 13ª, ambos con el mismo texto: enemigo **«Todos los mortales»**, aliado **Gurkazaal**. Los Elementales y los Yokai, las dos naciones no humanas más antiguas del mundo, terminan contra los mortales.

8. **Gongorguma cae y renace como Neo Gongorguma**, que desde la 11ª Cruzada es el enemigo declarado de Udrax, Ecla y Bastrea. Explica por qué existen dos calendarios separados por 140 años.

9. **El mundo pasa de dieciséis naciones a siete a lo largo de las catorce Cruzadas.** Desaparecen, por este orden: Aegroum, Choubar y Esmua (tras la 1ª); Gliaddokx y Sharium (tras la 3ª); Bosmurg y Tabaxi (5ª–6ª); Gongorguma (7ª); Aeon (8ª); Mistarium (10ª). Quedan Bastrea, Ecla, Udrax, Ascaria, Ayashii, Knehapnest y Neo Gongorguma. **Las cruzadas 9ª a 14ª, que la guía daba por inexistentes, están documentadas aquí.**

10. **La «Gran Quebración» es el evento que abre la historia del mundo habitado**, anterior a la Primera Gran Migración de 9071, con ubicación «Egaroth» —el mundo entero— y año sin fijar (`9XXX`).

11. **Catorce razas migraron juntas en 9071**: Yokai, Humanos, Goblins, Orcos, Hombres Lagarto, Hombres Pájaro, Sagas, Nagas, Dracónidos, Enanos, Minotauros, Elementales, Hombres Gato y Elfos. Es el censo racial fundacional, y es la razón de que la tabla de eventos tenga catorce columnas de raza.

12. **Los Elementales de Aeon tienen tres generaciones de clanes con nombre:** *Ignífera, Aqualis, Aerium, Terranox* (5961) → *Pyrestorm, Hydrelith, Terraforge, Aerisong* (4705) → *Flamígero, Oceánico, Roca Viva, Aliento Celestial* (3909). Fuego, agua, aire y tierra, renombrados tres veces a lo largo de dos mil años.

13. **Los Perdidos tienen origen datado.** En 3105, en *Contacto pacífico entre los Goblins de Gliaddokx y los Humanos de Aegroum*, uno de los dos clanes se llama literalmente **«Actual Perdidos»**. Nacieron de un contacto pacífico.

14. **Sofía es, en Boundington, «patrona local amante de la cerveza»**, y su iglesia es la que ocupan los Perdidos, con la guarida del culto en la cripta. Una Creadora del conocimiento convertida en santa cervecera de barrio — y justo debajo, el culto oscuro.

15. **El Palacio Real de Ranmont tiene seis niveles**, con la corte al completo (Rey **Aldon**, Reina **Lysandra**, Princesa **Xeamia**, Consejero **Nefaros**, Sir **Galdrin**, Capitán **Rhomar**) y, en las catacumbas, la **Tumba del Primer Rey**, la **Cámara de los Secretos del Reino** y una **Sala del Ritual Oscuro**. **Riu'jin** es el líder final de los Perdidos y acaba ocupándolo.

16. **Bastrea es cuerpo y Ascaria es mente, y comparten la Sabiduría.** Las cifras lo dicen sin ambigüedad (Fue 76 vs 14 a nivel 30). Dos formas de la misma virtud: el bastreo aplica la sabiduría al elemento, el ascario al arcano.

17. **Cada clase tiene oficio civil y moralidad asignados.** El pícaro es *actor*; el druida, *boticario y leñador*; el chamán, *monje*; el artificiero, *profesor*. Y el sistema Luz/Sombra está formalizado: el Paladín es fuerte en Luz y débil ante las Sombras, el Brujo exactamente al revés.

18. **Sharium no es una nación nueva:** es Ashye con otro nombre (Shardmind + Shadar-kai + Drow, según `Partida de Rol avec Uri.docx`). **Knehapnest sí es un actor político real**: aparece en las catorce cruzadas, aunque no tenga ni una celda en el mapa. Habrá que decidir si es un reino sin territorio o un error persistente.

---

# LO QUE ESTÁ ROTO O VACÍO

**Roto de verdad (pérdida de datos):**

- **`Finales`, fila 9**: el desenlace **Malo de la Campaña 2.2 es `#REF!`**. El texto se perdió y no se puede recuperar.
- **`Ciudades y sus elementos` Hoja 3, columna `Pais`**: tecleada a mano, **desalineada una fila** y a partir de la fila 6 **dice «Ayashii» en las 246 filas restantes**. Es falsa en el 95 % de la hoja. Además pierde 10 de las 14 columnas de clanes de la Hoja 2 y tiene `#REF!` en cinco filas.
- **`Ciudades y sus elementos` Hoja 2, fila 50**: *La Marcha de las Cuatro Estaciones* está **duplicada en las columnas L–T con el año 7800** en vez de 5809. Fila pegada en las columnas equivocadas.
- **`Enemigos`**: tres `#REF!` en la columna de campaña, y el bestiario real **nunca estuvo en el archivo** — la columna `Referencia` son enlaces a un Google Doc externo.
- **Corrupción por autoformato de fecha, en dos sitios:** en `Finales`, las campañas **`2.2` y `2.4` se convirtieron en `02/02/2024` y `02/04/2024`**; en las cuatro hojas de `Bonificadores`, los rangos de tirada **`2-3`, `4-5`, `6-7`, `8-9`, `10-11` se convirtieron en fechas**. Se arregla formateando esas celdas como texto y reescribiéndolas.
- **`Ciudades.xlsx` Hoja 3**: la fila del 90 rompe su propio patrón (`0 (N) | 10 (P)` donde debería ir `60/40` o `0/100`). Y **ocho de las nueve filas son duplicados literales** de la primera.

**Vacío o plantilla sin usar:**

- **`Secretos`**: tres cabeceras y **cero filas**. Vacía al 100 %.
- **`Niveles de Creador`, nivel 4**: las ocho filas son `-`. Y **el nivel 5 no tiene ni una sola descripción de efecto** — 24 nombres de poder sin decir qué hacen. Los niveles 2 y 3 no son contenido: son concatenaciones de texto (`"Sacrificio" & " de Nivel 2"`).
- **`Campañas`**: de doce columnas, **seis están enteramente vacías** (NPCs, Oro, EXP, Enfrentamientos, Objetos, Total). Solo dos de las dieciséis campañas tienen año.
- **`Progreso`**: el esqueleto de las catorce cruzadas está, pero las columnas `Personalidad`, `Arquitectura` y `Leyendas y mitos` tienen **cuatro datos en 190 filas**. La columna `Calendario` dice **«Despertar Elemental» en las 190 filas**, lo cual es imposible cuando el mundo cambia de era, y los catorce bloques son copias literales unos de otros.
- **`Clases Razas`**: la tabla de clases tiene **12 de las 24 clases** del proyecto. Faltan todas las homebrew.
- **`Objetos`**: el bloque `INVENTARIO DE OBJETOS` tiene **dos objetos**.
- **`Loots`**: **6 filas de las 1000** que declara, todas de la Campaña 1.
- **`Index`**: está desactualizado (no lista siete de las hojas que existen) y anuncia una **`Campaña 3` que nunca se escribió** — la misma que en `Finales` tiene tres filas reservadas y ningún final.

**Incoherencias a decidir (no son errores, son elecciones pendientes):**

- **Bastrea, dos repartos distintos.** `Clases Razas` da `FUE+3, SAB+2, DES+1`; los `Bonificadores` aplican `FUE+3, DES+2, SAB+1`. Sabiduría y Destreza están intercambiadas.
- **La columna `Annexo` de `Progreso` es ambigua**: en seis casos se comporta como «anexionada por» (la nación desaparece a la cruzada siguiente) y en dos no. Hace falta una convención.
- **Colisiones de paleta**: Gurkazaal usa el mismo `#F4CCCC` que Ascaria en el mapa, y Sofía el mismo `#FFF2CC` que Aegroum y Ayashii.
- **`Ciudades.xlsx` Hoja 3**: falta saber **qué son N y P**. Es la única pregunta que queda de esa hoja, y son treinta segundos de respuesta para quien la escribió.
