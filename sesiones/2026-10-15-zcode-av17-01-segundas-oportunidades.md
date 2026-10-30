# 2026-10-15 · zcode — AV17-01: segundas oportunidades (tres salidas por tirada)

**Rama:** `agente/zcode/AV17-01` (worktree `~/onegai-ramas/zcode-AV17-01`), sobre el `main` unificado
(`0dbaee6`). **Ficheros:** `MotorGraphico-main/MotorGraphico/assets/adventures/boundington_primer_dia.json`
(+10 beats, +2 objetivos) y `boundington_segundo_dia.json` (+4 beats, +1 objetivo). Sin C++, sin catálogos
ajenos, sin niveles.

**Nota de la sesión:** empecé antes del §0 de Oriol y paré como mandaba la ficha; al recibir «continúa»,
trabajé sobre una rama base con una fotografía del índice staged (`git write-tree` + `commit-tree`, sin
tocar nada compartido). A mitad de sesión el repo se unificó (un solo git en `Software/`, el `.git` del juego
retirado, Qwen/Codex ya integrados en `main`): mi worktree quedó descolgado, lo recreé desde el `main` nuevo
y mis ediciones encajaron limpias (diff: solo mis adiciones). La rama base desechable ya no existe.

---

## 1. Tabla de tiradas de Boundington (entrega de AV17-01)

Qué abre el éxito y qué pasaba al fallar HOY (antes de esta sesión). «+3 salidas» = lo que he añadido.
Forma de la regla: **éxito** (lo de hoy) / **fallo → el PNJ ofrece otra cosa** / **fallo otra vez → explorar**.

### Día 1 (`boundington_primer_dia.json`)

| Tirada | Beat | Regla | Abre el éxito | Al fallar (antes) | Ahora |
|---|---|---|---|---|---|
| Luisarda — antorchas (Intimidación/Engaño) | `b1_luisarda`/`_recado` | rival 2 | La sala del fondo: pista de Sandes, la petición, la cena con Rosita | `b1_luisarda_calla` cerraba la tienda para siempre; la cena quedaba inaccesible | **+3 salidas**: favor del sastre (cobrar el hilo de plata) → sala; oír de noche a Luisarda y el sastre → pista + cena igualmente |
| Luisarda — Persuasión | ídem | exitos 2 (con recado) / **4 (sin recado: imposible con <4 dados)** | ídem | ídem | ídem. El 4 de éxitos es canon (BND-ALT, `_canon` del fichero): no lo toco, ahora fallarlo no mata el hilo |
| Aigren — su hijo (Persuasión/Intimidación) | `b1_aigren_hijo` | exitos 2 | `se_va` (saca al hijo) / `asedio` (pista del ataque) | parcial → rumor del líder; **pifia → `b1_aigren_nada` cerraba para siempre** | **+3 salidas (pifia)**: ayudar con el envío nocturno → rumor del líder + pista de las espadas; entrar de noche en la herrería → ver cargar espadas sin papeles (exploración) |
| Aigren — los Perdidos (Persuasión) | `b1_aigren_perdidos` | **exitos 4 (canon)** | `redimido`: pista + daga (día 2) + espadas para la guardia | `b1_aigren_discurso` cerraba para siempre | **+3 salidas**: «papel»: orden del capitán con sello (día 2, el capitán la da al salir) → entrega → redimido + daga; exploración nocturna igual que arriba. La opción «decirle que te interesan» (invitación a la cripta) sigue |
| Sectarios de la plaza | `b1_secta_s2` | rival 3 | masacre (combate) | no cerraba misión | sin cambios (la cripta se rehace en AV17-03) |
| Xila sobria | `b1_xila_s3` | exitos 2 | aliada (fiesta día 2) | «sin un trago no hablo» → **ya tenía** la ruta B (con alcohol) | sin cambios (ya tenía tres salidas de facto) |
| El gato de Mira | `b1_mira_s1` | rival 2 | baja el gato, invita a cenar la familia | el gato bajaba solo y **también invitaba** | sin cambios (no cerraba; la cinemática de trepar es AV17-05) |
| Los Verdaderos | `b1_verdaderos` | rival 4 | se van sin pelea | pelea (combate) → segunda oportunidad | sin cambios |
| Carterista | `b1_carterista` | rival 2 | pillado (hilo de Pulga) | te roba (menor) | sin cambios |
| Vigilante Barrio Alto (noche) | `b1_vigilante` | sigilo 3 / persuasión 4 | pasas | te echa (no cierra misión) | sin cambios |
| Ceremonia de la guarida | `b1_ceremonia` | arcano 2 / persuasión 2 | leer el ritual / charlar | combinaciones de opciones | sin cambios (AV17-03) |

### Día 2 (`boundington_segundo_dia.json`)

| Tirada | Beat | Regla | Abre el éxito | Al fallar (antes) | Ahora |
|---|---|---|---|---|---|
| **Capitán** | `b2_capitan` | persuasión rival 3 | `guardia_preparada` (aliado día 3) | `b2_capitan_no`: «solo da una», cerrado para siempre (la vía del camorrista seguía) | **+3 salidas**: volver al castillo con una **prueba** (cualquier pista del día 1/2) → aliado; o **Venides responde por ti** (afecto ≥ 1) → aliado; el rechazo deja pistá en el registro |
| Gorran — sigilo | `b2_gorran_sigilo` | cd 1.5 (= 3 dados) | robar espada+papeles sin pelear | — | sin cambios |
| Gorran — combate | `b2_gorran_combate` | sinMatar | victoria: espada+papeles | **derrota/huida cerraban la cueva para siempre: misión de la espada muerta** | **+ reapertura**: Gorran te grita «¡sigue abierta!» → la charla vuelve a empezar (limpia `gorran_gana/huido/visto` para no rearmar el cierre; repetible) |
| Fiesta — puerta | `b2_fiesta_puerta` | sigilo 3 | te cuelas | vigilante → persuasión 4 → te echa… pero el **cobrador** (20 oro) seguía como entrada alternativa | sin cambios (AV17-06) |
| Mansión | `b2_mansion` | interpretación 3 / percepción 2 / atletismo 3 | tres vistos distintos | cualquier una vale | sin cambios |
| Invitados | `b2_invitados` | engaño 3-4 / sigilo 3 | sonsacar / robar | fallo suave | sin cambios (AV17-06) |
| Gitanos / ritual | `b2_gitanos` / `b2_ritual` | persuasión 2 / interpretación 2 | alianza de la Barriada | sin alianza (aliado opcional) | sin cambios |

### Ocaso (`boundington_ocaso.json`)

| Tirada | Regla | Nota |
|---|---|---|
| Verina cautiva (percepción 1) | reconocerla cambia el texto, no cierra nada | sin cambios |
| Barricada (percepción 1) | ídem | sin cambios |

## 2. Qué he añadido (resumen)

- **Luisarda** (día 1): `b1_luisarda_otra_cosa` (la oferta), `b1_sastre_cobro` (el favor, en la misma
  sastrería: Luisarda y el sastre comparten `interior_sastreria.json`), `b1_luisarda_favor_cumplido`,
  `b1_luisarda_esperando`, `b1_luisarda_noche` (escuchar de noche), `b1_luisarda_noche_hablada` (la cena por
  la vía de la escucha). `b1_luisarda_calla` suspendido mientras la oferta viva. Objetivo nuevo
  `obj1_luisarda` (doneFlag `bnd_pista_luisarda`, donde la Boutique).
- **Aigren** (día 1): `b1_aigren_otra_cosa` (el envío nocturno), `b1_aigren_envio`, `b1_aigren_papel` (la
  orden del capitán), `b1_aigren_noche` (capas grises cargando espadas). `b1_aigren_discurso`/`_nada`
  suspendidos con las ofertas vivas. Flag nueva `bnd1_aigren_enterado` (la encienden las 4 salidas de
  información existentes + las mías) y objetivo `obj1_aigren`.
- **Capitán** (día 2): `b2_capitan_orden` (da la orden si Aigren la pidió), `b2_castillo_prueba` (volver al
  castillo: prueba o Venides; «Retirarse» la cierra). Log de pista en `b2_capitan_no`. Objetivo
  `obj2_capitan` (doneFlag `bnd2_guardia_preparada`, donde el castillo).
- **Aigren** (día 2): `b2_aigren_orden_entrega` (la orden → redimido + daga de Aigren, reutiliza la ruta del
  éxito canónico).
- **Gorran** (día 2): `b2_gorran_reintento` (reapertura tras derrota/huida, sin bucles).
- **Diario** (`assets/catalogs/quests.json`, fichas de mis misiones): 4 objetivos opcionales nuevos con
  `visibleFlag` (solo se ven al entrar en la vía secundaria): `bnd1_op_luisarda_segunda`,
  `bnd1_op_aigren_segunda`, `bnd1_op_aigren_orden`, `bnd2_op_capitan_prueba` (esta última visible con la
  flag nueva `bnd2_capitan_dijo_no`, que enciende `b2_capitan_no`). El test del diario
  (`test_diario_completo`) exige que cada objetivo de la AVENTURA case 1:1 con un apartado de la ficha:
  por eso las rutas secundarias van como objetivos de ficha (que sí admiten `visibleFlag`) y no como
  objetivos de aventura.
- Actos reutilizados sin tocar `actos.json`: `luisarda_con_rosita`, `aigren_redimido`, `guardia_preparada`.

## 3. Verificación

- `tools/validar_todo.py`: sección Aventuras OK; el FAIL es solo `validar_pnj.py` muriendo en `numpy`
  (igual en el árbol sin tocar: problema de entorno preexistente).
- `tools/medir_objetivos.py --solo-boundington --exigir`: **30/30 objetivos con destino**, exit 0.
- **Simulación del motor narrativo** (14 rutas, primera-coincidencia + ticks, libretos por día como el
  juego): Luisarda favor/noche/éxito, Aigren envío/exploración/orden/éxito, capitán prueba/Venides/
  retirarse/éxito, Gorran reapertura doble sin bucle — todo OK, con regresiones del camino de éxito.
- **Motor real**: `./build-release/juego 90` exit 0, sin ningún «NO cargo» (las aventuras editadas cargan);
  captura en `out/zcode_av17-01/titulo_tras_cambios.png` del worktree (out/ está ignorado en el repo
  unificado: no se commitea).
- ctest gl-free: 4 fallos, todos preexistentes (`test_validar_todo` y `test_aventuras_monsterid` mueren en
  el mismo numpy; `linea_base_core*` es la deuda D0007; cero C++ tocado).
- **Cómo forzar las tiradas en el juego real: no hay forma** (el RNG vive en `DicePoolEngine` vía
  `GameSession`); por eso la verificación de las vías de fallo es la simulación + regresiones. Nota para
  Oriol/Claude: un modo de depuración «forzar grado de tirada» haría esto jugable a mano.

## 4. Límites y decisiones (para el INFORME)

- Los objetivos de la AVENTURA son incondicionales, pero los de la FICCHA (`quests.json`) tienen
  `visibleFlag`: las rutas secundarias usan eso y quedan escondidas hasta que el fallo las abre.
- El «exitos 4» de Luisarda-sin-recado y Aigren-Perdidos es canon aprobado por Oriol (`_canon` del fichero,
  BND-ALT-01): NO lo he bajado. Con las tres salidas, fallarlo ya no mata el hilo.
- La cripta y la fiesta se dejan para AV17-03/AV17-06 como manda la ficha; el cobrador sigue siendo la
  entrada de la fiesta sin tirada.
- El envío nocturno de Aigren solo vale la noche del día 1 (al cambiar de libreto, el día 2 ya no está); la
  vía de la orden del capitán cubre el día 2. Anotado.
