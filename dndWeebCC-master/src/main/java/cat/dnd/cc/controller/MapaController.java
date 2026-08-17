package cat.dnd.cc.controller;

import cat.dnd.cc.model.Aventura;
import cat.dnd.cc.model.Evento;
import cat.dnd.cc.model.GeografiaMapa;
import cat.dnd.cc.model.Historia;
import cat.dnd.cc.model.PuntoMapa;
import cat.dnd.cc.model.RegionMapa;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import cat.dnd.cc.service.AventuraService;
import cat.dnd.cc.service.EventoService;
import cat.dnd.cc.service.GeografiaMapaService;
import cat.dnd.cc.service.HistoriaService;
import cat.dnd.cc.service.TierDeityService;
import cat.dnd.cc.service.TierRaceService;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.TreeSet;
import java.util.stream.Collectors;

/**
 * Mapa mundi interactivo: las 15 facciones del catálogo son también las 15 localizaciones
 * de sus historias (relación 1:1 comprobada en data/historias). Este controlador no crea
 * datos nuevos — agrupa lo que ya existe (historias, monstruos, tesoros, pnjs, aventuras)
 * por región para el panel lateral de /mapa. Además resuelve los "puntos" personalizados
 * (localizaciones que no encajan en ninguna facción) que el director crea a mano desde el
 * editor del mapa.
 *
 * La geografía (polígonos de nación + posición de marcadores + puntos personalizados) vive
 * en data/mapa/geografia.json vía GeografiaMapaService, no en código: el editor de puntos de
 * /mapa la lee al cargar la página y la reescribe con POST /mapa/guardar, así que ajustar el
 * mapa no requiere tocar Java ni reiniciar la app.
 */
@Controller
public class MapaController {

    private final HistoriaService historiaService;
    private final CatalogoAventuraRepository catalogo;
    private final AventuraService aventuraService;
    private final EventoService eventoService;
    private final GeografiaMapaService geografiaMapaService;
    private final TierDeityService deidadService;
    private final TierRaceService razaService;

    public MapaController(HistoriaService historiaService, CatalogoAventuraRepository catalogo,
                           AventuraService aventuraService, EventoService eventoService,
                           GeografiaMapaService geografiaMapaService,
                           TierDeityService deidadService, TierRaceService razaService) {
        this.historiaService = historiaService;
        this.catalogo = catalogo;
        this.aventuraService = aventuraService;
        this.eventoService = eventoService;
        this.geografiaMapaService = geografiaMapaService;
        this.deidadService = deidadService;
        this.razaService = razaService;
    }

    /** Icono de cada región — no es geografía, es solo un detalle visual del marcador,
     * así que se queda en código en vez de en el JSON editable. */
    private static final Map<String, String> ICONO = new LinkedHashMap<>();
    static {
        ICONO.put("milicia_corrupta_de_llerba", "🏰");
        ICONO.put("manada_del_rei_llop", "🐺");
        ICONO.put("sagas_de_la_luna_hueca", "🌙");
        ICONO.put("forjados_sin_amo", "⚙️");
        ICONO.put("hechiceros_del_vacio", "🔮");
        ICONO.put("corte_de_espinas", "🌳");
        ICONO.put("clan_de_gongorguma_renegado", "⛰️");
        ICONO.put("cofradia_de_mascaras_de_plata", "🎭");
        ICONO.put("yokai_del_umbral", "⛩️");
        ICONO.put("bandidos_del_camino_de_ceniza", "🗡️");
        ICONO.put("horrores_del_cielo_fragmentado", "🌌");
        ICONO.put("espectros_de_la_tomba", "💀");
        ICONO.put("renacidos_de_la_fosa", "⚰️");
        ICONO.put("nagas_del_pozo_azul", "🌊");
        ICONO.put("plaga_de_san_lazaro", "☠️");
    }

    @PostMapping("/mapa/guardar")
    @ResponseBody
    public ResponseEntity<Map<String, Object>> guardar(@RequestBody GeografiaMapa geografia) {
        try {
            geografiaMapaService.guardar(geografia);
            return ResponseEntity.ok(Map.of("ok", true));
        } catch (IOException e) {
            return ResponseEntity.internalServerError().body(Map.of("ok", false, "error", e.getMessage()));
        }
    }

    @GetMapping("/mapa")
    public String mapa(Model model) {
        GeografiaMapa geografia = geografiaMapaService.cargar();
        Map<String, int[]> posicion = geografia.getMarcadores();
        List<Historia> todas = historiaService.listarTodas();
        List<Aventura> todasAventuras = aventuraService.llistarTotes();
        List<Evento> todosEventos = eventoService.listarTodos();

        // Qué facciones toca cada aventura (vía las historias que contiene), para poder
        // listar "Aventuras" dentro del panel de cada región sin guardar ese vínculo a mano.
        Map<String, List<Aventura>> aventurasPorFaccion = new LinkedHashMap<>();
        for (Aventura aventura : todasAventuras) {
            Set<String> faccionesTocadas = new LinkedHashSet<>();
            for (Historia h : aventuraService.historiasDe(aventura)) {
                if (h.getFaction() != null && !h.getFaction().isBlank()) {
                    faccionesTocadas.add(h.getFaction());
                }
            }
            for (String faccion : faccionesTocadas) {
                aventurasPorFaccion.computeIfAbsent(faccion, k -> new ArrayList<>()).add(aventura);
            }
        }

        // Los eventos sí llevan su propia facción (campo opcional del formulario), así
        // que agruparlos es directo — no hace falta pasar por historias como con aventuras.
        Map<String, List<Evento>> eventosPorFaccion = new LinkedHashMap<>();
        for (Evento evento : todosEventos) {
            if (evento.getFaction() != null && !evento.getFaction().isBlank()) {
                eventosPorFaccion.computeIfAbsent(evento.getFaction(), k -> new ArrayList<>()).add(evento);
            }
        }

        // Agrupar por facción (= localización, es 1:1 en el catálogo actual).
        Map<String, List<Historia>> porFaccion = new LinkedHashMap<>();
        for (Historia h : todas) {
            porFaccion.computeIfAbsent(h.getFaction(), k -> new ArrayList<>()).add(h);
        }

        List<RegionMapa> regiones = new ArrayList<>();
        for (Map.Entry<String, List<Historia>> entry : porFaccion.entrySet()) {
            String faccion = entry.getKey();
            List<Historia> historiasFaccion = entry.getValue();
            String location = historiasFaccion.get(0).getLocation();
            String faccionNombre = historiasFaccion.get(0).getFactionNombre();

            Map<String, List<Historia>> historiasPorTrama = new LinkedHashMap<>();
            for (Historia h : historiasFaccion) {
                String trama = (h.getChain() != null && h.getChain().getTrama() != null)
                        ? h.getChain().getTrama() : "sin_trama";
                historiasPorTrama.computeIfAbsent(trama, k -> new ArrayList<>()).add(h);
            }
            for (List<Historia> pasos : historiasPorTrama.values()) {
                pasos.sort(Comparator.comparingInt(h -> h.getChain() != null ? h.getChain().getStep() : 0));
            }

            List<CatalogoAventuraRepository.Resumen> monstruos = new ArrayList<>();
            for (CatalogoAventuraRepository.Resumen r : catalogo.villanos()) {
                if (faccion.equals(r.getFaction())) monstruos.add(r);
            }
            for (CatalogoAventuraRepository.Resumen r : catalogo.enemigosRegulares()) {
                if (faccion.equals(r.getFaction())) monstruos.add(r);
            }

            // Los ids de tesoro embeben la facción (loot_<faccion>_t#), no hace falta
            // abrir cada JSON para saber cuáles son suyos.
            List<CatalogoAventuraRepository.Resumen> tesoros = new ArrayList<>();
            String prefijo = "loot_" + faccion + "_t";
            for (CatalogoAventuraRepository.Resumen r : catalogo.lootTables()) {
                if (r.getId() != null && r.getId().startsWith(prefijo)) tesoros.add(r);
            }

            // Los pnjs no tienen facción propia casi nunca, pero sí comparten el mismo
            // texto de localización que las historias — así se agrupan igual.
            List<CatalogoAventuraRepository.Resumen> npcs = new ArrayList<>();
            for (CatalogoAventuraRepository.Resumen r : catalogo.npcs()) {
                if (location.equals(r.getExtra())) npcs.add(r);
            }

            List<Aventura> aventurasRegion = aventurasPorFaccion.getOrDefault(faccion, List.of());
            List<Evento> eventosRegion = eventosPorFaccion.getOrDefault(faccion, List.of());

            int[] pos = posicion.getOrDefault(faccion, new int[]{500, 325});
            String icono = ICONO.getOrDefault(faccion, "📍");

            regiones.add(new RegionMapa(faccion, location, faccion, faccionNombre, icono, pos[0], pos[1],
                    historiasPorTrama, monstruos, tesoros, npcs, aventurasRegion, eventosRegion));
        }

        // Puntos personalizados: localizaciones creadas a mano desde el editor del mapa,
        // vinculadas manualmente a historias y/o aventuras (no dependen de la facción).
        List<PuntoMapa> puntos = new ArrayList<>();
        for (GeografiaMapa.Punto p : geografia.getPuntos()) {
            List<Historia> historiasPunto = p.getHistoriaIds().stream()
                    .map(id -> {
                        try {
                            return historiaService.obtenerPorId(id);
                        } catch (IllegalArgumentException e) {
                            return null;
                        }
                    })
                    .filter(Objects::nonNull)
                    .toList();
            List<Aventura> aventurasPunto = p.getAventuraIds().stream()
                    .map(id -> {
                        try {
                            return aventuraService.obtenirPerId(Long.valueOf(id));
                        } catch (RuntimeException e) {
                            return null;
                        }
                    })
                    .filter(Objects::nonNull)
                    .toList();
            List<Evento> eventosPunto = p.getEventoIds().stream()
                    .map(id -> {
                        try {
                            return eventoService.obtenerPorId(id);
                        } catch (IllegalArgumentException e) {
                            return null;
                        }
                    })
                    .filter(Objects::nonNull)
                    .toList();
            puntos.add(new PuntoMapa(p.getId(), p.getNombre(), p.getIcono(), p.getX(), p.getY(),
                    historiasPunto, aventurasPunto, eventosPunto));
        }

        // Listas ligeras (id + etiqueta) para los buscadores de "añadir historia/aventura"
        // del editor de puntos — no hace falta mandar el objeto completo al navegador.
        List<Map<String, String>> historiasResumen = todas.stream()
                .sorted(Comparator.comparing(Historia::getTitle, Comparator.nullsLast(String::compareTo)))
                .map(h -> Map.of("id", h.getId(), "titulo", h.getTitle() == null ? h.getId() : h.getTitle(),
                        "tiers", h.getTierRangeExpandido().stream().map(String::valueOf).collect(Collectors.joining(","))))
                .toList();
        List<Map<String, String>> aventurasResumen = todasAventuras.stream()
                .sorted(Comparator.comparing(Aventura::getNom, Comparator.nullsLast(String::compareTo)))
                .map(a -> Map.of("id", String.valueOf(a.getId()), "nombre", a.getNom() == null ? String.valueOf(a.getId()) : a.getNom()))
                .toList();
        List<Map<String, String>> eventosResumen = todosEventos.stream()
                .sorted(Comparator.comparing(Evento::getName, Comparator.nullsLast(String::compareTo)))
                .map(e -> Map.of("id", e.getId(), "nombre", e.getName() == null ? e.getId() : e.getName(),
                        "tier", String.valueOf(e.getTier())))
                .toList();

        // Tiers presentes en el catálogo (no asumimos 1-5 a fuego: se calcula de los
        // datos reales), para pintar los botones del filtro por tier del mapa.
        Set<Integer> tiersDisponibles = new TreeSet<>();
        for (Historia h : todas) {
            tiersDisponibles.addAll(h.getTierRangeExpandido());
        }

        // Ciudades: resolvemos los nombres de sus pnjs vinculados para el panel, pero
        // el objeto Ciudad (con x/y/rasgo/...) se pasa tal cual — la plantilla lo pinta.
        List<Map<String, String>> npcsResumen = catalogo.npcs().stream()
                .sorted(Comparator.comparing(CatalogoAventuraRepository.Resumen::getName,
                        Comparator.nullsLast(String::compareTo)))
                .map(n -> Map.of("id", n.getId(), "nombre", n.getName() == null ? n.getId() : n.getName()))
                .toList();

        // Zonas del terreno y cronología: se pintan tal cual; la cronología ordenada por año.
        List<GeografiaMapa.EventoHistorico> cronologia = new ArrayList<>(geografia.getEventosHistoricos());
        cronologia.sort(Comparator.comparingInt(GeografiaMapa.EventoHistorico::getAno));

        model.addAttribute("currentPage", "mapa");
        model.addAttribute("regiones", regiones);
        model.addAttribute("naciones", geografia.getNaciones());
        model.addAttribute("puntos", puntos);
        model.addAttribute("ciudades", geografia.getCiudades());
        model.addAttribute("zonas", geografia.getZonas());
        model.addAttribute("cronologia", cronologia);
        model.addAttribute("historiasResumen", historiasResumen);
        model.addAttribute("aventurasResumen", aventurasResumen);
        model.addAttribute("eventosResumen", eventosResumen);
        model.addAttribute("npcsResumen", npcsResumen);
        model.addAttribute("tiersDisponibles", tiersDisponibles);

        // Catálogos para los desplegables del panel de nación (dioses y razas se
        // ELIGEN de lista, nunca se escriben a mano — misma regla que en los editores
        // de cartas, ver docs/Auditoria_Campos_Referencia.md).
        model.addAttribute("deidadesResumen", deidadService.listarTodas().stream()
                .sorted(Comparator.comparing(d -> d.getName() == null ? d.getId() : d.getName(),
                        String.CASE_INSENSITIVE_ORDER))
                .map(d -> Map.of("id", d.getId(), "nombre", d.getName() == null ? d.getId() : d.getName()))
                .toList());
        model.addAttribute("razasResumen", razaService.listarTodas().stream()
                .sorted(Comparator.comparing(r -> r.getName() == null ? r.getId() : r.getName(),
                        String.CASE_INSENSITIVE_ORDER))
                .map(r -> Map.of("id", r.getId(), "nombre", r.getName() == null ? r.getId() : r.getName()))
                .toList());
        return "mapa/index";
    }
}
