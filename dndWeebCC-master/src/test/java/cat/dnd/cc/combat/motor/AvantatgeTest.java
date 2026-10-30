package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

/**
 * Tests de {@link Avantatge#resoldre(int, int)}: anul·lació mútua de fonts d'avantatge
 * i desavantatge segons la regla 7.2 del GDD (D3 del backlog WS-D).
 */
class AvantatgeTest {

    @Test
    void sensCapFontEsNormal() {
        assertEquals(Avantatge.NORMAL, Avantatge.resoldre(0, 0));
    }

    @Test
    void nomesAvantatgeDonaAvantatge() {
        assertEquals(Avantatge.AVANTATGE, Avantatge.resoldre(1, 0));
        assertEquals(Avantatge.AVANTATGE, Avantatge.resoldre(3, 0));
    }

    @Test
    void nomesDesavantatgeDonaDesavantatge() {
        assertEquals(Avantatge.DESAVANTATGE, Avantatge.resoldre(0, 1));
        assertEquals(Avantatge.DESAVANTATGE, Avantatge.resoldre(0, 2));
    }

    @Test
    void unaFontDeCadaEsCancelenIQuedaNormal() {
        assertEquals(Avantatge.NORMAL, Avantatge.resoldre(1, 1));
        assertEquals(Avantatge.NORMAL, Avantatge.resoldre(5, 5));
    }

    @Test
    void elBalancNetDecideixElModeSenseAcumular() {
        // 3 fonts d'avantatge contra 1 de desavantatge: es cancel·la 1 a 1 i sobreviu
        // AVANTATGE, mai un "super-avantatge" que ampliï la taula més enllà del 4.
        assertEquals(Avantatge.AVANTATGE, Avantatge.resoldre(3, 1));
        assertEquals(Avantatge.DESAVANTATGE, Avantatge.resoldre(1, 3));
    }

    @Test
    void valorsNegatiusEsTractenComZero() {
        // Defensiu: cap crida hauria de passar comptadors negatius, però resoldre()
        // no ha de trencar-se ni retornar un mode fals si passa (Math.max(0, ...) intern).
        assertEquals(Avantatge.AVANTATGE, Avantatge.resoldre(2, -3));
        assertEquals(Avantatge.NORMAL, Avantatge.resoldre(-1, -1));
    }
}
