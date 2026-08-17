# Mapa Mundi — Documento de requisitos

*Página `/mapa` de dndWeebCC. Documento de requisitos funcionales y técnicos generado a
partir de la sesión de análisis del 2026-07-16 con Oriol. Complementa
`docs/Sistema_Cartas_Tiers.md` (reglas del sistema) y `docs/Arquitectura_Datos_Onegai.md`
(modelo de datos del universo del juego).*

> **Estado del arte actual.** La página ya está considerablemente avanzada: 19 naciones
> con polígonos, 16 facciones jugables, 95 ciudades, 26 zonas, editor completo con
> arrastre de vértices, creación de puntos/ciudades/zonas, guardado a
> `data/mapa/geografia.json`, filtros por tier y por leyenda, cronología y paneles
> laterales. Este documento **no parte de cero**: define cómo reorganizar, modularizar
> y ampliar lo que ya existe.

---

## Índice

1. [Propósito y principio rector](#1-propósito-y-principio-rector)
2. [Estado actual (inventario honesto)](#2-estado-actual-inventario-honesto)
3. [Objetivos de esta iteración](#3-objetivos-de-esta-iteración)
4. [Arquitectura técnica objetivo](#4-arquitectura-técnica-objetivo)
5. [Requisitos funcionales — Lectura](#5-requisitos-funcionales--lectura)
6. [Requisitos funcionales — Edición](#6-requisitos-funcionales--edición)
7. [Requisitos funcionales — Funciones nuevas](#7-requisitos-funcionales--funciones-nuevas)
8. [Integración con el catálogo](#8-integración-con-el-catálogo)
9. [Modelo de datos](#9-modelo-de-datos)
10. [Flujos de usuario](#10-flujos-de-usuario)
11. [Estructura de archivos objetivo](#11-estructura-de-archivos-objetivo)
12. [Criterios de aceptación](#12-criterios-de-aceptación)
13. [Decisiones diferidas](#13-decisiones-diferidas)

---

## 1. Propósito y principio rector

**El mapa mundi es la herramienta de worldbuilding del director.**

No es una vista de jugador, ni un mapa táctico de combate, ni un tablero de mesa en
vivo. Es el lienzo donde el director **construye y mantiene el mundo**: naciones,
ciudades, zonas del terreno, eventos históricos y sus relaciones con el catálogo de
cartas (historias, aventuras, PNJs, monstruos, deidades, razas).

### Principio rector

> **Todo lo geográfico es editable desde el propio mapa, sin tocar código ni JSON a
> mano.** El director hace un cambio en el navegador, pulsa "Guardar cambios", y queda
> persistido en `data/mapa/geografia.json`. El código Java solo lee y escribe ese
> archivo; no contiene datos geográficos propios (salvo metadatos puramente visuales
> como el icono de una facción, que no son mundo).

Esto ya es cierto hoy para naciones, ciudades, zonas y eventos. Esta iteración lo
extiende a las nuevas funciones manteniendo el mismo principio.

---

## 2. Estado actual (inventario honesto)

### Lo que ya funciona y se mantiene

| Elemento | Estado | Cantidad |
|---|---|---:|
| Naciones (polígonos SVG con coordenadas exactas) | ✅ Editable | 19 |
| Facciones jugables (marcadores + paneles) | ✅ Solo lectura | 16 |
| Ciudades (capital/ciudad/pueblo/aldea) | ✅ Editable | 95 |
| Zonas del terreno (montaña/bosque/ruina/...) | ✅ Editable | 26 |
| Puntos personalizados | ✅ Editable | 0 |
| Eventos históricos (cronología) | ✅ Editable | 0 |
| Filtro por tier | ✅ Funciona | — |
| Leyenda clicable naciones/facciones | ✅ Funciona (ocultar) | — |
| Arrastre de vértices de nación | ✅ Funciona | — |
| Doble-clic añadir/quitar vértice | ✅ Funciona | — |
| Guardado a `geografia.json` | ✅ Funciona | — |
| Borrador en `localStorage` | ✅ Funciona | — |
| Copiar coordenadas (backup manual) | ✅ Funciona | — |

### Lo que hay que reorganizar (deuda técnica)

| Problema | Gravedad | Detalle |
|---|---|---|
| **JS todo inline** en `mapa/index.html` | 🔴 Alta | ~2389 líneas en un único `<script>` gigante que mezcla filtros, arrastre, creación, diálogos, paneles y guardado. Ilegible, no reutilizable, difícil de mantener. |
| **Sin modularidad** | 🔴 Alta | Cada nueva función (zoom, línea temporal, modales) añadiría cientos de líneas más al mismo bloque. Hay que partirlo en archivos por responsabilidad. |
| **Leyenda limitada** | 🟡 Media | Hoy solo permite ocultar/mostrar naciones y facciones. No filtra por foco ni por capas. |
| **Sin buscador global** | 🟡 Media | No hay forma de buscar "Ecla" y ver todo lo relacionado. |
| **Sin zoom por nación** | 🟡 Media | No se puede entrar al detalle de una nación. |
| **Sin línea temporal** | 🟡 Media | La cronología existe pero no afecta visualmente al mapa. |

---

## 3. Objetivos de esta iteración

Por orden de prioridad, acordados con el usuario:

1. **Limpiar y seccionar** el código del mapa (JS inline → archivos separados, CSS
   reutilizable, legibilidad). Sin funciones nuevas en este paso.
2. **Mejorar el editor**: simplificar la creación/edición en **modales emergentes por
   entidad**, cada uno en su propio archivo JS, ordenados y mantenibles.
3. **Leyenda interactiva ampliada**: foco por nación + capas por tipo + buscador global.
4. **Integración con el catálogo**: conectar más el mapa con historias, aventuras, PNJs,
   monstruos, deidades y razas (vinculación bidireccional visible).
5. **Funciones nuevas**: línea temporal y zoom por nación (vista detallada).

---

## 4. Arquitectura técnica objetivo

### Stack (sin cambios)

- **Backend**: Spring Boot + Java + Thymeleaf (paquete `cat.dnd.cc`).
- **Persistencia**: JSON plano en `data/mapa/geografia.json`.
  > *Anotación para el futuro: migración a base de datos embebida (H2/SQLite) queda
  > registrada como deuda técnica global del proyecto, no de esta página. Mientras
  > tanto, el director sigue editando JSON a su gusto.*
- **Frontend**: HTML + CSS (tokens/base/components) + **JavaScript vanilla
  modularizado** (sin framework). No se reescribe a SPA.

### Principios de organización del código frontend

1. **Un archivo JS por responsabilidad**, cargados en orden desde el HTML, todos bajo
   `static/js/mapa/`. Ningún `<script>` inline con lógica (solo los mínimos selectores
   de arranque).
2. **Un módulo CSS dedicado** `static/css/mapa.css` (ya existe), ampliado con clases
   reutilizables para evitar repetir estilos inline.
3. **Comunicación entre módulos** vía un objeto `window.GMMapa` compartido (espacio de
   nombres único del mapa) o eventos `CustomEvent` (`gm:panel-opened`, `gm:nacion-foco`,
   `gm:capa-toggle`). Evitar acoplamiento directo entre archivos.
4. **Plantilla base**: la página `mapa/index.html` se reduce a la estructura HTML
   semántica + inclusión de los módulos JS. Toda la lógica vive fuera.

### Separación backend / frontend

- **El backend (`MapaController`, `GeografiaMapaService`) sigue siendo delgado**: lee
  `geografia.json`, lo pasa al modelo, y expone `POST /mapa/guardar`. No asume lógica de
  filtros, zoom ni línea temporal — todo eso es frontend (los datos ya están en el
  modelo, el JS los filtra en cliente).
- **El frontend es el dueño de la interacción**: filtros, foco, zoom, modales, línea
  temporal. El backend no sabe qué nación está "en foco".

---

## 5. Requisitos funcionales — Lectura

### 5.1 Vista general del mapa (la que ya existe)

- Imagen `mapamundi.png` (1600×1200) como fondo del SVG.
- Polígonos de las 19 naciones dibujados con las coordenadas de `geografia.json`.
- Marcadores de facciones, ciudades, zonas y puntos superpuestos.
- Marco doble decorativo "look de mapa antiguo".
- Leyenda de naciones (con swatch de color) y de facciones (con icono).

### 5.2 Leyenda interactiva ampliada (NUEVO)

La leyenda pasa de ser un simple "ocultar/mostrar" a un panel de control con tres
modos de filtro, combinables entre sí:

**5.2.1 Foco por nación.**
- Clic en una nación (polígono o entrada de la leyenda) activa el **modo foco** de esa
  nación.
- En modo foco: la nación seleccionada se ve a brillo completo, el resto del mapa se
  atenúa (opacidad reducida, no oculto del todo — sigue siendo contexto).
- Aparece un panel lateral o dropdown con **todo lo de esa nación agrupado**:
  ciudades (ordenadas por tamaño), zonas del terreno, facciones presentes, deidades
  veneradas, razas predominantes, historias que ocurren allí, aventuras que la tocan,
  PNJs que viven en sus ciudades, eventos.
- Un botón "Salir del foco" devuelve a la vista general.
- *Relación con el zoom (5.5)*: el foco puede, opcionalmente, disparar el zoom a esa
  nación (decisión del usuario: ¿foco + zoom automáticos juntos, o separados?). Ver
  decisión diferida 13.1.

**5.2.2 Capas por tipo (toggle GIS).**
- Una fila de toggles, uno por tipo de elemento:
  - 🏰 Naciones (polígonos)
  - 📍 Facciones
  - 🏙 Ciudades
  - ⛰ Zonas del terreno
  - 📌 Puntos personalizados
  - 📜 Eventos históricos (cronología)
- Clic en un toggle oculta/muestra **todos** los elementos de ese tipo en el mapa.
- Independiente del foco por nación: se pueden combinar (p. ej. "foco en Ecla" + "solo
  ciudades y PNJs visibles").
- Estado de las capas se recuerda en `localStorage` entre sesiones.

**5.2.3 Buscador global.**
- Campo de texto en la cabecera del mapa.
- Al escribir, busca coincidencias en: nombres de naciones, ciudades, zonas, puntos,
  facciones, eventos históricos, **y elementos del catálogo** (historias, aventuras,
  PNJs, monstruos, deidades, razas) que estén vinculados a algo geográfico.
- Los resultados se muestran como lista desplegable; al elegir uno, el mapa hace foco
  + (opcional) zoom en la nación/zona donde vive ese elemento, y abre su panel.
- Ejemplo: escribir "Ecla" → aparece la nación Ecla, sus 8 ciudades, sus PNJs, sus
  historias. Elegir "Ecla" → foco + zoom a Ecla.
- Ejemplo: escribir "Rei Llop" → aparece la facción Manada del Rei Llop y la aventura
  asociada. Elegir → zoom a la región + panel de la facción abierto.

### 5.3 Filtros heredados (mantener)

- **Filtro por tier** (botones 1-5 + "Todos"): ya existe hoy, se mantiene y se amplía
  para que también aplique a ciudades y zonas (no solo a facciones y puntos).
- **Leyenda clicable simple** (ocultar una nación/facción concreta): se mantiene como
  acción rápida, coexiste con el foco del 5.2.1 (clic = foco, clic con modificador o
  icono de ojo = ocultar; o botón explícito "ocultar" en la entrada de leyenda).

### 5.4 Zoom general del lienzo

- El SVG debe ser **navegable**: zoom con rueda del ratón / botones +/-, y paneo
  (arrastrar el lienzo, no un vértice) cuando hay zoom aplicado.
- Hoy el SVG es estático a tamaño completo; hay que añadir capacidad de zoom/pan
  general sin romper el editor de vértices (que debe seguir funcionando con el zoom
  activo: las coordenadas se calculan contra el SVG, no contra la pantalla).

### 5.5 Zoom por nación / vista detallada (NUEVO)

- Acción explícita "Entrar en esta nación" (botón en el panel de la nación o doble-clic
  en su polígono fuera de modo edición).
- El lienzo hace zoom y centra la nación para que ocupe la mayor parte del viewport.
- En la vista detallada se muestra con **más detalle** que en la general:
  - Todas las ciudades de la nación con su etiqueta completa (no truncada).
  - Todas las zonas del terreno dentro de su territorio (point-in-polygon).
  - Las rutas/caminos entre ciudades (si 7.1 se implementa).
  - Un panel lateral fijo con la ficha completa de la nación: cultura, alineación,
    deidades, razas, curiosidades, cronología local (eventos cuyo lugar es esta
    nación).
- Botón "Volver a la vista general" (o tecla Escape).

---

## 6. Requisitos funcionales — Edición

### 6.1 Principio: modales por entidad

Toda creación y edición de entidades geográficas se hace a través de **modales
emergentes**, uno por tipo de entidad, **cada uno en su propio archivo JS**. Se
abren desde:

- La leyenda (botón "editar" en una nación).
- El panel lateral de un elemento (botón "editar").
- Botones de la barra de herramientas ("➕ Nueva ciudad", "➕ Nueva zona"...).

Ventajas frente a los `prompt()` encadenados de hoy: validación real, campos
estructurados, selección de listas (no texto libre), y un archivo por modal → mantenible.

### 6.2 Modal de Nación (`mapa-modal-nacion.js`)

Campos editables:

- **Nombre** (texto).
- **Color** (selector de color — es lo que pinta su polígono y su swatch en la leyenda).
- **Vértices del polígono** (se editan arrastrando en el lienzo, como hoy; el modal no
  los edita como números, solo informa de cuántos hay).
- **Cultura** (texto libre narrativo).
- **Alineación** (texto libre, p. ej. "neutral", "caótica buena").
- **Curiosidades** (texto libre narrativo).
- **Deidades veneradas** (multiselector del catálogo `TierDeity` — ids, nunca texto
  libre; ya llega al modelo como `deidadesResumen`).
- **Razas predominantes** (multiselector del catálogo `TierRace` — ids; ya llega como
  `razasResumen`).

### 6.3 Modal de Ciudad (`mapa-modal-ciudad.js`)

Campos:

- **Nombre** (texto).
- **Nación** (selector de las 19 naciones — no texto libre).
- **Tamaño** (capital | ciudad | pueblo | aldea — selector).
- **Población** (texto libre, "~12.000", "unas 300 almas").
- **Gobernante** (texto libre).
- **Rasgo distintivo** (texto libre, "puerto de contrabando").
- **Descripción** (texto libre narrativo).
- **Historias que ocurren aquí** (multiselector del catálogo `Historia` — buscador por
  título, como el editor de puntos actual).
- **PNJs de la ciudad** (multiselector del catálogo `NPC` — buscador por nombre).

La posición (x, y) se elige clicando en el lienzo tras crear, no en el modal.

### 6.4 Modal de Zona (`mapa-modal-zona.js`)

Campos:

- **Nombre** (texto).
- **Tipo** (montaña | bosque | lago | río | desierto | pantano | ruina | fortaleza |
  santuario | puerto | isla | paso — selector; determina el icono).
- **Nación** (selector; opcional — una zona puede ser "salvaje" sin nación).
- **Peligro** (texto libre de mesa, "nidos de arpía", "avalanchas en deshielo").
- **Descripción** (texto libre narrativo).
- **Historias que ocurren aquí** (multiselect de `Historia`).

### 6.5 Modal de Evento histórico (`mapa-modal-evento.js`)

Campos (ya definidos en `GeografiaMapa.EventoHistorico`):

- **Título** (texto).
- **Año** (numérico).
- **Era** (texto, opcional — p. ej. "Era de la Forja", "Antes de la Caída").
- **Lugar** (selector con datalist de naciones + ciudades + zonas — no texto libre puro).
- **Descripción** (texto libre de qué pasó).
- **Consecuencias** (texto libre de qué quedó jugable hoy).

### 6.6 Modal de Punto personalizado (`mapa-modal-punto.js`)

Campos (ya definidos en `GeografiaMapa.Punto`):

- **Nombre** (texto).
- **Icono** (selector de emoji).
- **Historias vinculadas** (multiselect de `Historia`).
- **Aventuras vinculadas** (multiselect de `Aventura`).
- **Eventos vinculados** (multiselect de `Evento`).

### 6.7 Comportamiento común de los modales

- **Un único sistema de modales** reutilizable (envoltorio HTML + función
  `abrirModal(titulo, campos, valoresIniciales, onGuardar)`), ya existe un boceto
  funcional hoy (`#gm-dialogo`) — se promueve a su propio módulo
  `mapa-modal-base.js` y lo usan los 5 modales anteriores.
- **Validación**: campos obligatorios marcados, bloqueo del botón "Guardar" hasta que
  sean válidos.
- **Cancelación**: botón "Cancelar" + clic fuera + tecla Escape, sin perder cambios no
  guardados en el resto del editor.
- **Guardado**: el modal actualiza el estado en memoria + `saveDraft()` a
  `localStorage`. El botón global "💾 Guardar cambios" de la barra de herramientas es
  lo único que escribe en el servidor (`POST /mapa/guardar`), como hoy.
- **Borrado**: cada modal de edición incluye un botón "🗑 Eliminar" con confirmación.

### 6.8 Barra de herramientas reorganizada

La toolbar actual tiene 8 botones mezclados. Se reorganiza en grupos visuales:

```
[Modo edición] | [➕ Nuevo: ciudad ▾] | [💾 Guardar] [📋 Copiar] [↩️ Restablecer] | [x, y]
```

- El desplegable "➕ Nuevo: ▾" agrupa Nueva ciudad / Nueva zona / Nuevo punto / Nuevo
  evento histórico, en vez de 4 botones sueltos.
- "Modo edición" se queda como toggle principal (sigue siendo necesario para arrastrar
  vértices y posicionar elementos).

---

## 7. Requisitos funcionales — Funciones nuevas

### 7.1 Línea temporal del mundo (NUEVO)

- Un **control deslizante** (slider) en la parte inferior del mapa, con marcas en los
  años de los eventos históricos.
- Al mover el slider, el mapa refleja el **estado del mundo en ese año**:
  - Ciudades/naciones que aún no existían según la cronología se atenúan o se ocultan
    (según decisión 13.2: ¿atenuar o ocultar?).
  - Los eventos posteriores al año seleccionado no aparecen en la cronología visible.
  - *(Fase 2, opcional)* Banderas manuales "esta ciudad existe desde el año X" /
    "esta nación se fundó en el año Y" para que la línea temporal sepa qué mostrar.
- Es **una capa de visualización**: no modifica `geografia.json`, solo filtra lo que se
  ve según el año seleccionado. Volver el slider al final = estado actual del mundo.
- Se apaga/desactiva si no hay eventos históricos (hoy hay 0 — la línea temporal
  aparecerá vacía hasta que el director escriba cronología, lo cual es coherente: el
  motor está listo para cuando haya contenido).

### 7.2 Vista detallada por nación (NUEVO)

Ver sección 5.5. Es la función de "zoom a una nación con su todo mucho más
detallado" que pidió el usuario explícitamente.

### 7.3 Funciones descartadas por ahora (registradas para el futuro)

- **Rutas y distancias** entre ciudades con cálculo de tiempo de viaje — descartado en
  esta iteración; requeriría definir la red de caminos.
- **Niebla de guerra** para sesiones en vivo — descartado; el mapa es herramienta de
  director, no de mesa en vivo (ver principio rector de la sección 1). Queda apuntado
  para si en el futuro se separa una "vista de jugador".

---

## 8. Integración con el catálogo

### 8.1 Principio

Cada elemento geográfico puede **referenciar cartas del catálogo por id**, y el panel
del elemento **resuelve y muestra** esas referencias como enlaces clicables. Hoy ya
ocurre parcialmente (historias, aventuras, PNJs, monstruos, tesoros en facciones).
Esta iteración lo hace consistente en toda la página.

### 8.2 Matriz de integración objetivo

| Elemento geográfico | Referencia a catálogo | Dónde se ve |
|---|---|---|
| Nación | `deidadIds`, `razaIds` | Panel de nación + vista detallada |
| Facción | Historias, aventuras, PNJs, monstruos, tesoros, eventos (derivados) | Panel de facción (ya existe) |
| Ciudad | `historiaIds`, `npcIds` | Panel de ciudad + vista detallada de nación |
| Zona | `historiaIds` | Panel de zona |
| Punto | `historiaIds`, `aventuraIds`, `eventoIds` | Panel de punto |
| Evento histórico | `lugar` (texto con datalist de naciones/ciudades/zonas) | Cronología |

### 8.3 Bidireccionalidad visible (mejora)

Hoy la integración es **unidireccional**: el mapa sabe qué historias hay en una región,
pero la ficha de una historia no dice "ocurre en el mapa aquí". Esta iteración no
cambia el modelo de la historia (eso es otra página), pero sí:

- En el **buscador global** (5.2.3), buscar el título de una historia devuelve el
  punto/nación/facción donde ocurre, y abre su panel. Así, sin tocar la ficha de la
  historia, el director puede saltar del catálogo al mapa.

### 8.4 Lo que el backend ya provee (no hace falta añadir)

`MapaController.mapa()` ya pasa al modelo, listo para consumir:

- `regiones` (facciones con historias/monstruos/tesoros/pnjs/aventuras/eventos resueltos)
- `naciones`, `ciudades`, `zonas`, `puntos`, `cronologia`
- `historiasResumen`, `aventurasResumen`, `eventosResumen`, `npcsResumen`
- `deidadesResumen`, `razasResumen`
- `tiersDisponibles`

Los modales y buscadores consumen estos resúmenes vía los `<datalist>` ya presentes
en la página (patrón existente y bueno: una sola lista en el DOM, no una copia por
elemento). **No hace falta mandar más datos al navegador** para implementar esta
iteración — solo reorganizar el JS que ya los consume.

---

## 9. Modelo de datos

### 9.1 Sin cambios estructurales

`GeografiaMapa` (`GeografiaMapa.java`) ya define todo lo necesario:

- `Nacion` (nombre, color, points, cultura, alineacion, curiosidades, deidadIds, razaIds)
- `Ciudad` (id, nombre, nacion, tamano, poblacion, gobernante, rasgo, descripcion, x, y, historiaIds, npcIds)
- `Zona` (id, nombre, tipo, nacion, peligro, descripcion, x, y, historiaIds)
- `Punto` (id, nombre, icono, x, y, historiaIds, aventuraIds, eventoIds)
- `EventoHistorico` (id, titulo, ano, era, lugar, descripcion, consecuencias)
- `marcadores` (facción → [x, y])

**Esta iteración no añade campos al modelo.** Toda la funcionalidad nueva (foco,
capas, buscador, zoom, línea temporal) es **capa de presentación** sobre los datos que
ya existen.

### 9.2 Posible adición mínima (decisión diferida 13.3)

Para que la línea temporal (7.1) pueda ocultar ciudades/naciones por fecha, haría falta
un campo opcional `anoFundacion` / `anoDisolucion` en `Ciudad` y `Nacion`. Si se quiere
esa función en su versión completa, hay que añadirlos. Si basta con filtrar solo por
eventos históricos (sin ocultar ciudades), no hace falta tocar el modelo. Decisión
pendiente — ver 13.3.

---

## 10. Flujos de usuario

### Flujo A — Explorar el mundo (sin editar)

1. Director entra a `/mapa`.
2. Ve el mapa general con todo visible.
3. Escribe "Bastrea" en el buscador → aparece la nación y sus ciudades.
4. Clica "Bastrea" → la nación entra en foco (se atenúa el resto) y opcionalmente hace
   zoom (según 13.1).
5. En el panel lateral ve: ciudades de Bastrea ordenadas por tamaño, zonas, deidades,
   razas, historias.
6. Clica una ciudad → abre su panel con PNJs y historias.
7. Pulsa Escape → vuelve a la vista general.

### Flujo B — Crear una ciudad nueva

1. Director activa "Modo edición".
2. Despliega "➕ Nuevo: ▾" → "Ciudad".
3. Clica en el lienzo donde quiere la ciudad → se abre el modal de ciudad.
4. Rellena: nombre "Puerto de Mijorn", nación "Bastrea" (selector), tamaño "pueblo",
   rasgo "puerto pesquero".
5. Vincula un PNJ del catálogo buscándolo por nombre.
6. "Guardar" → el modal cierra, la ciudad aparece en el lienzo, `localStorage` guarda
   el borrador.
7. Pulsa "💾 Guardar cambios" → se escribe en `geografia.json`.

### Flujo C — Editar la identidad de una nación

1. Modo edición ON.
2. Doble-clic en el polígono de "Ecla" (o botón "editar" en su entrada de leyenda) →
   abre el modal de nación.
3. Cambia su color, añade una deidad del catálogo, escribe su cultura.
4. "Guardar" → el polígono recolorea, la leyenda actualiza su swatch, el panel de foco
   muestra la nueva deidad.

### Flujo D — Ver el mundo en el pasado (línea temporal)

1. Director crea 3 eventos históricos (años 100, 250, 400) desde el modal de evento.
2. Guarda cambios.
3. Desplaza el slider de la línea temporal al año 200.
4. El mapa atenúa/oculta lo posterior a 200 (según 13.2) y la cronología solo muestra
   el evento del 100.
5. Desplaza al 500 → todo visible de nuevo.

---

## 11. Estructura de archivos objetivo

### 11.1 JavaScript (lo importante de esta iteración)

```
src/main/resources/static/js/
├── theme.js                    (existe — tema claro/oscuro)
├── panels.js                   (existe — sistema de paneles laterales, reutilizable)
├── selector-csv.js             (existe — otra página)
└── mapa/                       (NUEVO — todo el JS del mapa, hoy inline)
    ├── mapa-main.js            Arranque: orquesta carga, cablea eventos globales.
    ├── mapa-estado.js          Estado compartido (foco, capas activas, tier ocultos,
    │                           zoom, año de la línea temporal). Espacio de nombres
    │                           window.GMMapa. Emite CustomEvents al cambiar.
    ├── mapa-lienzo.js          Zoom/pan general del SVG + conversión de coordenadas
    │                           (toSvgPoint, que hoy vive inline).
    ├── mapa-filtros.js         Filtro por tier + leyenda clicable (lo que hoy está
    │                           inline, extraído y ampliado con capas 5.2.2).
    ├── mapa-foco.js            Foco por nación (5.2.1): atenúa el resto, muestra panel.
    ├── mapa-buscador.js        Buscador global (5.2.3).
    ├── mapa-zoom-nacion.js     Vista detallada por nación (5.5).
    ├── mapa-linea-temporal.js  Slider de año + filtrado por fecha (7.1).
    ├── mapa-vertices.js        Editor de polígonos: arrastrar/añadir/quitar vértices
    │                           (lo que hoy está inline, extraído).
    ├── mapa-colocacion.js      Colocar nuevas ciudades/zonas/puntos con clic +
    │                           generación aleatoria (Alt+clic).
    ├── mapa-modal-base.js      Sistema de modales reutilizable (promueve #gm-dialogo).
    ├── mapa-modal-nacion.js    Modal de nación (6.2).
    ├── mapa-modal-ciudad.js    Modal de ciudad (6.3).
    ├── mapa-modal-zona.js      Modal de zona (6.4).
    ├── mapa-modal-evento.js    Modal de evento histórico (6.5).
    ├── mapa-modal-punto.js     Modal de punto personalizado (6.6).
    └── mapa-guardado.js        saveDraft() a localStorage + POST /mapa/guardar +
                                copiar coordenadas + banner de borrador pendiente.
```

**Regla**: cada archivo exporta su inicialización como función y se llama desde
`mapa-main.js` en orden. Ningún archivo supera ~300 líneas; si crece, se parte.

### 11.2 CSS

```
src/main/resources/static/css/
├── mapa.css                    (existe — ampliar con clases reutilizables)
└── (en mapa.css, añadir:)
    .gm-page-header             Cabecera de página reutilizable (sustituye el flex
    .gm-page-actions            inline repetido en 20+ plantillas — ver documento
                                de estructura HTML).
    .gm-map-toolbar             Barra de herramientas agrupada (6.8).
    .gm-map-layer-toggle        Botón de capa GIS (5.2.2).
    .gm-map-search              Buscador global + dropdown de resultados.
    .gm-map-timeline            Slider de línea temporal.
    .gm-modal                   Sistema de modales (6.7).
    .gm-modal__field            Campo de formulario de modal.
```

### 11.3 HTML

`templates/mapa/index.html` se reduce drásticamente: de ~2389 líneas (con el JS inline)
a la estructura HTML semántica (~400-500 líneas) + las inclusiones `<script src>` de
los módulos en orden. Los `<template>` de paneles se quedan en el HTML (son datos para
Thymeleaf, no lógica).

---

## 12. Criterios de aceptación

La iteración se considera completa cuando:

- [x] `mapa/index.html` no contiene ningún `<script>` con lógica; solo includes.
      *(Hecho: de 2.589 líneas con 2.073 de JS inline a 532 líneas de HTML semántico
      + 13 includes.)*
- [x] Cada archivo de `static/js/mapa/` tiene una responsabilidad única y < ~300 líneas.
      *(Hecho: 13 módulos, el mayor de 302 líneas, con el estado compartido en el
      espacio de nombres `window.GMMapa` — ver `mapa-nucleo.js`.)*
- [ ] El editor funciona idéntico a hoy tras la refactorización (arrastre, crear
      ciudades/zonas/puntos/eventos, guardar) — **sin regresiones**.
- [ ] Los 5 modales (nación, ciudad, zona, evento, punto) abren, validan, guardan y
      cancelan correctamente, cada uno desde su propio archivo JS.
- [ ] El foco por nación atenúa el resto y muestra el panel agrupado.
- [ ] Las capas por tipo (toggle GIS) ocultan/muestran elementos correctamente.
- [ ] El buscador global devuelve resultados del catálogo y salta al foco.
- [ ] El zoom por nación muestra la vista detallada y se sale con Escape.
- [ ] La línea temporal filtra lo visible por año (mínimo: eventos; ver 13.3).
- [ ] El zoom/pan general del lienzo funciona sin romper el editor de vértices.
- [ ] La integración con el catálogo (sección 8) es consistente en todos los paneles.
- [ ] No se ha añadido CSS inline nuevo; todo va a clases de `mapa.css` o `components.css`.

---

## 13. Decisiones diferidas

Preguntas que dejamos abiertas para resolver cuando llegue el momento de implementar
cada parte. No bloquean la documentación.

### 13.1 ¿Foco y zoom por nación van juntos o separados?

- **Opción A**: al hacer foco en una nación, automáticamente se hace zoom a ella
  (experiencia más guiada, menos clics).
- **Opción B**: son dos acciones separadas; foco solo atenúa, zoom es una acción
  explícita "Entrar en la nación" (más control, más clics).
- *Recomendación tentativa*: A para usuarios nuevos (comportamiento por defecto), con
  un ajuste para cambiar a B. Decidir al implementar 5.2.1 + 5.5.

### 13.2 Línea temporal: ¿atenizar o ocultar lo posterior al año seleccionado?

- **Atenizar**: el director sigue viendo el resto del mapa como contexto difuminado.
- **Ocultar**: más limpio visualmente, pero pierdes la referencia espacial.
- *Recomendación tentativa*: atenizar por defecto (coherente con el foco por nación
  5.2.1, que también atenúa en vez de ocultar). Decidir al implementar 7.1.

### 13.3 Línea temporal: ¿filtrar solo eventos, o también ciudades/naciones por fecha?

- **Solo eventos** (mínimo viable): no toca el modelo, filtra la cronología visible.
- **También ciudades/naciones**: requiere añadir `anoFundacion` / `anoDisolucion`
  opcionales a `Ciudad` y `Nacion` (sección 9.2). Más potente, más trabajo.
- *Recomendación tentativa*: implementar primero "solo eventos" y decidir si el modelo
  necesita fechas cuando se vea en uso.

### 13.4 ¿Dónde viven los metadatos visuales de facciones?

Hoy `MapaController` tiene un `Map<String,String> ICONO` en código (iconos de las 15
facciones). No es geografía, es puramente visual. Se mantiene en código por ahora
(comentado como excepción). Si crece, mover a `geografia.json` como campo opcional de
la facción. No urgente.
