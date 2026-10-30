# 2026-09-17 · Claude (Cowork) · E-04 cierre: `comercio.abrir`

**Encargo de Oriol:** seguir con el Gantt usando `MAPA_DEL_CODIGO`, con cuidado por los demás agentes. Va después de [E-14](2026-09-17-claude-e14-colision.md), [G-02](2026-09-17-claude-g02-herramientas.md) y [D-01](2026-09-17-claude-d01-contrato-cargadores.md).

## Elección

`comercio.abrir` era el último comando del catálogo declarado y sin cablear, y lo único que mantenía abierto E-04 aparte de la CI. Tras los commits automáticos de Eigent, `GameSession.h` ya no tenía WIP ajeno. Toca un fichero de cruce (`GameSession.cpp`, 6 paquetes) y cuatro tests de otros paquetes que usaban ese comando como «el que no está cableado». Todo eso se listó en la fila del TABLERO antes de empezar.

## Qué se hizo (MG `ff1f3b6`)

- **`GameSession::openShop(id)`**, más un helper único `resolveShopOffers` con la regla de precios que ya usaba `interact()`.
- **`sell()` usa la recompra fijada al abrir la tienda.** Antes buscaba al tendero en el mundo por nombre: no lo encontraba si la tienda la abría un evento, confundía homónimos y desreferenciaba `m_world` sin comprobarlo.
- **`SessionCommandHost`**: `comercio.abrir` devuelve Wait y la espera se resuelve al salir de Shop.
- **Catálogo:** `comercio.abrir` queda como implementado y el parámetro `tienda` pasa a ObjectRef.
- **Tests adaptados**, detalle en `planificacion/seguimiento/E-04-cierre.json`. Se actualizaron `FORMATO_AVENTURAS.md` y `dades.py`.

## Verificación

Se compiló con g++ 11 a partir de las listas de fuentes de `CMakeLists` (sin cmake):
- `test_session_command_host` 15/15, también con ASan+UBSan.
- Las 5 mutaciones se detectan.
- Suites vecinas verdes: `event_commands`, `adventure_validation`, `problems_state`, `demo_mision_llave`, `game_session`, `business_*`, `game_mode`, `carry_over`, `save_game`.
- **Sin verificar:** CMake real y CI.

## ⚠️ HEAD no construye

`motor_project` incluye `src/Project/ProjectDatabaseState.cpp` (E-16), que entró con los commits automáticos de «Eigent User». Ahí `save`/`load` se definen como `Result<void>` mientras la cabecera los declara `bool`, así que no compila, y con él cae casi todo lo que enlaza `motor_project`. Para verificar lo excluí. El arreglo corresponde a la sesión de E-16, o a Oriol si nadie la retoma.

## Para quien siga

- **Application** tiene que pasar `SessionCommandHost` como fallback del intérprete: sigue pendiente, es fichero de cruce.
- Las elecciones en diálogo son de G-14 y la política de derrota, de G-11.
