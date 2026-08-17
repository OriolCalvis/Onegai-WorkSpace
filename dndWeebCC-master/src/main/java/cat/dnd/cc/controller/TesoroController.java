package cat.dnd.cc.controller;

import cat.dnd.cc.repository.CatalogoAventuraRepository;
import com.fasterxml.jackson.databind.JsonNode;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.server.ResponseStatusException;

import java.util.List;

/**
 * Álbum de Tesoros (data/loot): tablas de botín — oro, objetos con probabilidad y la
 * recompensa que cada historia promete al terminar. Mismo enfoque de solo lectura que
 * Monstruos y PNJs, sobre el catálogo que ya usa el constructor de aventuras.
 */
@Controller
@RequestMapping("/tesoros")
public class TesoroController {

    private final CatalogoAventuraRepository catalogo;

    public TesoroController(CatalogoAventuraRepository catalogo) {
        this.catalogo = catalogo;
    }

    @GetMapping
    public String listar(Model model) {
        List<CatalogoAventuraRepository.Resumen> tesoros = catalogo.lootTables();
        model.addAttribute("currentPage", "tesoros");
        model.addAttribute("tesoros", tesoros);
        return "tesoros/lista";
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        JsonNode carta = catalogo.completa(id);
        if (carta == null || !"loot_table".equals(carta.path("type").asText())) {
            throw new ResponseStatusException(HttpStatus.NOT_FOUND, "Tesoro no encontrado: " + id);
        }
        model.addAttribute("currentPage", "tesoros");
        model.addAttribute("carta", carta);
        model.addAttribute("catalogo", catalogo);
        return "tesoros/detalle";
    }
}
