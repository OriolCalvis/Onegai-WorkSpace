package cat.dnd.cc.eines;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.io.InputStream;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

/**
 * Configuració del motor de requisits de dotes i habilitats.
 */
public class EngineConfig {
    private static final String CONFIG_RESOURCE = "/JSONS/dotes/config.json";

    public final Map<String, String> attributeAliases = new HashMap<>();
    public final Set<String> tags = new HashSet<>();
    public final Map<String, List<String>> competencyAliases = new HashMap<>();
    public final Map<String, String> otrosPrefixHandlers = new HashMap<>();

    public static EngineConfig load() {
        EngineConfig config = defaults();
        ObjectMapper mapper = new ObjectMapper();
        try (InputStream inputStream = EngineConfig.class.getResourceAsStream(CONFIG_RESOURCE)) {
            if (inputStream == null) {
                return config;
            }
            Map<String, Object> raw = mapper.readValue(inputStream, new TypeReference<>() { });
            config.loadAttributeAliases(raw.get("attribute_aliases"));
            config.loadTags(raw.get("tags"));
            config.loadCompetencyAliases(raw.get("competency_aliases"));
            config.loadOtrosPrefixHandlers(raw.get("otros_prefix_handlers"));
            return config;
        } catch (IOException e) {
            return config;
        }
    }

    private static EngineConfig defaults() {
        EngineConfig config = new EngineConfig();
        config.attributeAliases.put("FUE", "fuerza");
        config.attributeAliases.put("STR", "fuerza");
        config.attributeAliases.put("DES", "destreza");
        config.attributeAliases.put("DEX", "destreza");
        config.attributeAliases.put("CON", "constitucion");
        config.attributeAliases.put("INT", "inteligencia");
        config.attributeAliases.put("SAB", "sabiduria");
        config.attributeAliases.put("WIS", "sabiduria");
        config.attributeAliases.put("CAR", "carisma");
        config.attributeAliases.put("CHA", "carisma");
        config.tags.addAll(List.of("GURKAZAAL", "CHRONOS", "SOFIA", "ENVIDIA", "VIDA", "MUERTE", "EGOS", "EROS"));
        config.competencyAliases.put("escudos", List.of("escudos", "escudo"));
        config.competencyAliases.put("armas_marciales", List.of("armas_marciales", "armas marciales"));
        config.competencyAliases.put("armaduras_pesadas", List.of("armaduras_pesadas", "armadura pesada"));
        config.competencyAliases.put("arcos", List.of("arcos", "arco", "arco largo", "arco corto"));
        config.competencyAliases.put("ballestas", List.of("ballestas", "ballesta"));
        config.otrosPrefixHandlers.put("tag:", "tag");
        config.otrosPrefixHandlers.put("cap:", "capability");
        config.otrosPrefixHandlers.put("prof:", "competency");
        config.otrosPrefixHandlers.put("pact:", "pact");
        return config;
    }

    @SuppressWarnings("unchecked")
    private void loadAttributeAliases(Object rawAliases) {
        if (!(rawAliases instanceof Map<?, ?> aliases)) {
            return;
        }
        aliases.forEach((key, value) -> {
            if (key != null && value != null) {
                attributeAliases.put(normUpper(String.valueOf(key)), norm(String.valueOf(value)).toLowerCase(Locale.ROOT));
            }
        });
    }

    private void loadTags(Object rawTags) {
        if (!(rawTags instanceof List<?> rawList)) {
            return;
        }
        tags.clear();
        rawList.stream()
                .filter(item -> item != null && !String.valueOf(item).isBlank())
                .map(item -> normUpper(String.valueOf(item)))
                .forEach(tags::add);
    }

    private void loadCompetencyAliases(Object rawAliases) {
        if (!(rawAliases instanceof Map<?, ?> aliases)) {
            return;
        }
        competencyAliases.clear();
        aliases.forEach((key, value) -> {
            if (key == null) {
                return;
            }
            List<String> values = new ArrayList<>();
            if (value instanceof List<?> list) {
                list.stream()
                        .filter(item -> item != null && !String.valueOf(item).isBlank())
                        .map(item -> normKey(String.valueOf(item)))
                        .forEach(values::add);
            }
            values.add(normKey(String.valueOf(key)));
            competencyAliases.put(normKey(String.valueOf(key)), values);
        });
    }

    private void loadOtrosPrefixHandlers(Object rawHandlers) {
        if (!(rawHandlers instanceof Map<?, ?> handlers)) {
            return;
        }
        otrosPrefixHandlers.clear();
        handlers.forEach((key, value) -> {
            if (key != null && value != null) {
                otrosPrefixHandlers.put(norm(String.valueOf(key)).toLowerCase(Locale.ROOT), normKey(String.valueOf(value)));
            }
        });
    }

    public String canonAttr(String rawAttribute) {
        if (rawAttribute == null || rawAttribute.isBlank()) {
            return "";
        }
        String upper = normUpper(rawAttribute);
        String direct = attributeAliases.get(upper);
        if (direct != null) {
            return direct;
        }
        String normalized = norm(rawAttribute).toLowerCase(Locale.ROOT);
        return attributeAliases.containsValue(normalized) ? normalized : normalized.replace(' ', '_');
    }

    public boolean matchTag(String token) {
        return tags.contains(normUpper(token));
    }

    public boolean matchCompetency(String alias, Iterable<String> availableCompetencies) {
        if (alias == null || availableCompetencies == null) {
            return false;
        }
        Set<String> expected = expandedCompetencies(alias);
        for (String competency : availableCompetencies) {
            if (expected.contains(normKey(competency))) {
                return true;
            }
        }
        return false;
    }

    public Set<String> expandedCompetencies(String alias) {
        Set<String> expected = new HashSet<>();
        String key = normKey(alias);
        expected.add(key);
        competencyAliases.forEach((canonical, aliases) -> {
            if (canonical.equals(key) || aliases.contains(key)) {
                expected.add(canonical);
                expected.addAll(aliases);
            }
        });
        return expected;
    }

    public static String norm(String value) {
        if (value == null) {
            return "";
        }
        return Normalizer.normalize(value, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "")
                .trim();
    }

    public static String normUpper(String value) {
        return norm(value).toUpperCase(Locale.ROOT);
    }

    public static String normKey(String value) {
        return norm(value)
                .toLowerCase(Locale.ROOT)
                .replaceAll("[^a-z0-9]+", "_")
                .replaceAll("^_+|_+$", "");
    }
}
