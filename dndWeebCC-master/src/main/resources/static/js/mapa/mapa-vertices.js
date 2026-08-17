/*
 * mapa-vertices.js — Arrossegament de vèrtexs i marcadors, i col·locació de punts nous
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada és a mapa-init.js.
 */
(function (GM) {
    'use strict';

    'use strict';

    GM.buildHandles = function buildHandles() {
        GM.handlesGroup.innerHTML = '';
        GM.polygons.forEach(function (poly, pi) {
            GM.parsePoints(poly).forEach(function (pt, vi) {
                var c = document.createElementNS(GM.svgNS, 'circle');
                c.setAttribute('cx', pt[0]);
                c.setAttribute('cy', pt[1]);
                c.setAttribute('r', 7);
                c.classList.add('gm-editor-handle');
                c.dataset.poly = pi;
                c.dataset.vertex = vi;
                c.addEventListener('pointerdown', GM.onHandlePointerDown);
                c.addEventListener('dblclick', GM.onHandleDblClick);
                GM.handlesGroup.appendChild(c);
            });
        });
    }


    GM.pointsToString = function pointsToString(pts) {
        return pts.map(function (pt) { return pt[0] + ',' + pt[1]; }).join(' ');
    }


    GM.distToSegment = function distToSegment(p, a, b) {
        var dx = b[0] - a[0], dy = b[1] - a[1];
        var lengthSq = dx * dx + dy * dy;
        if (lengthSq === 0) return Math.hypot(p.x - a[0], p.y - a[1]);
        var t = ((p.x - a[0]) * dx + (p.y - a[1]) * dy) / lengthSq;
        t = Math.max(0, Math.min(1, t));
        return Math.hypot(p.x - (a[0] + t * dx), p.y - (a[1] + t * dy));
    }


    // Doble clic sobre el territorio de una nación = añade un vértice nuevo en el
    // punto exacto del clic, insertado en el lado del polígono más cercano.
    GM.onPolygonDblClick = function onPolygonDblClick(evt) {
        if (!GM.editing) return;
        evt.preventDefault();
        evt.stopPropagation();
        var poly = evt.currentTarget;
        var pi = GM.polygons.indexOf(poly);
        var p = GM.toSvgPoint(evt);
        var pts = GM.parsePoints(poly);
        var insertAt = 1, minDist = Infinity;
        for (var i = 0; i < pts.length; i++) {
            var a = pts[i], b = pts[(i + 1) % pts.length];
            var d = GM.distToSegment(p, a, b);
            if (d < minDist) { minDist = d; insertAt = i + 1; }
        }
        pts.splice(insertAt, 0, [p.x, p.y]);
        poly.setAttribute('points', GM.pointsToString(pts));
        GM.buildHandles();
        GM.saveDraft();
    }


    // Doble clic sobre un punto existente = lo elimina (mínimo 3 vértices por polígono).
    GM.onHandleDblClick = function onHandleDblClick(evt) {
        if (!GM.editing) return;
        evt.preventDefault();
        evt.stopPropagation();
        var pi = parseInt(evt.target.dataset.poly, 10);
        var vi = parseInt(evt.target.dataset.vertex, 10);
        var poly = GM.polygons[pi];
        var pts = GM.parsePoints(poly);
        if (pts.length <= 3) { alert('Una nación necesita al menos 3 puntos — no se puede quitar más.'); return; }
        pts.splice(vi, 1);
        poly.setAttribute('points', GM.pointsToString(pts));
        GM.buildHandles();
        GM.saveDraft();
    }


    GM.onHandlePointerDown = function onHandlePointerDown(evt) {
        if (!GM.editing) return;
        evt.preventDefault();
        evt.stopPropagation();
        GM.dragging = { type: 'vertex', poly: parseInt(evt.target.dataset.poly, 10), vertex: parseInt(evt.target.dataset.vertex, 10), handle: evt.target };
        evt.target.setPointerCapture(evt.pointerId);
    }


    GM.onMarkerPointerDown = function onMarkerPointerDown(evt) {
        if (!GM.editing) return;
        var marker = evt.currentTarget;
        evt.preventDefault();
        evt.stopPropagation();
        GM.dragging = { type: 'marker', el: marker, moved: false };
        marker.setPointerCapture(evt.pointerId);
    }


    GM.onSvgPointerMove = function onSvgPointerMove(evt) {
        var p = GM.toSvgPoint(evt);
        if (GM.editing) GM.coordsLabel.textContent = 'x: ' + p.x + ', y: ' + p.y;
        if (!GM.dragging) return;
        if (GM.dragging.type === 'vertex') {
            GM.dragging.handle.setAttribute('cx', p.x);
            GM.dragging.handle.setAttribute('cy', p.y);
            var poly = GM.polygons[GM.dragging.poly];
            var handlesForPoly = GM.handlesGroup.querySelectorAll('[data-poly="' + GM.dragging.poly + '"]');
            var pts = Array.from(handlesForPoly).map(function (h) {
                return h.getAttribute('cx') + ',' + h.getAttribute('cy');
            }).join(' ');
            poly.setAttribute('points', pts);
        } else if (GM.dragging.type === 'marker') {
            GM.dragging.moved = true;
            GM.dragging.el.setAttribute('transform', 'translate(' + p.x + ',' + p.y + ')');
        }
        GM.saveDraft();
    }


    GM.onSvgPointerUp = function onSvgPointerUp(evt) {
        if (GM.placingCiudad) {
            var sobre = evt.target.closest && evt.target.closest('.gm-mapa-marcador, .gm-editor-handle');
            if (!sobre) GM.colocarNuevaCiudad(evt, GM.nuevaCiudadBtn && GM.nuevaCiudadBtn.dataset.aleatoria === '1');
            return;
        }
        if (GM.placingZona) {
            var sobreZ = evt.target.closest && evt.target.closest('.gm-mapa-marcador, .gm-editor-handle');
            if (!sobreZ) GM.colocarNuevaZona(evt, GM.nuevaZonaBtn && GM.nuevaZonaBtn.dataset.aleatoria === '1');
            return;
        }
        if (GM.placingPunto) {
            var enElemento = evt.target.closest && evt.target.closest('.gm-mapa-marcador, .gm-editor-handle');
            if (!enElemento) GM.colocarNuevoPunto(evt);
            return;
        }
        if (GM.dragging && GM.dragging.type === 'marker' && GM.dragging.moved) {
            // Evita que el click de "soltar" reabra el panel lateral de la región.
            var marker = GM.dragging.el;
            var suppress = function (e) { e.stopImmediatePropagation(); e.preventDefault(); marker.removeEventListener('click', suppress, true); };
            marker.addEventListener('click', suppress, true);
        }
        GM.dragging = null;
    }


    GM.setPlacingPunto = function setPlacingPunto(value) {
        GM.placingPunto = value;
        GM.lienzo.classList.toggle('is-placing', value);
        GM.nuevoPuntoBtn.textContent = value ? '✖️ Cancelar' : '➕ Nuevo punto';
        if (value) {
            GM.coordsLabel.textContent = 'Haz clic en el mapa para colocar el nuevo punto…';
        } else {
            GM.coordsLabel.textContent = GM.editing
                ? 'Arrastra un punto · doble clic en el mapa = añadir · doble clic en un punto = quitar'
                : '';
        }
    }


    GM.colocarNuevoPunto = function colocarNuevoPunto(evt) {
        var p = GM.toSvgPoint(evt);
        GM.setPlacingPunto(false);
        GM.abrirDialogo('➕ Nuevo punto de interés', [
            { clave: 'nombre', etiqueta: 'Nombre', tipo: 'texto', requerido: true },
            { clave: 'icono', etiqueta: 'Icono', tipo: 'select-otro',
              opciones: ['📌', '🏚', '🕯', '🗿', '⚔️', '🕳', '🌋', '🗝', '🛖', '🎪'], vacio: '📌' }
        ], { icono: '📌' }, function (datos) {
            var id = 'punto_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
            GM.crearMarcadorPunto({ id: id, nombre: datos.nombre, icono: datos.icono || '📌',
                x: p.x, y: p.y, historiaIds: [], aventuraIds: [], eventoIds: [] });
            GM.saveDraft();
        });
    }
})(window.GMMapa);
