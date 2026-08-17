/*
 * mapa-leyenda.js — Llegenda 4a: capes GIS, mode foc per nació i cercador global (doc §5.2)
 *
 * Mòdul del Mapa Mundi. Veure mapa-nucleo.js per a l'espai de noms compartit.
 */
// Implementa 5.2.1/5.2.2/5.2.3 del doc: clic en nación = modo foco (resto
// atenuado + panel agrupado), fila de capas GIS arriba, buscador global.
(function () {
    var svg = document.getElementById('gm-mapa-svg');
    if (!svg) return;

    function marcadores(selector) {
        return Array.prototype.slice.call(svg.querySelectorAll(selector));
    }

    // ---------- Capas (toggle estilo GIS) ----------
    var capas = {
        naciones: function () { return marcadores('.gm-mapa-nacion__borde, .gm-mapa-nacion__etiqueta'); },
        facciones: function () { return marcadores('.gm-mapa-marcador[data-region-id]'); },
        ciudades: function () { return marcadores('.gm-mapa-marcador--ciudad'); },
        zonas: function () { return marcadores('.gm-mapa-marcador--zona'); },
        puntos: function () { return marcadores('.gm-mapa-marcador--punto'); },
        eventos: function () {
            var c = document.getElementById('gm-cronologia');
            return c ? [c] : [];
        }
    };

    document.querySelectorAll('.gm-capa-chip').forEach(function (chip) {
        chip.addEventListener('click', function () {
            var off = chip.classList.toggle('is-off');
            (capas[chip.getAttribute('data-capa')] || function () { return []; })()
                .forEach(function (el) { el.classList.toggle('gm-mapa-oculto', off); });
        });
    });

    // ---------- Lista de naciones: "…N más ▾" ----------
    var botonMas = document.getElementById('gm-leyenda-mas');
    if (botonMas) {
        botonMas.addEventListener('click', function () {
            var extras = document.querySelectorAll('.gm-leyenda-extra');
            var ocultas = extras.length && extras[0].classList.contains('u-oculto-extra');
            extras.forEach(function (f) { f.classList.toggle('u-oculto-extra', !ocultas); });
            botonMas.textContent = ocultas ? 'mostrar menos ▴' : ('…' + extras.length + ' más ▾');
        });
    }

    // ---------- Modo foco por nación (5.2.1) ----------
    var focoActual = null;
    var panel = document.getElementById('gm-foco-panel');
    var panelTitulo = document.getElementById('gm-foco-titulo');
    var panelResumen = document.getElementById('gm-foco-resumen');

    function limpiarFoco() {
        focoActual = null;
        svg.querySelectorAll('.gm-mapa-atenuado').forEach(function (el) {
            el.classList.remove('gm-mapa-atenuado');
        });
        document.querySelectorAll('.gm-leyenda-fila.is-foco').forEach(function (f) {
            f.classList.remove('is-foco');
            var marca = f.querySelector('.gm-leyenda-fila__foco');
            if (marca) marca.hidden = true;
        });
        if (panel) panel.hidden = true;
    }

    function aplicarFoco(nombre) {
        if (focoActual === nombre) { limpiarFoco(); return; }
        limpiarFoco();
        focoActual = nombre;

        // atenúa todo lo que no pertenezca a la nación en foco
        marcadores('.gm-mapa-nacion__borde polygon[data-nacion], .gm-mapa-nacion__etiqueta text[data-nacion]')
            .forEach(function (el) {
                el.classList.toggle('gm-mapa-atenuado', el.dataset.nacion !== nombre);
            });
        marcadores('.gm-mapa-marcador--ciudad').forEach(function (el) {
            el.classList.toggle('gm-mapa-atenuado', el.dataset.ciudadNacion !== nombre);
        });
        marcadores('.gm-mapa-marcador--zona').forEach(function (el) {
            el.classList.toggle('gm-mapa-atenuado', el.dataset.zonaNacion !== nombre);
        });
        // facciones y puntos no llevan nación asignada: quedan atenuados en foco
        marcadores('.gm-mapa-marcador[data-region-id], .gm-mapa-marcador--punto').forEach(function (el) {
            el.classList.add('gm-mapa-atenuado');
        });

        var fila = document.querySelector('.gm-leyenda-fila[data-foco-nacion="' + nombre + '"]');
        if (fila) {
            fila.classList.add('is-foco');
            var marca = fila.querySelector('.gm-leyenda-fila__foco');
            if (marca) marca.hidden = false;
        }

        // panel agrupado (5.2.2): qué hay dentro de la nación
        var nCiudades = marcadores('.gm-mapa-marcador--ciudad').filter(function (el) {
            return el.dataset.ciudadNacion === nombre;
        }).length;
        var nZonas = marcadores('.gm-mapa-marcador--zona').filter(function (el) {
            return el.dataset.zonaNacion === nombre;
        }).length;
        var poligono = svg.querySelector('.gm-mapa-nacion__borde polygon[data-nacion="' + nombre + '"]');
        var nDeidades = poligono ? (poligono.dataset.nacionDeidades || '').split(',').filter(Boolean).length : 0;
        var nRazas = poligono ? (poligono.dataset.nacionRazas || '').split(',').filter(Boolean).length : 0;

        if (panel) {
            panelTitulo.textContent = '📌 ' + nombre + ' en foco';
            panelResumen.textContent = nCiudades + ' ciudades · ' + nZonas + ' zonas · '
                + nDeidades + ' deidades · ' + nRazas + ' razas';
            panel.hidden = false;
        }
    }

    document.querySelectorAll('.gm-leyenda-fila[data-foco-nacion]').forEach(function (fila) {
        fila.addEventListener('click', function (e) {
            if (e.target.closest('.gm-leyenda-ojo')) return;   // el 👁 solo oculta/muestra
            aplicarFoco(fila.getAttribute('data-foco-nacion'));
        });
    });
    var cerrar = document.getElementById('gm-foco-cerrar');
    if (cerrar) cerrar.addEventListener('click', limpiarFoco);

    // ---------- Buscador global (5.2.3): nación, ciudad, facción… ----------
    var buscador = document.getElementById('gm-leyenda-buscar');
    if (buscador) {
        buscador.addEventListener('input', function () {
            var q = buscador.value.trim().toLowerCase();

            // filtra las filas de la leyenda (naciones y facciones)
            document.querySelectorAll('[data-buscable]').forEach(function (fila) {
                var nombre = (fila.getAttribute('data-buscable') || '').toLowerCase();
                var coincide = !q || nombre.indexOf(q) !== -1;
                fila.style.display = coincide ? '' : 'none';
                if (q && coincide) fila.classList.remove('u-oculto-extra');
            });

            // atenúa en el mapa lo que no coincide
            var porNombre = [
                ['.gm-mapa-nacion__borde polygon[data-nacion], .gm-mapa-nacion__etiqueta text[data-nacion]', 'nacion'],
                ['.gm-mapa-marcador--ciudad', 'ciudadNombre'],
                ['.gm-mapa-marcador--zona', 'zonaNombre'],
                ['.gm-mapa-marcador--punto', 'puntoNombre'],
                ['.gm-mapa-marcador[data-region-id]', 'panelTitle']
            ];
            porNombre.forEach(function (par) {
                marcadores(par[0]).forEach(function (el) {
                    var nombre = (el.dataset[par[1]] || '').toLowerCase();
                    el.classList.toggle('gm-mapa-atenuado', !!q && nombre.indexOf(q) === -1);
                });
            });
            if (!q && !focoActual) {
                svg.querySelectorAll('.gm-mapa-atenuado').forEach(function (el) {
                    el.classList.remove('gm-mapa-atenuado');
                });
            }
        });
    }
})();
