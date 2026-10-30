/*
 * mapa-dialogo.js — Diàleg modal reutilitzable del editor
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    GM.cerrarDialogo = function cerrarDialogo() { GM.dlg.hidden = true; GM.dlgFondo.hidden = true; GM.dlgAccion = null; }

    GM.esc = function esc(s) { var d = document.createElement('div'); d.textContent = s || ''; return d.innerHTML; }


    // campos: [{clave, etiqueta, tipo: 'texto'|'numero'|'area'|'select'|'select-otro',
    //           opciones, vacio, dado(fn→valor aleatorio), requerido, ayuda}]
    // 'select-otro' añade la opción "✏️ Otro…" que despliega un texto libre — para
    // campos narrativos con banco de sugerencias; los de referencia usan 'select' puro.
    GM.abrirDialogo = function abrirDialogo(titulo, campos, valores, onOk) {
        GM.dlgTitulo.textContent = titulo;
        GM.dlgCampos.innerHTML = '';
        var controles = {};

        campos.forEach(function (campo) {
            var field = document.createElement('div');
            field.className = 'gm-field';
            var label = document.createElement('label');
            label.textContent = campo.etiqueta;
            field.appendChild(label);
            var fila = document.createElement('div');
            fila.className = 'gm-dialogo__fila';
            var control, otroInput = null;
            var valor = valores[campo.clave] != null ? String(valores[campo.clave]) : '';

            if (campo.tipo === 'select' || campo.tipo === 'select-otro') {
                control = document.createElement('select');
                control.className = 'gm-select';
                (campo.opciones || []).forEach(function (op) {
                    var o = document.createElement('option');
                    if (typeof op === 'string') {
                        o.value = op;
                        o.textContent = op || ('— ' + (campo.vacio || 'sin especificar') + ' —');
                    } else { o.value = op.v; o.textContent = op.t; }
                    control.appendChild(o);
                });
                if (campo.tipo === 'select-otro') {
                    var oOtro = document.createElement('option');
                    oOtro.value = '__otro__';
                    oOtro.textContent = '✏️ Otro…';
                    control.appendChild(oOtro);
                    otroInput = document.createElement('input');
                    otroInput.type = 'text';
                    otroInput.className = 'gm-input gm-dialogo__otro';
                    otroInput.placeholder = 'Escribe el valor…';
                    otroInput.hidden = true;
                    control.addEventListener('change', function () {
                        otroInput.hidden = control.value !== '__otro__';
                        if (!otroInput.hidden) otroInput.focus();
                    });
                }
                var existe = Array.prototype.some.call(control.options, function (o) { return o.value === valor; });
                if (existe) control.value = valor;
                else if (otroInput && valor) { control.value = '__otro__'; otroInput.hidden = false; otroInput.value = valor; }
            } else if (campo.tipo === 'area') {
                control = document.createElement('textarea');
                control.className = 'gm-textarea';
                control.rows = 3;
                control.value = valor;
            } else {
                control = document.createElement('input');
                control.type = campo.tipo === 'numero' ? 'number' : 'text';
                control.className = 'gm-input';
                control.value = valor;
            }
            controles[campo.clave] = { control: control, otro: otroInput };
            fila.appendChild(control);

            if (campo.dado) {
                var dado = document.createElement('button');
                dado.type = 'button';
                dado.className = 'gm-btn gm-btn--outline gm-btn--sm gm-dialogo__dado';
                dado.textContent = '🎲';
                dado.title = 'Generar al azar';
                dado.addEventListener('click', function () {
                    var generado = campo.dado(leerValores());
                    var c = controles[campo.clave];
                    if (c.control.tagName === 'SELECT') {
                        var hay = Array.prototype.some.call(c.control.options, function (o) { return o.value === generado; });
                        if (hay) { c.control.value = generado; if (c.otro) c.otro.hidden = true; }
                        else if (c.otro) { c.control.value = '__otro__'; c.otro.hidden = false; c.otro.value = generado; }
                    } else {
                        c.control.value = generado;
                    }
                });
                fila.appendChild(dado);
            }
            field.appendChild(fila);
            if (otroInput) field.appendChild(otroInput);
            if (campo.ayuda) {
                var ayuda = document.createElement('p');
                ayuda.className = 'gm-hint';
                ayuda.style.margin = '4px 0 0';
                ayuda.textContent = campo.ayuda;
                field.appendChild(ayuda);
            }
            GM.dlgCampos.appendChild(field);
        });

        function leerValores() {
            var out = {};
            campos.forEach(function (campo) {
                var c = controles[campo.clave];
                var v = c.control.value;
                if (v === '__otro__') v = c.otro ? c.otro.value : '';
                out[campo.clave] = typeof v === 'string' ? v.trim() : v;
            });
            return out;
        }

        GM.dlgAccion = function () {
            var out = leerValores();
            for (var i = 0; i < campos.length; i++) {
                if (campos[i].requerido && !out[campos[i].clave]) {
                    controles[campos[i].clave].control.focus();
                    return; // falta un obligatorio: el diálogo no se cierra
                }
            }
            GM.cerrarDialogo();
            onOk(out);
        };
        GM.dlg.hidden = false;
        GM.dlgFondo.hidden = false;
        var primero = GM.dlgCampos.querySelector('input, select, textarea');
        if (primero) primero.focus();
    }


    // ======================================================================
    //  DIÁLOGO MODAL DEL EDITOR — formularios con selects (adiós, prompts)
    // ======================================================================
    GM.dlg = document.getElementById('gm-dialogo');

    GM.dlgFondo = document.getElementById('gm-dialogo-fondo');

    GM.dlgTitulo = document.getElementById('gm-dialogo-titulo');

    GM.dlgCampos = document.getElementById('gm-dialogo-campos');

    GM.dlgOk = document.getElementById('gm-dialogo-ok');

    GM.dlgCancelar = document.getElementById('gm-dialogo-cancelar');

    GM.dlgAccion = null;

})(window.GMMapa);
