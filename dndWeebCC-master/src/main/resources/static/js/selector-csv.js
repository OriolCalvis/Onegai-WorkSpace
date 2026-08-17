// Selector CSV sin escritura a mano: el usuario elige del desplegable y el valor se
// acumula como chips. El dato real vive en un input oculto con formato "a, b, c",
// exactamente lo que ListaTexto.splitCsv espera en el servidor, así que los
// controladores no cambian. Si una carta trae un valor que ya no está en el catálogo,
// el chip se muestra marcado (no se pierde al guardar) para que el editor lo vea.
(function () {
    'use strict';

    function iniciar(caja) {
        var oculto = caja.querySelector('.gm-csvsel__valor');
        var chips = caja.querySelector('.gm-csvsel__chips');
        var select = caja.querySelector('.gm-csvsel__select');
        if (!oculto || !chips || !select) return;

        var etiquetas = {};
        Array.prototype.forEach.call(select.options, function (op) {
            if (op.value) etiquetas[op.value] = op.textContent;
        });

        function valores() {
            return oculto.value.split(',')
                .map(function (v) { return v.trim(); })
                .filter(function (v) { return v.length > 0; });
        }

        function pintar() {
            chips.innerHTML = '';
            valores().forEach(function (v) {
                var conocido = Object.prototype.hasOwnProperty.call(etiquetas, v);
                var chip = document.createElement('span');
                chip.className = 'gm-csvsel__chip' + (conocido ? '' : ' gm-csvsel__chip--desconocido');
                chip.title = conocido ? v : v + ' — no está en el catálogo actual';

                var texto = document.createElement('span');
                texto.textContent = conocido ? etiquetas[v] : v;
                chip.appendChild(texto);

                var quitar = document.createElement('button');
                quitar.type = 'button';
                quitar.className = 'gm-csvsel__quitar';
                quitar.setAttribute('aria-label', 'Quitar ' + (conocido ? etiquetas[v] : v));
                quitar.textContent = '×';
                quitar.addEventListener('click', function () {
                    oculto.value = valores().filter(function (x) { return x !== v; }).join(', ');
                    pintar();
                });
                chip.appendChild(quitar);
                chips.appendChild(chip);
            });
        }

        select.addEventListener('change', function () {
            var v = select.value;
            if (v && valores().indexOf(v) === -1) {
                var vs = valores();
                vs.push(v);
                oculto.value = vs.join(', ');
                pintar();
            }
            select.value = '';
        });

        pintar();
    }

    document.querySelectorAll('.gm-csvsel').forEach(iniciar);
})();
