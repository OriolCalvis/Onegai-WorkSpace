# Formato de aventuras — la línea de eventos

Cómo se escribe una historia en este motor, y cómo se hace crecer sin
reescribir nada.

- Motor: [`include/RPG/NarrativeEngine.h`](include/RPG/NarrativeEngine.h) · [`src/RPG/NarrativeEngine.cpp`](src/RPG/NarrativeEngine.cpp)
- Ejemplo real: [`assets/adventures/aventura_silbido_medianoche.json`](assets/adventures/aventura_silbido_medianoche.json)
- Prueba ejecutable: [`examples/demo_aventura.cpp`](examples/demo_aventura.cpp) → `./build/demo_aventura`

---

## 1. El modelo, en tres frases

1. El mundo tiene un conjunto de **flags** (strings). Eso es *todo* el estado narrativo.
2. Una aventura es una lista de **beats**. Cada beat dice *cuándo* entra en juego (trigger + condición sobre flags), *qué se ve* (speaker + líneas) y *qué cambia* (efectos).
3. Cuando pasa algo, el motor recorre los beats **en orden de archivo** y dispara **el primero que encaja**. Uno solo. Después aplica sus efectos.

```
el jugador habla con "guardia"
        │
        ▼
  NarrativeEngine::fire(Talk, "guardia", state)
        │
        ├─ beat_guardia_pista   trigger=talk:guardia  requires notFlags[pista_guardia]  ✔ ── gana
        ├─ beat_guardia_repite  trigger=talk:guardia  requires allFlags[pista_guardia]  ✘
        └─ ...
        │
        ▼
  enciende la flag, devuelve las líneas al HUD
```

No hay grafo, ni máquina de estados, ni número de etapa. **El orden lo
impone la condición, no la estructura** — y por eso la aventura de
ejemplo se puede resolver en cualquiera de las 6 permutaciones sin una
sola línea de código dedicada a ello.

---

## 2. El archivo

```jsonc
{
  "id": "adv_silbido_medianoche",     // obligatorio
  "name": "El Silbido de Medianoche",
  "description": "...",

  // Diario de misiones. Un objetivo NO tiene lógica: es texto atado a
  // una flag. Se marca hecho cuando la flag está encendida.
  "objectives": [
    { "id": "obj_guardia", "text": "Preguntar al guardia...", "doneFlag": "silbido_pista_guardia" }
  ],

  "beats": [
    {
      "id": "beat_guardia_pista",              // obligatorio, único
      "trigger": { "type": "talk", "target": "guardia" },
      "requires": { "notFlags": ["silbido_pista_guardia"] },
      "speaker": "Guardia",                    // vacío = voz del narrador
      "lines": ["...", "..."],
      "effects": [
        { "type": "setFlag", "arg": "silbido_pista_guardia" }
      ],
      "once": false                            // opcional
    }
  ]
}
```

### Triggers

| `type`  | `target`            | Cuándo se dispara |
| ------- | ------------------- | ----------------- |
| `talk`  | `objectId` del NPC  | El jugador habla con ese NPC (`GameSession::interact()`) |
| `enter` | ruta del nivel      | Se carga ese nivel (`GameSession::enterLevelNarrative()`) |
| `auto`  | — | En cada `tick()`. La sesión lo llama al cerrar cada diálogo |

`auto` es el trigger de los **cierres y las consecuencias diferidas**:
"cuando se cumplan estas flags, pase lo que pase". Es lo que evita tener
que colgar el final de la aventura del último NPC de la cadena.

### Condición (`requires`)

Las tres listas se combinan con AND entre sí. Omitirla entera = siempre cierta.

| Campo      | Significado |
| ---------- | ----------- |
| `allFlags` | hacen falta **todas** |
| `anyFlags` | hace falta **al menos una** |
| `notFlags` | no debe haber **ninguna** |
| `nivelMinimo` | el héroe tiene **ese nivel o más** (NIVELES-01) |
| `tierMinimo` | el héroe está en **ese tier o más** |
| `variables` | cada variable con nombre, **dentro de su rango** (VARIABLES-01): `{"afecto_venides": {"min": 1}, "franja": 2}`. Un número es el valor exacto; un objeto, `min` y/o `max` (los dos incluidos). Una variable que nadie ha tocado vale 0. |
| `moral` | la moral del héroe (la ya asentada, de −100 a 100) **dentro del rango** (MORAL-01): `{"min": 20}` es «Respetado o más»; `{"max": -20}`, «Canalla o menos». Sirve para **añadir** líneas u opciones según el tramo, nunca para cerrar el camino (wf-04: «el villano juega, no es expulsado del juego»). |
| `lugar` | el nivel donde está el jugador (DIALOGOS-01): `"assets/levels/interior_herreria.json"` o una lista. El mismo PNJ dice otra cosa en otro sitio. Si el juego no sabe dónde está (demos sin niveles), no se mira. |

**El día y la franja** (FASE1-BND-01) son dos variables más: `dia` (1, 2, 3...) y
`franja` (0 = mañana, 1 = tarde, 2 = noche). Las pone el efecto `pasarFranja`
(abajo) y un beat las pide como cualquier variable: `{"franja": 0}` es «por la
mañana» (Xila sobrio), `{"franja": {"max": 1}}` «de día», `{"franja": 2}` «de
noche» (el carterista, el vigilante del Barrio Alto). El HUD las enseña en la
cabecera del minimapa («Día 1 · Mañana») en cuanto `dia` vale 1 o más. No es el
reloj de los temporizadores: aquel cuenta segundos de exploración; esto avanza
cuando la historia lo dice.

**El héroe que se creó** (HEROE-01). Lo que deja el cuaderno de creación
(raza, clase, origen, deidad, las siete respuestas, el nombre y los atributos)
llega a las aventuras sin campos nuevos: la sesión pone, antes de cada disparo,
flags `heroe_*` (`heroe_raza_<id>`, `heroe_pueblo_<categoría>`,
`heroe_clase_<id>`, `heroe_rol_<rol>`, `heroe_origen_<id>`,
`heroe_elemento_<elemento>`, `heroe_deidad_<id>`, `heroe_fe_<categoría>`,
`heroe_vinculo_<id>`... `heroe_virtud_<id>`), las variables `heroe_con`,
`heroe_des`, `heroe_int` y `heroe_car`, y los textos `{nombre}`, `{raza}`,
`{pueblo}`, `{clase}`, `{origen}`, `{deidad}`, `{vinculo}`, `{miedo}`,
`{defecto}`, `{meta}`, `{ideal}`, `{caracter}` y `{virtud}`, que se sustituyen
en `lines`, `speaker` y el `texto` de las opciones. Las categorías (cómo mira
Boundington a cada pueblo, qué fe es cuál) están en
`assets/rules/heroe_en_la_historia.json`. Una aventura nunca escribe una flag
`heroe_*`.

Un beat `auto` con `nivelMinimo` salta solo cuando el héroe llega a ese nivel
(al cerrar el diálogo en que sube): es la forma de que las misiones de más
prestigio **surjan** en el mundo en vez de anunciarse con «necesitas nivel 8»
(Oriol, 06/10: «que sea una necesidad surgida, no algo estático»). El nivel no
da atributos; solo abre estas puertas y trae recompensas.

### Opciones (`opciones`, OPCIONES-01)

Lo que el jugador puede **contestar** al acabar de leer un beat (fase 1 de «Los
Perdidos de Boundington — revisión para jugarla entera»: unirse al culto o no,
entregar o salvar a Verina, ayudar a Venides o acabar con él). Hasta cuatro;
el beat necesita `lines` (la pregunta).

```json
{"id": "b3_sandes", "trigger": {"type": "talk", "target": "sandes"}, "speaker": "Sandes",
 "lines": ["Dame a la chica y te dejo ir."],
 "opciones": [
   {"texto": "Entregarle a Verina", "lines": ["Sabía que eras listo."],
    "effects": [{"type": "setFlag", "arg": "bnd3_verina_entregada"}]},
   {"texto": "Negarte", "effects": [{"type": "setFlag", "arg": "bnd3_sandes_negado"}]},
   {"texto": "Pedirle a Venides que hable", "requires": {"variables": {"afecto_venides": {"min": 1}}},
    "effects": [{"type": "addVariable", "arg": "afecto_venides", "value": -1}]}
 ]}
```

| Campo | Significado |
| ----- | ----------- |
| `texto` | Lo que se lee en la lista (obligatorio, 60 letras como mucho). |
| `requires` | Como la del beat: la opción solo sale si se cumple, mirada justo **después** de los efectos del beat. |
| `speaker` | Quién dice las `lines` de la opción. Sin él, el mismo del beat; `""`, el narrador. |
| `lines` | Lo que se lee al elegirla. Sin líneas, el diálogo se cierra y la aventura sigue por sus flags (un beat `auto` que las pida salta al cerrarse). |
| `effects` | Los mismos efectos que un beat, en orden. |
| `sigue` | `true`: **la charla sigue** (DIALOGOS-01). Después de esta opción (y de lo automático que salte por ella: una tirada se cuenta antes) se vuelve a disparar el mismo trigger del beat, y el primer beat que encaje ofrece lo que queda por hablar. Un combate la corta. |
| `pasaPagina` | `true`: es «Hablar de otra cosa» de una charla (va a la página siguiente). |
| `despedida` | `true`: es la salida de la charla. |

**Las charlas** (DIALOGOS-01, «Boundington: los diálogos»: «que se sienta D&D»).
Una conversación es un beat de apertura y unas **páginas** (`"pagina": true` en
el beat) con hasta tres temas cada una, «Hablar de otra cosa» y la despedida.
Cada tema se pregunta una vez (su opción pide `notFlags` de su propia flag) y
lleva `sigue`. Si una página se ha quedado **sin temas** (ya preguntados, o
que piden algo que el héroe no tiene), el motor no la enseña: pasa página solo,
sin decir nada, hasta la primera que tenga algo; y si no le queda nada a
ninguna, se queda en ella sin «Hablar de otra cosa». Las flags de la
contabilidad de una charla `P` son `P_t_<tema>` (preguntado), `P_visto`,
`P_en_charla`, `P_abierta` y `P_n_<tema>` (un tema con sus propias preguntas),
y la variable `P_pag`. Las de Boundington se generan con
`out/claude_dialogos02/guiones/lib.py` (`charla(...)`).

En el juego, en la última página del diálogo sale la lista encima de la hoja:
W/S (o las flechas) mueven la elegida, 1-4 eligen directamente y ENTER decide.
Mientras haya una elección pendiente, el diálogo no se cierra. El registro
apunta «Eliges: …».

### Temporizadores de misión (F1, bitácora 2.18)

Campo de primer nivel `timers` (opcional). Tiempo **narrativo**: no es el
calendario del juego ni segundos crudos — es el reloj de UNA misión, y
solo avanza mientras el jugador explora activamente (el orquestador lo
congela en combate, diálogo, pausa y AFK). La casa arde si no superas la
zona antes de irte; el rescate tiene un tiempo; el asesino vuelve a matar
si tardas demasiado.

| Campo | Significado |
|---|---|
| `id` | Identificador del timer (los beats lo arrancan con `startTimer`) |
| `nombre` | Opcional. Lo que lee el jugador en el HUD («El incendio»); sin él, el HUD pone «TIEMPO». Solo se escribe al guardar si lo tiene |
| `duracion` | Segundos ACTIVOS (obligatorio si expira por tiempo) |
| `arrancaPor` | Flag que lo arranca (alternativa al efecto `startTimer`) |
| `expiraPor` | `["tiempo"]`, `["salir_de_zona"]` o ambas; por defecto `["tiempo"]` |
| `zona` | Nivel donde corre (obligatorio con `salir_de_zona`) |
| `flagExpira` | Flag al expirar, por cualquier causa (obligatorio) |
| `presentacion` | `contador`, `barra` o `secreto` (el jugador no lo ve; solo nota lo que pasa en el mundo) |
| `avisos` | `[{fraccion, flag}]`: fracción del tiempo RESTANTE (0..1] que enciende la flag al cruzarse |

Al expirar se enciende `flagExpira` y las consecuencias las resuelven los
beats `auto` de siempre — el timer no inventa efectos nuevos:

```jsonc
{
  "timers": [
    {
      "id": "casa_en_llamas",
      "duracion": 60.0,
      "expiraPor": ["tiempo", "salir_de_zona"],   // la casa arde igual si te vas
      "zona": "nivel_casa",
      "flagExpira": "casa_quemada",
      "presentacion": "secreto",                   // el mundo avisa, no el HUD
      "avisos": [ { "fraccion": 0.5, "flag": "humo_en_la_escalera" } ]
    }
  ]
}
```

### Efectos

| `type`      | Campos | Efecto |
| ----------- | ------ | ------ |
| `setFlag`   | `arg`  | Enciende una flag |
| `clearFlag` | `arg`  | Apaga una flag |
| `grantGold` | `value` | Oro a la partida (negativo = peaje) |
| `log`       | `arg`  | Línea suelta al log de la partida |
| `skillCheck` | `skillCheck: {skillId, rivalDice, flagBotch, flagPartial, flagSuccess, flagCritical}` | Resuelve una **tirada enfrentada** (Ed. 3, bitácora 2.17): el jugador lanza tantos d6 como el `castingStat` de la skill (del catálogo Nd6) y el rival declarado por el beat tira `rivalDice` dados; el grado de la enfrentada (BOTCH/PARTIAL/SUCCESS/CRITICAL) enciende **una** de las cuatro flags. Así un beat puede ramificar en cuatro variantes por tirada. |
| `startTimer` | `arg` | Arranca el temporizador de misión declarado en `timers` con ese id (F1, bitácora 2.18). El reloj solo avanza en exploración activa (ni combate, ni diálogo, ni pausa, ni AFK). |
| `grantLevel` | `value`, `arg` | Niveles para el héroe (`value` en niveles, con fracciones: `0.25` = un cuarto de nivel; 1 si no se dice, hasta 20; `arg` = por qué, sale en el registro como «Por …»). Veinte niveles por tier: sin los hitos del tier no se pasa del 20 (40, 60, 80) ni se sube de tier (NIVELES-01). |
| `grantMilestone` | `value`, `arg` | Hitos (`value` = cuántos, 1 si no se dice, de 1 a 5; `arg` = por qué, «Hito: …»): uno por capítulo. Para pasar de tier hacen falta el nivel tope (20, 40, 60, 80) y los hitos del tier: 2 en Tier 1, 4 en Tier 2, 8 en Tier 3 y 11 en Tier 4, empezando de cero en cada tier; Tier 5 es el techo. Boundington da uno y el capítulo 2 que se elija, el otro. El guardado lleva tier, nivel e hitos (H2-04). |
| `setVariable` | `arg`, `value` | Pone la variable `arg` a `value` (VARIABLES-01: los puntos de Creadores, Sho-Noco y Gurkazaal, el afecto de Venides, la reputación con un gremio, la franja del día). Las variables se guardan con la partida. |
| `addVariable` | `arg`, `value`, `min`?, `max`? | Suma `value` (o resta, si es negativo) a la variable `arg`, sin salirse de `[min, max]` si se dicen: `{"type": "addVariable", "arg": "rep_mercaderes", "value": 1, "min": 0, "max": 4}`. |
| `pasarFranja` | `arg`? o `value`? | Pasa el tiempo de la historia (FASE1-BND-01). Con `arg` (`"manana"`, `"tarde"` o `"noche"`, sin tilde), hasta la siguiente vez que sea esa franja (0 pasos si ya lo es: «subir a dormir» de noche no pasa nada); sin `arg`, `value` franjas (1 si no se dice, hasta 9). Después de la noche viene la mañana del día siguiente; si la cuenta no había empezado, empieza el día 1. Deja en el registro «Amanece el día 2.», «Llega la tarde.» o «Anochece.». |
| `tablaAleatoria` | `tabla: {dado, flags, que?}` | Tira 1d`dado` (de 2 a 20) y enciende la flag de esa cara: `flags` lleva una por cara, en orden. `que` encabeza la línea del registro («El traje: 1d4, sale un 3.»). Diferida como `skillCheck`: la tira la sesión. Es el 1d4 del traje roto de la fiesta. |
| `descanso` | `arg` | `"corto"`: cura 1d8 por tier del héroe, sin pasar del máximo; `"largo"`: toda la vida (la regla del reglamento, `Sistema_Cartas_Tiers.md` §7.5). Las cartas ya vuelven a la mano en cada combate. La comida con la familia de Pico Dragón es un corto; dormir, un largo. |
| `darObjeto` | `arg`, `value`? | Mete en la mochila `value` (1 si no se dice, hasta 20) unidades del objeto `arg` (un id del catálogo de objetos): «Recibes: Poción x2.». Un id que no está en el catálogo no entra y se avisa en el registro; `validar_todo.py` lo da por error antes. |
| `acto` | `arg` | Apunta en el libro del día el acto `arg`, un id de `assets/rules/actos.json` (MORAL-01, más abajo). La moral no se mueve hasta dormir; la pureza y la reputación con las facciones, en el momento. El registro dice «Presionaste a Aigren con su hijo. Esta noche te pesará.» (o «te honrará»). Va en la opción que se elige (la intención) o en el beat del resultado (lo que pasó), según lo que diga el motivo. Un id que no está en el catálogo se avisa en el registro; `validar_todo.py` lo da por error antes. |
| `herida` | `arg`, `value`? | Hiere al héroe 1d`value` (4 si no se dice; de 2 a 20) por `arg`: «El fuego: 1d4, pierdes 3 PV.». No mata: el héroe se queda como mínimo en 1 PV. Diferida como `skillCheck`: la tira la sesión. |

El `skillCheck` es **diferido** como `grantGold`: el motor narrativo no
tiene RNG ni catálogo, así que solo empaqueta la petición y deja que
`GameSession` la resuelva con `DicePoolEngine` (el mismo del combate, lo
que keeps narrativa y combate en el mismo sistema de reglas). El grado se
traduce a flags — no a magnitud — porque en la narrativa lo que importa es
"qué percibes / convences / averiguas", no cuánto daño.

**Ed. 3:** la dificultad ya no es una CD fija — el reto tira dados: el
guardia «tira» 3, la trampa «tira» 2, una cerradura maestra «tira» 5.
Resultado para el jugador: supera al rival → SUCCESS; empate → PARTIAL;
pierde → BOTCH; pifia (ningún 5/6 y mínimo dos 1es) → BOTCH aunque el
rival también tire mal. Los beats antiguos con `"cd"` siguen cargando:
`GameSession` convierte cd a dados (0.5→1, 1.0→2, 1.5→3, 2.0→4, 2.5→5).

```jsonc
{
  "type": "skillCheck",
  "skillCheck": {
    "skillId": "percepcion",
    "rivalDice": 3,                               // el guardia tira 3d6
    "flagBotch":    "prologo_percibe_nada",
    "flagPartial":  "prologo_percibe_parcial",
    "flagSuccess":  "prologo_percibe_exito",
    "flagCritical": "prologo_percibe_critico"
  }
}
```

**Éxitos fijos** (MORAL-01, Oriol 08/10: «superar 2 seises en tu tirada de
carisma», a la letra): con `"exitos": N` en lugar de `rivalDice` no hay rival.
Cada 6 es un éxito y cada 5, medio. Con N éxitos o más → SUCCESS (CRITICAL si
todos los dados son 6, con dos dados o más); si falta uno como mucho y algo
salió → PARTIAL; si no, o si es pifia → BOTCH. El juego lo enseña como
«Persuasión · 2 éxitos». Es difícil a propósito: con 3 dados, 2 éxitos salen un
8,8 % de las veces y 4 éxitos, nunca (hacen falta 4 dados o más).

```jsonc
{ "type": "skillCheck",
  "skillCheck": { "skillId": "persuasion", "exitos": 2,
                  "flagBotch": "bnd1_aigren_a_pifia", "flagPartial": "bnd1_aigren_a_parcial",
                  "flagSuccess": "bnd1_aigren_a_exito", "flagCritical": "bnd1_aigren_a_critico" } }
```

Las skills utilitarias (`percepcion`, `sigilo`, `persuasion`,
`conocimiento_arcano`, `intimidacion`, `investigacion`, `engano`,
`atletismo`, `interpretacion`) viven en `assets/catalogs/skills.json`
generadas por `tools/gen_skills_utilitarias.py`. Para que un `skillCheck`
funcione, la `GameSession` debe recibir el catálogo Nd6 vía
`setNd6SkillCatalog(...)`; sin él, el check se ignora con un aviso en el
log (la aventura sigue, solo sin tirada).

Un tipo de trigger o de efecto desconocido es **error de carga**, no un
beat que nunca se dispara en silencio. Un objetivo sin `doneFlag`
también. Y la carga es todo-o-nada: si el beat 7 está mal, no se carga
ninguno.

---

---

### La moral: el libro del día (MORAL-01)

Cada elección con peso apunta un **acto** (el efecto `acto`). Lo apuntado se
**asienta al dormir** (el primer `descanso` `"largo"` que llegue; en el Ocaso,
la barca de Sho-Noco) con los seis números de `assets/rules/moral.json`, que
Oriol dio por buenos el 08/10/2026:

| Número | Fichero | Qué hace |
| ------ | ------- | -------- |
| Peso | `peso: {menor: 2, medio: 5, mayor: 12}` | Lo que vale cada gravedad. |
| Repetición | `repeticion: [1, 0.5, 0.25, 0]` | El 2.º acto del mismo `tipo` el mismo día vale la mitad; el 3.º, un cuarto; del 4.º en adelante, nada. |
| Tope | `topePorNoche: 10` | Los menores y los medios de un día no mueven más de ±10. Los mayores no tienen tope: van después, uno a uno. |
| Resistencia | `resistencia: 100` | Lo que te aleja del 0 vale × (1 − \|moral\| / 100); lo que te acerca, entero. |
| Cuándo | — | Al dormir; mientras tanto, el registro dice «Esta noche te pesará/honrará». |
| Marca | — | Un acto mayor que pesa, contra un inocente, deja una marca que no se va aunque la moral suba («Eso no se va: …»). |

Los **tramos**: Villano (−75 o menos), Desgraciado (−60), Canalla (−20),
Neutral, Respetado (+20), Salvador (+60) y Héroe (+75 o más). Cambian el aura
del héroe a la mañana siguiente y lo que pide `requires.moral`. La **pureza**
(de 100 hacia abajo) va aparte: la toca lo profano (bajar sin armas a la cripta,
ver la ceremonia, unirse a los Perdidos). La **reputación** va por facción
(`aegroum`, `perdidos`, `barriada_surysal`: solo las humanas y conocidas) y se
mueve en el momento.

`assets/rules/actos.json` es el catálogo (`actos: [...]`):

| Campo | Qué es |
| ----- | ------ |
| `id` | Lo que se pone en `{"type": "acto", "arg": "…"}`. |
| `tipo` | Para la repetición: `amenazar_inocentes`, `proteger_inocentes`, `ayudar`… |
| `gravedad` | `menor`, `medio` o `mayor`. |
| `signo` | `1` te honra, `-1` te pesa, `0` no toca la moral (solo pureza o reputación). |
| `contra` | `inocente`, `culpable`, `combatiente` o `rendido`: con `inocente`, un mayor que pesa deja marca. |
| `pureza` | Lo que resta a la pureza (negativo), si algo. |
| `reputacion` | `{faccion: delta}`: menor ±5, medio ±15, mayor ±40. |
| `motivo` | La frase que lee el jugador, en pasado: «Presionaste a Aigren con su hijo». |

`tools/simular_moral.py` juega las cinco rutas de `tools/moral/rutas_boundington.json`
con estos números (y con los que se le pasen) y `test_moral` falla si una ruta de
Boundington llega a ±60.

## Comandos de evento (E-04)

**Ojo: esto NO son los `efectos` de los beats de arriba.** Son dos cosas
distintas que conviven a propósito. Los *efectos* son el sistema antiguo,
el que ejecutan las aventuras ya escritas, y no cambian. Los *comandos*
son los del editor de eventos nuevo (E-01/E-05), los que ejecuta el
intérprete de E-03 y los que el autor monta sin escribir JSON.

El catálogo vive en `include/RPG/EventCommand.h` y **es la única fuente**:
de ahí salen los formularios del editor (E-05/E-06), la validación
estática (E-07) y la ejecución (E-03). Si cada uno llevara su lista, el
editor acabaría ofreciendo comandos que el intérprete no sabe ejecutar.

`implemented` distingue **declarado** de **cableado**. Un comando
declarado y no cableado aparece en el editor deshabilitado y, si se
ejecuta, falla diciendo qué le falta. Ofrecer al autor un comando que no
hace nada es peor que no ofrecerlo.

| `type` | Parámetros | Ramas | ¿Cableado? |
| --- | --- | ---: | --- |
| `texto.mostrar` | `texto`, `hablante` (opcional) | 0 | sí |
| `bandera.encender` | `bandera` | 0 | sí |
| `bandera.apagar` | `bandera` | 0 | sí |
| `variable.fijar` | `variable`, `valor` | 0 | sí |
| `variable.sumar` | `variable`, `valor` | 0 | sí |
| `interruptor.fijar` | `evento`, `letra` (A-D), `valor` | 0 | sí |
| `control.esperar` | `ms` | 0 | sí |
| `control.si` | `clase`, `referencia`, `comparacion`, `valor`, `negada` | 2 | sí |
| `inventario.dar` | `objeto`, `cantidad` | 0 | sí |
| `inventario.quitar` | `objeto`, `cantidad` | 0 | sí |
| `economia.oro` | `valor` | 0 | sí |
| `mapa.ir` | `mapa`, `entrada` | 0 | sí |
| `combate.iniciar` | `encuentro`, `flag_victoria`, `flag_derrota`, `flag_huida`, `ineludible` | 0 | sí |
| `control.segun_desenlace` | — | 3 | sí |
| `comercio.abrir` | `tienda` (id del PNJ en el catálogo de objetos) | 0 | sí |

Las familias de partida las resuelve `RPG/SessionCommandHost`, que entra
por el `fallback` de `NarrativeCommandHost`. `comercio.abrir` abre la tienda
del PNJ **sin** disparar su narrativa y espera a que el jugador la cierre.
Si la tienda no se puede abrir (id desconocido, no es un PNJ, no vende nada
con precio, o la partida no está explorando) el comando falla con el
motivo: no se espera a una tienda que nunca se abrió.

**Lanzar el combate y ramificar su desenlace son DOS comandos**, como pide
el diseño de bloques (§6, B09 y B14). `combate.iniciar` lanza y espera;
`control.segun_desenlace` ramifica en victoria, huida y derrota, en ese
orden. Separarlos permite meter cosas entre medias y que un combate sin
ramas siga siendo legal. Ramificar sin un combate delante es un error:
silenciarlo daría una rama al azar, y el autor vería la escena de victoria
después de un combate que no ha habido.

Un combate que no se puede iniciar —enemigo que no está en el catálogo, o
sin mundo donde ponerlo— **falla** en vez de quedarse esperando: si
esperara, el evento se colgaría en un combate que nunca empezó y el
jugador se quedaría delante de un PNJ mudo.

**Un peaje mayor que el saldo falla.** `GameSession::addGold()` nunca deja
el oro negativo y recorta en silencio; para un peaje de evento eso es justo
lo que no queremos, porque la puerta se abriría sin haberla pagado.

**Quitar objetos es todo o nada.** Quitar tres de una llave cuando solo hay
dos dejaría al jugador sin las dos y sin la puerta abierta.

**`mapa.ir` anota la transición, no la ejecuta.** `GameSession` no carga
niveles a propósito —no hace I/O ni conoce rutas de assets— y expone la
transición pendiente para que quien orquesta la ejecute. El anfitrión sigue
la misma división.

**`mapa.ir` es terminal (DEC-30).** Lo que se ponga debajo no se ejecuta,
y el validador lo marcará como inalcanzable. La continuación, si hace
falta, se expresa como evento de entrada del mapa de destino.

**Las condiciones de inventario dan error, no falso.** «No tiene la
llave» y «no sé mirar el inventario desde aquí» son cosas distintas, y
confundirlas convierte una pieza que falta en una puerta que nunca se
abre y que nadie sabe por qué.

## 3. Los tres patrones que hacen falta saber

**Pista + repetición.** Dos beats para el mismo NPC, el condicionado
primero. El de arriba se lleva el primer encuentro; el de abajo es el
"ya te lo he contado".

```jsonc
{ "id": "a", "trigger": {"type":"talk","target":"guardia"}, "requires": {"notFlags":["pista"]}, "effects":[{"type":"setFlag","arg":"pista"}] },
{ "id": "b", "trigger": {"type":"talk","target":"guardia"}, "requires": {"allFlags":["pista"]} }
```

**Cierre que se apaga solo.** Un beat `auto` que exige sus condiciones y
además `notFlags` de su propia flag de resultado.

```jsonc
{ "id": "cierre", "trigger": {"type":"auto"},
  "requires": { "allFlags":["p1","p2","p3"], "notFlags":["resuelta"] },
  "effects": [{"type":"setFlag","arg":"resuelta"}, {"type":"grantGold","value":30}] }
```

Se prefiere esto a `"once": true` porque el estado queda **explícito en
las flags** (y por tanto en el guardado, cuando lo haya) en vez de
escondido en la contabilidad interna del motor.

**Puerta.** Un beat que solo existe si otro pasó antes: `allFlags` con la
flag del anterior. Con eso se construye una cadena lineal cuando se
quiera una — la cadena es una *consecuencia* de las condiciones, no una
estructura aparte.

---

## 4. Enchufarlo a una partida

```cpp
auto res = RPG::AdventureScript::loadFromFile("assets/adventures/mi_aventura.json");
RPG::AdventureScript guion = res.value();

RPG::NarrativeEngine engine;
engine.setAdventure(&guion);

RPG::NarrativeState state;          // las flags de ESTA partida
session.setNarrative(&engine, &state);
session.enterLevelNarrative("assets/levels/ciudad_centro.json");
```

Sin `setNarrative`, la sesión se comporta exactamente igual que antes de
existir este sistema. Con él:

- **La narrativa manda sobre el diálogo fijo del catálogo.** Si hay un
  beat activo para ese NPC, se dicen sus líneas; si no, las de siempre.
  Por eso un NPC de relleno pasa a formar parte de una aventura
  escribiendo un beat en un JSON — sin tocar `ciudad_objetos.json` ni el
  nivel.
- **Al cerrar un diálogo se hace un `tick()`.** Un solo beat por cierre,
  nunca un bucle: un contenido mal escrito no puede colgar el juego.

---

## 5. Cómo crece (y qué NO hay que hacer)

Todo lo que falta del sistema narrativo de `GAMEMACHINE_NECESIDADES.md`
entra **por estos tres sitios**, sin sistemas paralelos:

| Lo que falta | Dónde entra |
| ------------ | ----------- |
| Opciones de diálogo ramificado (P0-12) | `options[]` en el beat + `NarrativeResult` devolviendo las opciones disponibles |
| Abrir tienda, empezar combate, dar quest, cambiar de nivel, hito de tier | un valor más en `EffectType` + un campo más en `NarrativeResult` |
| Triggers de recoger objeto / derrotar enemigo / paso del tiempo | un valor más en `TriggerType` + una llamada a `fire()` desde donde ocurra |
| Requisitos de tier, oro, facción, objeto en inventario | campos nuevos en `NarrativeCondition` |
| Varias aventuras a la vez | `std::vector<const AdventureScript*>`; las flags ya son globales |
| Guardado | serializar `NarrativeState::flags()`. Es una lista de strings: eso es todo |
| Las 300 cartas de `assets/catalogs/adventures.json` | un `AdventureScript` con el id de su `AdventureDefinition` (la ficha) |

Lo que **no** hay que hacer: meter estado en los beats, dar a los
objetivos lógica propia, o hacer que el contenido consulte ids de beats.
Las flags son la única fuente de verdad; en cuanto haya dos, el sistema
se empieza a desincronizar solo.

---

## 6. Nota sobre acentos

Desde TILDES-01 y TILDES-02 (08/10/2026) lo que se lee en pantalla
(`lines`, `speaker`, `text`, el registro, los nombres) va **con sus tildes,
eñes y ¿ ¡**: la hoja de pergamino las pinta. Lo que no se lee sigue sin
ellas: los ids, las flags y las rutas (`bnd_tut_cerrado`, no
`bnd_tut_cerrádo`), porque son claves y se comparan letra a letra.


## Efecto `startBattle`

Pide un combate contra un enemigo del `ObjectCatalog`. Como `skillCheck`, el
motor narrativo no lo resuelve: lo empaqueta en `NarrativeResult.battles` y lo
ejecuta `GameSession`.

```jsonc
{
  "type": "startBattle",
  "startBattle": {
    "monsterId":   "perdido_saqueador",   // ObjectDefinition de categoria enemy
    "flagVictory": "ocaso_saqueadores_muertos",
    "flagDefeat":  "ocaso_saqueadores_ganan"
  }
}
```

**Las dos flags son obligatorias.** Un combate del que no se puede saber cómo
acabó es contenido que no ramifica, y entonces no hacía falta pedirlo.

### Huir y caer, según la misión (08/10/2026)

Oriol (08/10): «Depende, si la misión es una misión donde huir o fallar es
una opción, bien, pero de entrada no tienes ninguna obligación de terminarla
[...] a no ser que sean momentos de historia ineludibles». Por eso:

| Desenlace | Combate de la historia (por defecto) | `"ineludible": true` | Combate del mapa (sin aventura) |
|---|---|---|---|
| Ganar | `flagVictory` | `flagVictory` | — |
| Huir | `flagHuida` (o `flagDefeat` si no hay) | **no se puede**: la carta Huir sale bloqueada | el enemigo sigue ahí |
| Caer | el héroe se levanta con **un cuarto de la vida**, los que ganaron se van y se enciende `flagDefeat` | **fin de la partida** | fin de la partida |

```jsonc
"startBattle": {
  "monsterId": "perdido_saqueador",
  "flagVictory": "bnd_tut_saqueador_vencido",
  "flagDefeat":  "bnd_tut_saqueador_gana",
  "flagHuida":   "bnd_tut_saqueador_huido"   // opcional
}
```

- **`flagHuida`** (opcional): para contar distinto huir que caer («Echas a
  correr...» no es «Te levantas del polvo...»). Sin ella, huir enciende
  `flagDefeat`, como siempre.
- **`ineludible`** (opcional, `false` por defecto): un momento que no se puede
  esquivar. Ni se huye ni se sobrevive a caer. Un combate ineludible no lleva
  `flagHuida` (el cargador lo rechaza).
- **Caer** en un combate que no es ineludible no acaba nada: la aventura
  cuenta lo que pasó con un beat que lea `flagDefeat` (el atraco del
  tutorial, Verina que no se salva, el final «caído» del Ocaso). Si el beat
  pedía varias oleadas, las que faltaban se dan por perdidas (se enciende la
  `flagDefeat` de cada una).
- En los eventos de mapa, `combate.iniciar` lleva lo mismo: `flag_huida` e
  `ineludible`; y «Según desenlace» sigue por la rama de derrota cuando el
  héroe se levanta.

### Huir a 0 PV y pelear sin matar (FASE1-BND-01)

```jsonc
"startBattle": {
  "monsterId": "sandes",
  "enemigos": [{"id": "sandes", "cantidad": 1}, {"id": "perdido_fanatico", "cantidad": 2}],
  "huyen": ["sandes"],          // a 0 PV no muere: huye («Sandes huye malherido.»)
  "flagVictory": "bnd3_sandes_huido", "flagDefeat": "bnd3_sandes_gana"
}
```

- **`huyen`** (opcional): los ids del combate que a 0 PV se escapan en vez de
  morir. Cuenta como victoria (`flagVictory`) y el registro dice «X huye
  malherido.» por cada uno. Un id que no está en el combate es error de carga.
  Es Sandes («escapará al quedarse a 0 de vida») y Venides («siempre huye en
  cuando su vida llega a 0»).
- **`sinMatar`** (opcional, `false` por defecto): una pelea a puñetazos. Los
  enemigos quedan fuera de combate, nadie muere, y el registro lo dice («Quedan
  fuera de combate, pero nadie ha muerto.»). Es la pelea de taberna («si
  apalean, pero no matan al que la inicia»).
- Los dos solo se escriben si están: un combate de antes se escribe igual que antes.

### En grupo y con acompañantes (05-06/10/2026)

```jsonc
"startBattle": {
  "monsterId": "perdido_devoto",                       // el primero del grupo
  "enemigos": [{"id": "perdido_devoto", "cantidad": 3}], // o "cantidad": 3 junto a monsterId
  "acompanantes": [{"id": "venides"},
                   {"id": "xila", "siFlag": "bnd_pista_xila"}],
  "flagVictory": "bnd3_saqueadores_vencidos",
  "flagDefeat":  "bnd3_saqueadores_ganan"
}
```

- **`enemigos`** (opcional): todos los que salen, hasta 8. Si un nombre se
  repite, en el combate van numerados («Devoto de los Perdidos 2»). Al ganar
  caen todos; al huir, los vivos se quedan en el mundo.
- **`acompanantes`** (opcional, hasta 3): quien pelea al lado del héroe. Cada
  id tiene que estar en la lista `acompanantes` de la aventura (abajo); con
  `siFlag`, solo viene si esa flag está encendida. Oriol (05/10): «solo en
  Boundington, y solo en algunos momentos de la historia». El compañero que
  se creará con el personaje es otra cosa.

### La lista `acompanantes` de la aventura

Va en la raíz, junto a `beats` y `timers`. Solo se escribe si hay alguno.

```jsonc
"acompanantes": [
  {"id": "venides", "nombre": "Venides", "vida": 16,
   "stats": {"CON": 3, "DES": 2, "INT": 0, "CAR": 1},
   "ataques": 2, "atacable": true, "alCaer": "se_retira"},
  {"id": "xila", "nombre": "Xila", "vida": 6,
   "stats": {"CON": 1, "DES": 2, "INT": 0, "CAR": 2},
   "ataques": 0, "atacable": false, "alCaer": "cae",
   "cura": {"dado": 4, "mas": 1, "veces": 2, "umbral": 0.34}}
]
```

| Campo | Qué es | Por defecto |
|---|---|---|
| `id` | el del PNJ: da su retrato en el combate | obligatorio, sin repetir |
| `nombre` | como sale en el combate y en el registro | el `id` |
| `vida` | PV (1 o más) | obligatorio |
| `stats` | dados, de 0 a 8: ataca con 1 + DES y se defiende con el mayor de CON y DES | todo 0 |
| `ataques` | golpes por turno (0 a 4); 0 = no ataca | 1 |
| `atacable` | `false`: los enemigos no van a por él | `true` |
| `alCaer` | `"cae"` o `"se_retira"` (a 0 PV se va del combate) | `"cae"` |
| `cura` | cura 1d`dado` + `mas` a quien tenga la vida en `umbral` o menos, `veces` en toda la aventura | sin cura |

**Cómo pelean.** Actúan solos, en su orden, cuando el héroe cierra su turno
(después de sus invocaciones y antes que los enemigos). Un sanador cura al más
apurado si le quedan curas; si no, cada uno golpea al último enemigo en pie.
Los enemigos van a por quien les ha hecho daño el último (el héroe o un
acompañante); si nadie, a por el héroe. Si cae el héroe, la batalla se pierde
y los demás se retiran. Sin la mesa de cartas del héroe (un héroe sin ficha)
no vienen.

### Los que llegan a mitad: `refuerzos` (ROSITA-01, 14/10/2026)

```jsonc
"startBattle": {
  "monsterId": "sandes", "acompanantes": [{"id": "venides"}, {"id": "xila"}],
  "flagVictory": "bnd3_sandes_vencido", "flagDefeat": "bnd3_sandes_gana",
  "refuerzos": [{
    "requires": {"allFlags": ["bnd2_rosita_semilla"], "variables": {"confianza_rosita": {"min": 2}}},
    "cuando": {"vidaEnemigo": 0.25, "golpeMortal": true},
    "flags": ["bnd3_compania_vista", "bnd3_compania"],
    "lineas": ["Un trueno sin nube. [...]", "El guardabosques: «Tú pega. Nosotros te cubrimos.»"],
    "acompanantes": [{"id": "mago_de_las_tinieblas"}, {"id": "guardabosques"}, {"id": "guerrero_demoniaco"}]
  }]
}
```

Oriol: la compañía de Rosita llega «cuando Sandes esté a 1/4 de vida [...] y a
punto de morir». Cada refuerzo llega **una vez**, dentro del mismo combate, la
primera vez que se cumplen las dos cosas:

- su `requires` (igual que el de un beat: flags, variables, moral...);
- `cuando`: el enemigo principal (`monsterId`, el primero del grupo) ha bajado
  a `vidaEnemigo` de su vida, **o** el héroe a `vidaHeroe` (lo primero que
  pase). Valores en (0, 1]; hace falta al menos uno.

Al llegar se encienden sus `flags`, sus `lineas` salen en el registro del
combate («Llegan a tu lado: ...» al final) y sus `acompanantes` (como los del
combate, con `siFlag`; hasta 3) se ponen al final de tu bando con las curas que
les queden. Los refuerzos se miran en orden, así que uno puede pedir las flags
del anterior (el guerrero demoníaco le habla al héroe que también lo es).

**El golpe mortal (AV13-34; Oriol, 14/10, D0015 §13).** Con `"golpeMortal": true` (y `vidaEnemigo`),
el refuerzo **no mira las vidas**: llega cuando el enemigo principal, estando a `vidaEnemigo` de su vida
o menos, te da un golpe mortal. Está guionizado para que pase siempre: en cuanto el enemigo principal
está a esa vida o menos, su siguiente acción te deja **a las puertas (1 PV)**, tanto si el golpe se
quedaba corto como si ya te mataba, y el registro lo dice («Sandes te da un golpe mortal: caes de
rodillas, a las puertas.»). Es lo que lleva la compañía de Rosita (`BattleState::ponGolpeMortal`).

Sin `golpeMortal`, si el héroe cae de un golpe (de más de `vidaHeroe` a 0), el refuerzo no llega:
para eso, un beat que salte con `flagDefeat` (en Boundington, `b3_compania`:
llegan al caer y se repite el combate con ellos). Un refuerzo trae
acompañantes o enciende flags; si no, es un error con nombre. Lo hace
`Game::llegaRefuerzos` (`include/Game/Refuerzos.h`).

### La diferencia con `skillCheck`: el tiempo

Una tirada Nd6 se resuelve dentro del mismo `applyNarrative` y la flag ya está
puesta al volver. **Un combate dura turnos**: el beat termina, el jugador pelea,
y solo después se sabe el resultado. Por eso las flags de victoria/derrota no
las enciende `applyNarrative` sino `syncBattleOutcome`, cuando el `BattleState`
se cierra.

El contenido no nota la diferencia — pide una flag y esa flag acaba encendida —
pero **quien escriba la aventura sí debe tenerlo en cuenta**: el beat `auto` que
reaccione a `flagVictory` no puede dispararse en el mismo `tick()` que lanzó el
combate. Tiene que haber un `tick()` después de cerrarse la batalla.

### Oleadas

Si un beat pide varios combates, o llega uno mientras hay otro vivo, los
siguientes se **encolan**: el motor solo sabe pelear de uno en uno, y perder ese
dato en silencio convertiría una emboscada de tres oleadas en una sola. Al
cerrarse cada combate arranca el siguiente automáticamente.

### Casos límite

- **`monsterId` que no existe en el catálogo**: no revienta la partida (mismo
  criterio permisivo que `LevelLoader`), pero **enciende `flagDefeat`**. Si no lo
  hiciera, la aventura se quedaría esperando para siempre una flag que nadie va
  a poner, y el jugador no podría avanzar.
- **Huir** sin `flagHuida` cuenta como derrota a efectos de trama: el enemigo
  sigue vivo y lo que fuera a pasar, pasa.


## Los ficheros de ayer se siguen abriendo hoy (G-07, mecanismo)

Este formato se escribe a disco y se vuelve a leer más tarde. Nada en el
compilador impide que un cambio en el serializador deje ilegible lo que ya
hay escrito: no da error, no rompe ningún test, y el proyecto de alguien
simplemente deja de abrirse.

Contra eso hay ficheros **congelados** en `tests/fixtures/esquemas/`, que
`test_schema_fixtures` abre en cada ejecución y comprueba campo a campo —
no solo que parsee. También comprueba que volver a escribir lo leído da
exactamente el mismo texto: si el lector y el escritor dejaran de estar de
acuerdo, la diferencia acabaría siendo un campo perdido.

**Reglas al cambiar el formato:**

1. Si el cambio es **aditivo** (una clave nueva con valor por defecto), no
   se sube `schemaVersion` y la fixture antigua tiene que seguir
   abriéndose tal cual. Así entraron las capas de evento (E-01) y las
   variables narrativas (E-02).
2. Si el cambio **no es compatible**, se sube `schemaVersion`, se añade
   `…-vN.json` a las fixtures, **se deja la anterior** y se escribe aquí
   la ruta de migración. `test_schema_fixtures` falla hasta que se haga:
   ese es el punto.
3. Si una fixture vieja deja de abrirse, lo que se arregla es el lector o
   se escribe la migración. **Nunca la fixture.** Regenerarla para que
   pase convierte la prueba en un adorno.

Regenerar las fixtures es un acto deliberado y a mano, solo al añadir una
versión:

```bash
./test_schema_fixtures --escribir tests/fixtures/esquemas
```

Esto **no** es todavía la congelación de G-07: congelar los esquemas a la
1.0-B pide que los formatos estén terminados, y E-16 aún va a tocar el de
proyecto. Es el mecanismo que esa congelación necesitará.

## 7. Serialización (G-07)

### 7.1. serializeAdventure()

G-07 proporciona `RPG::serializeAdventure(const AdventureScript&)` en
`src/RPG/AdventureSerializer.cpp` (declaración en `include/RPG/AdventureSerializer.h`
y también en `include/RPG/NarrativeEngine.h`). Es el inverso de
`AdventureScript::loadFromString`: produce el JSON de aventura descrito en
esta sección.

**Contrato de idempotencia.** `AdventureScript::loadFromString(serializeAdventure(adv))`
reproduce `adv` exactamente, incluido el orden de campos. La fixture congelada
`tests/fixtures/esquemas/aventura-v1.json` codifica este contrato: el test
de idempotencia en `SchemaFixtureTests.cpp` falla si reescribirla cambia un
solo byte.

#### Mapeo de enums

| Tipo            | Valor          | Texto JSON       |
|-----------------|---------------|------------------|
| TriggerType     | Talk          | `"talk"`         |
| TriggerType     | Enter         | `"enter"`        |
| TriggerType     | Auto          | `"auto"`         |
| EffectType      | SetFlag       | `"setFlag"`      |
| EffectType      | ClearFlag     | `"clearFlag"`    |
| EffectType      | GrantGold     | `"grantGold"`    |
| EffectType      | Log           | `"log"`          |
| EffectType      | StartBattle   | `"startBattle"`  |
| EffectType      | SkillCheck    | `"skillCheck"`   |
| EffectType      | StartTimer    | `"startTimer"`   |
| EffectType      | GrantMilestone | `"grantMilestone"` |
| EffectType      | GrantLevel    | `"grantLevel"`   |
| EffectType      | SetVariable   | `"setVariable"`  |
| EffectType      | AddVariable   | `"addVariable"`  |
| EffectType      | PasarFranja   | `"pasarFranja"`  |
| EffectType      | TablaAleatoria | `"tablaAleatoria"` |
| EffectType      | Descanso      | `"descanso"`     |
| EffectType      | DarObjeto     | `"darObjeto"`    |
| EffectType      | Acto          | `"acto"`         |
| EffectType      | Herida        | `"herida"`       |
| Presentacion    | Contador      | `"contador"`     |
| Presentacion    | Barra         | `"barra"`        |
| Presentacion    | Secreto       | `"secreto"`      |

#### Campos condicionales

- `description`: se escribe solo si no está vacío.
- `requires`: se escribe solo si `NarrativeCondition::isEmpty()` es `false`.
  Dentro, `allFlags`, `anyFlags`, `notFlags` se escriben siempre (array,
  posiblemente vacío). `variables`, solo si hay alguna (el valor exacto como número; si no,
  `{min, max}` con los que haya).
- `opciones`: solo si el beat tiene alguna; cada una con `texto`, `requires`
  (si no está vacía), `speaker`, `lines` y `effects`.
- `addVariable`: `min` y `max` solo si los tiene.
- `tablaAleatoria`: el objeto `tabla` con `dado`, `flags` y `que` (este, si no está vacío).
- `skillCheck`: `exitos`, solo si es mayor que 0 (MORAL-01).
- `requires`: `moral`, solo si se pide (`{min, max}` con los que haya).
- `startBattle`: `huyen` y `sinMatar`, solo si los tiene.
- `startBattle`: `refuerzos`, solo si los tiene (ROSITA-01).
- `speaker`, `once`, `lines`, `effects`: siempre se escriben (el último
  incluso si está vacío).
- `skillCheck`: solo cuando el efecto es `SkillCheck`; `startBattle`: solo
  cuando el efecto es `StartBattle`.
- `value`: siempre se escribe (0 para efectos que no lo usan).
- Timer `zona`: solo si no está vacío.
- Timer `arrancaPor`: siempre se escribe (incluso si es `""`).
- Timer `expiraPor`: array de strings (`"tiempo"` y/o `"salir_de_zona"`).
- Timer `presentacion`: siempre se escribe.

#### Nota de cruce con E-02

`NarrativeEngine.h` es de cruce entre E-02 y G-07 desde G-07. La declaración
de `serializeAdventure` se coloca al lado de `loadFromString`/`loadFromFile`
como su contraparte natural. El struct completo (`AdventureScript`, sus
beats, efectos y timers) es propiedad de E-02; G-07 solo añade el método de
salida. Ver `FORMATO_PROYECTOS.md` para el mecanismo general de congelación.

## Cinemáticas (PROLOGO-02)

Una cinemática es una escena que se **ve**, no que se juega: los PNJ andan, la
pantalla se funde, se cambia de nivel... y el jugador no se mueve por su cuenta
mientras dura. Están todas en `assets/cinematicas/cinematicas.json` (la lógica en
`include/Game/Cinematica.h`; el juego, en `Engine/ApplicationCinematica`).

**Lo que se dice no va aquí.** La orden `beat` enciende una flag; la aventura
tiene un beat `auto` que la pide (`requires.allFlags`), y la cinemática espera a
que se cierre ese diálogo (con sus opciones, tiradas y charlas) antes de seguir.
Así las palabras siguen en el JSON de la aventura y las validan las mismas
herramientas.

```json
{
  "id": "barquero",
  "nivel": "assets/levels/prado_del_rio.json",
  "zona": [12, 0, 13, 17],
  "requiere": ["bnd_prologo_percibe_relatado"],
  "sin": [],
  "pasos": [
    {"anda": "jugador", "a": [12, 9]},
    {"flag": "bnd_prologo_barca_viene"},
    {"anda": "barca_de_sho_noco", "a": [14, 9], "velocidad": 1.4, "sinEsperar": true},
    {"anda": "sho_noco", "a": [14, 9], "velocidad": 1.4},
    {"beat": "bnd_prologo_barca_llega"}
  ],
  "alTerminar": ["bnd_prologo_barquero"]
}
```

**Cuándo empieza.** Sola, en exploración, la primera vez que se cumple todo:
- `nivel`: el nivel actual (vacío: cualquiera);
- `requiere` / `sin`: flags;
- `zona`: el jugador está entre esas dos esquinas, incluidas.

Al acabar enciende `cine_<id>_vista` (así no se repite) y las flags de
`alTerminar`. Luego avisa a la historia, que puede abrir un beat o una charla
automáticos.

**Las órdenes** (una por paso):

| Orden | Qué hace |
|---|---|
| `{"anda": quien, "a": [x, y]}` / `"por": [[x, y], ...]` | Anda en línea recta, primero en x y luego en y. `velocidad` en celdas/s (2,5). `"sinEsperar": true`: la orden siguiente no espera a que llegue. `"jugador"` anda con colisiones; los PNJ, no. |
| `{"coloca": quien, "en": [x, y]}` | Lo pone ahí sin andar. |
| `{"mira": quien, "hacia": "arriba"}` | Arriba, abajo, izquierda o derecha. |
| `{"espera": 1.5}` | Segundos. |
| `{"beat": flag}` | La historia habla; se espera a que acabe el diálogo. |
| `{"flag": flag}` | La enciende, sin esperar a nada. |
| `{"nivel": ruta, "en": [x, y]}` | Carga otro nivel con el jugador en esa celda. |
| `{"funde": 1, "segundos": 0.8, "color": "blanco"}` | Funde la pantalla a blanco o negro (1) o la devuelve (0). Al acabar la cinemática, el velo se va solo. |

**Además**, en el mismo fichero:
- `"espectro"`: mientras su `flag` esté encendida, el jugador y los `objetos`
  van con ese `tinte` (translúcidos).
- `"apariciones"`: `{"objeto": id, "si": flag, "sin": flag}`. El objeto se ve
  solo con `si` encendida y `sin` apagada, y se funde al cambiar. Mientras no se
  ve, tampoco se le habla ni choca con nadie. Para condiciones más largas:
  `"visibleSi"` u `"ocultoSi"` con `{"todas": [...], "alguna": [...], "ninguna": [...]}`
  (Edul en el yunque y Aigren que ya se ha ido, el segundo día):

```json
{"objeto": "aigren",
 "ocultoSi": {"todas": ["bnd2_encargo"],
              "alguna": ["bnd2_aigren_despedido", "bnd1_invitado_por_aigren"],
              "ninguna": ["bnd1_aigren_redimido"]}}
```

- `"titulo"`: el nivel que se ve detrás del título (la ciudad, con su gente) y
  el `camino` que hace la cámara, a `velocidad` celdas/s. Solo si la partida
  empieza en `conNivelInicial`.
- `"fondos"`: el color de fondo de un nivel en vez del azul oscuro.

`tools/validar_cinematicas.py` comprueba:
- los niveles;
- que cada actor está en el nivel donde actúa;
- las celdas;
- que cada `beat` lo pide algún beat;
- que lo que se `requiere` lo enciende alguien.

## Dónde se cumple un objetivo: `donde` (MAPA-01, 14/10/2026)

```json
{"id": "obj_tut_sastre", "text": "Preséntate al sastre: la sastrería, al norte de la plaza, junto al mercado.",
 "doneFlag": "bnd_tut_sastre", "donde": "assets/levels/interior_sastreria.json"}
```

`donde` es un nivel o una lista de niveles; también puede ser el `objectId` de algo
del nivel. Es opcional.

El minimapa marca con un **◆ rojo** (leyenda «Objetivo») el sitio del objetivo
seguido, que es el primero sin hacer de la misión activa:

- **Una entrada que lleva allí**, si está en este nivel.
- **La primera puerta del camino más corto**, si está más lejos. Por ejemplo, desde
  la ciudad este, la herrería está pasando por la plaza. El camino sale de las
  puertas de los niveles (`targetLevel`) y lo calcula `Game::RumboNiveles`.
- **En el borde de la ventana**, si el sitio cae fuera de ella, del lado en que queda.

Oriol: «sigo sin saber cómo completar la misión de conocer los tres comerciantes
de la plaza», «no encuentro la herrería».

## Quién habla en cada línea (DIALOGO-02, 14/10/2026)

Una línea escrita como `Quien, acotación: «texto»` se ve así en la hoja del diálogo:

- el rótulo de arriba dice **QUIEN**;
- el texto es `(acotación) texto`, sin las comillas de fuera.

Una línea sin ese prefijo es del `speaker` del beat. Si el beat no tiene `speaker`,
es de la **NARRACIÓN**.

No se parte (se queda como narración):

- un prefijo con un punto, como «Está dormido. El tuerto te mira: «...»»;
- un prefijo que empieza por un sitio («En el límite de Pico Dragón, ...: «...»»).

Oriol: «que ponga arriba narración en vez del nombre del personaje y en el
texto aparezca Sho-Noco: blabla, bastante cutre». Lo hace
`Pergamino::DialogoHoja::vozDe`.

## Cinemáticas: `desfase` (PROLOGO-02, 14/10/2026)

`{"anda": "sho_noco", "a": [14, 9], "desfase": [0, -0.35]}` corre el dibujo dentro
de su celda. El valor son celdas, de −1 a 1, y dura hasta que la cinemática acaba
o hasta otra orden con `desfase`. Sirve para poner a dos en la misma barca:
Sho-Noco en la popa y tú en la proa.

Con `"coloca": "jugador"`, el jugador no cambia de celda (eso es andar, con sus
reglas). Solo se corre su dibujo, y vuelve a su sitio al cargar otro nivel o al
acabar la cinemática.

## Andar libre después de una cinemática (FANTASMA-01, 14/10/2026)

Una cinemática no tiene por qué dejar abierto un diálogo al acabar. En los
recuerdos del prólogo, `alTerminar` enciende una flag `*_libre` (por ejemplo
`bnd_pr_r1_libre`). Con ella encendida pasan tres cosas:

- **Una narración, una vez.** Es un beat `auto` que dice «Nadie te ve. [...]
  Cuando quieras seguir, habla con Sho-Noco.».
- **Andas por el nivel.** Cada uno de la escena tiene un beat `talk` con lo que
  se ve al acercarse (no te ven, así que no hay charla).
- **Para seguir, se habla con alguien.** La charla con Sho-Noco es `talk` (no
  `auto`), y su salida enciende lo que pide la cinemática siguiente. El tema
  «Mirar un poco más» cierra la charla sin seguir y no se gasta.

El objetivo lleva `"donde": "sho_noco"`, así que el ◆ del minimapa marca a
quién hay que hablar.

Oriol: «no puedes jugar como fantasma».

## Rumores (D-MIS-03, AV14-23, 14/10/2026)

Oriol (D0015 §15): **el rumor es un beat que salta al hablar.** No hay un tipo de contenido nuevo: un
rumor es un beat `talk` con sus condiciones, escrito con cuatro costumbres para que todos se lean igual.

1. **Quién lo sabe:** `trigger: {"type": "talk", "target": "<objectId del PNJ>"}`.
2. **Cuándo lo cuenta:** en `requires`, lo que lo hace creíble: la franja (`variables.franja`), el sitio
   (`lugar`), lo que ya ha pasado (`allFlags`), tu moral (`moral`). Y **siempre**
   `notFlags: ["rumor_<tema>"]`: un rumor se cuenta una vez.
3. **Qué deja:** la flag `rumor_<tema>` (con el prefijo `rumor_`, para que otro beat, el diario o el
   códice puedan saber qué has oído). Si cuenta para «Preguntar por la ciudad», también `bnd_rumor`.
4. **Dónde va en el fichero:** **detrás** de la charla del PNJ. El motor dispara el primer beat que
   encaja (§1), así que un rumor delante se comería la conversación. Detrás, sale cuando la charla de
   ese momento no tiene nada que decir.

Las líneas, en boca de quien lo cuenta (`speaker`) y con su manera de hablar. Un rumor no es una pista
segura: puede ser verdad a medias, y eso se puede ramificar después con su flag.

```json
{
  "id": "rumor_trolls_rosita",
  "trigger": {"type": "talk", "target": "skilla"},
  "requires": {"allFlags": ["bnd0_noche"], "notFlags": ["rumor_trolls"]},
  "speaker": "Rosita",
  "lines": [
    "¿El borracho del rincón? Cuenta que la Furia mató a doce trolls con una espada que ardía.",
    "Doce. Y cada noche son más. A ver si un día llega a cien y se calla."
  ],
  "effects": [
    {"type": "setFlag", "arg": "rumor_trolls"},
    {"type": "setFlag", "arg": "bnd_rumor"}
  ]
}
```

La prueba `test_comercio_reputacion` (caso `un_rumor_es_un_beat_que_salta_al_hablar`) defiende las
cuatro costumbres: sale al hablar, una sola vez, y detrás de la charla no se la come.

## Recompensas de escena (AV18-08, 15/10/2026)

Un beat u opción paga con `grantGold` (valor positivo) o `darObjeto`. La `rewardGold` / `rewardItemIds` de `quests.json` **no** la aplica el motor: se dejan a 0 y se paga siempre desde los efectos.

Reglas que comprueba `tools/auditar_recompensas_escenas.py` (lo llama `validar_todo.py`):

1. **Un solo cobro.** El beat u opción pone una bandera que sus propios `requires.notFlags` exigen ausente (en los premios `rec_*` del beat nuevo, la bandera `rec_…`).
2. **Dilo en la escena.** El oro lleva un `log` «Recibes N de oro (de dónde).» o la cifra en las líneas.
3. **Alternativas excluyentes** comparten `grupo` en el libro; en el presupuesto cuenta solo la más alta.
4. **Todo premio positivo está en `tools/recompensas_escenas.json`** (día, canal, grupo, aventura, beat, opción). Un objeto de misión (p. ej. `furia_de_alaro`) se lista en `objetos_de_mision` y no cuenta como premio.
5. El día se mide con el canal `misiones` de `ECONOMIA.md` (±15 %). Los combates con premio narrativo van con `canal: "combates"` y se informan aparte.

Cuadro, totales y avisos: `planificacion_alcance_y_vision/RECOMPENSAS_ESCENAS.md`.
