package cat.dnd.cc.service;

import cat.dnd.cc.eines.CalculadoraVida;
import cat.dnd.cc.eines.FichaPersonatge;
import cat.dnd.cc.model.Personatge;
import cat.dnd.cc.model.TierBackground;
import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.model.TierFeat;
import cat.dnd.cc.model.TierRace;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.repository.PersonatgeRepository;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Un personaje se construye SIEMPRE combinando cartas ya existentes en el catálogo
 * (/cartas/clases, /cartas/razas, /cartas/transfondos, /cartas/habilidades, /cartas/armas,
 * /cartas/dotes). Este servicio resuelve esos ids contra los catálogos y calcula la ficha final
 * (stats resultantes y vida) siguiendo docs/Sistema_Cartas_Tiers.md, sección 5.
 */
@Service
public class PersonatgeService {
    private static final Logger log = LoggerFactory.getLogger(PersonatgeService.class);

    private final PersonatgeRepository personatgeRepository;
    private final TierClassService tierClassService;
    private final TierRaceService tierRaceService;
    private final TierBackgroundService tierBackgroundService;
    private final TierSkillService tierSkillService;
    private final TierEquipmentService tierEquipmentService;
    private final TierFeatService tierFeatService;
    private final TierSpellService tierSpellService;

    public PersonatgeService(PersonatgeRepository personatgeRepository, TierClassService tierClassService,
                              TierRaceService tierRaceService, TierBackgroundService tierBackgroundService,
                              TierSkillService tierSkillService, TierEquipmentService tierEquipmentService,
                              TierFeatService tierFeatService, TierSpellService tierSpellService) {
        this.personatgeRepository = personatgeRepository;
        this.tierClassService = tierClassService;
        this.tierRaceService = tierRaceService;
        this.tierBackgroundService = tierBackgroundService;
        this.tierSkillService = tierSkillService;
        this.tierEquipmentService = tierEquipmentService;
        this.tierFeatService = tierFeatService;
        this.tierSpellService = tierSpellService;
    }

    public long count() {
        return personatgeRepository.count();
    }

    public List<Personatge> llistarTots() {
        return personatgeRepository.findAll();
    }

    public Personatge obtenirPerId(Long id) {
        return personatgeRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Personatge no trobat: " + id));
    }

    public Personatge guardar(Personatge personatge) {
        return personatgeRepository.save(personatge);
    }

    public void eliminar(Long id) {
        personatgeRepository.deleteById(id);
    }

    // Listas para los desplegables/checkboxes del formulario: SIEMPRE cartas ya existentes,
    // nunca texto libre ni la posibilidad de crear una clase/raza/trasfondo nueva desde aquí.

    public List<TierClass> clasesDisponibles() {
        return tierClassService.listarTodas();
    }

    public List<TierRace> razasDisponibles() {
        return tierRaceService.listarTodas();
    }

    public List<TierBackground> transfondosDisponibles() {
        return tierBackgroundService.listarTodas();
    }

    public List<TierSkill> habilidadesDisponibles() {
        return tierSkillService.listarTodas();
    }

    public List<TierEquipment> equipoDisponible() {
        return tierEquipmentService.listarTodas();
    }

    public List<TierFeat> dotesDisponibles() {
        return tierFeatService.listarTodas();
    }

    /** Habilidades divinas: hechizos con castingStat CAR (magia divina, GDD 9.6/10.10). */
    public List<cat.dnd.cc.model.TierSpell> divinasDisponibles() {
        return tierSpellService.listarTodas().stream()
                .filter(s -> "CAR".equalsIgnoreCase(s.getCastingStat()))
                .toList();
    }

    /**
     * Resuelve un personaje contra el catálogo y calcula su ficha final: stats con todos los
     * bonos/penalizaciones de raza, trasfondo y equipo aplicados, y la vida resultante.
     * Si el personaje aún no tiene clase asignada no se puede calcular vida (devuelve 0).
     */
    public FichaPersonatge calcularFicha(Personatge personatge) {
        TierClass clase = buscarOpcional(personatge.getClaseId(), tierClassService::obtenerPorId);
        TierRace raza = buscarOpcional(personatge.getRazaId(), tierRaceService::obtenerPorId);
        TierBackground transfons = buscarOpcional(personatge.getTransfonsId(), tierBackgroundService::obtenerPorId);

        List<TierSkill> habilidades = new ArrayList<>();
        for (String id : personatge.getHabilidadIds()) {
            TierSkill skill = buscarOpcional(id, tierSkillService::obtenerPorId);
            if (skill != null) {
                habilidades.add(skill);
            }
        }

        List<TierEquipment> equipo = new ArrayList<>();
        for (String id : personatge.getEquipoIds()) {
            TierEquipment item = buscarOpcional(id, tierEquipmentService::obtenerPorId);
            if (item != null) {
                equipo.add(item);
            }
        }

        List<TierFeat> dotes = new ArrayList<>();
        for (String id : personatge.getDoteIds()) {
            TierFeat dote = buscarOpcional(id, tierFeatService::obtenerPorId);
            if (dote != null) {
                dotes.add(dote);
            }
        }

        int bonoEquipoCon = 0;
        int bonoEquipoDes = 0;
        int bonoEquipoCar = 0;
        int bonoEquipoInt = 0;
        int bonoEquipoVida = 0;
        int bonoEquipoArmadura = 0;
        for (TierEquipment item : equipo) {
            bonoEquipoCon += item.getBonusCon() - item.getPenaltyCon();
            bonoEquipoDes += item.getBonusDes() - item.getPenaltyDes();
            bonoEquipoCar += item.getBonusCar() - item.getPenaltyCar();
            bonoEquipoInt += item.getBonusInt() - item.getPenaltyInt();
            bonoEquipoVida += item.getBonusHealth();
            bonoEquipoArmadura += item.getBonusArmor();
        }

        int razaCon = raza != null ? raza.getStatBonusCon() : 0;
        int razaDes = raza != null ? raza.getStatBonusDes() : 0;
        int razaCar = raza != null ? raza.getStatBonusCar() : 0;
        int razaInt = raza != null ? raza.getStatBonusInt() : 0;

        int transfonsCon = transfons != null ? transfons.getStatBonusCon() : 0;
        int transfonsDes = transfons != null ? transfons.getStatBonusDes() : 0;
        int transfonsCar = transfons != null ? transfons.getStatBonusCar() : 0;
        int transfonsInt = transfons != null ? transfons.getStatBonusInt() : 0;

        int finalCon = personatge.getStatCon() + razaCon + transfonsCon + bonoEquipoCon;
        int finalDes = personatge.getStatDes() + razaDes + transfonsDes + bonoEquipoDes;
        int finalCar = personatge.getStatCar() + razaCar + transfonsCar + bonoEquipoCar;
        int finalInt = personatge.getStatInt() + razaInt + transfonsInt + bonoEquipoInt;

        double vida = 0;
        if (clase != null) {
            vida = CalculadoraVida.calcular(clase.getBaseHealth(), clase.getHealthScalingCon(), finalCon, personatge.getTier())
                    + bonoEquipoVida;
            // Desglose auditable del cálculo: si un número sale raro, aquí está el porqué.
            log.debug("VIDA de '{}': base {} ({}) + CON final {}×{} + bono tier {} (t{}) + equipo {} = {}",
                    personatge.getNom(), clase.getBaseHealth(), clase.getName(),
                    finalCon, clase.getHealthScalingCon(),
                    vida - clase.getBaseHealth() - clase.getHealthScalingCon() * finalCon - bonoEquipoVida,
                    personatge.getTier(), bonoEquipoVida, vida);
        } else {
            log.debug("VIDA de '{}': sin clase asignada — vida 0 (esperado, no es un error)",
                    personatge.getNom());
        }

        // Defensas derivadas (Manual Sagrado, sección 5):
        // CA = 10 + DES + bono de armadura equipada; Defensa mental = 10 + CAR;
        // Resistencia física = 10 + CON; Iniciativa = DES final (ya incluye bonos de equipo/raza/trasfondo).
        int claseArmadura = 10 + finalDes + bonoEquipoArmadura;
        int defensaMental = 10 + finalCar;
        int resistenciaFisica = 10 + finalCon;
        int iniciativa = finalDes;

        return new FichaPersonatge(clase, raza, transfons, habilidades, equipo, dotes,
                finalCon, finalDes, finalCar, finalInt, vida,
                claseArmadura, defensaMental, resistenciaFisica, iniciativa);
    }

    /**
     * Un personaje solo puede llevar un objeto por slot (sección 9.7/19.2 del GDD: 6 slots fijos,
     * un objeto por slot — nada de dos cascos a la vez). Devuelve un mensaje de error legible por
     * cada slot en conflicto, o una lista vacía si el equipo elegido es válido.
     */
    public List<String> validarSlotsEquipo(List<String> equipoIds) {
        Map<String, List<String>> nombresPorSlot = new LinkedHashMap<>();
        for (String id : equipoIds) {
            TierEquipment item = buscarOpcional(id, tierEquipmentService::obtenerPorId);
            if (item == null || item.getSlot() == null || item.getSlot().isBlank()) {
                continue;
            }
            nombresPorSlot.computeIfAbsent(item.getSlot(), s -> new ArrayList<>()).add(item.getName());
        }
        List<String> errores = new ArrayList<>();
        for (Map.Entry<String, List<String>> entrada : nombresPorSlot.entrySet()) {
            if (entrada.getValue().size() > 1) {
                errores.add("Solo puedes llevar un objeto en el slot \"" + entrada.getKey() + "\", "
                        + "pero has elegido varios: " + String.join(", ", entrada.getValue()) + ".");
            }
        }
        return errores;
    }

    private static final List<String> SLOTS_ARMADURA = List.of("cabeza", "torso", "piernas", "pies");
    private static final List<String> SLOTS_ARMA = List.of("arma_principal", "arma_secundaria");
    private static final Map<String, Integer> ORDEN_PESO = Map.of("ligera", 1, "media", 2, "pesada", 3);
    private static final int[] LIMITES_MANO_POR_TIER = {6, 10, 15, 20, 24, 28};

    /**
     * Un objeto de armadura/arma más pesado que lo que la clase soporta no está prohibido por
     * reglas de slot, pero sí es incompatible: la clase declara maxArmorWeight/maxWeaponWeight
     * (sección 9.1 del GDD) y aquí se rechaza el equipo que se pase de esa categoría, para que un
     * personaje no pueda equiparse una carta de equipo que su clase no sabría usar.
     */
    public List<String> validarCompatibilidadEquipo(String claseId, List<String> equipoIds) {
        List<String> errores = new ArrayList<>();
        TierClass clase = buscarOpcional(claseId, tierClassService::obtenerPorId);
        if (clase == null) {
            return errores;
        }
        int maxArmadura = ORDEN_PESO.getOrDefault(clase.getMaxArmorWeight(), 3);
        int maxArma = ORDEN_PESO.getOrDefault(clase.getMaxWeaponWeight(), 3);

        for (String id : equipoIds) {
            TierEquipment item = buscarOpcional(id, tierEquipmentService::obtenerPorId);
            if (item == null || item.getSlot() == null) {
                continue;
            }
            int pesoItem = ORDEN_PESO.getOrDefault(item.getWeightCategory(), 1);
            if (SLOTS_ARMADURA.contains(item.getSlot()) && pesoItem > maxArmadura) {
                errores.add("\"" + item.getName() + "\" es armadura " + item.getWeightCategory()
                        + ", pero " + clase.getName() + " solo puede llevar armadura hasta \"" + clase.getMaxArmorWeight() + "\".");
            }
            if (SLOTS_ARMA.contains(item.getSlot()) && pesoItem > maxArma) {
                errores.add("\"" + item.getName() + "\" es un arma " + item.getWeightCategory()
                        + ", pero " + clase.getName() + " solo puede usar armas hasta \"" + clase.getMaxWeaponWeight() + "\".");
            }
        }
        return errores;
    }

    /**
     * Límite de mano por tier (sección 3/11.1 del GDD): un personaje no puede llevar más cartas
     * de habilidad preparadas que las que permite su tier (el modelo todavía no soporta multiclase,
     * así que siempre se usa la columna "solo una clase" de la tabla).
     */
    public List<String> validarLimiteMano(int tier, List<String> habilidadIds) {
        List<String> errores = new ArrayList<>();
        int tierAcotado = Math.max(0, Math.min(tier, LIMITES_MANO_POR_TIER.length - 1));
        int limite = LIMITES_MANO_POR_TIER[tierAcotado];
        long seleccionadas = habilidadIds.stream().filter(id -> id != null && !id.isBlank()).count();
        if (seleccionadas > limite) {
            errores.add("Has elegido " + seleccionadas + " cartas de habilidad, pero el límite de mano en tier "
                    + tierAcotado + " es " + limite + ".");
        }
        return errores;
    }

    /** Límite de dotes por tier: 1 en tiers 0-1, 2 en 2-3, 3 en 4-5 (progresión de la sección 11). */
    private static final int[] LIMITE_DOTES_POR_TIER = {1, 1, 2, 2, 3, 3};

    /**
     * Nunca se juega una carta por debajo de su tier mínimo (GDD 10.5): un personaje de tier 1
     * no puede equipar objetos ni preparar habilidades/divinas de tier 2+.
     */
    public List<String> validarTierCartas(int tier, List<String> habilidadIds, List<String> equipoIds,
                                          List<String> hechizoIds) {
        List<String> errores = new ArrayList<>();
        for (String id : habilidadIds) {
            TierSkill s = buscarOpcional(id, tierSkillService::obtenerPorId);
            if (s != null && s.getTier() > tier) {
                errores.add("La habilidad \"" + s.getName() + "\" es de tier " + s.getTier()
                        + " y tu personaje es tier " + tier + " (GDD 10.5).");
            }
        }
        for (String id : equipoIds) {
            TierEquipment e = buscarOpcional(id, tierEquipmentService::obtenerPorId);
            if (e != null && e.getTier() > tier) {
                errores.add("El objeto \"" + e.getName() + "\" es de tier " + e.getTier()
                        + " y tu personaje es tier " + tier + " (GDD 10.5).");
            }
        }
        for (String id : hechizoIds) {
            cat.dnd.cc.model.TierSpell h = buscarOpcional(id, tierSpellService::obtenerPorId);
            if (h != null && h.getTier() > tier) {
                errores.add("La habilidad divina \"" + h.getName() + "\" es de tier " + h.getTier()
                        + " y tu personaje es tier " + tier + " (GDD 10.5).");
            }
        }
        return errores;
    }

    /** Límite de dotes según tier (tabla {1,1,2,2,3,3}). */
    public List<String> validarLimiteDotes(int tier, List<String> doteIds) {
        List<String> errores = new ArrayList<>();
        int tierAcotado = Math.max(0, Math.min(tier, LIMITE_DOTES_POR_TIER.length - 1));
        int limite = LIMITE_DOTES_POR_TIER[tierAcotado];
        long elegidas = doteIds.stream().filter(id -> id != null && !id.isBlank()).count();
        if (elegidas > limite) {
            errores.add("Has elegido " + elegidas + " dotes, pero el límite en tier " + tierAcotado
                    + " es " + limite + ".");
        }
        return errores;
    }

    /** Máximo 3 habilidades divinas por personaje (GDD 10.10). */
    public List<String> validarLimiteDivinas(List<String> hechizoIds) {
        List<String> errores = new ArrayList<>();
        long divinas = hechizoIds.stream()
                .map(id -> buscarOpcional(id, tierSpellService::obtenerPorId))
                .filter(s -> s != null && "CAR".equalsIgnoreCase(s.getCastingStat()))
                .count();
        if (divinas > 3) {
            errores.add("Has elegido " + divinas + " habilidades divinas; el máximo por personaje es 3 (GDD 10.10).");
        }
        return errores;
    }

    private <T> T buscarOpcional(String id, java.util.function.Function<String, T> buscador) {
        if (id == null || id.isBlank()) {
            return null;
        }
        try {
            return buscador.apply(id);
        } catch (IllegalArgumentException e) {
            return null;
        }
    }
}
