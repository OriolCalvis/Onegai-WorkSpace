package cat.dnd.cc.controller;

import cat.dnd.cc.model.Historia;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import cat.dnd.cc.service.HistoriaService;
import cat.dnd.cc.service.TierBackgroundService;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.server.ResponseStatusException;

import java.io.IOException;
import java.text.Normalizer;
import java.util.List;
import java.util.TreeSet;

/**
 * Álbum de PNJs (data/npcs): reparto reutilizable de la mesa del director, con su rol,
 * ubicación, agenda, actitud inicial y el secreto que los conecta con una historia real.
 * Mismo enfoque que el álbum de Monstruos: solo lectura, sobre el catálogo que ya usa
 * el constructor de aventuras (CatalogoAventuraRepository), sin duplicar el índice.
 */
@Controller
@RequestMapping("/npcs")
public class NpcController {

    private final CatalogoAventuraRepository catalogo;
    private final HistoriaService historiaService;
    private final TierBackgroundService tierBackgroundService;
    private final ObjectMapper objectMapper;

    public NpcController(CatalogoAventuraRepository catalogo, HistoriaService historiaService,
                         TierBackgroundService tierBackgroundService, ObjectMapper objectMapper) {
        this.catalogo = catalogo;
        this.historiaService = historiaService;
        this.tierBackgroundService = tierBackgroundService;
        this.objectMapper = objectMapper;
    }

    @GetMapping
    public String listar(Model model) {
        List<CatalogoAventuraRepository.Resumen> npcs = catalogo.npcs();

        TreeSet<String> roles = new TreeSet<>();
        for (CatalogoAventuraRepository.Resumen n : npcs) {
            if (n.getRank() != null && !n.getRank().isBlank()) {
                roles.add(n.getRank());
            }
        }

        model.addAttribute("currentPage", "npcs");
        model.addAttribute("npcs", npcs);
        model.addAttribute("roles", roles);
        return "npcs/lista";
    }

    /** Formulario de creación de PNJ (wireframe 10a): más corto que crear
     * personaje/aventura — sin stats de combate, solo lo narrativo. */
    @GetMapping("/crear")
    public String crear(Model model) {
        TreeSet<String> roles = new TreeSet<>();
        for (CatalogoAventuraRepository.Resumen n : catalogo.npcs()) {
            if (n.getRank() != null && !n.getRank().isBlank()) {
                roles.add(n.getRank());
            }
        }
        model.addAttribute("currentPage", "npcs");
        model.addAttribute("roles", roles);
        model.addAttribute("facciones", historiaService.facciones());
        model.addAttribute("meses", tierBackgroundService.listarTodas());
        model.addAttribute("historias", historiaService.listarTodas());
        return "npcs/formulario";
    }

    @PostMapping("/crear")
    public String guardar(@RequestParam String name,
                          @RequestParam(required = false) String role,
                          @RequestParam(required = false) String faction,
                          @RequestParam(required = false) String location,
                          @RequestParam(required = false) String month,
                          @RequestParam(required = false) String agenda,
                          @RequestParam(required = false) String actitud,
                          @RequestParam(required = false) String palanca,
                          @RequestParam(required = false) String secretHook,
                          @RequestParam(required = false) String flavorText) {
        if (name == null || name.isBlank()) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "El PNJ necesita un nombre");
        }
        String id = "npc_" + slug(name);

        ObjectNode npc = objectMapper.createObjectNode();
        npc.put("id", id);
        npc.put("name", name.trim());
        npc.put("type", "npc");
        npc.put("role", vacioComoNull(role) == null ? "secundario" : role.trim());
        npc.put("location", vacioComoNull(location));
        npc.put("faction", vacioComoNull(faction));
        npc.put("agenda", vacioComoNull(agenda));
        ObjectNode attitude = npc.putObject("attitude");
        attitude.put("inicial", vacioComoNull(actitud) == null ? "neutral" : actitud.trim());
        attitude.put("palanca", vacioComoNull(palanca));
        npc.putArray("dialogue");
        npc.putNull("services");
        npc.put("secretHook", vacioComoNull(secretHook));
        npc.put("month", vacioComoNull(month));
        npc.put("flavorText", vacioComoNull(flavorText));

        try {
            catalogo.guardarNpc(npc);
        } catch (IOException e) {
            throw new ResponseStatusException(HttpStatus.INTERNAL_SERVER_ERROR,
                    "No se pudo guardar el PNJ: " + e.getMessage());
        }
        return "redirect:/npcs/" + id;
    }

    private static String vacioComoNull(String s) {
        return (s == null || s.isBlank()) ? null : s.trim();
    }

    /** "Mercader Adiel" → "mercader_adiel" (mismo estilo de id que el catálogo). */
    private static String slug(String nombre) {
        String sinAcentos = Normalizer.normalize(nombre, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "");
        return sinAcentos.trim().toLowerCase()
                .replaceAll("[^a-z0-9]+", "_")
                .replaceAll("^_+|_+$", "");
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        JsonNode carta = catalogo.completa(id);
        if (carta == null || !"npc".equals(carta.path("type").asText())) {
            throw new ResponseStatusException(HttpStatus.NOT_FOUND, "PNJ no encontrado: " + id);
        }
        model.addAttribute("currentPage", "npcs");
        model.addAttribute("carta", carta);
        model.addAttribute("historiaSecreta", historiaOrNull(carta.path("secretHook").asText(null)));
        return "npcs/detalle";
    }

    /** El secretHook de un pnj apunta a una historia real, pero no todos los pnjs tienen
     * uno — resuelto aparte para que la ficha pueda enlazar el título sin reventar si el
     * id viniera vacío o desincronizado. */
    private Historia historiaOrNull(String id) {
        if (id == null || id.isBlank()) {
            return null;
        }
        try {
            return historiaService.obtenerPorId(id);
        } catch (RuntimeException e) {
            return null;
        }
    }
}
