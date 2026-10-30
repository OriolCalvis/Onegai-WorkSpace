package cat.dnd.cc.combat.motor;

/**
 * Seguiment de la condició {@link Condicio#FATIGA} (regla 7.4 del GDD), l'única de les 10
 * condicions amb graus en lloc d'un simple actiu/inactiu.
 *
 * <ul>
 *   <li>Nivell acotat entre 0 i 6.</li>
 *   <li>Cada nivell resta 1 dau a totes les tirades de característica. Aquesta classe només
 *       exposa {@link #dausARestar()}; el "mínim 1 dau mentre la característica sigui > 0"
 *       depèn del valor de l'estadística en el moment de la tirada i l'ha d'aplicar qui faci
 *       la tirada (p. ex. {@code Math.max(1, statOriginal - fatiga.dausARestar())}).</li>
 *   <li>En arribar a nivell 6, el personatge mor ({@link #esMortal()}).</li>
 *   <li>Un descans llarg redueix 2 nivells ({@link #descansarLlarg()}), sense baixar de 0.</li>
 * </ul>
 *
 * <p>Mutable i sense Spring, seguint el patró de {@link EstatTorn}: representa l'estat en
 * viu d'un personatge durant l'escena/campanya en curs, no persistència de sessió (D8).</p>
 */
public final class NivellFatiga {

    /** Nivell a partir del qual (inclòs) el personatge mor. */
    public static final int NIVELL_MORTAL = 6;

    private int nivell = 0;

    /**
     * Afegeix nivells de Fatiga (p. ex. per un efecte de carta), acotant el resultat a 6.
     *
     * @param nivells quantitat de nivells a afegir; ha de ser ≥ 0
     * @throws IllegalArgumentException si {@code nivells} és negatiu (per restar-ne, useu
     *                                   {@link #descansarLlarg()}, que és l'única via de la 7.4)
     */
    public void afegirNivells(int nivells) {
        if (nivells < 0) {
            throw new IllegalArgumentException("nivells no pot ser negatiu: " + nivells);
        }
        nivell = Math.min(NIVELL_MORTAL, nivell + nivells);
    }

    /**
     * Descans llarg (8h, regla 7.5): redueix la Fatiga en 2 nivells, sense baixar de 0.
     */
    public void descansarLlarg() {
        nivell = Math.max(0, nivell - 2);
    }

    /**
     * @return el nivell actual de Fatiga (0-6)
     */
    public int getNivell() {
        return nivell;
    }

    /**
     * @return {@code true} si el nivell ha arribat a {@link #NIVELL_MORTAL}: el personatge mor
     */
    public boolean esMortal() {
        return nivell >= NIVELL_MORTAL;
    }

    /**
     * @return el nombre de daus a restar de qualsevol tirada de característica per aquest
     *         nivell de Fatiga (equival al nivell actual; el mínim d'1 dau el garanteix qui
     *         apliqui aquest valor sobre l'estadística real)
     */
    public int dausARestar() {
        return nivell;
    }
}
