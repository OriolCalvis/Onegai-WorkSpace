/*
 * mapa-cronologia.js — Esdeveniments històrics del món
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.contCronologia = function contCronologia() { return document.getElementById('gm-cronologia'); }


    GM.lugaresDelMundo = function lugaresDelMundo() {
        var lugares = [''];
        GM.polygons.forEach(function (p) { lugares.push(p.dataset.nacion); });
        GM.markers.forEach(function (m) {
            if (m.dataset.ciudadId) lugares.push(m.dataset.ciudadNombre);
            if (m.dataset.zonaId) lugares.push(m.dataset.zonaNombre);
        });
        return lugares.filter(function (v, i, a) { return v !== undefined && a.indexOf(v) === i; });
    }


    GM.erasExistentes = function erasExistentes() {
        var eras = [''];
        GM.contCronologia().querySelectorAll('[data-evento-historico-id]').forEach(function (item) {
            if (item.dataset.ehEra) eras.push(item.dataset.ehEra);
        });
        return eras.filter(function (v, i, a) { return a.indexOf(v) === i; });
    }


    GM.formularioEventoHistorico = function formularioEventoHistorico(titulo, prefill, onOk) {
        GM.abrirDialogo(titulo, [
            { clave: 'titulo', etiqueta: 'Título', tipo: 'texto', requerido: true },
            { clave: 'ano', etiqueta: 'Año (negativo = antes de la era actual)', tipo: 'numero', requerido: true },
            { clave: 'era', etiqueta: 'Era', tipo: 'select-otro', opciones: GM.erasExistentes(), vacio: 'sin era' },
            { clave: 'lugar', etiqueta: 'Dónde ocurrió', tipo: 'select', opciones: GM.lugaresDelMundo(),
              vacio: 'lugar olvidado', ayuda: 'Naciones, ciudades y zonas del mapa actual.' },
            { clave: 'descripcion', etiqueta: 'Qué pasó', tipo: 'area' },
            { clave: 'consecuencias', etiqueta: 'Qué queda hoy (lo jugable)', tipo: 'area' }
        ], prefill, onOk);
    }


    GM.crearItemCronologia = function crearItemCronologia(datos) {
        var cont = GM.contCronologia();
        var viejo = cont.querySelector('[data-evento-historico-id="' + datos.id + '"]');
        if (viejo) viejo.remove();

        var item = document.createElement('div');
        item.className = 'gm-cronologia__item';
        item.dataset.eventoHistoricoId = datos.id;
        item.dataset.ehTitulo = datos.titulo;
        item.dataset.ehAno = datos.ano;
        item.dataset.ehEra = datos.era || '';
        item.dataset.ehLugar = datos.lugar || '';
        item.dataset.ehDescripcion = datos.descripcion || '';
        item.dataset.ehConsecuencias = datos.consecuencias || '';

        var linea = document.createElement('p');
        var ano = document.createElement('span');
        ano.className = 'gm-cronologia__ano';
        ano.textContent = 'Año ' + datos.ano;
        linea.appendChild(ano);
        linea.appendChild(document.createTextNode(' '));
        var tit = document.createElement('strong');
        tit.textContent = datos.titulo;
        linea.appendChild(tit);
        if (datos.era) { var era = document.createElement('span'); era.className = 'gm-hint'; era.textContent = ' · ' + datos.era; linea.appendChild(era); }
        if (datos.lugar) { var lug = document.createElement('span'); lug.className = 'gm-hint'; lug.textContent = ' · 📍 ' + datos.lugar; linea.appendChild(lug); }
        item.appendChild(linea);
        if (datos.descripcion) { var de = document.createElement('p'); de.className = 'gm-hint'; de.textContent = datos.descripcion; item.appendChild(de); }
        if (datos.consecuencias) { var cq = document.createElement('p'); cq.className = 'gm-hint'; cq.textContent = 'Hoy queda: ' + datos.consecuencias; item.appendChild(cq); }

        var acciones = document.createElement('span');
        acciones.className = 'gm-eh-acciones';
        acciones.hidden = !GM.editing;
        acciones.innerHTML = '<button type="button" class="gm-btn gm-btn--outline gm-btn--sm gm-eh-editar">✏️</button> ' +
            '<button type="button" class="gm-btn gm-btn--danger gm-btn--sm gm-eh-eliminar">🗑️</button>';
        item.appendChild(acciones);

        // Insertado en orden cronológico (por año)
        var anoNuevo = parseInt(datos.ano, 10) || 0;
        var siguiente = null;
        cont.querySelectorAll('[data-evento-historico-id]').forEach(function (otro) {
            if (siguiente) return;
            if ((parseInt(otro.dataset.ehAno, 10) || 0) > anoNuevo) siguiente = otro;
        });
        cont.insertBefore(item, siguiente);
        var vacia = document.getElementById('gm-cronologia-vacia');
        if (vacia) vacia.hidden = true;
        return item;
    }


    GM.eventoHistoricoDesdeItem = function eventoHistoricoDesdeItem(item) {
        return {
            id: item.dataset.eventoHistoricoId,
            titulo: item.dataset.ehTitulo,
            ano: parseInt(item.dataset.ehAno, 10) || 0,
            era: item.dataset.ehEra || '',
            lugar: item.dataset.ehLugar || '',
            descripcion: item.dataset.ehDescripcion || '',
            consecuencias: item.dataset.ehConsecuencias || ''
        };
    }


    GM.eventosHistoricosDesdeDom = function eventosHistoricosDesdeDom() {
        return Array.from(GM.contCronologia().querySelectorAll('[data-evento-historico-id]')).map(GM.eventoHistoricoDesdeItem);
    }


    // ======================================================================
    //  EVENTOS HISTÓRICOS — cronología del mundo
    // ======================================================================
    GM.nuevoEventoHistBtn = document.getElementById('gm-editor-nuevo-evento-hist');

})(window.GMMapa);
