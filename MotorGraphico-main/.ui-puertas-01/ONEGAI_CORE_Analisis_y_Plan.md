# Onegai — Análisis de los tres proyectos y diseño del entorno común

*Revisión 2 · 11 de agosto de 2026 · medido directamente sobre el disco, no sobre la documentación de cada proyecto*

> **Qué cambia respecto a la revisión 1.** La primera versión trató el motor de cartas como una fuente de lore contaminada de arriba abajo. Era una lectura incompleta: auditó los campos `faction` y `location` de las cartas y pasó por alto `data/mapa/geografia.json`, que es donde vive el mapa del mundo. Ese fichero **sí es canon**, es el activo más valioso del motor de cartas, y —esto es lo grave— **no se exporta a ningún sitio**. La sección 2.1 está reescrita en consecuencia, y con ella la estrategia de reconciliación.

---

## 0. La conclusión, primero

Los tres proyectos **ya están acoplados**, pero en la dirección equivocada y por los sitios equivocados.

No son tres productos que haya que unir. Son **tres consumidores de un mismo activo** —el mundo de Egaroth— y hoy cada uno guarda una copia privada y parcial de ese activo.

El reparto real de quién aporta qué, una vez medido:

| Proyecto | Lo que aporta y nadie más tiene | Lo que **no** debe aportar |
|---|---|---|
| **Onegai (documental)** | El lore. Siempre y sin excepción: entidades, cronología, panteón, naciones, personajes. | — |
| **dndWeebCC (cartas)** | **La lógica del juego** (reglamento, fórmulas, tiers, pilas) y **la geometría del mapa**: 19 polígonos de nación, 95 ciudades con coordenadas, 26 zonas. Es la única representación del mundo con geometría real que existe. | Lore. Cada vez que ha inventado mundo —facciones, aldeas, deidades renombradas— ha creado deuda. |
| **MotorGraphico** | Ejecución. Nada canónico, y eso es lo correcto. | Reglas propias. Ya implementó unas. |

De ahí salen los tres problemas, en orden de gravedad:

1. **El mapa canónico no llega a ninguna parte.** `geografia.json` no está entre los 20 catálogos exportados al motor gráfico. El juego no tiene mundo.
2. **El motor de cartas inventó un micro-setting** (16 facciones, 29 localizaciones) que clavó *encima* del mapa real y que sostiene 432 enemigos, 201 tablas de botín y 54 aventuras. No es un mundo paralelo: es contenido sin bautizar.
3. **El reglamento está implementado dos veces**, en Java y en C++, sin ninguna prueba que garantice que dan el mismo resultado. Y ya han divergido.

El entorno que hace falta no es un monorepo ni un bus de integración. Es **una fuente de verdad con dirección de flujo única** y **un banco de pruebas de conformidad**. Todo lo demás es consecuencia de esas dos piezas.

Las cifras que resumen el estado:

| | |
|---|---:|
| Naciones del mapa de cartas que son canónicas | **19 de 19** |
| Capitales del mapa que están sin nombrar, teniendo el canon el nombre real | **7** |
| Clases con hechizos iniciales | **2 de 61** |
| Hechizos que no puede lanzar ninguna clase | **202 de 210** |
| Aventuras exportadas al motor gráfico que este puede ejecutar | **0 de 78** |
| Avisos de degradación en la última exportación de datos | **1.795** |

---

# PARTE I — QUÉ HAY REALMENTE

## 1. Los tres proyectos, medidos

### 1.1 `dndWeebCC-master` — el motor de cartas

Spring Boot 3.2.5 / Java 17 / Thymeleaf. **17.378 líneas de Java**, 146 clases, 28 controladores, 24 servicios, 83 plantillas, 13 hojas CSS.

Es, con diferencia, **el proyecto más maduro de los tres en contenido**. El catálogo en disco:

| Tipo | Ficheros | | Tipo | Ficheros |
|---|---:|---|---|---:|
| habilidades | 483 | | monturas | 50 |
| enemigos | 432 | | trampas | 51 |
| armas/equipo | 413 | | razas | 43 |
| hechizos | 210 | | consumibles | 31 |
| invocaciones | 201 | | condiciones | 15 |
| pasivas | 66 | | transfondos | 12 |
| clases | 61 | | deidades | 10 |
| | | | dotes | 6 |
| | | | rasgos | 2 |

Más, fuera de `cartas/`: **302 historias**, **201 tablas de botín**, **134 PNJs**, **54 aventuras**, 12 personajes. Total: **2.816 ficheros JSON, 12 MB**.

El reglamento está especificado con un rigor poco común: `docs/Sistema_Cartas_Tiers.md` tiene 20 secciones con fórmulas cerradas (vida, defensas, techos por tier, pool de d6, pilas de recuperación). Y está **implementado**: `cat.dnd.cc.combat.motor` contiene `TiradaD6`, `Pila`, `EstatPiles`, `Recovery`, `Condicio`, `OrdreIniciativa`, `SalvacioPerMort`, `Avantatge`, `NivellFatiga` — con inyección de `Random` para determinismo en tests. Es trabajo serio.

Rama activa: `development`. Último commit: motor de combate completo (WS-D) y validador de aventuras (WS-E).

### 1.2 `MotorGraphico-main` — el motor gráfico / GameMachine

C++17 / OpenGL 3.3, isométrico, pixel art. **14.419 líneas** entre `src/` e `include/`, **28 demos ejecutables**, CMake, CI con sanitizers, `clang-format` y `clang-tidy`.

Fases 1–12 del Gantt completas: ventana, sprite batch, entidades animadas, post-procesado (framebuffer, niebla de guerra, LUT de color), parser JSON propio sin dependencias, carga de niveles TMX, skills, combate por turnos, HUD compuesto, catálogo de objetos, luces dinámicas, editor de niveles.

Sobre eso hay un módulo `RPG/` que es el intento de convertir el motor en *GameMachine Onegai*: `DicePoolEngine`, `ConditionEngine`, `InventoryEngine`, `CharacterSheet`, `TierRules`, `NarrativeEngine`, `SkillExecutor`.

Su documentación interna es notablemente autocrítica y merece la pena señalarlo, porque marca el estándar del resto de este informe: `ARCHITECTURE.md` documenta **7 fracturas de coherencia** detectadas por análisis morfológico, con la lección aprendida de cada una. `GAMEMACHINE_NECESIDADES.md` es un mapa de brecha de 24 subsistemas contra el GDD, con 18 huecos bloqueantes. **Ese documento es la mejor pieza de análisis que existe en los tres proyectos** y buena parte de lo que sigue lo confirma con datos.

No hay repositorio git inicializado.

### 1.3 `Onegai 2` — el proyecto documental / lore

20.388 ficheros, 3,1 GB. Pero eso engaña: descontando `node_modules`, entornos virtuales, StoryCraftr y las imágenes extraídas del timeline, quedan **2.108 ficheros**, y el material narrativo real son **339 documentos** (`.docx`, `.pdf`, `.md`, `.xlsx`) entre el vault de Obsidian y el corpus extraído.

Lo que ya está construido y es valiosísimo:

- **`Onegai_Cronologia_Maestra.md`** — 285 eventos con ID estable (`EV-…`), 17 eras, de la Cosmogonía al 300 a.f. Es el único sitio donde toda la historia está en un mismo orden.
- **`indice_personajes.md`** — el primer censo completo: 322 entradas según su propio recuento, con estado (`desarrollado` / `mencionado` / `solo nombre`), fuentes cruzadas y **colisiones marcadas en rojo**. 173 son solo un nombre: esa es la lista de trabajo.
- **`indice_lugares.md`** — 69 localizaciones.
- **`GUIA_DEL_PROYECTO.md`** — guía de "dónde está cada cosa", orientada a preguntas y no a carpetas.
- **`corpus_extraido/`** — 96 documentos convertidos a Markdown, ya legibles por máquina.
- Infraestructura de IA local (Ollama + dos agentes, AuthorAgent, PanelOnegai) para escritura y crítica.

**Esto es lo más importante de todo el análisis:** los índices ya construidos son, de facto, **el registro de entidades canónicas que el entorno necesita**. El trabajo duro —leer 339 documentos, unificar grafías, detectar contradicciones— ya está hecho. Lo que falta es convertirlo de prosa a datos estructurados, y eso es una fracción del esfuerzo.

---

## 2. Las fracturas

### Fractura 1a — El mapa canónico existe, es bueno, y no llega a ninguna parte

**Esta es la que más cuesta ver, porque el activo está sano y aun así no sirve de nada.**

`dndWeebCC/data/mapa/geografia.json` (43 KB) contiene el mundo de Egaroth con geometría real:

| | |
|---|---:|
| Naciones con polígono SVG y vértices editables | 19 |
| Ciudades con coordenadas x/y, tamaño, población y gobernante | 95 |
| Zonas de terreno (montaña, bosque, desierto, santuario…) | 26 |

Y es **canon**: las 19 naciones —Ecla, Ostad, Aegroum, Gliaddokx, Bastrea, Ascaria, Tabaxi, Wulcain, Aeon, Ayashii, Mistarium, Gongorguma, Ashye, Nocturnsea, Esmua, Bosmurg, Choubar, Udrax— son exactamente las del índice de lugares de Onegai. Las 5 zonas de santuario son Himetsumota, el Templo de Sofía, el Templo de Chronos, el Templo de Envidia y la Torre del Viento. Nada de esto está inventado.

Compárese con el propio mapa del proyecto documental, `Mapa_Egaroth.svg`: tiene 74 etiquetas de texto y **cero polígonos, cero círculos**. Es una lista de nombres colocados. **El motor de cartas tiene la única representación geométrica del mundo que existe en los tres proyectos.**

Y aquí está la fractura: **`geografia.json` no está entre los 20 catálogos que se exportan al motor gráfico.** Ni ese fichero ni ningún equivalente. El motor gráfico tiene 3 niveles de ciudad genéricos (`ciudad_centro`, `ciudad_este`, `ciudad_oeste`) y ningún concepto de nación, mundo ni localización narrativa. El activo más valioso del motor de cartas **muere en el motor de cartas**.

#### Y el canon puede devolver el favor ahora mismo

Cruzando las capitales de `geografia.json` contra `Mapa_Egaroth.svg`:

| Nación | Canon (Onegai) | Mapa de cartas | |
|---|---|---|---|
| Aegroum | Ranmont | Ranmont | ✅ |
| Ascaria | Finesaux | Finesaux | ✅ |
| Ecla | Umedan | Umedan | ✅ |
| Udrax | Bomengrid | Bomengrid | ✅ |
| Gongorguma | Guskedor | Gusku**e**dor | 🔵 variante ortográfica |
| Bastrea | N**u**mandum | N**i**mandum | 🔵 variante ortográfica |
| Gliaddokx | Havar'gruztak | Grahgan | 🔴 conflicto real |
| Mistarium | **Venordemn** | `Capital` | 🟡 el canon tiene el nombre |
| Bosmurg | **Tor'k Hazar** | `Capital III` | 🟡 |
| Choubar | **Klimnebra** | `Capital II` | 🟡 |
| Esmua | **Saif-l'sa** | `Capital IV` | 🟡 |
| Wulcain | **Dhin Thyraxion** | `Capital V` | 🟡 |
| Tabaxi | **Naka't-ol** | `Capital VI` | 🟡 |
| Ostad | — | `Capital I` | 🟡 falta en ambos |
| Ayashii | Himetsumota | — | ⬜ falta en cartas |
| Aeon | Conclave Elemental | — | ⬜ |
| Nocturnsea | Ciudad de Grytoz | — | ⬜ |
| Ashye | Zathor'aetz | — | ⬜ |

**47 de las 95 ciudades son marcadores de relleno** (`Capital II`, `Pueblo VI`, `Aldea IX`) con coordenadas correctas y sin nombre. En siete casos el canon ya tiene el nombre escrito. Cuatro capitales canónicas no están en el mapa. Solo hay **un conflicto real** (Gliaddokx) y dos variantes ortográficas.

Esto es el "nutrirse entre ellos" en su forma más literal y más barata: **las cartas ponen la geometría, el lore pone los nombres**, y ninguna de las dos partes tiene que renunciar a nada. Es media jornada de trabajo.

### Fractura 1b — El micro-setting sin bautizar

Las cartas tienen además campos estructurados `faction` (869 apariciones) y `location` (436). Ahí sí: contrastados contra el corpus, **las 16 facciones y las 29 localizaciones dan 0 coincidencias**.

| Las 16 facciones | Las 29 localizaciones (muestra) |
|---|---|
| espectros_de_la_tomba · corte_de_espinas · forjados_sin_amo · yokai_del_umbral · milicia_corrupta_de_llerba · cofradia_de_mascaras_de_plata · bandidos_del_camino_de_ceniza · plaga_de_san_lazaro · sagas_de_la_luna_hueca · nagas_del_pozo_azul · hechiceros_del_vacio · renacidos_de_la_fosa · manada_del_rei_llop · horrores_del_cielo_fragmentado · clan_de_gongorguma_renegado (×2 grafías) | los pasillos exteriores de la Tomba · la Puerta Norte de Llerba · los barrios de plata de Bastrea · el Camino de Ceniza · el cráter del Colegio Hundido · los canales del Pozo Azul · las ruinas del Gran Taller · el páramo bajo el Cielo Roto · la linde del Bosque que Reclama · el muelle viejo de Mijorn … |

De los 20 topónimos que sostienen ese contenido solo dos existen en Egaroth: `Bastrea` y `Gongorguma`. **Llerba**, la ciudad de la que cuelgan cinco facciones y localizaciones, no existe.

Pero el diagnóstico correcto **no** es "mundo paralelo". Las 16 facciones están clavadas como marcadores con coordenadas *sobre el mapa canónico* (`bandidos_del_camino_de_ceniza` en `[308, 920]`), y la descripción de Nimandum —capital de la canónica Bastrea— dice: *"la ciudad-puerto donde empieza La Tomba del Rei Llop: faroles rojos, deudas y máscaras de plata"*.

Es decir: es **un arco de aventura local, situado dentro del mundo real, que nunca se escribió de vuelta en el canon**. No hay que reconciliar dos cosmologías. Hay que decidir si ese arco entra en Egaroth y, si entra, escribirlo en el corpus. Es una decisión, no una migración.

### Fractura 1c — Las deidades, que sí son un error directo

| Carta | `id` | `name` | Canon Onegai |
|---|---|---|---|
| ✅ | `time` | Chronos | Chronos |
| ✅ | `knowledge` | Sofia | Sofía |
| ✅ | `self` | Egos | Egos el Justo |
| ✅ | `life` / `death` | Vida / Muerte | Vida / Muerte |
| ✅ | `eros` | Eros | Eros |
| ⚠️ | `war` | Guerra | Guerra / Destrucción (una entidad con dos nombres en el canon) |
| 🔴 | `morpheo` | **Deseo** | Morfeo y Deseo son **dos Creadores distintos** en el canon |
| 🔴 | `deitrok` | **Sastre** | El canon lo llama Deithrok/Deitrok; "Sastre" no aparece |
| 🔴 | `envy` | Envidia | El canon la identifica como **Mirathis**, cuyo asesinato es un evento central |

Y faltan por completo: **Egaroth** (el Padre Elemental, que da nombre al mundo), **Gurkazaal el Corrupto**, **Sho-Noco**, **Desgarro**, **Displicencia**, **Aegor**, y las tres lunas.

Un jugador que cree un personaje devoto en el motor de cartas no puede elegir al dios que creó el mundo.

**Duplicación adicional:** cada facción está almacenada dos veces con dos grafías (`espectros_de_la_tomba` en 53 sitios y `Los Espectros de la Tomba` en 25). Dos representaciones del mismo concepto — exactamente la regla que el `ARCHITECTURE.md` del motor gráfico prohíbe, cometida en los datos en vez de en el código.

### Fractura 2 — Las clases existen, pero casi ninguna puede lanzar un hechizo

El caso del **Apóstata** —clase fiel al documento original— lo enseña entero:

```json
"description": "Un exguerrero sagrado que ha renunciado a la luz para abrazar
                el poder oscuro, sacrificando su vitalidad para canalizar
                hechizos devastadores.",
"startingCards": { "passives": ["precio_de_la_apostasia"],
                   "skills":   ["marca_de_la_oscuridad", "pacto_de_sombra",
                                "susurro_corruptor"],
                   "spells":   [] }
```

Una clase cuya definición es *canalizar hechizos devastadores*, con **cero hechizos**.

No es un caso aislado. Medido sobre las 61 clases:

| | |
|---|---:|
| Clases con al menos un hechizo inicial | **2 de 61** |
| Hechizos iniciales en todo el sistema | **5** |
| Habilidades iniciales en todo el sistema | 182 |
| Hechizos con `classTags` (alguien puede aprenderlos) | **8 de 210** |
| **Hechizos huérfanos** | **202** |

Hay 210 cartas de hechizo escritas, y **202 no pertenecen a ninguna clase**. Existen en disco y son inalcanzables en juego. El trabajo está hecho; falta el enlace.

Y la pirámide de progresión está invertida respecto a lo que el sistema de tiers promete:

| Tier | 1 | 2 | 3 | 4 | 5 |
|---|---:|---:|---:|---:|---:|
| Habilidades + hechizos | 353 | 183 | 78 | 46 | **33** |

Seis tiers de progresión con 33 cartas en el último. El endgame está vacío: un personaje de tier 5 tiene menos opciones que uno de tier 1.

Esto no es deuda de motor, es **deuda de contenido**, y conviene decirlo claro porque cambia la prioridad: programar `magnitude_by_degree` en C++ no sirve de nada mientras 202 hechizos no tengan dueño. El propio `auditar_datos_onegai.py` del motor gráfico ya avisaba de esto en su cabecera; tenía razón.

### Fractura 3 — El puente cartas → motor pierde la mitad del contenido

El puente **existe** (`tools/convertir_definiciones_rpg.py` → `assets/catalogs/*.json`, 2.416 entradas en 20 catálogos) y eso ya es más de lo que suele haber. Pero es silenciosamente lesivo:

| Qué | En origen | Llega al motor | Pérdida |
|---|---:|---:|---|
| PNJs | 134 | 12 | El exportador lee `data/personatges/` (12) en vez de `data/npcs/` (134). **Un error de una línea que cuesta 122 PNJs.** |
| Historias | 302 | **0** | No existe catálogo `stories`. Toda la narrativa de las aventuras se queda fuera. |
| Trasfondos | 12 (con lore, personalidades, virtudes, deidades) | 50 desde `kits/` | El manifiesto lo marca `ok_fallback`. Los 12 trasfondos reales —los meses de nacimiento, con su astrología y sus deidades regentes, la pieza que mejor conecta con el canon— **no llegan**. |
| **Mapa del mundo** | 19 naciones, 95 ciudades, 26 zonas | **nada** | **No existe catálogo de geografía.** El activo más canónico del motor de cartas no sale de él (fractura 1a). |
| Aventuras | 54 | 78 entradas, **0 ejecutables** | Ver abajo. |
| Avisos de degradación | — | **1.795** | `rareza ausente, asignada 'common' por tier 0`, `estructura loot no estandar, adaptada`… |

El caso de las aventuras es el más ilustrativo. El `NarrativeEngine` de C++ lee un formato de **beats** (trigger + condición sobre flags + efectos), documentado en `FORMATO_AVENTURAS.md` y ejemplificado en `aventura_silbido_medianoche.json` (8 beats, 3 objetivos), escrito a mano. Las 78 aventuras exportadas del motor de cartas tienen `historiaIds`, `npcIds`, `enemigoIds`, `lootIds`, `trampaIds` y `notas` — y **ninguna tiene beats**. Son datos inertes: el motor las carga y no puede hacer nada con ellas.

Se exportan además con los campos duplicados en dos idiomas (`nom`/`name`, `descripcion`/`description`) porque no hay una capa de normalización.

**Y el toolchain no es reproducible:** seis de los nueve scripts de `MotorGraphico/tools/` tienen rutas absolutas hardcodeadas, y cinco de ellas apuntan a un sandbox muerto de una sesión anterior (`/sessions/quirky-nifty-heisenberg/…`). Ninguno de los validadores se ejecuta en CI.

### Fractura 4 — El reglamento está implementado dos veces

Existen dos implementaciones independientes de la misma regla 7.1 del GDD:

| | Java (`cat.dnd.cc.combat.motor.TiradaD6`) | C++ (`RPG::DicePoolEngine`) |
|---|---|---|
| Pool de Nd6, 6=1 / 5=0,5 | ✅ | ✅ |
| Ventaja (4=0,5) / Desventaja (solo 6) | ✅ | ✅ |
| Crítico / pifia | ✅ | ✅ |
| RNG inyectado para determinismo | ✅ | ✅ |
| **Resultado de una prueba** | `boolean superarCD()` | `enum Degree { BOTCH, PARTIAL, SUCCESS, CRITICAL }` |
| **Magnitud según grado** | no existe | `magnitude_by_degree` |

Ambas citan el GDD como fuente. Ambas son código cuidado. Y **ya han divergido**: el motor gráfico resuelve en cuatro grados y modula la magnitud del efecto según el grado; el motor de cartas resuelve en pasa/no pasa.

El remate: `magnitudeByDegree` / `degreeBonus` aparece en **0 de los 2.816 ficheros de datos originales** y en **2 catálogos exportados**. Es decir, el motor gráfico implementó una regla que ningún contenido usa, mientras el motor de cartas implementó otra distinta. Los tres niveles —especificación, implementación y contenido— dicen tres cosas.

Esto no se arregla eligiendo una implementación. Se arregla **haciendo que la especificación sea ejecutable**.

### Fractura 5 — No hay flujo, hay tres orígenes

```
HOY                                  DEBERÍA SER

  Onegai (lore)                        Onegai (corpus, prosa)
       │                                      │  extracción
       ✗  sin conexión                        ▼
       │                               ┌─────────────┐
  dndWeebCC ──── export ───┐           │ CANON       │ ← única verdad del mundo
   (mundo propio)  lossy   │           │ + REGLAS    │ ← única verdad mecánica
       │                   ▼           └──────┬──────┘
   reglamento v1     MotorGraphico             │ valida / alimenta
                      reglamento v2      ┌─────┴─────┐
                                         ▼           ▼
                                    dndWeebCC   MotorGraphico
                                    (autoría)   (ejecución)
```

Ninguno de los tres proyectos menciona nunca al otro en su código o sus datos: `grep -ri "Egaroth\|Cronologia_Maestra"` sobre todo `dndWeebCC` devuelve **cero resultados**. Los enlaces existentes son referencias en Markdown a rutas absolutas del disco de Oriol, que se rompen en cuanto algo se mueve.

---

# PARTE II — EL ENTORNO

## 3. Principio de diseño

> **Un concepto, una representación, un propietario, una dirección.**

Es literalmente la regla de oro que el propio `ARCHITECTURE.md` del motor gráfico se dio a sí mismo tras detectar sus 7 fracturas. Funcionó dentro de un proyecto; el entorno la aplica **entre** los tres.

De ahí salen cuatro decisiones:

**1. La verdad del mundo vive en un sitio y fluye en una sola dirección.**
`canon → cartas → motor`. Sin aristas de vuelta. Si el motor gráfico necesita un dato que no existe aguas arriba, es un bug de contenido, no de motor. Esto convierte discusiones de "¿quién tiene razón?" en una consulta.

**2. La verdad mecánica se expresa como datos + pruebas, no como código.**
Las fórmulas (techos por tier, vida, defensas, tabla del pool) viven en JSON. Y sobre ellas hay un **banco de conformidad**: casos entrada→salida esperada que *ambos* motores deben pasar. No hay que fusionar Java y C++, ni reescribir nada: hay que **atarlos al mismo contrato**. Es la pieza de mayor apalancamiento de todo el plan y probablemente la más barata.

**3. Los IDs canónicos son la clave de unión.**
Cada carta que menciona un lugar, facción, deidad o personaje lleva una referencia estructurada (`place_bastrea`, `deity_chronos`). La prosa sigue siendo libre; el enlace es un dato. Un validador comprueba que todo ID referenciado existe.

**4. Onegai es el referente de lore. Siempre, sin excepción.**
Ante cualquier discrepancia narrativa gana el corpus documental, aunque el motor de cartas tenga 400 cartas colgando de la versión equivocada. Esa regla no se negocia caso por caso: se escribe una vez y se aplica.

Pero "referente de lore" tiene un límite preciso, y conviene fijarlo porque es lo que hace viable el resto: **el motor de cartas no aporta lore, aporta reglas y geometría.** El mapa de `geografia.json` no es una versión rival del mundo — es el mismo mundo con coordenadas, que es justo lo que el corpus no tiene. Por eso el flujo no es "el lore sustituye al mapa", sino:

| Capa | Propietario | Cómo se resuelve un conflicto |
|---|---|---|
| Nombres, historia, panteón, quién gobierna qué | **Onegai** | Gana el corpus. Punto. |
| Coordenadas, polígonos, adyacencias, distancias | **dndWeebCC** | Gana el mapa: es la única fuente que tiene el dato. |
| Reglas, fórmulas, tiers, balance | **dndWeebCC** | Gana el GDD. |

Con ese reparto, los 7 nombres de capital que faltan en el mapa se rellenan desde el canon y las coordenadas se conservan. Nadie pierde trabajo.

Para el micro-setting de la fractura 1b (Llerba, la Tomba, las 16 facciones) el reparto anterior no decide, porque no hay dos versiones: hay una sola, escrita fuera del canon. Ahí solo caben dos salidas, y es **una decisión narrativa tuya, no técnica**: adoptarlo —escribirlo en el corpus, ubicarlo en una nación y una era— o marcarlo como material no canónico y reasignar las 432 cartas de enemigo a facciones reales. Adoptarlo es más barato y no tira nada; reasignar es más limpio. El entorno funciona igual con cualquiera de las dos, pero necesita que la elección esté hecha antes de la fase 3.

## 4. Estructura de `Onegai-Core`

Carpeta hermana de los tres. No sustituye a ninguno: los tres siguen siendo repositorios independientes que consumen de aquí.

```
Software/
├── Onegai-Core/                    ← NUEVO
│   ├── canon/                      El mundo como datos  ← lore: Onegai manda
│   │   ├── deities/                17 Creadores  (hoy: 10 cartas, 3 mal)
│   │   ├── races/                  razas canónicas + subrazas
│   │   ├── nations/                19 naciones, sus capitales y dinastías
│   │   ├── places/                 69 del índice + los del mapa
│   │   ├── factions/               16 del motor de cartas, una vez decididas
│   │   ├── characters/             322 del censo, con su estado
│   │   ├── eras/  events/          17 eras · 285 eventos EV-…
│   │   └── calendars/              los 7 calendarios paralelos
│   │
│   ├── atlas/                      ★ El mundo como geometría ← dndWeebCC manda
│   │   ├── nations.geo.json        19 polígonos, vértices editables
│   │   ├── cities.geo.json         95 puntos x/y  (43 aún sin nombre canónico)
│   │   ├── zones.geo.json          26 zonas, 5 santuarios
│   │   └── projection.json         la conversión entre los 3 sistemas de coords
│   │
│   ├── spec/                       El reglamento como contrato
│   │   ├── rules/                  tiers, vida, defensas, pool d6, pilas
│   │   └── conformance/            ★ casos entrada→salida que ambos motores pasan
│   │
│   ├── schema/                     JSON Schema de cada tipo de carta y entidad
│   │
│   ├── bridge/
│   │   ├── extract/                corpus (.docx/.md) → canon/
│   │   ├── validate/               refs rotas, colisiones, huérfanos
│   │   └── export/                 cartas → catálogos del motor, sin pérdida
│   │
│   ├── reports/                    salida generada de las auditorías
│   └── ONEGAI_CORE.md              cómo funciona esto
│
├── Onegai 2/            (autoría narrativa — emite canon)
├── dndWeebCC-master/    (autoría mecánica — consume canon, emite cartas)
└── MotorGraphico-main/  (ejecución — consume ambos, no emite nada canónico)
```

### La pieza clave: `spec/conformance/`

Un caso es un fichero plano, legible por cualquier lenguaje:

```json
{
  "id": "pool_des5_ventaja_seed42",
  "regla": "GDD 7.1 — pool de d6 con ventaja",
  "entrada": { "dados": 5, "modo": "ventaja", "seed": 42 },
  "esperado": { "tiradas": [6,4,2,5,6], "exitos": 3.0,
                "grado": "CRITICAL", "critico": false, "pifia": false }
}
```

`TiradaD6Conformance.java` y `test_conformance.cpp` recorren el mismo directorio. **Si divergen, falla el build de ambos.** Esto es lo que convierte "dos implementaciones" de un pasivo en un activo: dos verificaciones independientes de la misma regla.

Requiere una decisión previa que hoy está abierta y hay que cerrar explícitamente: **el generador de números aleatorios tiene que ser el mismo algoritmo en ambos lados** (un PCG32 o xoshiro de 30 líneas, no `java.util.Random` ni `std::mt19937`), o los casos no pueden fijar tiradas concretas. Es media jornada de trabajo y desbloquea todo lo demás.

## 5. Qué gana cada proyecto

| | Recibe del entorno | Aporta al entorno |
|---|---|---|
| **Onegai (lore)** | **Un mapa de verdad**: por primera vez el corpus tiene coordenadas, adyacencias y distancias — hoy `Mapa_Egaroth.svg` son 74 etiquetas flotando sin geometría. Y sabe qué está *usado*: los 173 personajes "solo nombre" pasan de ser una lista a deuda priorizada por cuántas cartas los citan. | El lore. Es el referente y no se discute. |
| **dndWeebCC (cartas)** | Los 7 nombres de capital que le faltan, servidos desde el canon. Autocompletado y validación contra entidades reales al crear una carta: deja de tener que inventar mundo para poder seguir trabajando. | Las reglas, 2.816 cartas y **la geometría del mundo**. |
| **MotorGraphico** | Un mundo que renderizar: 19 naciones, 95 ciudades y 26 zonas donde hoy hay tres niveles genéricos. Más 134 PNJs en vez de 12, 302 historias en vez de 0, y la garantía de que su combate da el mismo resultado que el de mesa. | Nada canónico — y eso es exactamente lo correcto. Es un renderizador. |

El beneficio cruzado que hoy no existe se ve mejor con un ejemplo concreto. La ciudad de **Numandum** aparece hoy tres veces sin saberlo: como etiqueta en el SVG del lore, como `Nimandum` con coordenadas `(563, 860)` en el mapa de cartas, y como escenario de la aventura de la Tomba del Rei Llop. Con el entorno montado es **una** entidad: el lore le da el nombre y la historia, el mapa le da el sitio, y el motor gráfico puede generar el nivel y colocar en él los PNJs y enemigos que ya están escritos — sin que nadie toque una línea de código. Eso es el "nutrirse entre ellos".

---

# PARTE III — HOJA DE RUTA

Ordenada por **desbloqueo**, no por tamaño. Cada fase deja algo verificable.

### Fase 0 — Cimientos y línea base
- Crear `Onegai-Core/` con la estructura de arriba, git propio.
- Portar los validadores existentes (`auditar_datos_onegai.py`, `validar_catalogos_rpg.py`) y **arreglar las rutas muertas** de `MotorGraphico/tools/`.
- Ejecutar la auditoría completa y guardar el resultado en `reports/` como línea base. Todo lo que venga después se mide contra esto.

### Fase 1 — El atlas (la fase que antes no estaba, y va primero)
> **Esta fase está en marcha.** Ver `Onegai-Core/ATLAS_Mapamundi_y_Volcado.md`: la rejilla canónica ya está extraída, los 8 mapamundis generados y verificados, y el volcado al vault funcionando. Lo que sigue en esta lista es lo que queda.

Es la más barata y la que más desbloquea, porque el activo ya existe y solo hay que sacarlo de su cárcel.

- Mover `geografia.json` a `atlas/` como fuente compartida, partido en naciones / ciudades / zonas.
- **Rellenar las 7 capitales con nombre de relleno** desde el canon: Venordemn, Tor'k Hazar, Klimnebra, Saif-l'sa, Dhin Thyraxion, Naka't-ol. Media jornada.
- Añadir las 4 capitales que faltan (Himetsumota, Conclave Elemental, Ciudad de Grytoz, Zathor'aetz).
- Unificar las dos variantes ortográficas (Numandum, Guskedor) y **resolver el conflicto de Gliaddokx** — Havar'gruztak o Grahgan, hay que elegir.
- Fijar la proyección entre los tres sistemas de coordenadas que existen hoy: el SVG del mapa de cartas, el `viewBox` de `Mapa_Egaroth.svg` (1472×1172) y la rejilla 26×25 de `Ciudades y sus elementos.xlsx`.
- **Exportar el atlas al motor gráfico.** Es el catálogo que no existe.
- Entregable: los tres proyectos comparten un mapa. Es la primera vez.

### Fase 2 — Extraer el canon
- Convertir `indice_personajes.md`, `indice_lugares.md` y `Onegai_Cronologia_Maestra.md` a `canon/*.json` con IDs estables. **El contenido ya existe**; esto es transformación de formato, no investigación.
- **Deidades**: 10 cartas, 3 errores directos, 6 ausencias. El arreglo de mayor relación impacto/esfuerzo del plan después del atlas.
- Congelar el esquema de entidad canónica y su JSON Schema.
- Resolver las colisiones que el índice ya marca en rojo (Deithrok vs Forja, el desfase de 100 años entre 2738 y 2838).

### Fase 3 — Enlazar cartas y canon
- Añadir `canonRefs` a las cartas y activar el validador de referencias.
- Unificar la doble grafía de facciones.
- **Ejecutar la decisión sobre el micro-setting** (adoptar o reasignar, ver principio 4). Es la única parte del plan que es trabajo creativo puro; aquí es donde los agentes de escritura local de `Onegai 2` aportan de verdad.

### Fase 4 — Cerrar la deuda de contenido
Antes de tocar una línea más de motor:

- **Asignar los 202 hechizos huérfanos** a las clases que los usarían. Es el mayor volumen de trabajo ya escrito y desperdiciado del proyecto.
- Dar hechizos iniciales a las 59 clases que no tienen (empezando por las que su descripción los promete, como el Apóstata).
- Llenar los tiers 4 y 5, hoy con 46 y 33 cartas.
- Meta verificable: ninguna carta huérfana, ninguna clase con `spells: []` que diga lanzar hechizos.

### Fase 5 — Puente sin pérdida
- Corregir la fuente de PNJs (`personatges` → `npcs`): **+122 PNJs por una línea**.
- Exportar las 302 historias.
- Usar los 12 trasfondos reales en vez del *fallback* de kits.
- Atacar los 1.795 avisos por categoría: la mayoría son dos patrones repetidos.
- **Decidir el formato de aventura.** Es una decisión de diseño, no de código: o el motor de cartas genera beats, o el `NarrativeEngine` aprende a leer cadenas de `historiaIds`. Sin esto, 54 aventuras no son jugables.
- Meta verificable: exportación con **0 avisos** y un validador que falle el build si reaparecen.

### Fase 6 — Conformidad del reglamento
- Elegir y portar el mismo PRNG a Java y C++.
- Escribir el banco de casos a partir de las secciones 5, 6 y 7 del GDD.
- Conectarlo a la CI de ambos proyectos.
- Cerrar la divergencia de grados: decidir si el sistema tiene 2 o 4 grados de éxito, escribirlo en `spec/rules/`, y alinear los dos motores **y el contenido**.

### Fase 7 — Que no se vuelva a romper
- CI en los tres repos que valide contra `Onegai-Core` en cada push.
- Reglas de mantenimiento en los `AGENTS.md` / `CLAUDE.md` de cada proyecto: *ninguna entidad de mundo se crea fuera del canon*.
- Informe periódico de deriva.

---

## 6. Riesgos

**El de verdad: la fase 3 es trabajo creativo, no técnico.** Decidir qué pasa con Llerba y la Tomba no lo hace un script. Es lo único que puede encallar el plan, y por eso las fases 1, 4, 5 y 6 están diseñadas para **no depender de ella** — se pueden hacer en paralelo mientras esa decisión madura.

**La revisión 1 de este informe es en sí misma un riesgo documentado.** Auditó `faction` y `location`, concluyó "mundo paralelo", y se le pasó el fichero donde vive el mapa. La lección para el entorno: **una auditoría que solo mira los campos que espera encontrar confirma su propia hipótesis.** Los validadores de `Onegai-Core` deben recorrer *todos* los ficheros de datos y reportar los que no encajan en ningún esquema conocido, en vez de mirar solo las claves de la lista.

**Sobre-ingeniería del canon.** La tentación será modelar el mundo entero antes de tocar nada. El antídoto: empezar por atlas y deidades, que son los que las cartas ya referencian. Lo que nadie referencia no necesita esquema todavía.

**El motor gráfico va por delante de su contenido.** Implementó `magnitude_by_degree` sin que exista un solo dato que lo use, mientras 202 hechizos escritos no tienen dueño. Es el patrón que causó las fracturas 2 y 4, y conviene invertirlo: **primero el contenido decide, luego el motor lo ejecuta.**

**Fatiga de validadores.** 1.795 avisos que nadie lee son ruido. La regla es dura pero necesaria: o el aviso rompe el build, o se borra el aviso.

---

## 7. Lo que yo haría el lunes

Cuatro cosas pequeñas, todas verificables el mismo día, elegidas porque cada una desbloquea algo desproporcionado:

1. **Arreglar las rutas muertas** de `MotorGraphico/tools/`. Ahora mismo el toolchain de exportación no corre en ninguna máquina. Sin esto no se puede medir nada.
2. **Rellenar las 7 capitales sin nombre** del mapa desde el canon. Es copiar siete palabras y es el primer intercambio real de valor entre el lore y las cartas.
3. **Cambiar `personatges` por `npcs`** en el exportador. 122 PNJs entran en el juego.
4. **Reescribir las 10 deidades** contra el canon y añadir a Egaroth. Es el momento en que los tres proyectos comparten su primera entidad de verdad.

---

### Anexo — Cómo se obtuvo cada cifra

Todas las cifras de este informe se midieron sobre el disco el 11/08/2026; ninguna se tomó de la documentación de los proyectos, que en varios puntos está desactualizada. El DAFO de `dndWeebCC` (19/07/2026) cita 2.017 cartas, 38 razas, 52 aventuras, 115 clases Java y 76 plantillas; el disco tiene hoy 2.816 ficheros JSON, 43 razas, 54 aventuras, 146 clases y 83 plantillas.

**Esa deriva de tres semanas es, en pequeño, el mismo problema que describe todo el informe**: cada proyecto documenta su propio estado y nadie recalcula. En `Onegai-Core`, `reports/` se genera; no se escribe a mano.

| Cifra | Método |
|---|---|
| Naciones, ciudades y zonas del mapa | Lectura de `data/mapa/geografia.json`; capitales contrastadas contra las 74 etiquetas de `Mapa_Egaroth.svg` |
| Facciones y localizaciones de las cartas | Recorrido recursivo de los 2.816 JSON extrayendo `faction` / `factionName` / `location` |
| Contraste con el canon | Búsqueda de cada valor en `indice_lugares.md` + `indice_personajes.md` + `Onegai_Cronologia_Maestra.md` + `TL_OCR_completo.md` |
| Hechizos huérfanos y clases sin hechizos | Conteo de `classTags` en los 210 hechizos y de `startingCards.spells` en las 61 clases |
| Pérdidas del puente | `assets/catalogs/MANIFIESTO.json` (`fuente`, `status`, `warnings_count`) contra el conteo real de `data/` |
| Aventuras ejecutables | Conteo de entradas con clave `beats` en `assets/catalogs/adventures.json` |
| Doble reglamento | Lectura de `TiradaD6.java` y `DicePoolEngine.h`; búsqueda de `magnitudeByDegree` en datos y catálogos |
| Líneas de código | `wc -l` sobre `src/main/java` y sobre `src/` + `include/` |
