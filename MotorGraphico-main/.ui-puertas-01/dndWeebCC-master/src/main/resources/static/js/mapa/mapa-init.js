/*
 * mapa-init.js — Arrencada del mapa
 *
 * Mòdul del Mapa Mundi. Tot l'estat i les funcions compartides viuen a
 * l'espai de noms `window.GMMapa` (GM), creat per mapa-nucleo.js. Aquest
 * fitxer només DEFINEIX; l'arrencada i la connexió d'esdeveniments són a
 * mapa-init.js, que es carrega l'últim.
 */
(function (GM) {
    'use strict';

    document.querySelectorAll('.gm-mapa-leyenda__item[data-nacion-toggle]').forEach(function (item) {
        item.addEventListener('click', function () {
            var nombre = item.dataset.nacionToggle;
            if (GM.hiddenNaciones.has(nombre)) GM.hiddenNaciones.delete(nombre); else GM.hiddenNaciones.add(nombre);
            item.classList.toggle('is-hidden', GM.hiddenNaciones.has(nombre));
            GM.aplicarFiltros();
        });
    });

    document.querySelectorAll('.gm-mapa-leyenda__item[data-region-toggle]').forEach(function (item) {
        item.addEventListener('click', function () {
            var faccion = item.dataset.regionToggle;
            if (GM.hiddenFacciones.has(faccion)) GM.hiddenFacciones.delete(faccion); else GM.hiddenFacciones.add(faccion);
            item.classList.toggle('is-hidden', GM.hiddenFacciones.has(faccion));
            GM.aplicarFiltros();
        });
    });

    document.querySelectorAll('.gm-tier-btn').forEach(function (btn) {
        btn.addEventListener('click', function () {
            var tier = btn.dataset.tier;
            if (GM.hiddenTiers.has(tier)) GM.hiddenTiers.delete(tier); else GM.hiddenTiers.add(tier);
            btn.classList.toggle('is-off', GM.hiddenTiers.has(tier));
            GM.aplicarFiltros();
        });
    });

    if (GM.tierTodosBtn) {
        GM.tierTodosBtn.addEventListener('click', function () {
            GM.hiddenTiers.clear();
            document.querySelectorAll('.gm-tier-btn').forEach(function (btn) { btn.classList.remove('is-off'); });
            GM.aplicarFiltros();
        });
    }


    document.addEventListener('gm:panel-opened', function (evt) {
        var origen = evt.detail && evt.detail.origen;
        if (!origen || !origen.dataset || !origen.dataset.puntoId) return;
        GM.renderPuntoEditor(origen);
    });


    GM.buildHandles();

    // OJO: loadDraft() se llama al FINAL del script (tras definirse los creadores de
    // ciudad/zona/cronología que necesita para restaurar el borrador) — llamarlo aquí
    // rompía la restauración de ciudades: GM_TAMANOS aún no existía.
    GM.polygons.forEach(function (poly) { poly.addEventListener('dblclick', GM.onPolygonDblClick); });


    GM.toggleBtn.addEventListener('click', function () { GM.setEditing(!GM.editing); });


    GM.nuevoPuntoBtn.addEventListener('click', function () { GM.setPlacingPunto(!GM.placingPunto); });


    GM.saveBtn.addEventListener('click', function () {
        var textoOriginal = GM.saveBtn.textContent;
        GM.saveBtn.disabled = true;
        GM.saveBtn.textContent = 'Guardando…';
        fetch(GM.saveBtn.dataset.guardarUrl, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(GM.buildPayload())
        }).then(function (res) {
            if (!res.ok) throw new Error('HTTP ' + res.status);
            return res.json();
        }).then(function (data) {
            if (!data.ok) throw new Error(data.error || 'error desconocido');
            try { localStorage.removeItem(GM.LS_KEY); } catch (e) { /* nada que limpiar */ }
            GM.banner.hidden = true;
            GM.saveBtn.textContent = '✅ Guardado';
            setTimeout(function () { GM.saveBtn.textContent = textoOriginal; GM.saveBtn.disabled = false; }, 1600);
        }).catch(function (err) {
            alert('No se pudo guardar en el servidor (' + err.message + '). Usa "Copiar coordenadas" como alternativa.');
            GM.saveBtn.textContent = textoOriginal;
            GM.saveBtn.disabled = false;
        });
    });


    GM.resetBtn.addEventListener('click', function () {
        if (!confirm('¿Restablecer todos los puntos a como estaban al cargar la página? Se perderán los cambios sin guardar.')) return;
        try { localStorage.removeItem(GM.LS_KEY); } catch (e) { /* nada que limpiar */ }
        location.reload();
    });


    GM.exportBtn.addEventListener('click', function () {
        GM.exportText.value = GM.buildExportText();
        GM.exportPanel.hidden = false;
        GM.exportText.focus();
        GM.exportText.select();
        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(GM.exportText.value).catch(function () { /* el usuario puede copiar a mano del textarea */ });
        }
    });


    GM.bannerDismiss.addEventListener('click', function () {
        try { localStorage.removeItem(GM.LS_KEY); } catch (e) { /* nada que limpiar */ }
        location.reload();
    });


    GM.markers.forEach(function (m) { m.addEventListener('pointerdown', GM.onMarkerPointerDown); });

    GM.svg.addEventListener('pointermove', GM.onSvgPointerMove);

    GM.svg.addEventListener('pointerup', GM.onSvgPointerUp);

    GM.svg.addEventListener('pointerleave', function () { GM.dragging = null; });


    // Delegación global para los botones de los editores de ciudad.
    document.body.addEventListener('click', function (evt) {
        var editor = evt.target.closest('.gm-ciudad-editor');
        if (!editor) return;
        var id = editor.dataset.ciudadId;
        var m = GM.ciudadPorId(id);
        if (!m) return;

        if (evt.target.classList.contains('gm-ciudad-editor__add-historia')) {
            var inputH = editor.querySelector('input[list="gm-historia-datalist"]');
            var hid = GM.idDesdeInput(inputH);
            if (hid) {
                var set = new Set((m.dataset.historiaIds || '').split(',').filter(Boolean)); set.add(hid);
                m.dataset.historiaIds = Array.from(set).join(','); inputH.value = '';
                GM.pintarChipsCiudad(editor); GM.saveDraft();
            }
        } else if (evt.target.classList.contains('gm-ciudad-editor__add-npc')) {
            var inputN = editor.querySelector('input[list="gm-npc-datalist"]');
            var nid = GM.idDesdeInput(inputN);
            if (nid) {
                var setN = new Set((m.dataset.npcIds || '').split(',').filter(Boolean)); setN.add(nid);
                m.dataset.npcIds = Array.from(setN).join(','); inputN.value = '';
                GM.pintarChipsCiudad(editor); GM.saveDraft();
            }
        } else if (evt.target.classList.contains('gm-ciudad-editor__editar-campos')) {
            var posC = GM.markerPos(m);
            GM.formularioCiudad('✏️ ' + m.dataset.ciudadNombre, {
                nombre: m.dataset.ciudadNombre, tamano: m.dataset.ciudadTamano, nacion: m.dataset.ciudadNacion,
                poblacion: m.dataset.ciudadPoblacion, gobernante: m.dataset.ciudadGobernante,
                rasgo: m.dataset.ciudadRasgo, descripcion: m.dataset.ciudadDescripcion
            }, posC.x, posC.y, function (datos) {
                m.dataset.ciudadNombre = datos.nombre; m.dataset.ciudadTamano = datos.tamano;
                m.dataset.ciudadNacion = datos.nacion; m.dataset.ciudadPoblacion = datos.poblacion;
                m.dataset.ciudadGobernante = datos.gobernante; m.dataset.ciudadRasgo = datos.rasgo;
                m.dataset.ciudadDescripcion = datos.descripcion; m.dataset.panelTitle = datos.nombre;
                m.querySelector('.gm-mapa-etiqueta').textContent = datos.nombre;
                m.querySelector('.gm-mapa-emoji').textContent = GM.iconoTamano(datos.tamano);
                GM.construirPanelCiudad(GM.ciudadDesdeMarcador(m));
                GM.saveDraft();
                GM.cerrarPanelSiCorresponde();
            });
        } else if (evt.target.classList.contains('gm-ciudad-editor__eliminar')) {
            if (!confirm('¿Eliminar la ciudad "' + m.dataset.ciudadNombre + '"?')) return;
            var tpl = document.getElementById('panel-ciudad-' + id); if (tpl) tpl.remove();
            m.remove();
            GM.markers = GM.markers.filter(function (x) { return x !== m; });
            GM.saveDraft();
            cerrarTodosLosPaneles && cerrarTodosLosPaneles();
            document.querySelectorAll('.gm-panel.is-open').forEach(function (pn) { pn.classList.remove('is-open'); });
            var bd = document.getElementById('gm-panel-backdrop'); if (bd) bd.classList.remove('is-open');
        }
    });


    // Al abrir la ficha de una ciudad, pinta sus chips (el contenido del template se
    // clona vacío; lo rellenamos cuando el panel ya está en el DOM).
    document.addEventListener('click', function (evt) {
        var origen = evt.target.closest('[data-ciudad-id]');
        if (!origen || !origen.classList.contains('gm-mapa-marcador')) return;
        setTimeout(function () {
            document.querySelectorAll('.gm-ciudad-editor').forEach(GM.pintarChipsCiudad);
        }, 30);
    });


    if (GM.nuevaCiudadBtn) {
        GM.nuevaCiudadBtn.addEventListener('click', function (e) {
            // clic normal = colocar a mano; Alt+clic = generar una ciudad al azar donde piques
            GM.setPlacingCiudad(!GM.placingCiudad);
            GM.nuevaCiudadBtn.dataset.aleatoria = e.altKey ? '1' : '';
        });
    }


    GM.dlgCancelar.addEventListener('click', GM.cerrarDialogo);

    GM.dlgFondo.addEventListener('click', GM.cerrarDialogo);

    document.addEventListener('keydown', function (e) { if (e.key === 'Escape' && !GM.dlg.hidden) GM.cerrarDialogo(); });

    GM.dlgOk.addEventListener('click', function () { if (GM.dlgAccion) GM.dlgAccion(); });


    if (GM.nuevaZonaBtn) {
        GM.nuevaZonaBtn.addEventListener('click', function (e) {
            GM.setPlacingZona(!GM.placingZona);
            GM.nuevaZonaBtn.dataset.aleatoria = e.altKey ? '1' : '';
        });
    }


    document.body.addEventListener('click', function (evt) {
        var editor = evt.target.closest('.gm-zona-editor');
        if (!editor) return;
        var m = GM.zonaPorId(editor.dataset.zonaId);
        if (!m) return;

        if (evt.target.classList.contains('gm-zona-editor__add-historia')) {
            var inputH = editor.querySelector('input[list="gm-historia-datalist"]');
            var hid = GM.idDesdeInput(inputH);
            if (hid) {
                var set = new Set((m.dataset.historiaIds || '').split(',').filter(Boolean)); set.add(hid);
                m.dataset.historiaIds = Array.from(set).join(','); inputH.value = '';
                GM.pintarChipsZona(editor); GM.saveDraft();
            }
        } else if (evt.target.classList.contains('gm-zona-editor__editar-campos')) {
            var posZ = GM.markerPos(m);
            GM.formularioZona('✏️ ' + m.dataset.zonaNombre, {
                nombre: m.dataset.zonaNombre, tipo: m.dataset.zonaTipo, nacion: m.dataset.zonaNacion,
                peligro: m.dataset.zonaPeligro, descripcion: m.dataset.zonaDescripcion
            }, posZ.x, posZ.y, function (datos) {
                m.dataset.zonaNombre = datos.nombre; m.dataset.zonaTipo = datos.tipo;
                m.dataset.zonaNacion = datos.nacion; m.dataset.zonaPeligro = datos.peligro;
                m.dataset.zonaDescripcion = datos.descripcion; m.dataset.panelTitle = datos.nombre;
                m.querySelector('.gm-mapa-etiqueta').textContent = datos.nombre;
                m.querySelector('.gm-mapa-emoji').textContent = GM.iconoZona(datos.tipo);
                GM.construirPanelZona(GM.zonaDesdeMarcador(m));
                GM.saveDraft();
                GM.cerrarPanelSiCorresponde();
            });
        } else if (evt.target.classList.contains('gm-zona-editor__eliminar')) {
            if (!confirm('¿Eliminar la zona "' + m.dataset.zonaNombre + '"?')) return;
            var tplZ = document.getElementById('panel-zona-' + m.dataset.zonaId); if (tplZ) tplZ.remove();
            m.remove();
            GM.markers = GM.markers.filter(function (x) { return x !== m; });
            GM.saveDraft();
            GM.cerrarPanelSiCorresponde();
        }
    });


    document.addEventListener('click', function (evt) {
        var origen = evt.target.closest('[data-zona-id]');
        if (!origen || !origen.classList || !origen.classList.contains('gm-mapa-marcador')) return;
        setTimeout(function () { document.querySelectorAll('.gm-zona-editor').forEach(GM.pintarChipsZona); }, 30);
    });


    // Los marcadores de zona que pinta el servidor llevan ⛰ fijo — se corrige según su tipo.
    GM.markers.forEach(function (m) {
        if (m.dataset.zonaId) {
            var em = m.querySelector('.gm-mapa-emoji');
            if (em) em.textContent = GM.iconoZona(m.dataset.zonaTipo);
        }
    });


    if (GM.nuevoEventoHistBtn) {
        GM.nuevoEventoHistBtn.addEventListener('click', function () {
            GM.formularioEventoHistorico('📜 Nuevo evento histórico', {}, function (datos) {
                datos.id = 'eh_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
                GM.crearItemCronologia(datos);
                GM.saveDraft();
            });
        });
    }


    GM.contCronologia().addEventListener('click', function (evt) {
        var item = evt.target.closest('[data-evento-historico-id]');
        if (!item) return;
        if (evt.target.classList.contains('gm-eh-editar')) {
            GM.formularioEventoHistorico('✏️ ' + item.dataset.ehTitulo, GM.eventoHistoricoDesdeItem(item), function (datos) {
                datos.id = item.dataset.eventoHistoricoId;
                GM.crearItemCronologia(datos);
                GM.saveDraft();
            });
        } else if (evt.target.classList.contains('gm-eh-eliminar')) {
            if (!confirm('¿Eliminar "' + item.dataset.ehTitulo + '" de la cronología?')) return;
            item.remove();
            GM.saveDraft();
        }
    });


    // Al abrir el panel de una nación: resaltarla en el mapa y construir su ficha.
    document.addEventListener('gm:panel-opened', function (evt) {
        var origen = evt.detail && evt.detail.origen;
        GM.polygons.forEach(function (p) { p.classList.remove('is-selected'); });
        if (!origen || !origen.dataset) return;
        if (origen.dataset.nacion && origen.dataset.panelSource === 'gm-panel-nacion-dinamico') {
            origen.classList.add('is-selected');
            GM.renderNacionPanel(origen);
        }
    });

    document.addEventListener('click', function (evt) {
        if (evt.target.closest('[data-panel-close]')
            || (evt.target.classList && evt.target.classList.contains('gm-panel-backdrop'))) {
            GM.polygons.forEach(function (p) { p.classList.remove('is-selected'); });
        }
    });

    // En modo edición, el clic sobre el territorio es para editar vértices — la ficha
    // de la nación se abre fuera del modo edición (evita aperturas accidentales).
    document.addEventListener('click', function (evt) {
        if (GM.editing && evt.target.closest && evt.target.closest('.gm-mapa-nacion__borde polygon')) {
            evt.stopPropagation();
        }
    }, true);

    // Accesibilidad: los polígonos tienen tabindex — Enter/Espacio abre su ficha.
    GM.polygons.forEach(function (poly) {
        poly.addEventListener('keydown', function (evento) {
            if (evento.key === 'Enter' || evento.key === ' ') {
                evento.preventDefault();
                poly.dispatchEvent(new MouseEvent('click', { bubbles: true }));
            }
        });
    });


    // Restaurar el borrador de localStorage AHORA, con todos los creadores ya definidos
    // (ciudades, zonas, cronología e identidad de naciones incluidos).
    GM.loadDraft();


    // ======================================================================

    // Enlace de vuelta desde una aventura o desde una historia sin región propia:
    // /mapa?faccion=xxx o /mapa?punto=yyy abre directamente ese marcador.
    (function abrirDesdeUrl() {
        var params = new URLSearchParams(location.search);
        var faccion = params.get('faccion');
        var puntoId = params.get('punto');
        var campo = faccion ? 'regionId' : (puntoId ? 'puntoId' : null);
        var valor = faccion || puntoId;
        if (!campo || !valor) return;
        var el = GM.markers.find(function (m) { return m.dataset[campo] === valor; });
        if (!el) return;
        requestAnimationFrame(function () {
            el.scrollIntoView({ behavior: 'smooth', block: 'center' });
            el.dispatchEvent(new MouseEvent('click', { bubbles: true }));
        });
    })();
})(window.GMMapa);
