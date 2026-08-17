package cat.dnd.cc.controller;

import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.service.TierEquipmentService;
import cat.dnd.cc.web.form.TierEquipmentForm;
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
 * Editor web de cartas de equipo del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md, 9.7).
 */
@Controller
@RequestMapping("/cartas/armas")
public class TierEquipmentController {

    private final TierEquipmentService tierEquipmentService;

    public TierEquipmentController(TierEquipmentService tierEquipmentService) {
        this.tierEquipmentService = tierEquipmentService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierEquipment> cartas = tierEquipmentService.listarTodas();
        model.addAttribute("currentPage", "cartas-armas");
        model.addAttribute("cartas", cartas);
        return "cartas/armas/lista";
    }

    @GetMapping("/nueva")
    public String mostrarFormularioCreacion(Model model) {
        model.addAttribute("currentPage", "cartas-armas");
        model.addAttribute("form", new TierEquipmentForm());
        model.addAttribute("esNueva", true);
        return "cartas/armas/formulario";
    }

    @PostMapping
    public String crear(@ModelAttribute("form") TierEquipmentForm form, BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-armas");
            model.addAttribute("esNueva", true);
            return "cartas/armas/formulario";
        }
        TierEquipment carta = aModelo(form, null);
        tierEquipmentService.guardar(carta);
        return "redirect:/cartas/armas/" + carta.getId();
    }

    @GetMapping("/{id}")
    public String detalle(@PathVariable String id, Model model) {
        TierEquipment carta = tierEquipmentService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-armas");
        model.addAttribute("carta", carta);
        return "cartas/armas/detalle";
    }

    @GetMapping("/{id}/editar")
    public String mostrarFormularioEdicion(@PathVariable String id, Model model) {
        TierEquipment carta = tierEquipmentService.obtenerPorId(id);
        model.addAttribute("currentPage", "cartas-armas");
        model.addAttribute("form", aFormulario(carta));
        model.addAttribute("esNueva", false);
        return "cartas/armas/formulario";
    }

    @PostMapping("/{id}/editar")
    public String actualizar(@PathVariable String id, @ModelAttribute("form") TierEquipmentForm form,
                              BindingResult result, Model model) {
        if (result.hasErrors()) {
            model.addAttribute("currentPage", "cartas-armas");
            model.addAttribute("esNueva", false);
            return "cartas/armas/formulario";
        }
        TierEquipment carta = aModelo(form, id);
        tierEquipmentService.guardar(carta);
        return "redirect:/cartas/armas/" + id;
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable String id) {
        tierEquipmentService.eliminar(id);
        return "redirect:/cartas/armas";
    }

    private TierEquipment aModelo(TierEquipmentForm form, String idExistente) {
        TierEquipment carta = new TierEquipment();
        carta.setId(idExistente != null ? idExistente : blancoANulo(form.getId()));
        carta.setName(form.getName());
        carta.setSlot(form.getSlot());
        carta.setTier(form.getTier());
        carta.setRarity(form.getRarity());
        carta.setWeightCategory(form.getWeightCategory());
        carta.setBonusCon(form.getBonusCon());
        carta.setBonusDes(form.getBonusDes());
        carta.setBonusCar(form.getBonusCar());
        carta.setBonusInt(form.getBonusInt());
        carta.setBonusHealth(form.getBonusHealth());
        carta.setBonusArmor(form.getBonusArmor());
        carta.setPenaltyCon(form.getPenaltyCon());
        carta.setPenaltyDes(form.getPenaltyDes());
        carta.setPenaltyCar(form.getPenaltyCar());
        carta.setPenaltyInt(form.getPenaltyInt());
        carta.setGrantedTags(ListaTexto.splitCsv(form.getGrantedTags()));
        carta.setLinkedSkill(blancoANulo(form.getLinkedSkill()));
        carta.setReqCon(form.getReqCon());
        carta.setReqDes(form.getReqDes());
        carta.setReqCar(form.getReqCar());
        carta.setReqInt(form.getReqInt());
        carta.setRestrictions(ListaTexto.splitCsv(form.getRestrictions()));
        carta.setFlavorText(form.getFlavorText());
        return carta;
    }

    private TierEquipmentForm aFormulario(TierEquipment carta) {
        TierEquipmentForm form = new TierEquipmentForm();
        form.setId(carta.getId());
        form.setName(carta.getName());
        form.setSlot(carta.getSlot());
        form.setTier(carta.getTier());
        form.setRarity(carta.getRarity());
        form.setWeightCategory(carta.getWeightCategory());
        form.setBonusCon(carta.getBonusCon());
        form.setBonusDes(carta.getBonusDes());
        form.setBonusCar(carta.getBonusCar());
        form.setBonusInt(carta.getBonusInt());
        form.setBonusHealth(carta.getBonusHealth());
        form.setBonusArmor(carta.getBonusArmor());
        form.setPenaltyCon(carta.getPenaltyCon());
        form.setPenaltyDes(carta.getPenaltyDes());
        form.setPenaltyCar(carta.getPenaltyCar());
        form.setPenaltyInt(carta.getPenaltyInt());
        form.setGrantedTags(ListaTexto.joinCsv(carta.getGrantedTags()));
        form.setLinkedSkill(carta.getLinkedSkill());
        form.setReqCon(carta.getReqCon());
        form.setReqDes(carta.getReqDes());
        form.setReqCar(carta.getReqCar());
        form.setReqInt(carta.getReqInt());
        form.setRestrictions(ListaTexto.joinCsv(carta.getRestrictions()));
        form.setFlavorText(carta.getFlavorText());
        return form;
    }

    private String blancoANulo(String texto) {
        return (texto == null || texto.isBlank()) ? null : texto.trim();
    }
}
