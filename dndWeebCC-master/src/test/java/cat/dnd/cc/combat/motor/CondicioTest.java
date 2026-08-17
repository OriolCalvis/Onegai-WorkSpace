package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests de {@link Condicio}: catàleg de les 10 condicions de la regla 7.4 del GDD
 * (D6 del backlog WS-D).
 */
class CondicioTest {

    @Test
    void hiHaExactamentDeuCondicions() {
        assertEquals(10, Condicio.values().length);
    }

    @Test
    void capDescripcioEsBuidaOnNulla() {
        for (Condicio c : Condicio.values()) {
            assertTrue(c.descripcio() != null && !c.descripcio().isBlank(),
                    c.name() + " ha de tenir descripció");
        }
    }

    @Test
    void nomesFatigaEsGraduable() {
        for (Condicio c : Condicio.values()) {
            if (c == Condicio.FATIGA) {
                assertTrue(c.esGraduable());
            } else {
                assertFalse(c.esGraduable(), c.name() + " no hauria de ser graduable");
            }
        }
    }
}
