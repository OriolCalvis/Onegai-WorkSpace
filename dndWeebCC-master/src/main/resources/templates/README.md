# Plantilles Thymeleaf

Aquest directori conté la capa visual del servidor. Les plantilles reben models ja preparats pels controladors; no haurien de contenir regles de negoci complexes.

## Estructura

| Directori | Us |
|---|---|
| `Blocks/` | Header, sidebar, footer, layout i fragments reutilitzables |
| `cartas/` | Índex del catàleg i plantilles per tipus de carta |
| `personatges/` | Llista, formulari i fitxa de joc |
| `aventuras/` | Llista, assistent, detall, impressió i constructor per actes |
| `historias/` | Llista, creació manual i detall amb escenes |
| `npcs/` | Àlbum, creació i fitxa GM |
| `tesoros/` | Loot tables |
| `eventos/` | CRUD d'events del mon |
| `mapa/` | Mapa Mundi interactiu |

## Patró recomanat

1. Usar `Blocks/layout-gm.html` o els fragments v2 quan sigui viable.
2. Mantenir els formularis com a inputs simples i passar transformacions al controller/form DTO.
3. Evitar JS inline gran. Si la interacció passa de pocs listeners, crear fitxer a `static/js/`.
4. Evitar styles inline llargs; preferir `components.css`, `utilities.css` o CSS especific.
5. Per carta nova: mantenir coherència amb la plantilla de cinc zones i `carta-fisica.css`.

## Nomenclatura

- `lista.html`: album o taula principal.
- `detalle.html` / `detall.html`: fitxa de lectura.
- `formulario.html` / `formulari.html`: creació/edició.
- `imprimir.html`: sortida preparada per paper/PDF.
- `constructor.html`: editor modular de la baralla d'història per actes d'una aventura.
