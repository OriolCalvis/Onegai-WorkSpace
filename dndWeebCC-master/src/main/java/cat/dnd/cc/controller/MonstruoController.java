package cat.dnd.cc.controller;

import cat.dnd.cc.repository.CatalogoAventuraRepository;
import com.fasterxml.jackson.databind.JsonNode;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.server.ResponseStatusException;
import org.springframework.http.HttpStatus;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

/**
 * Álbum de monstruos (data/cartas/enemigos): catálogo de lectura para consultar en mesa,
 * separado del constructor de aventuras (que ya usa este mismo catálogo internamente para
 * resolver villanos, enemigos sueltos y sus cartas completas).
 * <p>
 * De momento es solo consulta (lista + ficha) — no hay alta/edición web todavía porque el
 * esquema de un enemigo es mucho más variable que el resto de cartas (fases de jefe, perfil
 * de villano, arena, acciones legendarias... son opcionales y no todos los monstruos los
 * tienen), así que se lee tal cual está en disco en vez de forzarlo a un formulario rígido.
 */
@Controller
@RequestMapping("/cartas/enemigos")
public class MonstruoController {

    private final CatalogoAventuraRepository catalogo;

    public MonstruoController(CatalogoAventuraRepository catalogo) {
        this.catalogo = catalogo;
    }

    @GetMapping
    public String listar(Model model) {
        List<CatalogoAventuraRepository.Resumen> monstruos = new ArrayList<>();
        monstruos.addAll(catalogo.villanos());
        monstruos.addAll(catalogo.enemigosRegulares());
        monstruos.sort(Comparator.comparingInt(CatalogoAventuraRepository.Resumen::getTier)
                .thenComparing(CatalogoAventuraRepository.Resumen::getName, String.CASE_INSENSITIVE_ORDER));

        model.addAttribute("currentPage", "cartas-enemigos");
        model.addAttribute("monstruos", monstruos);
        model.addAttribute("totalVillanos", catalogo.villanos().size());
        model.addAttribute("totalRegulares", catalogo.enemigosRegulares().size());
        return "cartas/enemigos/lista";
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        JsonNode carta = catalogo.completa(id);
        if (carta == null || !"enemy".equals(carta.path("type").asText())) {
            throw new ResponseStatusException(HttpStatus.NOT_FOUND, "Monstruo no encontrado: " + id);
        }
        model.addAttribute("currentPage", "cartas-enemigos");
        model.addAttribute("carta", carta);
        return "cartas/enemigos/detalle";
    }
}
