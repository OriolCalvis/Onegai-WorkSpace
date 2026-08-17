package cat.dnd.cc.combat.motor;

/**
 * Carga del Destino (regla 7.5.4 del GDD): en lloc de morir definitivament, el jugador pot
 * acceptar una cicatriu permanent i perdre 2 punts d'una estadística a l'atzar, a canvi de
 * sobreviure. Es pot fer servir <b>una única vegada per personatge</b>, mai més.
 *
 * <p>Aquesta classe només fa de guàrdia d'aquesta limitació d'ús únic: no decideix quina
 * estadística perd els 2 punts ni escriu la cicatriu narrativa, que és responsabilitat de
 * qui gestioni la fitxa.</p>
 */
public final class CargaDelDesti {

    private boolean usada = false;

    /**
     * @return {@code true} si aquest personatge encara pot fer servir la Carga del Destino
     */
    public boolean disponible() {
        return !usada;
    }

    /**
     * Consumeix la Carga del Destino d'aquest personatge.
     *
     * @throws IllegalStateException si ja s'havia fet servir abans
     */
    public void usar() {
        if (usada) {
            throw new IllegalStateException("La Carga del Destino ja s'ha fet servir per aquest personatge");
        }
        usada = true;
    }
}
