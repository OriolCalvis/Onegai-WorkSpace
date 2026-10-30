package cat.dnd.cc.eines;

import cat.dnd.cc.model.TierClass;

import java.util.ArrayList;
import java.util.List;

/**
 * La vida de un personaje, Ed. 4 (docs/Sistema_Cartas_Tiers.md §5, unificada con el motor):
 *
 * <pre>
 *   Vida = min( Vida base de clase + CON × 3 + CAR × 1 + Bono de tier + Bono de equipo,
 *               Techo del tier )
 * </pre>
 *
 * Los números son los del motor ({@code MotorGraphico/assets/rules/tier_rules.json}): desde
 * DANO-02 (06/10/2026) el daño se tira en d6 por diferencia de éxitos (3,5 por dado en vez
 * de 10 por éxito) y toda la vida bajó a la mitad — las bases de clase (tanque 8,
 * equilibrado 6, ágil y apoyo 5, lanzador 4), los techos y los bonos de tier.
 * <p>
 * El multiplicador de CON es el mismo para todas las clases (GDD §5): el
 * {@code healthScaling} de la carta de clase ya no entra en la cuenta.
 */
public final class CalculadoraVida {

    /** Bono de vida de cada tier (0..5). */
    private static final int[] BONO_POR_TIER = {0, 0, 2, 5, 8, 13};
    /** Techo de vida de cada tier (0..5): nadie pasa de aquí, lleve lo que lleve. */
    private static final int[] TECHO_POR_TIER = {14, 18, 25, 35, 48, 63};

    private CalculadoraVida() {
    }

    private static int acotaTier(int tier) {
        return Math.max(0, Math.min(5, tier));
    }

    public static int bonoTier(int tier) {
        return BONO_POR_TIER[acotaTier(tier)];
    }

    public static int techoTier(int tier) {
        return TECHO_POR_TIER[acotaTier(tier)];
    }

    /** Vida = min(base + CON×3 + CAR + bono de tier + bono de equipo, techo del tier). */
    public static int calcular(int vidaBase, int con, int car, int tier, int bonoEquipo) {
        int total = vidaBase + con * 3 + car + bonoTier(tier) + bonoEquipo;
        return Math.min(total, techoTier(tier));
    }

    public record PuntoVida(int con, int vida) {
    }

    /**
     * La vida de la clase con CON de 0 a 6 (CAR 0, sin equipo) en su tier, para que quien
     * diseña la clase vea de un vistazo dónde toca techo. Cada punto de CAR suma 1 más.
     */
    public static List<PuntoVida> previsualizar(TierClass carta) {
        List<PuntoVida> puntos = new ArrayList<>();
        for (int con = 0; con <= 6; con++) {
            puntos.add(new PuntoVida(con, calcular(carta.getBaseHealth(), con, 0, carta.getTier(), 0)));
        }
        return puntos;
    }
}
