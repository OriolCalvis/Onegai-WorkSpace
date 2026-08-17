package cat.dnd.cc.service;

import cat.dnd.cc.eines.EngineConfig;
import cat.dnd.cc.eines.OtrosHandler;
import cat.dnd.cc.eines.OtrosHandlers;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.io.InputStream;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * Motor de càrrega i validació de dotes, invocacions, reaccions i habilitats del catàleg JSON.
 */
public class SistemaHabilidades {
    private static final String HABILIDADES_RESOURCE = "/JSONS/dotes/habilidades.json";
    private static final String LEGACY_HABILIDADES_RESOURCE = "/dotes/habilidades.json";
    private static final Pattern NIVEL_PATTERN = Pattern.compile("NIVEL\\s+(\\d+)");
    private static final Pattern ATRIBUTO_PATTERN = Pattern.compile("(FUE|STR|DES|DEX|CON|INT|SAB|WIS|CAR|CHA)\\s*(\\d{1,2})\\+?");
    private static final Pattern ATRIBUTO_OR_PATTERN = Pattern.compile(
            "(FUE|STR|DES|DEX|CON|INT|SAB|WIS|CAR|CHA)\\s*(\\d{1,2})\\+?\\s+O\\s+"
                    + "(FUE|STR|DES|DEX|CON|INT|SAB|WIS|CAR|CHA)\\s*(\\d{1,2})\\+?"
    );
    private static final List<String> TAGS_REQUISITO = List.of(
            "GURKAZAAL", "CHRONOS", "SOFÍA", "SOFIA", "ENVIDIA", "VIDA", "MUERTE", "EGOS", "EROS"
    );
    private static final Map<String, String> ATR_ALIAS = Map.ofEntries(
            Map.entry("FUE", "fuerza"),
            Map.entry("STR", "fuerza"),
            Map.entry("DES", "destreza"),
            Map.entry("DEX", "destreza"),
            Map.entry("CON", "constitucion"),
            Map.entry("INT", "inteligencia"),
            Map.entry("SAB", "sabiduria"),
            Map.entry("WIS", "sabiduria"),
            Map.entry("CAR", "carisma"),
            Map.entry("CHA", "carisma")
    );

    private final EngineConfig cfg = EngineConfig.load();
    private final Map<String, OtrosHandler> otrosHandlers = Map.of(
            "tag", OtrosHandlers.TAG,
            "capability", OtrosHandlers.CAPABILITY,
            "competency", OtrosHandlers.COMPETENCY,
            "pact", OtrosHandlers.PACT
    );
    private final Map<String, Habilidad> habilidadesMap = new LinkedHashMap<>();
    private final int fuerza;
    private final int destreza;
    private final int constitucion;
    private final int inteligencia;
    private final int sabiduria;
    private final int carisma;
    private final int nivel;
    private final List<String> competencias;
    private final Set<String> etiquetas = new HashSet<>();
    private final Set<String> pactosConocidos = new HashSet<>();

    private Map<String, String> atributosMap = new HashMap<>();
    private Map<String, String> competenciasMap = new HashMap<>();
    private boolean puedeLanzarHechizos;
    private boolean strictOtrosChecks;

    public final List<String> habilidades = new ArrayList<>();
    public final List<String> dotes = new ArrayList<>();
    public final List<String> invocaciones = new ArrayList<>();
    public final List<String> reacciones = new ArrayList<>();
    public final List<String> aEspecialesProficiencies = new ArrayList<>();

    public SistemaHabilidades(int fuerza, int destreza, int constitucion,
                              int inteligencia, int sabiduria, int carisma,
                              int nivel, List<String> competencias) {
        this.fuerza = fuerza;
        this.destreza = destreza;
        this.constitucion = constitucion;
        this.inteligencia = inteligencia;
        this.sabiduria = sabiduria;
        this.carisma = carisma;
        this.nivel = nivel;
        this.competencias = competencias == null ? new ArrayList<>() : new ArrayList<>(competencias);
        cargarHabilidadesDesdeJSON();
    }

    public SistemaHabilidades setPuedeLanzarHechizos(boolean puedeLanzarHechizos) {
        this.puedeLanzarHechizos = puedeLanzarHechizos;
        return this;
    }

    public SistemaHabilidades addEtiqueta(String etiqueta) {
        if (etiqueta != null && !etiqueta.isBlank()) {
            this.etiquetas.add(EngineConfig.normUpper(etiqueta));
        }
        return this;
    }

    public SistemaHabilidades conocerPacto(String nombre) {
        if (nombre != null && !nombre.isBlank()) {
            this.pactosConocidos.add(slug(nombre));
        }
        return this;
    }

    public SistemaHabilidades setStrictOtrosChecks(boolean strictOtrosChecks) {
        this.strictOtrosChecks = strictOtrosChecks;
        return this;
    }

    private void cargarHabilidadesDesdeJSON() {
        ObjectMapper mapper = new ObjectMapper();
        try (InputStream inputStream = openHabilidadesResource()) {
            if (inputStream == null) {
                return;
            }
            Map<String, Object> datos = mapper.readValue(inputStream, new TypeReference<>() { });
            atributosMap = readStringMap(datos.get("atributos"));
            competenciasMap = readStringMap(datos.get("competencias"));
            readMapList(datos.get("dotes")).stream()
                    .map(this::buildFromDote)
                    .filter(Objects::nonNull)
                    .forEach(habilidad -> habilidadesMap.put(habilidad.id, habilidad));
            readMapList(datos.get("invocaciones")).stream()
                    .map(this::buildFromInvocacion)
                    .filter(Objects::nonNull)
                    .forEach(habilidad -> habilidadesMap.put(habilidad.id, habilidad));
            readMapList(datos.get("reacciones")).stream()
                    .map(this::buildFromReaccion)
                    .filter(Objects::nonNull)
                    .forEach(habilidad -> habilidadesMap.put(habilidad.id, habilidad));
            readMapList(datos.get("habilidades")).stream()
                    .map(this::buildFromHechizoLike)
                    .filter(Objects::nonNull)
                    .forEach(habilidad -> habilidadesMap.put(habilidad.id, habilidad));
        } catch (IOException e) {
            habilidadesMap.clear();
        }
    }

    private InputStream openHabilidadesResource() {
        InputStream inputStream = getClass().getResourceAsStream(HABILIDADES_RESOURCE);
        return inputStream == null ? getClass().getResourceAsStream(LEGACY_HABILIDADES_RESOURCE) : inputStream;
    }

    private Habilidad buildFromDote(Map<String, Object> source) {
        String nombre = str(source.get("nombre"));
        if (nombre == null || nombre.isBlank()) {
            return null;
        }
        Habilidad habilidad = new Habilidad();
        habilidad.id = idFromSource(source, nombre);
        habilidad.nombre = nombre;
        habilidad.tipo = "dote";
        habilidad.descripcion = "";
        habilidad.beneficios = readStringList(source.get("beneficios"));
        habilidad.requisitos = parseRequisitosObject(readObjectMap(source.get("requisitos")));
        return habilidad;
    }

    private Habilidad buildFromInvocacion(Map<String, Object> source) {
        String nombre = str(source.get("nombre"));
        if (nombre == null || nombre.isBlank()) {
            return null;
        }
        Habilidad habilidad = new Habilidad();
        habilidad.id = slug("invoc_" + nombre);
        habilidad.nombre = nombre;
        habilidad.tipo = "invocacion";
        habilidad.descripcion = composeDescripcion(source);
        habilidad.beneficios = readStringList(source.get("efectos"));
        habilidad.requisitos = parseRequisitosText(str(source.get("requisitos")));
        return habilidad;
    }

    private Habilidad buildFromReaccion(Map<String, Object> source) {
        String nombre = str(source.get("nombre"));
        if (nombre == null || nombre.isBlank()) {
            return null;
        }
        Habilidad habilidad = new Habilidad();
        habilidad.id = slug("reac_" + nombre);
        habilidad.nombre = nombre;
        habilidad.tipo = "reaccion";
        habilidad.descripcion = "Disparador: " + str(source.get("disparador"))
                + " | Efecto: " + str(source.get("efecto"))
                + " | Limitaciones: " + str(source.get("limitaciones"))
                + " | Recarga: " + str(source.get("recarga"));
        habilidad.beneficios = new ArrayList<>();
        habilidad.requisitos = parseRequisitosText(str(source.get("limitaciones")));
        return habilidad;
    }

    private Habilidad buildFromHechizoLike(Map<String, Object> source) {
        String nombre = str(source.get("nombre"));
        if (nombre == null || nombre.isBlank()) {
            return null;
        }
        Habilidad habilidad = new Habilidad();
        habilidad.id = slug("hab_" + nombre);
        habilidad.nombre = nombre;
        habilidad.tipo = "habilidad";
        habilidad.descripcion = composeDescripcion(source);
        habilidad.beneficios = new ArrayList<>();
        Requisitos requisitos = emptyRequisitos();
        requisitos.nivelMinimo = intOrZero(source.get("nivel"));
        habilidad.requisitos = requisitos;
        return habilidad;
    }

    private Requisitos parseRequisitosObject(Map<String, Object> reqObj) {
        Requisitos requisitos = emptyRequisitos();
        if (reqObj.isEmpty()) {
            return requisitos;
        }
        requisitos.nivelMinimo = intOrZero(reqObj.get("nivel_minimo"));
        requisitos.atributos = normalizeAtributos(readObjectMap(reqObj.get("atributos")));
        requisitos.competencias = readStringList(reqObj.get("competencias"));
        requisitos.otros = readStringList(reqObj.get("otros"));
        return requisitos;
    }

    private Requisitos parseRequisitosText(String text) {
        Requisitos requisitos = emptyRequisitos();
        if (text == null || text.isBlank()) {
            return requisitos;
        }
        String upperText = normalizeUpper(text);
        Matcher nivelMatcher = NIVEL_PATTERN.matcher(upperText);
        if (nivelMatcher.find()) {
            requisitos.nivelMinimo = intOrZero(nivelMatcher.group(1));
        }
        Matcher atributoMatcher = ATRIBUTO_PATTERN.matcher(upperText);
        while (atributoMatcher.find()) {
            String atributo = ATR_ALIAS.getOrDefault(atributoMatcher.group(1), atributoMatcher.group(1));
            int valor = intOrZero(atributoMatcher.group(2));
            requisitos.atributos.put(atributo, Math.max(requisitos.atributos.getOrDefault(atributo, 0), valor));
        }
        for (String tag : TAGS_REQUISITO) {
            if (upperText.contains(tag)) {
                addUnique(requisitos.otros, "SOFIA".equals(tag) ? "SOFÍA" : tag);
            }
        }
        addCompetenciasFromText(upperText, requisitos.competencias);
        if (upperText.contains("CONOCER") && upperText.contains("PACTO DEL COMETA")) {
            addUnique(requisitos.otros, "CONOCER \"PACTO DEL COMETA\"");
        }
        if (upperText.contains("CAPACIDAD DE LANZAR CONJUROS") || upperText.contains("PUEDE_LANZAR_HECHIZOS")) {
            addUnique(requisitos.otros, "CAPACIDAD DE LANZAR CONJUROS");
        }
        return requisitos;
    }

    private void addCompetenciasFromText(String upperText, List<String> requisitosCompetencias) {
        if (upperText.contains("ARCO")) {
            addUnique(requisitosCompetencias, "arcos");
        }
        if (upperText.contains("BALLESTA")) {
            addUnique(requisitosCompetencias, "ballestas");
        }
        if (upperText.contains("ARMAS MARCIALES")) {
            addUnique(requisitosCompetencias, "armas_marciales");
        }
        if (upperText.contains("ARMADURA PESADA")) {
            addUnique(requisitosCompetencias, "armaduras_pesadas");
        }
        if (upperText.contains("ESCUDO")) {
            addUnique(requisitosCompetencias, "escudos");
        }
    }

    private Map<String, Integer> normalizeAtributos(Map<String, Object> rawAttributes) {
        Map<String, Integer> normalized = new HashMap<>();
        rawAttributes.forEach((key, value) -> {
            String canon = cfg.canonAttr(key);
            int intValue = intOrZero(value);
            if (!canon.isBlank() && intValue > 0) {
                normalized.put(canon, intValue);
            }
        });
        return normalized;
    }

    public List<Habilidad> getHabilidadesDisponibles() {
        return habilidadesMap.values().stream()
                .filter(this::cumpleRequisitos)
                .collect(Collectors.toList());
    }

    public void aplicarHabilidad(String idHabilidad, Map<String, Object> opciones) {
        Habilidad habilidad = habilidadesMap.get(idHabilidad);
        if (habilidad == null || !cumpleRequisitos(habilidad)) {
            return;
        }
        habilidad.beneficios.forEach(beneficio -> aplicarBeneficio(beneficio, opciones));
        registrarHabilidadAplicada(habilidad);
    }

    public void agregarHabilidad(Habilidad habilidad) {
        if (habilidad == null || !cumpleRequisitos(habilidad)) {
            return;
        }
        registrarHabilidadAplicada(habilidad);
        aplicarBeneficios(habilidad);
    }

    public boolean cumpleRequisitos(Habilidad habilidad) {
        if (habilidad == null || habilidad.requisitos == null) {
            return true;
        }
        if (habilidad.requisitos.nivelMinimo > 0 && this.nivel < habilidad.requisitos.nivelMinimo) {
            return false;
        }
        if (!cumpleAtributos(habilidad.requisitos)) {
            return false;
        }
        for (String competencia : habilidad.requisitos.competencias) {
            if (!hasCompetenciaAlias(competencia)) {
                return false;
            }
        }
        for (String otroRequisito : habilidad.requisitos.otros) {
            if (!cumpleOtroRequisito(otroRequisito)) {
                return false;
            }
        }
        return true;
    }


    private boolean cumpleAtributos(Requisitos requisitos) {
        Set<String> atributosFallidos = new HashSet<>();
        for (Map.Entry<String, Integer> entry : requisitos.atributos.entrySet()) {
            if (obtenerValorAtributo(entry.getKey()) < entry.getValue()) {
                atributosFallidos.add(entry.getKey());
            }
        }
        if (atributosFallidos.isEmpty()) {
            return true;
        }
        Set<String> atributosSinCubrir = new HashSet<>(atributosFallidos);
        for (String otro : requisitos.otros) {
            Matcher matcher = ATRIBUTO_OR_PATTERN.matcher(normalizeUpper(otro));
            while (matcher.find()) {
                String atributoIzquierda = ATR_ALIAS.getOrDefault(matcher.group(1), matcher.group(1));
                int valorIzquierda = intOrZero(matcher.group(2));
                String atributoDerecha = ATR_ALIAS.getOrDefault(matcher.group(3), matcher.group(3));
                int valorDerecha = intOrZero(matcher.group(4));
                boolean cumpleAlternativa = obtenerValorAtributo(atributoIzquierda) >= valorIzquierda
                        || obtenerValorAtributo(atributoDerecha) >= valorDerecha;
                if (cumpleAlternativa) {
                    atributosSinCubrir.remove(atributoIzquierda);
                    atributosSinCubrir.remove(atributoDerecha);
                }
            }
        }
        return atributosSinCubrir.isEmpty();
    }

    private boolean cumpleOtroRequisito(String token) {
        if (token == null || token.isBlank()) {
            return true;
        }
        Contexto contexto = buildContexto();
        String normalized = EngineConfig.norm(token).toLowerCase(Locale.ROOT);
        for (Map.Entry<String, String> entry : cfg.otrosPrefixHandlers.entrySet()) {
            String prefix = entry.getKey().toLowerCase(Locale.ROOT);
            if (normalized.startsWith(prefix)) {
                OtrosHandler handler = otrosHandlers.get(entry.getValue());
                return handler == null ? !strictOtrosChecks : handler.test(token, contexto, cfg);
            }
        }
        if (normalized.contains("opcional")) {
            return true;
        }
        if (ATRIBUTO_OR_PATTERN.matcher(normalizeUpper(token)).find() || ATRIBUTO_PATTERN.matcher(normalizeUpper(token)).find()) {
            return true;
        }
        if (cfg.matchTag(token)) {
            return contexto.tags.contains(EngineConfig.normUpper(token));
        }
        if (cfg.matchCompetency(token, contexto.competencias)) {
            return true;
        }
        if (normalized.contains("capacidad de lanzar conjuros") || normalized.contains("puede_lanzar_hechizos")) {
            return contexto.capabilities.getOrDefault("spellcasting", false);
        }
        if (normalized.contains("pacto del cometa")) {
            return contexto.pactos.contains(slug("Pacto del Cometa"));
        }
        return !strictOtrosChecks;
    }

    private Contexto buildContexto() {
        Contexto contexto = new Contexto();
        contexto.fuerza = fuerza;
        contexto.destreza = destreza;
        contexto.constitucion = constitucion;
        contexto.inteligencia = inteligencia;
        contexto.sabiduria = sabiduria;
        contexto.carisma = carisma;
        contexto.nivel = nivel;
        contexto.competencias.addAll(this.competencias);
        contexto.pactos.addAll(this.pactosConocidos);
        contexto.capabilities.put("spellcasting", this.puedeLanzarHechizos);
        contexto.tags.addAll(this.etiquetas);
        return contexto;
    }

    private void aplicarBeneficio(String beneficio, Map<String, Object> opciones) {
        if (beneficio == null || beneficio.isBlank()) {
            return;
        }
        String[] partes = beneficio.split("\\|");
        String tipoBeneficio = partes[0];
        switch (tipoBeneficio) {
            case "atributo" -> {
                if (partes.length >= 3) {
                    aumentarAtributo(partes[1], intOrZero(partes[2]));
                }
            }
            case "ventaja" -> {
                if (partes.length >= 3) {
                    agregarVentaja(partes[1], partes[2]);
                }
            }
            case "competencia" -> {
                if (partes.length >= 2) {
                    agregarCompetencia(partes[1]);
                }
            }
            case "elegir" -> aplicarEleccion(partes, opciones);
            default -> {
                // Beneficio en texto libre: se conserva sin ejecutar una acción mecánica.
            }
        }
    }

    private void aplicarEleccion(String[] partes, Map<String, Object> opciones) {
        if (partes.length < 4 || opciones == null) {
            return;
        }
        Object opcionElegida = opciones.get(partes[1]);
        if (opcionElegida != null) {
            aplicarBeneficio(partes[1] + "|" + opcionElegida + "|" + partes[2], Collections.emptyMap());
        }
    }

    private void registrarHabilidadAplicada(Habilidad habilidad) {
        switch (habilidad.tipo) {
            case "dote" -> dotes.add(habilidad.id);
            case "invocacion" -> invocaciones.add(habilidad.id);
            case "reaccion" -> reacciones.add(habilidad.id);
            case "habilidad_especial", "especial" -> aEspecialesProficiencies.add(habilidad.id);
            default -> habilidades.add(habilidad.id);
        }
    }

    private void aplicarBeneficios(Habilidad habilidad) {
        habilidad.beneficios.forEach(beneficio -> aplicarBeneficio(beneficio, Collections.emptyMap()));
    }

    private int obtenerValorAtributo(String atributo) {
        return switch (atributo) {
            case "fuerza" -> fuerza;
            case "destreza" -> destreza;
            case "constitucion" -> constitucion;
            case "inteligencia" -> inteligencia;
            case "sabiduria" -> sabiduria;
            case "carisma" -> carisma;
            default -> 0;
        };
    }

    private boolean hasCompetenciaAlias(String alias) {
        return cfg.matchCompetency(alias, this.competencias);
    }

    private void aumentarAtributo(String atributo, int cantidad) {
        // Reservado para integrar mutaciones reales sobre la ficha de personaje.
    }

    private void agregarVentaja(String tipo, String objetivo) {
        // Reservado para integrar ventajas temporales en la ficha de personaje.
    }

    private void agregarCompetencia(String competencia) {
        if (competencia != null && !competencia.isBlank() && !competencias.contains(competencia)) {
            competencias.add(competencia);
        }
    }

    private String composeDescripcion(Map<String, Object> source) {
        List<String> parts = new ArrayList<>();
        for (String key : List.of("nivel", "tipo", "categoria", "accion", "tiempo_lanzamiento", "alcance", "duracion", "descripcion", "efecto", "uso", "escuela")) {
            if (source.containsKey(key)) {
                parts.add(key + ": " + source.get(key));
            }
        }
        return String.join(" | ", parts);
    }

    private String idFromSource(Map<String, Object> source, String nombre) {
        String id = str(source.get("id"));
        return id == null || id.isBlank() ? slug(nombre) : id;
    }

    private static Requisitos emptyRequisitos() {
        Requisitos requisitos = new Requisitos();
        requisitos.nivelMinimo = 0;
        requisitos.atributos = new HashMap<>();
        requisitos.competencias = new ArrayList<>();
        requisitos.otros = new ArrayList<>();
        return requisitos;
    }

    private static String str(Object value) {
        return value == null ? null : String.valueOf(value);
    }

    private static int intOrZero(Object value) {
        if (value instanceof Number number) {
            return number.intValue();
        }
        if (value == null) {
            return 0;
        }
        try {
            return Integer.parseInt(String.valueOf(value));
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    private static String slug(String value) {
        if (value == null) {
            return null;
        }
        String normalized = Normalizer.normalize(value, Normalizer.Form.NFD).replaceAll("\\p{M}", "");
        return normalized.toLowerCase(Locale.ROOT)
                .replaceAll("[^a-z0-9]+", "_")
                .replaceAll("^_+|_+$", "");
    }

    private static String normalizeUpper(String value) {
        if (value == null) {
            return "";
        }
        return Normalizer.normalize(value, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "")
                .toUpperCase(Locale.ROOT)
                .trim();
    }

    private static void addUnique(List<String> target, String value) {
        if (value != null && !value.isBlank() && !target.contains(value)) {
            target.add(value);
        }
    }

    @SuppressWarnings("unchecked")
    private static Map<String, Object> readObjectMap(Object value) {
        if (!(value instanceof Map<?, ?> rawMap)) {
            return new HashMap<>();
        }
        Map<String, Object> result = new HashMap<>();
        rawMap.forEach((key, mapValue) -> {
            if (key != null) {
                result.put(String.valueOf(key), mapValue);
            }
        });
        return result;
    }

    private static Map<String, String> readStringMap(Object value) {
        Map<String, String> result = new HashMap<>();
        readObjectMap(value).forEach((key, mapValue) -> result.put(key, str(mapValue)));
        return result;
    }

    private static List<Map<String, Object>> readMapList(Object value) {
        if (!(value instanceof List<?> rawList)) {
            return new ArrayList<>();
        }
        return rawList.stream()
                .filter(Map.class::isInstance)
                .map(SistemaHabilidades::readObjectMap)
                .collect(Collectors.toCollection(ArrayList::new));
    }

    private static List<String> readStringList(Object value) {
        if (!(value instanceof List<?> rawList)) {
            return new ArrayList<>();
        }
        return rawList.stream()
                .filter(Objects::nonNull)
                .map(String::valueOf)
                .filter(item -> !item.isBlank())
                .collect(Collectors.toCollection(ArrayList::new));
    }

    public Map<String, Habilidad> getHabilidadesMap() {
        return Collections.unmodifiableMap(habilidadesMap);
    }

    public Map<String, String> getAtributosMap() {
        return Collections.unmodifiableMap(atributosMap);
    }

    public Map<String, String> getCompetenciasMap() {
        return Collections.unmodifiableMap(competenciasMap);
    }

    public List<String> getCompetencias() {
        return Collections.unmodifiableList(competencias);
    }

    public static class Habilidad {
        public String id;
        public String nombre;
        public String tipo;
        public Requisitos requisitos;
        public List<String> beneficios = new ArrayList<>();
        public String descripcion;
    }

    public static class Requisitos {
        public int nivelMinimo;
        public Map<String, Integer> atributos = new HashMap<>();
        public List<String> competencias = new ArrayList<>();
        public List<String> otros = new ArrayList<>();
    }

    public static class Contexto {
        public int fuerza;
        public int destreza;
        public int constitucion;
        public int inteligencia;
        public int sabiduria;
        public int carisma;
        public int nivel;
        public Set<String> competencias = new HashSet<>();
        public Set<String> pactos = new HashSet<>();
        public Map<String, Boolean> capabilities = new HashMap<>();
        public Set<String> tags = new HashSet<>();
    }
}
