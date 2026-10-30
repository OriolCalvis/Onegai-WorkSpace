package cat.dnd.cc.controller;

import cat.dnd.cc.model.TierConsumable;
import cat.dnd.cc.service.TierConsumableService;
import cat.dnd.cc.web.form.TierConsumableForm;
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
 * Editor web de cartas de consumible del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.8).
 */
@Controller
@RequestMapping("/cartas/consumibles")
public class TierConsumableController {

    private final TierConsumableService tierConsumableService;

    public TierConsumableController(TierConsumableService tierConsumableService) {
        this.tierConsumableService = tierConsumableService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierConsumable> cartas = tierConsumableService.listarTodas();
        model.addAttribute("currentPage", "cartas-consumibles");
        model.addAttribute("cartas", cartas);
        return "cartas/consumibles/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-consumibles");
        model.addAttribute("form", new TierConsumableForm());
        model.addAttribute("esNueva", true);
        return "cartas/consumibles/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierConsumableForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-consumibles");
            model.addAttribute("esNueva", true);
            return "cartas/consumibles/formulario";
        }
        TierConsumable carta = aModelo(form, null);
        tierConsumableService.guardar(carta);
        return "redirect:/cartas/consumibles/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierConsumable carta = tierConsumableService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-consumibles");
        model.addAttribute("carta", carta);
        return "cartas/consumibles/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierConsumable carta = tierConsumableService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-consumibles");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/consumibles/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierConsumableForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-consumibles");
            model.addAttribute("esNueva", false);
            return "cartas/consumibles/formulario";
        }
        TierConsumable carta = aModelo(form, id);
        tierConsumableService.guardar(carta);
        return "redirect:/cartas/consumibles/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierConsumableService.eliminar(id);
        return "redirect:/cartas/consumibles";
    }

    private TierConsumable aModelo(TierConsumableForm form, String idExistente) {
        TierConsumable carta = new TierConsumable();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setTier(form.getTier());
        carta.setRarity(form.getRarity());
        carta.setActionType(form.getActionType());
        carta.getEffect().setDescription(form.getEffectDescription());
        carta.setUses(form.getUses());
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierConsumableForm aFormulario(TierConsumable carta) {
        TierConsumableForm form = new TierConsumableForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setTier(carta.getTier());
        form.setRarity(carta.getRarity());
        form.setActionType(carta.getActionType());
        form.setEffectDescription(carta.getEffect().getDescription());
        form.setUses(carta.getUses());
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
