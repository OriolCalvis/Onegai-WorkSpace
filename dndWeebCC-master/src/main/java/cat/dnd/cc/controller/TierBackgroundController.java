package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierBackground;
import cat.dnd.cc.model.TierRace;
import cat.dnd.cc.service.TierBackgroundService;
import cat.dnd.cc.web.form.TierBackgroundForm;
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
 * Editor web de cartas de trasfondo del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md).
 * Permite crear y editar trasfondos propios en JSON, guardados en data/cartas/transfondos.
 */
@Controller
@RequestMapping("/cartas/transfondos")
public class TierBackgroundController {

    private final TierBackgroundService tierBackgroundService;

    public TierBackgroundController(TierBackgroundService tierBackgroundService) {
        this.tierBackgroundService = tierBackgroundService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierBackground> cartas = tierBackgroundService.listarTodas();
        model.addAttribute("currentPage", "cartas-transfondos");
        model.addAttribute("cartas", cartas);
        return "cartas/transfondos/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-transfondos");
        model.addAttribute("form", new TierBackgroundForm());
        model.addAttribute("esNueva", true);
        return "cartas/transfondos/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierBackgroundForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-transfondos");
            model.addAttribute("esNueva", true);
            return "cartas/transfondos/formulario";
        }
        TierBackground carta = aModelo(form, null);
        tierBackgroundService.guardar(carta);
        return "redirect:/cartas/transfondos/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierBackground carta = tierBackgroundService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-transfondos");
        model.addAttribute("carta", carta);
        return "cartas/transfondos/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierBackground carta = tierBackgroundService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-transfondos");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/transfondos/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierBackgroundForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-transfondos");
            model.addAttribute("esNueva", false);
            return "cartas/transfondos/formulario";
        }
        TierBackground carta = aModelo(form, id);
        tierBackgroundService.guardar(carta);
        return "redirect:/cartas/transfondos/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierBackgroundService.eliminar(id);
        return "redirect:/cartas/transfondos";
    }

    private TierBackground aModelo(TierBackgroundForm form, String idExistente) {
        TierBackground carta = new TierBackground();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setTier(form.getTier());
        carta.setStatBonusCon(form.getStatBonusCon());
        carta.setStatBonusDes(form.getStatBonusDes());
        carta.setStatBonusCar(form.getStatBonusCar());
        carta.setStatBonusInt(form.getStatBonusInt());
        carta.setNarrativeSkills(ListaTexto.splitCsv(form.getNarrativeSkills()));
        carta.setContacts(ListaTexto.splitCsv(form.getContacts()));
        carta.setBonusEquipment(ListaTexto.splitCsv(form.getBonusEquipment()));
        carta.setNarrativePassive(new TierRace.Trait(form.getNarrativePassiveName(), form.getNarrativePassiveDescription()));
        carta.setComplication(form.getComplication());
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierBackgroundForm aFormulario(TierBackground carta) {
        TierBackgroundForm form = new TierBackgroundForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setTier(carta.getTier());
        form.setStatBonusCon(carta.getStatBonusCon());
        form.setStatBonusDes(carta.getStatBonusDes());
        form.setStatBonusCar(carta.getStatBonusCar());
        form.setStatBonusInt(carta.getStatBonusInt());
        form.setNarrativeSkills(ListaTexto.joinCsv(carta.getNarrativeSkills()));
        form.setContacts(ListaTexto.joinCsv(carta.getContacts()));
        form.setBonusEquipment(ListaTexto.joinCsv(carta.getBonusEquipment()));
        form.setNarrativePassiveName(carta.getNarrativePassive().getName());
        form.setNarrativePassiveDescription(carta.getNarrativePassive().getDescription());
        form.setComplication(carta.getComplication());
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
