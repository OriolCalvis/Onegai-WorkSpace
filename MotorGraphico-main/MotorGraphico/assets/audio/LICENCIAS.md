# Licencias del audio del juego

Una fila por fichero de `assets/audio/`. `tools/validar_audio.py` la comprueba.

Formato de cada fila (campos separados por ` · `):

`fichero · autor · URL de origen · licencia · fecha de descarga`

## Dos categorías de licencia (no se mezclan)

| Categoría | Valor del campo «licencia» | Qué exige el validador |
|---|---|---|
| Dominio público | `CC0` | nada más |
| Autorización específica | `AUT:<id>` (p. ej. `AUT:erang_dungeon_synth`) | que exista `assets/audio/licencias/<id>.md` con el autor, la URL, la fecha de consulta y las condiciones |

Cualquier otro valor es un problema: una licencia que no está en estas dos categorías no entra.

## Autorizaciones específicas registradas

| id | Autor | Texto de la licencia |
|---|---|---|
| `erang_dungeon_synth` | Erang | `licencias/erang_dungeon_synth.md` (no es CC0; ver sus condiciones) |

## Filas

<!-- Una fila por fichero. Todavía no hay audio descargado. -->
