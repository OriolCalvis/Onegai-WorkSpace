/*
 * mapa-nucleo.js — Estat compartit, referències del DOM i utilitats de datalist
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
window.GMMapa = window.GMMapa || {};

(function (GM) {
    'use strict';

    // id → etiqueta, sacado de los <option> de los buscadores compartidos (evita
    // mandar los catálogos dos veces al navegador: una para el <datalist>, otra en JS).
    GM.mapaDesdeDatalist = function mapaDesdeDatalist(datalistId) {
        var mapa = {};
        document.querySelectorAll('#' + datalistId + ' option').forEach(function (opt) {
            var m = /^(.*) \[([^\]]+)\]$/.exec(opt.value);
            if (m) mapa[m[2]] = m[1];
        });
        return mapa;
    }

    // id → tiers (csv), para recalcular en vivo el filtro de tier de un punto
    // personalizado cada vez que se le añade/quita una historia o evento desde su editor.
    GM.tiersDesdeDatalist = function tiersDesdeDatalist(datalistId) {
        var mapa = {};
        document.querySelectorAll('#' + datalistId + ' option').forEach(function (opt) {
            var m = /^(.*) \[([^\]]+)\]$/.exec(opt.value);
            if (m) mapa[m[2]] = opt.dataset.tiers || '';
        });
        return mapa;
    }

    GM.extraerIdDeDatalist = function extraerIdDeDatalist(texto) {
        var m = /\[([^\]]+)\]\s*$/.exec((texto || '').trim());
        return m ? m[1] : null;
    }


    // Recalcula data-tiers de un punto como la unión de los tiers de todas sus
    // historias y eventos vinculados (usa GM_HISTORIA_TIERS / GM_EVENTO_TIERS, ya en memoria).
    GM.recalcularTiersDePunto = function recalcularTiersDePunto(marker) {
        var idsHistorias = (marker.dataset.historiaIds || '').split(',').filter(Boolean);
        var idsEventos = (marker.dataset.eventoIds || '').split(',').filter(Boolean);
        var tiers = new Set();
        idsHistorias.forEach(function (id) {
            (GM.GM_HISTORIA_TIERS[id] || '').split(',').filter(Boolean).forEach(function (t) { tiers.add(t); });
        });
        idsEventos.forEach(function (id) {
            (GM.GM_EVENTO_TIERS[id] || '').split(',').filter(Boolean).forEach(function (t) { tiers.add(t); });
        });
        marker.dataset.tiers = Array.from(tiers).sort(function (a, b) { return a - b; }).join(',');
    }


    GM.LS_KEY = 'gm-mapa-editor-draft-v1';

    GM.lienzo = document.querySelector('.gm-mapa-lienzo');

    GM.svg = document.getElementById('gm-mapa-svg');

    GM.handlesGroup = document.getElementById('gm-editor-handles');

    GM.toggleBtn = document.getElementById('gm-editor-toggle');

    GM.saveBtn = document.getElementById('gm-editor-save');

    GM.nuevoPuntoBtn = document.getElementById('gm-editor-nuevo-punto');

    GM.resetBtn = document.getElementById('gm-editor-reset');

    GM.exportBtn = document.getElementById('gm-editor-export');

    GM.exportPanel = document.getElementById('gm-editor-export-panel');

    GM.exportText = document.getElementById('gm-editor-export-text');

    GM.coordsLabel = document.getElementById('gm-editor-coords');

    GM.banner = document.getElementById('gm-editor-banner');

    GM.bannerDismiss = document.getElementById('gm-editor-banner-dismiss');

    GM.svgNS = 'http://www.w3.org/2000/svg';


    GM.polygons = Array.from(document.querySelectorAll('.gm-mapa-nacion__borde polygon'));

    GM.markers = Array.from(document.querySelectorAll('.gm-mapa-marcador'));

    GM.editing = false;

    GM.placingPunto = false;

    GM.dragging = null; // { type: 'vertex', poly, vertex } | { type: 'marker', el, id }


    GM.GM_HISTORIAS = GM.mapaDesdeDatalist('gm-historia-datalist');

    GM.GM_AVENTURAS = GM.mapaDesdeDatalist('gm-aventura-datalist');

    GM.GM_EVENTOS = GM.mapaDesdeDatalist('gm-evento-datalist');

    GM.GM_NPCS = GM.mapaDesdeDatalist('gm-npc-datalist');


    GM.GM_HISTORIA_TIERS = GM.tiersDesdeDatalist('gm-historia-datalist');

    GM.GM_EVENTO_TIERS = GM.tiersDesdeDatalist('gm-evento-datalist');

})(window.GMMapa);
