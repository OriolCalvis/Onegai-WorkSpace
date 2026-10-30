# H2-04 y NIVELES-01: niveles, hitos y tiers · 06/10/2026

Sesión de Claude (Cowork), tras `2026-10-06-claude-dano-diferencia.md`. Oriol: «continúa». Del
Gantt de la fase H, la siguiente sin hacer de H2 era **H2-04 — hitos y tierUp: la progresión que
ya existe y nadie llama** (`planificacion/JUGABILIDAD_FASE_H.md`: «terminar una aventura que
concede hito → el personaje sube de tier → su mano y sus límites cambian»).

La ficha ya tenía `milestones` y `grantMilestones()` (con el arreglo del tope de vida al subir,
`tests/RPG/SheetMilestonesTests.cpp`), pero nadie los llamaba.

## Qué hay ahora

- **Efecto de aventura `grantMilestone`** (`value` = cuántos, 1 si no se dice, de 1 a 5; `arg` =
  por qué). Lectura, escritura de ida y vuelta y validación en el cargador, el serializador y
  `AdventureBeatValidation`; `NarrativeResult::milestones`. Documentado en `FORMATO_AVENTURAS.md`.
- **GameSession** (`GameSessionNarrativa.cpp`, sin tocar `GameSession.h`): los hitos van a la
  ficha del héroe con las reglas de tier del nivel. En el registro: «Hito: salir vivo de
  Boundington.», «Consigues un hito (1 en total).» y, si sube, «Subes a Tier 2.». Al subir, la
  vida que gana el tope se le suma.
- **Arreglo en `CharacterSheet::grantMilestones`**: pasaba el total histórico a `tierUp`, que cuenta
  desde el tier actual, así que de uno en uno se llegaba a T3 con 3 hitos en vez de 5. Ahora
  descuenta lo ya pagado: T2 a los 2, T3 a los 5, T4 a los 9.
- **Guardado**: `playerMilestones`, clave aditiva (solo se escribe si hay hitos; los guardados de
  antes se leen igual). Al cargar, el tier sale de los hitos antes de restaurar la vida.
- **Contenido**: el Ocaso da un hito al salir vivo de Boundington (las dos huidas); caer no da
  ninguno. Con T1 → T2 a 2 hitos, la primera aventura sola no sube de tier.
- Lo que cambia al subir lo hace ya el resto del motor con `ficha.tier`: el tope de vida, el de
  dados del equipo y las cartas de Creador, que esperan al Tier 2. El cuaderno ya enseña «T2».

## Pruebas

- Nuevas: `el_efecto_grantmilestone_se_lee_se_escribe_y_se_valida` y
  `un_beat_concede_hitos_y_el_heroe_sube_de_tier_y_lo_guarda` (`test_game_session` 33/33),
  `h204_de_uno_en_uno_t3_pide_cinco_hitos` (`test_sheet_milestones` 9/9), `playerMilestones` en
  `test_save_game` (32/32). `test_kit_basico` 15/15. `validar_todo.py` OK.
- ctest: las mismas 9 que fallaban. La guardia: las mismas 44 deudas.

## NIVELES-01: veinte niveles por tier y un hito para pasar (misma tarde)

Oriol, al ver lo de arriba:

> «Boundington no es final de Tier 1 ni de lejos [...] dijimos 20 niveles para subir al tier [...]
> Boundington entero debe dejarte máximo nivel 15 completando todo al 80 % y 17 al 100 %;
> completar al 100 % tiene que ser difícil. El hito 2 será la siguiente campaña [...] Boundington
> canon como mucho debería dejarte al nivel 8, que sería la mitad, con un par de subquests
> importantes al nivel 10.»
> «Y claro, no solo subiendo nivel llegas a Tier 2: sin los hitos te quedas en Tier 1.»
> «Y sin poder subir al 21, obviamente.»

Cambia la regla de arriba (T2 a los 2 hitos) por esta:

- **La ficha tiene `nivel`** (empieza en 1, hasta 100). Veinte niveles por tier.
- **Un hito por tier**: es el permiso para pasar. Sin hito no se pasa del nivel 20 ni de Tier 1;
  con el 20 y el hito, Tier 2 y ya se puede subir al 21. Lo mismo a los 40, 60 y 80
  (`CharacterSheet::grantLevels`, `grantMilestones`, `nivelMaximo`). El tier nunca baja.
- **Efecto `grantLevel`** (`value` de 1 a 20, 1 si no se dice; `arg` = por qué). En el registro:
  «Por acabar el prologo.», «Subes a nivel 8.»; en el 20 sin hito, «Nivel 20: hace falta un hito
  para seguir subiendo.». `grantMilestone` queda para el final de una campaña.
- **Guardado**: `playerLevel` y `playerMilestones`, claves aditivas.
- **Boundington ya no da hito** (el de Tier 2 es la campaña siguiente). Da niveles:

| Qué | Niveles | Nivel al que llegas |
|---|---|---|
| Tutorial, prólogo, prueba de la iglesia, fin del día 1, fin del día 2 | +1 cada uno | |
| Salir vivo del Ocaso (cualquiera de las dos huidas) | +2 | **8 (canon)** |
| La espada de Skilla (Gorran) y la alianza con los gitanos de Surysal | +1 cada una | **10** |
| Todos los niños del bosque, los desagües, la prueba de los Barrios Altos, Venides con 20 hombres y salvar a Verina | +1 cada una | **15** |
| Las secundarias de los días 1 y 2 que faltan por escribir | +2 en total | **17 (100 %)** |

Caer en Boundington no da niveles. Las difíciles (todos los niños, los 20 hombres, Verina) son las
que separan el 80 % del 100 %. El reparto está en `_canon.niveles` del Ocaso y se cambia en los
JSON sin tocar código.

Pruebas: `niveles_veinte_por_tier_y_un_hito_para_pasar` y los casos de hitos rehechos con la regla
nueva (`test_sheet_milestones` 9/9), `los_beats_dan_niveles_y_el_hito_deja_pasar_a_tier_2_y_se_guarda`
y la lectura de `grantLevel` (`test_game_session` 33/33), `playerLevel` en `test_save_game`
(32/32). ctest: las mismas 9. Guardia: 44.

Pendiente para Oriol: qué da cada nivel dentro del tier (hoy solo el número y el tope para pasar);
y el guardado sigue sin llevar la ficha entera.


## NIVELES-01, segunda vuelta: el reparto de Oriol, fracciones de nivel y hitos por tier

Oriol, sobre la tabla de arriba:

> Canon (tutorial, prólogo, iglesia, fin del día 1 y del día 2): nivel 4. Salir vivo del Ocaso: 6.
> Espada, gitanos, niños, desagües, Barrios Altos, 20 hombres y Verina: 8. Las misiones de la
> ciudad y la reputación básica con los PNJ de los días 1 y 2: 10. Al menos 10 misiones
> secundarias con 3 o 4 retos (encontrar algo, investigar, infiltrarse, robar un banco, salvar un
> banco...), en 4 pasos: 5 niveles. Misiones secretas, mazmorra secreta y puzzles secretos: 2.
> «Boundington da 1 hito, pero necesitas 2 hitos para subir [...] el capítulo 2 te dará otro hito
> [...] tú decides qué capítulo 2 haces, dependiendo de cuál elijas y de tus elecciones en
> Boundington.» «Tier 2 son 4 hitos, tier 3 son 8, tier 4 son 11, tier 5 no tiene forma de
> superarse, es el cap.» «Por cada tier reinicias [...] no se van sumando sin parar.»

Con sus respuestas a dos preguntas (las siete de la fila del 8 reparten +2 entre todas; fracciones
de nivel con progreso dentro del nivel):

- **Fracciones**: `grantLevel` admite `0.25`; la ficha guarda `progresoNivel` (100 puntos = un
  nivel). Registro: «Nivel 8, 25 % hacia el 9.». En el tope del tier el progreso no se acumula.
- **Hitos por tier**, de cero en cada tier: 2 para salir de Tier 1, 4 de Tier 2, 8 de Tier 3, 11 de
  Tier 4; Tier 5 es el techo (hasta el nivel 100). Los que sobran no pasan al tier siguiente.
  `hitosTier`, `hitosQueFaltan()`, `hitosParaSalirDe(t)`. Registro: «Consigues un hito (1 de 2 para
  Tier 2).».
- **Boundington vuelve a dar un hito** al salir vivo; el otro será el capítulo 2.
- **Guardado**: `playerLevel`, `playerLevelProgress`, `playerTier`, `playerTierMilestones` y
  `playerMilestones` (total histórico), todas aditivas; al cargar, `restauraProgresion` rehace la
  ficha con los topes del tier.
- **La hoja** (`FichaVista`): «Luisarda  Nivel 8 (25 %)  T1».

Reparto en los JSON de Boundington:

| Qué | Niveles | Llegas al |
|---|---|---|
| Tutorial, prólogo, prueba de la iglesia, fin del día 1, fin del día 2 | 0,6 cada uno | 4 |
| Salir vivo del Ocaso | +2 y un hito | 6 (canon) |
| Espada de Skilla, gitanos, todos los niños, desagües, Barrios Altos, Venides con 20 hombres, Verina | 0,29 o 0,28 cada una (+2 entre las siete) | 8 |
| Por escribir: misiones de la ciudad y reputación con los PNJ de los días 1 y 2 | +2 | 10 |
| Por escribir: 10 secundarias de 4 pasos | +5 (0,125 por paso) | 15 |
| Por escribir: misiones secretas, mazmorra secreta, puzzles secretos | +2 | 17 |

Pendiente (no hecho): que los capítulos no completados suban su dificultad al tier del héroe
cuando sube («todos los otros capítulos aumentarán su dificultad para adaptarse a Tier 2»), y la
elección del capítulo 2 según lo hecho en Boundington.

## NIVELES-01, tercera vuelta: el nivel no da nada; abre misiones que surgen

Oriol: «El nivel no debería dar nada [...] es una forma de poder acceder a misiones de más
prestigio [...] que se valoren las subidas de nivel con recompensas [...] pero no hay nada de stats
aquí: los stats funcionan y progresan con la equipación». A tres preguntas:

- Recompensas: «los importantes solo vienen de completar una misión; en los otros niveles elige 1
  de 3, pero que no siempre sean equipamientos; solo 1 prenda de cada por tier. Con los tiers se
  amplía la capacidad de equipamiento (joyería, guantes, cinturón...); a partir de Tier 3 se
  habilitan los sets [...] los mejores equipos se consiguen en misiones o por un loot súper raro».
- Cartas de creador: como hoy, y aparte las recompensas pueden dar cartas de creador extra que se
  cambian por una equipada.
- Misiones de prestigio: «que sea una necesidad surgida, no algo estático, así damos al jugador
  sentimiento de progreso del mundo».

Hecho ahora:

- `requires.nivelMinimo` y `requires.tierMinimo` en los beats (`NarrativeCondition`). El estado de
  la aventura lleva el nivel y el tier del héroe (`NarrativeState::ponHeroe`, no se guarda: sale
  de la ficha); GameSession lo pone al subir y al cargar. Un beat `auto` con `nivelMinimo` salta
  solo al llegar: la misión surge. `FORMATO_AVENTURAS.md` lo cuenta.
- El nivel sigue sin tocar atributos, vida ni dados.

Propuesto, sin hacer (espera a Oriol): la elección de 1 de 3 al subir, las ranuras de equipo por
tier y los sets de Tier 3 (ver la respuesta del 06/10 en la conversación y el documento del
proyecto).

## NIVELES-01, cuarta vuelta: el equipo pide nivel

Oriol: «podríamos limitar ciertos objetos con los niveles, para nivelar el progreso».

- `EquipmentDefinition::requiredLevel` (`"requiredLevel"` en equipment.json). `InventoryEngine::can_equip`
  no deja ponérsela antes de ese nivel: «Espada de Alaro: necesitas nivel 8 (tienes el 6)». En la
  mochila se puede llevar antes (un botín raro espera a que llegues).
- `tools/niveles_equipo.py` pone el nivel de las 413 piezas con una regla por tier y por lo que
  suma la pieza a los atributos, y deja en EXCEPCIONES lo que se fije a mano:

| Tier | Nivel | Piezas |
|---|---|---|
| 1 | 1: comunes, equipo inicial de las clases y las 15 poco comunes que necesita el kit | 100 |
| 1 | 5: poco comunes +0/+1 | 40 |
| 1 | 10: poco comunes +2 | 6 |
| 2 | 20: +0/+1 · 26: +2 (y la armadura de placas del Coloso, equipo inicial, 1) | 18 · 63 · 1 |
| 3 | 40: +0/+2 · 46: +3 · 52: +4 | 63 · 78 · 9 |
| 4 | 60 | 35 |

- El kit del cofre se elige con un héroe de nivel 1: sale igual que antes en las 63 clases
  (`simulador_dificultad kits`, modo nuevo, y `test_kit_basico` 15/15).
- Pruebas: `una_pieza_con_nivel_espera_a_que_el_heroe_llegue` y
  `el_catalogo_reparte_el_equipo_por_niveles` (`test_inventory_engine` 9/9). ctest: las mismas 9.
