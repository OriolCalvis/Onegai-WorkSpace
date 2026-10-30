package cat.dnd.cc.eines;

import java.util.Locale;

public final class OtrosHandlers {
    public static final OtrosHandler TAG = (token, contexto, config) ->
            contexto.tags.contains(EngineConfig.normUpper(valueAfterPrefix(token)));

    public static final OtrosHandler CAPABILITY = (token, contexto, config) ->
            contexto.capabilities.getOrDefault(EngineConfig.normKey(valueAfterPrefix(token)), false);

    public static final OtrosHandler COMPETENCY = (token, contexto, config) ->
            config.matchCompetency(valueAfterPrefix(token), contexto.competencias);

    public static final OtrosHandler PACT = (token, contexto, config) ->
            contexto.pactos.contains(EngineConfig.normKey(valueAfterPrefix(token)));

    private OtrosHandlers() {
    }

    private static String valueAfterPrefix(String token) {
        if (token == null) {
            return "";
        }
        int separator = token.indexOf(':');
        String value = separator >= 0 ? token.substring(separator + 1) : token;
        return value.trim().toLowerCase(Locale.ROOT).isBlank() ? "" : value.trim();
    }
}
