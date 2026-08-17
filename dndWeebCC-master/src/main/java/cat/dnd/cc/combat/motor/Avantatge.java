package cat.dnd.cc.combat.motor;

/**
 * Mode de tirada segons la regla 7.2 del GDD (avantatge/desavantatge).
 *
 * <p>El mode determina quines cares del d6 compten com a èxit:</p>
 * <ul>
 *   <li>{@link #NORMAL} — 6 = 1 èxit, 5 = 0,5 èxits (taula base).</li>
 *   <li>{@link #AVANTATGE} — 6 = 1, 5 = 0,5, 4 = 0,5 (el 4 també compta mig).</li>
 *   <li>{@link #DESAVANTATGE} — només el 6 compta (1 èxit).</li>
 * </ul>
 *
 * <p><b>No acumulables</b>: diversos avantatges i una desavantatge es cancel·len
 * mútuament i tornen a la taula normal. Vegeu {@link #resoldre(int, int)}.</p>
 *
 * <p>Aquest enum viu al {@code motor} perquè {@link TiradaD6} l'usa directament: vegeu
 * {@link TiradaD6#tirar(int, int, int, java.util.Random)} per tirar resolent l'anul·lació
 * mútua en un sol pas (D3 del backlog WS-D, ja cablejat).</p>
 */
public enum Avantatge {

    NORMAL,
    AVANTATGE,
    DESAVANTATGE;

    /**
     * Resol el mode resultant aplicant l'anul·lació mútua de la regla 7.2:
     * cada avantatge es cancel·la amb una desavantatge; el que en sobreviu decideix
     * el mode. Si no en queda capun, {@link #NORMAL}.
     *
     * @param numAvantatges  quantitat de fonts d'avantatge actives (≥ 0)
     * @param numDesavantatges quantitat de fonts de desavantatge actives (≥ 0)
     * @return el mode efectiu; mai {@code null}
     */
    public static Avantatge resoldre(int numAvantatges, int numDesavantatges) {
        int balanc = Math.max(0, numAvantatges) - Math.max(0, numDesavantatges);
        if (balanc > 0) {
            return AVANTATGE;
        }
        if (balanc < 0) {
            return DESAVANTATGE;
        }
        return NORMAL;
    }
}
