package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import java.util.List;
import java.util.Random;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests del motor de tirades (regla 7.1 + 7.2 del GDD). Són deterministes perquè
 * usen un {@link Random} de cara fixa o amb seed fixa, seguint el patró de
 * {@code CalculadoraVidaTest} (matemàtica pura, sense context Spring).
 */
class TiradaD6Test {

    @Test
    void taulaBaseNormalSolVal5IMigI6Sencer() {
        assertEquals(1.0, tirarAmbCaraFixa(6, Avantatge.NORMAL).exitos(), "6 → 1 èxit");
        assertEquals(0.5, tirarAmbCaraFixa(5, Avantatge.NORMAL).exitos(), "5 → 0,5 èxits");
        assertEquals(0.0, tirarAmbCaraFixa(4, Avantatge.NORMAL).exitos(), "4 → 0");
        assertEquals(0.0, tirarAmbCaraFixa(3, Avantatge.NORMAL).exitos(), "3 → 0");
        assertEquals(0.0, tirarAmbCaraFixa(1, Avantatge.NORMAL).exitos(), "1 → 0");
    }

    @Test
    void avantatgeFaQueElQuatreValguiMig() {
        assertEquals(1.0, tirarAmbCaraFixa(6, Avantatge.AVANTATGE).exitos());
        assertEquals(0.5, tirarAmbCaraFixa(5, Avantatge.AVANTATGE).exitos());
        assertEquals(0.5, tirarAmbCaraFixa(4, Avantatge.AVANTATGE).exitos(), "amb avantatge el 4 val 0,5");
        assertEquals(0.0, tirarAmbCaraFixa(3, Avantatge.AVANTATGE).exitos());
    }

    @Test
    void desavantatgeNomesContaElSis() {
        assertEquals(1.0, tirarAmbCaraFixa(6, Avantatge.DESAVANTATGE).exitos());
        assertEquals(0.0, tirarAmbCaraFixa(5, Avantatge.DESAVANTATGE).exitos(), "amb desavantatge el 5 no compta");
        assertEquals(0.0, tirarAmbCaraFixa(4, Avantatge.DESAVANTATGE).exitos());
    }

    @Test
    void criticQuanTotsElsDausSis() {
        TiradaD6.ResultatTirada r = TiradaD6.tirar(3, Avantatge.NORMAL, rngCara(6));
        assertTrue(r.critic());
        assertFalse(r.pifia());
        assertEquals(3.0, r.exitos());
        assertEquals(3, r.daus().size());
    }

    @Test
    void pifiaQuanZeroExits() {
        TiradaD6.ResultatTirada r = TiradaD6.tirar(4, Avantatge.NORMAL, rngCara(2));
        assertTrue(r.pifia());
        assertFalse(r.critic());
        assertEquals(0.0, r.exitos());
    }

    @Test
    void zeroDausEsPifiaNoCritic() {
        TiradaD6.ResultatTirada r = TiradaD6.tirar(0, Avantatge.NORMAL, new Random(0));
        assertTrue(r.pifia());
        assertFalse(r.critic(), "0 daus no és crític (cal com a mínim un 6)");
        assertEquals(0.0, r.exitos());
        assertTrue(r.daus().isEmpty());
    }

    @Test
    void tiradaDeterministaAmbSeedFixaEsReproduible() {
        // Mateixa seed → mateix resultat exacte. Aquesta és la propietat clau pels tests.
        TiradaD6.ResultatTirada r1 = TiradaD6.tirar(5, Avantatge.NORMAL, new Random(42));
        TiradaD6.ResultatTirada r2 = TiradaD6.tirar(5, Avantatge.NORMAL, new Random(42));
        assertEquals(r1, r2, "mateixa seed ha de donar mateixa tirada");
        assertEquals(5, r1.daus().size());
        assertEquals(r1.daus().stream().mapToInt(Integer::intValue).sum(),
                r2.daus().stream().mapToInt(Integer::intValue).sum());
    }

    @Test
    void superarCDMajorOIgualQue() {
        TiradaD6.ResultatTirada r = new TiradaD6.ResultatTirada(List.of(6, 5), 1.5, Avantatge.NORMAL, false, false);
        assertTrue(TiradaD6.superarCD(r, TiradaD6.CD_NORMAL), "1,5 èxits ≥ CD 1,0");
        assertTrue(TiradaD6.superarCD(r, 1.5), "1,5 èxits ≥ CD 1,5 (igualat)");
        assertFalse(TiradaD6.superarCD(r, 2.0), "1,5 èxits < CD 2,0");
    }

    @Test
    void overloadAmbFontsResolElBalancAbansDeTirar() {
        // 2 avantatges contra 1 desavantatge → sobreviu AVANTATGE → el 4 val 0,5.
        TiradaD6.ResultatTirada r = TiradaD6.tirar(1, 2, 1, rngCara(4));
        assertEquals(Avantatge.AVANTATGE, r.mode());
        assertEquals(0.5, r.exitos());
    }

    @Test
    void overloadAmbFontsIgualadesCancelaITornaANormal() {
        // 1 avantatge i 1 desavantatge es cancel·len: taula normal → el 4 no compta.
        TiradaD6.ResultatTirada r = TiradaD6.tirar(1, 1, 1, rngCara(4));
        assertEquals(Avantatge.NORMAL, r.mode());
        assertEquals(0.0, r.exitos());
    }

    @Test
    void overloadNomesDesavantatgeNoAcumulaMesEnllaDeLaTaula() {
        // 3 fonts de desavantatge segueixen sent DESAVANTATGE, no una taula més dura.
        TiradaD6.ResultatTirada r = TiradaD6.tirar(1, 0, 3, rngCara(5));
        assertEquals(Avantatge.DESAVANTATGE, r.mode());
        assertEquals(0.0, r.exitos(), "amb desavantatge el 5 no compta");
    }

    @Test
    void argumentsInvalidsLancenExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> TiradaD6.tirar(-1, Avantatge.NORMAL, new Random(0)));
        assertThrows(IllegalArgumentException.class, () -> TiradaD6.tirar(3, (Avantatge) null, new Random(0)));
        assertThrows(IllegalArgumentException.class, () -> TiradaD6.tirar(3, Avantatge.NORMAL, null));
        assertThrows(IllegalArgumentException.class, () -> TiradaD6.superarCD(null, 1.0));
    }

    // --- Utilitats ---

    /**
     * Random que sempre retorna la mateixa cara (1-6) per poder aïllar la taula d'èxits.
     * {@code nextInt(6)} retorna 0..5; TiredeD6 hi suma 1 → 1..6.
     */
    private static Random rngCara(int cara) {
        return new Random() {
            @Override
            public int nextInt(int bound) {
                return cara - 1;
            }
        };
    }

    /**
     * Tirada d'un sol dau forçant una cara concreta i un mode.
     */
    private static TiradaD6.ResultatTirada tirarAmbCaraFixa(int cara, Avantatge mode) {
        return TiradaD6.tirar(1, mode, rngCara(cara));
    }
}
