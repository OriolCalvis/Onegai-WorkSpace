package cat.dnd.cc.combat.motor;

/**
 * Estat transitori dels recursos d'acció d'un personatge durant un torn (regla 7.3 del GDD).
 *
 * <p>Cada torn un personatge disposa de: 1 Acció principal (⚔), 1 Acció de Moviment (👣) i
 * 1 Reacció (🛡) per ronda. Pot substituir l'acció principal I la de moviment per una
 * Canalització (⏳) que roman activa fins que s'interromp explícitament. Les Accions lliures
 * (parlar, deixar anar un objecte...) són il·limitades i no es gasten mai; aquesta classe no
 * en fa seguiment, com tampoc en fa dels tipus {@link TipusAccio#ACCION_MENOR},
 * {@link TipusAccio#PASIVA}, {@link TipusAccio#PREPARACION} ni {@link TipusAccio#CONSUMIBLE},
 * que no consumeixen cap dels quatre recursos limitats.</p>
 *
 * <p>Com que en aquest sistema d'iniciativa cada personatge actua un únic cop per ronda,
 * "una Reacció per ronda" i "una Reacció per torn (d'aquest personatge)" són equivalents:
 * es recupera a {@link #iniciarNouTorn()} igual que l'acció principal i la de moviment.</p>
 *
 * <p>Mutable i sense Spring: representa l'estat en viu d'un personatge durant l'escena en
 * curs (D8 li donarà persistència real de sessió; per ara viu només en memòria del procés
 * que la instanciï). No és thread-safe: cal un {@code EstatTorn} per personatge i no s'ha
 * de compartir entre fils.</p>
 */
public final class EstatTorn {

    private boolean accioPrincipalDisponible = true;
    private boolean accioMovimentDisponible = true;
    private boolean reaccioDisponible = true;
    private boolean canalitzant = false;

    /**
     * Consumeix el recurs corresponent a un tipus d'acció.
     *
     * @param tipus tipus d'acció a gastar; mai {@code null}
     * @throws IllegalArgumentException si {@code tipus} és {@code null}
     * @throws IllegalStateException    si el recurs ja s'ha gastat aquest torn/ronda, o si
     *                                   es demana {@link TipusAccio#CANALIZACION} sense tenir
     *                                   lliures alhora l'acció principal i la de moviment
     */
    public void consumir(TipusAccio tipus) {
        if (tipus == null) {
            throw new IllegalArgumentException("tipus no pot ser null");
        }
        switch (tipus) {
            case ACCION -> consumirAccioPrincipal();
            case MOVIMIENTO -> consumirAccioMoviment();
            case REACCION -> consumirReaccio();
            case CANALIZACION -> consumirCanalitzacio();
            case ACCION_MENOR, PASIVA, PREPARACION, CONSUMIBLE -> {
                // No gasten cap dels quatre recursos limitats de la 7.3: es resolen com a
                // accions lliures o efectes instantanis/passius a nivell de la pròpia carta.
            }
        }
    }

    private void consumirAccioPrincipal() {
        exigirDisponible(accioPrincipalDisponible, "L'acció principal ja s'ha gastat aquest torn");
        accioPrincipalDisponible = false;
    }

    private void consumirAccioMoviment() {
        exigirDisponible(accioMovimentDisponible, "L'acció de moviment ja s'ha gastat aquest torn");
        accioMovimentDisponible = false;
    }

    private void consumirReaccio() {
        exigirDisponible(reaccioDisponible, "La reacció ja s'ha gastat aquesta ronda");
        reaccioDisponible = false;
    }

    /**
     * La Canalització substitueix l'acció principal I la de moviment del torn (regla 7.3):
     * calen totes dues lliures i queden gastades alhora.
     */
    private void consumirCanalitzacio() {
        exigirDisponible(accioPrincipalDisponible && accioMovimentDisponible,
                "Cal l'acció principal i la de moviment lliures per canalitzar");
        accioPrincipalDisponible = false;
        accioMovimentDisponible = false;
        canalitzant = true;
    }

    private static void exigirDisponible(boolean disponible, String missatge) {
        if (!disponible) {
            throw new IllegalStateException(missatge);
        }
    }

    /**
     * Interromp una canalització en curs: dany que falla la prova de manteniment de CAR/CON
     * que defineixi la carta (a resoldre amb {@link TiradaD6} pel cridant), o final voluntari.
     * No restaura l'acció principal ni la de moviment: ja es van gastar en iniciar-la.
     */
    public void interrompreCanalitzacio() {
        canalitzant = false;
    }

    /**
     * Recupera l'acció principal, la de moviment i la reacció a l'inici d'un nou torn
     * d'aquest personatge. Una canalització activa NO es cancel·la aquí: roman activa fins
     * que {@link #interrompreCanalitzacio()} l'atura explícitament, tal com indica la 7.3
     * ("la habilidad permanece activa mientras el personaje mantenga concentración").
     */
    public void iniciarNouTorn() {
        accioPrincipalDisponible = true;
        accioMovimentDisponible = true;
        reaccioDisponible = true;
    }

    public boolean isAccioPrincipalDisponible() {
        return accioPrincipalDisponible;
    }

    public boolean isAccioMovimentDisponible() {
        return accioMovimentDisponible;
    }

    public boolean isReaccioDisponible() {
        return reaccioDisponible;
    }

    public boolean isCanalitzant() {
        return canalitzant;
    }
}
