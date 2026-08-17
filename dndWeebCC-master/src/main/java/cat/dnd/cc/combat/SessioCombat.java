package cat.dnd.cc.combat;

import cat.dnd.cc.combat.motor.EstatTorn;
import cat.dnd.cc.combat.motor.OrdreIniciativa;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Un encontre de combat en viu: un grup de participants (personatges i NPCs/enemics) amb el
 * seu ordre d'iniciativa (regla 7.3 del GDD, {@link OrdreIniciativa}) i l'estat de torn
 * ({@link EstatTorn}) de cadascun (D4).
 *
 * <p>Purament en memòria (D10): es crea en començar el combat des de {@code MesaCombatService}
 * i es descarta en acabar-lo. No té relació amb {@code Aventura.Combate} (l'"encontre
 * preparat" a nivell de disseny d'aventura): aquesta classe és la sessió en viu, un cop el DJ
 * ja ha decidit que aquell combat comença ara.</p>
 */
public final class SessioCombat {

    /**
     * @param id           identificador estable del participant dins d'aquesta sessió
     * @param nom          nom a mostrar (personatge o NPC)
     * @param esPersonatge {@code true} si és un personatge jugador; {@code false} si és NPC/enemic
     * @param iniciativa   Iniciativa ja resolta (DES + Modificadors)
     * @param desBase      DES base, només per al desempat de {@link OrdreIniciativa}
     */
    public record Participant(String id, String nom, boolean esPersonatge, double iniciativa, int desBase) {
        public Participant {
            Objects.requireNonNull(id, "id no pot ser null");
            Objects.requireNonNull(nom, "nom no pot ser null");
        }
    }

    private final String nom;
    private final List<Participant> participants;
    private final List<String> ordreTorn;
    private final Map<String, EstatTorn> tornsPerParticipant = new LinkedHashMap<>();
    private int indexTornActual = 0;

    /**
     * @param nom          nom d'aquest combat (per mostrar-lo a la mesa del DJ)
     * @param participants tots els participants, com a mínim un
     */
    public SessioCombat(String nom, List<Participant> participants) {
        if (nom == null || nom.isBlank()) {
            throw new IllegalArgumentException("nom no pot ser buit");
        }
        if (participants == null || participants.isEmpty()) {
            throw new IllegalArgumentException("cal com a mínim un participant per començar un combat");
        }
        this.nom = nom;
        this.participants = List.copyOf(participants);

        List<OrdreIniciativa.Participant> perOrdenar = this.participants.stream()
                .map(p -> new OrdreIniciativa.Participant(p.id(), p.iniciativa(), p.desBase()))
                .toList();
        this.ordreTorn = OrdreIniciativa.ordenar(perOrdenar);

        for (Participant p : this.participants) {
            tornsPerParticipant.put(p.id(), new EstatTorn());
        }
    }

    public String nom() {
        return nom;
    }

    public List<Participant> participants() {
        return participants;
    }

    /** @return els {@code id} dels participants, ja ordenats per Iniciativa (regla 7.3) */
    public List<String> ordreTorn() {
        return ordreTorn;
    }

    /** @return l'{@code id} del participant a qui toca actuar ara mateix */
    public String participantActualId() {
        return ordreTorn.get(indexTornActual);
    }

    public Participant participantPerId(String id) {
        for (Participant p : participants) {
            if (p.id().equals(id)) {
                return p;
            }
        }
        throw new IllegalArgumentException("participant no trobat: " + id);
    }

    /**
     * @return l'{@link EstatTorn} d'un participant d'aquesta sessió
     * @throws IllegalArgumentException si el participant no hi és
     */
    public EstatTorn estatTornDe(String participantId) {
        EstatTorn estat = tornsPerParticipant.get(participantId);
        if (estat == null) {
            throw new IllegalArgumentException("participant no trobat: " + participantId);
        }
        return estat;
    }

    /**
     * Passa el torn al següent participant de l'ordre d'iniciativa (cíclic: després de
     * l'últim torna al primer, començant una nova ronda) i li recupera l'acció principal,
     * la de moviment i la reacció ({@link EstatTorn#iniciarNouTorn()}).
     */
    public void seguentTorn() {
        indexTornActual = (indexTornActual + 1) % ordreTorn.size();
        estatTornDe(participantActualId()).iniciarNouTorn();
    }

    /** @return quants participants hi ha en aquest combat */
    public int mida() {
        return participants.size();
    }
}
