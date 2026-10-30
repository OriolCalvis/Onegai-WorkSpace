/*
 * panels.js — Piloto de rediseño visual
 * Paneles flotantes reutilizables, sin dependencias externas:
 *   1) Vista previa de carta: pulsar una carta clona el contenido de su
 *      <template data-panel-source="..."> dentro del panel flotante central
 *      (#gm-panel-preview) y lo abre.
 *   2) Panel de filtros anclado: botón para mostrar/ocultar en pantallas pequeñas.
 *   3) Subsecciones plegables dentro de un panel (características/habilidades/etc.).
 *   4) Drawer de navegación móvil: bajo 820px la sidebar se abre/cierra con el
 *      botón hamburguesa del header (Plan de cierre, tarea 43).
 */
(function () {
    // ---- Drawer de navegación móvil ----
    var MEDIA_SIDEBAR_MOVIL = '(max-width: 820px)';
    var ultimoFocoAntesSidebar = null;
    var contadorA11yIds = 0;

    function asegurarId(elemento, prefijo) {
        if (!elemento) return null;
        if (!elemento.id) {
            contadorA11yIds += 1;
            elemento.id = prefijo + '-' + contadorA11yIds;
        }
        return elemento.id;
    }

    function esSidebarMovil() {
        return !!(window.matchMedia && window.matchMedia(MEDIA_SIDEBAR_MOVIL).matches);
    }

    function obtenerSidebar() {
        return document.getElementById('gm-sidebar');
    }

    function obtenerToggleSidebar() {
        return document.querySelector('[data-menu-toggle]');
    }

    function obtenerBackdropSidebar() {
        var backdrop = document.getElementById('gm-sidebar-backdrop');
        if (!backdrop) {
            backdrop = document.createElement('div');
            backdrop.id = 'gm-sidebar-backdrop';
            backdrop.className = 'gm-sidebar-backdrop';
            backdrop.setAttribute('aria-hidden', 'true');
            document.body.appendChild(backdrop);
        }
        return backdrop;
    }

    function obtenerFocuseablesSidebar(sidebar) {
        if (!sidebar) return [];
        return Array.prototype.slice.call(
            sidebar.querySelectorAll('a[href], button:not([disabled]), [tabindex]:not([tabindex="-1"])')
        ).filter(function (elemento) {
            return !elemento.hasAttribute('hidden')
                && elemento.getAttribute('aria-hidden') !== 'true'
                && elemento.offsetParent !== null;
        });
    }

    function enfocarPrimerElementoSidebar() {
        var sidebar = obtenerSidebar();
        if (!sidebar) return;
        var focuseables = obtenerFocuseablesSidebar(sidebar);
        if (focuseables.length) {
            focuseables[0].focus();
            return;
        }
        sidebar.focus();
    }

    function restaurarFocoTrasSidebar() {
        if (ultimoFocoAntesSidebar && typeof ultimoFocoAntesSidebar.focus === 'function') {
            ultimoFocoAntesSidebar.focus();
            ultimoFocoAntesSidebar = null;
            return;
        }
        var toggle = obtenerToggleSidebar();
        if (toggle) {
            toggle.focus();
        }
    }

    function sincronizarAccesibilidadSidebar(abierta) {
        var sidebar = obtenerSidebar();
        var toggle = obtenerToggleSidebar();
        var backdrop = obtenerBackdropSidebar();
        var esMovil = esSidebarMovil();
        if (!sidebar) return;

        sidebar.setAttribute('aria-hidden', esMovil && !abierta ? 'true' : 'false');
        backdrop.setAttribute('aria-hidden', abierta ? 'false' : 'true');
        document.body.classList.toggle('gm-scroll-lock', esMovil && abierta);

        if (toggle) {
            toggle.setAttribute('aria-expanded', abierta ? 'true' : 'false');
            toggle.setAttribute('aria-label', abierta ? 'Cerrar menú de navegación' : 'Abrir menú de navegación');
        }
    }

    function ponerEstadoSidebar(abierta) {
        var sidebar = obtenerSidebar();
        if (!sidebar) return;
        if (abierta && esSidebarMovil()) {
            var activo = document.activeElement;
            if (activo && !sidebar.contains(activo)) {
                ultimoFocoAntesSidebar = activo;
            }
        }
        sidebar.classList.toggle('is-open', abierta);
        obtenerBackdropSidebar().classList.toggle('is-open', abierta);
        sincronizarAccesibilidadSidebar(abierta);
        if (abierta && esSidebarMovil()) {
            enfocarPrimerElementoSidebar();
        } else if (!abierta && esSidebarMovil()) {
            restaurarFocoTrasSidebar();
        }
    }

    function sidebarAbierta() {
        var sidebar = obtenerSidebar();
        return !!(sidebar && sidebar.classList.contains('is-open'));
    }

    function normalizarPanelesFlotantes(root) {
        (root || document).querySelectorAll('.gm-panel').forEach(function (panel) {
            if (!panel.hasAttribute('tabindex')) {
                panel.setAttribute('tabindex', '-1');
            }
            var titulo = panel.querySelector('.gm-panel__title, .gm-panel-preview-title');
            if (titulo) {
                panel.setAttribute('aria-labelledby', asegurarId(titulo, 'gm-panel-title'));
            }
            if (!panel.classList.contains('gm-panel--anchored') && !panel.hasAttribute('aria-modal')) {
                panel.setAttribute('aria-modal', 'true');
            }
        });
    }

    function normalizarBackdrops(root) {
        (root || document).querySelectorAll('.gm-panel-backdrop').forEach(function (backdrop) {
            backdrop.setAttribute('aria-hidden', backdrop.classList.contains('is-open') ? 'false' : 'true');
        });
    }

    function panelAncladoVisible(panel) {
        return !esSidebarMovil() || panel.classList.contains('is-visible-movil');
    }

    function sincronizarPanelAnclado(panel) {
        if (!panel) return;
        var panelId = asegurarId(panel, 'gm-panel-anchored');
        var titulo = panel.querySelector('.gm-panel__title, .gm-panel-preview-title');
        if (titulo) {
            panel.setAttribute('aria-labelledby', asegurarId(titulo, 'gm-panel-anchored-title'));
        }
        if (!panel.hasAttribute('role')) {
            panel.setAttribute('role', 'complementary');
        }

        var visible = panelAncladoVisible(panel);
        panel.setAttribute('aria-hidden', visible ? 'false' : 'true');

        document.querySelectorAll('[data-panel-anchor-toggle="' + panelId + '"]').forEach(function (toggle) {
            toggle.setAttribute('aria-controls', panelId);
            toggle.setAttribute('aria-expanded', visible ? 'true' : 'false');
        });
    }

    function sincronizarTodosLosPanelesAnclados(root) {
        (root || document).querySelectorAll('.gm-panel--anchored').forEach(sincronizarPanelAnclado);
    }

    function sincronizarPanelSeccion(seccion) {
        if (!seccion) return;
        var toggle = seccion.querySelector('.gm-panel-section__toggle');
        var contenido = seccion.querySelector('.gm-panel-section__content');
        if (!toggle || !contenido) return;

        var seccionId = asegurarId(seccion, 'gm-panel-section');
        var toggleId = asegurarId(toggle, seccionId + '-toggle');
        var contenidoId = asegurarId(contenido, seccionId + '-content');
        var expandida = seccion.classList.contains('is-expanded');

        toggle.setAttribute('aria-controls', contenidoId);
        toggle.setAttribute('aria-expanded', expandida ? 'true' : 'false');
        contenido.setAttribute('role', 'region');
        contenido.setAttribute('aria-labelledby', toggleId);
        contenido.setAttribute('aria-hidden', expandida ? 'false' : 'true');
        contenido.hidden = !expandida;
    }

    function sincronizarTodasLasSecciones(root) {
        (root || document).querySelectorAll('.gm-panel-section').forEach(sincronizarPanelSeccion);
    }

    function abrirPanel(panel) {
        if (!panel) return;
        normalizarPanelesFlotantes(panel);
        panel.classList.add('is-open');
        var backdrop = document.querySelector('[data-panel-backdrop-for="' + panel.id + '"]')
            || document.getElementById('gm-panel-backdrop');
        if (backdrop) {
            backdrop.classList.add('is-open');
            backdrop.setAttribute('aria-hidden', 'false');
        }
        panel.setAttribute('aria-hidden', 'false');
    }

    function cerrarPanel(panel) {
        if (!panel) return;
        panel.classList.remove('is-open');
        var backdrop = document.querySelector('[data-panel-backdrop-for="' + panel.id + '"]')
            || document.getElementById('gm-panel-backdrop');
        if (backdrop) {
            backdrop.classList.remove('is-open');
            backdrop.setAttribute('aria-hidden', 'true');
        }
        panel.setAttribute('aria-hidden', 'true');
    }

    function cerrarTodosLosPaneles() {
        document.querySelectorAll('.gm-panel.is-open').forEach(cerrarPanel);
    }

    function mostrarPreviewDeCarta(origen) {
        var plantillaId = origen.getAttribute('data-panel-source');
        var plantilla = document.getElementById(plantillaId);
        var panel = document.getElementById('gm-panel-preview');
        var cuerpo = document.getElementById('gm-panel-preview-body');
        var titulo = document.getElementById('gm-panel-preview-title');
        if (!plantilla || !panel || !cuerpo) return;

        cuerpo.innerHTML = '';
        cuerpo.appendChild(plantilla.content.cloneNode(true));
        if (titulo) {
            titulo.textContent = origen.getAttribute('data-panel-title') || '';
        }
        abrirPanel(panel);
        // Aviso para quien necesite reaccionar a la apertura (p. ej. el editor de
        // puntos del mapa mundi, que repinta su sección editable con datos frescos
        // cada vez que se abre un panel — el contenido del <template> es estático).
        document.dispatchEvent(new CustomEvent('gm:panel-opened', { detail: { origen: origen, panel: panel } }));
    }

    document.addEventListener('click', function (evento) {
        // Abrir/cerrar el drawer de navegación móvil
        if (evento.target.closest('[data-menu-toggle]')) {
            ponerEstadoSidebar(!sidebarAbierta());
            return;
        }
        if (evento.target.id === 'gm-sidebar-backdrop') {
            ponerEstadoSidebar(false);
            return;
        }
        // Al navegar desde la sidebar, cerrar el drawer
        if (evento.target.closest('#gm-sidebar a')) {
            ponerEstadoSidebar(false);
            // sin return: dejar que el enlace navegue con normalidad
        }

        // Abrir preview de carta
        var carta = evento.target.closest('[data-panel-source]');
        if (carta && !evento.target.closest('a, button')) {
            mostrarPreviewDeCarta(carta);
            return;
        }

        // Cerrar panel (botón de cierre o backdrop)
        var cierre = evento.target.closest('[data-panel-close]');
        if (cierre) {
            cerrarTodosLosPaneles();
            return;
        }
        if (evento.target.classList && evento.target.classList.contains('gm-panel-backdrop')) {
            cerrarTodosLosPaneles();
            return;
        }

        // Abrir/cerrar panel anclado (filtros) en móvil
        var toggleAnclado = evento.target.closest('[data-panel-anchor-toggle]');
        if (toggleAnclado) {
            var idAnclado = toggleAnclado.getAttribute('data-panel-anchor-toggle');
            var panelAnclado = document.getElementById(idAnclado);
            if (panelAnclado) {
                panelAnclado.classList.toggle('is-visible-movil');
                sincronizarPanelAnclado(panelAnclado);
            }
            return;
        }

        // Subsecciones plegables (características / habilidades / categorías)
        var seccionToggle = evento.target.closest('.gm-panel-section__toggle');
        if (seccionToggle) {
            var seccion = seccionToggle.closest('.gm-panel-section');
            if (seccion) {
                seccion.classList.toggle('is-expanded');
                sincronizarPanelSeccion(seccion);
            }
            return;
        }
    });

    document.addEventListener('keydown', function (evento) {
        if (sidebarAbierta() && esSidebarMovil() && evento.key === 'Tab') {
            var sidebar = obtenerSidebar();
            var toggle = obtenerToggleSidebar();
            var focuseables = obtenerFocuseablesSidebar(sidebar);
            if (!focuseables.length) {
                evento.preventDefault();
                if (sidebar) sidebar.focus();
                return;
            }
            var primero = focuseables[0];
            var ultimo = focuseables[focuseables.length - 1];
            var activo = document.activeElement;

            if (evento.shiftKey && (activo === primero || activo === sidebar)) {
                evento.preventDefault();
                ultimo.focus();
                return;
            }
            if (!evento.shiftKey && (activo === ultimo || activo === toggle)) {
                evento.preventDefault();
                primero.focus();
                return;
            }
        }
        if (evento.key === 'Escape') {
            cerrarTodosLosPaneles();
            if (sidebarAbierta()) {
                evento.preventDefault();
                ponerEstadoSidebar(false);
            }
        }
    });

    // Páginas de detalle de UNA carta: el usuario ya decidió verla entera, así que
    // todas las secciones llegan expandidas (revisión de diseño 2.2 — el acordeón
    // solo tiene sentido en el álbum, no en la ficha individual).
    document.addEventListener('DOMContentLoaded', function () {
        sincronizarAccesibilidadSidebar(false);
        normalizarPanelesFlotantes(document);
        normalizarBackdrops(document);
        sincronizarTodosLosPanelesAnclados(document);

        var cartas = document.querySelectorAll('article.gm-card');
        if (cartas.length === 1) {
            document.querySelectorAll('.gm-panel-section').forEach(function (seccion) {
                seccion.classList.add('is-expanded');
            });
        }
        sincronizarTodasLasSecciones(document);
    });

    document.addEventListener('gm:panel-opened', function (evento) {
        window.requestAnimationFrame(function () {
            var panel = evento.detail && evento.detail.panel ? evento.detail.panel : document;
            normalizarPanelesFlotantes(panel);
            sincronizarTodasLasSecciones(panel);
            normalizarBackdrops(document);
        });
    });

    if (window.matchMedia) {
        var mediaSidebar = window.matchMedia(MEDIA_SIDEBAR_MOVIL);
        var reajustarSidebar = function () {
            if (!mediaSidebar.matches && sidebarAbierta()) {
                var sidebar = obtenerSidebar();
                if (sidebar) {
                    sidebar.classList.remove('is-open');
                }
                obtenerBackdropSidebar().classList.remove('is-open');
            }
            sincronizarAccesibilidadSidebar(false);
            sincronizarTodosLosPanelesAnclados(document);
            ultimoFocoAntesSidebar = null;
        };

        if (typeof mediaSidebar.addEventListener === 'function') {
            mediaSidebar.addEventListener('change', reajustarSidebar);
        } else if (typeof mediaSidebar.addListener === 'function') {
            mediaSidebar.addListener(reajustarSidebar);
        }
    }
})();
