package cat.dnd.cc.model;

import com.fasterxml.jackson.annotation.JsonAnyGetter;
import com.fasterxml.jackson.annotation.JsonAnySetter;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Base de las cartas que pueden traer campos que el editor todavía no conoce.
 * <p>
 * CARTAS-CLASES-01 (08/10/2026): las clases que suben del motor traen campos que estos
 * modelos no tenían — los ganchos de cada carta ({@code effects[].mechanicHook}), con qué
 * stat tira ({@code castingStat}), a quién apunta ({@code target}), la rareza, la familia
 * de la clase ({@code especialista}, {@code propia}), su set de Tier 2 ({@code classSet})...
 * Antes, Jackson los ignoraba al leer y el siguiente guardado desde el editor los borraba
 * del fichero sin avisar. Aquí se quedan: lo que el modelo no conoce entra en
 * {@link #getExtras()} al leer y vuelve a salir igual al escribir.
 * <p>
 * Los formularios de edición crean la carta desde cero; por eso los controladores copian
 * los extras de la carta guardada antes de escribir la editada ({@link #copiarExtrasDe}).
 */
public abstract class CartaConExtras {

    private final Map<String, Object> extras = new LinkedHashMap<>();

    /** Los campos del JSON que este modelo no conoce, en el orden en que llegaron. */
    @JsonAnyGetter
    public Map<String, Object> getExtras() {
        return extras;
    }

    @JsonAnySetter
    public void setExtra(String nombre, Object valor) {
        extras.put(nombre, valor);
    }

    /** Copia los extras de otra carta sin pisar los que esta ya tenga. */
    public void copiarExtrasDe(CartaConExtras otra) {
        if (otra != null) {
            otra.extras.forEach(extras::putIfAbsent);
        }
    }

    /** Un extra de texto, o null si no está o no es texto. */
    public String extraTexto(String clave) {
        Object valor = extras.get(clave);
        return valor instanceof String texto && !texto.isBlank() ? texto : null;
    }

    /** Un extra que es un objeto JSON, o un mapa vacío. */
    @SuppressWarnings("unchecked")
    public Map<String, Object> extraMapa(String clave) {
        Object valor = extras.get(clave);
        return valor instanceof Map<?, ?> mapa ? (Map<String, Object>) mapa : Collections.emptyMap();
    }

    /** Un extra que es una lista JSON, o una lista vacía. */
    @SuppressWarnings("unchecked")
    public List<Object> extraLista(String clave) {
        Object valor = extras.get(clave);
        return valor instanceof List<?> lista ? (List<Object>) lista : Collections.emptyList();
    }

    /** Los {@code effects[].mechanicHook} de la carta (vacío si no tiene). */
    public List<String> ganchos() {
        return extraLista("effects").stream()
                .filter(e -> e instanceof Map<?, ?>)
                .map(e -> ((Map<?, ?>) e).get("mechanicHook"))
                .filter(h -> h instanceof String s && !s.isBlank())
                .map(String.class::cast)
                .toList();
    }
}
