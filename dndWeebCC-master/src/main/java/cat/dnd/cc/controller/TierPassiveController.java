package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierPassive;
import cat.dnd.cc.service.TierPassiveService;
import cat.dnd.cc.web.form.TierPassiveForm;
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
 * Editor web de cartas de pasiva del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.4).
 */
@Controller
@RequestMapping("/cartas/pasivas")
public class TierPassiveController {

    private final TierPassiveService tierPassiveService;

    public TierPassiveController(TierPassiveService tierPassiveService) {
        this.tierPassiveService = tierPassiveService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierPassive> cartas = tierPassiveService.listarTodas();
        model.addAttribute("currentPage", "cartas-pasivas");
        model.addAttribute("cartas", cartas);
        return "cartas/pasivas/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-pasivas");
        model.addAttribute("form", new TierPassiveForm());
        model.addAttribute("esNueva", true);
        return "cartas/pasivas/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierPassiveForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-pasivas");
            model.addAttribute("esNueva", true);
            return "cartas/pasivas/formulario";
        }
        TierPassive carta = aModelo(form, null);
        tierPassiveService.guardar(carta);
        return "redirect:/cartas/pasivas/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierPassive carta = tierPassiveService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-pasivas");
        model.addAttribute("carta", carta);
        return "cartas/pasivas/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierPassive carta = tierPassiveService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-pasivas");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/pasivas/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierPassiveForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-pasivas");
            model.addAttribute("esNueva", false);
            return "cartas/pasivas/formulario";
        }
        TierPassive carta = aModelo(form, id);
        tierPassiveService.guardar(carta);
        return "redirect:/cartas/pasivas/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierPassiveService.eliminar(id);
        return "redirect:/cartas/pasivas";
    }

    private TierPassive aModelo(TierPassiveForm form, String idExistente) {
        TierPassive carta = new TierPassive();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setTier(form.getTier());
        carta.setClassTags(ListaTexto.splitCsv(form.getClassTags()));
        carta.setTrigger(form.getTrigger());
        carta.getEffect().setDescription(form.getEffectDescription());
        carta.getEffect().setScaling(form.getEffectScaling());
        carta.setLimitations(ListaTexto.splitCsv(form.getLimitations()));
        carta.setSynergyTags(ListaTexto.splitCsv(form.getSynergyTags()));
        carta.setUnique(form.isUnique());
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierPassiveForm aFormulario(TierPassive carta) {
        TierPassiveForm form = new TierPassiveForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setTier(carta.getTier());
        form.setClassTags(ListaTexto.joinCsv(carta.getClassTags()));
        form.setTrigger(carta.getTrigger());
        form.setEffectDescription(carta.getEffect().getDescription());
        form.setEffectScaling(carta.getEffect().getScaling());
        form.setLimitations(ListaTexto.joinCsv(carta.getLimitations()));
        form.setSynergyTags(ListaTexto.joinCsv(carta.getSynergyTags()));
        form.setUnique(carta.isUnique());
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
