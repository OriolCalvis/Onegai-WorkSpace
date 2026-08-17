package cat.dnd.cc.combat.motor;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Motor de tirades del sistema ONEGAI (regla 7.1 del GDD).
 *
 * <p>Es tiren tants d6 com el valor de la característica implicada (p. ex. DES 5 → 5d6).
 * La taula d'èxits depèn del {@link Avantatge} actiu:</p>
 *
 * <table>
 *   <caption>Taula d'èxits per cara segons el mode</caption>
 *   <tr><th>Cara</th><th>NORMAL</th><th>AVANTATGE</th><th>DESAVANTATGE</th></tr>
 *   <tr><td>6</td><td>1,0</td><td>1,0</td><td>1,0</td></tr>
 *   <tr><td>5</td><td>0,5</td><td>0,5</td><td>0</td></tr>
 *   <tr><td>4</td><td>0</td><td>0,5</td><td>0</td></tr>
 *   <tr><td>1-3</td><td>0</td><td>0</td><td>0</td></tr>
 * </table>
 *
 * <p>Una tirada és <b>crític</b> quan tots els daus són 6, i <b>pifia</b> quan suma 0 èxits.
 * El resultat ha d'igualar o superar una {@code CD} (0,5 Fàcil · 1 Normal · 1,5 Difícil ·
 * 2 Molt difícil · 2,5+ Extraordinària).</p>
 *
 * <p>Classe {@code final} amb mètodes {@code static}, sense estat ni I/O, perquè sigui
 * testejable de forma aïllada i determinista injectant un {@link Random} amb seed fixa.
 * Mai es fa {@code new Random()} ni {@code Math.random()} dins d'aquesta classe.</p>
 */
public final class TiradaD6 {

    /** CD estàndard del manual (regla 7.1). */
    public static final double CD_FACIL = 0.5;
    public static final double CD_NORMAL = 1.0;
    public static final double CD_DESAFIANTE = 1.0;
    public static final double CD_DIFICIL = 1.5;
    public static final double CD_MOLT_DIFICIL = 2.0;
    public static final double CD_EXTRAORDINARIA = 2.5;

    private TiradaD6() {
    }

    /**
     * Tirada base en mode {@link Avantatge#NORMAL}.
     *
     * @param numDaus nombre de d6 a tirar (serà el valor del stat implicat)
     * @param rng     generador de nombres (s'injecta per determinisme en tests)
     * @return el resultat de la tirada; mai {@code null}
     */
    public static ResultatTirada tirar(int numDaus, Random rng) {
        return tirar(numDaus, Avantatge.NORMAL, rng);
    }

    /**
     * Tirada resolent primer l'anul·lació mútua de fonts d'avantatge/desavantatge (regla 7.2):
     * cada font d'avantatge es cancel·la amb una de desavantatge i el que en sobreviu decideix
     * la taula; mai s'acumulen més enllà d'{@link Avantatge#AVANTATGE}/{@link Avantatge#DESAVANTATGE}.
     *
     * <p>Punt d'entrada recomanat quan el mode encara no s'ha resolt (p. ex. des de D4/D6, on
     * diverses cartes/condicions poden aportar fonts independents). Si el mode ja és conegut,
     * useu directament {@link #tirar(int, Avantatge, Random)}.</p>
     *
     * @param numDaus              nombre de d6 a tirar (≥ 0)
     * @param numFontsAvantatge    fonts d'avantatge actives (≥ 0)
     * @param numFontsDesavantatge fonts de desavantatge actives (≥ 0)
     * @param rng                  generador injectat
     * @return el resultat de la tirada amb el mode ja resolt
     */
    public static ResultatTirada tirar(int numDaus, int numFontsAvantatge, int numFontsDesavantatge, Random rng) {
        Avantatge mode = Avantatge.resoldre(numFontsAvantatge, numFontsDesavantatge);
        return tirar(numDaus, mode, rng);
    }

    /**
     * Tirada aplicant un {@link Avantatge} que canvia quines cares compten (regla 7.2).
     *
     * @param numDaus nombre de d6 a tirar (≥ 0)
     * @param mode    avantatge/desavantatge actiu; mai {@code null}
     * @param rng     generador injectat
     * @return el resultat de la tirada; per a 0 daus retorna un resultat amb 0 èxits i pifia
     */
    public static ResultatTirada tirar(int numDaus, Avantatge mode, Random rng) {
        if (numDaus < 0) {
            throw new IllegalArgumentException("numDaus no pot ser negatiu: " + numDaus);
        }
        if (mode == null) {
            throw new IllegalArgumentException("mode no pot ser null");
        }
        if (rng == null) {
            throw new IllegalArgumentException("rng no pot ser null (injecta'l per determinisme)");
        }

        List<Integer> daus = new ArrayList<>(numDaus);
        double exitos = 0.0;
        for (int i = 0; i < numDaus; i++) {
            int cara = 1 + rng.nextInt(6);
            daus.add(cara);
            exitos += exitosPerCara(cara, mode);
        }
        boolean critic = !daus.isEmpty() && daus.stream().allMatch(c -> c == 6);
        boolean pifia = exitos == 0.0;
        return new ResultatTirada(daus, exitos, mode, critic, pifia);
    }

    /**
     * Taula d'èxits per cara i mode (regla 7.1 + 7.2).
     */
    private static double exitosPerCara(int cara, Avantatge mode) {
        return switch (mode) {
            case NORMAL -> cara == 6 ? 1.0 : cara == 5 ? 0.5 : 0.0;
            case AVANTATGE -> cara >= 5 ? (cara == 6 ? 1.0 : 0.5) : cara == 4 ? 0.5 : 0.0;
            case DESAVANTATGE -> cara == 6 ? 1.0 : 0.0;
        };
    }

    /**
     * Comprova si una tirada supera o iguala una CD (regla 7.1).
     *
     * @param resultat la tirada prèvia
     * @param cd       dificultat objectiu
     * @return {@code true} si els èxits ≥ la CD
     */
    public static boolean superarCD(ResultatTirada resultat, double cd) {
        if (resultat == null) {
            throw new IllegalArgumentException("resultat no pot ser null");
        }
        return resultat.exitos() >= cd;
    }

    /**
     * Resultat immutable d'una tirada de pool de d6.
     *
     * @param daus    llista de cares tirades (1-6), en ordre
     * @param exitos  suma d'èxits segons la taula
     * @param mode    mode actiu durant la tirada
     * @param critic  {@code true} si tots els daus són 6
     * @param pifia   {@code true} si la tirada suma 0 èxits
     */
    public record ResultatTirada(
            List<Integer> daus,
            double exitos,
            Avantatge mode,
            boolean critic,
            boolean pifia
    ) {
        public ResultatTirada {
            daus = daus == null ? List.of() : List.copyOf(daus);
        }
    }
}
