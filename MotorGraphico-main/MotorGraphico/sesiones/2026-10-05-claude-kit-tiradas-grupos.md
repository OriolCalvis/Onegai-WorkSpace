# El kit a 2, 2, 1, 1 con topes por tier, el nombre de los relojes, todas las tiradas y los combates en grupo · 05/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-05-claude-dado-estados.md` (H2-01, H2-02, H2-03 y
H3-06). Oriol, al ver los dados:

> «La gracia que la primera equipación la balanceamos a 2 dados, lo más alto, 1 lo más bajo [...]
> hay que respetar los caps de dados máximos que cada tier puede ofrecer a la espec más alta. Y
> nunca jamás un jugador puede tener las tres a lo más alto [...]. Pero que sea extremadamente
> difícil de conseguir.»
> «¿Añado un campo "nombre" opcional a los temporizadores?» «Sí.»
> «Cuando una acción tira varias veces, solo se ve la última tirada: dale más pausa al juego y que
> se vean todas.»
> «Los combates del juego siguen siendo de uno contra uno [...]. Mírate la historia de
> Boundington.»

Y a la pregunta del kit: «Principal y secundaria +2» (las otras dos, +1; la raza suma encima).

La revisión de la campaña para jugarla entera va aparte, en un documento vivo de Claude:
«Los Perdidos de Boundington — revisión para jugarla entera». La entrega, con capturas del juego
real, está en `Claude outputs/kit_tiradas_grupos_2026-10-05/LEEME.md`.

## 1. El kit del cofre: +2, +2, +1, +1

- `assets/rules/kit_basico.json` pide ahora `"bonos": {"principal": 2, "secundario": 2,
  "resto": 1}`, `"exacto": true` y hasta 5 piezas (el arma y una por ranura de armadura). El
  formato de antes (`atributos` + `bonoMinimo`) se sigue leyendo.
- `Game/Personajes/KitBasico`: el arma de la clase (o la más parecida con la que el kit salga
  exacto) y, de todas las combinaciones de cabeza, torso, piernas y pies que suman exactamente lo
  que pide la regla, la que lleva más piezas del equipo inicial de la clase; empate: menos piezas,
  las más comunes, las que se ven y el id. `Kit::objetivo`, `Kit::sobran` y `objetivoDe()` son
  nuevos.
- **Las 63 clases salen exactas.** El bardo de las cuatro estaciones lleva su sable y su armadura
  de cuero, más capucha, faldón y zuecos: CON 1, DES 2, INT 1, CAR 2.
- El papiro del cofre, con más de tres piezas, va compacto: una línea por pieza y la ranura donde
  iría el bono si no lo tiene.

## 2. Los topes de dados por tier

- `include/RPG/TopesDados.h` (nuevo, solo cabecera): con el tope T del tier, el atributo con más
  bono del **equipo** llega como mucho a T, el segundo a 3/4 de T más uno y los otros dos a la
  mitad. Con el tope 4 de Tier 1, el mejor equipo da 4, 4, 2 y 2: dos arriba, tres nunca. El kit
  es el reparto del tope 2.
- `TierRule::topeDadosEquipo` en `tier_rules.json` y en las reglas por defecto: **2, 4, 6, 8, 8 y
  8 para los tiers 0 a 5. Es una propuesta** (el GDD no tiene topes por tier).
- `CharacterSheet::stat()` cuenta el equipo con el tope desde `recompute_derived()` (que ya llaman
  la sesión y `InventoryEngine::equip`), y `tierUp()` lo sube con el tier. La raza suma encima.
  Sin `recompute_derived()` no hay tope, como antes.
- **Ojo:** el catálogo de Tier 1 todavía permite juntar INT 8 o CAR 8 en un solo atributo (ahora
  se recorta a 4 en partida), y el 4, 4, 2, 2 no se alcanza con las piezas que hay. Rehacer el
  equipo de Tier 1 para que el mejor reparto exista y sea muy difícil es contenido pendiente.

## 3. Nombre opcional en los temporizadores

- `TimerDefinition::nombre`: lo lee el cargador y el serializador lo escribe solo si lo tiene (las
  aventuras y las fixtures congeladas no cambian). `FORMATO_AVENTURAS.md` lo documenta.
- El HUD lo enseña en lugar de «TIEMPO», en Lora para que quepa («El vacío se cierra 0:39»). Las
  líneas de `MissionTimerWidget` también.

## 4. Todas las tiradas, con pausa

- `RPG::HistorialTiradas` (en `TiradaVisible.h`) guarda las últimas 32 tiradas de la mesa con el
  número de la acción que las hizo. La mesa empieza acción en `resolverAliado`,
  `resolverEnemigo` y `finDeRonda`; `CartasEnCombate::tiradas()` es nuevo y `ultimaTirada()`
  sigue igual.
- `BarraCombate` las pone en cola: cada una se queda **1,6 s** en pantalla (`kPausaTirada`) y
  brilla al salir. El panel dice «2/3» si la acción tiró varias veces.
- El rival no actúa hasta que se han visto (`rivalListo()`), y ganar o perder espera también
  (`Application::desenlaceTrasTiradas`); huir no espera.
- Lo que no hace todavía: la vida y los estados cambian al resolverse la acción, no al ver su
  tirada. Con muchas tiradas habrá que sincronizarlo, como dijo Oriol.

## 5. Combates en grupo

- **Formato:** `startBattle` admite `"enemigos": [{"id", "cantidad"}]` o `"cantidad"` junto a
  `monsterId` (hasta 8). Se escribe compacto y de ida y vuelta da lo mismo. La validación mira
  todos los ids.
- **Motor:** `GameSession::startBattle` recibe una lista. Si un nombre se repite, va numerado
  («Devoto de los Perdidos 2»). Al ganar caen todos y reparten su botín; al huir, los vivos se
  quedan en el mundo y los caídos no. `SessionData::m_battleEnemyHandles` sustituye al handle
  único.
- **HUD:** si las placas enteras no caben encima del rótulo del turno, van compactas (60 px):
  cinco a la vista, con su número siempre visible y los estados en fichas.
- **Contenido (`boundington_ocaso.json`):** los combates del Ocaso dependen de los hombres de
  Venides, como dice el canon («los guardias retienen a los Perdidos»):

  | | Con 20 hombres | Con 10 | Solo |
  |---|---|---|---|
  | La puerta del cuartel («tres Perdidos») | 1 devoto | 2 devotos | 3 devotos |
  | Los fanáticos («dos que no van armados») | 1 | 1 | 2 |
  | Los captores de Verina | el carcelero | el carcelero | carcelero y devoto |

  El **devoto de los Perdidos** es nuevo (`boundington_enemigos.json`): la ficha Cultist de
  PARTIDAS.docx (9 PV), con el sprite del cultista (alias en `sprites.json`). El tutorial y
  Gorran no cambian.

### Lo que dicen los números

Simulación con la mesa de verdad (BattleState + CartasEnCombate), 2.000 combates por caso, un
bardo de Tier 1 con el kit (24 PV) que solo usa Atacar:

| Contra | Gana |
|---|---|
| 1 saqueador (22 PV) | 82–89 % |
| 2 saqueadores | 17–35 % |
| 3 saqueadores | 0–3 % |
| 2 devotos (9 PV) | 78–89 % |
| 3 devotos | 36–62 % |
| 2 fanáticos | 31–54 % |

En el juego real, contra tres saqueadores Nerea cayó en la primera ronda: 5 de un parcial y 15
de un crítico.

Lo que más pesa es el **crítico de «todos 6» con un dado**: un enemigo sin atributos tira 1 dado,
saca un 6 una vez de cada seis, y si gana es crítico ×1,5: **22 de daño** (`SkillExecutor`,
paso 7). La pifia del defensor da otro crítico de 15. Un héroe de 19 a 24 PV no aguanta dos.
Además, **el kit sube la vida máxima pero no la actual**: Nerea empieza el primer combate con
19/24.

## Verificación

- Pruebas nuevas o cambiadas, en verde:
  - `test_kit_basico` 15/15: las 63 clases exactas, la regla de los topes, la ficha que recorta
    y el `tierUp`;
  - `test_cuaderno_creacion` 12/12;
  - `test_mission_timer_widget` 14/14: el nombre en las líneas, cargado y escrito;
  - `test_pergamino` 61/61: el «2/3» del panel, el nombre en el retal y las placas compactas;
  - `test_barra_combate` 19/19: la cola, la pausa y el rival que espera;
  - `test_cartas_combate` 33/33: un área deja una tirada por blanco con la misma acción;
  - `test_game_session` 29/29: el grupo sale entero, al ganar caen todos, al huir se quedan los
    vivos, y el formato.
- `test_schema_fixtures` 9/9: las fixtures congeladas siguen dando el mismo texto.
- `demo_campana_boundington` dispara 77 de 83 beats; los combates nuevos del Ocaso, todos.
- ctest 202/211. Las 9 que fallan son las 8 de antes más `demo_creacion_personaje`, que espera
  43 razas jugables. PNJ-02 dejó a los dos yokai como `playable: false` y quedan 41: no es de
  esta sesión.
- La guardia da las mismas 44 deudas que en H3-06. `Application.h` tiene 519 líneas (techo 519),
  `GameSession.h` 546 (techo 546) y `BarraCombate.h` no suma miembros.
- Juego real con OpenGL por software:
  - el reloj con nombre;
  - el cofre con cinco piezas;
  - la ficha con +2, +2, +1, +1 y la raza encima;
  - un combate contra dos devotos con «1/2» y «2/2» y las tiradas del rival una detrás de otra;
  - cinco devotos con las placas compactas;
  - tres saqueadores.

## Pendiente

- **El commit**, con `include/RPG/TopesDados.h` (nuevo, no hace falta CMake).
- `sprites.json` se tocó en el Mac a mano (una línea de alias) porque otra sesión lo había
  cambiado.
- **Para Oriol:**
  1. Los topes 2, 4, 6, 8, 8 y 8.
  2. Si el crítico de «todos 6» debe pedir al menos dos dados.
  3. Si ponerse el kit cura hasta la vida nueva.
  4. Si el héroe tendrá compañeros en el combate, que es lo que piden los números de la
     historia.
