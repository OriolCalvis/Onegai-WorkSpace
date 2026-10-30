package cat.dnd.cc.combat.motor;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Estat en viu de les piles de recuperació de les cartes d'<b>un</b> personatge durant una
 * sessió (regla 6 del GDD, "el sistema de pilas"): <i>"No hay barras de puntos (maná,
 * energía...) que trackear turno a turno. El recurso de un personaje es literalmente dónde
 * están sus cartas de habilidad."</i>
 *
 * <p>Cada carta registrada viu en una de tres {@link Pila}: Activa (disponible per jugar),
 * Descans Curt o Descans Llarg. En jugar-se, es <b>mou</b> (no es gasta) cap a la pila que
 * indica el seu {@link Recovery}; les cartes {@link Recovery#NINGUNO} (o {@link Recovery#ACTIVA})
 * mai deixen la pila Activa.</p>
 *
 * <p>Aquesta classe no coneix {@code TierSkill}/{@code TierSpell}/{@code Personatge}: qui la
 * faci servir ha de {@link #registrar(String, Recovery)} cada carta amb el {@code recovery}
 * ja resolt (per exemple amb {@link Recovery#desDeCamp(String)} sobre
 * {@code tierSkill.getRecovery()}). Una instància = un personatge; no és thread-safe.</p>
 */
public final class EstatPiles {

    private final Map<String, Recovery> recoveryPerCarta = new LinkedHashMap<>();
    private final Map<String, Pila> pilaActualPerCarta = new LinkedHashMap<>();

    /**
     * Registra una carta d'aquest personatge amb el seu {@code recovery} declarat. Comença
     * a la pila Activa. Tornar a registrar un {@code cardId} ja existent el reinicia a Activa
     * amb el nou {@code recovery} (útil per recarregar la mà a l'inici d'una sessió).
     *
     * @param cardId   identificador de la carta (habilitat/hechizo); mai {@code null}
     * @param recovery recovery declarat per la carta; mai {@code null}
     */
    public void registrar(String cardId, Recovery recovery) {
        if (cardId == null) {
            throw new IllegalArgumentException("cardId no pot ser null");
        }
        if (recovery == null) {
            throw new IllegalArgumentException("recovery no pot ser null");
        }
        recoveryPerCarta.put(cardId, recovery);
        pilaActualPerCarta.put(cardId, Pila.ACTIVA);
    }

    /**
     * Juga una carta registrada: la mou de la pila Activa a la pila que indica el seu
     * {@code recovery}. Les cartes {@link Recovery#NINGUNO}/{@link Recovery#ACTIVA} es queden
     * a Activa (no és un error jugar-les repetidament).
     *
     * @param cardId identificador de la carta, ja registrada
     * @throws IllegalArgumentException si la carta no s'ha registrat
     * @throws IllegalStateException    si la carta no és ara mateix a la pila Activa (ja
     *                                   s'havia jugat i encara no ha tornat amb un descans)
     */
    public void jugar(String cardId) {
        Recovery recovery = recoveryDe(cardId);
        Pila actual = pilaDe(cardId);
        if (recovery != Recovery.NINGUNO && recovery != Recovery.ACTIVA && actual != Pila.ACTIVA) {
            throw new IllegalStateException(
                    "La carta " + cardId + " no és a la pila Activa (és a " + actual + ")");
        }
        Pila desti = switch (recovery) {
            case ACTIVA, NINGUNO -> Pila.ACTIVA;
            case DESCANSO_CORTO -> Pila.DESCANSO_CORTO;
            case DESCANSO_LARGO -> Pila.DESCANSO_LARGO;
        };
        pilaActualPerCarta.put(cardId, desti);
    }

    /**
     * Redirigeix explícitament una carta ja registrada a una altra pila, sense mirar el seu
     * {@code recovery} declarat. Pensat per a excepcions de carta concretes (p. ex. una
     * pasiva que, un cop per descans curt, torna un hechizo directament a Activa en lloc
     * d'anar a la seva pila per defecte): qui gestioni el combat decideix quan s'aplica.
     *
     * @throws IllegalArgumentException si la carta no s'ha registrat, o si {@code desti} és
     *                                   {@code null}
     */
    public void moureA(String cardId, Pila desti) {
        recoveryDe(cardId);
        if (desti == null) {
            throw new IllegalArgumentException("desti no pot ser null");
        }
        pilaActualPerCarta.put(cardId, desti);
    }

    /**
     * Descans curt (1h, regla 7.5.5): tota la pila Descans Curt torna a Activa. Descans
     * Llarg no es toca.
     */
    public void descansarCurt() {
        moureTots(Pila.DESCANSO_CORTO, Pila.ACTIVA);
    }

    /**
     * Descans llarg (8h, regla 7.5.5): tant Descans Llarg com qualsevol carta encara no
     * recuperada a Descans Curt tornen a Activa.
     */
    public void descansarLlarg() {
        moureTots(Pila.DESCANSO_LARGO, Pila.ACTIVA);
        moureTots(Pila.DESCANSO_CORTO, Pila.ACTIVA);
    }

    private void moureTots(Pila origen, Pila desti) {
        for (Map.Entry<String, Pila> entrada : pilaActualPerCarta.entrySet()) {
            if (entrada.getValue() == origen) {
                entrada.setValue(desti);
            }
        }
    }

    /**
     * @return la pila on és ara mateix una carta registrada
     * @throws IllegalArgumentException si la carta no s'ha registrat
     */
    public Pila pilaDe(String cardId) {
        Pila pila = pilaActualPerCarta.get(cardId);
        if (pila == null) {
            throw new IllegalArgumentException("carta no registrada: " + cardId);
        }
        return pila;
    }

    /**
     * @return quantes cartes registrades hi ha ara mateix a la pila indicada
     */
    public int comptar(Pila pila) {
        int total = 0;
        for (Pila actual : pilaActualPerCarta.values()) {
            if (actual == pila) {
                total++;
            }
        }
        return total;
    }

    /**
     * @return els {@code cardId} de les cartes registrades que ara mateix són a la pila
     *         indicada, en ordre de registre
     */
    public List<String> cartesA(Pila pila) {
        List<String> resultat = new ArrayList<>();
        for (Map.Entry<String, Pila> entrada : pilaActualPerCarta.entrySet()) {
            if (entrada.getValue() == pila) {
                resultat.add(entrada.getKey());
            }
        }
        return List.copyOf(resultat);
    }

    private Recovery recoveryDe(String cardId) {
        Recovery recovery = recoveryPerCarta.get(cardId);
        if (recovery == null) {
            throw new IllegalArgumentException("carta no registrada: " + cardId);
        }
        return recovery;
    }
}
