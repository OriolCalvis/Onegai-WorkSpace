package cat.dnd.cc.combat;

import cat.dnd.cc.combat.motor.EstatTorn;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests de {@link SessioCombat}: ordre de torn per Iniciativa i estat de torn per
 * participant (D10 del backlog WS-D).
 */
class SessioCombatTest {

    private static SessioCombat.Participant p(String id, String nom, boolean esPersonatge, double iniciativa, int des) {
        return new SessioCombat.Participant(id, nom, esPersonatge, iniciativa, des);
    }

    @Test
    void ordreTornSegueixLaIniciativaDescendent() {
        SessioCombat combat = new SessioCombat("Emboscada", List.of(
                p("npc-1", "Bandit", false, 8, 3),
                p("pj-1", "Heroi", true, 12, 5),
                p("pj-2", "Mag", true, 10, 4)
        ));
        assertEquals(List.of("pj-1", "pj-2", "npc-1"), combat.ordreTorn());
    }

    @Test
    void elPrimerParticipantDeLOrdreEsQuiActuaAlComencar() {
        SessioCombat combat = new SessioCombat("Emboscada", List.of(
                p("npc-1", "Bandit", false, 8, 3),
                p("pj-1", "Heroi", true, 12, 5)
        ));
        assertEquals("pj-1", combat.participantActualId());
    }

    @Test
    void seguentTornAvancaIRecuperaElsRecursosDelNouActual() {
        SessioCombat combat = new SessioCombat("Emboscada", List.of(
                p("pj-1", "Heroi", true, 12, 5),
                p("npc-1", "Bandit", false, 8, 3)
        ));
        EstatTorn tornBandit = combat.estatTornDe("npc-1");
        tornBandit.consumir(cat.dnd.cc.combat.motor.TipusAccio.ACCION);
        assertFalse(tornBandit.isAccioPrincipalDisponible());

        combat.seguentTorn();

        assertEquals("npc-1", combat.participantActualId());
        assertTrue(tornBandit.isAccioPrincipalDisponible(), "el nou torn recupera els recursos");
    }

    @Test
    void seguentTornEsCiclicITornaAlPrimerDesdeLUltim() {
        SessioCombat combat = new SessioCombat("Duel", List.of(
                p("pj-1", "Heroi", true, 12, 5),
                p("pj-2", "Mag", true, 10, 4)
        ));
        combat.seguentTorn(); // pj-2
        combat.seguentTorn(); // torna a pj-1
        assertEquals("pj-1", combat.participantActualId());
    }

    @Test
    void participantPerIdRetornaElParticipantCorrecte() {
        SessioCombat combat = new SessioCombat("Duel", List.of(p("pj-1", "Heroi", true, 12, 5)));
        assertEquals("Heroi", combat.participantPerId("pj-1").nom());
    }

    @Test
    void participantPerIdDesconegutLlencaExcepcio() {
        SessioCombat combat = new SessioCombat("Duel", List.of(p("pj-1", "Heroi", true, 12, 5)));
        assertThrows(IllegalArgumentException.class, () -> combat.participantPerId("mai-existent"));
    }

    @Test
    void estatTornDeDesconegutLlencaExcepcio() {
        SessioCombat combat = new SessioCombat("Duel", List.of(p("pj-1", "Heroi", true, 12, 5)));
        assertThrows(IllegalArgumentException.class, () -> combat.estatTornDe("mai-existent"));
    }

    @Test
    void capParticipantLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> new SessioCombat("Buit", List.of()));
        assertThrows(IllegalArgumentException.class, () -> new SessioCombat("Buit", null));
    }

    @Test
    void nomBuitLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class,
                () -> new SessioCombat("", List.of(p("pj-1", "Heroi", true, 12, 5))));
    }

    @Test
    void midaRetornaElNombreDeParticipants() {
        SessioCombat combat = new SessioCombat("Duel", List.of(
                p("pj-1", "Heroi", true, 12, 5),
                p("pj-2", "Mag", true, 10, 4)
        ));
        assertEquals(2, combat.mida());
    }
}
