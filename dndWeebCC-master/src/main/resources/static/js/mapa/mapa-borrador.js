/*
 * mapa-borrador.js — Esborrany a localStorage, payload de desat i mode edició
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada és a mapa-init.js.
 */
(function (GM) {
    'use strict';


    GM.saveDraft = function saveDraft() {
        var data = {
            polys: GM.polygons.map(function (poly) { return poly.getAttribute('points'); }),
            markers: GM.markers.filter(function (m) { return !m.dataset.puntoId; })
                .map(function (m) { return { id: m.dataset.regionId, pos: GM.markerPos(m) }; }),
            puntos: GM.markers.filter(function (m) { return m.dataset.puntoId; })
                .map(function (m) {
                    var pos = GM.markerPos(m);
                    return {
                        id: m.dataset.puntoId, nombre: m.dataset.puntoNombre, icono: m.dataset.puntoIcono,
                        x: Math.round(pos.x), y: Math.round(pos.y),
                        historiaIds: (m.dataset.historiaIds || '').split(',').filter(Boolean),
                        aventuraIds: (m.dataset.aventuraIds || '').split(',').filter(Boolean),
                        eventoIds: (m.dataset.eventoIds || '').split(',').filter(Boolean)
                    };
                }),
            ciudades: GM.markers.filter(function (m) { return m.dataset.ciudadId; }).map(GM.ciudadDesdeMarcador),
            zonas: GM.markers.filter(function (m) { return m.dataset.zonaId; }).map(GM.zonaDesdeMarcador),
            eventosHistoricos: GM.eventosHistoricosDesdeDom(),
            nacionesInfo: GM.polygons.map(GM.identidadDeNacion)
        };
        try { localStorage.setItem(GM.LS_KEY, JSON.stringify(data)); } catch (e) { /* almacenamiento no disponible, se ignora */ }
    }


    // Identidad de worldbuilding de una nación, leída de los data-* de su polígono
    // (es donde viven los cambios de la sesión hasta que se pulsa "Guardar cambios").
    GM.identidadDeNacion = function identidadDeNacion(poly) {
        return {
            nombre: poly.dataset.nacion,
            color: poly.getAttribute('fill'),
            cultura: poly.dataset.nacionCultura || '',
            alineacion: poly.dataset.nacionAlineacion || '',
            curiosidades: poly.dataset.nacionCuriosidades || '',
            deidadIds: (poly.dataset.nacionDeidades || '').split(',').filter(Boolean),
            razaIds: (poly.dataset.nacionRazas || '').split(',').filter(Boolean)
        };
    }


    GM.aplicarIdentidadANacion = function aplicarIdentidadANacion(info) {
        var poly = GM.polygons.find(function (p) { return p.dataset.nacion === info.nombre; });
        if (!poly) return;
        if (info.color) {
            poly.setAttribute('fill', info.color);
            poly.setAttribute('stroke', info.color);
            GM.actualizarSwatchLeyenda(info.nombre, info.color);
        }
        poly.dataset.nacionCultura = info.cultura || '';
        poly.dataset.nacionAlineacion = info.alineacion || '';
        poly.dataset.nacionCuriosidades = info.curiosidades || '';
        poly.dataset.nacionDeidades = (info.deidadIds || []).join(',');
        poly.dataset.nacionRazas = (info.razaIds || []).join(',');
    }


    GM.actualizarSwatchLeyenda = function actualizarSwatchLeyenda(nacion, color) {
        var item = document.querySelector('.gm-mapa-leyenda__item[data-nacion-toggle="' + nacion + '"] .gm-mapa-leyenda__swatch');
        if (item) item.style.background = color;
    }


    GM.loadDraft = function loadDraft() {
        var raw;
        try { raw = localStorage.getItem(GM.LS_KEY); } catch (e) { return; }
        if (!raw) return;
        try {
            var data = JSON.parse(raw);
            data.polys.forEach(function (points, i) { if (GM.polygons[i]) GM.polygons[i].setAttribute('points', points); });
            data.markers.forEach(function (entry) {
                var el = GM.markers.find(function (m) { return m.dataset.regionId === entry.id; });
                if (el) el.setAttribute('transform', 'translate(' + entry.pos.x + ',' + entry.pos.y + ')');
            });
            (data.ciudades || []).forEach(function (c) {
                var el = GM.markers.find(function (m) { return m.dataset.ciudadId === c.id; });
                if (!el) { GM.crearMarcadorCiudad(c); return; }
                el.setAttribute('transform', 'translate(' + c.x + ',' + c.y + ')');
                el.dataset.ciudadNombre = c.nombre; el.dataset.ciudadNacion = c.nacion || '';
                el.dataset.ciudadTamano = c.tamano || 'ciudad'; el.dataset.ciudadPoblacion = c.poblacion || '';
                el.dataset.ciudadGobernante = c.gobernante || ''; el.dataset.ciudadRasgo = c.rasgo || '';
                el.dataset.ciudadDescripcion = c.descripcion || '';
                el.dataset.historiaIds = (c.historiaIds || []).join(',');
                el.dataset.npcIds = (c.npcIds || []).join(',');
                el.dataset.panelTitle = c.nombre;
                var et = el.querySelector('.gm-mapa-etiqueta'); if (et) et.textContent = c.nombre;
                var em = el.querySelector('.gm-mapa-emoji'); if (em) em.textContent = GM.iconoTamano(c.tamano);
                GM.construirPanelCiudad(c);
            });
            (data.zonas || []).forEach(function (z) {
                var el = GM.markers.find(function (m) { return m.dataset.zonaId === z.id; });
                if (!el) { GM.crearMarcadorZona(z); return; }
                el.setAttribute('transform', 'translate(' + z.x + ',' + z.y + ')');
                el.dataset.zonaNombre = z.nombre; el.dataset.zonaTipo = z.tipo || 'montana';
                el.dataset.zonaNacion = z.nacion || ''; el.dataset.zonaPeligro = z.peligro || '';
                el.dataset.zonaDescripcion = z.descripcion || '';
                el.dataset.historiaIds = (z.historiaIds || []).join(',');
                el.dataset.panelTitle = z.nombre;
                var etZ = el.querySelector('.gm-mapa-etiqueta'); if (etZ) etZ.textContent = z.nombre;
                var emZ = el.querySelector('.gm-mapa-emoji'); if (emZ) emZ.textContent = GM.iconoZona(z.tipo);
                GM.construirPanelZona(z);
            });
            (data.eventosHistoricos || []).forEach(GM.crearItemCronologia);
            (data.nacionesInfo || []).forEach(GM.aplicarIdentidadANacion);
            (data.puntos || []).forEach(function (p) {
                var el = GM.markers.find(function (m) { return m.dataset.puntoId === p.id; });
                if (!el) {
                    GM.crearMarcadorPunto(p);
                    return;
                }
                el.setAttribute('transform', 'translate(' + p.x + ',' + p.y + ')');
                el.dataset.puntoNombre = p.nombre;
                el.dataset.puntoIcono = p.icono;
                el.dataset.historiaIds = (p.historiaIds || []).join(',');
                el.dataset.aventuraIds = (p.aventuraIds || []).join(',');
                el.dataset.eventoIds = (p.eventoIds || []).join(',');
                GM.recalcularTiersDePunto(el);
                el.setAttribute('data-panel-title', p.nombre);
                var etiqueta = el.querySelector('.gm-mapa-etiqueta');
                if (etiqueta) etiqueta.textContent = p.nombre;
                var emoji = el.querySelector('.gm-mapa-emoji');
                if (emoji) emoji.textContent = p.icono;
            });
            GM.banner.hidden = false;
        } catch (e) { /* borrador corrupto, se ignora */ }
    }


    // Lee los datos de una ciudad desde los data-* de su marcador SVG.
    GM.ciudadDesdeMarcador = function ciudadDesdeMarcador(m) {
        var pos = GM.markerPos(m);
        return {
            id: m.dataset.ciudadId,
            nombre: m.dataset.ciudadNombre,
            nacion: m.dataset.ciudadNacion || '',
            tamano: m.dataset.ciudadTamano || 'ciudad',
            poblacion: m.dataset.ciudadPoblacion || '',
            gobernante: m.dataset.ciudadGobernante || '',
            rasgo: m.dataset.ciudadRasgo || '',
            descripcion: m.dataset.ciudadDescripcion || '',
            x: Math.round(pos.x), y: Math.round(pos.y),
            historiaIds: (m.dataset.historiaIds || '').split(',').filter(Boolean),
            npcIds: (m.dataset.npcIds || '').split(',').filter(Boolean)
        };
    }


    GM.buildPayload = function buildPayload() {
        var payload = { naciones: [], marcadores: {}, puntos: [], ciudades: [], zonas: [], eventosHistoricos: [] };
        GM.polygons.forEach(function (poly) {
            var info = GM.identidadDeNacion(poly);
            info.points = poly.getAttribute('points');
            payload.naciones.push(info);
        });
        payload.zonas = GM.markers.filter(function (m) { return m.dataset.zonaId; }).map(GM.zonaDesdeMarcador);
        payload.eventosHistoricos = GM.eventosHistoricosDesdeDom();
        GM.markers.forEach(function (m) {
            var pos = GM.markerPos(m);
            if (m.dataset.ciudadId) {
                payload.ciudades.push(GM.ciudadDesdeMarcador(m));
            } else if (m.dataset.puntoId) {
                payload.puntos.push({
                    id: m.dataset.puntoId, nombre: m.dataset.puntoNombre, icono: m.dataset.puntoIcono,
                    x: Math.round(pos.x), y: Math.round(pos.y),
                    historiaIds: (m.dataset.historiaIds || '').split(',').filter(Boolean),
                    aventuraIds: (m.dataset.aventuraIds || '').split(',').filter(Boolean),
                    eventoIds: (m.dataset.eventoIds || '').split(',').filter(Boolean)
                });
            } else {
                payload.marcadores[m.dataset.regionId] = [Math.round(pos.x), Math.round(pos.y)];
            }
        });
        return payload;
    }


    GM.buildExportText = function buildExportText() {
        // Mismo JSON que espera POST /mapa/guardar — sirve para pegar a mano en
        // data/mapa/geografia.json si el guardado automático fallara.
        return JSON.stringify(GM.buildPayload(), null, 2);
    }


    GM.setEditing = function setEditing(value) {
        GM.editing = value;
        GM.lienzo.classList.toggle('is-editing', GM.editing);
        GM.toggleBtn.textContent = GM.editing ? '✅ Salir del modo edición' : '🖊️ Modo edición';
        GM.saveBtn.hidden = !GM.editing;
        GM.nuevoPuntoBtn.hidden = !GM.editing;
        if (GM.nuevaCiudadBtn) GM.nuevaCiudadBtn.hidden = !GM.editing;
        if (GM.nuevaZonaBtn) GM.nuevaZonaBtn.hidden = !GM.editing;
        if (GM.nuevoEventoHistBtn) GM.nuevoEventoHistBtn.hidden = !GM.editing;
        document.querySelectorAll('.gm-eh-acciones').forEach(function (a) { a.hidden = !GM.editing; });
        GM.resetBtn.hidden = !GM.editing;
        GM.exportBtn.hidden = !GM.editing;
        GM.coordsLabel.textContent = GM.editing
            ? 'Arrastra un punto · doble clic en el mapa = añadir · doble clic en un punto = quitar · "🏙 Nueva ciudad" / "⛰ Nueva zona" (Alt+clic = al azar)'
            : '';
        if (!GM.editing) {
            GM.exportPanel.hidden = true;
            GM.setPlacingPunto(false);
            GM.setPlacingCiudad(false);
            GM.setPlacingZona(false);
        }
    }
})(window.GMMapa);
