/*
 * dependencias.js — Sandbox de fichas de la pantalla de dependencias (WS-E · E11, 13a)
 *
 * Simula EN CLIENTE el filtrado del mazo por actos del GDD §20.3, el mismo criterio que
 * ValidadorAventuraService#simularSupervivientes usa en servidor para el peor caso:
 *   · BASE / INYECTADA / CADENA entran siempre en el mazo de su acto.
 *   · CONDICIONAL entra solo si la carta de su requisito tiene marcada la ficha exigida.
 *   · BALANCE_FINAL se juega aparte (nunca se apaga).
 * Las ramas ↳ de las cartas (Inyectadas) se resaltan cuando su condición se cumple.
 *
 * Los datos llegan en window.DEP_CARTAS (catálogo ligero servido por
 * AventuraController#dependencias). Sin llamadas al servidor: todo el estado vive aquí.
 */
(function () {
    'use strict';

    var CARTAS = window.DEP_CARTAS || [];
    if (!CARTAS.length) return;

    /** Estado del sandbox: code → 'VERDE' | 'ROJA' (ausente = sin resolver). */
    var fichas = {};

    var porCodigo = {};
    CARTAS.forEach(function (c) { if (c.code) porCodigo[c.code] = c; });

    // ---- fichas: el botón de cada carta cicla — → Verde → Roja → — ----

    function ciclar(code) {
        if (!fichas[code]) fichas[code] = 'VERDE';
        else if (fichas[code] === 'VERDE') fichas[code] = 'ROJA';
        else delete fichas[code];
    }

    function pintarFicha(btn) {
        var estado = fichas[btn.dataset.ficha];
        btn.classList.remove('dp-ficha--none', 'dp-ficha--verde', 'dp-ficha--roja');
        if (estado === 'VERDE') { btn.classList.add('dp-ficha--verde'); btn.textContent = 'Verde'; }
        else if (estado === 'ROJA') { btn.classList.add('dp-ficha--roja'); btn.textContent = 'Roja'; }
        else { btn.classList.add('dp-ficha--none'); btn.textContent = '—'; }
    }

    // ---- filtrado §20.3 ----

    /** true si la carta entraría hoy en el mazo de su acto con las fichas marcadas. */
    function encendida(carta) {
        if (carta.tipo !== 'CONDICIONAL') return true;
        if (!carta.req || !carta.req.code) return true; // sin requisito → el validador ya lo marca
        return fichas[carta.req.code] === carta.req.estado;
    }

    function repintar() {
        document.querySelectorAll('.dp-ficha[data-ficha]').forEach(pintarFicha);

        var apagadas = 0;
        CARTAS.forEach(function (carta) {
            if (!carta.code) return;
            var nodo = document.querySelector('.dp-card[data-code="' + carta.code + '"]');
            if (!nodo) return;
            var on = encendida(carta);
            nodo.classList.toggle('dp-card--off', !on);
            // resalte suave solo para condicionales que han pasado el filtro gracias a una ficha
            nodo.classList.toggle('dp-card--on', on && carta.tipo === 'CONDICIONAL');
            if (!on) apagadas++;
        });

        document.querySelectorAll('.dp-rama[data-rama-code]').forEach(function (el) {
            var cumple = fichas[el.dataset.ramaCode] === el.dataset.ramaEstado;
            el.classList.toggle('dp-rama--activa', cumple);
        });

        var marcadas = Object.keys(fichas).length;
        var resumen = document.getElementById('dp-resumen');
        if (resumen) {
            resumen.textContent = marcadas === 0
                ? 'Sin fichas marcadas: se muestra el mazo completo.'
                : marcadas + ' ficha(s) marcada(s) · ' + apagadas + ' carta(s) filtrada(s) fuera.';
        }
    }

    // ---- eventos ----

    document.addEventListener('click', function (ev) {
        var btn = ev.target.closest('.dp-ficha[data-ficha]');
        if (btn) {
            ciclar(btn.dataset.ficha);
            repintar();
            return;
        }
        var salto = ev.target.closest('.dp-inc-code[data-goto]');
        if (salto) {
            ev.preventDefault();
            var nodo = document.querySelector('.dp-card[data-code="' + salto.dataset.goto + '"]');
            if (nodo) {
                nodo.scrollIntoView({ behavior: 'smooth', block: 'center' });
                nodo.classList.add('dp-card--hilite');
                setTimeout(function () { nodo.classList.remove('dp-card--hilite'); }, 1600);
            }
        }
    });

    function marcarTodas(estado) {
        fichas = {};
        if (estado) {
            CARTAS.forEach(function (c) {
                // El Balance Final no lleva ficha: no es una carta que se resuelva Verde/Roja.
                if (c.code && c.tipo !== 'BALANCE_FINAL') fichas[c.code] = estado;
            });
        }
        repintar();
    }

    var btnVerde = document.getElementById('dp-todo-verde');
    var btnRoja = document.getElementById('dp-todo-roja');
    var btnReset = document.getElementById('dp-reset');
    if (btnVerde) btnVerde.addEventListener('click', function () { marcarTodas('VERDE'); });
    if (btnRoja) btnRoja.addEventListener('click', function () { marcarTodas('ROJA'); });
    if (btnReset) btnReset.addEventListener('click', function () { marcarTodas(null); });

    repintar();
})();
