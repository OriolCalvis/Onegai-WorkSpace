package cat.dnd.cc.eines;

import cat.dnd.cc.model.TierClass;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

/**
 * Tests de la fórmula de vida (GDD sección 5):
 * Vida = Vida Base de Clase + (CON × Multiplicador de Clase) + Bono de Tier.
 */
class CalculadoraVidaTest {

    @Test
    void vidaEnTierUnoNoLlevaBonoDeTier() {
        // 20 base + 4 CON × 3.0 + 0 (tier 1) = 32
        assertEquals(32.0, CalculadoraVida.calcular(20, 3.0, 4, 1));
    }

    @Test
    void bonoDeTierSigueLaTablaDelManual() {
        // La tabla de bonos de tier es 0/0/4/9/16/25 para tiers 0-5.
        double base = CalculadoraVida.calcular(10, 1.0, 1, 0);
        assertEquals(11.0, base);
        assertEquals(11.0, CalculadoraVida.calcular(10, 1.0, 1, 1));
        assertEquals(15.0, CalculadoraVida.calcular(10, 1.0, 1, 2));
        assertEquals(20.0, CalculadoraVida.calcular(10, 1.0, 1, 3));
        assertEquals(27.0, CalculadoraVida.calcular(10, 1.0, 1, 4));
        assertEquals(36.0, CalculadoraVida.calcular(10, 1.0, 1, 5));
    }

    @Test
    void tierDesconocidoNoAportaBono() {
        assertEquals(11.0, CalculadoraVida.calcular(10, 1.0, 1, 99));
        assertEquals(11.0, CalculadoraVida.calcular(10, 1.0, 1, -1));
    }

    @Test
    void multiplicadorFraccionarioSeRespeta() {
        // Un mago frágil: 12 base + 6 CON × 1.5 = 21
        assertEquals(21.0, CalculadoraVida.calcular(12, 1.5, 6, 1));
    }

    @Test
    void previsualizarGeneraOchoPuntosCrecientes() {
        TierClass carta = new TierClass();
        carta.setBaseHealth(20);
        carta.setHealthScalingCon(3.0);
        carta.setTier(2);

        List<CalculadoraVida.PuntoVida> puntos = CalculadoraVida.previsualizar(carta);

        assertEquals(8, puntos.size());
        assertEquals(1, puntos.get(0).con());
        assertEquals(8, puntos.get(7).con());
        // 20 + 1×3 + 4 = 27 ... 20 + 8×3 + 4 = 48
        assertEquals(27.0, puntos.get(0).vida());
        assertEquals(48.0, puntos.get(7).vida());
        for (int i = 1; i < puntos.size(); i++) {
            assertEquals(3.0, puntos.get(i).vida() - puntos.get(i - 1).vida(),
                    "cada punto de CON debe sumar exactamente el multiplicador de la clase");
        }
    }
}
