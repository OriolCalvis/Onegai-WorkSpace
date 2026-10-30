package cat.dnd.cc.controller;

import cat.dnd.cc.service.EventoService;
import cat.dnd.cc.service.HistoriaService;
import cat.dnd.cc.service.TierClassService;
import cat.dnd.cc.service.TierConditionService;
import cat.dnd.cc.service.TierEquipmentService;
import cat.dnd.cc.service.TierFeatService;
import cat.dnd.cc.service.TierPassiveService;
import cat.dnd.cc.service.TierRaceService;
import cat.dnd.cc.service.TierSkillService;
import cat.dnd.cc.service.TierSpellService;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ModelAttribute;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.TreeSet;

/**
 * Pone a disposición de los editores de cartas las LISTAS SELECCIONABLES de todo lo que
 * ya existe en los catálogos: clases, habilidades, hechizos, equipo, pasivas, razas,
 * historias/eventos y los vocabularios de tags que ya se usan en los datos.
 *
 * Motivo: los campos que referencian cosas ya creadas (classTags, evolvesInto,
 * startingEquipment, linkedSkill, summonedBy, compatibleWith, continuesTo...) eran texto
 * libre y una falta de ortografía rompía la referencia en silencio. Con desplegables no
 * se puede escribir mal un id.
 *
 * Solo aplica a los controladores listados en assignableTypes (los editores que tienen
 * campos de referencia); el resto de páginas no paga el coste de cargar los catálogos.
 */
@ControllerAdvice(assignableTypes = {
        TierSkillController.class, TierSpellController.class, TierClassController.class,
        TierFeatController.class, TierPassiveController.class, TierEquipmentController.class,
        TierSummonController.class, TierDeityController.class, EventoController.class})
public class OpcionesReferenciaAdvice {

    /** Una opción de desplegable: el id que se guarda y la etiqueta legible que se muestra. */
    public record Opcion(String id, String etiqueta) { }

    private static final Comparator<Opcion> POR_ETIQUETA =
            Comparator.comparing(Opcion::etiqueta, String.CASE_INSENSITIVE_ORDER);

    private final TierClassService clases;
    private final TierSkillService habilidades;
    private final TierSpellService hechizos;
    private final TierEquipmentService equipo;
    private final TierPassiveService pasivas;
    private final TierRaceService razas;
    private final HistoriaService historias;
    private final EventoService eventos;
    private final TierConditionService condiciones;
    private final TierFeatService dotes;

    public OpcionesReferenciaAdvice(TierClassService clases, TierSkillService habilidades,
                                    TierSpellService hechizos, TierEquipmentService equipo,
                                    TierPassiveService pasivas, TierRaceService razas,
                                    HistoriaService historias, EventoService eventos,
                                    TierConditionService condiciones, TierFeatService dotes) {
        this.clases = clases;
        this.habilidades = habilidades;
        this.hechizos = hechizos;
        this.equipo = equipo;
        this.pasivas = pasivas;
        this.razas = razas;
        this.historias = historias;
        this.eventos = eventos;
        this.condiciones = condiciones;
        this.dotes = dotes;
    }

    @ModelAttribute
    public void opcionesDeReferencia(Model model) {
        // ── Catálogos por id ────────────────────────────────────────────────
        List<Opcion> opClases = ordenadas(clases.listarTodas().stream()
                .map(c -> new Opcion(c.getId(), c.getName())).toList());
        List<Opcion> opHabilidades = ordenadas(habilidades.listarTodas().stream()
                .map(h -> new Opcion(h.getId(), h.getName() + " (T" + h.getTier() + ")")).toList());
        List<Opcion> opHechizos = ordenadas(hechizos.listarTodas().stream()
                .map(h -> new Opcion(h.getId(), h.getName() + " (T" + h.getTier() + ")")).toList());
        List<Opcion> opEquipo = ordenadas(equipo.listarTodas().stream()
                .map(e -> new Opcion(e.getId(), e.getName()
                        + (e.getSlot() != null ? " · " + e.getSlot() : ""))).toList());
        List<Opcion> opPasivas = ordenadas(pasivas.listarTodas().stream()
                .map(p -> new Opcion(p.getId(), p.getName())).toList());
        List<Opcion> opRazas = ordenadas(razas.listarTodas().stream()
                .map(r -> new Opcion(r.getId(), r.getName())).toList());

        model.addAttribute("opcionesClases", opClases);
        model.addAttribute("opcionesHabilidades", opHabilidades);
        model.addAttribute("opcionesHechizos", opHechizos);
        model.addAttribute("opcionesEquipo", opEquipo);
        model.addAttribute("opcionesPasivas", opPasivas);
        model.addAttribute("opcionesRazas", opRazas);

        // Invocaciones.summonedBy: puede ser un hechizo o una habilidad.
        List<Opcion> invocables = new ArrayList<>();
        hechizos.listarTodas().forEach(h -> invocables.add(new Opcion(h.getId(), "Hechizo: " + h.getName())));
        habilidades.listarTodas().forEach(h -> invocables.add(new Opcion(h.getId(), "Habilidad: " + h.getName())));
        model.addAttribute("opcionesInvocables", ordenadas(invocables));

        // Deidades.compatibleWith: razas y clases mezcladas.
        List<Opcion> razasYClases = new ArrayList<>();
        razas.listarTodas().forEach(r -> razasYClases.add(new Opcion(r.getId(), "Raza: " + r.getName())));
        clases.listarTodas().forEach(c -> razasYClases.add(new Opcion(c.getId(), "Clase: " + c.getName())));
        model.addAttribute("opcionesRazasYClases", ordenadas(razasYClases));

        // Eventos.continuesTo: otro evento o una historia.
        List<Opcion> continuaciones = new ArrayList<>();
        eventos.listarTodos().forEach(e -> continuaciones.add(new Opcion(e.getId(), "Evento: " + e.getName())));
        historias.listarTodas().forEach(h -> continuaciones.add(new Opcion(h.getId(), "Historia: " + h.getTitle())));
        model.addAttribute("opcionesContinuaciones", ordenadas(continuaciones));

        // ── Vocabularios: valores que YA existen en los datos (no inventamos nada) ──
        TreeSet<String> roles = new TreeSet<>(String.CASE_INSENSITIVE_ORDER);
        TreeSet<String> mecanicas = new TreeSet<>(String.CASE_INSENSITIVE_ORDER);
        TreeSet<String> escuelas = new TreeSet<>(String.CASE_INSENSITIVE_ORDER);
        TreeSet<String> escalados = new TreeSet<>(String.CASE_INSENSITIVE_ORDER);
        TreeSet<String> tagsEquipo = new TreeSet<>(String.CASE_INSENSITIVE_ORDER);

        // Para requiredTags/incompatibleTags/synergyTags: cualquier tag que YA se use en
        // los datos, más los ids de condición (anclado, enfurecido...) y los tags que
        // otorgan las dotes, porque los requisitos mezclan las tres cosas.
        TreeSet<String> tagsTodos = new TreeSet<>(String.CASE_INSENSITIVE_ORDER);

        habilidades.listarTodas().forEach(h -> {
            if (h.getRoleTags() != null) roles.addAll(h.getRoleTags());
            if (h.getMechanicTags() != null) mecanicas.addAll(h.getMechanicTags());
            if (h.getRequiredTags() != null) tagsTodos.addAll(h.getRequiredTags());
            if (h.getIncompatibleTags() != null) tagsTodos.addAll(h.getIncompatibleTags());
            if (h.getEffect() != null && esTexto(h.getEffect().getScaling())) escalados.add(h.getEffect().getScaling());
        });
        hechizos.listarTodas().forEach(h -> {
            if (h.getMechanicTags() != null) mecanicas.addAll(h.getMechanicTags());
            if (h.getRequiredTags() != null) tagsTodos.addAll(h.getRequiredTags());
            if (h.getIncompatibleTags() != null) tagsTodos.addAll(h.getIncompatibleTags());
            if (esTexto(h.getSchool())) escuelas.add(h.getSchool());
        });
        equipo.listarTodas().forEach(e -> {
            if (e.getGrantedTags() != null) tagsEquipo.addAll(e.getGrantedTags());
        });
        dotes.listarTodas().forEach(d -> {
            if (d.getGrantedTags() != null) tagsTodos.addAll(d.getGrantedTags());
        });
        pasivas.listarTodas().forEach(p -> {
            if (p.getEffect() != null && esTexto(p.getEffect().getScaling())) escalados.add(p.getEffect().getScaling());
        });
        condiciones.listarTodas().forEach(c -> tagsTodos.add(c.getId()));
        tagsTodos.addAll(tagsEquipo);
        tagsTodos.addAll(mecanicas);
        escalados.add("none");
        escalados.add("con_tier");
        escalados.add("CON");
        escalados.add("DES");
        escalados.add("INT");
        escalados.add("CAR");

        model.addAttribute("opcionesRoles", comoOpciones(roles));
        model.addAttribute("opcionesMecanicas", comoOpciones(mecanicas));
        model.addAttribute("opcionesEscuelas", comoOpciones(escuelas));
        model.addAttribute("opcionesEscalados", comoOpciones(escalados));
        model.addAttribute("opcionesTagsEquipo", comoOpciones(tagsEquipo));
        model.addAttribute("opcionesTagsTodos", comoOpciones(tagsTodos));
    }

    private static List<Opcion> ordenadas(List<Opcion> lista) {
        List<Opcion> copia = new ArrayList<>(lista);
        copia.sort(POR_ETIQUETA);
        return copia;
    }

    private static List<Opcion> comoOpciones(TreeSet<String> valores) {
        return valores.stream().filter(v -> !v.isBlank()).map(v -> new Opcion(v, v)).toList();
    }

    private static boolean esTexto(String v) {
        return v != null && !v.isBlank();
    }
}
