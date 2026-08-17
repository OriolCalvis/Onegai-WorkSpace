package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierSummon;
import cat.dnd.cc.service.TierSummonService;
import cat.dnd.cc.web.form.TierSummonForm;
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
 * Editor web de cartas de invocación del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.10).
 */
@Controller
@RequestMapping("/cartas/invocaciones")
public class TierSummonController {

    private final TierSummonService tierSummonService;

    public TierSummonController(TierSummonService tierSummonService) {
        this.tierSummonService = tierSummonService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierSummon> cartas = tierSummonService.listarTodas();
        model.addAttribute("currentPage", "cartas-invocaciones");
        model.addAttribute("cartas", cartas);
        return "cartas/invocaciones/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-invocaciones");
        model.addAttribute("form", new TierSummonForm());
        model.addAttribute("esNueva", true);
        return "cartas/invocaciones/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierSummonForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-invocaciones");
            model.addAttribute("esNueva", true);
            return "cartas/invocaciones/formulario";
        }
        TierSummon carta = aModelo(form, null);
        tierSummonService.guardar(carta);
        return "redirect:/cartas/invocaciones/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierSummon carta = tierSummonService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-invocaciones");
        model.addAttribute("carta", carta);
        return "cartas/invocaciones/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierSummon carta = tierSummonService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-invocaciones");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/invocaciones/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierSummonForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-invocaciones");
            model.addAttribute("esNueva", false);
            return "cartas/invocaciones/formulario";
        }
        TierSummon carta = aModelo(form, id);
        tierSummonService.guardar(carta);
        return "redirect:/cartas/invocaciones/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierSummonService.eliminar(id);
        return "redirect:/cartas/invocaciones";
    }

    private TierSummon aModelo(TierSummonForm form, String idExistente) {
        TierSummon carta = new TierSummon();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setSummonedBy(form.getSummonedBy());
        carta.setTier(form.getTier());
        carta.setHealth(form.getHealth());
        List<TierSummon.Attack> ataques = new ArrayList<>();
        for (String[] par : ListaTexto.splitPares(form.getAttacks())) {
            ataques.add(new TierSummon.Attack(par[0], par[1]));
        }
        carta.setAttacks(ataques);
        carta.setMovement(form.getMovement());
        carta.getPassive().setName(form.getPassiveName());
        carta.getPassive().setDescription(form.getPassiveDescription());
        carta.setDuration(form.getDuration());
        carta.setControl(form.getControl());
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierSummonForm aFormulario(TierSummon carta) {
        TierSummonForm form = new TierSummonForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setSummonedBy(carta.getSummonedBy());
        form.setTier(carta.getTier());
        form.setHealth(carta.getHealth());
        List<String[]> pares = new ArrayList<>();
        for (TierSummon.Attack ataque : carta.getAttacks()) {
            pares.add(new String[]{ataque.getName(), ataque.getEffect()});
        }
        form.setAttacks(ListaTexto.joinPares(pares));
        form.setMovement(carta.getMovement());
        form.setPassiveName(carta.getPassive().getName());
        form.setPassiveDescription(carta.getPassive().getDescription());
        form.setDuration(carta.getDuration());
        form.setControl(carta.getControl());
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
