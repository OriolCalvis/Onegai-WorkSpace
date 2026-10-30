# Compañeros en combate, el crítico de dos dados y el estudio de dificultad · 06/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-05-claude-kit-tiradas-grupos.md`. Oriol, a las
preguntas de aquella entrega:

> «¿El crítico de "todos 6" debe pedir al menos dos dados? Sí.»
> «¿El héroe tendrá compañeros en combate (Venides, Verina, Xila)? Sí. Pero no para siempre,
> solo en Boundington, y solo en algunos momentos de la historia. De normal el jugador será
> acompañado de otro personaje, que podremos personalizar cuando creemos nuestro personaje,
> este tiene su propia historia, y es la que se podrá explorar en el postgame.»

Y después, sobre la dificultad:

> «La idea sería que un combate 1 contra 3, tú siendo el 1, murieras un 40 % de las veces, aprox,
> para parametrizar la dificultad; pero un 2 contra 3 murieras pocas veces, mucha mala suerte;
> 3 contra 3 solo morirías si son todos más fuertes que tú [...] prueba con el Apóstata, el Señor
> de las Armas y la Furia de los Creadores aparte del Bardo.»
> «Y luego piensa en el equipo, que puedes subir los stats máximos, y haz un ranking por niveles
> de fuerza.»

Seguimiento: `planificacion/seguimiento/ACOMPANANTES-01.json`.

## 1. El crítico de «todos 6» pide dos dados

- `DicePoolEngine::roll_pool`: `critical` solo con `kDadosParaCritico` (2) dados o más. Un dado
  solo con un 6 es un éxito (10 de daño), no un crítico ×1,5 (22).
- Pruebas que fijaban el crítico de un dado, cambiadas a dos seises: `SkillExecutorTests`,
  `SkillEffectTests`, `SkillTests` y `demo_battle` (que decía «si algún día se decide que no
  deben acumularse, este test lo dirá»). Nueva: `un_seis_con_un_solo_dado_no_es_critico`.
- `GDD_MG.md` §2.3 (fuera del repo del motor) todavía dice «TOTS els daus de la pool»: no lo
  toco, es canon.

## 2. Acompañantes de la historia (Venides, Xila)

- **Datos.** La aventura define a quien puede ir contigo (`AdventureScript::acompanantes`:
  vida, dados, golpes por turno, si se le puede atacar, si se retira a 0 PV y su cura) y cada
  `startBattle` pide los suyos (`"acompanantes": [{"id", "siFlag"}]`, hasta 3). Lectura,
  escritura y validación en `src/RPG/Narrative/AcompanantesAventura.*`; formato en
  `FORMATO_AVENTURAS.md`. Las aventuras sin acompañantes se escriben igual que antes.
- **Motor.** `Game::Acompanante` (cuerpo sin ficha) y la mesa
  (`CartasEnCombate::sumaAcompanante`, `src/Game/CartasAcompanantes.cpp`):
  - actúan solos cuando el héroe cierra su turno, tras sus invocaciones y antes que los
    enemigos; cada uno es una acción aparte en el historial de tiradas («1/2», «2/2»);
  - un sanador cura 1d`dado`+`mas` al más apurado (vida ≤ umbral) mientras le queden curas;
    las curas valen para toda la aventura (variable `curas_<id>_<aventura>`);
  - si no, golpean al último enemigo en pie; el héroe suele ir al primero y así cada uno
    entretiene a uno;
  - **los enemigos van a por quien les ha hecho daño el último** (héroe o acompañante;
    `SituacionCombate::agresor`), si no a por el héroe. Las invocaciones siguen sin quitarle la
    atención al héroe;
  - a Xila no se le ataca; Venides «siempre huye en cuanto su vida llega a 0»: se retira;
  - si cae el héroe, los demás se retiran y la batalla se pierde.
- **Sesión.** `GameSession::startBattle` recibe los acompañantes (misma línea en la cabecera:
  `GameSession.h` sigue en 546). Sin mesa de cartas (héroe sin ficha) no vienen. El registro
  dice «A tu lado: Venides, Xila.».
- **HUD.** En el orden de turno, detrás del héroe: su retrato, la vida de Venides en una barra,
  las curas de Xila en marcas verdes, el marco carmín mientras se ve su tirada y su zona para
  apuntarle una carta de aliado (`aliadosEnMesa`).
- **El Ocaso** (`boundington_ocaso.json`): Venides pelea a tu lado en los siete combates
  (16 PV, CON 3, DES 2, dos golpes: la ficha Soldier de PARTIDAS.docx), y Xila si le conociste el
  primer día (`bnd_pista_xila`: no pelea, cura 1d4+1 dos veces en la noche). Verina se une tras
  el rescate, cuando ya no quedan combates. Tres líneas de los combates de la puerta dicen ahora
  que Venides está contigo.
- **Lo que no es esto:** el compañero personalizable que se creará con el personaje (su
  historia, el postgame). Cuando llegue, `Game::Companeros` (CLASES-03) es el sitio.

## 3. Los dados de los enemigos en los datos

- `ObjectDefinition::combat.stats` (`{"CON": 1, "DES": 0, ...}`, 0 a 8): los dados de un
  enemigo sin ficha en `monsters.json`. `EnemyCatalogBridge::createEnemy` los pone en el
  `EnemyBrain`; `exportJson` los escribe. Sin `stats`, todo 0, como antes.
- Los del Ocaso, en la escalera del estudio: devoto = F2 (12 PV, CON 1), fanático = F5
  (12 PV, CON 1, DES 2: usan magia, peor que armados), carcelero = F3 (16 PV, CON 1). El
  saqueador del tutorial y Gorran no cambian.

## 4. El estudio de dificultad

`examples/simulador_dificultad.cpp` (objetivo `simulador_dificultad`, `cmake/SimuladorDificultad.cmake`):
combates con el motor de verdad (BattleState, la mesa de cartas, los acompañantes). El héroe se
crea como en el juego para **cada una de las 41 razas jugables**: la clase con todas sus
cartas, el kit del cofre puesto pieza a pieza y la vida de TierRules. Juega una IA de cartas
por clase (prioridades escritas a mano y rematar al que está a un golpe; el Bardo elige
Invierno, la mejor estación para aguantar). 50 combates por raza: 2.050 por clase y caso.

Los enemigos son de los de las aventuras, sin ficha: vida y dados (ataca con 1 + DES, se
defiende con el mayor de CON y DES).

### La escalera de fuerza (Tier 1) contra el héroe con el kit

| | Enemigo | 1 contra 1 | 1 contra 2 | 1 contra 3 |
|---|---|---|---|---|
| F1 esbirro | 9 PV, sin dados | 0 % | 1 % | 9 % |
| **F2 estándar** | **12 PV, CON 1** | **0,4 %** | **10 %** | **41 %** |
| F3 duro | 16 PV, CON 1 | 1 % | 18 % | 59 % |
| F4 veterano | 12 PV, CON 1, DES 1 | 1 % | 20 % | 62 % |
| F5 élite | 12 PV, CON 1, DES 2 (los monstruos de Tier 1 del catálogo) | 4 % | 35 % | 80 % |
| F6 élite dura | 16 PV, CON 2, DES 2 | 6 % | 53 % | 92 % |
| F7 campeón | 20 PV, CON 2, DES 2 | 8 % | 57 % | 94 % |
| F8 campeón fiero | 16 PV, CON 2, DES 3 | 16 % | 72 % | 98 % |

(Media de las cuatro clases; cae el héroe.)

### Niveles de fuerza del héroe: el equipo hasta el tope de Tier 1

| Nivel | Equipo | El escalón que da el 40 % en 1 contra 3 |
|---|---|---|
| H1 | el kit del cofre (+2, +2, +1, +1 en la clase) | F2 estándar (41 %) |
| H2 | tope 3: 3, 3, 1, 1 en la clase | F3 duro (42 %) |
| H3 | tope 4: 4, 4, 2, 2 en la clase (el máximo de Tier 1) | F5 élite (41 %) |
| H4 | tope 4 en CON y DES | F6 élite dura (41 %) |

### La regla de Oriol, nivel a nivel (media de las cuatro clases)

| Héroe contra su escalón | 1 contra 1 | 1 contra 2 | 1 contra 3 | 2 contra 3 | 3 contra 3 |
|---|---|---|---|---|---|
| H1 contra F2 | 0,4 % | 10 % | 41 % | 5,5 % | 1,6 % |
| H2 contra F3 | 0,5 % | 10 % | 42 % | 3,2 % | 0,6 % |
| H3 contra F5 | 0,1 % | 7 % | 41 % | 3,2 % | 0,5 % |
| H4 contra F6 | 0,2 % | 7 % | 41 % | 3,0 % | 0,6 % |
| **Un escalón más fuertes** | | | | | |
| H1 contra F4 | 1,4 % | 20 % | 62 % | 11 % | 4,1 % |
| H2 contra F5 | 1,4 % | 23 % | 69 % | 11 % | 2,8 % |
| H3 contra F6 | 0,5 % | 15 % | 60 % | 5,6 % | 0,9 % |
| H4 contra F8 | 0,6 % | 17 % | 62 % | 8,6 % | 1,8 % |

El compañero de los «2 contra 3» y «3 contra 3» es uno de su mismo nivel (20 a 26 PV, sus
mismos dados, un golpe por turno).

### Lo que dicen los números

1. **La regla se cumple con un enemigo por nivel.** Si el escalón del enemigo sigue al nivel de
   equipo del héroe, 1 contra 3 se queda en el 40 %, 2 contra 3 en el 3-6 % y 3 contra 3 por
   debajo del 2 %; un escalón por encima, 60 %, 10 % y 3 %. La dificultad se parametriza con
   el escalón: fácil un escalón por debajo, difícil uno por encima.
2. **Las cartas cuentan mucho**: contra tres estándar, solo con Atacar cae el 71 %; jugando sus
   cartas, el 41 %.
3. **La raza manda más que la clase.** Con el kit, contra tres estándar, la mejor raza de cada
   clase (CON 5-6) cae el 4-12 % y la peor (CON 1-2 y DES 1-2, 15-20 PV) el 78-88 %. La CON se
   cuenta dos veces: ×3 en la vida y como dados de defensa.
4. **Las cartas tiran todas con DES**: `castingStat` es `null` en `skills.json` y el motor pone
   DES. El atributo principal de la clase (CAR del Apóstata, la Furia y el Bardo; INT del Señor
   de las Armas) no pone ni un dado: solo da vida (CAR ×1). Por eso el mejor equipo en CON y DES
   (H4) vale un escalón más que el mismo tope en la clase (H3).
5. **Por clases**, en 1 contra 3 el Señor de las Armas y la Furia caen más (45-48 %) que el
   Apóstata y el Bardo (36 %); en grupo, el Apóstata es el que más cae (11 % en 2 contra 3, el
   Bardo un 2 %): su primer turno no hace daño (canaliza e invoca) y sus razas de poca CON tienen
   18 PV.
6. **La vida cuenta a saltos de 5**: un golpe es 5 (parcial), 10 (éxito) o 15 (crítico). Un
   enemigo de 9 o de 10 PV es el mismo (cae de un éxito), y uno de 16 a 20 también (dos).
7. **El Ocaso con Venides** (los tres combates seguidos, sin descanso): con 20 o 10 hombres
   casi nunca se cae (0-0,1 %); solo, un 6 % (el Apóstata, un 22 %). Sin Venides serían un 10,
   24 y 80 %. Xila apenas cambia nada (5,4 % en vez de 6,3 %): su cura es pequeña, como en el
   canon.
8. **Los 1 contra 1 de Boundington**: el saqueador del tutorial mata un 1 % de las veces y
   Gorran un 4 %. Un jefe al 20 % sería Gorran con 30 PV, CON 2 y DES 2; al 30 %, 26 PV, CON 2
   y DES 3 (no lo cambio: es pregunta).

Para rehacer las cuentas: `cd build && ./simulador_dificultad <modo> [combates por raza]`, con
los modos `fichas`, `escalera`, `equipo`, `niveles`, `razas`, `ocaso` (lee los datos de verdad
del Ocaso), `jefes` y `log` (un combate de cada clase con su registro).

## Verificación

- Pruebas nuevas, en verde:
  - `test_cartas_combate` 38/38: Venides golpea dos veces al último tras el héroe y sus tiradas
    van aparte; cada enemigo va a por quien le hizo daño; a Xila no se le ataca y cura mientras
    le quedan; Venides se retira a 0 y si cae el héroe se pierde; el formato se lee, se escribe
    y se valida;
  - `test_game_session` 31/31: el combate de la aventura trae a los suyos (Xila con su flag), las
    curas duran toda la aventura, y sin mesa no vienen;
  - `test_enemy_catalog_bridge` 14/14: los dados del enemigo salen del objeto y se exportan;
  - `test_pergamino` 62/62: Venides y Xila en el orden de turno, con su zona y sus marcas;
  - `test_skill_executor`, `test_skill_effect`, `test_skill`, `demo_battle`: el crítico de dos
    dados.
- ctest: las mismas 9 que fallaban antes (`demo_creacion_personaje`, `demo_nova_partida`,
  `test_gdb12_screens`, `test_g14_hud_timers_commerce`, `test_application*`, `test_catalog_content`,
  `linea_base_core`), ninguna nueva.
- La guardia, las mismas 44 deudas. Avisa, como debe, de que `CartasAcompanantes.cpp` y
  `AcompanantesAventura.cpp` van en el mismo commit que su línea de CMake.
- `demo_campana_boundington` pasa con el Ocaso nuevo.
- Juego real (OpenGL por software): tres devotos con Venides y Xila al lado; el marco pasa a
  Venides mientras se ve su «1/2»; el devoto que golpeó Venides va a por él; Xila cura y gasta
  sus marcas; el devoto caído sale tachado.

## Pendiente

- **El commit**, con los ficheros nuevos y su CMake: `include/Game/Acompanante.h`,
  `src/Game/CartasAcompanantes.cpp`, `src/RPG/Narrative/AcompanantesAventura.{h,cpp}`,
  `tests/Game/AcompanantesTests.cpp`, `examples/simulador_dificultad.cpp`,
  `cmake/SimuladorDificultad.cmake`.
- **Para Oriol:**
  1. ¿Fijamos la escalera F1-F8 como la de Tier 1 y la dificultad como «el escalón del enemigo
     respecto al nivel de equipo del héroe»?
  2. ¿Las cartas tiran con el atributo principal de la clase? Hoy todas tiran con DES.
  3. ¿Bajamos el peso de la CON (vida ×3 y dados de defensa) para que la raza no decida tanto?
  4. ¿Gorran más duro? Hoy mata un 4 % de las veces.
  5. Siguen abiertas: los topes 2, 4, 6, 8, 8 y 8, y si ponerse el kit cura.
