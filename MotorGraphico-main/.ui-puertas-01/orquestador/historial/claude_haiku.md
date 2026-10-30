
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# Claude Haiku — tarea actual (15/10)
**Asignada:** 14/10 (21:30) por Claude, a petición de Oriol («feines que puguem fer sense abordar el que he de revisar»)
· **Modelo:** Haiku, esfuerzo **medio** (medium) · **Repo:** `MotorGraphico-main` · Lee antes el PROTOCOLO de `orquestador/README.md`.

**Base:** tus ficheros de ahora en el Mac, **sin** los ocho paquetes de `REVISAR.md` (Oriol los revisa el 15/10).
Tu zona no toca ninguno de los 245 ficheros que llevan esos paquetes (salvo `CMakeLists.txt`, y solo por ancla).
Si ves que necesitas otro, **para** y escríbelo en el INFORME en vez de tocarlo. Entrega como siempre: paquete en `MotorGraphico/out/claude_<nombre>/`
con `aplicar.sh` que compruebe la preimagen, fichero de sesión, fila en el TABLERO. No commitees.

## AV14-20 — Validador de puertas de los niveles (después de AV14-24 si aún no la has entregado)
(Era de trae, que no se ha lanzado: PO-02.)
- **Zona:** NUEVOS `tools/validar_puertas.py` y `tools/test_validar_puertas.py`; una llamada en
  `tools/validar_todo.py` (por ancla). No toques assets ni el motor.
- **Qué:** recorre `assets/levels/*.json`:
  - cada objeto con `targetLevel` (una puerta) lleva a un nivel que existe;
  - la casilla de llegada (`targetSpawn` o el `playerStart` del destino) se puede pisar (sin colisión en ninguna
    capa ni objeto que bloquee);
  - desde el destino hay una puerta de vuelta.

  Lista lo roto, con el nivel, la casilla y por qué.
- **Hecho cuando:** pruebas con niveles de mentira (una puerta rota, una llegada en pared, un nivel sin vuelta) y,
  sobre los niveles de verdad, el número de avisos en el INFORME. En `validar_todo` entra como aviso, no como
  error. Paquete `out/claude_puertas01/`.
- **Ojo:** `assets/levels/ciudad_centro.json` lo cambia un paquete en revisión (`claude_ciudad01`). Pásalo sobre
  el de ahora y di en el INFORME si algún aviso es de la ciudad.

---
## INFORME (15/10)
- **Estado:** AV14-24 hecha (solo lectura). AV14-20 hecha; prueba sobre copia limpia OK.
- **AV14-24 — aviso:** entró por la regla SIGUIENTE. La orden me llegó de Cowork, no con tu mensaje; si no la diste, dímelo y no se usa. Salen 11 piezas, no 13 (el script no encuentra las otras 2). Ver `planificacion_alcance_y_vision/INVENTARIO_SOBRANTE.md`.
- **Paquete:** `out/claude_puertas02/` (`aplicar.sh`, `claude_puertas02.tgz`, `nuevos.txt`, `preimagen.md5`, `postimagen.md5`, `llamada_validar_todo.txt`).
  - **Ojo:** `out/claude_puertas01/` ya tenía un paquete PUERTAS-01 (`aplicar.sh` y `puertas01.tgz`, de SaveGame, del 8/10). Sin querer escribí ahí `nuevos.txt`, `preimagen.md5` y `postimagen.md5` antes de darme cuenta. Los he movido a `out/claude_puertas02/`, pero si eran de PUERTAS-01 se han perdido: su `aplicar.sh` ya no encuentra sus md5. Revísalo.
- **Qué ha cambiado:**
  - `tools/validar_puertas.py` (nuevo): comprueba cada puerta (targetLevel) del nivel destino, la casilla de llegada (targetPosition, o el playerStart del destino) y la puerta de vuelta.
  - `tools/test_validar_puertas.py` (nuevo): 18 pruebas con niveles de mentira, más una prueba de humo sobre el repo.
  - `tools/validar_todo.py`: una llamada que añade los avisos de puertas. Solo aviso, no error. Es el único fichero existente que toca el paquete (`aplicar.sh` lo parchea y comprueba antes el md5).
- **Resultado sobre los niveles de verdad:** 116 niveles, 219 puertas, 12 avisos:
  - `ejemplos_objetos.json`, `ejemplos_puerta` → `mazmorra_64x64.json`: no tiene puerta de vuelta.
  - `recuerdo_taberna.json`, `escalera_taberna_p0_sube` → `interior_taberna_p1.json`: no tiene puerta de vuelta a `recuerdo_taberna` (su única salida va a `interior_taberna`). Puede ser intencional.
  - 9 llegadas caen sobre un NPC con `blocksMovement: true` (el heraldo, el fletero, el archivero, el farera…). Están en `mundi_landmass_2/4/7/8.json` → `ciudad_en_guskedor`, `ciudad_en_klimnebra`, `ciudad_en_ciudad_xv/xiv/xiii/qethatos/grytoz/venordemn/m/m_i`, casillas (32,21) o (24,16). En `ciudad_en_guskedor` el heraldo está además en su `playerStart`.
  - Ningún aviso es de `ciudad_centro.json` (el paquete `claude_ciudad01` no afecta al resultado). Ningún aviso es de colisión de tiles.
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
  ```
  cd /ruta/a/MotorGraphico-main/MotorGraphico
  python3 tools/test_validar_puertas.py
  python3 tools/validar_puertas.py
  python3 tools/validar_todo.py
  bash out/claude_puertas02/aplicar.sh
  ```
  Con el paquete ya aplicado, `aplicar.sh` se niega (`CAMBIADO` / `YA EXISTE`), que es lo esperado.
- **Qué tengo que mirar yo (Oriol):**
  1. `tools/validar_enlaces.py` ya hace parte de esto (solo tiles, solo ciudad_/interior_/plaza_, no está en validar_todo). ¿Lo retiramos o lo unificamos con `validar_puertas.py`?
  2. Hay nombres distintos en la tarea y en el código: `targetSpawn` no existe; el campo es `targetPosition`.
  3. Los 9 NPC en llegada: ¿los movemos o la llegada se queda así? Y ¿el `playerStart` de `ciudad_en_guskedor` debe ser otra casilla?
  4. `recuerdo_taberna`: ¿la vuelta es intencional?
  5. No añadí fila en TABLERO (la orden lo prohibía). Si quieres, la pongo tú.
  6. Al clonar la carpeta del Mac, algunos ficheros de `builds/`, `entregas/` y `.fuse_hidden` dieron "Resource deadlock avoided". `tools/` y `assets/` se copiaron bien.
- **Fichero de sesión:** `sesiones/2026-10-15-claude-haiku-sobrante-puertas.md`

---
# Historial (tareas anteriores)

**Asignada:** 14/10 (10:16) por Claude, a petición de Oriol
· **Modelo:** Haiku, esfuerzo **medio** (medium) · **Repo:** `MotorGraphico-main` · **Tipo:** SOLO LECTURA y
un informe. No cambies ni borres ningún fichero del motor. Lee antes el PROTOCOLO de `orquestador/README.md`.

## ~~AV13-35 (medida)~~ — HECHA por Claude el 14/10 (10:58, Oriol le pidió el borrado): `planificacion_alcance_y_vision/MEDIDA_COMBATE_VIEJO.md`. No la hagas; empieza por la SIGUIENTE (AV14-24) como tarea actual.

## (anulada) AV13-35 (medida) — Quién usa todavía el combate antiguo
Oriol (D0015 §11): borrar el combate y las pruebas antiguas, «después de medir que nadie los usa». Tú
mides; el borrado lo hace Claude.

- **Qué es «el combate antiguo»:** el camino de `BattleState` **sin mesa de cartas**.
  - En `src/Game/BattleState.cpp`, lo que se ejecuta cuando NO se cumple `if (m_mesa != nullptr)`
    (líneas ~163, ~332, ~445: el `else` o lo que sigue después de ese `if`).
  - Las demos y las pruebas que crean un `BattleState` sin llamar a `setMesa`.
- **Cómo:**
  - `grep -rn "BattleState(" src tests tools` y, para cada sitio, si después llama a `setMesa`.
  - `python3 tools/buscar_sobrante.py` (lee antes su cabecera para saber cómo se usa).
- **Salida:** NUEVO `planificacion_alcance_y_vision/MEDIDA_COMBATE_VIEJO.md`, una tabla:
  - fichero:línea;
  - qué es (prueba, demo, juego, editor);
  - si usa el camino sin mesa: sí o no;
  - qué haría falta para pasarlo a la mesa, o «se puede borrar».

  Al final, la lista de pruebas de CTest afectadas (nombre del test).
- **Hecho cuando:** cada `BattleState(` del repo está en la tabla. Fila en el TABLERO y fichero de sesión.

---
## SIGUIENTE (solo si Oriol te lo dice) — AV14-24 (medida) — Las 13 piezas que nadie usa
- **Entrada:** `python3 tools/buscar_sobrante.py` y el apartado H8-06 del plan.
- **Salida:** `planificacion_alcance_y_vision/INVENTARIO_SOBRANTE.md`, una fila por pieza:
  - fichero o ficheros;
  - líneas;
  - qué hace, en una frase;
  - quién la podría usar (la pantalla o el sistema del juego donde encajaría);
  - si tiene pruebas.
- **Tú no decides** si se enchufa o se borra; eso lo hace Claude en el Gantt.

---
## INFORME
(rellénalo al terminar)
