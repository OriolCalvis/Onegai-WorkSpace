package cat.dnd.cc.combat.motor;

/**
 * Tipus d'acció que consumeix una carta jugable durant un torn (regla 7.3 del GDD).
 *
 * <p>Correspon un a un amb els valors del camp {@code actionType} de les cartes
 * jugables (avui un {@code String} lliure a {@code TierSkill}/{@code TierConsumable};
 * vegeu {@link #desDeActionType(String)} per convertir-lo a aquest enum de forma
 * validada).</p>
 *
 * <ul>
 *   <li>{@link #ACCION} — Acció principal (⚔): atacar, jugar una habilitat, usar un objecte.</li>
 *   <li>{@link #ACCION_MENOR} — acció secundària que no gasta els recursos limitats de 7.3.</li>
 *   <li>{@link #REACCION} — Reacció (🛡), 1 per ronda, es juga fora del propi torn.</li>
 *   <li>{@link #PASIVA} — sempre activa, no consumeix res en jugar-se.</li>
 *   <li>{@link #MOVIMIENTO} — Acció de Moviment (👣).</li>
 *   <li>{@link #PREPARACION} — efecte previ a l'escena, no gasta recursos de torn.</li>
 *   <li>{@link #CANALIZACION} — Canalització (⏳): substitueix l'acció principal I la de
 *       moviment del torn; vegeu {@link EstatTorn}.</li>
 *   <li>{@link #CONSUMIBLE} — gasta l'objecte, no un dels quatre recursos de 7.3.</li>
 * </ul>
 */
public enum TipusAccio {

    ACCION,
    ACCION_MENOR,
    REACCION,
    PASIVA,
    MOVIMIENTO,
    PREPARACION,
    CANALIZACION,
    CONSUMIBLE;

    /**
     * Converteix el valor lliure del camp {@code actionType} de la carta a aquest enum.
     *
     * @param actionType valor tal com es guarda a la carta ({@code "accion"},
     *                    {@code "accion_menor"}, {@code "reaccion"}, {@code "pasiva"},
     *                    {@code "movimiento"}, {@code "preparacion"}, {@code "canalizacion"}
     *                    o {@code "consumible"}); {@code null} o buit es tracta com a
     *                    {@link #ACCION} (el valor per defecte als formularis de carta)
     * @return el tipus d'acció equivalent; mai {@code null}
     * @throws IllegalArgumentException si el valor no és cap dels vuit reconeguts
     */
    public static TipusAccio desDeActionType(String actionType) {
        if (actionType == null || actionType.isBlank()) {
            return ACCION;
        }
        return switch (actionType.trim().toLowerCase()) {
            case "accion" -> ACCION;
            case "accion_menor" -> ACCION_MENOR;
            case "reaccion" -> REACCION;
            case "pasiva" -> PASIVA;
            case "movimiento" -> MOVIMIENTO;
            case "preparacion" -> PREPARACION;
            case "canalizacion" -> CANALIZACION;
            case "consumible" -> CONSUMIBLE;
            default -> throw new IllegalArgumentException("actionType desconegut: " + actionType);
        };
    }
}
