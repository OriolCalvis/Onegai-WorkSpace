package cat.dnd.cc.model;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Estat d'una carta ja resolta a la taula (GDD §20): la fitxa verda (completada/èxit)
 * o roja (ignorada/fracàs) que es col·loca damunt de la carta i decideix el filtratge
 * dels actes posteriors. Es llegeix i s'escriu en minúscules ({@code "verde"} / {@code "roja"}).
 */
public enum FichaEstado {
    VERDE, ROJA;

    @JsonValue
    public String json() {
        return name().toLowerCase();
    }

    @JsonCreator
    public static FichaEstado desde(String valor) {
        return valor == null ? null : valueOf(valor.trim().toUpperCase());
    }
}
