package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierFeat;
import cat.dnd.cc.service.TierFeatService;
import cat.dnd.cc.web.form.TierFeatForm;
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
 * Editor web de cartas de dote del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.6).
 */
@Controller
@RequestMapping("/cartas/dotes")
public class TierFeatController {

    private final TierFeatService tierFeatService;

    public TierFeatController(TierFeatService tierFeatService) {
        this.tierFeatService = tierFeatService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierFeat> cartas = tierFeatService.listarTodas();
        model.addAttribute("currentPage", "cartas-dotes");
        model.addAttribute("cartas", cartas);
        return "cartas/dotes/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-dotes");
        model.addAttribute("form", new TierFeatForm());
        model.addAttribute("esNueva", true);
        return "cartas/dotes/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierFeatForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-dotes");
            model.addAttribute("esNueva", true);
            return "cartas/dotes/formulario";
        }
        TierFeat carta = aModelo(form, null);
        tierFeatService.guardar(carta);
        return "redirect:/cartas/dotes/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierFeat carta = tierFeatService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-dotes");
        model.addAttribute("carta", carta);
        return "cartas/dotes/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierFeat carta = tierFeatService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-dotes");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/dotes/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierFeatForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-dotes");
            model.addAttribute("esNueva", false);
            return "cartas/dotes/formulario";
        }
        TierFeat carta = aModelo(form, id);
        tierFeatService.guardar(carta);
        return "redirect:/cartas/dotes/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierFeatService.eliminar(id);
        return "redirect:/cartas/dotes";
    }

    private TierFeat aModelo(TierFeatForm form, String idExistente) {
        TierFeat carta = new TierFeat();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setTier(form.getTier());
        carta.setRarity(form.getRarity());
        carta.setClassTags(ListaTexto.splitCsv(form.getClassTags()));
        carta.setReqCon(form.getReqCon());
        carta.setReqDes(form.getReqDes());
        carta.setReqCar(form.getReqCar());
        carta.setReqInt(form.getReqInt());
        carta.setRequiredTags(ListaTexto.splitCsv(form.getRequiredTags()));
        carta.setIncompatibleTags(ListaTexto.splitCsv(form.getIncompatibleTags()));
        carta.setGrantedTags(ListaTexto.splitCsv(form.getGrantedTags()));
        carta.getEffect().setDescription(form.getEffectDescription());
        carta.getEffect().setScaling(form.getEffectScaling());
        carta.setLimitations(ListaTexto.splitCsv(form.getLimitations()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierFeatForm aFormulario(TierFeat carta) {
        TierFeatForm form = new TierFeatForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setTier(carta.getTier());
        form.setRarity(carta.getRarity());
        form.setClassTags(ListaTexto.joinCsv(carta.getClassTags()));
        form.setReqCon(carta.getReqCon());
        form.setReqDes(carta.getReqDes());
        form.setReqCar(carta.getReqCar());
        form.setReqInt(carta.getReqInt());
        form.setRequiredTags(ListaTexto.joinCsv(carta.getRequiredTags()));
        form.setIncompatibleTags(ListaTexto.joinCsv(carta.getIncompatibleTags()));
        form.setGrantedTags(ListaTexto.joinCsv(carta.getGrantedTags()));
        form.setEffectDescription(carta.getEffect().getDescription());
        form.setEffectScaling(carta.getEffect().getScaling());
        form.setLimitations(ListaTexto.joinCsv(carta.getLimitations()));
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
