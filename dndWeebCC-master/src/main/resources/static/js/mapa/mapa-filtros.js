/*
 * mapa-filtros.js — Filtres per nació, facció i tier
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.aplicarFiltros = function aplicarFiltros() {
        document.querySelectorAll('.gm-mapa-nacion__borde polygon[data-nacion], .gm-mapa-nacion__etiqueta text[data-nacion]')
            .forEach(function (el) {
                el.classList.toggle('gm-mapa-oculto', GM.hiddenNaciones.has(el.dataset.nacion));
            });
        GM.markers.forEach(function (m) {
            var oculto = false;
            if (m.dataset.regionId && GM.hiddenFacciones.has(m.dataset.regionId)) oculto = true;
            var tiers = (m.dataset.tiers || '').split(',').filter(Boolean);
            if (tiers.length && tiers.every(function (t) { return GM.hiddenTiers.has(t); })) oculto = true;
            m.classList.toggle('gm-mapa-oculto', oculto);
        });
    }


    // ============ FILTROS: leyenda clicable (naciones/facciones) + tier ============
    // Solo esconden visualmente (display:none) — no tocan los datos, así que no hace
    // falta guardar nada para usarlos, y se resetean solos al recargar la página.
    GM.hiddenNaciones = new Set();

    GM.hiddenFacciones = new Set();

    GM.hiddenTiers = new Set();


    GM.tierTodosBtn = document.getElementById('gm-tier-todos');
})(window.GMMapa);
