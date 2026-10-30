# Ronda del 15/10 — seis agentes de Claude en paralelo

Pedido por Oriol (14/10, 23:45), después de jugar con los once paquetes aplicados:
> «me gustaría que los NPCs se movieran un poco y tuvieran algunas animaciones más y que cada uno hiciera alguna
> cosa, se pueden repetir algunas animaciones; la ciudad principal de Boundington se sigue viendo demasiado
> caótica; necesito ayudas en los minimapas para poder marcar las misiones y los objetivos que el jugador
> considere importantes, porque es bastante difícil incluso para mí saber qué se tiene que hacer.»
> «Fes servir més agents i divideix les feines per ampliar abast.»

Los lanza y los integra Claude Cowork. **Base de todos:** la carpeta de Oriol de ahora (con los once paquetes del
14/10 ya aplicados). Cada uno trabaja en su copia `/tmp/ag/<carril>/MG` y entrega un paquete en
`MotorGraphico/out/claude_<carril>01/` que entra **por ancla** (formato de `out/claude_reputacion01/`).
Claude Cowork los prueba juntos en este orden: pnj_datos → pnj_motor → misiones → mapa → ciudad → puertas.

| Carril | Agente | Qué | Zona (lo que puede tocar) |
|---|---|---|---|
| `pnj_motor` | Opus | Los PNJ se mueven y hacen cosas (motor) | NUEVOS `include/Game/RutinasPnj.h`, `src/Game/RutinasPnj.cpp`, `tests/Game/RutinasPnjTests.cpp`, `cmake/RutinasPnj.cmake`; por ancla: `GameSession*.cpp/.h`, `src/Engine/ApplicationNivel.cpp`, `src/Engine/ApplicationBucle.cpp`, `src/Render/HojaPersonaje.cpp`, `CMakeLists.txt` |
| `pnj_datos` | Sonnet | Qué hace cada PNJ (datos) | NUEVOS `assets/catalogs/rutinas_pnj.json`, `tools/validar_rutinas_pnj.py`, `tools/test_validar_rutinas_pnj.py`; informe `planificacion_alcance_y_vision/RUTINAS_PNJ.md` |
| `mapa` | Opus | Seguir una misión, flecha al objetivo, mapa grande con chinchetas | `src/HUD/Pergamino/*` (MinimapaTinta, HudRetales, InterfazPergamino, PaginaMisiones, nuevos), `include/HUD/Pergamino/*`, `src/Game/DiarioMisiones.cpp`, `include/Game/DiarioMisiones.h`, `SaveGame*` (solo añadir campos), `src/Engine/Application.cpp` (la tecla), `ApplicationHud.cpp`; por ancla |
| `misiones` | Sonnet | Que cada objetivo diga adónde ir (datos) | `assets/adventures/*.json`, `assets/catalogs/quests.json`, `assets/misiones/*.json`; NUEVOS `tools/medir_objetivos.py` y su prueba; informe `planificacion_alcance_y_vision/OBJETIVOS_CLAROS.md` |
| `ciudad` | Opus | Boundington menos caótica (2.ª vuelta de AV15-03) | `tools/gen_ciudad.py`, `tools/gen_ciudad_deco.py`, `tools/despejar_calles.py`, `assets/levels/ciudad_centro.json`; informe `planificacion_alcance_y_vision/RUIDO_CIUDAD_2.md` |
| `puertas` | Haiku | Arreglar las llegadas tapadas y meter las guardas en validar_todo | `assets/levels/mundi_landmass_*.json`, `assets/levels/ciudad_en_*.json`, `tools/validar_todo.py` |

Nadie toca la zona de otro. Si hace falta, se escribe en el INFORME y lo junta Claude Cowork.

## Contrato entre `pnj_motor` y `pnj_datos`: `assets/catalogs/rutinas_pnj.json`

```json
{
  "version": 1,
  "porDefecto": { "rutina": "deambular", "radio": 2, "pausa": [3, 8], "mira": "jugador" },
  "rutinas": {
    "mercader":        { "rutina": "actividad", "radio": 1, "pausa": [4, 9], "mira": "calle",
                          "actividad": { "nombre": "pregonar", "movimiento": "vaiven", "icono": "oro", "cada": [6, 12] },
                          "frases": ["¡Fruta fresca!", "¡Mirad, mirad!"] },
    "guardia":         { "rutina": "vigilar", "radio": 0, "pausa": [5, 10], "mira": "giro" },
    "anciana":         { "rutina": "deambular", "radio": 2, "pausa": [6, 14] },
    "npc_race_*":      { "rutina": "deambular", "radio": 3, "pausa": [3, 9] },
    "bardo":           { "rutina": "actividad", "radio": 0, "actividad": { "nombre": "tocar", "movimiento": "golpe", "icono": "", "cada": [1, 1] } }
  }
}
```

- **Clave:** el `objectId` exacto, o un prefijo acabado en `*` (gana la clave exacta; entre prefijos, el más largo).
- **`rutina`** (lo que hace el motor):
  - `quieto` — no se mueve; solo respira y mira.
  - `deambular` — cada `pausa` segundos va a una casilla libre a `radio` o menos de su sitio de origen, y vuelve
    de vez en cuando.
  - `ir_y_volver` — entre su sitio y la casilla `destino` ([dx, dy] relativo) por el camino corto.
  - `charlar` — busca a otro PNJ a 3 casillas o menos y se ponen de cara; si no hay nadie, `deambular`.
  - `vigilar` — en su sitio, girando de cara en cara.
  - `actividad` — en su sitio (o a `radio`), hace su `movimiento` de vez en cuando.
- **`movimiento`** (con los fotogramas que YA hay: andar en 4 rumbos, quieto en 4 rumbos y la respiración del
  jugador; se pueden repetir):
  - `respirar` — el ciclo de reposo (`cicloDeReposo`).
  - `vaiven` — alterna mirar a izquierda y derecha (barrer, pregonar, buscar).
  - `golpe` — baja y sube 2 px, rítmico (martillo, tocar, trabajar).
  - `giro` — pasa por los cuatro rumbos despacio (vigilar, otear).
  - `andar_sitio` — el ciclo de andar sin moverse (bailar, impaciente).
- **`icono`**: id de `assets/textures/iconos.json` que sale en un bocadillo sobre la cabeza mientras dura la
  actividad (`""` = sin bocadillo).
- **`mira`**: `jugador` (se gira hacia ti si pasas a 2 casillas o menos), `calle`, `vecino`, `giro` o `fijo:arriba|derecha|abajo|izquierda`.
- **`frases`** (opcional, ≤ 40 caracteres): de vez en cuando, en un bocadillo de texto, solo si estás a 4 casillas o menos.
- **Reglas que el motor cumple siempre:**
  - un PNJ nunca pisa: tu casilla, la casilla de una puerta ni la de delante de una puerta, ni el carril recto
    de una calle principal (el de `despejar_calles.py`);
  - si hablas con él, se para y te mira;
  - las tiendas y los diálogos lo siguen encontrando (sigue siendo el mismo objeto del mundo);
  - en combate y en cinemáticas no se mueve (manda la cinemática);
  - con `animacionesReducidas` (accesibilidad), no hay `vaiven` ni `andar_sitio`.
- **Sin catálogo** (o un PNJ que no esté): `porDefecto`. El motor no se rompe nunca por el catálogo.

---
## Estado al acabar la ronda (15/10, 00:30)

La máquina de trabajo (la VM del Mac) **se colgó** a mitad de ronda: seis agentes a la vez y tres compilando
fueron demasiado para 4 núcleos y 3 GB. Cuatro agentes acabaron igual, trabajando en su contenedor:

| Carril | Paquete | Estado | Qué falta para darlo por bueno |
|---|---|---|---|
| `pnj_datos` | `out/claude_pnj_datos01/` | Hecho. 108 tipos y 218 PNJ, todos con rutina; validador 0 errores, 0 avisos | Nada (solo datos) |
| `pnj_motor` | `out/claude_pnj_motor01/` | Hecho. 16 pruebas; `juego` compilado y capturado en el contenedor | `cmake . && ninja` en la VM; probarlo con el catálogo de `pnj_datos` |
| `mapa` | `out/claude_mapa01/` | Hecho (seguir misión, flecha, mapa con M y chinchetas). Pruebas GL-free en verde | **`juego` sin compilar**; capturas reales |
| `ciudad` | `out/claude_ciudad02/` | Hecho. 297 → 190 objetos; ropa 62 → 17; banderas 16 → 6; guardas en verde | `ctest` y capturas reales del juego |
| `misiones` | — | **No hecho** (el shell se colgó). Dejó el análisis: ver abajo | Relanzar |
| `puertas` | — | **No hecho** | Relanzar |

**Nada de esto se aplica todavía.** Claude Cowork tiene que probarlos juntos en la VM (orden: pnj_datos →
pnj_motor → mapa → ciudad) cuando la VM vuelva. Choques ya conocidos: `pnj_motor` toca dos líneas de
`InterfazPergamino` y una de `SaveGame.cpp` (zona `mapa`); `mapa` tocó `InputMap.h` e `InputSystem.cpp` (tecla M).

Lo que dejó `misiones` para relanzar: para que la flecha encuentre a una persona de otro nivel, `donde` tiene que
ser `["assets/levels/X.json", "objectId"]` (el nivel traza la ruta; el objeto marca a la persona al llegar). Faltan
`donde` en: prólogo (`obj_cofre`, `obj_equipo`, `obj_rio`), primer día (`obj_pruebas`), segundo día (los 4), ocaso
(los 3), precuela de la taberna (los 4), y las opcionales `bnd1_op_*` de `quests.json`. Rosita es `skilla`.

---
## Ronda siguiente, pedida por Oriol (14/10, 23:53): trabajos y acciones

> «implementar trabajos y acciones: estudiar en la biblioteca, luchar en el coliseo, forjar armas en la herrería
> para que las venda el tendero, bañarte en las termas, sentarte en los bancos y sillas, subirte a caballos u
> otros animales, ayudar con el huerto y cultivar, pescar, ayudar en el bar y trabajar allí sirviendo».

**La base ya existe:** ACTUAR-01 (`include/Game/Actuacion.h`): un objeto del catálogo con un bloque `actuacion`;
al interactuar, una tirada enfrentada y un resultado (oro, público), con descanso por pasos guardado en la partida.
La idea es generalizarlo a **«sitios de oficio»**: un bloque `oficio` en el objeto (yunque, pupitre, banco,
bañera, huerto, caña, barra del bar, montura) con su tirada, su tiempo, lo que da y lo que gasta.

Propuesta de carriles (sin lanzar todavía; **un solo agente compilando a la vez**):
1. **Motor de oficios** (Opus): `Game::Oficios` generaliza `Actuacion` (tirada, tiempo de juego que pasa, resultado:
   oro, objeto, mejora de habilidad, moral, fatiga) + la hoja de pergamino «Trabajar» con lo que vas a ganar antes
   de aceptar. Primero los tres que reutilizan lo que hay: **servir en el bar**, **estudiar en la biblioteca**
   (sube una competencia o da una carta de lore), **pescar**.
2. **Sentarse y montar** (Opus, después del 1): punto de asiento en bancos y sillas (AV14-14), y montar: el
   jugador «encima» de un animal con otra velocidad. Hace falta arte de Pixel (sentado, a caballo): hasta
   entonces, una pose de sustitución.
3. **Forja → tienda** (Sonnet): recetas de forja (mineral + tirada → arma con calidad), y que lo forjado entre en el
   stock del tendero (AV4-07, la tienda ya existe).
4. **Coliseo** (Opus o Claude Cowork): combates por rondas con apuesta y premio, con el combate por casillas
   que ya hay.
5. **Termas y huerto** (Sonnet): termas = descanso (cura, quita fatiga, cuesta oro); huerto = plantar, esperar días
   (el reloj del mundo) y cosechar.
6. **Datos** (Sonnet): qué objetos de cada interior son sitios de oficio, con sus números, y los textos.

**Decidido (15/10, 07:02):** dan oro, experiencia y algo de reputación; el orden lo pone Claude. Ver `decisiones/D0017-oficios-dan-oro-experiencia-y-reputacion.md` (con el contrato de `assets/catalogs/oficios.json`).

**Bloqueo:** la VM de esta sesión está colgada (`device_bash` no responde). Hace falta una sesión nueva para compilar e integrar.
