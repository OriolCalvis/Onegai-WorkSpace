# Estructura de la web y flujos de usuario

Documento vivo de referencia: qué existe, qué está a medias y qué falta. Se actualiza en cada pase de trabajo. Complementa `docs/Sistema_Cartas_Tiers.md` (reglas del sistema) con la arquitectura del sitio.

## 1. Mapa del sitio

```
Inicio (/)  →  dashboard con contadores y accesos directos
├── Personajes (/personatges)
│   ├── Listado y edición básica ................ Hecho
│   ├── Crear personaje (selecciona cartas ya creadas: clase, raza, trasfondo,
│   │   habilidades, equipo, dotes — nunca texto libre) ........ Hecho
│   └── Ficha con todas las cartas asignadas + botón "Imprimir" (impresión
│       del navegador con CSS @media print) .................... Hecho
├── Cartas (/cartas)  →  landing con resumen por tipo
│   ├── Clases (/cartas/clases) .................. Hecho — CRUD completo
│   ├── Razas (/cartas/razas) .................... Hecho — CRUD completo
│   ├── Trasfondos (/cartas/transfondos) ......... Hecho — CRUD completo
│   ├── Habilidades (/cartas/habilidades) ........ Hecho — CRUD completo, 10 cartas de ejemplo
│   ├── Armas (/cartas/armas) .................... Hecho — CRUD completo (armas, armaduras, escudos, accesorios), 12 cartas de ejemplo
│   └── Dotes (/cartas/dotes) .................... Hecho — CRUD completo, 6 cartas de ejemplo
├── Exportaciones (/exportacions) ................ Hecho — PDF / JSON de cartas y fichas
└── Configuración (/configuracio) ................ Hecho — ajustes generales

Legado (oculto del menú, solo lectura, sigue activo por URL directa):
/classes · /races · /transfons · /biblioteca
```

Notas:
- El sistema legado (DnD 5e clásico, seis stats, niveles) no se ha borrado. Solo se quitó de header/sidebar. Las rutas siguen respondiendo por si hace falta consultarlas.
- `/cartas/razas` (legado, no confundir con `/cartas/razas` nuevo) tenía un bug de plantilla ya documentado y con aviso en pantalla — no se ha tocado porque es contenido de solo lectura.

## 2. Estado por área

| Área | Modelo/Form | Repositorio | Servicio/Controlador | Plantillas | Nav |
|---|---|---|---|---|---|
| Clases | Hecho | Hecho | Hecho | Hecho | Hecho |
| Razas | Hecho | Hecho | Hecho | Hecho | Hecho |
| Trasfondos | Hecho | Hecho | Hecho | Hecho | Hecho |
| Habilidades | Hecho | Hecho | Hecho | Hecho | Hecho |
| Armas / armaduras / objetos | Hecho | Hecho | Hecho | Hecho | Hecho |
| Dotes | Hecho | Hecho | Hecho | Hecho | Hecho |
| Personajes: CRUD + selección de cartas | Hecho | Hecho (persiste en disco en `data/personatges/`) | Hecho | Hecho | Hecho |
| Ficha con cartas asignadas + imprimir (navegador) | — | — | Hecho | Hecho | Hecho |
| UX táctil (drag&drop, catálogo visual) | — | — | No iniciado | No iniciado | — |

> **Persistencia de personajes — IMPLEMENTADA.** `PersonatgeRepository` sigue el mismo patrón que
> el resto de catálogos (caché `LinkedHashMap` + lectura/escritura en disco con Jackson + carga en
> el constructor), así que los personajes sobreviven a reinicios. La versión anterior de este doc
> describía el estado previo (en memoria). Pendiente como **mejora de UX**, no como bloqueante:
> añadir un indicador visual "✓ guardado" tras crear/editar para que el usuario tenga feedback
> explícito de la persistencia (referencia: wireframe `3a:411`).

## 3. Flujos clave de usuario

**1. Crear o editar una carta** (Hecho, los 6 tipos: Clases, Razas, Trasfondos, Habilidades, Armas, Dotes)
Elegir tipo de carta → rellenar formulario → guardar (queda como JSON en `data/cartas/<tipo>`) → ver detalle.
Solo se hace aquí: el formulario de personaje nunca permite crear una carta nueva, solo elegirlas.

**2. Crear personaje seleccionando cartas** (Hecho)
Elegir raza, clase y trasfondo (dropdowns sobre el catálogo real) → marcar habilidades, armas/armaduras/objetos
y dotes disponibles (checkboxes sobre el catálogo real) → ajustar stats base CON/DES/INT/CAR (1-8) → guardar.
La vida no se edita a mano: se calcula sola (clase + CON final + tier + bonos de equipo).

**3. Ver e imprimir la ficha de un personaje** (Hecho)
Abrir personaje → ve la vida y stats finales calculados, más todas las cartas asignadas (clase, raza,
trasfondo, habilidades, armas/objetos, dotes) → botón "Imprimir ficha" que usa el diálogo de impresión
del navegador (CSS `@media print` oculta menú/sidebar/botones).

**4. Explorar el catálogo con experiencia visual tipo juego de cartas** (Pendiente, mejora de UX)
Filtrar/buscar cartas → seleccionar o arrastrar → feedback visual táctil en botones y cartas. Se aplicaría tanto a las listas de cartas existentes como a la pantalla de creación de personaje (flujo 2). Sigue usando Thymeleaf + una capa de JavaScript moderno, sin reescribir a SPA.

## 4. Orden de trabajo

1. ~~Terminar Dotes + las 3 plantillas Thymeleaf pendientes (Habilidades, Armas, Dotes) + nav.~~ Hecho.
2. ~~Creación de personaje seleccionando cartas reales (flujo 2).~~ Hecho.
3. ~~Ficha de personaje con todas sus cartas + vista imprimible (flujo 3).~~ Hecho.
4. ~~Persistir personajes en disco (`data/personatges/`) igual que las cartas, para que no se pierdan al reiniciar.~~ Hecho.
5. Rediseño visual/UX táctil sobre catálogos y creación de personaje (flujo 4).
6. Indicador visual "✓ guardado" tras crear/editar personaje (feedback explícito de la persistencia).
7. Balance real de las cartas (de momento el objetivo era que todo funcionase end-to-end; los 28 cartas
   de ejemplo de habilidades/armas/dotes son un punto de partida razonable, no un pase de balance).
