# Modelo de prompt para generar contenido del catálogo

*Plantillas de instrucción para crear contenido nuevo (a mano, con IA o por script) que respete
el GDD edición 2, el esquema de la app Java y el contenido ya existente. Copiar el bloque del
tipo que toque, rellenar los huecos `{...}` y adjuntar siempre el PROMPT MAESTRO.*

---

## PROMPT MAESTRO (adjuntar siempre)

```
Actúa como diseñador de contenido del sistema "Cartas y Tiers" (proyecto dndWeebCC / Onegai).
Reglas innegociables:

1. RESPETA LO EXISTENTE. Antes de crear nada, consulta los ids ya presentes en data/cartas/**.
   Nunca sobrescribas ni dupliques un id. Si un concepto ya existe, crea una variante con id
   nuevo o no lo crees.
2. TODA REFERENCIA ES UN ID REAL. startingEquipment, learnableSkills, linkedSkill, summonedBy,
   unlocks, loot... apuntan a ids de cartas que existen (o que creas en el mismo lote). Prohibido
   texto libre tipo "Arma: Espadas largas".
3. IDIOMA: español, ids en snake_case sin acentos. flavorText de 1 frase con voz propia.
4. RECURSOS: no existe maná/energía/puntos. Toda habilidad/hechizo declara
   recovery: activa | descanso_corto | descanso_largo | ninguno (GDD sección 6).
   Impacto de combate relevante → nunca "activa" ni "ninguno" (GDD 10.7).
5. STATS: solo CON, DES, INT, CAR. Rango de personaje en tier 0-1: 1-6.
6. TIER ANTES QUE EFECTO (GDD 12.1): fija el tier y acota el efecto a él. Rareza acorde:
   t1 common/uncommon · t2 uncommon/rare · t3 rare · t4 epic · t5 epic/mythic.
7. TODA CARTA FUERTE LLEVA UNA DEBILIDAD explícita (GDD 12.5): incompatibleTags, requiredStats,
   duration: concentration, range: melee o una limitación textual.
8. SALIDA: JSON válido, un archivo por carta, con exactamente los campos del esquema del tipo
   (ni más ni menos). Sin comentarios dentro del JSON.
9. Lote máximo recomendado: 20 cartas por interacción, con tabla-resumen final para revisión.
```

---

## 1. Equipo / armas / loot (`data/cartas/armas/`, tipo `equipment`)

```
Genera {N} cartas de equipo de tier {T} y rareza {R} para el slot {cabeza|torso|piernas|pies|
arma_principal|arma_secundaria|tesoro}.

Esquema: { id, name, type:"equipment", slot, tier, rarity, statBonuses:{CON,DES,CAR,INT,health},
penalties:{CON,DES,CAR,INT}, grantedTags[], linkedSkill|null, requiredStats:{...},
restrictions[], flavorText, weightCategory:"ligera|media|pesada" }

Presupuesto de bonos (suma de statBonuses − suma de penalties):
  common ≤ 1 · uncommon ≤ 2 · rare ≤ 3 · epic ≤ 4 (y +1 extra de tope por cada tier > 1).
Armadura pesada: siempre alguna penalización (DES) o restricción. Un objeto de rareza épica+
recuerda la regla de "solo un épico/legendario equipado" (GDD sección 3).
1 de cada 4 piezas lleva linkedSkill (habilidad propia, id real) en vez de más números.
Reparte entre los 6 slots — piernas y pies también existen.
```

## 2. Clases (`data/cartas/clases/`, tipo `class`) — estándar "Ancla del Vacío"

```
Genera {N} clases nuevas. Una clase NO es una lista de bonos: es una mecánica central con nombre
(¿qué decisión distinta obliga a tomar cada turno? — GDD 10) expresada en su pasiva.

Esquema app: { id, name, type:"class", role:"tank|balanced|agile|caster|support", tier:1,
baseHealth, healthScaling:{CON}, primaryStat, secondaryStat,
primaryResource:"ninguno (sistema de pilas...)", secondaryResource:null,
startingEquipment[ids reales], startingCards:{ passives:[1 id], skills:[3 ids],
spells:[], learnableSkills:[5-8 ids] }, allowedEquipmentTags[], restrictedTags[],
specializations[2-3 nombres], description, maxArmorWeight, maxWeaponWeight }

Vida/escalado por rol: tank 16/×4 · balanced 12/×3 · agile 10/×2 · support 10/×2 · caster 8/×1.5.
OBLIGATORIO por clase: 1 carta pasiva (esquema §5 del GDD 9.4, unique:true) + 3 cartas de
habilidad iniciales REALES (creadas en el mismo lote) + learnableSkills apuntando a ids reales.
Reparte primaryStat: máximo 1/3 del lote con CAR principal.
Si la mecánica central necesita un estado, crea también su carta de Condición (§8).
```

## 3. Habilidades y hechizos (`habilidades/` tipo `skill`, `hechizos/` tipo `spell`)

```
Genera {N} habilidades para la clase {id_clase} (o universales) de tier {T}.

Skill: { id, name, type:"skill", tier, rarity, classTags[], roleTags[], mechanicTags[],
requiredStats{}, requiredTags[], incompatibleTags[], recovery, actionType:"accion|accion_menor|
reaccion|movimiento|preparacion|canalizacion", range, duration, effect:{description,scaling},
limitations[], evolvesInto|null, flavorText }

Spell añade: school, castingStat:"INT|CAR" (INT=arcano, CAR=divino/afinidad), area|null,
upgradeConditions|null. Daño (regla 7.6): la tirada ya lo genera — la carta solo declara BONOS
FIJOS orientativos: t1 +0..+2 · t2 +2..+3 · t3 +3..+5 · t5 +6+ (o "suma tu {stat}").
recovery según impacto: básico/utilidad → activa · fuerte por combate → descanso_corto ·
definitorio del día → descanso_largo. Máx. 1 carta "activa" de daño por clase (su truco).
```

## 4. Subrazas / razas (`data/cartas/razas/`, tipo `race`)

```
Genera {N} subrazas variantes de los pueblos existentes (o nuevos).

Esquema: { id, name, type:"race", tier:1, statBonuses:{CON,DES,CAR,INT},
passiveTrait:{name,description}, activeTrait:{name,description} (NUNCA null),
affinities[], limitations[], narrativeTags[], unlocks[], flavorText }

LÍMITE DURO: suma de statBonuses ≤ 2 (GDD 9.2: el bono es menor, el rasgo es el motivo).
El rasgo racial tiene nombre propio y regla jugable (1/combate, 1/descanso...).
Metadatos de lore (edad, tamaño, cultura) van al flavorText en UNA frase, no un párrafo.
```

## 4b. Trasfondos — PROMPT MAESTRO CANÓNICO (decisión 2026-07, sustituye al esquema anterior)

```
Eres un diseñador narrativo especializado en TTRPGs y sistemas modulares basados en cartas.
Crea Trasfondos para Onegai RPG siguiendo estrictamente estas normas.

FILOSOFÍA: un Trasfondo NO es una clase ni una raza. Solo responde a
"¿quién era este personaje antes de convertirse en aventurero?".
Aporta personalidad, historia, contexto, motivaciones, conflictos y narrativa.
NUNCA aporta poder mecánico.

PROHIBIDO otorgar: estadísticas, vida, armadura, movimiento, habilidades, pasivas,
competencias, bonus, modificadores, hechizos, equipo o recursos. Eso vive en otras cartas.

OBJETIVO: ayudar a construir la personalidad SIN obligar a interpretar un personaje
concreto: cada lista ofrece 5-10 OPCIONES para elegir, de modo que dos personajes con
el mismo Trasfondo puedan ser completamente diferentes.

ESQUEMA JSON (exacto):
{ "id", "name", "type": "BACKGROUND", "tier": 1, "description",
  "lore": { "origin", "culture", "environment", "importantEvents": [],
            "deities": [ids reales del panteón], "symbols": [], "traditions": [] },
  "characterCreation": {
    "choosePersonality": 1, "personalities": [ {id,title,description} ×5-10 ],
    "chooseVirtue": 1,      "virtues":       [ {id,name,description} ×5-10 ],
    "chooseFlaw": 1,        "flaws":         [ {id,name,description} ×5-10 ],
    "chooseGoal": 1,        "goals":         [ {id,name,description} ×5-10 ],
    "chooseFear": 1,        "fears":         [ {id,name,description} ×5-10 ],
    "chooseIdeal": 1,       "ideals":        [ {id,name,description} ×5-10 ],
    "chooseBond": 1,        "bonds":         [ {id,name,description} ×5-10 ] },
  "alignment": { "title", "description" },   // arquetipo narrativo, NO bueno/malo
  "tags": [] }

- Personalidades: una forma de actuar ("Habla poco, dice mucho").
- Defectos: conflicto interno, jamás desventaja mecánica.
- Objetivos: misión personal del personaje, no del Director.
- Miedos: lo que evita. Ideales: sus valores. Vínculos: personas/lugares/objetos/juramentos.
- Lore: solo contexto (lugar, cultura, dioses, hechos). Nunca reglas.
- Descripciones breves, evocadoras y útiles para interpretar.
- Salida: un único JSON limpio listo para data/cartas/transfondos/.
```

## 5. Dotes (`data/cartas/dotes/`, tipo `feat`)

```
Esquema: { id, name, type:"feat", tier, rarity, classTags[], requiredStats{}, requiredTags[],
incompatibleTags[], grantedTags[], effect:{description,scaling}, limitations[], flavorText }
Una dote = 1 efecto pasivo pequeño y siempre activo. Nunca replica una pasiva de clase.
requiredStats crece con tier (t1: stat 2 · t2: stat 3 · t3+: stat 4).
```

## 6. Invocaciones (`data/cartas/invocaciones/`, tipo `summon`)

```
Esquema: { id, name, type:"summon", summonedBy:"id real de skill/spell", tier, health,
attacks:[{name,effect}], movement, passive:{name,description}, duration, control:
"automatica|jugador|car_invocador", flavorText }
Vida ≈ 6 + 3×tier. 1-2 ataques (daño ≤ tier d6). Límite simultáneo lo gobierna INT (GDD 4).
Si summonedBy no existe todavía, crea también esa carta de habilidad/hechizo en el lote.
```

## 7. Deidades y habilidades divinas (`deidades/` tipo `deity` + `hechizos/` con castingStat CAR)

```
Genera {N} deidades y {M} habilidades divinas repartidas entre ellas.
Deity: { id, name, type:"deity", domain, favor:{description, scaling:"CAR|none"},
compatibleWith[ids], obligations[], flavorText }
Habilidad divina = spell con castingStat:"CAR", mechanicTags incluyendo "Divino" y el dominio,
y campo extra "deity": id de la deidad que la otorga. El favor es menor (1/descanso largo);
el poder real está en sus hechizos. Toda deidad tiene 1-2 obligaciones/tabúes con mordida.
```

## 8. Condiciones (`data/cartas/condiciones/`, tipo `condition`)

```
Esquema: { id, name, type:"condition", category:"positiva|negativa|neutro", source, stackable,
duration, effects:[{description, mechanicHook:"gancho_maquina"}], cureConditions[], flavorText }
mechanicHook con vocabulario estable: velocidad_0, bono_ca:N, ventaja:{tipo}, desventaja:{tipo},
dano_por_turno:NdM, bono_alcance:N... (el motor de combate del Bloque D los consumirá).
```

## 9. Enemigos y villanos (`data/cartas/enemigos/`, tipo `enemy` — Bloque G, sin pantalla aún)

```
Esquema: { id, name, type:"enemy", rank:"criatura|elite|jefe", tier, health, armor,
stats:{CON,DES,CAR,INT}, attacks:[{name,effect}], skills[ids o inline], conditionsInflicted[ids
de condiciones], loot[ids de equipo/consumibles], phases:[solo jefes: {name,trigger,change}],
faction, flavorText }

Balance: criatura vida ≈ 8+4×tier · elite ≈ 16+6×tier · jefe ≈ 30+10×tier (+fases).
CD implícita: sus ataques atacan CA/Defensa mental/Resistencia física del GDD 5.
VILLANOS: rank jefe + campo "villainProfile": { objetivo, método, debilidad_narrativa,
vínculo (mes/raza/deidad existente) }. Recicla las facciones oscuras ya escritas
(revenants, espectros, sagas, traidores_oscuros, hechiceros_oscuros...) como facciones.
Todo enemigo suelta loot con ids reales.
```

## 10. Historias / ganchos (`data/historias/`, tipo `story`)

```
Esquema (el de los 300 archivos reales ya en disco): {
  id (prefijo hist_), title, type:"story",
  hook (2-3 frases: quién, dónde, qué quiere),
  location (nombre exacto de nación/ciudad/zona del mapa),
  antagonist (id EXACTO de un enemigo del catálogo, tier coherente con tierRange),
  tierRange:[min,max],
  reward (id de loot_table real del catálogo),
  decision (un dilema real sin opción claramente buena),
  complication (lo que sale mal aunque el grupo tenga razón),
  months[0-2 ids de trasfondo mes_* afines],
  faction (id de facción real),
  chain { trama (mismo id para todas las de la cadena), step (1..n), of (total de la cadena) },
  consequence (opcional: qué deja la historia resuelta en el mundo),
  flavorText
}

Reglas duras:
- Una historia conecta SIEMPRE al menos: 1 enemigo real + 1 recompensa real + 1 mes o facción.
- Nada de "un mal antiguo despierta": el gancho nombra a alguien, en un sitio, queriendo algo.
- "decision" es un dilema sin opción claramente buena (Proteger / Negociar / Romper, u otro).
- "chain" solo si la historia forma parte de una trama encadenada; si es autoconclusiva, chain = null.
- Si la historia tiene "consequence", que deje huella jugable (una facción enemistada, una ruina
  nueva, un PNJ que aparece o desaparece).
Destino: un archivo por historia en data/historias/<id>.json.
+ REGLAS OBLIGATORIAS + CATÁLOGO (enemigos, historias, loot, facciones, naciones, ciudades, zonas, trasfondos)
```

> **Fuente única de verdad para historias.** Esta es la plantilla canónica; `Guia_Prompts §A1`
> describe el mismo tipo con el mismo esquema — úsala indistintamente, no hay dos versiones.

## 11. Personajes (`data/personatges/`, esquema de la app)

```
Esquema: { id numérico nuevo, nom, razaId, claseId, transfonsId (mes), tier, statCon, statDes,
statCar, statInt, habilidadIds[], equipoIds[], doteIds[], historia }
Reparto de stats en tier 1: 7 puntos, rango 1-4, coherente con la clase (primaryStat el más alto).
habilidadIds ⊆ learnableSkills de su clase (respetar límite de mano del tier: t1=10).
equipoIds respetan slots (1 por slot) y maxArmorWeight/maxWeaponWeight de la clase.
historia: 3-5 frases que conectan mes (virtud/defecto/objetivo) + raza + clase + un gancho.
```

## 12. PNJs (`data/npcs/`, tipo `npc`)

```
Esquema (el de los 133 PNJs ya en disco): {
  id (prefijo npc_), name, type:"npc",
  role: "contacto|mercader|mentor|autoridad|informante|rival|victima|traidor",
  location (lugar exacto del mapa),
  faction (id de facción real),
  agenda (1 frase: qué quiere DE VERDAD y de quién — no su oficio),
  attitude { inicial: "aliado|neutral|hostil", palanca: "qué lo mueve de bando" },
  dialogue [3 frases con su voz propia, distinguible de cualquier otro PNJ del catálogo],
  services (array de servicios mecánicos con ids reales: tienda/curación/información/transporte,
            o null si no ofrece ninguno),
  secretHook (id de historia/enemigo/localización real al que conecta el secreto, o null),
  month (id de mes_* de nacimiento — le da virtud y defecto gratis),
  flavorText
}

Un PNJ no es decoración: agenda + palanca + secreto conectado a contenido real (id de misión,
enemigo o localización). Su mes de nacimiento le da virtud y defecto gratis — usarlo.
Destino: data/npcs/<id>.json.
+ REGLAS OBLIGATORIAS + CATÁLOGO (npcs, facciones, naciones, ciudades, zonas, historias)
```

> **Fuente única de verdad para PNJs.** Esta es la plantilla canónica; `Guia_Prompts §A2`
> describe el mismo tipo con el mismo esquema — úsala indistintamente, no hay dos versiones.

## 13. Monturas (`data/cartas/monturas/`, tipo `mount` — afinidad ♞)

```
Esquema: { id, name, type:"mount", tier, rarity, health, armor, movement (6-10, siempre > a pie),
stats:{CON,DES}, capacity:1|2, mountedTags:["♞"], ridingRequirements:{stat mínimo o tag},
mountedSkill:{name,description, recovery} | null (1 maniobra propia: carga, salto, pisotón...),
dismountTrigger:"qué la derriba/espanta", terrain:["llano","montaña","agua","aire"],
upkeep:"1 frase de mantenimiento narrativo", flavorText }

Regla: la montura es una carta activa que NO ocupa mano (como el equipo). Mientras estás montado
ganas su movement y su mountedSkill, y el equipo/cartas con afinidad ♞ se activan. Vida propia:
si cae, prueba de DES o quedas Caído. Máx. 1 montura activa. Vida ≈ 8 + 4×tier.
```

## 14. Trampas (`data/cartas/trampas/`, tipo `trap` — mazo de aventura, Bloque H)

```
Esquema: { id, name, type:"trap", tier, rarity, trigger:"qué la activa (pisar/abrir/luz/sonido)",
detection:{stat:"INT|DES", cd:0.5-2.5}, disarm:{stat, cd, herramienta|null},
effect:{description, defensa:"CA|defensa_mental|resistencia_fisica", condicionInfligida:id|null},
onFail:"qué pasa si el desarme falla (¡peor que no intentarlo!)",
reset:"única|recargable|persistente", flavorText }

CD y daño acotados por tier (t1 CD 0.5-1 · t3 CD 1.5 · t5 CD 2+). Toda trampa se puede detectar
Y desarmar (dos tiradas distintas). Las de tier 3+ infligen una condición real (id de carta de
condición), no solo daño. 1 de cada 5 es "narrativa": no daña, complica (alarma, sello, deuda).
```

## 15. Artefactos (`data/cartas/artefactos/`, tipo `equipment` con `artifact:true`)

```
Un artefacto ES equipo (esquema §1) con rareza epic|mythic y tres campos extra:
{ ..., "artifact": true, "uniqueRule": "1 regla que rompe el juego de forma acotada",
"awakening": { "condition": "hito narrativo que lo despierta", "unlocks": "id de habilidad" },
"burden": "el precio: obligación, tabú, atención de una facción o penalización" }

Reglas duras: máx. 1 épico/legendario equipado (GDD sección 3). Todo artefacto tiene burden con
mordida real (no "pesa mucho"). uniqueRule modifica UNA regla (una pila, una defensa, un tipo de
tirada), nunca dos. awakening conecta con una historia o jefe real (ids). Numerar A-001...A-100.
```

## 16. Tablas de loot (`data/loot/`, tipo `loot_table`)

```
Esquema: { id, name, type:"loot_table", tierRange:[min,max],
drops:[{ item:"id real de equipo/consumible/artefacto", chance:1-100, condition|null }],
gold:{min,max}, clue:"id de historia/misión que puede caer"|null, flavorText }

Suma de sentido, no de %: común 60-100 · uncommon 25-60 · rare 10-25 · epic 5-10 · mythic solo
por condición ("derrotar sin usar descansos", "fase 3 alcanzada"). Toda tabla puede soltar una
PISTA además de objetos — el loot también cuenta historia. Cada enemigo/trampa/estancia referencia
una loot_table por id, nunca lista objetos inline.
```

> **Fuente única de verdad para loot.** `chance` SIEMPRE en escala **1-100** (no 0-1). `drops[]`
> SIEMPRE con clave **`item`** (no `itemId`). Los 200 archivos de `data/loot/` ya siguen este
> esquema. `Guia_Prompts §A3` apunta aquí como referencia.

## 17. Jefes y mini-jefes (extensión del §9 con fases)

```
Un jefe = enemigo (§9) con rank:"jefe" (mini-jefe: "elite") y además:
"phases": [2-3 fases: { name, healthThreshold:%, enterEffect:"qué cambia al entrar (id de
  habilidad que gana / condición que aplica / arena que altera)", aiModifier:"defensivo|estandar|
  agresivo" }],
"legendaryActions": 0-2 reacciones extra por ronda (solo jefes t3+),
"arena": { name, hazard:"1 peligro de zona con id de trampa o condición" },
"villainProfile": { objetivo, metodo, debilidad_narrativa, vinculo:"mes/raza/deidad/facción real" },
"lootTable": id (obligatoria, con drop condicionado por fase alcanzada),
"storyCard": { title, text, unlock:"al encontrarlo o derrotarlo" }

Vida jefe ≈ 30+10×tier (mini-jefe 16+6×tier). Cada fase debe CAMBIAR la decisión del grupo, no
solo subir números. La debilidad narrativa es explotable mecánicamente (ventaja, saltar una fase).
```

## 18. Aventuras por actos y Cartas de Historia (GDD sección 20)

Cuatro prompts encadenados que producen una aventura completa del sistema de mazos por actos
(GDD §20): fichas 🟢/🔴, filtrado entre actos, cartas Base/condicionales/inyectadas/🔗 y
Balance Final. **Orden obligatorio: §18a (esqueleto) → §18b (cartas) → §18c (validación).**
El §18d genera todo de golpe para aventuras pequeñas.

### §18a — Esqueleto de aventura + Matriz de Cruce (SIEMPRE primero)

```
Eres el arquitecto de aventuras de Onegai RPG (sistema de mazos por actos, GDD sección 20).
Diseña el ESQUELETO de una aventura, SIN escribir todavía el texto de las cartas.

Entrada: tema {TEMA}, tier recomendado {T}, arquitectura {espina_de_pescado|facciones|crawler},
tamaño {N1 cartas Acto I / N2 Acto II / N3 Acto III} (sugerido 10/8/6).

Devuelve, en este orden:
1. "adventure": esquema 5.15 de Arquitectura_Datos_Onegai (id prefijo adventure_, name,
   description, recommendedTier, objectives, possibleEndings 2-4 ligados a fichas).
2. "hilos": 4-6 hilos argumentales. Cada hilo = mini-árbol {nombre, cartas:[códigos A1-xx,
   B2-xx, C3-xx]} con presentación/desarrollo/desenlace repartidos entre actos.
3. "matriz_de_cruce": una fila por carta de Acto I y II ignorable:
   | Código | Título provisional | Verde activa... | Roja activa... |
4. "reparto_por_acto": lista de códigos por acto marcando cada carta como
   base | condicional (con su requisito) | inyectada | cadena (🔗 orden x/y) | balance_final.

REGLAS DURAS (GDD 20.4):
- ≥40% de cartas Base en CADA acto (válvula de escape).
- Toda carta ignorable tiene consecuencia en la matriz (efecto mariposa). Ninguna fila vacía.
- Máx. 1 requisito por Etiqueta de Activación (2 solo excepcional). Acto II solo filtra contra
  A1; Acto III contra A1 y B2. PROHIBIDO requisito hacia delante o dentro del mismo acto.
- Exactamente 1 carta de Balance Final en el Acto III (nunca se filtra, se juega la última).
- Cartas 🔗 solo donde el orden importe de verdad; numeradas x/y consecutivas del mismo acto.
- Si la arquitectura es "facciones": 3-4 facciones REALES del catálogo, 2 cartas por facción
  en Acto I. Si es "crawler": las 🔗 son los descensos de nivel.
```

### §18b — Cartas de Historia (el texto, por lotes de un acto)

```
Eres el guionista de aventuras de Onegai RPG. Te pego el esqueleto aprobado del §18a.
Escribe las cartas del Acto {I|II|III} de la aventura {adventure_id}.

Esquema por carta: {
  id (carta_hist_{aventura}_{codigo}), adventureId, code ("A1-03"), act:1|2|3, title,
  cardKind: "base|condicional|inyectada|cadena|balance_final",
  chain: {order,of} | null                      (solo cadena 🔗),
  activation: {requires:"A1-02", state:"verde|roja"} | null   (solo condicional; máx 1),
  scene: "texto de la escena (el director lo lee/parafrasea)",
  branches: [{when:"A1-02=roja", text:"..."}] | []   (obligatorio ≥2 en inyectadas),
  ignoreHook: "línea 'Si ignoran esto...' que se lee al poner ficha Roja",
  references: { npcIds[], enemyIds[], locationId|null, lootTableId|null, storyId|null },
  balanceTable: [{condition:"≥3 Rojas en Acto I", epilogue:"..."}] | null (solo balance_final)
}

REGLAS DURAS:
- AUTOCONTENCIÓN (20.4.1): cada scene plantea y resuelve un mini-conflicto en la propia
  carta. El mazo se baraja: nada de "como visteis en la carta anterior".
- ignoreHook OBLIGATORIO en toda carta ignorable: 1 frase, en segunda persona, que haga
  sentir la decisión ("Nadie volverá a preguntar por el herrero...").
- Las inyectadas son cardKind base a efectos de filtro, pero sus branches consultan fichas
  concretas de actos ANTERIORES y las dos ramas deben ser jugables (no "no pasa nada").
- references con ids REALES del catálogo (enemigos, npcs, loot, mapa). El antagonista de
  cada combate es un enemigo real de tier acorde. Sin referencias inventadas.
- Combates acordes al tier: usa la vida/rango del §9; jefes solo en Acto III.
- balanceTable: 4-6 filas que cubran TODOS los hilos de la matriz, incluidos los podados;
  condiciones contables en mesa (nº de fichas o ficha concreta), sin ambigüedad.
- Salida: un JSON por carta, destino data/aventuras/{adventure_id}/cartas/<code>.json,
  + tabla-resumen (code, kind, requisito, 1 línea) para revisión.
```

### §18c — Validación de aventura (pásalo SIEMPRE antes de guardar)

```
Eres el revisor de aventuras de Onegai RPG. Te pego el esqueleto (§18a) y todas las cartas
(§18b) de la aventura {adventure_id}. NO reescribas nada: audita y devuelve un informe.

Comprueba y lista toda violación con su código de carta:
1. FILTRO: ningún requisito apunta hacia delante ni al propio acto; Acto II solo usa A1,
   Acto III solo A1/B2; máx 1 requisito por carta.
2. MARIPOSA: toda carta ignorable aparece en la matriz con consecuencia Verde Y Roja reales
   (existe la carta o la rama que cita). Consecuencias huérfanas = error.
3. VÁLVULA: % de Base por acto (falla si <40%). Simula el peor caso (todo Rojas) y el mejor
   (todo Verdes): ¿cuántas cartas sobreviven al filtro en Acto II y III? Falla si <3.
4. CADENA: numeraciones x/y completas y sin huecos; ninguna 🔗 con activation.
5. BALANCE FINAL: exactamente 1; sus condiciones son contables y cubren todos los hilos.
6. REFERENCIAS: todo id de references existe en el catálogo (te lo pego).
7. AUTOCONTENCIÓN: señala toda scene que dependa de otra carta para entenderse.
Devuelve: tabla {carta, regla violada, gravedad, arreglo sugerido} + veredicto APTA/NO APTA.
```

### §18d — Pack "aventura completa de una vez" (solo aventuras cortas, ≤15 cartas)

```
Eres el equipo completo de diseño de aventuras de Onegai RPG. Crea una aventura COMPLETA
del sistema de mazos por actos (GDD §20): esqueleto §18a + todas las cartas §18b + tu
propia pasada de validación §18c, en una sola respuesta.

Entrada: {TEMA}, tier {T}, arquitectura {...}, tamaño 6/5/4 (máximo para este pack).
Devuelve un único JSON: { "adventure", "hilos", "matriz_de_cruce", "cartas": [...] }
+ el informe de validación al final. Si tu propia validación falla, corrige y revalida
antes de responder. Destino: data/aventuras/{adventure_id}/.
+ REGLAS OBLIGATORIAS + CATÁLOGO (enemigos, npcs, loot, historias, naciones, ciudades, zonas, facciones)
```

> **Nota de implementación.** La Carta de Historia de esta sección es el equivalente digital
> del tipo físico del GDD §20; el campo `storyDeck` del esquema `adventure` (Arquitectura 5.15)
> lista sus ids. No existe todavía pantalla web: los archivos se adelantan en `data/aventuras/`
> y el tipo se estrenará con el patrón Modelo→Controlador habitual.

---

## Plan de producción por cantidades (petición 2026-07)

Estado real del catálogo hoy y qué prompt usar para cada lote. Regla general: **lotes de ≤20**,
validación del protocolo al final de cada lote, ids nuevos siempre.

| Objetivo | Tenemos | Faltan | Prompt | Notas |
|---|---:|---:|---|---|
| 400 enemigos (200+200) | **✅ HECHO — 400 en `data/cartas/enemigos`** (`scripts/generar_enemigos.py`): 50 semillas del usuario adaptadas a ed. 2 + 350 en 15 facciones del mundo | 0 | §9 | 254 criaturas · 126 élites (2 fases) · 20 jefes (3 fases). Condiciones por id real (las 10 del manual ya son cartas), loot real, sin dados (7.6), vida = fórmula §9. Pendiente: pantalla web (patrón Modelo→Controlador) |
| 50 jefes/mini-jefes | **✅ HECHO — 50 enemigos importantes con villainProfile completo** (`scripts/generar_jefes.py`): 20 jefes existentes enriquecidos + 15 líderes de facción (jefe, incl. el Rei Llop Despierto t5) + 15 mini-jefes con nombre (elite t3) | 0 | §17 | Todos con: objetivo/método/debilidad narrativa explotable/vínculo real (deidad, clase o facción), arena con peligro, acciones legendarias (jefes), storyCard y loot mítico condicionado a alcanzar la fase de Desesperación |
| 300 equipamientos | **✅ HECHO — 300/300 normales + 100 artefactos + 31 consumibles** (`generar_equipo_data.py` + `generar_equipo_data_2.py`) + 50 kits | 0 | §1 | 2ª ola: 108 piezas uncommon/rare/epic para torso/cabeza/piernas/pies/secundaria, 19 armas con nombre y regla propia, 12 habilidades de objeto (linkedSkill real), 30 consumibles (varios curan condiciones reales). Pendiente técnico: compatibilitySymbols/precisionStat como campos del modelo (tarea 27) |
| 100 artefactos | **✅ HECHO — 100 reliquias A-001..A-100** (`scripts/generar_artefactos.py`): 5 por deidad (80 epic t3 + 20 mythic t4) | 0 | §15 | Cada una con uniqueRule acotada, burden con mordida y awakening que desbloquea el MILAGRO real (t3) de su deidad. Viven en data/cartas/armas con artifact:true |
| 200 loots | **✅ HECHO — 200 tablas en `data/loot/`** (`scripts/generar_loot.py`): 75 facción×tier + 50 de jefes importantes + 75 contenedores (cofre/alijo/relicario × 5 entornos) | 0 | §16 | Drops 100% ids reales; artefactos SOLO por condición (fase Desesperación, abrir sin forzar...); toda tabla t3+ puede soltar pista |
| 200 cartas de deidad | **✅ HECHO — panteón de 20 deidades + 200 habilidades divinas** (`scripts/generar_panteon.py`) | 0 | §7 | Regla nueva GDD 10.10: divinas universales (cualquier clase), máx. 3 por personaje — capa de personalización. Cada deidad: favor + obligación + 10 habilidades (5×t1 cura/bendición/golpe/guardia/utilidad, 4×t2, 1 milagro t3). Cierra las 4 deidades que las razas del GDD ya referenciaban (Rey Lobo, Corte Salvaje, Padre de la Forja, Reina Velada) |
| 300 historias | **✅ HECHO — 300 en `data/historias`** (`scripts/generar_historias_npcs.py`): 15 facciones × 4 tramas madre × 5 escalones encadenados | 0 | §10 | Antagonista = id real del bestiario; recompensa = tabla de loot real; meses afines reales |
| 100 NPCs | **✅ HECHO — 133 en `data/npcs`** (12 canon de las misiones del usuario + generados) | 0 | §12 | Agenda + palanca + secreto→historia real + mes de nacimiento real |
| 200 invocaciones | **✅ HECHO — 200** (`scripts/generar_invocaciones_trampas_monturas.py`): 14 familias temáticas, cada una con su hechizo real invocar_* (196 nuevos) | 0 | §6 | Control automatica/jugador/car_invocador; vida 6+3×tier; límite por INT |
| 30 clases nuevas | **✅ HECHO — 56/56 clases completas** (26 originales + 30 nuevas en 3 lotes, todas al estándar Ancla del Vacío) | 0 | §2 | Las 30 nuevas reequilibran el sistema: stats CAR 18 · INT 14 · CON 13 · DES 11, roles caster 14 · agile 11 · tank 11 · balanced 10 · support 10. Novedades: 1ª clase montada (Jinete de Tormentas + corcel_de_tormenta ♞), caster de CON (Cantor de la Tierra), clases que juegan con las pilas del grupo (Maestro de Ceremonias), con el calendario (Peregrino de los Doce Meses) y con consumibles (Quemador de Reliquias, Cocinero) |
| Habilidades t1 (todas las clases) | solo Ancla del Vacío completa | ~80-170 | §3 | 1 pasiva + 3 habilidades × clase; por tandas de 5 clases |
| 50 trampas | **✅ HECHO — 50 en `data/cartas/trampas`** (10 conceptos × 5 tiers) | 0 | §14 | Detección y desarme como pruebas distintas; onFail peor que no intentar; condiciones por id real; 10 narrativas |
| 50 monturas | **✅ HECHO — 50 en `data/cartas/monturas`** (10 especies × 5 tiers) | 0 | §13 | mountedSkill propio con recovery, derribo con prueba de DES, upkeep narrativo, afinidad ♞ |

**Sistema para usarlo después en la web** (en este orden):

1. Cada lote se guarda como un archivo por carta en su carpeta de `data/` + entrada en el
   `index.json` del dominio (patrón de los scripts onegai).
2. Tipos que la web ya lee (equipo, clases, habilidades, deidades, invocaciones, razas...):
   caen directos en `data/cartas/<tipo>/` con el esquema de la app y aparecen en su catálogo.
3. Tipos nuevos sin pantalla (enemigos, historias, NPCs, monturas, trampas, artefactos*, loot):
   se guardan ya en `data/` con estos esquemas, y cada tipo se estrena en la web replicando el
   patrón existente Modelo→Form→Repositorio→Servicio→Controlador→plantillas de 5 zonas
   (~1 tarde por tipo; los artefactos no necesitan tipo nuevo: son equipo con 3 campos extra).
4. Los 4 scripts generadores se retocan una vez (mapa de enums del documento
   `Valoracion_Scripts_Generadores.md`) para emitir directamente estos esquemas.

**Decisiones de diseño ya tomadas (no reabrir sin motivo):**

- **Daño:** éxitos de la tirada de ataque × multiplicador del arma (△ ×1 · ○ ×1,5 · □ ×2 ·
  hechizos ×1,5) + bonos fijos de carta. Sin dados de daño. Regla completa: GDD sección 7.6.
  En los prompts: las armas NUNCA declaran dado de daño; su peso es su multiplicador.
- **Destino:** todo lote se guarda en `data/` con el esquema de la app. `JSONS/onegai/` no se usa.

---

## Protocolo de lote (para cualquier tipo)

1. Listar ids existentes del tipo → excluirlos.
2. Generar el lote (≤20 a mano/IA; por script, validar contra el esquema).
3. Validar: JSON parsea · ids únicos · referencias existen · presupuesto de balance respetado.
4. Tabla-resumen (id, tier, rareza, 1 línea de efecto) para revisión humana.
5. Guardar un archivo por carta en su carpeta. Nunca tocar archivos existentes.
