package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests d'{@link EstatTorn}: recursos d'acció d'un torn segons la regla 7.3 del GDD
 * (D4 del backlog WS-D).
 */
class EstatTornTest {

    @Test
    void alComencarTotEstaDisponibleINoEsCanalitza() {
        EstatTorn torn = new EstatTorn();
        assertTrue(torn.isAccioPrincipalDisponible());
        assertTrue(torn.isAccioMovimentDisponible());
        assertTrue(torn.isReaccioDisponible());
        assertFalse(torn.isCanalitzant());
    }

    @Test
    void consumirAccioPrincipalLaDeixaIndisponibleSenseTocarLaResta() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.ACCION);
        assertFalse(torn.isAccioPrincipalDisponible());
        assertTrue(torn.isAccioMovimentDisponible());
        assertTrue(torn.isReaccioDisponible());
    }

    @Test
    void consumirAccioPrincipalDuesVegadesLlencaExcepcio() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.ACCION);
        assertThrows(IllegalStateException.class, () -> torn.consumir(TipusAccio.ACCION));
    }

    @Test
    void consumirMovimentDuesVegadesLlencaExcepcio() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.MOVIMIENTO);
        assertThrows(IllegalStateException.class, () -> torn.consumir(TipusAccio.MOVIMIENTO));
    }

    @Test
    void consumirReaccioDuesVegadesLlencaExcepcio() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.REACCION);
        assertThrows(IllegalStateException.class, () -> torn.consumir(TipusAccio.REACCION));
    }

    @Test
    void tipusSenseRecursLimitatEsPotConsumirRepetidamentSenseError() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.ACCION_MENOR);
        torn.consumir(TipusAccio.PASIVA);
        torn.consumir(TipusAccio.PREPARACION);
        torn.consumir(TipusAccio.CONSUMIBLE);
        torn.consumir(TipusAccio.ACCION_MENOR);
        assertTrue(torn.isAccioPrincipalDisponible());
        assertTrue(torn.isAccioMovimentDisponible());
        assertTrue(torn.isReaccioDisponible());
    }

    @Test
    void canalitzarGastaAlhoraAccioPrincipalIMovimentIActivaElFlag() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.CANALIZACION);
        assertFalse(torn.isAccioPrincipalDisponible());
        assertFalse(torn.isAccioMovimentDisponible());
        assertTrue(torn.isCanalitzant());
        assertTrue(torn.isReaccioDisponible(), "canalitzar no toca la reacció");
    }

    @Test
    void canalitzarSenseAccioPrincipalDisponibleLlencaExcepcio() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.ACCION);
        assertThrows(IllegalStateException.class, () -> torn.consumir(TipusAccio.CANALIZACION));
    }

    @Test
    void canalitzarSenseAccioMovimentDisponibleLlencaExcepcio() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.MOVIMIENTO);
        assertThrows(IllegalStateException.class, () -> torn.consumir(TipusAccio.CANALIZACION));
    }

    @Test
    void interrompreCanalitzacioApagaElFlagPeroNoRestauraAccions() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.CANALIZACION);
        torn.interrompreCanalitzacio();
        assertFalse(torn.isCanalitzant());
        assertFalse(torn.isAccioPrincipalDisponible());
        assertFalse(torn.isAccioMovimentDisponible());
    }

    @Test
    void iniciarNouTornRecuperaAccioPrincipalMovimentIReaccio() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.ACCION);
        torn.consumir(TipusAccio.MOVIMIENTO);
        torn.consumir(TipusAccio.REACCION);
        torn.iniciarNouTorn();
        assertTrue(torn.isAccioPrincipalDisponible());
        assertTrue(torn.isAccioMovimentDisponible());
        assertTrue(torn.isReaccioDisponible());
    }

    @Test
    void iniciarNouTornNoCancelALaCanalitzacioActiva() {
        EstatTorn torn = new EstatTorn();
        torn.consumir(TipusAccio.CANALIZACION);
        torn.iniciarNouTorn();
        assertTrue(torn.isCanalitzant(), "la concentració es manté fins que s'interromp explícitament");
        // Però l'acció principal i la de moviment del nou torn tornen a estar lliures:
        // canalitzar només va gastar les del torn en que es va iniciar.
        assertTrue(torn.isAccioPrincipalDisponible());
        assertTrue(torn.isAccioMovimentDisponible());
    }

    @Test
    void consumirNullLlencaExcepcio() {
        EstatTorn torn = new EstatTorn();
        assertThrows(IllegalArgumentException.class, () -> torn.consumir(null));
    }
}
