package cat.dnd.cc.eines;

import java.util.ArrayList;
import java.util.List;

/**
 * Convierte entre una lista de cadenas (List&lt;String&gt;) y su representación en un único
 * campo de texto separado por comas, tal y como se captura en los formularios de cartas
 * (equipo inicial, tags, habilidades...). Mantiene los formularios simples sin necesitar
 * campos dinámicos por JavaScript.
 */
public final class ListaTexto {

    private ListaTexto() {
    }

    public static List<String> splitCsv(String texto) {
        List<String> resultado = new ArrayList<>();
        if (texto == null || texto.isBlank()) {
            return resultado;
        }
        for (String parte : texto.split(",")) {
            String limpio = parte.trim();
            if (!limpio.isEmpty()) {
                resultado.add(limpio);
            }
        }
        return resultado;
    }

    public static String joinCsv(List<String> lista) {
        if (lista == null || lista.isEmpty()) {
            return "";
        }
        return String.join(", ", lista);
    }

    /**
     * Convierte un campo de texto tipo "Mordisco: 1d6 veneno; Zarpazo: daño físico" en una lista
     * de pares [nombre, valor], para editar en un único textarea listas de sub-objetos simples
     * (p. ej. los ataques de una carta de Invocación) sin necesitar campos dinámicos por JavaScript.
     */
    public static List<String[]> splitPares(String texto) {
        List<String[]> resultado = new ArrayList<>();
        if (texto == null || texto.isBlank()) {
            return resultado;
        }
        for (String parte : texto.split(";")) {
            String limpio = parte.trim();
            if (limpio.isEmpty()) {
                continue;
            }
            String[] kv = limpio.split(":", 2);
            resultado.add(new String[]{kv[0].trim(), kv.length > 1 ? kv[1].trim() : ""});
        }
        return resultado;
    }

    public static String joinPares(List<String[]> pares) {
        if (pares == null || pares.isEmpty()) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (String[] par : pares) {
            if (sb.length() > 0) {
                sb.append("; ");
            }
            sb.append(par[0]).append(": ").append(par[1]);
        }
        return sb.toString();
    }
}
