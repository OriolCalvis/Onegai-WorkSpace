package cat.dnd.cc.controller;

import cat.dnd.cc.model.Historia;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import cat.dnd.cc.service.HistoriaService;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.server.ResponseStatusException;

import java.text.Normalizer;
import java.util.ArrayList;
import java.util.List;

@Controller
@RequestMapping("/historias")
public class HistoriaController {

    private final HistoriaService historiaService;
    private final CatalogoAventuraRepository catalogo;
    private final ObjectMapper objectMapper;

    public HistoriaController(HistoriaService historiaService, CatalogoAventuraRepository catalogo,
                              ObjectMapper objectMapper) {
        this.historiaService = historiaService;
        this.catalogo = catalogo;
        this.objectMapper = objectMapper;
    }

    @GetMapping
    public String listar(Model model) {
        model.addAttribute("currentPage", "historias");
        model.addAttribute("historias", historiaService.listarTodas());
        model.addAttribute("facciones", historiaService.facciones());
        return "historias/lista";
    }

    /**
     * Formulario manual de creación de historia (wireframe 12a): máximo control por
     * historia, cero uso del script — cubre el caso de una historia suelta a medida,
     * no el poblado en lote (que sigue siendo scripts/generar_historias_npcs.py).
     */
    @GetMapping("/crear")
    public String crear(Model model) {
        model.addAttribute("currentPage", "historias");
        model.addAttribute("facciones", historiaService.facciones());
        model.addAttribute("villanos", catalogo.villanos());
        model.addAttribute("enemigos", catalogo.enemigosRegulares());
        model.addAttribute("npcs", catalogo.npcs());
        model.addAttribute("lootTables", catalogo.lootTables());
        model.addAttribute("trampas", catalogo.trampas());
        return "historias/formulario";
    }

    @PostMapping("/crear")
    public String guardar(@RequestParam String title,
                          @RequestParam(defaultValue = "1") int tierMin,
                          @RequestParam(defaultValue = "1") int tierMax,
                          @RequestParam(required = false) String faction,
                          @RequestParam(required = false) String location,
                          @RequestParam(required = false) String hook,
                          @RequestParam(required = false) String antagonist,
                          @RequestParam(required = false) String reward,
                          @RequestParam(required = false) String decision,
                          @RequestParam(required = false) String complication,
                          @RequestParam(required = false) String consequence,
                          @RequestParam(required = false) String flavorText,
                          @RequestParam(required = false) String escenasJson) {
        if (title == null || title.isBlank()) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "La historia necesita un título");
        }
        Historia h = new Historia();
        String slug = slug(title);
        h.setId("hist_" + slug);
        h.setTitle(title.trim());
        h.setHook(vacioComoNull(hook));
        h.setLocation(vacioComoNull(location));
        h.setAntagonist(vacioComoNull(antagonist));
        List<Integer> rango = new ArrayList<>();
        rango.add(Math.min(tierMin, tierMax));
        rango.add(Math.max(tierMin, tierMax));
        h.setTierRange(rango);
        h.setReward(vacioComoNull(reward));
        h.setDecision(vacioComoNull(decision));
        h.setComplication(vacioComoNull(complication));
        h.setConsequence(vacioComoNull(consequence));
        h.setFaction(vacioComoNull(faction));
        h.setFlavorText(vacioComoNull(flavorText));
        Historia.Chain chain = new Historia.Chain();
        chain.setTrama(slug);
        chain.setStep(1);
        chain.setOf(1);
        h.setChain(chain);

        // Escenas (wireframe 13a): llegan como JSON serializado por el propio formulario.
        if (escenasJson != null && !escenasJson.isBlank()) {
            try {
                h.setEscenas(objectMapper.readValue(escenasJson, new TypeReference<List<Historia.Escena>>() { }));
            } catch (Exception e) {
                throw new ResponseStatusException(HttpStatus.BAD_REQUEST,
                        "Escenas mal formadas: " + e.getMessage());
            }
        }

        historiaService.guardar(h);
        return "redirect:/historias/" + h.getId();
    }

    private static String vacioComoNull(String s) {
        return (s == null || s.isBlank()) ? null : s.trim();
    }

    /** "Contrabando de Sal" → "contrabando_de_sal". */
    private static String slug(String nombre) {
        String sinAcentos = Normalizer.normalize(nombre, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "");
        return sinAcentos.trim().toLowerCase()
                .replaceAll("[^a-z0-9]+", "_")
                .replaceAll("^_+|_+$", "");
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        Historia historia = historiaService.obtenerPorId(id);
        model.addAttribute("currentPage", "historias");
        model.addAttribute("historia", historia);
        model.addAttribute("nombreAntagonista", historiaService.nombreAntagonista(historia.getAntagonist()));

        // El antagonista y la recompensa son ids reales del mismo catálogo que usa el
        // constructor de aventuras — si existen, la ficha enlaza directamente a la ficha
        // completa del monstruo y del tesoro en vez de dejarlos como texto suelto.
        JsonNode antagonista = catalogo.completa(historia.getAntagonist());
        model.addAttribute("antagonistaEnlazable", antagonista != null && "enemy".equals(antagonista.path("type").asText()));

        JsonNode tesoro = catalogo.completa(historia.getReward());
        model.addAttribute("tesoroEnlazable", tesoro != null && "loot_table".equals(tesoro.path("type").asText()));
        model.addAttribute("nombreTesoro", tesoro != null ? tesoro.path("name").asText() : historia.getReward());

        // PNJs cuyo secreto (secretHook) apunta a esta historia — vínculo inverso: el
        // pnj ya sabe qué historia lo conecta, aquí mostramos lo contrario, qué pnjs
        // conocen el secreto de esta historia.
        List<CatalogoAventuraRepository.Resumen> pnjsRelacionados = new ArrayList<>();
        for (CatalogoAventuraRepository.Resumen n : catalogo.npcs()) {
            JsonNode npcCompleto = catalogo.completa(n.getId());
            if (npcCompleto != null && id.equals(npcCompleto.path("secretHook").asText(null))) {
                pnjsRelacionados.add(n);
            }
        }
        model.addAttribute("pnjsRelacionados", pnjsRelacionados);

        // Para resolver nombres legibles del reparto de cada escena (wireframe 13a).
        model.addAttribute("catalogo", catalogo);

        return "historias/detalle";
    }
}
