package cat.dnd.cc.combat.motor;

/**
 * Les piles on pot viure una carta jugable durant una sessió (regla 6 del GDD).
 *
 * <p>Nota de disseny: la documentació d'UI (fitxa de personatge, Zona 4) mostra una tercera
 * caixa anomenada "Exiliadas" a més de Descans Curt i Descans Llarg, però la secció 6 del
 * GDD —la font de veritat mecànica— només defineix aquestes tres piles. "Exiliadas" no té
 * cap suport mecànic ni al GDD ni al codi: caldrà que algú decideixi si és sinònim de
 * Descans Llarg o un concepte nou abans d'afegir-lo aquí.</p>
 */
public enum Pila {
    ACTIVA,
    DESCANSO_CORTO,
    DESCANSO_LARGO
}
