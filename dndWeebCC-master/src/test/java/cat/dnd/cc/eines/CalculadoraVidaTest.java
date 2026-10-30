package cat.dnd.cc.eines;

import cat.dnd.cc.model.TierClass;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

/**
 * La fórmula de vida de la Ed. 4 (GDD §5, CARTAS-CLASES-01):
 * Vida = min(Vida base de clase + CON×3 + CAR + Bono de tier + Bono de equipo, Techo del tier),
 * con los números del motor (tier_rules.json): bonos 0/0/2/5/8/13 y techos 14/18/25/35/48/63.
 */
class CalculadoraVidaTest {

    @Test
    void conPorTresYCarPorUnoSinBonoEnTierUno() {
        // Bardo (ágil, base 5) con CON 2 y CAR 2: 5 + 6 + 2 + 0 = 13
        assertEquals(13, CalculadoraVida.calcular(5, 2, 2, 1, 0));
    }

    @Test
    void elBonoDeTierSigueLaTablaDelMotor() {
        assertEquals(0, CalculadoraVida.bonoTier(0));
        assertEquals(0, CalculadoraVida.bonoTier(1));
        assertEquals(2, CalculadoraVida.bonoTier(2));
        assertEquals(5, CalculadoraVida.bonoTier(3));
        assertEquals(8, CalculadoraVida.bonoTier(4));
        assertEquals(13, CalculadoraVida.bonoTier(5));
        // lanzador (base 4), CON 1, CAR 0 en tier 3: 4 + 3 + 0 + 5 = 12
        assertEquals(12, CalculadoraVida.calcular(4, 1, 0, 3, 0));
    }

    @Test
    void nadiePasaDelTechoDeSuTier() {
        assertEquals(14, CalculadoraVida.techoTier(0));
        assertEquals(18, CalculadoraVida.techoTier(1));
        assertEquals(25, CalculadoraVida.techoTier(2));
        assertEquals(63, CalculadoraVida.techoTier(5));
        // tanque (base 8) con CON 4, CAR 2 y +3 de equipo en tier 1: 8 + 12 + 2 + 0 + 3 = 25 -> 18
        assertEquals(18, CalculadoraVida.calcular(8, 4, 2, 1, 3));
    }

    @Test
    void unTierFueraDeRangoSeAcota() {
        assertEquals(CalculadoraVida.techoTier(5), CalculadoraVida.techoTier(99));
        assertEquals(CalculadoraVida.bonoTier(0), CalculadoraVida.bonoTier(-1));
    }

    @Test
    void previsualizarDaDeCeroASeisDeConYTocaTecho() {
        TierClass carta = new TierClass();
        carta.setBaseHealth(6);
        carta.setTier(1);

        List<CalculadoraVida.PuntoVida> puntos = CalculadoraVida.previsualizar(carta);

        assertEquals(7, puntos.size());
        assertEquals(0, puntos.get(0).con());
        assertEquals(6, puntos.get(0).vida());   // 6 + 0
        assertEquals(15, puntos.get(3).vida());  // 6 + 9
        assertEquals(18, puntos.get(4).vida());  // 6 + 12 = 18, el techo
        assertEquals(18, puntos.get(6).vida());  // 6 + 18 = 24 -> 18
    }
}
