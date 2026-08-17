package cat.dnd.cc.combat.motor;

/**
 * Valor del camp {@code recovery} d'una carta jugable (habilitat/hechizo), regla 6 del GDD
 * ("el sistema de pilas"): indica a quina {@link Pila} va la carta després de jugar-se.
 *
 * <p>Correspon un a un amb el {@code String} lliure que ja existeix a
 * {@code TierSkill.recovery} / {@code TierSpell.recovery}. Als 483 fitxers de dades actuals
 * només apareixen quatre valors: {@code activa}, {@code descanso_corto}, {@code descanso_largo}
 * i {@code ninguno} (aquest últim, un únic cas). Vegeu {@link #desDeCamp(String)}.</p>
 */
public enum Recovery {

    /** La carta torna (o es queda) a la pila Activa en jugar-se. */
    ACTIVA,

    /** La carta va a la pila Descans Curt en jugar-se; torna a Activa amb un descans curt. */
    DESCANSO_CORTO,

    /** La carta va a la pila Descans Llarg en jugar-se; només torna amb un descans llarg. */
    DESCANSO_LARGO,

    /** La carta mai es mou de la pila Activa (passives o d'ús lliure, baix impacte). */
    NINGUNO;

    /**
     * Converteix el valor lliure del camp {@code recovery} de la carta a aquest enum.
     *
     * @param recovery valor tal com es guarda a la carta; {@code null} o buit es tracta com
     *                  a {@link #ACTIVA} (el valor per defecte al model)
     * @return el recovery equivalent; mai {@code null}
     * @throws IllegalArgumentException si el valor no és cap dels quatre reconeguts
     */
    public static Recovery desDeCamp(String recovery) {
        if (recovery == null || recovery.isBlank()) {
            return ACTIVA;
        }
        return switch (recovery.trim().toLowerCase()) {
            case "activa" -> ACTIVA;
            case "descanso_corto" -> DESCANSO_CORTO;
            case "descanso_largo" -> DESCANSO_LARGO;
            case "ninguno" -> NINGUNO;
            default -> throw new IllegalArgumentException("recovery desconegut: " + recovery);
        };
    }
}
