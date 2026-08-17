# Formato de proyecto

Un **proyecto** es un pack de contenido: sus mapas, sus niveles, sus
aventuras y sus catálogos de objetos. «Los Perdidos de Boundington» es un
proyecto; cada cuadrante del experimento de las cuatro IAs es otro.

Vive en `assets/proyectos/<id>.json`, y su id está listado en
`assets/proyectos/index.json`. Lo lee `Editor::ProjectIndex`.

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

Si un fichero no está en ningún manifiesto, el índice lo reporta como
**huérfano** en vez de repartirlo a ojo. Que sobre contenido es un aviso;
que se lo quede el proyecto equivocado es un bug silencioso.

---

## El manifiesto

```json
{
  "id": "boundington",
  "nombre": "Los Perdidos de Boundington",
  "descripcion": "La caída de la ciudad, tres días",
  "epoca": "1981 b.f.",
  "autor": "Oriol + Claude",
  "prefijo": "",
  "niveles":   ["ciudad_centro.json", "interior_vacio.json"],
  "mapas":     ["ciudad_centro.tmx"],
  "aventuras": ["boundington_prologo.json"],
  "catalogos": ["boundington_npcs.json"],
  "entrada":   "assets/adventures/boundington_prologo.json"
}
```

| Campo | Qué es |
|---|---|
| `id` | **Solo `[a-z0-9_]`.** Acaba siendo nombre de fichero *y* prefijo de ids |
| `nombre` | Lo que se ve en la pantalla de proyectos |
| `epoca` | Fecha del canon, p. ej. `"1981 b.f."`. El mundo está fechado |
| `prefijo` | **Debe acabar en `_`**. Sin él sale `mmtabernero`, que ni se lee ni se busca. Vacío = sin prefijo |
| `niveles`, `mapas`, `aventuras`, `catalogos` | Nombres de fichero **sin ruta**. Se resuelven contra `assets/levels/`, `assets/maps/`, `assets/adventures/` y `assets/objects/` |
| `entrada` | Por dónde arranca. Ver abajo |

### `entrada`: dos formas, las dos válidas

Se admiten las dos porque **las dos se escribieron de verdad** antes de
que esto estuviera documentado:

```json
"entrada": "assets/adventures/boundington_prologo.json"   // ruta desde la raíz
"entrada": "ciudad_en_guskedor.json"                       // solo el nombre
```

El nombre suelto se resuelve contra `assets/adventures/` y luego contra
`assets/levels/`. `Project::entryKind` deja anotado a qué resolvió
(`Adventure`, `Level`, `Missing` o `None`), y la pantalla de proyectos
muestra **la ruta resuelta**, no la que pone el manifiesto — para que un
`entrada` que no resuelve se vea en vez de fallar callando.

**Un proyecto puede arrancar en un nivel y no en una aventura.** Un pack
de escenario sin historia es legítimo: se abre y se camina por él. Sin
`entrada` el proyecto se puede editar pero no jugar, y el índice lo dice
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
