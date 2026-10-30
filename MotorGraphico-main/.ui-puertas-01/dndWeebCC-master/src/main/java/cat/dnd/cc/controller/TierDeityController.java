package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierDeity;
import cat.dnd.cc.service.TierDeityService;
import cat.dnd.cc.web.form.TierDeityForm;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;

import java.util.List;

/**
 * Editor web de cartas de deidad del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.11).
 */
@Controller
@RequestMapping("/cartas/deidades")
public class TierDeityController {

    private final TierDeityService tierDeityService;

    public TierDeityController(TierDeityService tierDeityService) {
        this.tierDeityService = tierDeityService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierDeity> cartas = tierDeityService.listarTodas();
        model.addAttribute("currentPage", "cartas-deidades");
        model.addAttribute("cartas", cartas);
        return "cartas/deidades/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-deidades");
        model.addAttribute("form", new TierDeityForm());
        model.addAttribute("esNueva", true);
        return "cartas/deidades/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierDeityForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-deidades");
            model.addAttribute("esNueva", true);
            return "cartas/deidades/formulario";
        }
        TierDeity carta = aModelo(form, null);
        tierDeityService.guardar(carta);
        return "redirect:/cartas/deidades/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierDeity carta = tierDeityService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-deidades");
        model.addAttribute("carta", carta);
        return "cartas/deidades/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierDeity carta = tierDeityService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-deidades");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/deidades/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierDeityForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-deidades");
            model.addAttribute("esNueva", false);
            return "cartas/deidades/formulario";
        }
        TierDeity carta = aModelo(form, id);
        tierDeityService.guardar(carta);
        return "redirect:/cartas/deidades/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierDeityService.eliminar(id);
        return "redirect:/cartas/deidades";
    }

    private TierDeity aModelo(TierDeityForm form, String idExistente) {
        TierDeity carta = new TierDeity();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setDomain(form.getDomain());
        carta.getFavor().setDescription(form.getFavorDescription());
        carta.getFavor().setScaling(form.getFavorScaling());
        carta.setCompatibleWith(ListaTexto.splitCsv(form.getCompatibleWith()));
        carta.setObligations(ListaTexto.splitCsv(form.getObligations()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierDeityForm aFormulario(TierDeity carta) {
        TierDeityForm form = new TierDeityForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setDomain(carta.getDomain());
        form.setFavorDescription(carta.getFavor().getDescription());
        form.setFavorScaling(carta.getFavor().getScaling());
        form.setCompatibleWith(ListaTexto.joinCsv(carta.getCompatibleWith()));
        form.setObligations(ListaTexto.joinCsv(carta.getObligations()));
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
