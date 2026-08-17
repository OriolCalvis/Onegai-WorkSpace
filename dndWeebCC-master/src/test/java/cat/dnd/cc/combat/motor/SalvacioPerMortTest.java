package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import java.util.Random;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests de {@link SalvacioPerMort}: salvació per mort i mort instantània (regla 7.5 del
 * GDD, D7 del backlog WS-D).
 */
class SalvacioPerMortTest {

    @Test
    void exitEstabilitzaA1DeVidaIndependentmentDeLaVidaAnterior() {
        SalvacioPerMort.ResultatSalvacio r = SalvacioPerMort.tirar(1, -3, rngCara(6));
        assertEquals(SalvacioPerMort.Resultat.ESTABILITZAT, r.resultat());
        assertEquals(SalvacioPerMort.VIDA_ESTABILITZAT, r.vidaResultant());
    }

    @Test
    void fracasBaixaUnPuntDeVida() {
        // 1d6 mostrant un 5 → 0,5 èxits: fracàs (no crític).
        SalvacioPerMort.ResultatSalvacio r = SalvacioPerMort.tirar(1, 0, rngCara(5));
        assertEquals(SalvacioPerMort.Resultat.EMPITJORA, r.resultat());
        assertEquals(-1, r.vidaResultant());
    }

    @Test
    void fracasCriticBaixaDosPuntsDeVida() {
        // 1d6 mostrant un 2 → 0 èxits: fracàs crític.
        SalvacioPerMort.ResultatSalvacio r = SalvacioPerMort.tirar(1, -2, rngCara(2));
        assertEquals(SalvacioPerMort.Resultat.EMPITJORA_GREU, r.resultat());
        assertEquals(-4, r.vidaResultant());
    }

    @Test
    void zeroConSempreFracasaCriticament() {
        // Sense daus (CON 0) no hi ha manera d'obtenir cap èxit.
        SalvacioPerMort.ResultatSalvacio r = SalvacioPerMort.tirar(0, -1, new Random(0));
        assertEquals(SalvacioPerMort.Resultat.EMPITJORA_GREU, r.resultat());
        assertEquals(-3, r.vidaResultant());
    }

    @Test
    void mortInstantaniaPerSotaDeMenysCon() {
        assertTrue(SalvacioPerMort.esMortInstantania(-6, 5), "-6 < -CON(5)");
        assertFalse(SalvacioPerMort.esMortInstantania(-5, 5), "exactament -CON no és 'per sota'");
    }

    @Test
    void mortInstantaniaAlLlindarAbsolutIndependentDelCon() {
        assertTrue(SalvacioPerMort.esMortInstantania(-10, 100));
        assertTrue(SalvacioPerMort.esMortInstantania(-11, 1));
    }

    @Test
    void vidaPositivaONoProuNegativaNoEsMortInstantania() {
        assertFalse(SalvacioPerMort.esMortInstantania(0, 5));
        assertFalse(SalvacioPerMort.esMortInstantania(-2, 5));
    }

    // --- Utilitats (mateix patró que TiradaD6Test) ---

    private static Random rngCara(int cara) {
        return new Random() {
            @Override
            public int nextInt(int bound) {
                return cara - 1;
            }
        };
    }
}
