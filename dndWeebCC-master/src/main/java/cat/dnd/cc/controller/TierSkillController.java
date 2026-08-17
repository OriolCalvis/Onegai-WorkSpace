package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.service.TierSkillService;
import cat.dnd.cc.web.form.TierSkillForm;
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
 * Editor web de cartas de habilidad del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.5).
 */
@Controller
@RequestMapping("/cartas/habilidades")
public class TierSkillController {

    private final TierSkillService tierSkillService;

    public TierSkillController(TierSkillService tierSkillService) {
        this.tierSkillService = tierSkillService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierSkill> cartas = tierSkillService.listarTodas();
        model.addAttribute("currentPage", "cartas-habilidades");
        model.addAttribute("cartas", cartas);
        return "cartas/habilidades/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-habilidades");
        model.addAttribute("form", new TierSkillForm());
        model.addAttribute("esNueva", true);
        return "cartas/habilidades/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierSkillForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-habilidades");
            model.addAttribute("esNueva", true);
            return "cartas/habilidades/formulario";
        }
        TierSkill carta = aModelo(form, null);
        tierSkillService.guardar(carta);
        return "redirect:/cartas/habilidades/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierSkill carta = tierSkillService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-habilidades");
        model.addAttribute("carta", carta);
        return "cartas/habilidades/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierSkill carta = tierSkillService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-habilidades");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/habilidades/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierSkillForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-habilidades");
            model.addAttribute("esNueva", false);
            return "cartas/habilidades/formulario";
        }
        TierSkill carta = aModelo(form, id);
        tierSkillService.guardar(carta);
        return "redirect:/cartas/habilidades/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierSkillService.eliminar(id);
        return "redirect:/cartas/habilidades";
    }

    private TierSkill aModelo(TierSkillForm form, String idExistente) {
        TierSkill carta = new TierSkill();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setTier(form.getTier());
        carta.setRarity(form.getRarity());
        carta.setClassTags(ListaTexto.splitCsv(form.getClassTags()));
        carta.setRoleTags(ListaTexto.splitCsv(form.getRoleTags()));
        carta.setMechanicTags(ListaTexto.splitCsv(form.getMechanicTags()));
        carta.setReqCon(form.getReqCon());
        carta.setReqDes(form.getReqDes());
        carta.setReqCar(form.getReqCar());
        carta.setReqInt(form.getReqInt());
        carta.setRequiredTags(ListaTexto.splitCsv(form.getRequiredTags()));
        carta.setIncompatibleTags(ListaTexto.splitCsv(form.getIncompatibleTags()));

        // cost queda como legado (edición 1): ya no se edita desde el formulario, pero se conserva
        // el valor existente en la carta si la hubiera, para no perder datos de ediciones anteriores.
        TierSkill.Cost cost = new TierSkill.Cost();
        cost.setResource(form.getCostResource());
        cost.setAmount(form.getCostAmount());
        carta.setCost(cost);

        carta.setRecovery(form.getRecovery());
        carta.setActionType(form.getActionType());
        carta.setRange(form.getRange());
        carta.setDuration(form.getDuration());
        carta.setDefenseStat(blancoANulo(form.getDefenseStat()));

        TierSkill.Effect effect = new TierSkill.Effect();
        effect.setDescription(form.getEffectDescription());
        effect.setScaling(form.getEffectScaling());
        carta.setEffect(effect);

        carta.setLimitations(ListaTexto.splitCsv(form.getLimitations()));
        carta.setUpgradePath(ListaTexto.splitCsv(form.getUpgradePath()));
        carta.setEvolvesInto(blancoANulo(form.getEvolvesInto()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierSkillForm aFormulario(TierSkill carta) {
        TierSkillForm form = new TierSkillForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setTier(carta.getTier());
        form.setRarity(carta.getRarity());
        form.setClassTags(ListaTexto.joinCsv(carta.getClassTags()));
        form.setRoleTags(ListaTexto.joinCsv(carta.getRoleTags()));
        form.setMechanicTags(ListaTexto.joinCsv(carta.getMechanicTags()));
        form.setReqCon(carta.getReqCon());
        form.setReqDes(carta.getReqDes());
        form.setReqCar(carta.getReqCar());
        form.setReqInt(carta.getReqInt());
        form.setRequiredTags(ListaTexto.joinCsv(carta.getRequiredTags()));
        form.setIncompatibleTags(ListaTexto.joinCsv(carta.getIncompatibleTags()));
        form.setCostResource(carta.getCost().getResource());
        form.setCostAmount(carta.getCost().getAmount());
        form.setRecovery(carta.getRecovery());
        form.setActionType(carta.getActionType());
        form.setRange(carta.getRange());
        form.setDuration(carta.getDuration());
        form.setDefenseStat(carta.getDefenseStat());
        form.setEffectDescription(carta.getEffect().getDescription());
        form.setEffectScaling(carta.getEffect().getScaling());
        form.setLimitations(ListaTexto.joinCsv(carta.getLimitations()));
        form.setUpgradePath(ListaTexto.joinCsv(carta.getUpgradePath()));
        form.setEvolvesInto(carta.getEvolvesInto());
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
