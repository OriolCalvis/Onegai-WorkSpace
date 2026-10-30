package cat.dnd.cc.combat;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests de {@link MesaCombatService}: manté com a molt una {@link SessioCombat} activa
 * (D10 del backlog WS-D).
 */
class MesaCombatServiceTest {

    private static SessioCombat.Participant p(String id, String nom) {
        return new SessioCombat.Participant(id, nom, true, 10, 4);
    }

    @Test
    void senseCombatIniciatActualEsBuit() {
        MesaCombatService service = new MesaCombatService();
        assertTrue(service.actual().isEmpty());
    }

    @Test
    void iniciarDeixaLaSessioComActual() {
        MesaCombatService service = new MesaCombatService();
        service.iniciar("Emboscada", List.of(p("pj-1", "Heroi")));
        assertTrue(service.actual().isPresent());
        assertEquals("Emboscada", service.actual().get().nom());
    }

    @Test
    void iniciarUnCombatNouSubstitueixLAnterior() {
        MesaCombatService service = new MesaCombatService();
        service.iniciar("Primer", List.of(p("pj-1", "Heroi")));
        service.iniciar("Segon", List.of(p("pj-2", "Mag")));
        assertEquals("Segon", service.actual().orElseThrow().nom());
    }

    @Test
    void finalitzarDeixaActualBuit() {
        MesaCombatService service = new MesaCombatService();
        service.iniciar("Emboscada", List.of(p("pj-1", "Heroi")));
        service.finalitzar();
        assertFalse(service.actual().isPresent());
    }

    @Test
    void finalitzarSenseCombatActiuNoPeta() {
        MesaCombatService service = new MesaCombatService();
        service.finalitzar();
        assertTrue(service.actual().isEmpty());
    }
}
