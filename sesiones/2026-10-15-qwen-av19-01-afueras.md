# 2026-10-15 · qwen · AV19-01 las afueras de Boundington

Cola del orquestador (`orquestador/qwen.md`, tarea 3 de 7). Rama `agente/qwen/AV19-01` (sobre
`agente/qwen/AV19-05`), worktree `~/onegai-ramas/qwen-AV19-05`, commit `e04b493`.

## Qué se hizo

- `afueras_boundington` (40x32) en `tools/av19_niveles.py::afueras()`: senda desde el vado, río con
  puente de madera (x19-21) y vado de arena (x30-33), bosquecillo al oeste, cuatro granjas
  (casa de tile + dos filas de bancales), establo al este, y marcador de sendero a la montaña al sur.
- Conexión ida y vuelta con `ciudad_surysal`: `puerta_afueras_boundington` en (15,46) (sur de los
  campamentos, celda comprobada andable y libre) y `salida_afueras_boundington` en (20,1).
- Marcadores nuevos en `boundington_av19_objetos.json`: `bancal_huerto`, `establo` y
  `sendero_a_montana_gorran` (con `eventId`, sin `targetLevel` hasta que exista `montana_gorran.json`).
- `gen_niveles_av19.py` gana `colision=` por nombre de tile en `Nivel` (exteriores: agua, roca y
  casas bloquean por tile, como `gen_ciudad.py`) y revisa los ids contra los catálogos del proyecto
  boundington, no contra todo `assets/objects/` (un id de otro proyecto pasaba antes por válido:
  `arbusto`).

## Verificación

- `conectividad.py` exit 0: 1011/1011 celdas, 69 objetos alcanzables.
- `validar_puertas.py` 12 (= HEAD), `validar_enlaces.py` 0, `validar_todo.py` 1 error + 22 avisos (= HEAD).
- `juego --nivel assets/levels/afueras_boundington.json 3` carga y vuelca frame.

## Pendiente encadenado

- AV19-03 convertirá `sendero_a_montana_gorran` en puerta real a `montana_gorran.json`.
- Si Pixel entrega arte de granja/establo, se sustituye el tile de casa por su bloque.
