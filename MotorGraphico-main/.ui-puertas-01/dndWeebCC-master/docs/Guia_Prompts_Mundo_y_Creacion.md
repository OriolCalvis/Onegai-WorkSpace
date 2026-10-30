# Guía de prompts — Mundo, mapa y elementos de creación

Complementa `Plantilla_Prompt_Contenido.md` (que cubre las 17 familias de cartas). Esta
guía cubre lo que **aún no tiene editor cómodo en la web** o resulta tedioso: el mapa
mundi entero (naciones, ciudades, zonas, cronología, puntos) y los elementos narrativos
(historias, PNJs, loot, eventos de juego). Cada prompt está pensado para que el usuario
haga el mínimo esfuerzo: pega el catálogo, pide en una frase, y recibe JSON listo.

---

## 0. El contrato común (léelo una vez, aplícalo siempre)

Todo prompt de esta guía comparte estas reglas. Cópialas al final de cualquier prompt:

```text
REGLAS OBLIGATORIAS
1. NO INVENTES IDS. Solo puedes referenciar ids del CATÁLOGO que te pego abajo.
   Si necesitas algo que no existe, devuélvelo en una lista aparte "sugerencias_nuevas"
   (nombre + para qué), nunca como referencia.
2. El id de cada elemento nuevo es snake_case sin acentos y será también su nombre de
   archivo (id == archivo.json). No repitas ids del catálogo: jamás sobrescribas.
3. Devuelve SOLO JSON válido (sin comentarios, sin markdown alrededor).
4. Coordenadas del mapa: enteros dentro de 1600×1200 (sistema de mapamundi.png).
5. Español neutro de mesa. Nada de poder mecánico en campos narrativos.
6. Inspiración D&D 5e permitida; contenido propietario (nombres de monstruos/hechizos
   registrados) prohibido.

CATÁLOGO VIGENTE
{{pega aquí la salida de: python3 scripts/exportar_catalogo_ids.py <catálogos que apliquen>}}
```

**El catálogo nunca se escribe a mano**: `scripts/exportar_catalogo_ids.py` lo genera
desde los datos reales (`clases`, `historias`, `naciones`, `ciudades`, `zonas`,
`facciones`, `eventos_historicos`, `npcs`, `deidades`... o sin argumentos para todo).

Tras generar contenido: guardar los archivos donde diga cada sección, arrancar la app y
mirar `/diagnostico` (la revisión de integridad avisa de cualquier referencia rota), y
`python3 scripts/auditar_campos_referencia.py` si se tocaron formularios.

---

## 1. Mapa mundi

Todo vive junto en `data/mapa/geografia.json` (claves: `naciones`, `marcadores`,
`puntos`, `ciudades`, `zonas`, `eventosHistoricos`). Los prompts devuelven el fragmento
a añadir dentro de la clave correspondiente; se puede pegar a mano o desde el editor de
/mapa (que guarda el JSON completo).

### §M1 — Nación / país

Esquema actual: `{nombre, color, points}` — points = polígono `"x1,y1 x2,y2 ..."`.

```text
Eres cartógrafo del mundo de Onegai RPG. Crea {{N}} nación(es) nueva(s).

Para cada una devuelve:
{
  "nacion": { "nombre": "...", "color": "#rrggbb", "points": "x1,y1 x2,y2 ..." },
  "marcador": [x, y],
  "identidad": {
    "gobierno": "...", "capital_sugerida": "...", "economia": "...",
    "relaciones": { "aliada_de": ["nación existente"], "tension_con": ["..."] },
    "gancho_de_mesa": "una frase que dé ganas de viajar allí"
  }
}

Reglas del polígono: 6-12 vértices, dentro de 1600×1200, SIN solaparse con las naciones
existentes (te pego sus polígonos abajo), color no usado por ninguna otra.
El bloque "identidad" es para la descripción y futuras ampliaciones: no va al JSON del
mapa todavía, guárdalo como comentario de diseño.
+ REGLAS OBLIGATORIAS + CATÁLOGO (naciones con sus points, ciudades)
```

### §M2 — Ciudad o pueblo

Esquema actual: `{id, nombre, nacion, tamano(capital|ciudad|pueblo|aldea), poblacion,
gobernante, rasgo, descripcion, x, y, historiaIds[], npcIds[]}`.

```text
Eres el urbanista de Onegai RPG. Crea {{N}} asentamiento(s) en {{NACION}}.

Mezcla de tamaños coherente (1 capital máximo por nación). Coordenadas DENTRO del
polígono de la nación (te lo pego). "rasgo" = lo que la hace única y jugable
("puerto de contrabando", "las campanas suenan solas al alba"). "poblacion" = texto
("~12.000", "unas 300 almas"). Vincula historiaIds/npcIds SOLO del catálogo cuya
localización encaje; si no encaja ninguna, listas vacías.
Devuelve: array JSON de ciudades listo para pegar en "ciudades".
+ REGLAS OBLIGATORIAS + CATÁLOGO (naciones con points, ciudades, historias, npcs)
```

### §M3 — Zona de terreno (montañas, bosques, ruinas...)

Esquema actual: `{id, nombre, tipo, nacion, peligro, descripcion, x, y, historiaIds[]}`.
Tipos con icono en el mapa: montana, bosque, lago, rio, desierto, pantano, ruina,
fortaleza, santuario, puerto, isla, paso.

```text
Eres el geógrafo de Onegai RPG. Crea {{N}} zona(s) de terreno para {{NACION o "todo el mapa"}}.

"peligro" = amenaza concreta de mesa ("nidos de arpía en los riscos", "avalanchas en
deshielo"), no un número. "descripcion" = 2-3 frases evocadoras + 1 detalle explotable
por un director. Reparte tipos con lógica geográfica (los ríos nacen en montañas, los
puertos tocan costa). Vincula historiaIds solo si la localización de la historia encaja.
Devuelve: array JSON listo para pegar en "zonas".
+ REGLAS OBLIGATORIAS + CATÁLOGO (naciones, zonas, historias)
```

### §M4 — Evento histórico (cronología)

Esquema actual: `{id, titulo, ano(entero), era, lugar, descripcion, consecuencias}`.

```text
Eres el cronista de Onegai RPG. Crea una cronología de {{N}} eventos históricos.

"ano" es un entero (negativos = antes de la era actual); deben ser coherentes entre sí
y con los eventos ya existentes que te pego. "lugar" debe ser el NOMBRE EXACTO de una
nación, ciudad o zona del catálogo. "consecuencias" es lo jugable: qué quedó del suceso
que la mesa aún pueda tocar hoy (una ruina, un odio entre naciones, una reliquia — si
mencionas una reliquia, que sea un artefacto del catálogo de equipo).
Cubre distintas escalas: fundaciones, guerras, catástrofes, milagros, pactos.
Devuelve: array JSON listo para pegar en "eventosHistoricos".
+ REGLAS OBLIGATORIAS + CATÁLOGO (naciones, ciudades, zonas, eventos_historicos, equipo, deidades)
```

### §M5 — Punto de interés suelto

Esquema actual: `{id, nombre, icono(emoji), x, y, historiaIds[], aventuraIds[], eventoIds[]}`.
Para localizaciones que no son ni ciudad ni accidente geográfico (una posada de cruce,
un obelisco). Mismo patrón que §M3 con icono libre.

### §M6 — Pack "país vivo" (el que ahorra más tiempo)

```text
Eres el equipo completo de worldbuilding de Onegai RPG. Crea un PAÍS VIVO completo:

1 nación (§M1) + su capital y 2-3 asentamientos (§M2) + 3-4 zonas de terreno (§M3)
+ 3 eventos históricos que la expliquen (§M4) + 4-6 PNJs residentes (§A2)
+ 2-3 historias locales tier 1-2 (§A1) que conecten los PNJs, las zonas y algún evento.

Todo debe cruzarse: las historias ocurren en las ciudades/zonas creadas, los PNJs
aparecen en historiaIds/npcIds, las consecuencias de los eventos históricos se notan
en los rasgos de las ciudades. Devuelve un único JSON con las claves:
{ "nacion", "marcador", "ciudades", "zonas", "eventosHistoricos", "npcs", "historias" }
+ REGLAS OBLIGATORIAS + CATÁLOGO (todo: python3 scripts/exportar_catalogo_ids.py)
```

---

## 2. Elementos de la app sin editor cómodo

### §A1 — Historia

**Esquema canónico y reglas:** ver `Plantilla_Prompt_Contenido.md §10` (fuente única de verdad
para historias). Resumen operativo: el esquema de los 300 archivos reales en `data/historias/` es
`{id, title, type:"story", hook, location, antagonist, tierRange[min,max], reward, decision,
complication, months[ids mes_*], faction, chain{trama, step, of}, consequence?, flavorText}`.

```text
Eres guionista de Onegai RPG. Crea {{N}} historias para {{facción/lugar/tema}}.

"antagonist" = id EXACTO de un enemigo del catálogo (rango acorde al tierRange).
"location" = nombre exacto de nación/ciudad/zona del mapa. "months" = 0-2 ids mes_*
si la historia resuena con esos trasfondos. "chain" solo si forman una trama
(mismo "trama", "step" 1..n). "decision" = un dilema real sin opción claramente buena.
"reward" puede nombrar una tabla de loot del catálogo.
Destino: un archivo por historia en data/historias/<id>.json.
+ REGLAS OBLIGATORIAS + CATÁLOGO (enemigos, historias, loot, facciones, naciones, ciudades, zonas, trasfondos)
```

### §A2 — PNJ

**Esquema canónico y reglas:** ver `Plantilla_Prompt_Contenido.md §12` (fuente única de verdad
para PNJs). Resumen operativo: el esquema de los 133 PNJs reales en `data/npcs/` es
`{id, name, type:"npc", role, location, faction, agenda, attitude{inicial, palanca},
dialogue[3], services|null, secretHook, month(id mes_*), flavorText}`.

```text
Eres el director de casting de Onegai RPG. Crea {{N}} PNJs para {{lugar/facción}}.

"agenda" = qué quiere DE VERDAD (no su oficio). "secretHook" = el secreto que un grupo
puede descubrir y usar. "dialogue" = 3 frases con su voz propia (que se distinga de
cualquier otro PNJ del catálogo). "services" = qué ofrece mecánicamente a la mesa
(comercio, información, transporte, curación...). "location" = lugar exacto del mapa.
Destino: data/npcs/<id>.json.
+ REGLAS OBLIGATORIAS + CATÁLOGO (npcs, facciones, naciones, ciudades, zonas, historias)
```

### §A3 — Tabla de loot

Esquema real (el de los 200 archivos ya en `data/loot/`): `{id, name, type:"loot_table",
tierRange[min,max], drops[{item, chance, condition}], gold{min,max}, clue, flavorText}`.
**Importante: `chance` en escala 1-100, NO 0-1. La clave de cada drop es `item`, NO `itemId`.**
Esquema canónico idéntico al `§16` de `Plantilla_Prompt_Contenido.md` (fuente única de verdad).

```text
Eres el tesorero de Onegai RPG. Crea {{N}} tablas de loot para {{contexto}}.

"drops" solo con ids reales de equipo/consumibles/artefactos (clave "item", no "itemId"),
rarezas acordes al tierRange (nada épico en tier 1). "chance" en escala 1-100 (no 0-1),
y la suma de valor esperado debe ser modesta: el loot complementa, no rompe la economía.
"clue" = id de historia/misión que puede caer, o null. "condition" = null salvo para mythic
(que solo cae por condición: "derrotar sin descansos", "fase 3 alcanzada"...).
Destino: data/loot/<id>.json (prefijo loot_).
+ REGLAS OBLIGATORIAS + CATÁLOGO (equipo, consumibles, loot, historias)
```

### §A4 — Evento de juego (disparadores en partida)

Esquema real (`data/eventos/*.json`): `{id, name, description, tier, faction,
trigger{type(manual|temporal|condicion|lugar|historia), condition}, effects[],
playerOptions[{id, text, requirementText, consequenceText}], continuesTo, oneTime}`.

```text
Eres el diseñador de eventos de Onegai RPG. Crea {{N}} eventos de juego.

Cada evento = una situación que INTERRUMPE la partida y exige elegir. 2-4 playerOptions
con requisitos distintos (una física, una social, una de recursos...) y consecuencias
que importen. "continuesTo" solo con un id existente de evento o historia.
"trigger.condition" concreta y comprobable en mesa.
Destino: data/eventos/<id>.json (prefijo evento_).
+ REGLAS OBLIGATORIAS + CATÁLOGO (eventos, historias, facciones, condiciones)
```

### §A5 — Aventuras por actos (mazos de historia)

**Fuente única de verdad:** `Plantilla_Prompt_Contenido.md §18` (§18a esqueleto+matriz →
§18b cartas → §18c validación; §18d pack completo para aventuras cortas). Implementa el
sistema de la sección 20 del GDD: fichas 🟢/🔴, filtrado entre actos, cartas Base/
condicionales/inyectadas/🔗 y Balance Final. Destino: `data/aventuras/{adventure_id}/`.

### Cartas (clases, enemigos, equipo, hechizos...)

Ya cubiertas en `Plantilla_Prompt_Contenido.md` §1-§17 — usar aquellas plantillas, que
incluyen las reglas de balance (vida por rango, máx 1 truco activa por clase, daño =
éxitos × multiplicador, etc.). Esta guía no las duplica.

---

## 3. Ampliaciones posibles (qué información podríamos añadir)

Prioridad sugerida según lo que más facilitaría la vida en mesa:

| Elemento | Hoy tiene | Ampliación que más aporta |
|---|---|---|
| Nación | nombre, color, polígono | gobierno, capitalId, relaciones diplomáticas (alianzas/tensiones), rasgo económico → daría contexto automático a historias y precios |
| Ciudad | nación, tamaño, población, gobernante, rasgo | servicios disponibles (mercado, templo de deidadId, gremio), precios locales, festividad del calendario (mes_*) → conectaría con trasfondos |
| Zona | tipo, peligro, descripción | tabla de encuentros por tier (enemigoIds + pesos), recurso cosechable (itemId), clima → exploración procedural |
| Evento histórico | año, era, lugar, consecuencias | reliquiaId (artefacto), facciones implicadas, eventoHistoricoId padre → cronología navegable y botín con historia |
| Historia | antagonista, facción, cadena | lootTableId explícito, zonaId/ciudadId (id en vez de texto libre "location") → enlaces navegables en el mapa |
| PNJ | agenda, secreto, diálogo | ciudadId (id en vez de texto), inventario de mercader (itemIds), actitud por facción → comercio jugable |
| Deidad | dominio, favor, milagro | grantedSpells editable en web, festividad (mes), templos (ciudadIds) |
| Trasfondo | characterCreation completo | editor web acorde al Prompt Maestro §4b (hoy el formulario usa el esquema viejo) |

Cuando se implemente una ampliación: añadir el campo al modelo Java (Jackson ignora los
campos extra mientras tanto, así que el JSON puede adelantarse), convertir su selección
en desplegable si referencia otro catálogo (regla de `Auditoria_Campos_Referencia.md`)
y añadir el campo a `CAMPOS_REFERENCIA` del script de auditoría.
