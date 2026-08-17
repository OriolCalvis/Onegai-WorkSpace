package cat.dnd.cc.controller;

import cat.dnd.cc.service.TierBackgroundService;
import cat.dnd.cc.service.TierClassService;
import cat.dnd.cc.service.TierConditionService;
import cat.dnd.cc.service.TierConsumableService;
import cat.dnd.cc.service.TierDeityService;
import cat.dnd.cc.service.TierEquipmentService;
import cat.dnd.cc.service.TierFeatService;
import cat.dnd.cc.service.TierPassiveService;
import cat.dnd.cc.service.TierRaceService;
import cat.dnd.cc.service.TierSkillService;
import cat.dnd.cc.service.TierSpecialTraitService;
import cat.dnd.cc.service.TierSpellService;
import cat.dnd.cc.service.TierSummonService;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

/**
 * Página de entrada al sistema de tiers y cartas: enlaza a los editores de clases, razas, trasfondos,
 * habilidades, armas/objetos, dotes, pasivas, hechizos, consumibles, rasgos, invocaciones y deidades.
 */
@Controller
public class CartasController {

    private final TierClassService tierClassService;
    private final TierRaceService tierRaceService;
    private final TierBackgroundService tierBackgroundService;
    private final TierSkillService tierSkillService;
    private final TierEquipmentService tierEquipmentService;
    private final TierFeatService tierFeatService;
    private final TierPassiveService tierPassiveService;
    private final TierSpellService tierSpellService;
    private final TierConsumableService tierConsumableService;
    private final TierSpecialTraitService tierSpecialTraitService;
    private final TierSummonService tierSummonService;
    private final TierDeityService tierDeityService;
    private final TierConditionService tierConditionService;
    private final CatalogoAventuraRepository catalogoAventuraRepository;

    public CartasController(TierClassService tierClassService, TierRaceService tierRaceService,
                             TierBackgroundService tierBackgroundService, TierSkillService tierSkillService,
                             TierEquipmentService tierEquipmentService, TierFeatService tierFeatService,
                             TierPassiveService tierPassiveService, TierSpellService tierSpellService,
                             TierConsumableService tierConsumableService, TierSpecialTraitService tierSpecialTraitService,
                             TierSummonService tierSummonService, TierDeityService tierDeityService,
                             TierConditionService tierConditionService, CatalogoAventuraRepository catalogoAventuraRepository) {
        this.tierClassService = tierClassService;
        this.tierRaceService = tierRaceService;
        this.tierBackgroundService = tierBackgroundService;
        this.tierSkillService = tierSkillService;
        this.tierEquipmentService = tierEquipmentService;
        this.tierFeatService = tierFeatService;
        this.tierPassiveService = tierPassiveService;
        this.tierSpellService = tierSpellService;
        this.tierConsumableService = tierConsumableService;
        this.tierSpecialTraitService = tierSpecialTraitService;
        this.tierSummonService = tierSummonService;
        this.tierDeityService = tierDeityService;
        this.tierConditionService = tierConditionService;
        this.catalogoAventuraRepository = catalogoAventuraRepository;
    }

    @GetMapping("/cartas")
    public String index(Model model) {
        model.addAttribute("currentPage", "cartas");
        model.addAttribute("numTierClases", tierClassService.count());
        model.addAttribute("numTierRazas", tierRaceService.count());
        model.addAttribute("numTierTransfondos", tierBackgroundService.count());
        model.addAttribute("numTierHabilidades", tierSkillService.count());
        model.addAttribute("numTierArmas", tierEquipmentService.count());
        model.addAttribute("numTierDotes", tierFeatService.count());
        model.addAttribute("numTierPasivas", tierPassiveService.count());
        model.addAttribute("numTierHechizos", tierSpellService.count());
        model.addAttribute("numTierConsumibles", tierConsumableService.count());
        model.addAttribute("numTierRasgos", tierSpecialTraitService.count());
        model.addAttribute("numTierInvocaciones", tierSummonService.count());
        model.addAttribute("numTierDeidades", tierDeityService.count());
        model.addAttribute("numTierCondiciones", tierConditionService.count());
        model.addAttribute("numMonstruos", catalogoAventuraRepository.villanos().size()
                + catalogoAventuraRepository.enemigosRegulares().size());
        return "cartas/index";
    }
}
