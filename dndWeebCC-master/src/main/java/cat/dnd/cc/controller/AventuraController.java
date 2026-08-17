package cat.dnd.cc.controller;

import cat.dnd.cc.model.Aventura;
import cat.dnd.cc.model.CartaImpresion;
import cat.dnd.cc.model.GeografiaMapa;
import cat.dnd.cc.model.Historia;
import cat.dnd.cc.model.TierDeity;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import cat.dnd.cc.service.AventuraService;
import cat.dnd.cc.service.GeografiaMapaService;
import cat.dnd.cc.service.HistoriaService;
import com.fasterxml.jackson.databind.JsonNode;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

@Controller
@RequestMapping("/aventuras")
public class AventuraController {

    private final AventuraService aventuraService;
    private final HistoriaService historiaService;
    private final cat.dnd.cc.service.TierDeityService tierDeityService;
    private final GeografiaMapaService geografiaMapaService;
    private final cat.dnd.cc.service.ConstructorAventuraService constructorService;
    private final cat.dnd.cc.service.ValidadorAventuraService validadorService;

    public AventuraController(AventuraService aventuraService, HistoriaService historiaService,
                              cat.dnd.cc.service.TierDeityService tierDeityService,
                              GeografiaMapaService geografiaMapaService,
                              cat.dnd.cc.service.ConstructorAventuraService constructorService,
                              cat.dnd.cc.service.ValidadorAventuraService validadorService) {
        this.aventuraService = aventuraService;
        this.historiaService = historiaService;
        this.tierDeityService = tierDeityService;
        this.geografiaMapaService = geografiaMapaService;
        this.constructorService = constructorService;
        this.validadorService = validadorService;
    }

    @GetMapping
    public String listar(Model model) {
        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventuras", aventuraService.llistarTotes());
        return "aventuras/llista";
    }

    @GetMapping("/crear")
    public String mostrarFormulario(Model model) {
        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventura", new Aventura());
        anadirCatalogo(model);
        return "aventuras/formulari";
    }

    @PostMapping("/crear")
    public String crear(@ModelAttribute("aventura") Aventura aventura) {
        aventuraService.guardar(aventura);
        return "redirect:/aventuras";
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable Long id, Model model) {
        Aventura aventura = aventuraService.obtenirPerId(id);
        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventura", aventura);
        model.addAttribute("historias", aventuraService.historiasDe(aventura));
        model.addAttribute("historiaService", historiaService);
        model.addAttribute("aventuraService", aventuraService);

        // Vínculos con el Mapa Mundi: las facciones que tocan sus historias (siempre,
        // automático) y cualquier punto personalizado que el director haya vinculado a
        // mano desde el editor del mapa — para poder ir de la aventura al mapa y no solo
        // al revés.
        Set<String> faccionesTocadas = new LinkedHashSet<>();
        for (Historia h : aventuraService.historiasDe(aventura)) {
            if (h.getFaction() != null && !h.getFaction().isBlank()) {
                faccionesTocadas.add(h.getFaction());
            }
        }
        GeografiaMapa geografia = geografiaMapaService.cargar();
        String idTexto = String.valueOf(id);
        List<GeografiaMapa.Punto> puntosVinculados = geografia.getPuntos().stream()
                .filter(p -> p.getAventuraIds().contains(idTexto))
                .toList();
        model.addAttribute("faccionesTocadas", faccionesTocadas);
        model.addAttribute("puntosVinculados", puntosVinculados);
        return "aventuras/detall";
    }

    /** Cuántas cartas caben físicamente en una hoja A4 con el formato fijo de 63,5×88,9mm (3×3). */
    private static final int CARTAS_POR_PAGINA = 9;

    /**
     * Vista de impresión: todas las cartas que componen la aventura (historias, deidades,
     * reparto, botín y trampas), todas con el mismo formato físico, listas para imprimir
     * de una tacada y recortar.
     * <p>
     * Los siete tipos se combinan en UNA sola lista uniforme (CartaImpresion) y se parten
     * en páginas de {@link #CARTAS_POR_PAGINA} aquí, en el servidor — no en CSS. Cada página
     * es un contenedor acotado que SIEMPRE cabe en una hoja, así que nunca hace falta que el
     * motor de impresión fragmente nada entre hojas (que es donde grid/flex fallan y cortan
     * cartas o generan páginas en blanco).
     */
    @GetMapping("/{id}/imprimir")
    public String imprimir(@PathVariable Long id, Model model) {
        Aventura aventura = aventuraService.obtenirPerId(id);
        CatalogoAventuraRepository catalogo = aventuraService.catalogo();

        List<CartaImpresion> cartas = new ArrayList<>();
        for (Historia h : aventuraService.historiasDe(aventura)) {
            cartas.add(mapHistoria(h));
        }
        for (String deidadId : aventura.getDeidadIds()) {
            TierDeity d = deidadOrNull(deidadId);
            if (d != null) cartas.add(mapDeidad(d));
        }
        for (JsonNode n : resolverCartas(aventura.getNpcIds(), catalogo)) cartas.add(mapNpc(n));
        for (JsonNode v : resolverCartas(aventura.getVillanoIds(), catalogo)) cartas.add(mapEnemigo(v, "Villano"));
        for (JsonNode e : resolverCartas(aventura.getEnemigoIds(), catalogo)) cartas.add(mapEnemigo(e, "Enemigo"));
        for (JsonNode l : resolverCartas(aventura.getLootIds(), catalogo)) cartas.add(mapLoot(l, catalogo));
        for (JsonNode t : resolverCartas(aventura.getTrampaIds(), catalogo)) cartas.add(mapTrampa(t));

        // Validación antes de imprimir: una carta sin título o sin ningún texto de
        // reglas no aporta nada en papel — mejor no imprimirla que imprimirla vacía.
        List<CartaImpresion> cartasValidas = cartas.stream()
                .filter(c -> !esVacia(c))
                .toList();

        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventura", aventura);
        model.addAttribute("totalCartas", cartasValidas.size());
        model.addAttribute("paginas", partir(cartasValidas, CARTAS_POR_PAGINA));
        return "aventuras/imprimir";
    }

    private boolean esVacia(CartaImpresion c) {
        boolean sinTitulo = c.getTitulo() == null || c.getTitulo().isBlank();
        boolean sinTexto = (c.getDesc() == null || c.getDesc().isBlank())
                && (c.getMeta() == null || c.getMeta().isBlank());
        return sinTitulo || sinTexto;
    }

    /** Recorta un texto a un largo máximo seguro para que quepa en la carta pase lo
     * que pase con el motor de impresión (no depende de que -webkit-line-clamp
     * funcione en el PDF/impresión, que es donde ha fallado hasta ahora). */
    private static String truncar(String texto, int max) {
        if (texto == null) return "";
        String limpio = texto.strip();
        if (limpio.length() <= max) return limpio;
        int corte = limpio.lastIndexOf(' ', max);
        if (corte < max / 2) corte = max;
        return limpio.substring(0, corte).stripTrailing() + "…";
    }

    /** "milicia_corrupta_de_llerba" → "Milicia Corrupta De Llerba", para que ninguna
     * carta muestre el id en bruto ni una facción en minúsculas sin más. */
    private static String capitalizar(String idOrFrase) {
        if (idOrFrase == null || idOrFrase.isBlank()) return "";
        String[] palabras = idOrFrase.replace('_', ' ').trim().split("\\s+");
        StringBuilder sb = new StringBuilder();
        for (String palabra : palabras) {
            if (palabra.isEmpty()) continue;
            if (sb.length() > 0) sb.append(' ');
            sb.append(Character.toUpperCase(palabra.charAt(0))).append(palabra.substring(1));
        }
        return sb.toString();
    }

    /** Parte una lista en sublistas de tamaño fijo (la última puede quedar más corta). */
    private static <T> List<List<T>> partir(List<T> lista, int tamano) {
        List<List<T>> paginas = new ArrayList<>();
        for (int i = 0; i < lista.size(); i += tamano) {
            paginas.add(lista.subList(i, Math.min(i + tamano, lista.size())));
        }
        return paginas;
    }

    private TierDeity deidadOrNull(String id) {
        try {
            return tierDeityService.obtenerPorId(id);
        } catch (RuntimeException e) {
            return null;
        }
    }

    private List<JsonNode> resolverCartas(List<String> ids, CatalogoAventuraRepository catalogo) {
        return ids.stream()
                .map(catalogo::completa)
                .filter(Objects::nonNull)
                .toList();
    }

    /** Cuántos caracteres caben con holgura en el bloque de descripción de una carta
     * de 63,5×88,9mm a 9px — probado a ojo contra las cartas más largas del catálogo. */
    private static final int MAX_DESC = 170;
    private static final int MAX_TITULO = 60;
    private static final int MAX_META = 60;

    private CartaImpresion mapHistoria(Historia h) {
        List<Integer> tr = h.getTierRange();
        String tier = "TIER " + h.getTierMin() + "-" + (tr != null && tr.size() > 1 ? tr.get(1) : h.getTierMin());
        String faccion = capitalizar(h.getFaction());
        return new CartaImpresion("📜", tier, truncar(h.getTitle(), MAX_TITULO), "Historia · " + faccion,
                null, truncar(h.getHook(), MAX_DESC), List.of(faccion), "historias · " + h.getId());
    }

    private CartaImpresion mapDeidad(TierDeity d) {
        String desc = d.getFavor() != null ? d.getFavor().getDescription() : "";
        List<String> badges = d.getCompatibleWith() != null ? d.getCompatibleWith() : List.of();
        return new CartaImpresion("✝", d.getDomain(), truncar(d.getName(), MAX_TITULO), "Deidad",
                null, truncar(desc, MAX_DESC), badges, "deidades · " + d.getId());
    }

    private CartaImpresion mapNpc(JsonNode n) {
        String rol = n.path("role").asText("");
        String tier = rol.isEmpty() ? "PNJ" : capitalizar(rol);
        String flavor = n.path("flavorText").asText("");
        String desc = flavor.isEmpty() ? n.path("agenda").asText("") : flavor;
        String faccion = n.path("faction").asText("");
        List<String> badges = faccion.isEmpty() ? List.of() : List.of(capitalizar(faccion));
        return new CartaImpresion("🧑", tier, truncar(n.path("name").asText(), MAX_TITULO), n.path("location").asText(""),
                null, truncar(desc, MAX_DESC), badges, "npcs · " + n.path("id").asText());
    }

    private CartaImpresion mapEnemigo(JsonNode e, String etiqueta) {
        String tier = "TIER " + e.path("tier").asInt(1);
        String rol = etiqueta + " · " + capitalizar(e.path("role").asText(""));
        String meta = truncar("Vida " + e.path("derived").path("vida").asInt(0) + " · CA " + e.path("derived").path("ca").asInt(0), MAX_META);
        String pasiva = e.path("passive").path("description").asText("");
        String desc = pasiva.isEmpty() ? e.path("flavorText").asText("") : pasiva;
        String faccion = e.path("faction").asText("");
        String faccionNom = e.path("factionName").asText("");
        List<String> badges = faccion.isEmpty() ? List.of() : List.of(faccionNom.isEmpty() ? capitalizar(faccion) : faccionNom);
        String icono = "Villano".equals(etiqueta) ? "👑" : "💀";
        return new CartaImpresion(icono, tier, truncar(e.path("name").asText(), MAX_TITULO), rol,
                meta, truncar(desc, MAX_DESC), badges, "enemigos · " + e.path("id").asText());
    }

    private CartaImpresion mapLoot(JsonNode l, CatalogoAventuraRepository catalogo) {
        int t0 = l.path("tierRange").path(0).asInt(1);
        int t1 = l.path("tierRange").path(1).asInt(t0);
        String tier = "TIER " + t0 + "-" + t1;
        String meta = l.has("gold") ? "Oro " + l.path("gold").path("min").asInt(0) + "-" + l.path("gold").path("max").asInt(0) : null;
        String flavor = l.path("flavorText").asText("");
        String desc = flavor.isEmpty() ? l.path("clue").asText("") : flavor;
        List<String> badges = new ArrayList<>();
        JsonNode drops = l.path("drops");
        for (int i = 0; i < drops.size() && i < 3; i++) {
            String itemId = drops.get(i).path("item").asText();
            // Nombre legible del objeto, nunca el id en bruto (data/cartas/armas|consumibles).
            badges.add(capitalizar(catalogo.nombreDe(itemId)));
        }
        return new CartaImpresion("💰", tier, truncar(l.path("name").asText(), MAX_TITULO), "Botín",
                meta, truncar(desc, MAX_DESC), badges, "loot · " + l.path("id").asText());
    }

    private CartaImpresion mapTrampa(JsonNode t) {
        String tier = "TIER " + t.path("tier").asInt(1);
        String categoria = t.path("category").asText("");
        String rol = categoria.isEmpty() ? "Trampa" : capitalizar(categoria);
        String meta = t.has("detection")
                ? truncar("Detección " + t.path("detection").path("stat").asText("") + " CD " + t.path("detection").path("cd").asInt(0), MAX_META)
                : null;
        String efecto = t.path("effect").path("description").asText("");
        String desc = efecto.isEmpty() ? t.path("flavorText").asText("") : efecto;
        String rareza = t.path("rarity").asText("");
        List<String> badges = rareza.isEmpty() ? List.of() : List.of(capitalizar(rareza));
        return new CartaImpresion("🪤", tier, truncar(t.path("name").asText(), MAX_TITULO), rol,
                meta, truncar(desc, MAX_DESC), badges, "trampas · " + t.path("id").asText());
    }

    @GetMapping("/{id}/editar")
    public String editar(@PathVariable Long id, Model model) {
        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventura", aventuraService.obtenirPerId(id));
        anadirCatalogo(model);
        return "aventuras/formulari";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable Long id, @ModelAttribute("aventura") Aventura aventura) {
        aventura.setId(id);
        aventuraService.guardar(aventura);
        return "redirect:/aventuras/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable Long id) {
        aventuraService.eliminar(id);
        return "redirect:/aventuras";
    }

    // ==== Constructor de aventuras por actos (GDD §20 · Fase 1) ====

    @GetMapping("/{id}/constructor")
    public String constructor(@PathVariable Long id,
                              @org.springframework.web.bind.annotation.RequestParam(required = false) String edit,
                              Model model) {
        Aventura aventura = aventuraService.obtenirPerId(id);
        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventura", aventura);

        cat.dnd.cc.web.form.CartaAventuraForm form = new cat.dnd.cc.web.form.CartaAventuraForm();
        String editando = null;
        if (edit != null && !edit.isBlank()) {
            for (cat.dnd.cc.model.CartaAventura c : aventura.getCartasHistoria()) {
                if (edit.equals(c.getCode())) {
                    form = constructorService.aFormulario(c);
                    editando = c.getCode();
                    break;
                }
            }
        }
        model.addAttribute("cartaForm", form);
        model.addAttribute("editando", editando);
        anadirCatalogo(model);
        return "aventuras/constructor";
    }

    @PostMapping("/{id}/cartas")
    public String anadirCarta(@PathVariable Long id,
                              @ModelAttribute("cartaForm") cat.dnd.cc.web.form.CartaAventuraForm form) {
        constructorService.anadirCarta(id, form);
        return "redirect:/aventuras/" + id + "/constructor";
    }

    @PostMapping("/{id}/cartas/{code}/editar")
    public String editarCarta(@PathVariable Long id, @PathVariable String code,
                              @ModelAttribute("cartaForm") cat.dnd.cc.web.form.CartaAventuraForm form) {
        constructorService.editarCarta(id, code, form);
        return "redirect:/aventuras/" + id + "/constructor";
    }

    @PostMapping("/{id}/cartas/{code}/mover")
    public String moverCarta(@PathVariable Long id, @PathVariable String code,
                             @org.springframework.web.bind.annotation.RequestParam int actoDestino) {
        constructorService.moverDeActo(id, code, actoDestino);
        return "redirect:/aventuras/" + id + "/constructor";
    }

    @PostMapping("/{id}/cartas/{code}/borrar")
    public String borrarCarta(@PathVariable Long id, @PathVariable String code) {
        constructorService.eliminarCarta(id, code);
        return "redirect:/aventuras/" + id + "/constructor";
    }

    // ==== Dependencias y validación (GDD §20.4 · Fase 2, wireframe 13) ====

    /**
     * Pantalla 13: grafo de dependencias por actos (13a, con sandbox de fichas en cliente)
     * + panel de validación con las incidencias del {@code ValidadorAventuraService} (13b).
     * <p>
     * El sandbox no llama al servidor: se sirve un catálogo ligero de las cartas
     * (código, acto, tipo, requisito y ramas) que {@code /js/dependencias.js} usa para
     * simular el filtrado del GDD §20.3 al marcar fichas Verde/Roja.
     */
    @GetMapping("/{id}/dependencias")
    public String dependencias(@PathVariable Long id, Model model) {
        Aventura aventura = aventuraService.obtenirPerId(id);
        List<cat.dnd.cc.service.ValidadorAventuraService.Incidencia> incidencias =
                validadorService.validar(aventura);

        // Catálogo ligero para el sandbox JS (mismo patrón que enemigosLigeros).
        List<java.util.Map<String, Object>> cartasLigeras = new ArrayList<>();
        for (cat.dnd.cc.model.CartaAventura c : aventura.getCartasHistoria()) {
            java.util.Map<String, Object> m = new java.util.LinkedHashMap<>();
            m.put("code", c.getCode() == null ? "" : c.getCode());
            m.put("acto", c.getActo());
            m.put("tipo", c.getTipo().name());
            m.put("titulo", c.getTitulo() == null ? "" : c.getTitulo());
            if (c.getActivacion() != null && c.getActivacion().getRequiereCode() != null
                    && !c.getActivacion().getRequiereCode().isBlank()) {
                m.put("req", java.util.Map.of(
                        "code", c.getActivacion().getRequiereCode(),
                        "estado", c.getActivacion().getEstado() == null ? "" : c.getActivacion().getEstado().name()));
            }
            List<java.util.Map<String, String>> ramas = new ArrayList<>();
            for (cat.dnd.cc.model.CartaAventura.Rama r : c.getRamas()) {
                if (r.getCuandoCode() == null || r.getCuandoCode().isBlank()) continue;
                ramas.add(java.util.Map.of(
                        "code", r.getCuandoCode(),
                        "estado", r.getCuandoEstado() == null ? "" : r.getCuandoEstado().name()));
            }
            if (!ramas.isEmpty()) m.put("ramas", ramas);
            cartasLigeras.add(m);
        }

        long errores = incidencias.stream()
                .filter(i -> i.gravedad() == cat.dnd.cc.service.ValidadorAventuraService.Gravedad.ERROR).count();
        model.addAttribute("currentPage", "aventuras");
        model.addAttribute("aventura", aventura);
        model.addAttribute("incidencias", incidencias);
        model.addAttribute("numErrores", errores);
        model.addAttribute("numAvisos", incidencias.size() - errores);
        model.addAttribute("cartasLigeras", cartasLigeras);
        return "aventuras/dependencias";
    }

    private void anadirCatalogo(Model model) {
        model.addAttribute("historiasDisponibles", historiaService.listarTodas());
        model.addAttribute("facciones", historiaService.facciones());
        model.addAttribute("deidadesDisponibles", tierDeityService.listarTodas());
        var catalogo = aventuraService.catalogo();
        model.addAttribute("npcsDisponibles", catalogo.npcs());
        model.addAttribute("villanosDisponibles", catalogo.villanos());
        model.addAttribute("enemigosDisponibles", catalogo.enemigosRegulares());
        model.addAttribute("lootDisponible", catalogo.lootTables());
        model.addAttribute("trampasDisponibles", catalogo.trampas());
        // catálogo mínimo para el generador de combates aleatorios en cliente
        model.addAttribute("enemigosLigeros", catalogo.enemigosRegulares().stream()
                .map(r -> java.util.Map.of("id", r.getId(), "n", r.getName(),
                        "t", r.getTier(), "r", r.getRank(), "f", r.getFaction() == null ? "" : r.getFaction()))
                .toList());
    }
}
