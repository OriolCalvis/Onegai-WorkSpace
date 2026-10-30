/*
 * mapa-zonas.js — Creador i editor de zones del terreny
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.iconoZona = function iconoZona(t) { return (GM.GM_TIPOS_ZONA[t] || GM.GM_TIPOS_ZONA.montana).icono; }


    GM.generarNombreZona = function generarNombreZona(tipo, nacion) {
        var molde = GM.azar((GM.GM_TIPOS_ZONA[tipo] || GM.GM_TIPOS_ZONA.montana).nombres);
        return molde.replace('{X}', GM.generarNombreCiudad(nacion));
    }


    GM.formularioZona = function formularioZona(titulo, prefill, x, y, onOk) {
        var nacionSugerida = prefill.nacion || GM.nacionEnPunto(x, y) || '';
        var tiposOpts = Object.keys(GM.GM_TIPOS_ZONA).map(function (t) {
            return { v: t, t: GM.GM_TIPOS_ZONA[t].icono + ' ' + t.charAt(0).toUpperCase() + t.slice(1) };
        });
        GM.abrirDialogo(titulo, [
            { clave: 'nombre', etiqueta: 'Nombre', tipo: 'texto', requerido: true,
              dado: function (v) { return GM.generarNombreZona(v.tipo || 'montana', v.nacion || nacionSugerida); } },
            { clave: 'tipo', etiqueta: 'Tipo de terreno', tipo: 'select', opciones: tiposOpts },
            { clave: 'nacion', etiqueta: 'Nación', tipo: 'select', opciones: [''].concat(GM.nacionesDisponibles()),
              vacio: 'tierra de nadie', ayuda: nacionSugerida ? 'Detectada por posición: ' + nacionSugerida : '' },
            { clave: 'peligro', etiqueta: 'Peligro de mesa', tipo: 'select-otro',
              opciones: [''].concat(GM.GM_PELIGROS_ZONA), vacio: 'sin peligro conocido',
              dado: function () { return GM.azar(GM.GM_PELIGROS_ZONA); } },
            { clave: 'descripcion', etiqueta: 'Descripción breve', tipo: 'area' }
        ], Object.assign({ tipo: 'montana', nacion: nacionSugerida }, prefill), onOk);
    }


    GM.zonaDesdeMarcador = function zonaDesdeMarcador(m) {
        var pos = GM.markerPos(m);
        return {
            id: m.dataset.zonaId,
            nombre: m.dataset.zonaNombre,
            tipo: m.dataset.zonaTipo || 'montana',
            nacion: m.dataset.zonaNacion || '',
            peligro: m.dataset.zonaPeligro || '',
            descripcion: m.dataset.zonaDescripcion || '',
            x: Math.round(pos.x), y: Math.round(pos.y),
            historiaIds: (m.dataset.historiaIds || '').split(',').filter(Boolean)
        };
    }


    GM.crearMarcadorZona = function crearMarcadorZona(z) {
        var g = document.createElementNS(GM.svgNS, 'g');
        g.setAttribute('class', 'gm-mapa-marcador gm-mapa-marcador--zona');
        g.setAttribute('tabindex', '0');
        g.setAttribute('role', 'button');
        g.setAttribute('transform', 'translate(' + z.x + ',' + z.y + ')');
        g.dataset.panelSource = 'panel-zona-' + z.id;
        g.dataset.panelTitle = z.nombre;
        g.dataset.zonaId = z.id;
        g.dataset.zonaNombre = z.nombre;
        g.dataset.zonaTipo = z.tipo || 'montana';
        g.dataset.zonaNacion = z.nacion || '';
        g.dataset.zonaPeligro = z.peligro || '';
        g.dataset.zonaDescripcion = z.descripcion || '';
        g.dataset.historiaIds = (z.historiaIds || []).join(',');
        g.innerHTML =
            '<rect x="-14" y="-14" width="28" height="28" rx="6" transform="rotate(45)" class="gm-mapa-zona__rombo"></rect>' +
            '<text class="gm-mapa-emoji u-fs-16" y="1">' + GM.iconoZona(z.tipo) + '</text>' +
            '<text class="gm-mapa-etiqueta" y="33">' + GM.esc(z.nombre) + '</text>';
        GM.svg.appendChild(g);
        GM.markers.push(g);
        g.addEventListener('pointerdown', GM.onMarkerPointerDown);
        GM.construirPanelZona(z);
        return g;
    }


    GM.construirPanelZona = function construirPanelZona(z) {
        var tplId = 'panel-zona-' + z.id;
        var viejo = document.getElementById(tplId);
        if (viejo) viejo.remove();
        var tpl = document.createElement('template');
        tpl.id = tplId;
        var detalle = '';
        if (z.peligro || z.descripcion) {
            detalle = '<div class="gm-panel-section is-expanded"><button type="button" class="gm-panel-section__toggle">El lugar ▾</button><div class="gm-panel-section__content">' +
                (z.peligro ? '<p><strong>Peligro:</strong> ' + GM.esc(z.peligro) + '</p>' : '') +
                (z.descripcion ? '<p>' + GM.esc(z.descripcion) + '</p>' : '') + '</div></div>';
        }
        tpl.innerHTML =
            '<p class="gm-hint">' + GM.iconoZona(z.tipo) + ' ' + GM.esc(z.tipo) + (z.nacion ? ' · ' + GM.esc(z.nacion) : '') + '</p>' + detalle +
            '<div class="gm-panel-section is-expanded gm-zona-editor" data-zona-id="' + z.id + '">' +
            '<button type="button" class="gm-panel-section__toggle">✏️ Editar zona ▾</button>' +
            '<div class="gm-panel-section__content">' +
            '<p class="gm-hint u-mt-0">Historias que ocurren aquí</p><div class="gm-zona-editor__historias"></div>' +
            '<div class="gm-punto-editor__add-row"><input type="text" list="gm-historia-datalist" class="gm-input gm-punto-editor__input" placeholder="Buscar historia por título…">' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-zona-editor__add-historia">+ Añadir</button></div>' +
            '<div class="gm-punto-editor__acciones">' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-zona-editor__editar-campos">✏️ Nombre y detalles</button>' +
            '<button type="button" class="gm-btn gm-btn--danger gm-btn--sm gm-zona-editor__eliminar">🗑️ Eliminar zona</button>' +
            '</div></div></div>';
        document.body.appendChild(tpl);
    }


    GM.setPlacingZona = function setPlacingZona(v) {
        GM.placingZona = v;
        if (v) { GM.setPlacingPunto(false); GM.setPlacingCiudad(false); }
        GM.lienzo.classList.toggle('is-placing', v || GM.placingCiudad || GM.placingPunto);
        if (GM.nuevaZonaBtn) GM.nuevaZonaBtn.classList.toggle('gm-btn--primary', v);
        if (v) GM.coordsLabel.textContent = '⛰ Clic en el mapa para situar la zona';
    }


    GM.colocarNuevaZona = function colocarNuevaZona(evt, aleatoria) {
        var p = GM.toSvgPoint(evt);
        GM.setPlacingZona(false);
        function crear(datos) {
            var id = 'zona_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
            GM.crearMarcadorZona(Object.assign({ id: id, x: Math.round(p.x), y: Math.round(p.y), historiaIds: [] }, datos));
            GM.saveDraft();
        }
        if (aleatoria) {
            var nac = GM.nacionEnPunto(p.x, p.y) || '';
            var tipo = GM.azar(Object.keys(GM.GM_TIPOS_ZONA));
            crear({ nombre: GM.generarNombreZona(tipo, nac), tipo: tipo, nacion: nac,
                peligro: GM.azar(GM.GM_PELIGROS_ZONA), descripcion: '' });
        } else {
            GM.formularioZona('⛰ Nueva zona del terreno', {}, p.x, p.y, crear);
        }
    }


    GM.zonaPorId = function zonaPorId(id) { return GM.markers.find(function (m) { return m.dataset.zonaId === id; }); }


    GM.pintarChipsZona = function pintarChipsZona(editor) {
        var m = GM.zonaPorId(editor.dataset.zonaId);
        if (!m) return;
        var cont = editor.querySelector('.gm-zona-editor__historias');
        if (!cont) return;
        cont.innerHTML = '';
        (m.dataset.historiaIds || '').split(',').filter(Boolean).forEach(function (val) {
            var chip = document.createElement('span');
            chip.className = 'gm-badge';
            chip.style.cssText = 'margin:0 4px 4px 0; display:inline-flex; align-items:center; gap:4px;';
            chip.textContent = (GM.GM_HISTORIAS[val] || val) + ' ';
            var x = document.createElement('button');
            x.type = 'button'; x.textContent = '×';
            x.style.cssText = 'border:none; background:none; cursor:pointer; font-weight:700;';
            x.addEventListener('click', function () {
                m.dataset.historiaIds = (m.dataset.historiaIds || '').split(',')
                    .filter(function (v) { return v && v !== val; }).join(',');
                GM.pintarChipsZona(editor); GM.saveDraft();
            });
            chip.appendChild(x);
            cont.appendChild(chip);
        });
    }


    // ======================================================================
    //  CREADOR DE ZONAS DEL TERRENO (montañas, bosques, ruinas...)
    // ======================================================================
    GM.nuevaZonaBtn = document.getElementById('gm-editor-nueva-zona');

    GM.placingZona = false;


    GM.GM_TIPOS_ZONA = {
        montana:   { icono: '⛰', nombres: ['Picos de {X}', 'Sierra {X}', 'Agujas de {X}', 'Cumbres de {X}'] },
        bosque:    { icono: '🌲', nombres: ['Bosque de {X}', 'Espesura {X}', 'Fronda de {X}'] },
        lago:      { icono: '🌊', nombres: ['Lago {X}', 'Aguas de {X}', 'Espejo de {X}'] },
        rio:       { icono: '🏞', nombres: ['Río {X}', 'Cauce de {X}', 'Vado de {X}'] },
        desierto:  { icono: '🏜', nombres: ['Desierto de {X}', 'Dunas de {X}', 'Yermo {X}'] },
        pantano:   { icono: '🐸', nombres: ['Pantano de {X}', 'Ciénaga {X}', 'Marjal de {X}'] },
        ruina:     { icono: '🏚', nombres: ['Ruinas de {X}', 'Restos de {X}', 'Vestigios de {X}'] },
        fortaleza: { icono: '🏯', nombres: ['Fuerte {X}', 'Bastión de {X}', 'Atalaya de {X}'] },
        santuario: { icono: '⛩', nombres: ['Santuario de {X}', 'Altar de {X}', 'Ermita de {X}'] },
        puerto:    { icono: '⚓', nombres: ['Puerto {X}', 'Fondeadero de {X}', 'Cala de {X}'] },
        isla:      { icono: '🏝', nombres: ['Isla {X}', 'Islote de {X}', 'Peñón de {X}'] },
        paso:      { icono: '🛤', nombres: ['Paso de {X}', 'Desfiladero de {X}', 'Garganta de {X}'] }
    };

    GM.GM_PELIGROS_ZONA = ['nidos de arpía en los riscos', 'avalanchas en deshielo',
        'bandidos con peaje propio', 'niebla que desorienta', 'bestia territorial',
        'terreno que cede', 'espíritus que piden un trueque', 'plantas venenosas al roce',
        'crecidas repentinas', 'ecos que atraen depredadores', 'frío que muerde de noche',
        'ruinas con guardián dormido'];

})(window.GMMapa);
