package cat.dnd.cc.controller;

import cat.dnd.cc.service.PersonatgeService;
import cat.dnd.cc.service.TierBackgroundService;
import cat.dnd.cc.service.TierClassService;
import cat.dnd.cc.service.TierRaceService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

@Controller
public class HomeController {
    private final PersonatgeService personatgeService;
    private final TierClassService tierClassService;
    private final TierRaceService tierRaceService;
    private final TierBackgroundService tierBackgroundService;

    public HomeController(
            PersonatgeService personatgeService,
            TierClassService tierClassService,
            TierRaceService tierRaceService,
            TierBackgroundService tierBackgroundService
    ) {
        this.personatgeService = personatgeService;
        this.tierClassService = tierClassService;
        this.tierRaceService = tierRaceService;
        this.tierBackgroundService = tierBackgroundService;
    }

    @GetMapping("/")
    public String home(Model model) {
        model.addAttribute("currentPage", "home");
        model.addAttribute("numPersonatges", personatgeService.count());
        model.addAttribute("numTierClases", tierClassService.count());
        model.addAttribute("numTierRazas", tierRaceService.count());
        model.addAttribute("numTierTransfondos", tierBackgroundService.count());
        return "home";
    }

    @GetMapping("/configuracio")
    public String configuracio(Model model) {
        model.addAttribute("currentPage", "configuracio");
        return "configuracio";
    }
}
