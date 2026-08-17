package cat.dnd.cc.combat.motor;

import java.util.Random;

/**
 * Mort i agonia (regla 7.5 del GDD).
 *
 * <p>Arribar a 0 de Vida no és mort instantània llevat que un efecte ho digui explícitament:
 * el personatge cau Moribund (no pot fer accions principals ni de moviment) i, en començar
 * cada un dels seus torns, fa una <b>Salvació per mort</b>: tira tants d6 com el seu CON
 * contra {@link TiradaD6#CD_NORMAL}, sense més bonificadors llevat que una carta ho permeti
 * explícitament.</p>
 *
 * <ul>
 *   <li>Èxit (≥1 èxit) → s'estabilitza a {@link #VIDA_ESTABILITZAT} de Vida.</li>
 *   <li>Fracàs (0,5 èxits) → baixa 1 punt de Vida (cap a negatiu).</li>
 *   <li>Fracàs crític (0 èxits) → baixa 2 punts de Vida.</li>
 * </ul>
 *
 * <p>Vegeu {@link #esMortInstantania(int, int)} per al llindar de mort sense possibilitat
 * de salvació (7.5.3), i {@link CargaDelDesti} per a la clàusula de supervivència 7.5.4.</p>
 */
public final class SalvacioPerMort {

    /** Vida a la qual s'estabilitza un personatge que supera la Salvació per mort. */
    public static final int VIDA_ESTABILITZAT = 1;

    /** Llindar absolut de mort instantània, independent del CON (regla 7.5.3). */
    public static final int LLINDAR_MORT_ABSOLUT = -10;

    private SalvacioPerMort() {
    }

    /** Els tres desenllaços possibles d'una Salvació per mort. */
    public enum Resultat {
        ESTABILITZAT,
        EMPITJORA,
        EMPITJORA_GREU
    }

    /**
     * @param resultat      desenllaç de la tirada
     * @param vidaResultant Vida després d'aplicar el desenllaç
     */
    public record ResultatSalvacio(Resultat resultat, int vidaResultant) {
    }

    /**
     * Tira la Salvació per mort i retorna el desenllaç i la Vida que en resulta.
     *
     * @param con        CON del personatge (nombre de d6 a tirar); ≥ 0
     * @param vidaActual Vida abans de la salvació (típicament ≤ 0, ja Moribund)
     * @param rng        generador injectat, per determinisme
     * @return el desenllaç de la salvació; mai {@code null}
     */
    public static ResultatSalvacio tirar(int con, int vidaActual, Random rng) {
        TiradaD6.ResultatTirada t = TiradaD6.tirar(con, rng);
        if (t.exitos() >= 1.0) {
            return new ResultatSalvacio(Resultat.ESTABILITZAT, VIDA_ESTABILITZAT);
        }
        if (t.exitos() > 0.0) {
            return new ResultatSalvacio(Resultat.EMPITJORA, vidaActual - 1);
        }
        return new ResultatSalvacio(Resultat.EMPITJORA_GREU, vidaActual - 2);
    }

    /**
     * Mort instantània sense possibilitat de salvació (regla 7.5.3): quan la Vida baixa
     * de {@code -con} (en negatiu) o arriba a {@link #LLINDAR_MORT_ABSOLUT} (−10).
     *
     * @param vida Vida a comprovar (típicament la resultant d'una Salvació fallida)
     * @param con  CON del personatge
     * @return {@code true} si aquesta Vida suposa mort instantània
     */
    public static boolean esMortInstantania(int vida, int con) {
        return vida < -con || vida <= LLINDAR_MORT_ABSOLUT;
    }
}
