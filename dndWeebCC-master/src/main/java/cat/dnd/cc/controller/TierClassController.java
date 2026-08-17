package cat.dnd.cc.controller;

import cat.dnd.cc.eines.CalculadoraVida;
import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.service.TierClassService;
import cat.dnd.cc.web.form.TierClassForm;
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
 * Editor web de cartas de clase del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md).
 * Permite crear, editar y combinar clases propias en JSON, guardadas en data/cartas/clases.
 */
@Controller
@RequestMapping("/cartas/clases")
public class TierClassController {

    private final TierClassService tierClassService;

    public TierClassController(TierClassService tierClassService) {
        this.tierClassService = tierClassService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierClass> cartas = tierClassService.listarTodas();
        model.addAttribute("currentPage", "cartas-clases");
        model.addAttribute("cartas", cartas);
        return "cartas/clases/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-clases");
        model.addAttribute("form", new TierClassForm());
        model.addAttribute("esNueva", true);
        return "cartas/clases/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierClassForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-clases");
            model.addAttribute("esNueva", true);
            return "cartas/clases/formulario";
        }
        TierClass carta = aModelo(form, null);
        tierClassService.guardar(carta);
        return "redirect:/cartas/clases/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierClass carta = tierClassService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-clases");
        model.addAttribute("carta", carta);
        model.addAttribute("previsionVida", CalculadoraVida.previsualizar(carta));
        return "cartas/clases/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierClass carta = tierClassService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-clases");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/clases/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierClassForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-clases");
            model.addAttribute("esNueva", false);
            return "cartas/clases/formulario";
        }
        TierClass carta = aModelo(form, id);
        tierClassService.guardar(carta);
        return "redirect:/cartas/clases/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierClassService.eliminar(id);
        return "redirect:/cartas/clases";
    }

    private TierClass aModelo(TierClassForm form, String idExistente) {
        TierClass carta = new TierClass();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setRole(form.getRole());
        carta.setTier(form.getTier());
        carta.setBaseHealth(form.getBaseHealth());
        carta.setHealthScalingCon(form.getHealthScalingCon());
        carta.setPrimaryStat(form.getPrimaryStat());
        carta.setSecondaryStat(blancoANulo(form.getSecondaryStat()));
        carta.setPrimaryResource(form.getPrimaryResource());
        carta.setSecondaryResource(blancoANulo(form.getSecondaryResource()));
        carta.setStartingEquipment(ListaTexto.splitCsv(form.getStartingEquipment()));

        TierClass.StartingCards startingCards = new TierClass.StartingCards();
        startingCards.setPassives(ListaTexto.splitCsv(form.getStartingPassives()));
        startingCards.setSkills(ListaTexto.splitCsv(form.getStartingSkills()));
        startingCards.setSpells(ListaTexto.splitCsv(form.getStartingSpells()));
        startingCards.setLearnableSkills(ListaTexto.splitCsv(form.getLearnableSkills()));
        carta.setStartingCards(startingCards);

        carta.setAllowedEquipmentTags(ListaTexto.splitCsv(form.getAllowedEquipmentTags()));
        carta.setRestrictedTags(ListaTexto.splitCsv(form.getRestrictedTags()));
        carta.setMaxArmorWeight(form.getMaxArmorWeight());
        carta.setMaxWeaponWeight(form.getMaxWeaponWeight());
        carta.setSpecializations(ListaTexto.splitCsv(form.getSpecializations()));
        carta.setDescription(form.getDescription());
        return carta;
    }

    private TierClassForm aFormulario(TierClass carta) {
        TierClassForm form = new TierClassForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setRole(carta.getRole());
        form.setTier(carta.getTier());
        form.setBaseHealth(carta.getBaseHealth());
        form.setHealthScalingCon(carta.getHealthScalingCon());
        form.setPrimaryStat(carta.getPrimaryStat());
        form.setSecondaryStat(carta.getSecondaryStat());
        form.setPrimaryResource(carta.getPrimaryResource());
        form.setSecondaryResource(carta.getSecondaryResource());
        form.setStartingEquipment(ListaTexto.joinCsv(carta.getStartingEquipment()));
        form.setStartingPassives(ListaTexto.joinCsv(carta.getStartingCards().getPassives()));
        form.setStartingSkills(ListaTexto.joinCsv(carta.getStartingCards().getSkills()));
        form.setStartingSpells(ListaTexto.joinCsv(carta.getStartingCards().getSpells()));
        form.setLearnableSkills(ListaTexto.joinCsv(carta.getStartingCards().getLearnableSkills()));
        form.setAllowedEquipmentTags(ListaTexto.joinCsv(carta.getAllowedEquipmentTags()));
        form.setRestrictedTags(ListaTexto.joinCsv(carta.getRestrictedTags()));
        form.setMaxArmorWeight(carta.getMaxArmorWeight());
        form.setMaxWeaponWeight(carta.getMaxWeaponWeight());
        form.setSpecializations(ListaTexto.joinCsv(carta.getSpecializations()));
        form.setDescription(carta.getDescription());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
