package cat.dnd.cc.model;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Tipus de carta d'història dins d'una aventura per actes (GDD §20):
 * <ul>
 *   <li>{@code BASE} — entra sempre a la baralla, sense requisit.</li>
 *   <li>{@code CONDICIONAL} — entra només si es compleix el seu requisit de fitxa.</li>
 *   <li>{@code INYECTADA} — és base a efectes de filtre, però el text consulta fitxes en robar-la.</li>
 *   <li>{@code CADENA} — ordre fix; no es barreja.</li>
 *   <li>{@code BALANCE_FINAL} — es juga sempre l'última; compta totes les fitxes.</li>
 * </ul>
 * Se serialitza i es llegeix en minúscules ({@code "base"}, {@code "balance_final"}...) per
 * encaixar amb l'esquema de generació de {@code Plantilla_Prompt_Contenido.md §18b}.
 */
public enum CardKind {
    BASE, CONDICIONAL, INYECTADA, CADENA, BALANCE_FINAL;

    @JsonValue
    public String json() {
        return name().toLowerCase();
    }

    @JsonCreator
    public static CardKind desde(String valor) {
        if (valor == null || valor.isBlank()) {
            return BASE;
        }
        return valueOf(valor.trim().toUpperCase());
    }
}
