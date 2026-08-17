/*
 * theme.js — Piloto de rediseño visual
 * Alterna el tema de página (día/noche) guardando la preferencia en localStorage.
 * Nunca toca la paleta de las cartas (esa es fija a propósito, ver tokens.css).
 */
(function () {
    var STORAGE_KEY = 'gm-theme';
    var root = document.documentElement;

    function temaGuardado() {
        try {
            return window.localStorage.getItem(STORAGE_KEY);
        } catch (e) {
            return null;
        }
    }

    function guardarTema(tema) {
        try {
            window.localStorage.setItem(STORAGE_KEY, tema);
        } catch (e) {
            /* localStorage no disponible: el tema simplemente no persiste entre visitas */
        }
    }

    function aplicarTema(tema) {
        root.setAttribute('data-theme', tema);
        document.querySelectorAll('[data-theme-toggle]').forEach(function (boton) {
            boton.textContent = tema === 'noche' ? '☀️' : '🌙';
            boton.setAttribute('aria-label', tema === 'noche' ? 'Cambiar a modo día' : 'Cambiar a modo noche');
        });
    }

    var inicial = temaGuardado() || 'dia';
    aplicarTema(inicial);

    document.addEventListener('click', function (evento) {
        var boton = evento.target.closest('[data-theme-toggle]');
        if (!boton) {
            return;
        }
        var actual = root.getAttribute('data-theme') === 'noche' ? 'noche' : 'dia';
        var siguiente = actual === 'noche' ? 'dia' : 'noche';
        aplicarTema(siguiente);
        guardarTema(siguiente);
    });
})();
