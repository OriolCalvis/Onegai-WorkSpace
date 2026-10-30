package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

/**
 * Tests de {@link TipusAccio#desDeActionType(String)}: mapeig del camp lliure
 * {@code actionType} de les cartes als vuit valors de la regla 7.3 del GDD (D4 del
 * backlog WS-D).
 */
class TipusAccioTest {

    @Test
    void mapaTotsElsValorsReconeguts() {
        assertEquals(TipusAccio.ACCION, TipusAccio.desDeActionType("accion"));
        assertEquals(TipusAccio.ACCION_MENOR, TipusAccio.desDeActionType("accion_menor"));
        assertEquals(TipusAccio.REACCION, TipusAccio.desDeActionType("reaccion"));
        assertEquals(TipusAccio.PASIVA, TipusAccio.desDeActionType("pasiva"));
        assertEquals(TipusAccio.MOVIMIENTO, TipusAccio.desDeActionType("movimiento"));
        assertEquals(TipusAccio.PREPARACION, TipusAccio.desDeActionType("preparacion"));
        assertEquals(TipusAccio.CANALIZACION, TipusAccio.desDeActionType("canalizacion"));
        assertEquals(TipusAccio.CONSUMIBLE, TipusAccio.desDeActionType("consumible"));
    }

    @Test
    void nullOBuitDonaAccioPrincipalPerDefecte() {
        assertEquals(TipusAccio.ACCION, TipusAccio.desDeActionType(null));
        assertEquals(TipusAccio.ACCION, TipusAccio.desDeActionType(""));
        assertEquals(TipusAccio.ACCION, TipusAccio.desDeActionType("   "));
    }

    @Test
    void ignoraMajusculesIEspaisSobrants() {
        assertEquals(TipusAccio.REACCION, TipusAccio.desDeActionType(" Reaccion "));
        assertEquals(TipusAccio.CANALIZACION, TipusAccio.desDeActionType("CANALIZACION"));
    }

    @Test
    void valorDesconegutLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> TipusAccio.desDeActionType("volar"));
    }
}
