package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests de {@link CargaDelDesti}: clàusula de supervivència d'ús únic (regla 7.5.4 del
 * GDD, D7 del backlog WS-D).
 */
class CargaDelDestiTest {

    @Test
    void disponiblePerDefecte() {
        assertTrue(new CargaDelDesti().disponible());
    }

    @Test
    void usarLaMarcaComNoDisponible() {
        CargaDelDesti carga = new CargaDelDesti();
        carga.usar();
        assertFalse(carga.disponible());
    }

    @Test
    void usarDuesVegadesLlencaExcepcio() {
        CargaDelDesti carga = new CargaDelDesti();
        carga.usar();
        assertThrows(IllegalStateException.class, carga::usar);
    }
}
