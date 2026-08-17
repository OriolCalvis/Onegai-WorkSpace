package cat.dnd.cc.combat.motor;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Ordenació determinista de l'ordre de torn per Iniciativa (regla 7.3 del GDD).
 *
 * <p>La Iniciativa és determinista i sense tirada (`DES + Modificadors`, secció 5 del GDD):
 * aquesta classe <b>no calcula</b> la Iniciativa de cap personatge (això ja el fa
 * {@code PersonatgeService.calcularFicha()}, exposat a {@code FichaPersonatge#iniciativa()});
 * només ordena una llista de participants que ja porten el seu valor resolt.</p>
 *
 * <p>Criteri d'ordenació (7.3):</p>
 * <ol>
 *   <li>Iniciativa descendent.</li>
 *   <li>Empat → decideix qui tingui més DES base.</li>
 *   <li>Si encara persisteix l'empat, ho decideix el director de joc: aquesta classe
 *       <b>no</b> inventa cap criteri més enllà d'aquest punt. {@link #ordenar(List)} manté
 *       l'ordre relatiu d'entrada per als empatats totals (ordenació estable) com a valor per
 *       defecte provisional, i {@link #grupsEmpatatsSenseResoldre(List)} exposa exactament
 *       quins participants necessiten que el DJ ho resolgui (per a la UI de D10).</li>
 * </ol>
 *
 * <p>Classe {@code final} sense estat ni Spring, seguint el patró de {@link TiradaD6}.</p>
 */
public final class OrdreIniciativa {

    private OrdreIniciativa() {
    }

    /**
     * Un participant amb la seva Iniciativa i DES base ja resolts (vegeu la classe).
     *
     * @param id         identificador del participant (personatge o enemic); mai {@code null}
     * @param iniciativa Iniciativa ja calculada (DES + Modificadors)
     * @param desBase    DES base, usat només com a criteri de desempat
     */
    public record Participant(String id, double iniciativa, int desBase) {
        public Participant {
            Objects.requireNonNull(id, "id no pot ser null");
        }
    }

    /**
     * Ordena els participants de major a menor Iniciativa, desempatant per DES base.
     * Els empats totals (mateixa Iniciativa i mateix DES base) mantenen l'ordre relatiu
     * amb que es van rebre a {@code participants} (ordenació estable).
     *
     * @param participants llista de participants; mai {@code null}
     * @return nova llista amb els {@code id} en ordre de torn
     * @throws IllegalArgumentException si {@code participants} és {@code null}
     */
    public static List<String> ordenar(List<Participant> participants) {
        exigirLlistaNoNull(participants);
        List<Participant> copia = new ArrayList<>(participants);
        copia.sort(
                Comparator.comparingDouble(Participant::iniciativa).reversed()
                        .thenComparing(Comparator.comparingInt(Participant::desBase).reversed())
        );
        return copia.stream().map(Participant::id).toList();
    }

    /**
     * Detecta els grups de participants que segueixen empatats després d'aplicar Iniciativa
     * i DES base: la regla 7.3 diu que a partir d'aquí ho decideix el director de joc, no un
     * algorisme. Cada grup retornat té 2 o més {@code id} amb exactament la mateixa Iniciativa
     * i el mateix DES base.
     *
     * @param participants llista de participants; mai {@code null}
     * @return llista de grups empatats (buida si no n'hi ha cap); cada grup manté l'ordre
     *         relatiu d'entrada
     * @throws IllegalArgumentException si {@code participants} és {@code null}
     */
    public static List<List<String>> grupsEmpatatsSenseResoldre(List<Participant> participants) {
        exigirLlistaNoNull(participants);
        Map<ClauEmpat, List<String>> agrupats = new LinkedHashMap<>();
        for (Participant p : participants) {
            ClauEmpat clau = new ClauEmpat(p.iniciativa(), p.desBase());
            agrupats.computeIfAbsent(clau, k -> new ArrayList<>()).add(p.id());
        }
        List<List<String>> empats = new ArrayList<>();
        for (List<String> grup : agrupats.values()) {
            if (grup.size() > 1) {
                empats.add(List.copyOf(grup));
            }
        }
        return empats;
    }

    private static void exigirLlistaNoNull(List<Participant> participants) {
        if (participants == null) {
            throw new IllegalArgumentException("participants no pot ser null");
        }
    }

    /** Clau d'agrupació per detectar empats exactes d'Iniciativa + DES base. */
    private record ClauEmpat(double iniciativa, int desBase) {
    }
}
