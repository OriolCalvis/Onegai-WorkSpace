/*
 * mapa-ciudades.js — Creador i editor de ciutats
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.iconoTamano = function iconoTamano(t) { return (GM.GM_TAMANOS[t] || GM.GM_TAMANOS.ciudad).icono; }


    GM.azar = function azar(l) { return l[Math.floor(Math.random() * l.length)]; }


    GM.generarNombreCiudad = function generarNombreCiudad(nacion) {
        var banco = GM.GM_SILABAS[nacion] || GM.GM_SILABAS._gen;
        return GM.azar(banco.pre) + GM.azar(banco.suf);
    }


    GM.nacionesDisponibles = function nacionesDisponibles() {
        return GM.polygons.map(function (p) { return p.dataset.nacion; })
            .filter(function (v, i, a) { return v && a.indexOf(v) === i; });
    }


    // Construye el marcador SVG de una ciudad y lo engancha al sistema de arrastre.
    GM.crearMarcadorCiudad = function crearMarcadorCiudad(c) {
        var g = document.createElementNS(GM.svgNS, 'g');
        g.setAttribute('class', 'gm-mapa-marcador gm-mapa-marcador--ciudad');
        g.setAttribute('tabindex', '0');
        g.setAttribute('role', 'button');
        g.setAttribute('transform', 'translate(' + c.x + ',' + c.y + ')');
        g.dataset.panelTitle = c.nombre;
        g.dataset.ciudadId = c.id;
        g.dataset.ciudadNombre = c.nombre;
        g.dataset.ciudadNacion = c.nacion || '';
        g.dataset.ciudadTamano = c.tamano || 'ciudad';
        g.dataset.ciudadPoblacion = c.poblacion || '';
        g.dataset.ciudadGobernante = c.gobernante || '';
        g.dataset.ciudadRasgo = c.rasgo || '';
        g.dataset.ciudadDescripcion = c.descripcion || '';
        g.dataset.historiaIds = (c.historiaIds || []).join(',');
        g.dataset.npcIds = (c.npcIds || []).join(',');
        g.innerHTML =
            '<circle r="16" class="gm-mapa-ciudad__circulo"></circle>' +
            '<text class="gm-mapa-emoji u-fs-17" y="1">' + GM.iconoTamano(c.tamano) + '</text>' +
            '<text class="gm-mapa-etiqueta" y="31">' + c.nombre + '</text>';
        GM.svg.appendChild(g);
        GM.markers.push(g);
        g.addEventListener('pointerdown', GM.onMarkerPointerDown);
        GM.construirPanelCiudad(c);
        return g;
    }


    // Inserta (o rehace) el <template> del panel de vista previa de una ciudad,
    // para que al clicarla se abra su ficha aunque se acabe de crear.
    GM.construirPanelCiudad = function construirPanelCiudad(c) {
        var tplId = 'panel-ciudad-' + c.id;
        var viejo = document.getElementById(tplId);
        if (viejo) viejo.remove();
        var tpl = document.createElement('template');
        tpl.id = tplId;
        var lineaTam = GM.iconoTamano(c.tamano) + ' ' + (c.tamano || 'ciudad') +
            (c.nacion ? ' · ' + c.nacion : '') + (c.poblacion ? ' · ' + c.poblacion + ' hab.' : '');
        var detalle = '';
        if (c.gobernante || c.rasgo || c.descripcion) {
            detalle = '<div class="gm-panel-section is-expanded"><button type="button" class="gm-panel-section__toggle">La ciudad ▾</button><div class="gm-panel-section__content">' +
                (c.gobernante ? '<p><strong>Gobierna:</strong> ' + c.gobernante + '</p>' : '') +
                (c.rasgo ? '<p><strong>Rasgo:</strong> ' + c.rasgo + '</p>' : '') +
                (c.descripcion ? '<p>' + c.descripcion + '</p>' : '') + '</div></div>';
        }
        tpl.innerHTML =
            '<p class="gm-hint">' + lineaTam + '</p>' + detalle +
            '<div class="gm-panel-section is-expanded gm-ciudad-editor" data-ciudad-id="' + c.id + '">' +
            '<button type="button" class="gm-panel-section__toggle">✏️ Editar ciudad ▾</button>' +
            '<div class="gm-panel-section__content">' +
            '<p class="gm-hint u-mt-0">Historias que ocurren aquí</p><div class="gm-ciudad-editor__historias"></div>' +
            '<div class="gm-punto-editor__add-row"><input type="text" list="gm-historia-datalist" class="gm-input gm-punto-editor__input" placeholder="Buscar historia por título…">' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-ciudad-editor__add-historia">+ Añadir</button></div>' +
            '<p class="gm-hint">PNJs de la ciudad</p><div class="gm-ciudad-editor__npcs"></div>' +
            '<div class="gm-punto-editor__add-row"><input type="text" list="gm-npc-datalist" class="gm-input gm-punto-editor__input" placeholder="Buscar PNJ por nombre…">' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-ciudad-editor__add-npc">+ Añadir</button></div>' +
            '<div class="gm-punto-editor__acciones">' +
            '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-ciudad-editor__editar-campos">✏️ Nombre y detalles</button>' +
            '<button type="button" class="gm-btn gm-btn--danger gm-btn--sm gm-ciudad-editor__eliminar">🗑️ Eliminar ciudad</button>' +
            '</div></div></div>';
        document.body.appendChild(tpl);
    }


    GM.setPlacingCiudad = function setPlacingCiudad(v) {
        GM.placingCiudad = v;
        if (v) { GM.setPlacingPunto(false); GM.setPlacingZona(false); }
        GM.lienzo.classList.toggle('is-placing', v || GM.placingPunto || GM.placingZona);
        if (GM.nuevaCiudadBtn) GM.nuevaCiudadBtn.classList.toggle('gm-btn--primary', v);
        GM.coordsLabel.textContent = v ? '🏙 Clic en el mapa para situar la ciudad' : '';
    }


    // Formulario modal de ciudad: solo el nombre (y el gobernante, que es un nombre
    // propio) se escriben; nación, tamaño, población y rasgo se ELIGEN de listas.
    // Sustituye a la cadena de prompt() que invitaba a rellenar basura ("sdff").
    GM.formularioCiudad = function formularioCiudad(titulo, prefill, x, y, onOk) {
        var nacionSugerida = prefill.nacion || GM.nacionEnPunto(x, y) || '';
        var poblaciones = [''].concat(GM.GM_TAMANOS.capital.pobl, GM.GM_TAMANOS.ciudad.pobl,
            GM.GM_TAMANOS.pueblo.pobl, GM.GM_TAMANOS.aldea.pobl);
        GM.abrirDialogo(titulo, [
            { clave: 'nombre', etiqueta: 'Nombre', tipo: 'texto', requerido: true,
              dado: function (v) { return GM.generarNombreCiudad(v.nacion || nacionSugerida); } },
            { clave: 'nacion', etiqueta: 'Nación', tipo: 'select', opciones: [''].concat(GM.nacionesDisponibles()),
              vacio: 'sin nación', ayuda: nacionSugerida ? 'Detectada por posición: ' + nacionSugerida : '' },
            { clave: 'tamano', etiqueta: 'Tamaño', tipo: 'select', opciones: [
                { v: 'capital', t: '🏰 Capital' }, { v: 'ciudad', t: '🏙 Ciudad' },
                { v: 'pueblo', t: '🏘 Pueblo' }, { v: 'aldea', t: '🏡 Aldea' }] },
            { clave: 'poblacion', etiqueta: 'Población', tipo: 'select-otro', opciones: poblaciones,
              vacio: 'sin especificar',
              dado: function (v) { return GM.azar((GM.GM_TAMANOS[v.tamano] || GM.GM_TAMANOS.ciudad).pobl); } },
            { clave: 'gobernante', etiqueta: 'Quién gobierna', tipo: 'texto',
              dado: function () { return GM.azar(GM.GM_TITULOS) + ' ' + GM.azar(GM.GM_NOMBRES_PERS); } },
            { clave: 'rasgo', etiqueta: 'Rasgo distintivo', tipo: 'select-otro',
              opciones: [''].concat(GM.GM_RASGOS), vacio: 'sin rasgo',
              dado: function () { return GM.azar(GM.GM_RASGOS); } },
            { clave: 'descripcion', etiqueta: 'Descripción breve', tipo: 'area' }
        ], Object.assign({ nacion: nacionSugerida, tamano: 'ciudad' }, prefill), onOk);
    }


    // ¿Sobre qué nación cae un punto? (point-in-polygon simple sobre los polígonos SVG).
    GM.nacionEnPunto = function nacionEnPunto(x, y) {
        for (var i = 0; i < GM.polygons.length; i++) {
            var pts = GM.polygons[i].getAttribute('points').trim().split(/\s+/).map(function (par) {
                var c = par.split(','); return { x: +c[0], y: +c[1] };
            });
            var dentro = false;
            for (var a = 0, b = pts.length - 1; a < pts.length; b = a++) {
                if (((pts[a].y > y) !== (pts[b].y > y)) &&
                    (x < (pts[b].x - pts[a].x) * (y - pts[a].y) / (pts[b].y - pts[a].y) + pts[a].x)) dentro = !dentro;
            }
            if (dentro) return GM.polygons[i].dataset.nacion;
        }
        return '';
    }


    GM.colocarNuevaCiudad = function colocarNuevaCiudad(evt, aleatoria) {
        var p = GM.toSvgPoint(evt);
        GM.setPlacingCiudad(false);
        function crear(datos) {
            var id = 'ciudad_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
            GM.crearMarcadorCiudad(Object.assign({ id: id, x: Math.round(p.x), y: Math.round(p.y), historiaIds: [], npcIds: [] }, datos));
            GM.saveDraft();
        }
        if (aleatoria) {
            var nac = GM.nacionEnPunto(p.x, p.y) || GM.azar(GM.nacionesDisponibles());
            var tam = GM.azar(['capital', 'ciudad', 'ciudad', 'pueblo', 'pueblo', 'aldea']); // ciudades y pueblos más frecuentes
            crear({
                nombre: GM.generarNombreCiudad(nac), tamano: tam, nacion: nac,
                poblacion: GM.azar(GM.GM_TAMANOS[tam].pobl),
                gobernante: GM.azar(GM.GM_TITULOS) + ' ' + GM.azar(GM.GM_NOMBRES_PERS),
                rasgo: GM.azar(GM.GM_RASGOS), descripcion: ''
            });
        } else {
            GM.formularioCiudad('🏙 Nueva ciudad', {}, p.x, p.y, crear);
        }
    }


    // Editor de ciudad dentro del panel de vista previa (delegación de eventos).
    GM.ciudadPorId = function ciudadPorId(id) { return GM.markers.find(function (m) { return m.dataset.ciudadId === id; }); }


    GM.pintarChipsCiudad = function pintarChipsCiudad(editor) {
        var id = editor.dataset.ciudadId;
        var m = GM.ciudadPorId(id);
        if (!m) return;
        function chips(cont, csv, mapaNombres, quitarCb) {
            cont.innerHTML = '';
            (csv || '').split(',').filter(Boolean).forEach(function (val) {
                var chip = document.createElement('span');
                chip.className = 'gm-badge';
                chip.style.cssText = 'margin:0 4px 4px 0; display:inline-flex; align-items:center; gap:4px;';
                chip.textContent = (mapaNombres[val] || val) + ' ';
                var x = document.createElement('button');
                x.type = 'button'; x.textContent = '×';
                x.style.cssText = 'border:none; background:none; cursor:pointer; font-weight:700;';
                x.addEventListener('click', function () { quitarCb(val); });
                chip.appendChild(x);
                cont.appendChild(chip);
            });
        }
        chips(editor.querySelector('.gm-ciudad-editor__historias'), m.dataset.historiaIds, GM.GM_HISTORIAS, function (val) {
            m.dataset.historiaIds = (m.dataset.historiaIds || '').split(',').filter(function (v) { return v && v !== val; }).join(',');
            GM.pintarChipsCiudad(editor); GM.saveDraft();
        });
        chips(editor.querySelector('.gm-ciudad-editor__npcs'), m.dataset.npcIds, GM.GM_NPCS, function (val) {
            m.dataset.npcIds = (m.dataset.npcIds || '').split(',').filter(function (v) { return v && v !== val; }).join(',');
            GM.pintarChipsCiudad(editor); GM.saveDraft();
        });
    }


    GM.idDesdeInput = function idDesdeInput(input) {
        var m = /\[([^\]]+)\]\s*$/.exec(input.value.trim());
        return m ? m[1] : null;
    }


    // ======================================================================
    //  CREADOR DE CIUDADES
    // ======================================================================
    GM.nuevaCiudadBtn = document.getElementById('gm-editor-nueva-ciudad');

    GM.placingCiudad = false;


    GM.GM_TAMANOS = {
        capital: { icono: '🏰', pobl: ['~18.000', '~25.000', 'más de 30.000'] },
        ciudad:  { icono: '🏙', pobl: ['~6.000', '~9.000', '~12.000'] },
        pueblo:  { icono: '🏘', pobl: ['~800', '~1.500', 'unas 2.000 almas'] },
        aldea:   { icono: '🏡', pobl: ['unas 80 almas', '~150', '~300'] }
    };

    // Bancos de nombres por raíz cultural de cada nación, para que el 🎲 dé topónimos
    // con sabor coherente. Si la nación no está mapeada, usa el banco genérico.
    GM.GM_SILABAS = {
        _gen:      { pre: ['Val', 'Mon', 'Cas', 'Puerto', 'Alto', 'Bel', 'Ríon', 'Ven'], suf: ['tera', 'brío', 'gar', 'donde', 'mar', 'vera', 'caña', 'lén'] },
        Ecla:      { pre: ['Sil', 'Ver', 'Fron', 'Rai', 'Hoja', 'Musgo'], suf: ['dara', 'nel', 'via', 'thal', 'wen', 'ren'] },
        Ostad:     { pre: ['Hierro', 'Yun', 'Mar', 'Forja', 'Kald', 'Grün'], suf: ['stad', 'burgo', 'heim', 'gard', 'holm', 'fort'] },
        Giladdokx: { pre: ['Xil', 'Zok', 'Grah', 'Vorn', 'Dokx', 'Kra'], suf: ['gorl', 'dokx', 'zar', 'ruth', 'gan', 'mok'] },
        Udrax:     { pre: ['Udr', 'Pie', 'Roca', 'Cav', 'Grum', 'Bara'], suf: ['dax', 'ax', 'delve', 'fund', 'kar', 'dûm'] },
        Bastrea:   { pre: ['Bas', 'Puerto', 'Vela', 'Cala', 'San', 'Mira'], suf: ['trea', 'mar', 'bahía', 'flor', 'lucía', 'gales'] },
        Ascaria:   { pre: ['Asc', 'Vel', 'Luz', 'Cre', 'Aur', 'Fae'], suf: ['aria', 'wyn', 'lume', 'thys', 'ndel', 'sar'] },
        Wulcain:   { pre: ['Wul', 'Garra', 'Colmillo', 'Nie', 'Skal', 'Hurl'], suf: ['cain', 'grim', 'fang', 'held', 'run', 'ward'] }
    };

    GM.GM_RASGOS = ['puerto de contrabando', 'mercado de reliquias', 'ruinas bajo la plaza',
        'gremio poderoso', 'santuario en discordia', 'guarnición corrupta', 'feria anual famosa',
        'minas agotándose', 'refugio de exiliados', 'puente estratégico', 'biblioteca prohibida',
        'pozo que susurra', 'muralla a medio caer', 'canal de aguas negras', 'colina coronada de menhires'];

    GM.GM_TITULOS = ['Alcalde', 'Baronesa', 'Consejo de', 'Capitán', 'Matriarca', 'Prior', 'Regidora', 'Señor'];

    GM.GM_NOMBRES_PERS = ['Aldric', 'Nara', 'Bruna', 'Damaso', 'Iru', 'Tonet', 'Velia', 'Sibil', 'Ferran', 'Ona', 'Gorlak', 'Eudald'];

})(window.GMMapa);
