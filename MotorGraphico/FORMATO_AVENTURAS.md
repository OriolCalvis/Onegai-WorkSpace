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

### Efectos

| `type`      | Campos | Efecto |
| ----------- | ------ | ------ |
| `setFlag`   | `arg`  | Enciende una flag |
| `clearFlag` | `arg`  | Apaga una flag |
| `grantGold` | `value` | Oro a la partida (negativo = peaje) |
| `log`       | `arg`  | Línea suelta al log de la partida |
| `skillCheck` | `skillCheck: {skillId, cd, flagBotch, flagPartial, flagSuccess, flagCritical}` | Resuelve una tirada Nd6 (GDD §7.1): lanza tantos d6 como el `castingStat` de la skill indique (sacado del catálogo Nd6), compara contra `cd` (escala 0.0..3.0: 0.5 Fácil, 1 Normal, 1.5 Difícil, 2 Muy difícil, 2.5+ Extraordinaria) y enciende **una** de las cuatro flags según el grado obtenido (BOTCH/PARTIAL/SUCCESS/CRITICAL). Así un beat puede ramificar en cuatro variantes por tirada. |

El `skillCheck` es **diferido** como `grantGold`: el motor narrativo no
tiene RNG ni catálogo, así que solo empaqueta la petición y deja que
`GameSession` la resuelva con `DicePoolEngine` (el mismo del combate, lo
que keeps narrativa y combate en el mismo sistema de reglas). El grado se
traduce a flags — no a magnitud — porque en la narrativa lo que importa es
"qué percibes / convences / averiguas", no cuánto daño.

```jsonc
{
  "type": "skillCheck",
  "skillCheck": {
    "skillId": "percepcion",
    "cd": 1.0,                                  // Difícil sería 1.5
    "flagBotch":    "prologo_percibe_nada",
    "flagPartial":  "prologo_percibe_parcial",
    "flagSuccess":  "prologo_percibe_exito",
    "flagCritical": "prologo_percibe_critico"
  }
}
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

Los campos que se dibujan en pantalla (`lines`, `speaker`, `text`) van
**sin tildes ni ñ**, igual que `assets/objects/ciudad_objetos.json`: la
`BitmapFont` actual es ASCII. Los campos de autor (`description`,
comentarios) pueden llevarlos. Cuando la fuente crezca, esto se cae solo.
