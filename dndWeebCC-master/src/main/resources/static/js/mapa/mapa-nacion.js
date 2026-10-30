/*
 * mapa-nacion.js — Fitxa de nació: identitat, color i informació
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.normalizarColor = function normalizarColor(c) {
        if (/^#[0-9a-fA-F]{6}$/.test(c || '')) return c;
        if (/^#[0-9a-fA-F]{3}$/.test(c || '')) return '#' + c[1] + c[1] + c[2] + c[2] + c[3] + c[3];
        return '#888888';
    }


    GM.dentroDeNacion = function dentroDeNacion(poly, x, y) {
        var pts = poly.getAttribute('points').trim().split(/\s+/).map(function (par) {
            var c = par.split(','); return { x: +c[0], y: +c[1] };
        });
        var dentro = false;
        for (var a = 0, b = pts.length - 1; a < pts.length; b = a++) {
            if (((pts[a].y > y) !== (pts[b].y > y)) &&
                (x < (pts[b].x - pts[a].x) * (y - pts[a].y) / (pts[b].y - pts[a].y) + pts[a].x)) dentro = !dentro;
        }
        return dentro;
    }


    GM.parrafoIdentidad = function parrafoIdentidad(nombre, valor) {
        return '<p><strong>' + nombre + ':</strong> ' +
            (valor ? GM.esc(valor) : '<span class="gm-hint">sin definir</span>') + '</p>';
    }


    GM.renderNacionPanel = function renderNacionPanel(poly) {
        var cuerpo = document.getElementById('gm-panel-preview-body');
        if (!cuerpo) return;
        cuerpo.innerHTML = '';
        var nombre = poly.dataset.nacion;

        // Qué hay dentro del territorio (point-in-polygon sobre los marcadores actuales)
        var dentro = { ciudades: [], zonas: [], puntos: [], facciones: [] };
        GM.markers.forEach(function (m) {
            var pos = GM.markerPos(m);
            if (!GM.dentroDeNacion(poly, pos.x, pos.y)) return;
            if (m.dataset.ciudadId) dentro.ciudades.push(m);
            else if (m.dataset.zonaId) dentro.zonas.push(m);
            else if (m.dataset.puntoId) dentro.puntos.push(m);
            else if (m.dataset.regionId) dentro.facciones.push(m);
        });

        var resumen = document.createElement('p');
        resumen.className = 'gm-hint';
        resumen.textContent = dentro.ciudades.length + ' ciudades · ' + dentro.zonas.length +
            ' zonas · ' + dentro.facciones.length + ' facciones jugables en su territorio';
        cuerpo.appendChild(resumen);

        // — Color en el mapa (en vivo: se guarda con "Guardar cambios") —
        var filaColor = document.createElement('div');
        filaColor.className = 'gm-nacion-color-row';
        var colorInput = document.createElement('input');
        colorInput.type = 'color';
        colorInput.value = GM.normalizarColor(poly.getAttribute('fill'));
        var colorTxt = document.createElement('span');
        colorTxt.className = 'gm-hint';
        colorTxt.textContent = 'Color de ' + nombre + ' en el mapa';
        filaColor.appendChild(colorInput);
        filaColor.appendChild(colorTxt);
        colorInput.addEventListener('input', function () {
            poly.setAttribute('fill', colorInput.value);
            poly.setAttribute('stroke', colorInput.value);
            GM.actualizarSwatchLeyenda(nombre, colorInput.value);
            GM.saveDraft();
        });
        cuerpo.appendChild(filaColor);

        // — Identidad: cultura, alineación, curiosidades + dioses y razas de catálogo —
        var dioses = (poly.dataset.nacionDeidades || '').split(',').filter(Boolean);
        var razas = (poly.dataset.nacionRazas || '').split(',').filter(Boolean);
        var ident = document.createElement('div');
        ident.className = 'gm-panel-section is-expanded';
        ident.innerHTML =
            '<button type="button" class="gm-panel-section__toggle">Identidad ▾</button>' +
            '<div class="gm-panel-section__content">' +
            GM.parrafoIdentidad('Cultura', poly.dataset.nacionCultura) +
            GM.parrafoIdentidad('Alineación', poly.dataset.nacionAlineacion) +
            GM.parrafoIdentidad('Curiosidades', poly.dataset.nacionCuriosidades) +
            '<p class="gm-hint u-mb-4">Dioses venerados</p><div class="gm-nacion-dioses"></div>' +
            '<div class="gm-punto-editor__add-row"><select class="gm-select gm-nacion-add-dios"></select>' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-nacion-add-dios-btn">+ Añadir</button></div>' +
            '<p class="gm-hint u-mb-4">Razas predominantes</p><div class="gm-nacion-razas"></div>' +
            '<div class="gm-punto-editor__add-row"><select class="gm-select gm-nacion-add-raza"></select>' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-nacion-add-raza-btn">+ Añadir</button></div>' +
            '<div class="gm-punto-editor__acciones">' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-nacion-editar-identidad">✏️ Editar cultura y alineación</button>' +
            '</div></div>';
        cuerpo.appendChild(ident);

        function pintarSeleccionados(cont, ids, mapa, alQuitar) {
            cont.innerHTML = '';
            if (!ids.length) { cont.innerHTML = '<span class="gm-hint">Ninguno todavía.</span>'; return; }
            ids.forEach(function (id) {
                var chip = document.createElement('span');
                chip.className = 'gm-badge';
                chip.style.cssText = 'margin:0 4px 4px 0; display:inline-flex; align-items:center; gap:4px;';
                chip.textContent = (mapa[id] || id) + ' ';
                var x = document.createElement('button');
                x.type = 'button'; x.textContent = '×';
                x.style.cssText = 'border:none; background:none; cursor:pointer; font-weight:700;';
                x.addEventListener('click', function () { alQuitar(id); });
                chip.appendChild(x);
                cont.appendChild(chip);
            });
        }

        function llenarSelect(sel, mapa, elegidos, vacio) {
            sel.innerHTML = '';
            var o0 = document.createElement('option');
            o0.value = '';
            o0.textContent = '— ' + vacio + ' —';
            sel.appendChild(o0);
            Object.keys(mapa).sort(function (a, b) { return mapa[a].localeCompare(mapa[b]); }).forEach(function (id) {
                if (elegidos.indexOf(id) !== -1) return;
                var o = document.createElement('option');
                o.value = id;
                o.textContent = mapa[id];
                sel.appendChild(o);
            });
        }

        pintarSeleccionados(ident.querySelector('.gm-nacion-dioses'), dioses, GM.GM_DEIDADES, function (id) {
            poly.dataset.nacionDeidades = dioses.filter(function (d) { return d !== id; }).join(',');
            GM.saveDraft(); GM.renderNacionPanel(poly);
        });
        pintarSeleccionados(ident.querySelector('.gm-nacion-razas'), razas, GM.GM_RAZAS, function (id) {
            poly.dataset.nacionRazas = razas.filter(function (r) { return r !== id; }).join(',');
            GM.saveDraft(); GM.renderNacionPanel(poly);
        });
        llenarSelect(ident.querySelector('.gm-nacion-add-dios'), GM.GM_DEIDADES, dioses, 'elegir deidad');
        llenarSelect(ident.querySelector('.gm-nacion-add-raza'), GM.GM_RAZAS, razas, 'elegir raza');
        ident.querySelector('.gm-nacion-add-dios-btn').addEventListener('click', function () {
            var v = ident.querySelector('.gm-nacion-add-dios').value;
            if (!v) return;
            poly.dataset.nacionDeidades = dioses.concat([v]).join(',');
            GM.saveDraft(); GM.renderNacionPanel(poly);
        });
        ident.querySelector('.gm-nacion-add-raza-btn').addEventListener('click', function () {
            var v = ident.querySelector('.gm-nacion-add-raza').value;
            if (!v) return;
            poly.dataset.nacionRazas = razas.concat([v]).join(',');
            GM.saveDraft(); GM.renderNacionPanel(poly);
        });
        ident.querySelector('.gm-nacion-editar-identidad').addEventListener('click', function () {
            GM.abrirDialogo('✏️ Identidad de ' + nombre, [
                { clave: 'cultura', etiqueta: 'Cultura (costumbres, lengua, oficios...)', tipo: 'area' },
                { clave: 'alineacion', etiqueta: 'Alineación predominante', tipo: 'select',
                  opciones: [''].concat(GM.GM_ALINEACIONES), vacio: 'sin definir' },
                { clave: 'curiosidades', etiqueta: 'Curiosidades (detalles de mesa)', tipo: 'area' }
            ], {
                cultura: poly.dataset.nacionCultura || '',
                alineacion: poly.dataset.nacionAlineacion || '',
                curiosidades: poly.dataset.nacionCuriosidades || ''
            }, function (datos) {
                poly.dataset.nacionCultura = datos.cultura;
                poly.dataset.nacionAlineacion = datos.alineacion;
                poly.dataset.nacionCuriosidades = datos.curiosidades;
                GM.saveDraft();
                GM.renderNacionPanel(poly);
            });
        });

        // — Qué hay en su territorio: enlaces que abren la ficha de cada elemento —
        function seccionMarcadores(titulo, lista, etiquetaDe) {
            if (!lista.length) return;
            var sec = document.createElement('div');
            sec.className = 'gm-panel-section is-expanded';
            var tog = document.createElement('button');
            tog.type = 'button';
            tog.className = 'gm-panel-section__toggle';
            tog.textContent = titulo + ' ▾';
            var contSec = document.createElement('div');
            contSec.className = 'gm-panel-section__content';
            lista.forEach(function (m) {
                var b = document.createElement('button');
                b.type = 'button';
                b.className = 'gm-badge';
                b.style.cssText = 'margin:0 4px 4px 0; cursor:pointer; border:none;';
                b.textContent = etiquetaDe(m);
                b.addEventListener('click', function () {
                    m.dispatchEvent(new MouseEvent('click', { bubbles: true }));
                });
                contSec.appendChild(b);
            });
            sec.appendChild(tog);
            sec.appendChild(contSec);
            cuerpo.appendChild(sec);
        }
        seccionMarcadores('Ciudades', dentro.ciudades, function (m) {
            return GM.iconoTamano(m.dataset.ciudadTamano) + ' ' + m.dataset.ciudadNombre;
        });
        seccionMarcadores('Zonas del terreno', dentro.zonas, function (m) {
            return GM.iconoZona(m.dataset.zonaTipo) + ' ' + m.dataset.zonaNombre;
        });
        seccionMarcadores('Puntos de interés', dentro.puntos, function (m) {
            return (m.dataset.puntoIcono || '📌') + ' ' + m.dataset.puntoNombre;
        });

        // — Facciones jugables del territorio: su ficha completa (historias, monstruos,
        //   tesoros, PNJs, aventuras, eventos), plegada para no abrumar —
        dentro.facciones.forEach(function (m) {
            var tpl = document.getElementById('panel-tpl-' + m.dataset.regionId);
            if (!tpl) return;
            var sec = document.createElement('div');
            sec.className = 'gm-panel-section';
            var tog = document.createElement('button');
            tog.type = 'button';
            tog.className = 'gm-panel-section__toggle';
            tog.textContent = '⚔️ ' + (m.dataset.panelTitle || m.dataset.regionId) + ' ▾';
            var contSec = document.createElement('div');
            contSec.className = 'gm-panel-section__content';
            contSec.appendChild(tpl.content.cloneNode(true));
            sec.appendChild(tog);
            sec.appendChild(contSec);
            cuerpo.appendChild(sec);
        });
    }


    // ======================================================================
    //  FICHA DE NACIÓN — clic en el territorio: feedback, color y toda su info
    // ======================================================================
    GM.GM_DEIDADES = GM.mapaDesdeDatalist('gm-deidad-datalist');

    GM.GM_RAZAS = GM.mapaDesdeDatalist('gm-raza-datalist');

    GM.GM_ALINEACIONES = ['legal bueno', 'neutral bueno', 'caótico bueno', 'legal neutral',
        'neutral', 'caótico neutral', 'legal malvado', 'neutral malvado', 'caótico malvado'];

})(window.GMMapa);
