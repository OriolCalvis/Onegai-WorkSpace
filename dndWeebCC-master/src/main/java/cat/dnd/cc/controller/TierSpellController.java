package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.service.TierSpellService;
import cat.dnd.cc.web.form.TierSpellForm;
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
 * Editor web de cartas de hechizo del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.6).
 */
@Controller
@RequestMapping("/cartas/hechizos")
public class TierSpellController {

    private final TierSpellService tierSpellService;

    public TierSpellController(TierSpellService tierSpellService) {
        this.tierSpellService = tierSpellService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierSpell> cartas = tierSpellService.listarTodas();
        model.addAttribute("currentPage", "cartas-hechizos");
        model.addAttribute("cartas", cartas);
        return "cartas/hechizos/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-hechizos");
        model.addAttribute("form", new TierSpellForm());
        model.addAttribute("esNueva", true);
        return "cartas/hechizos/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierSpellForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-hechizos");
            model.addAttribute("esNueva", true);
            return "cartas/hechizos/formulario";
        }
        TierSpell carta = aModelo(form, null);
        tierSpellService.guardar(carta);
        return "redirect:/cartas/hechizos/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierSpell carta = tierSpellService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-hechizos");
        model.addAttribute("carta", carta);
        return "cartas/hechizos/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierSpell carta = tierSpellService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-hechizos");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/hechizos/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierSpellForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-hechizos");
            model.addAttribute("esNueva", false);
            return "cartas/hechizos/formulario";
        }
        TierSpell carta = aModelo(form, id);
        tierSpellService.guardar(carta);
        return "redirect:/cartas/hechizos/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierSpellService.eliminar(id);
        return "redirect:/cartas/hechizos";
    }

    private TierSpell aModelo(TierSpellForm form, String idExistente) {
        TierSpell carta = new TierSpell();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setSchool(form.getSchool());
        carta.setTier(form.getTier());
        carta.setRarity(form.getRarity());
        carta.setClassTags(ListaTexto.splitCsv(form.getClassTags()));
        carta.setCastingStat(form.getCastingStat());
        carta.setRecovery(form.getRecovery());
        carta.setRange(form.getRange());
        carta.setArea(form.getArea());
        carta.setDuration(form.getDuration());
        carta.setMechanicTags(ListaTexto.splitCsv(form.getMechanicTags()));
        carta.setRequiredTags(ListaTexto.splitCsv(form.getRequiredTags()));
        carta.setIncompatibleTags(ListaTexto.splitCsv(form.getIncompatibleTags()));
        carta.getEffect().setDescription(form.getEffectDescription());
        carta.getEffect().setScaling(form.getEffectScaling());
        carta.setUpgradeConditions(form.getUpgradeConditions());
        carta.setLimitations(ListaTexto.splitCsv(form.getLimitations()));
        carta.setEvolvesInto(blancoANulo(form.getEvolvesInto()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierSpellForm aFormulario(TierSpell carta) {
        TierSpellForm form = new TierSpellForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setSchool(carta.getSchool());
        form.setTier(carta.getTier());
        form.setRarity(carta.getRarity());
        form.setClassTags(ListaTexto.joinCsv(carta.getClassTags()));
        form.setCastingStat(carta.getCastingStat());
        form.setRecovery(carta.getRecovery());
        form.setRange(carta.getRange());
        form.setArea(carta.getArea());
        form.setDuration(carta.getDuration());
        form.setMechanicTags(ListaTexto.joinCsv(carta.getMechanicTags()));
        form.setRequiredTags(ListaTexto.joinCsv(carta.getRequiredTags()));
        form.setIncompatibleTags(ListaTexto.joinCsv(carta.getIncompatibleTags()));
        form.setEffectDescription(carta.getEffect().getDescription());
        form.setEffectScaling(carta.getEffect().getScaling());
        form.setUpgradeConditions(carta.getUpgradeConditions());
        form.setLimitations(ListaTexto.joinCsv(carta.getLimitations()));
        form.setEvolvesInto(carta.getEvolvesInto());
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
