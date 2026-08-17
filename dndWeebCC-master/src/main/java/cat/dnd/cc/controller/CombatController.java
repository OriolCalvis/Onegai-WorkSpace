package cat.dnd.cc.controller;

import cat.dnd.cc.combat.MesaCombatService;
import cat.dnd.cc.combat.SessioCombat;
import cat.dnd.cc.combat.SessioCombatService;
import cat.dnd.cc.combat.motor.EstatPiles;
import cat.dnd.cc.eines.FichaPersonatge;
import cat.dnd.cc.model.Personatge;
import cat.dnd.cc.service.PersonatgeService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * Mesa de combate del director de juego (D10) y vista móvil de jugador (D11), backlog WS-D.
 *
 * <p>No hay wireframe previo para estas dos pantallas (los códigos "6a/6b/7a/7b" que citaba
 * el backlog resultaron ser, tras revisarlos, wireframes de diseño de carta ya usados en otro
 * contexto — ver nota en {@code docs/Backlog_Tareas_y_Gantt.md}, filas D10/D11). Este
 * controlador implementa una versión mínima y honesta: iniciativa fija, dock de NPCs
 * introducidos a mano por el DJ (sin resolver todavía contra el catálogo de enemigos), y
 * jugar una carta desde el móvil moviéndola de pila en vivo.</p>
 */
@Controller
@RequestMapping("/combat")
public class CombatController {

    /** Filas de NPC vacías que se ofrecen en el formulario de inicio (dock manual). */
    private static final int SLOTS_NPC_FORMULARI = 6;

    private final PersonatgeService personatgeService;
    private final MesaCombatService mesaCombatService;
    private final SessioCombatService sessioCombatService;

    public CombatController(PersonatgeService personatgeService, MesaCombatService mesaCombatService,
                             SessioCombatService sessioCombatService) {
        this.personatgeService = personatgeService;
        this.mesaCombatService = mesaCombatService;
        this.sessioCombatService = sessioCombatService;
    }

    @GetMapping("/iniciar")
    public String mostrarFormulariInici(Model model) {
        model.addAttribute("currentPage", "combate");
        model.addAttribute("personatges", personatgeService.llistarTots());
        model.addAttribute("slotsNpc", rangeSlots());
        return "combat/iniciar";
    }

    @PostMapping("/iniciar")
    public String iniciar(@RequestParam(name = "nom", defaultValue = "Combate") String nom,
                           @RequestParam(name = "personatgeIds", required = false) List<Long> personatgeIds,
                           @RequestParam(name = "npcNombre", required = false) List<String> npcNombre,
                           @RequestParam(name = "npcIniciativa", required = false) List<String> npcIniciativa,
                           @RequestParam(name = "npcDes", required = false) List<String> npcDes,
                           Model model) {
        List<SessioCombat.Participant> participants = new ArrayList<>();

        if (personatgeIds != null) {
            for (Long id : personatgeIds) {
                Personatge personatge = personatgeService.obtenirPerId(id);
                FichaPersonatge fitxa = personatgeService.calcularFicha(personatge);
                participants.add(new SessioCombat.Participant(
                        "pj-" + id, personatge.getNom(), true, fitxa.iniciativa(), fitxa.finalDes()));
            }
        }

        if (npcNombre != null) {
            for (int i = 0; i < npcNombre.size(); i++) {
                String nomNpc = npcNombre.get(i);
                if (nomNpc == null || nomNpc.isBlank()) {
                    continue;
                }
                double iniciativaNpc = parseDoubleODefecte(valorA(npcIniciativa, i), 0.0);
                int desNpc = (int) parseDoubleODefecte(valorA(npcDes, i), 0.0);
                participants.add(new SessioCombat.Participant(
                        "npc-" + i + "-" + nomNpc, nomNpc, false, iniciativaNpc, desNpc));
            }
        }

        if (participants.isEmpty()) {
            model.addAttribute("currentPage", "combate");
            model.addAttribute("personatges", personatgeService.llistarTots());
            model.addAttribute("slotsNpc", rangeSlots());
            model.addAttribute("error", "Cal com a mínim un participant (personatge o NPC) per començar el combat.");
            return "combat/iniciar";
        }

        mesaCombatService.iniciar(nom, participants);
        return "redirect:/combat/mesa";
    }

    @GetMapping("/mesa")
    public String mesa(Model model) {
        Optional<SessioCombat> sessio = mesaCombatService.actual();
        if (sessio.isEmpty()) {
            return "redirect:/combat/iniciar";
        }
        SessioCombat combat = sessio.get();
        String actualId = combat.participantActualId();
        model.addAttribute("currentPage", "combate");
        model.addAttribute("combat", combat);
        model.addAttribute("participantActual", combat.participantPerId(actualId));
        model.addAttribute("estatTornActual", combat.estatTornDe(actualId));
        return "combat/mesa";
    }

    @PostMapping("/mesa/siguiente-turno")
    public String seguentTorn() {
        mesaCombatService.actual().ifPresent(SessioCombat::seguentTorn);
        return "redirect:/combat/mesa";
    }

    @PostMapping("/mesa/finalizar")
    public String finalitzar() {
        mesaCombatService.finalitzar();
        return "redirect:/combat/iniciar";
    }

    @GetMapping("/jugador/{id}")
    public String jugador(@PathVariable Long id, Model model) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        FichaPersonatge fitxa = personatgeService.calcularFicha(personatge);
        EstatPiles piles = sessioCombatService.estatPilesDe(personatge);
        model.addAttribute("currentPage", "combate");
        model.addAttribute("personatge", personatge);
        model.addAttribute("habilidades", fitxa.habilidades());
        model.addAttribute("piles", piles);
        return "combat/jugador";
    }

    @PostMapping("/jugador/{id}/jugar/{cardId}")
    public String jugarCarta(@PathVariable Long id, @PathVariable String cardId) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        EstatPiles piles = sessioCombatService.estatPilesDe(personatge);
        try {
            piles.jugar(cardId);
        } catch (IllegalStateException jaJugada) {
            // Doble clic / pestaña duplicada: la carta ya no estaba en Activa. No hacemos nada más.
        }
        return "redirect:/combat/jugador/" + id;
    }

    @PostMapping("/jugador/{id}/descansar-curt")
    public String descansarCurt(@PathVariable Long id) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        sessioCombatService.estatPilesDe(personatge).descansarCurt();
        return "redirect:/combat/jugador/" + id;
    }

    @PostMapping("/jugador/{id}/descansar-llarg")
    public String descansarLlarg(@PathVariable Long id) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        sessioCombatService.estatPilesDe(personatge).descansarLlarg();
        return "redirect:/combat/jugador/" + id;
    }

    private static List<Integer> rangeSlots() {
        List<Integer> slots = new ArrayList<>();
        for (int i = 0; i < SLOTS_NPC_FORMULARI; i++) {
            slots.add(i);
        }
        return slots;
    }

    private static String valorA(List<String> llista, int index) {
        return (llista != null && index < llista.size()) ? llista.get(index) : null;
    }

    private static double parseDoubleODefecte(String valor, double perDefecte) {
        if (valor == null || valor.isBlank()) {
            return perDefecte;
        }
        try {
            return Double.parseDouble(valor.trim().replace(',', '.'));
        } catch (NumberFormatException formatIncorrecte) {
            return perDefecte;
        }
    }
}
