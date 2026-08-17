package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierCondition;
import cat.dnd.cc.service.TierConditionService;
import cat.dnd.cc.web.form.TierConditionForm;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;

import java.util.ArrayList;
import java.util.List;

/**
 * Editor web de cartas de condición (Estado/Bendición/Maldición) del sistema de tiers y cartas
 * (docs/Sistema_Cartas_Tiers.md, 7.4, y docs/Arquitectura_Datos_Onegai.md, 5.12).
 */
@Controller
@RequestMapping("/cartas/condiciones")
public class TierConditionController {

    private final TierConditionService tierConditionService;

    public TierConditionController(TierConditionService tierConditionService) {
        this.tierConditionService = tierConditionService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierCondition> cartas = tierConditionService.listarTodas();
        model.addAttribute("currentPage", "cartas-condiciones");
        model.addAttribute("cartas", cartas);
        return "cartas/condiciones/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-condiciones");
        model.addAttribute("form", new TierConditionForm());
        model.addAttribute("esNueva", true);
        return "cartas/condiciones/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierConditionForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-condiciones");
            model.addAttribute("esNueva", true);
            return "cartas/condiciones/formulario";
        }
        TierCondition carta = aModelo(form, null);
        tierConditionService.guardar(carta);
        return "redirect:/cartas/condiciones/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierCondition carta = tierConditionService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-condiciones");
        model.addAttribute("carta", carta);
        return "cartas/condiciones/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierCondition carta = tierConditionService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-condiciones");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/condiciones/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierConditionForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-condiciones");
            model.addAttribute("esNueva", false);
            return "cartas/condiciones/formulario";
        }
        TierCondition carta = aModelo(form, id);
        tierConditionService.guardar(carta);
        return "redirect:/cartas/condiciones/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierConditionService.eliminar(id);
        return "redirect:/cartas/condiciones";
    }

    private TierCondition aModelo(TierConditionForm form, String idExistente) {
        TierCondition carta = new TierCondition();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setCategory(form.getCategory());
        carta.setSource(form.getSource());
        carta.setStackable(form.isStackable());
        carta.setDuration(form.getDuration());
        List<TierCondition.Effect> efectos = new ArrayList<>();
        for (String[] par : ListaTexto.splitPares(form.getEffects())) {
            efectos.add(new TierCondition.Effect(par[0], par[1]));
        }
        carta.setEffects(efectos);
        carta.setCureConditions(ListaTexto.splitCsv(form.getCureConditions()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierConditionForm aFormulario(TierCondition carta) {
        TierConditionForm form = new TierConditionForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setCategory(carta.getCategory());
        form.setSource(carta.getSource());
        form.setStackable(carta.isStackable());
        form.setDuration(carta.getDuration());
        List<String[]> pares = new ArrayList<>();
        for (TierCondition.Effect efecto : carta.getEffects()) {
            pares.add(new String[]{efecto.getDescription(), efecto.getMechanicHook()});
        }
        form.setEffects(ListaTexto.joinPares(pares));
        form.setCureConditions(ListaTexto.joinCsv(carta.getCureConditions()));
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
