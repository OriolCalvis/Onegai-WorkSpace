package cat.dnd.cc.controller;

import cat.dnd.cc.model.Evento;
import cat.dnd.cc.model.GeografiaMapa;
import cat.dnd.cc.service.EventoService;
import cat.dnd.cc.service.GeografiaMapaService;
import cat.dnd.cc.service.HistoriaService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.ArrayList;
import java.util.List;

/**
 * CRUD de Eventos (docs/Arquitectura_Datos_Onegai.md §5.6): algo que pasa en el mundo,
 * con un disparador, efectos automáticos y opciones para que el grupo reaccione.
 * El campo "effects" (List&lt;String&gt;) se edita como textarea de una línea por efecto
 * — no viaja en el binding de *{...} de Evento porque el tipo no calza (String del
 * formulario vs List&lt;String&gt; del modelo), así que se recoge aparte y se parte aquí.
 */
@Controller
@RequestMapping("/eventos")
public class EventoController {

    private final EventoService eventoService;
    private final HistoriaService historiaService;
    private final GeografiaMapaService geografiaMapaService;

    public EventoController(EventoService eventoService, HistoriaService historiaService,
                             GeografiaMapaService geografiaMapaService) {
        this.eventoService = eventoService;
        this.historiaService = historiaService;
        this.geografiaMapaService = geografiaMapaService;
    }

    @GetMapping
    public String listar(Model model) {
        model.addAttribute("currentPage", "eventos");
        model.addAttribute("eventos", eventoService.listarTodos());
        return "eventos/lista";
    }

    @GetMapping("/crear")
    public String mostrarFormulario(Model model) {
        model.addAttribute("currentPage", "eventos");
        model.addAttribute("evento", new Evento());
        model.addAttribute("facciones", historiaService.facciones());
        return "eventos/formulario";
    }

    @PostMapping("/crear")
    public String crear(@ModelAttribute("evento") Evento evento,
                         @RequestParam(name = "effectsTexto", required = false) String effectsTexto) {
        aplicarEffectsTexto(evento, effectsTexto);
        eventoService.guardar(evento);
        return "redirect:/eventos";
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        Evento evento = eventoService.obtenerPorId(id);
        model.addAttribute("currentPage", "eventos");
        model.addAttribute("evento", evento);

        // Vínculos con el Mapa Mundi: la propia facción del evento (si tiene) y
        // cualquier punto personalizado que lo referencie desde el editor del mapa.
        java.util.Set<String> faccionesTocadas = new java.util.LinkedHashSet<>();
        if (evento.getFaction() != null && !evento.getFaction().isBlank()) {
            faccionesTocadas.add(evento.getFaction());
        }
        GeografiaMapa geografia = geografiaMapaService.cargar();
        List<GeografiaMapa.Punto> puntosVinculados = geografia.getPuntos().stream()
                .filter(p -> p.getEventoIds().contains(id))
                .toList();
        model.addAttribute("faccionesTocadas", faccionesTocadas);
        model.addAttribute("puntosVinculados", puntosVinculados);
        return "eventos/detalle";
    }

    @GetMapping("/{id}/editar")
    public String editar(@PathVariable String id, Model model) {
        model.addAttribute("currentPage", "eventos");
        model.addAttribute("evento", eventoService.obtenerPorId(id));
        model.addAttribute("facciones", historiaService.facciones());
        return "eventos/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("evento") Evento evento,
                              @RequestParam(name = "effectsTexto", required = false) String effectsTexto) {
        evento.setId(id);
        aplicarEffectsTexto(evento, effectsTexto);
        eventoService.guardar(evento);
        return "redirect:/eventos/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        eventoService.eliminar(id);
        return "redirect:/eventos";
    }

    private void aplicarEffectsTexto(Evento evento, String effectsTexto) {
        List<String> lineas = new ArrayList<>();
        if (effectsTexto != null) {
            for (String linea : effectsTexto.split("\\r?\\n")) {
                if (!linea.isBlank()) lineas.add(linea.strip());
            }
        }
        evento.setEffects(lineas);
        // Los th:field de playerOptions solo mandan filas con contenido si el usuario las
        // rellenó, pero una fila añadida y luego vaciada por JS puede llegar como objeto
        // vacío — se descarta para no guardar basura.
        evento.setPlayerOptions(evento.getPlayerOptions() == null ? new ArrayList<>()
                : evento.getPlayerOptions().stream()
                        .filter(o -> o.getText() != null && !o.getText().isBlank())
                        .toList());
    }
}
