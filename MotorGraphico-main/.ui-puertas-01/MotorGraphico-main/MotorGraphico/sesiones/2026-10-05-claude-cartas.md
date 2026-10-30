# H2-06 — cartas de cuatro clases en combate, y creación sin reparto · 05/10/2026

Sesión de Claude (Cowork). Encargo de Oriol: «continua amb l'integració del flux de
partida nova i creació d'una de les 6 classes més parametritzades com el bardo de
les 4 estacions, apòstata, senyor de les armes o fúria dels creadors».

Lo que precisó por el camino:

- Las cuatro clases, y solo lo que afecta al combate. Lo de fuera del combate, al
  final, cuando todo lo demás esté.
- Las cartas estándar se tienen todas desde el inicio. Las de creador (máx. 3 por
  clase) son las que se podrán cambiar; subir de tier vendrá después.
- A mitad: «los stats de CON, DES, INT y CAR no se suben durante la creación de
  personaje, solo por la bonificación de su equipo activo». La raza sí suma. Base 0.

La entrega, con capturas, GIF y preguntas abiertas, está en
`Claude outputs/cartas_2026-10-05/LEEME.md`.

## Cómo está hecho

```
skills.json / passives.json    effects[].mechanicHook, escritos por tools/importar_clases_companeros.py
  SkillDefinition / PassiveDefinition   mechanicHooks (los cargadores ya los leen)
Game (motor_level, sin GL)
  GanchosCarta      el DSL: estado:2:desventaja:todo, escudo:2:10, invoca:10:a,b, bloquea:ataque, pendiente:...
  CatalogoCartas    skills + passives + conditions + summons; cada carta con sus ganchos y su motivo de bloqueo
  MesaCombate       IMesaCombate: lo que BattleState delega cuando el héroe juega con cartas
  CartasEnCombate   la mano, el turno, estados, escudos, invocaciones, reacciones y la pasiva
    CartasJugar.cpp     jugar una carta, gancho a gancho
    CartasTiradas.cpp   tiradas enfrentadas Nd6 (SkillExecutor), golpes, reacciones, daño y escudos
BattleState              setMesa(); resolveAllyAction, resolveEnemyTurn y tickRound delegan si hay mesa
GameSession::startBattle CartasEnCombate::paraHeroe(ficha) -> setMesa
Application              menuDeCartas (las filas) y lineasDeCombate (el detalle de la fila marcada)
```

- **No hay código por carta.** Un efecto nuevo es una palabra del DSL que ya
  usaba `conditions.json`. Lo que el canon no cierra se escribe `pendiente:` y la
  carta sale bloqueada con su motivo: no se inventa ninguna regla.
- **BattleState solo delega.** Sin ficha o sin cartas, el combate es el de
  siempre, y las pruebas de combate que ya había pasan igual.
- La ficha creada conoce todas las cartas de su clase (`learnableSkills` además de
  las de arranque).
- **Creación sin reparto**: fuera el paso «Reparto» de `CharacterCreationScreen`,
  `NewGameFlow` y `PantallaNuevaPartida`. La base es 0 (`kStatMin = 0`); se sigue
  comprobando el tope de 6 en tier 0-1.

## Verificación

- `test_cartas_combate` 30/30, y 17/17 mutaciones la ponen en rojo.
- Pruebas de creación al día: `test_h104_new_game_flow`, `test_new_game_flow`,
  `test_character_creation_flow`, `test_character_creation`,
  `test_editor_apariencia` y `demo_creacion_personaje`.
- ctest GL-free en el contenedor: 192 de 197. Las 5 que fallan fallan igual sin el
  cambio: `test_g14_hud_timers_commerce` (no enlaza en Linux),
  `demo_nova_partida` (orden de un `unordered_map` en Linux),
  `test_gdb12_screens` (trabajo en vuelo del inventario), `test_catalog_content`
  (551 cartas y 140 PNJ frente a 492 y 134) y la guardia entera (las mismas 44
  deudas que el árbol del Mac).
- Guardia `--rapido` en el Mac: antes 9 problemas, después 10. Solo se suma
  `cmake/CartasCombateTests.cmake` sin versionar (y 7 avisos de fuentes nuevas sin
  versionar). Se arregla con el commit.
- Juego real (Linux, OpenGL por software): partida nueva con cada clase y un
  combate con cada una.
- Aplicado al árbol del Mac el 05/10 hacia las 10:45 (hora del Mac): preimagen md5
  36/36, los 14 nuevos no existían, `patch --dry-run` limpio y postimagen 50/50.

## Encontrado de paso, sin arreglar (no es de este paquete)

- «Volver al título» desde «Has caído» o desde la pausa revienta el juego:
  `m_session.reset()` y `Application::update()` la usa antes de que cambie la
  escena; además la escena de título no carga ningún nivel.

## Pendiente

Las preguntas para Oriol están en el LEEME (cartas de creador a tier 1, «+2 CA»,
stat de lanzamiento, los «2 PA», el contador de Desgarro, cómo se entra en Furia,
lo que falta del Señor de las Armas, la vida y el equipo con base 0, FUE de las
invocaciones). `PLA_DEFINITIU.md` (G-09) todavía describe el paso «Reparto»: quien
regenere el plan desde `planificacion/pla/dades.py` puede quitarlo.
