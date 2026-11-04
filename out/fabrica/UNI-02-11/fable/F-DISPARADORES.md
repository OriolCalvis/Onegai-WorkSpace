# F-DISPARADORES — disparadores por casilla que llevan al jugador a la escena (Fable, 04/11, solo lectura)

Petición de Oriol: «triggers que se activan si estamos en una casilla concreta; ampliarlo: llevar al jugador donde
empieza la cinemática y, allí, ejecutar el evento (combate, discurso o un simple "ha pasado por aquí")».
Rutas relativas a `MotorGraphico-main/MotorGraphico` (MG).

## 1. Inventario: lo que hay hoy

| Mecanismo | Dónde | Qué dispara | Mueve al jugador | Se configura en |
|---|---|---|---|---|
| **Zona de cinemática** (`laQueEmpieza`) | `include/Game/Cinematica.h:79-88`, `src/Game/Cinematica.cpp:260-273`, consultado cada frame en `src/Engine/ApplicationCinematica.cpp:88-94` | la cinemática; sus pasos `beat`/`flag`/`nivel` encadenan historia | sí, dentro de la escena: `anda jugador` (con colisiones), `coloca`, `nivel`+`en` (`Cinematica.h:16-25`) | `assets/cinematicas/cinematicas.json`: `nivel`, `zona [x0,y0,x1,y1]`, `requiere`, `sin`, `alTerminar`; una vez por partida (`cine_<id>_vista`) |
| **Touch / Autorun / Parallel / Interact** del mapa | `include/Map/EventLayerDocument.h:70-90`, `include/RPG/MapEventRunner.h:102-178`, `src/RPG/MapEventRunner.cpp` | páginas de evento (E-04 comandos: `mapa.ir`, flags, diálogo) | `mapa.ir` sí (transición vía `SessionCommandHost::requestedTransition()`); no «anda» | capa de eventos del nivel (FORMATO_EVENTOS.md). **Ningún nivel de Boundington tiene `"events"`** (grep: 0/`assets/levels/*.json`) |
| **RegionEnter/Stay/Exit** (U3-02) | `include/Map/RegionPresence.h`, `include/RPG/RegionTriggerDispatch.h:55-120`, `src/Map/RegionPresence.cpp` | páginas de evento con trigger de región | no | capa Region del nivel. **0 niveles la usan**; solo tests (`tests/RPG/RegionTrigger*`, `tests/Map/RegionTriggersTests.cpp`) |
| **Beat `enter`** | `include/RPG/NarrativeEngine.h:134-136` (`enterLevel`) | beat al entrar en un NIVEL (92 beats `enter` en `assets/adventures/boundington_*.json`) | no | `trigger: {type: enter, target: <nivel>}`; granularidad nivel, no casilla |
| **Eventos del mundo** | `include/RPG/Narrative/EventosDelMundo.h:27-60`, `GameSession::avaluaEventosDelMundo()` (`include/Game/GameSession.h:264`) | efectos (flags) por `cruzaMomento` / `senal` / `eventoPrevio`; sus `escenas` son documentación que mira `tools/validar_eventos.py`, no las ejecuta el motor | no, ni saben dónde está el jugador (`validar_eventos.py:6`, «sin lugar») | `assets/eventos/boundington.json` |
| **Arnés `pisaLaZona`** | `examples/demo_boundington_partida.h:424-434` y `:692` (`cinematica(GridCoord)`) | simula la cinemática: ejecuta sus pasos `beat` encendiendo la flag y drenando | teletransporta al **centro** de la zona `(x0+x1)/2,(y0+y1)/2` | ids a mano: `bgb_golpes`, `bgc_asalto`, `bgd_velas`, `recinto_subasta` (`:480-576`) |
| **Explorador** | `examples/explorar_boundington.cpp:496-504` (`cineQueEmpieza`) | igual: prueba `laQueEmpieza` con el centro de la zona | ídem (duplicado del arnés) | — |

**Dependencia de la casilla exacta.** 31 de 113 cinemáticas llevan `zona` (contadas sobre `cinematicas.json`), en 13 niveles
(`plaza_sermon` [49,49,61,58] en `ciudad_centro`; `bgd_velas` [15,23,27,31] en `mina2_gorran`; 6 tramos del río Gorran de
borde a borde [0,y,21,y+3]; las 3 `base_ruta_*` de `ciudad_oeste`, etc.). Solo el motor real comprueba la casilla del
jugador; arnés y explorador la **falsifican** con el centro, y lo hacen con dos copias de la misma aritmética. Nada hoy
lleva al jugador a un punto antes de empezar: cada cinemática que lo necesita escribe su propio `anda jugador` (100 pasos
`anda`, 9 `coloca`). El centro puede caer en casilla no transitable y nadie lo valida (`tools/validar_cinematicas.py` mira
niveles, actores, beats y flags; no la zona).

## 2. Diseño: ampliar la cinemática con `disparador`, no un sistema nuevo

**Dónde vive: en `cinematicas.json`, como campo opcional `disparador` de cada cinemática** (la zona ya está ahí; el
catálogo ya es GL-free, ya se consulta cada frame y ya tiene `requiere`/`sin`/`once`). Descartados: la capa de eventos
del mapa (0 niveles la usan, requiere editar 13 TMX/JSON y duplica las condiciones en otro lenguaje) y los eventos del
mundo (son «sin lugar» por contrato). Un combate o una marca sin cinemática se expresa como cinemática de 0 pasos.

```json
{ "id": "plaza_sermon", "nivel": "assets/levels/ciudad_centro.json",
  "zona": [49,49,61,58], "requiere": ["bnd2_dia_cerrado"], "sin": ["bnd3_campanas"],
  "disparador": { "lleva": {"a": [55, 53], "mira": "arriba", "como": "andando"},
                  "accion": {"cinematica": "plaza_sermon"}, "once": true },
  "pasos": [ ... ] }
```
- `zona` (ya existe) o `casilla: [x,y]` (azúcar: zona de 1x1). Condiciones = `requiere`/`sin` (ya existen).
- `lleva`: `a` casilla (debe estar dentro de la zona o adyacente), `mira` rumbo, `como`: `andando` (con colisiones, como
  `anda jugador`) | `fundido` (funde a negro, `coloca jugador`, funde de vuelta). Omitido: no se mueve nada.
- `accion`: exactamente una de `cinematica <id>` (por defecto, la propia), `beat <id>`, `combate <id>`, `marca <flag>`.
- `once` (defecto `true` = hoy, `cine_<id>_vista`); `false` = rearma al salir de la zona (lo que hace Touch en
  `MapEventRunner.h:60-63`): hace falta `m_hasLastCell`-equivalente en `EstadoCinematicas`.

**Cómo lo lee el motor.** `Cinematica.cpp::lee` parsea `disparador` a `struct Disparador {GridCoord a; std::string mira,
como; Accion accion; bool once;}` en `Cinematica.h`. `ReproductorCinematica::empieza(c)` antepone pasos sintéticos:
`lleva` → `{anda|funde+coloca+funde, jugador}` + `{mira, jugador}`; `accion` → `beat` ya existe; `marca` = paso `flag`;
`combate` = paso nuevo `Tipo::Combate` que llama a un `EscenarioCinematica::combate(id)` virtual con cuerpo vacío por
defecto (como `fuego`/`postura`, `Cinematica.h:179-181`), implementado en `ApplicationCinematica.cpp` con
`GameSession::startBattle` (`GameSession.h:531`, la misma puerta que el efecto `startBattle` de los beats). El bucle de
`ApplicationCinematica.cpp:88-94` no cambia. `once:false` toca `laQueEmpieza` (una línea: no saltar las vistas) y una
`std::set<std::string> rearmables` en el estado.

**Arnés y explorador sin duplicar.** Mover la aritmética del centro al catálogo: `GridCoord Cinematica::puntoDeEntrada()
const` = `disparador.a` si lo hay, si no el centro de la zona. `pisaLaZona` y `cineQueEmpieza` pasan a llamarla (dos
borrados). El arnés gana el `combate` y la `marca` gratis porque ya recorre `c->pasos` (`partida.h:703`).

**Validación** (`tools/validar_cinematicas.py`, ya en `validar_todo.py:52`): la `zona` cabe en el `width/height` del nivel
(ya lee el TMX, `:91`); `lleva.a` dentro de la zona o a 1 casilla y transitable (capa de colisión del TMX); `accion` con
exactamente una clave; `beat`/`combate` existen (beats en aventuras, encuentros donde viva el id que use `startBattle`);
`marca` sigue el prefijo de flags del guion; dos cinemáticas con misma `zona`+`nivel` y condiciones no excluyentes → aviso.
Caso de prueba en `tools/test_static_validators.py` con una copia mutada (zona fuera, `a` en muro, dos acciones).

**Prueba GL-free** (`tests/Game/CinematicaTests.cpp`, escenario de mentira): `lleva_al_jugador_y_luego_marca`: jugador en
(50,50) de la zona, `pasoDelJugador` cuenta pasos hasta (55,53), luego `enciende(marca)`; `once_false_rearma_al_salir`;
`combate_pide_al_escenario`. **Mutación**: quitar el `anda` sintético (el test de lleva falla porque la marca se enciende
con el jugador en (50,50)); poner `once` a la inversa; romper `puntoDeEntrada` para que devuelva el centro aun con `a`.

**Dos casos reales que mejoran.** (1) `plaza_sermon` (`cinematicas.json:4274`): zona 13x10; el explorador «no arrancaba»
hasta E-OCASO-ATASCO (`explorar_boundington.cpp:52`) y aún entra por el centro (55,53), que es donde Sandes predica: con
`lleva.a` el punto es declarado, validado como transitable y es el mismo en juego, arnés y explorador. (2) `bgd_velas`
(`:5900`, Gorran, mina2 [15,23,27,31]): el diácono enciende velas «alrededor de la puerta»; hoy el jugador puede entrar
por cualquiera de 117 casillas y la escena no lo recoloca; `lleva: {a:[21,30], mira:"arriba", como:"andando"}` y
`once:false` hasta `bgd_cerrada` evita que la escena quede «vista» si el jugador la cruza corriendo.

## 3. Plan en pasos E

| E | Qué | Ficheros | Riesgo |
|---|---|---|---|
| E1 | `puntoDeEntrada()` + arnés y explorador la usan (sin cambio de JSON) | `include/Game/Cinematica.h`, `src/Game/Cinematica.cpp`, `examples/demo_boundington_partida.h:424-434,:637`, `examples/explorar_boundington.cpp:496-504` | bajo; refactor puro, los demos siguen pasando |
| E2 | Parsear `disparador` (`lleva`, `accion` cine/beat/marca, `once`) y pasos sintéticos; tests GL-free + mutación | `Cinematica.h/.cpp`, `tests/Game/CinematicaTests.cpp`, `cmake/Cinematica.cmake` | medio: `empieza()` antepone pasos; cuidar `anda jugador` cuando ya está en `a` (0 pasos) |
| E3 | Validador: zona en el nivel, `a` transitable, acción única, ids existen | `tools/validar_cinematicas.py`, `tools/test_static_validators.py` | bajo; puede sacar errores latentes en las 31 zonas de hoy (bueno) |
| E4 | `combate <id>` → `EscenarioCinematica::combate` + `ApplicationCinematica.cpp` con `startBattle` | `Cinematica.h`, `src/Engine/ApplicationCinematica.cpp`, `GameSession.h:531` | medio: necesita decidir **D-A** (de dónde salen los enemigos) |
| E5 | `once:false` rearmable al salir de la zona | `Cinematica.cpp::laQueEmpieza`, `include/Engine/ApplicationCinematica.h` (estado) | medio: hoy «una vez por partida» es contrato (`Cinematica.h:27-29`); tocar solo con `once:false` explícito |
| E6 | Migrar `plaza_sermon` y `bgd_velas` (y las 3 `base_ruta_*`) al `disparador`; sesión + bitácora | `assets/cinematicas/cinematicas.json`, `sesiones/` | bajo; datos, validados por E3 |

**Decisiones para Oriol** (marcadas en el plan):
- **D-A (E4) — qué es `combate <id>`.** Opción 1 (recomendada): el id es un `encuentro` ya existente que resuelva el mismo
  camino que el efecto `startBattle` de los beats (una sola puerta, `GameSession.h:478`). Opción 2: lista inline de
  enemigos en el JSON de la cinemática (más flexible, pero duplica el formato de los beats).
- **D-B (E2) — `lleva.como` por defecto.** Opción 1 (recomendada): `andando` (se ve al jugador ir, coherente con
  `anda jugador`; si no hay camino, cae a fundido y lo anota el trace). Opción 2: `fundido` siempre (nunca se atasca, pero
  rompe la continuidad que Oriol pidió: «portar al jugador on ha de començar»).
- **D-C (E5) — `once:false` ¿rearma al salir de la zona o al cambiar de nivel?** Opción 1 (recomendada): al salir de la
  zona, como Touch (`MapEventRunner.h:60-63`). Opción 2: al salir del nivel (más barato, sin estado por casilla).
