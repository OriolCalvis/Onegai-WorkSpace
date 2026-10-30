# Formato de proyecto

Un **proyecto** es un pack de contenido: sus mapas, sus niveles, sus
aventuras y sus catálogos de objetos. «Los Perdidos de Boundington» es un
proyecto; cada cuadrante del experimento de las cuatro IAs es otro.

Vive en `assets/proyectos/<id>.json`, y su id está listado en
`assets/proyectos/index.json`. El formato unico es **Project v1** (`schemaVersion: 1`).
`ProjectSerializer` lo lee y escribe; `Editor::ProjectIndex` delega en el
serializador y resuelve las rutas contra la raiz de assets solicitada.

---

## Por qué existe

Antes no había proyectos: el editor abría siempre
`assets/maps/editor_map.tmx` y guardaba siempre ahí. Con cuatro autores
trabajando a la vez, `assets/` acabó con **cerca de cien niveles de cuatro
manos distintas mezclados en la misma carpeta**, y no había forma de decir
*«abre Boundington»* ni de mandarle a nadie una historia sola sin mandarle
todos y que adivinara.

> El número exacto no se pone aquí a propósito: cambia cada hora mientras
> los cuadrantes se escriben. Para el de hoy,
> `python3 tools/build_proyecto.py --todos`.

**El dueño se declara, no se deduce.** La primera versión agrupaba por el
prefijo del nombre de fichero y se rompió a la primera: un cuadrante llamó
a sus niveles `ciudad_en_*` y Boundington se los tragó enteros — **86
niveles en vez de 64**, porque también empieza por `ciudad_`.

Si el indice declara un manifiesto ausente o invalido, lo reporta como
**huérfano**. Crear o borrar otro proyecto conserva esa referencia para
que el autor pueda repararla. Que sobre contenido es un aviso;
que se lo quede el proyecto equivocado es un bug silencioso.

---

## El manifiesto

```json
{
  "schemaVersion": 1,
  "id": "boundington",
  "name": "Los Perdidos de Boundington",
  "description": "La caída de la ciudad, tres días",
  "epoch": "1981 b.f.",
  "author": "Oriol + Claude",
  "prefix": "",
  "settings": {
    "schemaVersion": 1,
    "mapsDirectory": "maps",
    "levelsDirectory": "levels",
    "adventuresDirectory": "adventures",
    "catalogsDirectory": "objects",
    "initialMap": "ciudad_centro.tmx"
  },
  "entry": "assets/adventures/boundington_prologo.json",
  "levels": ["ciudad_centro.json", "interior_vacio.json"],
  "maps": ["ciudad_centro.tmx"],
  "adventures": ["boundington_prologo.json"],
  "catalogs": ["boundington_npcs.json"]
}
```

| Campo | Qué es |
|---|---|
| `schemaVersion` | Version de formato: 1, tanto en la raiz como en settings. |
| `id` | Solo `[a-z0-9_]`; debe coincidir con el nombre de archivo y el indice. |
| `name`, `description`, `epoch`, `author` | Metadatos del proyecto. `name` no puede estar vacio. |
| `prefix` | Vacio o terminado en `_`. |
| `settings.*Directory` | Directorios relativos a assets, distintos entre si y sin `..`. |
| `settings.initialMap` | Nombre presente en `maps`, o vacio. |
| `levels`, `maps`, `adventures`, `catalogs` | Nombres de fichero sin ruta y sin duplicados; resueltos contra sus directorios en settings. |
| `entry` | Punto de arranque; vacio significa que el proyecto se edita pero no se juega. |

El indice conserva su contrato independiente: `index.json` contiene
`{"proyectos": ["boundington", ...]}`. No es un manifiesto Project.

### Migrar el formato anterior

Los manifiestos con `nombre`, `niveles`, `entrada`, etc. se rechazan con
un mensaje que indica el comando de migracion. No se mezclan campos de
ambos formatos ni se intenta adivinar una version.

```bash
python3 tools/migrar_proyectos.py          # previsualiza, sin escribir
python3 tools/migrar_proyectos.py --write  # migra los del indice
```

`--assets <ruta>` permite migrar otro pack. Se valida todo el lote antes
de escribir, se guarda una copia previa en la ruta temporal indicada y
cada manifiesto se reemplaza atomicamente. Si un campo no tiene migracion,
se informa en vez de perderlo. Una segunda ejecucion produce cero cambios.
`_nota` era una anotacion editorial y se omite; todos los campos de contenido
se conservan. Las claves y el formato de salida coinciden byte a byte con
`ProjectSerializer`, incluido UTF-8, orden, indentacion y salto de linea final.

### `entry`: rutas y nombres de archivo

Se admiten las dos porque **las dos se escribieron de verdad** antes de
que esto estuviera documentado:

```json
"entry": "assets/adventures/boundington_prologo.json"   // ruta desde la raíz
"entry": "ciudad_en_guskedor.json"                       // solo el nombre
```

Tambien se acepta una ruta relativa a assets, como `adventures/inicio.json`.
Las rutas con prefijo `assets/` se resuelven contra la raiz de assets
solicitada, incluso al abrir una copia del pack. El nombre suelto se busca
primero en `settings.adventuresDirectory` y despues en `settings.levelsDirectory`. `Project::entryKind` deja anotado a qué resolvió
(`Adventure`, `Level`, `Missing` o `None`), y la pantalla de proyectos
muestra **la ruta resuelta**, no la que pone el manifiesto — para que un
`entry` que no resuelve se vea en vez de fallar callando.

**Un proyecto puede arrancar en un nivel y no en una aventura.** Un pack
de escenario sin historia es legítimo: se abre y se camina por él. Sin
`entry` el proyecto se puede editar pero no jugar, y el índice lo dice
antes de ofrecer el botón.

---

## Reglas que `ProjectIndex::create()` hace cumplir

| Regla | Motivo |
|---|---|
| id solo `[a-z0-9_]` | acaba siendo nombre de fichero **y** prefijo de ids |
| prefijo acaba en `_` | sin él, `mm` + `tabernero` = `mmtabernero` |
| no duplicar un id | pisar un manifiesto ajeno no se avisa solo |
| no pisar un manifiesto suelto | si existe el fichero pero no está en el índice, `create()` se niega: darlo por libre lo machacaba con un esqueleto vacío |

`ProjectIndex::remove()` saca el proyecto del índice y borra su
manifiesto, y **nada más**: los niveles y mapas los puede estar usando
otro proyecto, y un borrado en cascada lanzado desde un editor es justo la
clase de operación que no se puede deshacer. Primero el índice, luego el
fichero — si el borrado falla, al menos no queda un id apuntando a algo
que el editor va a intentar abrir.

---

## Por qué un fichero índice y no escanear la carpeta

`scan()` lee `index.json` y después cada `<id>.json`. No recorre el
directorio porque **este motor evita `<filesystem>` a propósito** —
`TileMap.cpp` lo deja escrito: *«obliga a enlazar stdc++fs en algunos
toolchains»*. No se trae esa dependencia para listar cinco ficheros. Y
encaja con el criterio de arriba: se declara.

---

## Sacar la build de un proyecto

```bash
python3 tools/build_proyecto.py boundington    # -> builds/boundington/
python3 tools/build_proyecto.py --todos
```

Empaqueta **solo** lo de ese pack, más las texturas (que las comparten
todos). No compila C++: eso es `cmake --build` y ya funcionaba; lo que no
había forma de hacer era empaquetar la historia.

Dos comprobaciones que salieron de romperlo:

- **Cierre transitivo de `targetLevel`.** Un nivel puede llevar por una
  puerta a otro que no está en el manifiesto. Sin arrastrarlo, la build
  carga bien y el jugador cruza una puerta y **se cae al vacío**. Se
  arrastra y se avisa de qué se arrastró.
- **Cada `objectId` debe tener ficha *dentro* de la build.** Un catálogo
  que se queda fuera *no da error al cargar*: da un objeto invisible, que
  es peor que un crash porque no se nota hasta que alguien busca un PNJ
  que no está.

`builds/` está en `.gitignore`: son copias de `assets/`, que ya está
versionado. Versionarlas duplica el repo y garantiza que se queden viejas.

Desde el editor, la tecla **`B`** en la pantalla de proyectos hace lo
mismo y vuelca la salida en un panel.

El resto de la pantalla (teclas añadidas por H4-01):

| Tecla | Qué hace |
| --- | --- |
| `W` / `S` | mover por la lista; da la vuelta en los dos extremos |
| `ENTER` | abrir el proyecto marcado |
| `N` | crear: se teclea un **id**, `[a-z0-9_]` |
| `D` | duplicar: se teclea un **nombre visible** y el id sale de `suggestId` |
| `X` | borrar: pregunta antes, y solo `ENTER` confirma |
| `ESC` | atrás / salir |

Duplicar enseña el informe de `ProjectIndex::duplicate()`: qué ficheros se
copiaron con el prefijo nuevo y cuáles se **comparten** por no llevar el
prefijo del original. Es la información que el autor necesita antes de
ponerse a editar la copia, y por eso `duplicate()` la devuelve en vez de
imprimirla.

Borrar quita el manifiesto y su entrada del índice, y **deja los niveles, los
mapas y los catálogos en disco**: pueden estar referenciados por otro
proyecto, y un borrado en cascada desde el editor es justo la clase de
operación que no se puede deshacer. Lo que no reclame nadie saldrá como
huérfano al revisar el índice.

---

## Comprobar

```bash
python3 tools/build_proyecto.py --todos    # qué packs están completos
./build/demo_proyectos                     # ProjectIndex + ProjectHub, sin GL
```

`demo_proyectos` recorre el ciclo entero: listar, revisar, crear, rechazar
ids inválidos y duplicados, borrar, y sacar una build de verdad. Termina
dejando `assets/` como lo encontró — una prueba que ensucia el árbol de
trabajo se paga en cada commit posterior (pasó: `prueba_tmp` acabó
versionado).

**Qué prueba ese demo y qué no.** Escribe sobre `assets/` real y lanza
`tools/build_proyecto.py`, así que `CMakeLists.txt` lo excluye de CTest a
propósito: no se ejecuta en ninguna pasada automática. Y su «borrar» llama a
`ProjectIndex::remove()` directamente, no pasa por la pantalla — que hasta
H4-01 no podía borrar. La pantalla (`ProjectHub`) se prueba en
`ctest -R test_project_hub`, sobre un `assetsRoot` temporal y sin
empaquetador.

## Regresiones de C-11

`test_project_manifests` carga todos los manifiestos reales a traves del
serializador y del indice, compara bytes al guardar y comprueba el punto
de entrada. Tambien recorre crear/borrar con un indice vacio y conserva
referencias a proyectos ausentes o invalidos. `test_project_migration`
comprueba la previsualizacion, campos conservados, errores e idempotencia.

```bash
ctest --test-dir build --output-on-failure -R 'test_project_(manifests|migration|serialization)'
```


## Los ficheros de ayer se siguen abriendo hoy (G-07, mecanismo)

Este formato se escribe a disco y se vuelve a leer más tarde. Nada en el
compilador impide que un cambio en el serializador deje ilegible lo que ya
hay escrito: no da error, no rompe ningún test, y el proyecto de alguien
simplemente deja de abrirse.

Contra eso hay ficheros **congelados** en `tests/fixtures/esquemas/`, que
`test_schema_fixtures` abre en cada ejecución y comprueba campo a campo —
no solo que parsee. También comprueba que volver a escribir lo leído da
exactamente el mismo texto: si el lector y el escritor dejaran de estar de
acuerdo, la diferencia acabaría siendo un campo perdido.

**Reglas al cambiar el formato:**

1. Si el cambio es **aditivo** (una clave nueva con valor por defecto), no
   se sube `schemaVersion` y la fixture antigua tiene que seguir
   abriéndose tal cual. Así entraron las capas de evento (E-01) y las
   variables narrativas (E-02).
2. Si el cambio **no es compatible**, se sube `schemaVersion`, se añade
   `…-vN.json` a las fixtures, **se deja la anterior** y se escribe aquí
   la ruta de migración. `test_schema_fixtures` falla hasta que se haga:
   ese es el punto.
3. Si una fixture vieja deja de abrirse, lo que se arregla es el lector o
   se escribe la migración. **Nunca la fixture.** Regenerarla para que
   pase convierte la prueba en un adorno.

Regenerar las fixtures es un acto deliberado y a mano, solo al añadir una
versión:

```bash
./test_schema_fixtures --escribir tests/fixtures/esquemas
```

Esto **no** es todavía la congelación de G-07: congelar los esquemas a la
1.0-B pide que los formatos estén terminados, y E-16 aún va a tocar el de
proyecto. Es el mecanismo que esa congelación necesitará.
