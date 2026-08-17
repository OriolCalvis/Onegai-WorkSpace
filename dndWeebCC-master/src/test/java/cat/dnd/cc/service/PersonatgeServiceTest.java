package cat.dnd.cc.service;

import cat.dnd.cc.eines.FichaPersonatge;
import cat.dnd.cc.model.Personatge;
import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.model.TierRace;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.repository.PersonatgeRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.Mockito;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

/**
 * Tests unitarios (sin contexto Spring) de las validaciones de construcción de personaje
 * y del cálculo de ficha: slots de equipo, compatibilidad de peso, límite de mano y
 * agregación de stats/vida/defensas derivadas.
 */
class PersonatgeServiceTest {

    private TierClassService tierClassService;
    private TierRaceService tierRaceService;
    private TierBackgroundService tierBackgroundService;
    private TierSkillService tierSkillService;
    private TierEquipmentService tierEquipmentService;
    private TierFeatService tierFeatService;
    private TierSpellService tierSpellService;
    private PersonatgeService service;

    @BeforeEach
    void setUp() {
        PersonatgeRepository personatgeRepository = Mockito.mock(PersonatgeRepository.class);
        tierClassService = Mockito.mock(TierClassService.class);
        tierRaceService = Mockito.mock(TierRaceService.class);
        tierBackgroundService = Mockito.mock(TierBackgroundService.class);
        tierSkillService = Mockito.mock(TierSkillService.class);
        tierEquipmentService = Mockito.mock(TierEquipmentService.class);
        tierFeatService = Mockito.mock(TierFeatService.class);
        tierSpellService = Mockito.mock(TierSpellService.class);
        service = new PersonatgeService(personatgeRepository, tierClassService, tierRaceService,
                tierBackgroundService, tierSkillService, tierEquipmentService, tierFeatService, tierSpellService);

        // Por defecto, cualquier id desconocido no existe en el catálogo.
        when(tierClassService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierRaceService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierBackgroundService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierSkillService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierEquipmentService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierFeatService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierSpellService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
    }

    // ---- límites nuevos: tier de cartas, dotes y divinas ----

    @Test
    void unaCartaDeTierSuperiorAlPersonajeEsInvalida() {
        TierSkill fuerte = new TierSkill();
        fuerte.setId("golpe_supremo"); fuerte.setName("Golpe Supremo"); fuerte.setTier(3);
        Mockito.doReturn(fuerte).when(tierSkillService).obtenerPorId("golpe_supremo");

        List<String> errores = service.validarTierCartas(1, List.of("golpe_supremo"), List.of(), List.of());
        assertEquals(1, errores.size());
        assertTrue(errores.get(0).contains("tier 3"));
    }

    @Test
    void equipoDeTierIgualOInferiorEsValido() {
        TierEquipment casco = new TierEquipment();
        casco.setId("casco_t2"); casco.setName("Casco"); casco.setTier(2);
        Mockito.doReturn(casco).when(tierEquipmentService).obtenerPorId("casco_t2");
        assertTrue(service.validarTierCartas(2, List.of(), List.of("casco_t2"), List.of()).isEmpty());
    }

    @Test
    void limiteDeDotesPorTier() {
        // tier 1 → máximo 1 dote (tabla {1,1,2,2,3,3})
        assertEquals(1, service.validarLimiteDotes(1, List.of("d1", "d2")).size());
        assertTrue(service.validarLimiteDotes(1, List.of("d1")).isEmpty());
        assertTrue(service.validarLimiteDotes(4, List.of("d1", "d2", "d3")).isEmpty());
    }

    @Test
    void maximoTresHabilidadesDivinas() {
        for (int i = 1; i <= 4; i++) {
            cat.dnd.cc.model.TierSpell divina = new cat.dnd.cc.model.TierSpell();
            divina.setId("divina_" + i); divina.setName("Divina " + i); divina.setCastingStat("CAR");
            Mockito.doReturn(divina).when(tierSpellService).obtenerPorId("divina_" + i);
        }
        assertTrue(service.validarLimiteDivinas(List.of("divina_1", "divina_2", "divina_3")).isEmpty());
        assertEquals(1, service.validarLimiteDivinas(List.of("divina_1", "divina_2", "divina_3", "divina_4")).size());
    }

    private TierEquipment equipo(String id, String nombre, String slot, String peso) {
        TierEquipment item = new TierEquipment();
        item.setId(id);
        item.setName(nombre);
        item.setSlot(slot);
        item.setWeightCategory(peso);
        Mockito.doReturn(item).when(tierEquipmentService).obtenerPorId(id);
        return item;
    }

    // ---- validarSlotsEquipo ----

    @Test
    void dosObjetosEnElMismoSlotProducenError() {
        equipo("casco_a", "Casco de Hierro", "cabeza", "media");
        equipo("casco_b", "Capucha de Cuero", "cabeza", "ligera");

        List<String> errores = service.validarSlotsEquipo(List.of("casco_a", "casco_b"));

        assertEquals(1, errores.size());
        assertTrue(errores.get(0).contains("cabeza"));
    }

    @Test
    void unObjetoPorSlotEsValido() {
        equipo("casco_a", "Casco de Hierro", "cabeza", "media");
        equipo("peto_a", "Peto de Placas", "torso", "pesada");

        assertTrue(service.validarSlotsEquipo(List.of("casco_a", "peto_a")).isEmpty());
    }

    @Test
    void idsInexistentesSeIgnoranEnLaValidacionDeSlots() {
        assertTrue(service.validarSlotsEquipo(List.of("no_existe_1", "no_existe_2")).isEmpty());
    }

    // ---- validarCompatibilidadEquipo ----

    private TierClass clase(String id, String nombre, String maxArmadura, String maxArma) {
        TierClass clase = new TierClass();
        clase.setId(id);
        clase.setName(nombre);
        clase.setMaxArmorWeight(maxArmadura);
        clase.setMaxWeaponWeight(maxArma);
        Mockito.doReturn(clase).when(tierClassService).obtenerPorId(id);
        return clase;
    }

    @Test
    void armaduraMasPesadaQueElMaximoDeClaseEsIncompatible() {
        clase("arcanista", "Arcanista", "ligera", "ligera");
        equipo("peto_placas", "Peto de Placas", "torso", "pesada");

        List<String> errores = service.validarCompatibilidadEquipo("arcanista", List.of("peto_placas"));

        assertEquals(1, errores.size());
        assertTrue(errores.get(0).contains("Peto de Placas"));
    }

    @Test
    void armaMasPesadaQueElMaximoDeClaseEsIncompatible() {
        clase("arcanista", "Arcanista", "ligera", "ligera");
        equipo("mandoble", "Mandoble", "arma_principal", "pesada");

        assertEquals(1, service.validarCompatibilidadEquipo("arcanista", List.of("mandoble")).size());
    }

    @Test
    void equipoDentroDelLimiteDePesoEsCompatible() {
        clase("guardian", "Guardián de Hierro", "pesada", "pesada");
        equipo("peto_placas", "Peto de Placas", "torso", "pesada");
        equipo("mandoble", "Mandoble", "arma_principal", "pesada");

        assertTrue(service.validarCompatibilidadEquipo("guardian", List.of("peto_placas", "mandoble")).isEmpty());
    }

    @Test
    void sinClaseNoSePuedeValidarCompatibilidadYNoSeBloquea() {
        equipo("peto_placas", "Peto de Placas", "torso", "pesada");
        assertTrue(service.validarCompatibilidadEquipo(null, List.of("peto_placas")).isEmpty());
        assertTrue(service.validarCompatibilidadEquipo("clase_inexistente", List.of("peto_placas")).isEmpty());
    }

    // ---- validarLimiteMano ----

    @Test
    void superarElLimiteDeManoDelTierProduceError() {
        // Tier 1 → límite 10 (tabla {6, 10, 15, 20, 24, 28})
        List<String> once = List.of("h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9", "h10", "h11");
        assertEquals(1, service.validarLimiteMano(1, once).size());
    }

    @Test
    void elLimiteExactoDeManoEsValido() {
        List<String> seis = List.of("h1", "h2", "h3", "h4", "h5", "h6");
        assertTrue(service.validarLimiteMano(0, seis).isEmpty());
    }

    @Test
    void losIdsVaciosNoCuentanParaElLimiteDeMano() {
        java.util.ArrayList<String> ids = new java.util.ArrayList<>(List.of("h1", "h2", ""));
        ids.add(null);
        assertTrue(service.validarLimiteMano(0, ids).isEmpty());
    }

    @Test
    void elTierSeAcotaALaTablaDeLimites() {
        // Tier fuera de rango (99) se acota al último límite (28) sin lanzar excepción.
        List<String> pocas = List.of("h1", "h2");
        assertTrue(service.validarLimiteMano(99, pocas).isEmpty());
        assertTrue(service.validarLimiteMano(-3, pocas).isEmpty());
    }

    // ---- calcularFicha ----

    @Test
    void calcularFichaAgregaStatsVidaYDefensasDerivadas() {
        TierClass clase = clase("guardian", "Guardián de Hierro", "pesada", "pesada");
        clase.setBaseHealth(20);
        clase.setHealthScalingCon(3.0);

        TierRace raza = new TierRace();
        raza.setStatBonusCon(1);
        Mockito.doReturn(raza).when(tierRaceService).obtenerPorId("nan");

        TierEquipment peto = equipo("peto_placas", "Peto de Placas", "torso", "pesada");
        peto.setBonusCon(1);
        peto.setPenaltyDes(1);
        peto.setBonusHealth(2);
        peto.setBonusArmor(3);

        Personatge personatge = new Personatge();
        personatge.setClaseId("guardian");
        personatge.setRazaId("nan");
        personatge.setTier(2);
        personatge.setStatCon(4);
        personatge.setStatDes(2);
        personatge.setStatCar(1);
        personatge.setStatInt(1);
        personatge.setEquipoIds(List.of("peto_placas"));

        FichaPersonatge ficha = service.calcularFicha(personatge);

        // CON = 4 base + 1 raza + 1 equipo = 6; DES = 2 − 1 = 1
        assertEquals(6, ficha.finalCon());
        assertEquals(1, ficha.finalDes());
        assertEquals(1, ficha.finalCar());
        assertEquals(1, ficha.finalInt());
        // Vida = 20 + 6×3 + 4 (tier 2) + 2 (equipo) = 44
        assertEquals(44.0, ficha.vida());
        // CA = 10 + 1 DES + 3 armadura; DM = 10 + CAR; RF = 10 + CON; Iniciativa = DES
        assertEquals(14, ficha.claseArmadura());
        assertEquals(11, ficha.defensaMental());
        assertEquals(16, ficha.resistenciaFisica());
        assertEquals(1, ficha.iniciativa());
    }

    @Test
    void sinClaseLaVidaEsCero() {
        Personatge personatge = new Personatge();
        personatge.setStatCon(4);
        assertEquals(0.0, service.calcularFicha(personatge).vida());
    }
}
