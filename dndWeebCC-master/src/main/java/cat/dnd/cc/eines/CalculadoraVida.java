package cat.dnd.cc.eines;

import cat.dnd.cc.model.TierClass;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Aplica la fórmula de vida del sistema de tiers y cartas:
 *
 *   Vida = Vida Base de Clase + (CON × Multiplicador de Clase) + Bono de Tier
 *
 * (el "Bono de Equipo" de la fórmula completa no aplica aquí porque esta calculadora
 * previsualiza la clase en abstracto, sin equipo concreto).
 * Ver docs/Sistema_Cartas_Tiers.md, sección 5.
 */
public final class CalculadoraVida {

    private static final Map<Integer, Integer> BONO_POR_TIER = Map.of(
            0, 0,
            1, 0,
            2, 4,
            3, 9,
            4, 16,
            5, 25
    );

    private CalculadoraVida() {
    }

    public static double calcular(int vidaBase, double multiplicadorCon, int con, int tier) {
        int bonoTier = BONO_POR_TIER.getOrDefault(tier, 0);
        return vidaBase + (multiplicadorCon * con) + bonoTier;
    }

    public record PuntoVida(int con, double vida) {
    }

    /**
     * Genera una previsualización de vida para CON de 1 a 8 en el tier actual de la carta,
     * para que quien diseña la clase vea de un vistazo si el balance tiene sentido.
     */
    public static List<PuntoVida> previsualizar(TierClass carta) {
        List<PuntoVida> puntos = new ArrayList<>();
        for (int con = 1; con <= 8; con++) {
            double vida = calcular(carta.getBaseHealth(), carta.getHealthScalingCon(), con, carta.getTier());
            puntos.add(new PuntoVida(con, vida));
        }
        return puntos;
    }
}
