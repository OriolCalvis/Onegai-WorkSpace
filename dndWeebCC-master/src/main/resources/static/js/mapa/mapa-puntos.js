/*
 * mapa-puntos.js — Punts personalitzats: marcadors, panell i editor
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.toSvgPoint = function toSvgPoint(evt) {
        var pt = GM.svg.createSVGPoint();
        pt.x = evt.clientX;
        pt.y = evt.clientY;
        var ctm = GM.svg.getScreenCTM();
        if (!ctm) return { x: 0, y: 0 };
        var local = pt.matrixTransform(ctm.inverse());
        return { x: Math.round(local.x), y: Math.round(local.y) };
    }


    GM.parsePoints = function parsePoints(poly) {
        return poly.getAttribute('points').trim().split(/\s+/).map(function (p) {
            var xy = p.split(',');
            return [parseFloat(xy[0]), parseFloat(xy[1])];
        });
    }


    GM.markerPos = function markerPos(marker) {
        var m = /translate\(([-\d.]+),\s*([-\d.]+)\)/.exec(marker.getAttribute('transform') || '');
        return m ? { x: parseFloat(m[1]), y: parseFloat(m[2]) } : { x: 0, y: 0 };
    }


    // Engancha a un marcador (de facción o de punto) toda la interacción común:
    // arrastre en modo edición y apertura del panel con Enter/Espacio.
    GM.wireMarker = function wireMarker(marker) {
        marker.addEventListener('pointerdown', GM.onMarkerPointerDown);
        marker.addEventListener('keydown', function (evento) {
            if (evento.key === 'Enter' || evento.key === ' ') {
                evento.preventDefault();
                marker.dispatchEvent(new MouseEvent('click', { bubbles: true }));
            }
        });
    }


    // Crea un punto personalizado nuevo: el marcador arrastrable en el SVG + el
    // <template> que panels.js clona al abrir su panel. Se usa tanto para "➕ Nuevo
    // punto" como para recuperar puntos de un borrador de localStorage que aún no se
    // guardó en el servidor.
    GM.crearMarcadorPunto = function crearMarcadorPunto(datos) {
        var g = document.createElementNS(GM.svgNS, 'g');
        g.setAttribute('class', 'gm-mapa-marcador gm-mapa-marcador--punto');
        g.setAttribute('tabindex', '0');
        g.setAttribute('role', 'button');
        g.setAttribute('transform', 'translate(' + datos.x + ',' + datos.y + ')');
        g.dataset.panelSource = 'panel-tpl-' + datos.id;
        g.dataset.panelTitle = datos.nombre;
        g.dataset.puntoId = datos.id;
        g.dataset.puntoNombre = datos.nombre;
        g.dataset.puntoIcono = datos.icono;
        g.dataset.historiaIds = (datos.historiaIds || []).join(',');
        g.dataset.aventuraIds = (datos.aventuraIds || []).join(',');
        g.dataset.eventoIds = (datos.eventoIds || []).join(',');
        g.dataset.tiers = '';

        var circle = document.createElementNS(GM.svgNS, 'circle');
        circle.setAttribute('r', '18');
        circle.setAttribute('stroke-width', '3');
        g.appendChild(circle);

        var emoji = document.createElementNS(GM.svgNS, 'text');
        emoji.setAttribute('class', 'gm-mapa-emoji');
        emoji.setAttribute('y', '1');
        emoji.setAttribute('style', 'font-size:18px;');
        emoji.textContent = datos.icono;
        g.appendChild(emoji);

        var etiqueta = document.createElementNS(GM.svgNS, 'text');
        etiqueta.setAttribute('class', 'gm-mapa-etiqueta');
        etiqueta.setAttribute('y', '33');
        etiqueta.textContent = datos.nombre;
        g.appendChild(etiqueta);

        GM.svg.appendChild(g);
        GM.wireMarker(g);
        GM.markers.push(g);
        GM.recalcularTiersDePunto(g);

        var plantilla = document.createElement('template');
        plantilla.id = 'panel-tpl-' + datos.id;
        plantilla.innerHTML =
            '<p class="gm-hint">Punto personalizado nuevo — todavía sin guardar.</p>' +
            '<div class="gm-panel-section is-expanded gm-punto-editor" data-punto-id="' + datos.id + '">' +
            '  <button type="button" class="gm-panel-section__toggle">✏️ Editar vínculos ▾</button>' +
            '  <div class="gm-panel-section__content">' +
            '    <p class="gm-hint u-mt-0">Historias vinculadas</p>' +
            '    <div class="gm-punto-editor__historias"></div>' +
            '    <div class="gm-punto-editor__add-row">' +
            '      <input type="text" list="gm-historia-datalist" class="gm-input gm-punto-editor__input" placeholder="Buscar historia por título…">' +
            '      <button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-punto-editor__add-historia">+ Añadir</button>' +
            '    </div>' +
            '    <p class="gm-hint">Aventuras vinculadas</p>' +
            '    <div class="gm-punto-editor__aventuras"></div>' +
            '    <div class="gm-punto-editor__add-row">' +
            '      <input type="text" list="gm-aventura-datalist" class="gm-input gm-punto-editor__input" placeholder="Buscar aventura por nombre…">' +
            '      <button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-punto-editor__add-aventura">+ Añadir</button>' +
            '    </div>' +
            '    <p class="gm-hint">Eventos vinculados</p>' +
            '    <div class="gm-punto-editor__eventos"></div>' +
            '    <div class="gm-punto-editor__add-row">' +
            '      <input type="text" list="gm-evento-datalist" class="gm-input gm-punto-editor__input" placeholder="Buscar evento por nombre…">' +
            '      <button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-punto-editor__add-evento">+ Añadir</button>' +
            '    </div>' +
            '    <div class="gm-punto-editor__acciones">' +
            '      <button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-punto-editor__renombrar">✏️ Renombrar</button>' +
            '      <button type="button" class="gm-btn gm-btn--danger gm-btn--sm gm-punto-editor__eliminar">🗑️ Eliminar punto</button>' +
            '    </div>' +
            '  </div>' +
            '</div>';
        document.body.appendChild(plantilla);

        return g;
    }


    GM.eliminarMarcadorPunto = function eliminarMarcadorPunto(marker) {
        var idx = GM.markers.indexOf(marker);
        if (idx >= 0) GM.markers.splice(idx, 1);
        var plantilla = document.getElementById('panel-tpl-' + marker.dataset.puntoId);
        if (plantilla) plantilla.remove();
        marker.remove();
        GM.cerrarPanelSiCorresponde();
        GM.saveDraft();
    }


    GM.cerrarPanelSiCorresponde = function cerrarPanelSiCorresponde() {
        var panel = document.getElementById('gm-panel-preview');
        if (panel) {
            panel.classList.remove('is-open');
            panel.setAttribute('aria-hidden', 'true');
            var backdrop = document.getElementById('gm-panel-backdrop');
            if (backdrop) backdrop.classList.remove('is-open');
        }
    }


    // Repinta la sección "Editar vínculos" de un punto con los datos actuales del
    // marcador — se llama cada vez que se abre su panel (el <template> es estático,
    // así que si no hiciéramos esto se vería siempre el estado de la última carga
    // de página en vez de los cambios hechos en esta sesión).
    GM.renderPuntoEditor = function renderPuntoEditor(marker) {
        var cuerpo = document.getElementById('gm-panel-preview-body');
        var contenedor = cuerpo && cuerpo.querySelector('.gm-punto-editor');
        if (!contenedor) return;

        function pintarChips(contenedorLista, ids, mapaEtiquetas, tipoDato) {
            contenedorLista.innerHTML = '';
            if (!ids.length) {
                contenedorLista.innerHTML = '<span class="gm-hint">Ninguna todavía.</span>';
                return;
            }
            ids.forEach(function (id) {
                var chip = document.createElement('span');
                chip.className = 'gm-badge gm-punto-chip';
                var texto = document.createElement('span');
                texto.textContent = mapaEtiquetas[id] || id;
                chip.appendChild(texto);
                var quitar = document.createElement('button');
                quitar.type = 'button';
                quitar.className = 'gm-punto-chip__quitar';
                quitar.setAttribute('aria-label', 'Quitar');
                quitar.textContent = '×';
                quitar.dataset[tipoDato] = id;
                chip.appendChild(quitar);
                contenedorLista.appendChild(chip);
            });
        }

        function idsDe(campo) {
            return (marker.dataset[campo] || '').split(',').filter(Boolean);
        }

        pintarChips(contenedor.querySelector('.gm-punto-editor__historias'), idsDe('historiaIds'), GM.GM_HISTORIAS, 'quitarHistoria');
        pintarChips(contenedor.querySelector('.gm-punto-editor__aventuras'), idsDe('aventuraIds'), GM.GM_AVENTURAS, 'quitarAventura');
        var eventosEl = contenedor.querySelector('.gm-punto-editor__eventos');
        if (eventosEl) pintarChips(eventosEl, idsDe('eventoIds'), GM.GM_EVENTOS, 'quitarEvento');

        // Quitar vínculo
        contenedor.querySelectorAll('[data-quitar-historia]').forEach(function (btn) {
            btn.addEventListener('click', function () {
                var restantes = idsDe('historiaIds').filter(function (id) { return id !== btn.dataset.quitarHistoria; });
                marker.dataset.historiaIds = restantes.join(',');
                GM.recalcularTiersDePunto(marker);
                GM.renderPuntoEditor(marker);
                GM.saveDraft();
                GM.aplicarFiltros();
            });
        });
        contenedor.querySelectorAll('[data-quitar-aventura]').forEach(function (btn) {
            btn.addEventListener('click', function () {
                var restantes = idsDe('aventuraIds').filter(function (id) { return id !== btn.dataset.quitarAventura; });
                marker.dataset.aventuraIds = restantes.join(',');
                GM.renderPuntoEditor(marker);
                GM.saveDraft();
            });
        });
        contenedor.querySelectorAll('[data-quitar-evento]').forEach(function (btn) {
            btn.addEventListener('click', function () {
                var restantes = idsDe('eventoIds').filter(function (id) { return id !== btn.dataset.quitarEvento; });
                marker.dataset.eventoIds = restantes.join(',');
                GM.recalcularTiersDePunto(marker);
                GM.renderPuntoEditor(marker);
                GM.saveDraft();
                GM.aplicarFiltros();
            });
        });

        // Los botones de abajo (añadir/renombrar/eliminar) son fijos — no se
        // recrean cada vez que se repintan los chips — así que solo se enganchan
        // la primera vez que se abre el panel; si no, cada añadir/quitar apilaría
        // un listener nuevo encima de los anteriores.
        if (contenedor.dataset.wired) return;
        contenedor.dataset.wired = '1';

        // Añadir vínculo
        var addHistoriaBtn = contenedor.querySelector('.gm-punto-editor__add-historia');
        if (addHistoriaBtn) {
            addHistoriaBtn.addEventListener('click', function () {
                var input = contenedor.querySelector('.gm-punto-editor__add-row input[list="gm-historia-datalist"]');
                var id = GM.extraerIdDeDatalist(input.value);
                if (!id || !GM.GM_HISTORIAS[id]) { alert('Elige una historia de la lista sugerida.'); return; }
                var actuales = idsDe('historiaIds');
                if (actuales.indexOf(id) === -1) actuales.push(id);
                marker.dataset.historiaIds = actuales.join(',');
                GM.recalcularTiersDePunto(marker);
                input.value = '';
                GM.renderPuntoEditor(marker);
                GM.saveDraft();
                GM.aplicarFiltros();
            });
        }
        var addAventuraBtn = contenedor.querySelector('.gm-punto-editor__add-aventura');
        if (addAventuraBtn) {
            addAventuraBtn.addEventListener('click', function () {
                var campo = contenedor.querySelector('input[list="gm-aventura-datalist"]');
                var id = GM.extraerIdDeDatalist(campo.value);
                if (!id || !GM.GM_AVENTURAS[id]) { alert('Elige una aventura de la lista sugerida.'); return; }
                var actuales = idsDe('aventuraIds');
                if (actuales.indexOf(id) === -1) actuales.push(id);
                marker.dataset.aventuraIds = actuales.join(',');
                campo.value = '';
                GM.renderPuntoEditor(marker);
                GM.saveDraft();
            });
        }
        var addEventoBtn = contenedor.querySelector('.gm-punto-editor__add-evento');
        if (addEventoBtn) {
            addEventoBtn.addEventListener('click', function () {
                var campo = contenedor.querySelector('input[list="gm-evento-datalist"]');
                var id = GM.extraerIdDeDatalist(campo.value);
                if (!id || !GM.GM_EVENTOS[id]) { alert('Elige un evento de la lista sugerida.'); return; }
                var actuales = idsDe('eventoIds');
                if (actuales.indexOf(id) === -1) actuales.push(id);
                marker.dataset.eventoIds = actuales.join(',');
                GM.recalcularTiersDePunto(marker);
                campo.value = '';
                GM.renderPuntoEditor(marker);
                GM.saveDraft();
                GM.aplicarFiltros();
            });
        }

        var renombrarBtn = contenedor.querySelector('.gm-punto-editor__renombrar');
        if (renombrarBtn) {
            renombrarBtn.addEventListener('click', function () {
                var nuevo = prompt('Nuevo nombre para el punto:', marker.dataset.puntoNombre);
                if (!nuevo || !nuevo.trim()) return;
                marker.dataset.puntoNombre = nuevo.trim();
                marker.setAttribute('data-panel-title', nuevo.trim());
                var etiqueta = marker.querySelector('.gm-mapa-etiqueta');
                if (etiqueta) etiqueta.textContent = nuevo.trim();
                var tituloPanel = document.getElementById('gm-panel-preview-title');
                if (tituloPanel) tituloPanel.textContent = nuevo.trim();
                GM.saveDraft();
            });
        }

        var eliminarBtn = contenedor.querySelector('.gm-punto-editor__eliminar');
        if (eliminarBtn) {
            eliminarBtn.addEventListener('click', function () {
                if (!confirm('¿Eliminar "' + marker.dataset.puntoNombre + '" del mapa? Solo se borra al pulsar "Guardar cambios".')) return;
                GM.eliminarMarcadorPunto(marker);
            });
        }
    }

})(window.GMMapa);
