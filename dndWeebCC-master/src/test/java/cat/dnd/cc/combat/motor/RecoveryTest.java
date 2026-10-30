package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

/**
 * Tests de {@link Recovery#desDeCamp(String)}: mapeig del camp {@code recovery} de les
 * cartes (D8 del backlog WS-D).
 */
class RecoveryTest {

    @Test
    void mapaTotsElsValorsReconeguts() {
        assertEquals(Recovery.ACTIVA, Recovery.desDeCamp("activa"));
        assertEquals(Recovery.DESCANSO_CORTO, Recovery.desDeCamp("descanso_corto"));
        assertEquals(Recovery.DESCANSO_LARGO, Recovery.desDeCamp("descanso_largo"));
        assertEquals(Recovery.NINGUNO, Recovery.desDeCamp("ninguno"));
    }

    @Test
    void nullOBuitDonaActivaPerDefecte() {
        assertEquals(Recovery.ACTIVA, Recovery.desDeCamp(null));
        assertEquals(Recovery.ACTIVA, Recovery.desDeCamp(""));
        assertEquals(Recovery.ACTIVA, Recovery.desDeCamp("  "));
    }

    @Test
    void ignoraMajusculesIEspaisSobrants() {
        assertEquals(Recovery.DESCANSO_CORTO, Recovery.desDeCamp(" Descanso_Corto "));
        assertEquals(Recovery.DESCANSO_LARGO, Recovery.desDeCamp("DESCANSO_LARGO"));
    }

    @Test
    void valorDesconegutLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> Recovery.desDeCamp("descanso_medio"));
    }
}
