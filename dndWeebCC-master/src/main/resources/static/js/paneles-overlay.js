/*
 * paneles-overlay.js — Paneles modulares superpuestos a un lienzo.
 *
 * Interacción por paneles: un rail de iconos encima del mapa abre y cierra
 * paneles acoplados al lado izquierdo o derecho DEL LIENZO (no ventanas
 * flotantes sueltas). Cada panel puede plegarse al rail (✕ del panel o su
 * icono) y cambiarse de lado (⇄). El estado (abierto/cerrado, lado) se
 * guarda en localStorage.
 *
 * API pública (window.GmPaneles):
 *   init(workspace)                          → prepara rail y contenedores
 *   crear({ id, icono, titulo, lado, nodos, abierto }) → panel
 *
 * Mejora progresiva: si este script no carga, los bloques originales siguen
 * en el flujo normal de la página.
 */
(function () {
    'use strict';

    var CLAVE = 'gm-paneles-v1';

    function leerEstados() {
        try { return JSON.parse(localStorage.getItem(CLAVE)) || {}; }
        catch (e) { return {}; }
    }

    function guardarEstado(id, parcial) {
        var estados = leerEstados();
        estados[id] = Object.assign({}, estados[id] || {}, parcial);
        try { localStorage.setItem(CLAVE, JSON.stringify(estados)); } catch (e) { /* nada */ }
    }

    var workspace = null;
    var rail = null;
    var lados = {};

    function init(ws) {
        workspace = ws;

        rail = document.createElement('div');
        rail.className = 'gm-dock-rail';
        rail.setAttribute('role', 'toolbar');
        rail.setAttribute('aria-label', 'Paneles del mapa');

        lados.izq = document.createElement('div');
        lados.izq.className = 'gm-dock-lado gm-dock-lado--izq';
        lados.der = document.createElement('div');
        lados.der.className = 'gm-dock-lado gm-dock-lado--der';

        workspace.appendChild(rail);
        workspace.appendChild(lados.izq);
        workspace.appendChild(lados.der);
    }

    /**
     * Crea un panel acoplado y mueve dentro los nodos indicados
     * (moverlos conserva sus listeners).
     * def = { id, icono, titulo, lado: 'izq'|'der', nodos: [Node...], abierto }
     */
    function crear(def) {
        if (!workspace) return null;
        var guardado = leerEstados()[def.id] || {};
        var lado = (guardado.lado === 'izq' || guardado.lado === 'der') ? guardado.lado : (def.lado || 'izq');
        var abierto = (typeof guardado.abierto === 'boolean') ? guardado.abierto : !!def.abierto;

        // ---- panel ----
        var panel = document.createElement('section');
        panel.className = 'gm-dock-panel';
        panel.id = 'dock-' + def.id;
        panel.setAttribute('role', 'region');
        panel.setAttribute('aria-label', def.titulo);

        var cabecera = document.createElement('div');
        cabecera.className = 'gm-dock-panel__header';

        var titulo = document.createElement('h3');
        titulo.className = 'gm-dock-panel__titulo';
        titulo.textContent = (def.icono ? def.icono + ' ' : '') + def.titulo;

        var btnLado = document.createElement('button');
        btnLado.type = 'button';
        btnLado.className = 'gm-dock-panel__boton';
        btnLado.title = 'Cambiar de lado';
        btnLado.textContent = '⇄';

        var btnCerrar = document.createElement('button');
        btnCerrar.type = 'button';
        btnCerrar.className = 'gm-dock-panel__boton';
        btnCerrar.title = 'Plegar al rail';
        btnCerrar.textContent = '✕';

        cabecera.appendChild(titulo);
        cabecera.appendChild(btnLado);
        cabecera.appendChild(btnCerrar);

        var cuerpo = document.createElement('div');
        cuerpo.className = 'gm-dock-panel__body';
        (def.nodos || []).forEach(function (nodo) {
            if (nodo) cuerpo.appendChild(nodo);
        });

        panel.appendChild(cabecera);
        panel.appendChild(cuerpo);
        lados[lado].appendChild(panel);
        panel.classList.toggle('is-cerrado', !abierto);

        // ---- botón del rail ----
        var btnRail = document.createElement('button');
        btnRail.type = 'button';
        btnRail.className = 'gm-dock-rail__btn';
        btnRail.textContent = def.icono || '▦';
        btnRail.title = def.titulo;
        btnRail.setAttribute('aria-pressed', abierto ? 'true' : 'false');
        btnRail.classList.toggle('is-activo', abierto);
        rail.appendChild(btnRail);

        function poner(estadoAbierto) {
            panel.classList.toggle('is-cerrado', !estadoAbierto);
            btnRail.classList.toggle('is-activo', estadoAbierto);
            btnRail.setAttribute('aria-pressed', estadoAbierto ? 'true' : 'false');
            guardarEstado(def.id, { abierto: estadoAbierto });
        }

        btnRail.addEventListener('click', function () {
            poner(panel.classList.contains('is-cerrado'));
        });
        btnCerrar.addEventListener('click', function () { poner(false); });

        btnLado.addEventListener('click', function () {
            var nuevo = (panel.parentNode === lados.izq) ? 'der' : 'izq';
            lados[nuevo].appendChild(panel);
            guardarEstado(def.id, { lado: nuevo });
        });

        return panel;
    }

    window.GmPaneles = { init: init, crear: crear };
})();
