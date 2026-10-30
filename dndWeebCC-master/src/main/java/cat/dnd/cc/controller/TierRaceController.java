package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierRace;
import cat.dnd.cc.service.TierRaceService;
import cat.dnd.cc.web.form.TierRaceForm;
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
 * Editor web de cartas de raza del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md).
 * Permite crear y editar razas propias en JSON, guardadas en data/cartas/razas.
 */
@Controller
@RequestMapping("/cartas/razas")
public class TierRaceController {

    private final TierRaceService tierRaceService;

    public TierRaceController(TierRaceService tierRaceService) {
        this.tierRaceService = tierRaceService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierRace> cartas = tierRaceService.listarTodas();
        model.addAttribute("currentPage", "cartas-razas");
        model.addAttribute("cartas", cartas);
        return "cartas/razas/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-razas");
        model.addAttribute("form", new TierRaceForm());
        model.addAttribute("esNueva", true);
        return "cartas/razas/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierRaceForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-razas");
            model.addAttribute("esNueva", true);
            return "cartas/razas/formulario";
        }
        TierRace carta = aModelo(form, null);
        tierRaceService.guardar(carta);
        return "redirect:/cartas/razas/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierRace carta = tierRaceService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-razas");
        model.addAttribute("carta", carta);
        return "cartas/razas/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierRace carta = tierRaceService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-razas");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/razas/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierRaceForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-razas");
            model.addAttribute("esNueva", false);
            return "cartas/razas/formulario";
        }
        TierRace carta = aModelo(form, id);
        tierRaceService.guardar(carta);
        return "redirect:/cartas/razas/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierRaceService.eliminar(id);
        return "redirect:/cartas/razas";
    }

    private TierRace aModelo(TierRaceForm form, String idExistente) {
        TierRace carta = new TierRace();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setTier(form.getTier());
        carta.setStatBonusCon(form.getStatBonusCon());
        carta.setStatBonusDes(form.getStatBonusDes());
        carta.setStatBonusCar(form.getStatBonusCar());
        carta.setStatBonusInt(form.getStatBonusInt());
        carta.setPassiveTrait(new TierRace.Trait(form.getPassiveTraitName(), form.getPassiveTraitDescription()));
        if (form.getActiveTraitName() != null && !form.getActiveTraitName().isBlank()) {
            carta.setActiveTrait(new TierRace.Trait(form.getActiveTraitName(), form.getActiveTraitDescription()));
        } else {
            carta.setActiveTrait(new TierRace.Trait());
        }
        carta.setAffinities(ListaTexto.splitCsv(form.getAffinities()));
        carta.setLimitations(ListaTexto.splitCsv(form.getLimitations()));
        carta.setNarrativeTags(ListaTexto.splitCsv(form.getNarrativeTags()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierRaceForm aFormulario(TierRace carta) {
        TierRaceForm form = new TierRaceForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setTier(carta.getTier());
        form.setStatBonusCon(carta.getStatBonusCon());
        form.setStatBonusDes(carta.getStatBonusDes());
        form.setStatBonusCar(carta.getStatBonusCar());
        form.setStatBonusInt(carta.getStatBonusInt());
        form.setPassiveTraitName(carta.getPassiveTrait().getName());
        form.setPassiveTraitDescription(carta.getPassiveTrait().getDescription());
        form.setActiveTraitName(carta.getActiveTrait().getName());
        form.setActiveTraitDescription(carta.getActiveTrait().getDescription());
        form.setAffinities(ListaTexto.joinCsv(carta.getAffinities()));
        form.setLimitations(ListaTexto.joinCsv(carta.getLimitations()));
        form.setNarrativeTags(ListaTexto.joinCsv(carta.getNarrativeTags()));
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
