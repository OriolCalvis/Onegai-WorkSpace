/*
 * mapa-paneles.js — Enlaza las herramientas del Mapa Mundi con el sistema de
 * paneles superpuestos (paneles-overlay.js). Mejora progresiva: mueve los
 * bloques ya renderizados (listeners intactos) dentro de paneles acoplados
 * sobre el lienzo del mapa.
 *
 *   🧰 Herramientas  — toolbar de edición con menú "➕ Nuevo ▾" (doc §6.8) — izq.
 *   🎚 Tiers         — filtros de tier — izq (plegado).
 *   🗺 Leyenda       — buscador + capas + naciones (foco) + facciones — der.
 *   📜 Cronología    — eventos históricos del mundo — der (plegado).
 *
 * La ficha de nación/ciudad/zona (#gm-panel-preview) se acopla al lado derecho
 * del lienzo en escritorio (sin telón de fondo): consultar una ficha ya no
 * bloquea el mapa. En móvil conserva su modal centrado.
 */
(function () {
    'use strict';
    if (!window.GmPaneles) return;

    var lienzo = document.querySelector('.gm-mapa-lienzo');
    if (!lienzo || !lienzo.parentNode) return;

    // ---------- workspace relativo alrededor del lienzo ----------
    var ws = document.createElement('div');
    ws.className = 'gm-mapa-workspace';
    lienzo.parentNode.insertBefore(ws, lienzo);
    ws.appendChild(lienzo);
    GmPaneles.init(ws);

    // ---------- toolbar 6.8: agrupar los "➕" en un desplegable ----------
    var edicion = document.getElementById('gm-toolbar-edicion');
    if (edicion) {
        var nuevos = ['gm-editor-nueva-ciudad', 'gm-editor-nueva-zona', 'gm-editor-nuevo-punto']
            .map(function (id) { return document.getElementById(id); })
            .filter(Boolean);

        if (nuevos.length) {
            var menu = document.createElement('details');
            menu.className = 'gm-nuevo-menu';
            var resumen = document.createElement('summary');
            resumen.className = 'gm-btn gm-btn--outline gm-btn--sm';
            resumen.textContent = '➕ Nuevo ▾';
            var lista = document.createElement('div');
            lista.className = 'gm-nuevo-menu__lista';
            nuevos.forEach(function (btn) { lista.appendChild(btn); });   // conserva listeners
            menu.appendChild(resumen);
            menu.appendChild(lista);
            var guardarBtn = document.getElementById('gm-editor-save');
            edicion.insertBefore(menu, guardarBtn ? guardarBtn.nextSibling : null);
            lista.addEventListener('click', function () { menu.open = false; });
            document.addEventListener('pointerdown', function (e) {
                if (menu.open && !menu.contains(e.target)) menu.open = false;
            });
        }

        GmPaneles.crear({
            id: 'herramientas',
            icono: '🧰',
            titulo: 'Herramientas',
            lado: 'izq',
            abierto: true,
            nodos: [document.getElementById('gm-editor-banner'), edicion]
        });
    }

    // ---------- filtros de tier ----------
    var tiers = document.getElementById('gm-toolbar-tiers');
    if (tiers) {
        GmPaneles.crear({
            id: 'tiers',
            icono: '🎚',
            titulo: 'Filtros de tier',
            lado: 'izq',
            abierto: false,
            nodos: [tiers]
        });
    }

    // ---------- leyenda + capas + foco (4a / doc 5.2) ----------
    var leyenda = document.getElementById('gm-leyenda-bloque');
    if (leyenda) {
        GmPaneles.crear({
            id: 'leyenda',
            icono: '🗺',
            titulo: 'Leyenda y capas',
            lado: 'der',
            abierto: true,
            nodos: [leyenda]
        });
    }

    // ---------- cronología del mundo ----------
    var cronologia = document.getElementById('gm-cronologia-bloque');
    if (cronologia) {
        GmPaneles.crear({
            id: 'cronologia',
            icono: '📜',
            titulo: 'Cronología del mundo',
            lado: 'der',
            abierto: false,
            nodos: [cronologia]
        });
    }

    // ---------- ficha de nación/ciudad/zona acoplada al lienzo ----------
    var ficha = document.getElementById('gm-panel-preview');
    if (ficha) {
        ws.appendChild(ficha);                    // absolute respecto al workspace
        ficha.classList.add('gm-panel--dock');
        var backdrop = document.getElementById('gm-panel-backdrop');
        if (backdrop) backdrop.classList.add('gm-backdrop-oculto');   // solo escritorio (CSS)
    }
})();
