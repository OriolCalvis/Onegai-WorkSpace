package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierSpecialTrait;
import cat.dnd.cc.service.TierSpecialTraitService;
import cat.dnd.cc.web.form.TierSpecialTraitForm;
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
 * Editor web de cartas de rasgo especial del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.9).
 */
@Controller
@RequestMapping("/cartas/rasgos")
public class TierSpecialTraitController {

    private final TierSpecialTraitService tierSpecialTraitService;

    public TierSpecialTraitController(TierSpecialTraitService tierSpecialTraitService) {
        this.tierSpecialTraitService = tierSpecialTraitService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierSpecialTrait> cartas = tierSpecialTraitService.listarTodas();
        model.addAttribute("currentPage", "cartas-rasgos");
        model.addAttribute("cartas", cartas);
        return "cartas/rasgos/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-rasgos");
        model.addAttribute("form", new TierSpecialTraitForm());
        model.addAttribute("esNueva", true);
        return "cartas/rasgos/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierSpecialTraitForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-rasgos");
            model.addAttribute("esNueva", true);
            return "cartas/rasgos/formulario";
        }
        TierSpecialTrait carta = aModelo(form, null);
        tierSpecialTraitService.guardar(carta);
        return "redirect:/cartas/rasgos/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierSpecialTrait carta = tierSpecialTraitService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-rasgos");
        model.addAttribute("carta", carta);
        return "cartas/rasgos/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierSpecialTrait carta = tierSpecialTraitService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-rasgos");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/rasgos/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierSpecialTraitForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-rasgos");
            model.addAttribute("esNueva", false);
            return "cartas/rasgos/formulario";
        }
        TierSpecialTrait carta = aModelo(form, id);
        tierSpecialTraitService.guardar(carta);
        return "redirect:/cartas/rasgos/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierSpecialTraitService.eliminar(id);
        return "redirect:/cartas/rasgos";
    }

    private TierSpecialTrait aModelo(TierSpecialTraitForm form, String idExistente) {
        TierSpecialTrait carta = new TierSpecialTrait();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setOrigin(form.getOrigin());
        carta.setTier(form.getTier());
        carta.getEffect().setDescription(form.getEffectDescription());
        carta.setLimitations(ListaTexto.splitCsv(form.getLimitations()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierSpecialTraitForm aFormulario(TierSpecialTrait carta) {
        TierSpecialTraitForm form = new TierSpecialTraitForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setOrigin(carta.getOrigin());
        form.setTier(carta.getTier());
        form.setEffectDescription(carta.getEffect().getDescription());
        form.setLimitations(ListaTexto.joinCsv(carta.getLimitations()));
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
