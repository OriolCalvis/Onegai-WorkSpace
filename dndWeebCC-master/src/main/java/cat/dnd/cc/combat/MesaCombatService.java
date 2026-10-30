package cat.dnd.cc.combat;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;

/**
 * Mesa de combat del director de joc (D10 del backlog WS-D): manté com a molt <b>una</b>
 * {@link SessioCombat} activa alhora, purament en memòria.
 *
 * <p>És coherent amb la resta de l'eina: ONEGAI és una eina d'un sol DJ jugant a la seva
 * taula, no una aplicació multi-sessió. Si mai calgués gestionar diverses meses de combat
 * en paral·lel, aquest servei és el punt a ampliar (per exemple, indexant per un identificador
 * de sessió/aventura en lloc de guardar-ne una sola).</p>
 */
@Service
public class MesaCombatService {

    private SessioCombat sessioActiva;

    /**
     * Comença un combat nou, descartant qualsevol sessió anterior que quedés activa sense
     * finalitzar explícitament.
     *
     * @param nom          nom del combat
     * @param participants tots els participants (personatges i NPCs), com a mínim un
     * @return la sessió acabada de crear
     */
    public SessioCombat iniciar(String nom, List<SessioCombat.Participant> participants) {
        sessioActiva = new SessioCombat(nom, participants);
        return sessioActiva;
    }

    /** @return la sessió de combat activa, si n'hi ha cap en curs */
    public Optional<SessioCombat> actual() {
        return Optional.ofNullable(sessioActiva);
    }

    /** Acaba el combat en curs. Cridar-ho sense combat actiu no fa res. */
    public void finalitzar() {
        sessioActiva = null;
    }
}
