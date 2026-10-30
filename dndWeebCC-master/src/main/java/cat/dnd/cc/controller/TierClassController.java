package cat.dnd.cc.controller;

import cat.dnd.cc.eines.CalculadoraVida;
import cat.dnd.cc.eines.ListaTexto;
import cat.dnd.cc.eines.mazo.MazoDeClase;
import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.service.MazoClaseService;
import cat.dnd.cc.service.TierClassService;
import cat.dnd.cc.web.form.TierClassForm;
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
import org.springframework.web.bind.annotation.RequestParam;

import java.util.List;

/**
 * Editor web de cartas de clase del sistema de tiers y cartas (docs/Sistema_Cartas_Tiers.md).
 * Permite crear, editar y combinar clases propias en JSON, guardadas en data/cartas/clases.
 */
@Controller
@RequestMapping("/cartas/clases")
public class TierClassController {

    private final TierClassService tierClassService;
    private final MazoClaseService mazoClaseService;

    public TierClassController(TierClassService tierClassService, MazoClaseService mazoClaseService) {
        this.tierClassService = tierClassService;
        this.mazoClaseService = mazoClaseService;
    }

    @GetMapping
    public String listar(Model model) {
        List<TierClass> cartas = tierClassService.listarTodas();
        model.addAttribute("currentPage", "cartas-clases");
        model.addAttribute("cartas", cartas);
        // CARTAS-CLASES-01: familia de cada clase (filtro del álbum y descarga de mazos)
        model.addAttribute("familias", MazoDeClase.FAMILIAS);
        model.addAttribute("familiaPorId", mazoClaseService.familiaPorClase());
        model.addAttribute("recuentoFamilias", mazoClaseService.recuentoPorFamilia());
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
        model.addAttribute("techoVida", CalculadoraVida.techoTier(Math.max(1, carta.getTier())));
        // CARTAS-CLASES-01: el mazo físico de la clase, tal como sale en el PDF
        model.addAttribute("mazo", mazoClaseService.mazo(carta, MazoDeClase.Opciones.porDefecto()));
        model.addAttribute("familiaClase", MazoDeClase.nombreFamilia(carta));
        model.addAttribute("claseEd4", MazoDeClase.esEd4(carta));
        return "cartas/clases/detalle";
    }

    /**
     * CARTAS-CLASES-01 — El mazo físico de la clase en PDF: hojas A4 con 9 cartas de
     * 63,5 × 88,9 mm para recortar y jugar en la mesa. Por defecto lleva la clase, su pasiva,
     * sus cartas estándar y de Creador, sus invocaciones, el equipo inicial y las cartas de
     * reglas de mesa; el set de Tier 2 solo si se pide ({@code set=true}). Con
     * {@code descargar=false} el navegador lo abre en vez de guardarlo.
     */
    @GetMapping("/{id}/mazo.pdf")
    public ResponseEntity<byte[]> mazoPdf(@PathVariable String id,
                                          @RequestParam(defaultValue = "false") boolean sinCreador,
                                          @RequestParam(defaultValue = "false") boolean sinEquipo,
                                          @RequestParam(defaultValue = "false") boolean set,
                                          @RequestParam(defaultValue = "false") boolean sinReglas,
                                          @RequestParam(defaultValue = "true") boolean descargar) {
        TierClass carta = tierClassService.obtenerPorId(id);
        byte[] pdf = mazoClaseService.pdf(carta, opciones(sinCreador, sinEquipo, set, sinReglas));
        return fichero(pdf, MediaType.APPLICATION_PDF, "mazo-" + id + ".pdf", descargar);
    }

    /** Los mazos de una familia (o de todas) en un solo PDF; cada clase empieza en hoja nueva. */
    @GetMapping("/mazos.pdf")
    public ResponseEntity<byte[]> mazosPdf(@RequestParam(defaultValue = "todas") String familia,
                                           @RequestParam(defaultValue = "false") boolean sinCreador,
                                           @RequestParam(defaultValue = "false") boolean sinEquipo,
                                           @RequestParam(defaultValue = "false") boolean set,
                                           @RequestParam(defaultValue = "false") boolean sinReglas) {
        String clave = claveFamiliaSegura(familia);
        List<TierClass> lista = mazoClaseService.clasesDeFamilia(clave);
        byte[] pdf = mazoClaseService.pdfDeVarias(lista, opciones(sinCreador, sinEquipo, set, sinReglas));
        return fichero(pdf, MediaType.APPLICATION_PDF, "mazos-" + clave + ".pdf", true);
    }

    /** Los mazos de una familia (o de todas) en un ZIP, un PDF por clase. */
    @GetMapping("/mazos.zip")
    public ResponseEntity<byte[]> mazosZip(@RequestParam(defaultValue = "todas") String familia,
                                           @RequestParam(defaultValue = "false") boolean sinCreador,
                                           @RequestParam(defaultValue = "false") boolean sinEquipo,
                                           @RequestParam(defaultValue = "false") boolean set,
                                           @RequestParam(defaultValue = "false") boolean sinReglas) {
        String clave = claveFamiliaSegura(familia);
        List<TierClass> lista = mazoClaseService.clasesDeFamilia(clave);
        byte[] zip = mazoClaseService.zip(lista, opciones(sinCreador, sinEquipo, set, sinReglas));
        return fichero(zip, MediaType.parseMediaType("application/zip"), "mazos-" + clave + ".zip", true);
    }

    /**
     * Lo que lleva el mazo. Los parámetros van «en negativo» (sinCreador, sinEquipo, sinReglas)
     * para que un enlace sin parámetros dé el mazo completo y una casilla sin marcar del
     * formulario no quite nada; el set de Tier 2, que no se empieza con él, se pide aparte.
     */
    private static MazoDeClase.Opciones opciones(boolean sinCreador, boolean sinEquipo, boolean set, boolean sinReglas) {
        return new MazoDeClase.Opciones(!sinCreador, !sinEquipo, set, !sinReglas);
    }

    /** Solo claves conocidas en el nombre del fichero (nada de lo que llegue por la URL tal cual). */
    private static String claveFamiliaSegura(String familia) {
        return MazoDeClase.FAMILIAS.containsKey(familia) ? familia : "todas";
    }

    private static ResponseEntity<byte[]> fichero(byte[] datos, MediaType tipo, String nombre, boolean descargar) {
        return ResponseEntity.ok()
                .contentType(tipo)
                .header(HttpHeaders.CONTENT_DISPOSITION,
                        (descargar ? "attachment" : "inline") + "; filename=\"" + nombre + "\"")
                .body(datos);
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
        // CARTAS-CLASES-01: el formulario no conoce todos los campos de la carta (ganchos,
        // rareza, familia, set...); se copian de la guardada para no borrarlos al editar.
        carta.copiarExtrasDe(tierClassService.obtenerPorId(id));
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
