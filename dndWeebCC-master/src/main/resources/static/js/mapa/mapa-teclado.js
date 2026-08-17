/*
 * mapa-teclado.js — Accessibilitat: obre els marcadors amb Enter i Espai
 *
 * Mòdul del Mapa Mundi. Veure mapa-nucleo.js per a l'espai de noms compartit.
 */
(function () {
    'use strict';

    // panels.js abre con click; los marcadores del mapa son <g> con tabindex,
    // así que también deben abrirse con Enter/Espacio para ser accesibles por teclado.
    document.querySelectorAll('.gm-mapa-marcador').forEach(function (marcador) {
        marcador.addEventListener('keydown', function (evento) {
            if (evento.key === 'Enter' || evento.key === ' ') {
                evento.preventDefault();
                marcador.dispatchEvent(new MouseEvent('click', { bubbles: true }));
            }
        });
    });
})();
