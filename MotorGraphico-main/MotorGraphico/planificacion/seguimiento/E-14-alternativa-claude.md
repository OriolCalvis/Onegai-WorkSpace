# E-14 — implementación alternativa (Claude, Cowork) · 17/09/2026

**Estado: aportación, no integrada.** Reclamé E-14 en `TABLERO.md` a las 09:38 y
lo programé en una copia aislada. A las 09:44 otra sesión, sin fila en el
tablero, hizo commit de `6a3be66` («E-14») en las mismas rutas, y a las 09:48
seguía editando `CatalogPickerState.{h,cpp}` y su cmake; su E-16 en curso ya
incluye la cabecera. Por la regla 2 no escribo en esas rutas. Oriol decidió:
dejar E-14 a esa sesión y dejar aquí esta versión para que la aproveche.

## Qué hay en el parche

`E-14-alternativa-claude.patch` (contra `6a3be66`, se aplica con
`patch -p1` desde la raíz del repo; comprobado con `--dry-run`). Sustituye los
cuatro ficheros de `6a3be66`:

- `include/Editor/CatalogPickerState.h`, `src/Editor/CatalogPickerState.cpp`
  (namespace `Editor`, junto a `ProblemsState`):
  - `PickerIndex`: entradas de objetos, habilidades, monstruos, estados, mapas y
    eventos (con el mapa como ámbito). Rechaza id vacío y id repetido
    **sin pisar** la primera entrada, y dice en qué dos ficheros chocan.
    Plegado de búsqueda precalculado al añadir y búsqueda por clave O(1).
  - Adaptadores plantilla: `AddFromIdCatalog` (ObjectCatalog) y
    `AddFromRpgCatalog` (cualquier `RPG::Catalogs::Catalog<T>`), y
    `AddEventsFromMap`. No atan el selector a motor_level/motor_rpg.
  - `CheckReference`: Empty / Valid / Missing / WrongKind, con mensaje que dice
    qué era («"lobo" es monstruo…; aquí se espera un objeto»; evento de otro mapa).
  - `FindLostReferences(MapDocument, PickerIndex)`: objetos, mapas y eventos
    rotos con su `ProblemLocation` de E-15 (el «Ir ahí» resuelve al comando exacto).
  - `CatalogPickerState`: búsqueda por palabras con `NormalizeForSearch` (el
    mismo plegado de E-15, DEC-15), filtro por origen proyecto/importado,
    selección por identidad que sobrevive a filtros y recargas, referencia
    perdida conservada al abrir (no se sustituye por la primera fila),
    etiquetas «Nombre (id)» cuando el nombre se repite, `accept()` que sólo
    devuelve ids válidos, `offer()` que rechaza tipos incompatibles,
    teclado, y «Abrir definición» (lo importado sale como sólo lectura).
  - `PickerKindForParam(CommandParamType)`: qué selector abre cada parámetro
    del catálogo de comandas de E-04 (para E-06).
- `tests/Editor/CatalogPickerTests.cpp`: 8 casos. Los cuatro de la prueba del
  plan + refresco, duplicados, teclado y «botín y destino de puerta por nombre»
  con `ObjectCatalog` y `EquipmentCatalog` reales detrás de los adaptadores.
- `cmake/CatalogPickerTests.cmake`: compila además `ProblemsState`,
  `AdventureValidation` y `EventCommands` (mismo reparto que
  `ProblemsStateTests.cmake`) y enlaza `motor_level`, que desde `6a3be66` ya
  contiene `CatalogPickerState.cpp`. `motor_level` es estática: el objeto sólo
  se enlaza donde se usa, así que no añade dependencias a otros ejecutables.

## Verificación (VM local, sin cmake: g++ 11 con la lista de fuentes de las librerías)

- 8/8 contra `6a3be66` en copia aislada y 8/8 contra el árbol vivo con el WIP ajeno.
- 8/8 con ASan+UBSan (`detect_leaks=1`), sin diagnósticos.
- Falsabilidad, 6 mutaciones, las 6 detectadas: sin desambiguar etiquetas,
  sin plegar la búsqueda, `select` que acepta cualquier id, abrir que elige la
  primera fila, `FindLostReferences` vacío, id repetido que pisa.
- **No verificado:** configuración CMake real ni CI (la VM no tiene cmake ni red).

## Medición con los assets reales (sonda, no versionada)

Indexando `assets/catalogs/{skills,spells,monsters,conditions,equipment,consumables}.json`
y **todos** los `assets/objects/*.json` a la vez: 4583 entradas y 2574 choques de
id. Casi todos son `libreria_completa.json` (2112) y `libreria_armas.json` (413),
que repiten lo mismo que `equipment.json`; más 36 en `mundi_lugares.json`, 10 en
`ciudad_objetos.json` y `pocion`/`eter` en `este_norte_objetos.json` y
`test_objects.json`. Conclusión para E-16/E-11: el índice del selector se tiene
que construir con **los catálogos que declara el proyecto**, no con la carpeta
entera. Rendimiento: 12 pulsaciones sobre 3433 objetos en 2 ms; `refresh`, 1,5 ms.

## Qué no cubre

- El panel ImGui (`editor/src/CatalogPicker.cpp`) y la integración en
  `ObjectInspector` y en los formularios de E-05/E-06.
- `CommandParamType` no tiene `SkillRef`/`MonsterRef`/`ConditionRef`: hasta que
  E-04 los añada, esos selectores sólo se pueden usar desde inspectores.
- Carga de los catálogos del proyecto (lista de catálogos declarados): E-16/E-11.
