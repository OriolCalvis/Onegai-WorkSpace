package cat.dnd.cc.combat;

import cat.dnd.cc.combat.motor.EstatPiles;
import cat.dnd.cc.combat.motor.Recovery;
import cat.dnd.cc.model.Personatge;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.service.TierSkillService;
import cat.dnd.cc.service.TierSpellService;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;

/**
 * Estat de sessió de combat en viu (D8/D9 del backlog WS-D): manté un {@link EstatPiles} per
 * personatge, resolent les seves habilitats i hechizos contra el catàleg real.
 *
 * <p>És estat purament transitori i en memòria: no hi ha cap repositori ni fitxer a
 * {@code data/} darrere. Es perd en reiniciar l'aplicació, tal com pertoca a un "estado de
 * sesión" (no és cap catàleg a persistir). És el primer bean d'aquest tipus al projecte —
 * segueix l'estil dels {@code @Service} existents però sense escriptura a disc.</p>
 *
 * <p>Ids d'habilitat/hechizo trencats o orfes (que ja no existeixin al catàleg) simplement
 * no es registren: no s'ha de trencar la fitxa d'un personatge per una referència vella.</p>
 */
@Service
public class SessioCombatService {

    private final TierSkillService tierSkillService;
    private final TierSpellService tierSpellService;
    private final Map<Long, EstatPiles> pilesPerPersonatge = new ConcurrentHashMap<>();

    public SessioCombatService(TierSkillService tierSkillService, TierSpellService tierSpellService) {
        this.tierSkillService = tierSkillService;
        this.tierSpellService = tierSpellService;
    }

    /**
     * Retorna l'{@link EstatPiles} en viu d'aquest personatge. Si és la primera vegada que
     * es demana, es crea i es registren totes les seves habilitats/hechizos (amb el
     * {@code recovery} resolt del catàleg), tots començant a la pila Activa. Si ja existia,
     * es retorna tal qual: no es reinicia l'estat d'una partida en curs cada cop que es
     * consulta la fitxa.
     *
     * @param personatge personatge amb {@code id} ja assignat (persistit)
     * @return l'estat de piles d'aquest personatge; mai {@code null}
     */
    public EstatPiles estatPilesDe(Personatge personatge) {
        if (personatge == null || personatge.getId() == null) {
            throw new IllegalArgumentException("personatge (amb id) no pot ser null");
        }
        return pilesPerPersonatge.computeIfAbsent(personatge.getId(), id -> construirEstatInicial(personatge));
    }

    /**
     * Descarta l'estat en viu d'un personatge (p. ex. en tancar una escena/sessió). La
     * següent crida a {@link #estatPilesDe(Personatge)} el reconstruirà de zero, tot Activa.
     */
    public void reiniciar(Long personatgeId) {
        pilesPerPersonatge.remove(personatgeId);
    }

    private EstatPiles construirEstatInicial(Personatge personatge) {
        EstatPiles piles = new EstatPiles();
        for (String habilidadId : orBuit(personatge.getHabilidadIds())) {
            registrarSiExisteix(piles, habilidadId, tierSkillService::obtenerPorId, TierSkill::getRecovery);
        }
        for (String hechizoId : orBuit(personatge.getHechizoIds())) {
            registrarSiExisteix(piles, hechizoId, tierSpellService::obtenerPorId, TierSpell::getRecovery);
        }
        return piles;
    }

    private <T> void registrarSiExisteix(EstatPiles piles, String cardId,
                                          Function<String, T> buscador,
                                          Function<T, String> recoveryDe) {
        if (cardId == null || cardId.isBlank()) {
            return;
        }
        try {
            T carta = buscador.apply(cardId);
            piles.registrar(cardId, Recovery.desDeCamp(recoveryDe.apply(carta)));
        } catch (IllegalArgumentException noTrobadaORecoveryInvalid) {
            // Id orfe/trencat, o un valor de "recovery" malmès a les dades (ja n'hi ha hagut
            // casos, p. ex. la condició "ralentizado" que no existeix al catàleg real): no
            // bloquegem tota la fitxa per una sola carta, senzillament no es registra.
        }
    }

    private static List<String> orBuit(List<String> ids) {
        return ids == null ? List.of() : ids;
    }
}
