package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests de {@link NivellFatiga}: seguiment per graus de la condició Fatiga (regla 7.4/7.5
 * del GDD, D6 del backlog WS-D).
 */
class NivellFatigaTest {

    @Test
    void comencaANivell0INoEsMortal() {
        NivellFatiga fatiga = new NivellFatiga();
        assertEquals(0, fatiga.getNivell());
        assertFalse(fatiga.esMortal());
        assertEquals(0, fatiga.dausARestar());
    }

    @Test
    void afegirNivellsSumaAlNivellActual() {
        NivellFatiga fatiga = new NivellFatiga();
        fatiga.afegirNivells(2);
        assertEquals(2, fatiga.getNivell());
        fatiga.afegirNivells(1);
        assertEquals(3, fatiga.getNivell());
        assertEquals(3, fatiga.dausARestar());
    }

    @Test
    void noSuperaElNivell6ToTiQueSAfegeixiMoltMes() {
        NivellFatiga fatiga = new NivellFatiga();
        fatiga.afegirNivells(20);
        assertEquals(NivellFatiga.NIVELL_MORTAL, fatiga.getNivell());
    }

    @Test
    void arribarANivell6EsMortal() {
        NivellFatiga fatiga = new NivellFatiga();
        fatiga.afegirNivells(6);
        assertTrue(fatiga.esMortal());
    }

    @Test
    void afegirNivellsNegatiuLlencaExcepcio() {
        NivellFatiga fatiga = new NivellFatiga();
        assertThrows(IllegalArgumentException.class, () -> fatiga.afegirNivells(-1));
    }

    @Test
    void descansarLlargRedueixDosNivells() {
        NivellFatiga fatiga = new NivellFatiga();
        fatiga.afegirNivells(5);
        fatiga.descansarLlarg();
        assertEquals(3, fatiga.getNivell());
    }

    @Test
    void descansarLlargNoBaixaPerSotaDeZero() {
        NivellFatiga fatiga = new NivellFatiga();
        fatiga.afegirNivells(1);
        fatiga.descansarLlarg();
        assertEquals(0, fatiga.getNivell());
    }

    @Test
    void unDescansLlargPotDeixarDeSerMortal() {
        NivellFatiga fatiga = new NivellFatiga();
        fatiga.afegirNivells(6);
        assertTrue(fatiga.esMortal());
        fatiga.descansarLlarg();
        assertEquals(4, fatiga.getNivell());
        assertFalse(fatiga.esMortal());
    }
}
