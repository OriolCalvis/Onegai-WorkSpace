package cat.dnd.cc.controller;

import cat.dnd.cc.combat.SessioCombatService;
import cat.dnd.cc.combat.motor.EstatPiles;
import cat.dnd.cc.combat.motor.Pila;
import cat.dnd.cc.eines.FichaPersonatge;
import cat.dnd.cc.model.Personatge;
import cat.dnd.cc.service.PdfService;
import cat.dnd.cc.service.PersonatgeService;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

@Controller
@RequestMapping("/personatges")
public class PersonatgeController {
    private final PersonatgeService personatgeService;
    private final PdfService pdfService;
    private final ObjectMapper objectMapper;
    private final SessioCombatService sessioCombatService;

    public PersonatgeController(PersonatgeService personatgeService, PdfService pdfService, ObjectMapper objectMapper,
                                 SessioCombatService sessioCombatService) {
        this.personatgeService = personatgeService;
        this.pdfService = pdfService;
        this.objectMapper = objectMapper;
        this.sessioCombatService = sessioCombatService;
    }

    @GetMapping
    public String llistar(Model model) {
        List<Personatge> personatges = personatgeService.llistarTots();
        model.addAttribute("currentPage", "personatges");
        model.addAttribute("personatges", personatges);
        model.addAttribute("fitxes", personatges.stream()
                .collect(java.util.stream.Collectors.toMap(Personatge::getId, personatgeService::calcularFicha)));
        return "personatges/llista";
    }

    @GetMapping("/crear")
    public String mostrarFormulariCreacio(Model model) {
        model.addAttribute("currentPage", "personatges");
        model.addAttribute("personatge", new Personatge());
        afegirCatalegAlModel(model);
        return "personatges/formulari";
    }

    @PostMapping("/crear")
    public String crearPersonatge(@ModelAttribute("personatge") Personatge personatge, BindingResult result, Model model) {
        List<String> errores = validarPersonatge(personatge);
        if (result.hasErrors() || !errores.isEmpty()) {
            model.addAttribute("currentPage", "personatges");
            model.addAttribute("erroresEquipo", errores);
            model.addAttribute("esNueva", true);
            afegirCatalegAlModel(model);
            return "personatges/formulari";
        }
        personatgeService.guardar(personatge);
        return "redirect:/personatges/" + personatge.getId() + "?guardado=1";
    }

    private List<String> validarPersonatge(Personatge personatge) {
        List<String> errores = new ArrayList<>();
        errores.addAll(personatgeService.validarSlotsEquipo(personatge.getEquipoIds()));
        errores.addAll(personatgeService.validarCompatibilidadEquipo(personatge.getClaseId(), personatge.getEquipoIds()));
        errores.addAll(personatgeService.validarLimiteMano(personatge.getTier(), personatge.getHabilidadIds()));
        errores.addAll(personatgeService.validarTierCartas(personatge.getTier(), personatge.getHabilidadIds(),
                personatge.getEquipoIds(), personatge.getHechizoIds()));
        errores.addAll(personatgeService.validarLimiteDotes(personatge.getTier(), personatge.getDoteIds()));
        errores.addAll(personatgeService.validarLimiteDivinas(personatge.getHechizoIds()));
        return errores;
    }

    /** Cuántas cartas mini caben en una hoja A4 con el formato fijo de 45×63mm (4×4). */
    private static final int HABILIDADES_POR_PAGINA = 16;

    @GetMapping("/{id}")
    public String detall(@PathVariable Long id, Model model) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        FichaPersonatge fitxa = personatgeService.calcularFicha(personatge);
        model.addAttribute("currentPage", "personatges");
        model.addAttribute("personatge", personatge);
        model.addAttribute("fitxa", fitxa);
        // Igual que en la impresión de aventuras: el álbum de habilidades puede crecer
        // hasta 20 cartas (tier 3) y no cabe siempre en una sola hoja al imprimir. Se
        // trocea aquí, en el servidor, para que cada página quede acotada y nunca haga
        // falta que el navegador fragmente un grid/álbum largo entre hojas.
        model.addAttribute("habilidadesPaginas", partir(fitxa.habilidades(), HABILIDADES_POR_PAGINA));
        // Zona 4 (Recuperación, D8/D9): estado en vivo de las pilas de este personaje,
        // resuelto de verdad contra sus habilidades/hechizos (no ceros estáticos).
        EstatPiles estatPiles = sessioCombatService.estatPilesDe(personatge);
        model.addAttribute("pilaActiva", estatPiles.comptar(Pila.ACTIVA));
        model.addAttribute("pilaDescansoCorto", estatPiles.comptar(Pila.DESCANSO_CORTO));
        model.addAttribute("pilaDescansoLargo", estatPiles.comptar(Pila.DESCANSO_LARGO));
        return "personatges/detall";
    }

    /** Parte una lista en sublistas de tamaño fijo (la última puede quedar más corta). */
    private static <T> List<List<T>> partir(List<T> lista, int tamano) {
        List<List<T>> paginas = new ArrayList<>();
        for (int i = 0; i < lista.size(); i += tamano) {
            paginas.add(lista.subList(i, Math.min(i + tamano, lista.size())));
        }
        return paginas;
    }

    @GetMapping("/{id}/editar")
    public String editar(@PathVariable Long id, Model model) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        model.addAttribute("currentPage", "personatges");
        model.addAttribute("personatge", personatge);
        afegirCatalegAlModel(model);
        return "personatges/formulari";
    }

    @PostMapping("/{id}/editar")
    public String actualitzar(@PathVariable Long id, @ModelAttribute("personatge") Personatge personatge,
                               BindingResult result, Model model) {
        List<String> errores = validarPersonatge(personatge);
        if (result.hasErrors() || !errores.isEmpty()) {
            model.addAttribute("currentPage", "personatges");
            model.addAttribute("erroresEquipo", errores);
            model.addAttribute("esNueva", false);
            afegirCatalegAlModel(model);
            return "personatges/formulari";
        }
        personatge.setId(id);
        personatgeService.guardar(personatge);
        return "redirect:/personatges/" + id + "?guardado=1";
    }

    private void afegirCatalegAlModel(Model model) {
        model.addAttribute("clasesDisponibles", personatgeService.clasesDisponibles());
        model.addAttribute("razasDisponibles", personatgeService.razasDisponibles());
        model.addAttribute("transfondosDisponibles", personatgeService.transfondosDisponibles());
        model.addAttribute("habilidadesDisponibles", personatgeService.habilidadesDisponibles());
        model.addAttribute("equipoDisponible", personatgeService.equipoDisponible());
        model.addAttribute("dotesDisponibles", personatgeService.dotesDisponibles());
        model.addAttribute("divinasDisponibles", personatgeService.divinasDisponibles());
    }

    @PostMapping("/{id}/eliminar")
    public String eliminar(@PathVariable Long id) {
        personatgeService.eliminar(id);
        return "redirect:/personatges";
    }

    @GetMapping("/{id}/export/pdf")
    public ResponseEntity<byte[]> exportarPdf(@PathVariable Long id) {
        Personatge personatge = personatgeService.obtenirPerId(id);
        byte[] pdf = pdfService.generarPdfPersonatge(personatge, personatgeService.calcularFicha(personatge));
        String filename = "personatge-" + id + ".pdf";
        return ResponseEntity.ok()
                .contentType(MediaType.APPLICATION_PDF)
                .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"" + filename + "\"")
                .body(pdf);
    }

    @GetMapping("/{id}/export/json")
    public ResponseEntity<byte[]> exportarJson(@PathVariable Long id) throws Exception {
        Personatge personatge = personatgeService.obtenirPerId(id);
        String json = objectMapper.writerWithDefaultPrettyPrinter().writeValueAsString(personatge);
        byte[] bytes = json.getBytes(StandardCharsets.UTF_8);
        String filename = "personatge-" + id + ".json";
        return ResponseEntity.ok()
                .contentType(MediaType.APPLICATION_JSON)
                .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"" + filename + "\"")
                .body(bytes);
    }
}
