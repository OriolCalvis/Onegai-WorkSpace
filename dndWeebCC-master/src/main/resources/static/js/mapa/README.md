# Moduls JavaScript del Mapa Mundi

El mapa va sortir d'un script inline molt gran. Ara la logica viu en moduls petits sota `static/js/mapa/` i comparteix estat a `window.GMMapa`.

## Ordre de carrega

L'ordre a `templates/mapa/index.html` importa:

1. `mapa-nucleo.js`
2. `mapa-filtros.js`
3. `mapa-puntos.js`
4. `mapa-vertices.js`
5. `mapa-borrador.js`
6. `mapa-dialogo.js`
7. `mapa-ciudades.js`
8. `mapa-zonas.js`
9. `mapa-cronologia.js`
10. `mapa-nacion.js`
11. `mapa-teclado.js`
12. `mapa-leyenda.js`
13. `mapa-init.js`

Els dotze primers defineixen estat i funcions; `mapa-init.js` connecta listeners i arrenca.

## Responsabilitats

| Modul | Responsabilitat |
|---|---|
| `mapa-nucleo.js` | Estat compartit, refs DOM i datalists |
| `mapa-filtros.js` | Ocultar/mostrar per nacio, faccio i tier |
| `mapa-puntos.js` | Punts personalitzats i editor de punt |
| `mapa-vertices.js` | Drag de vertexs i marcadors |
| `mapa-borrador.js` | Draft localStorage, payload i mode edicio |
| `mapa-dialogo.js` | Modal reusable |
| `mapa-ciudades.js` | Crear/editar ciutats |
| `mapa-zonas.js` | Crear/editar zones |
| `mapa-cronologia.js` | Events historics |
| `mapa-nacion.js` | Fitxa de nacio |
| `mapa-teclado.js` | Accessibilitat Enter/Espai |
| `mapa-leyenda.js` | Capes, focus i cercador |
| `mapa-init.js` | Wiring final |

## Regles per tocar-los

- Evita efectes en carregar un modul. Si cal connectar events, fes-ho a `mapa-init.js`.
- Comparteix via `window.GMMapa`, no amb variables globals soltes.
- Si afegeixes una eina visual nova, mira tambe `paneles-overlay.js` i `mapa-paneles.js`.
- Verifica sempre amb:

```bash
node --check src/main/resources/static/js/mapa/*.js
```
