package cat.dnd.cc.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.context.event.ApplicationReadyEvent;
import org.springframework.context.event.EventListener;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.Instant;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Stream;

/**
 * La "run de prueba" del proyecto: al arrancar (y bajo demanda desde /diagnostico)
 * recorre TODO el catálogo y verifica que la información está bien guardada y que
 * ninguna referencia apunta al vacío — clases sin pasiva real, invocaciones sin
 * hechizo, historias con antagonista inexistente, aventuras con ids huérfanos...
 *
 * Cada problema se registra como WARN (error no crítico: la app funciona, pero ese
 * dato está roto y hay que saberlo) con su porqué exacto. El resumen queda en INFO
 * y el último informe se guarda en memoria para el panel web.
 */
@Service
public class DiagnosticoService {

    private static final Logger log = LoggerFactory.getLogger(DiagnosticoService.class);
    private static final int MAX_PROBLEMAS_EN_LOG = 50;

    /** Resultado de una pasada completa de integridad. */
    public record Informe(Instant fecha, long duracionMs, Map<String, Integer> conteos,
                          List<String> problemas, int totalReferenciasRevisadas) {
        public boolean sano() { return problemas.isEmpty(); }
    }

    private final ObjectMapper objectMapper;
    private volatile Informe ultimoInforme;

    public DiagnosticoService(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
    }

    @EventListener(ApplicationReadyEvent.class)
    public void alArrancar() {
        log.info("Arranque completo — lanzando chequeo de integridad del catálogo...");
        Informe informe = ejecutar();
        if (informe.sano()) {
            log.info("INTEGRIDAD OK: {} cartas en {} catálogos, {} referencias revisadas, 0 problemas ({} ms)",
                    informe.conteos().values().stream().mapToInt(Integer::intValue).sum(),
                    informe.conteos().size(), informe.totalReferenciasRevisadas(), informe.duracionMs());
        } else {
            log.warn("INTEGRIDAD CON AVISOS: {} problemas encontrados — detalle a continuación y en /diagnostico",
                    informe.problemas().size());
            informe.problemas().stream().limit(MAX_PROBLEMAS_EN_LOG)
                    .forEach(p -> log.warn("  · {}", p));
            if (informe.problemas().size() > MAX_PROBLEMAS_EN_LOG) {
                log.warn("  · ... y {} más (ver /diagnostico)", informe.problemas().size() - MAX_PROBLEMAS_EN_LOG);
            }
        }
    }

    public Informe ultimo() {
        return ultimoInforme;
    }

    public synchronized Informe ejecutar() {
        long inicio = System.nanoTime();
        Map<String, Set<String>> ids = new LinkedHashMap<>();
        Map<String, List<JsonNode>> cartas = new LinkedHashMap<>();
        Map<String, Integer> conteos = new LinkedHashMap<>();
        List<String> problemas = new ArrayList<>();
        int[] referencias = {0};

        // 1) indexar todos los catálogos (id → existe)
        Map<String, Path> carpetas = new LinkedHashMap<>();
        try (Stream<Path> tipos = Files.list(Paths.get("data", "cartas"))) {
            tipos.filter(Files::isDirectory).sorted()
                    .forEach(p -> carpetas.put(p.getFileName().toString(), p));
        } catch (IOException e) {
            problemas.add("No se pudo listar data/cartas: " + e.getMessage());
        }
        carpetas.put("historias", Paths.get("data", "historias"));
        carpetas.put("npcs", Paths.get("data", "npcs"));
        carpetas.put("loot", Paths.get("data", "loot"));
        carpetas.put("aventuras", Paths.get("data", "aventuras"));
        carpetas.put("personatges", Paths.get("data", "personatges"));

        for (Map.Entry<String, Path> entrada : carpetas.entrySet()) {
            Set<String> setIds = new HashSet<>();
            List<JsonNode> lista = new ArrayList<>();
            if (Files.isDirectory(entrada.getValue())) {
                try (Stream<Path> ficheros = Files.list(entrada.getValue())) {
                    ficheros.filter(f -> f.toString().endsWith(".json")).sorted().forEach(f -> {
                        try {
                            JsonNode nodo = objectMapper.readTree(f.toFile());
                            String id = nodo.path("id").asText(f.getFileName().toString().replace(".json", ""));
                            if (!setIds.add(id)) {
                                problemas.add(entrada.getKey() + ": id duplicado '" + id + "'");
                            }
                            lista.add(nodo);
                        } catch (IOException e) {
                            problemas.add(entrada.getKey() + ": fichero ilegible " + f.getFileName()
                                    + " — " + e.getMessage());
                        }
                    });
                } catch (IOException e) {
                    problemas.add(entrada.getKey() + ": carpeta ilegible — " + e.getMessage());
                }
            }
            ids.put(entrada.getKey(), setIds);
            cartas.put(entrada.getKey(), lista);
            conteos.put(entrada.getKey(), setIds.size());
        }

        // 2) referencias cruzadas — cada comprobación dice QUÉ está roto y POR QUÉ importa
        var comprobar = new Object() {
            void ref(String origen, JsonNode carta, String campo, String valor, String destino) {
                if (valor == null || valor.isBlank()) {
                    return;
                }
                referencias[0]++;
                if (!ids.getOrDefault(destino, Set.of()).contains(valor)) {
                    problemas.add(origen + " '" + carta.path("id").asText() + "': " + campo
                            + " → '" + valor + "' no existe en " + destino);
                }
            }

            void lista(String origen, JsonNode carta, String campo, String destino) {
                for (JsonNode v : carta.path(campo)) {
                    ref(origen, carta, campo, v.asText(), destino);
                }
            }
        };

        for (JsonNode c : cartas.getOrDefault("clases", List.of())) {
            JsonNode inicio_ = withId(c.path("startingCards"), c);
            comprobar.lista("clase", inicio_, "passives", "pasivas");
            comprobar.lista("clase", inicio_, "skills", "habilidades");
            comprobar.lista("clase", inicio_, "learnableSkills", "habilidades");
            comprobar.lista("clase", c, "startingEquipment", "armas");
        }
        for (JsonNode c : cartas.getOrDefault("invocaciones", List.of())) {
            String origen = c.path("summonedBy").asText();
            referencias[0]++;
            boolean existe = ids.getOrDefault("habilidades", Set.of()).contains(origen)
                    || ids.getOrDefault("hechizos", Set.of()).contains(origen);
            if (!existe) {
                problemas.add("invocación '" + c.path("id").asText() + "': summonedBy → '"
                        + origen + "' no existe ni en habilidades ni en hechizos");
            }
        }
        for (JsonNode c : cartas.getOrDefault("deidades", List.of())) {
            comprobar.lista("deidad", c, "grantedSpells", "hechizos");
        }
        for (JsonNode c : cartas.getOrDefault("enemigos", List.of())) {
            comprobar.lista("enemigo", c, "conditionsInflicted", "condiciones");
        }
        for (JsonNode c : cartas.getOrDefault("historias", List.of())) {
            comprobar.ref("historia", c, "antagonist", c.path("antagonist").asText(), "enemigos");
            comprobar.ref("historia", c, "reward", c.path("reward").asText(), "loot");
            comprobar.lista("historia", c, "months", "transfondos");
        }
        for (JsonNode c : cartas.getOrDefault("npcs", List.of())) {
            comprobar.ref("npc", c, "secretHook", c.path("secretHook").asText(), "historias");
            comprobar.ref("npc", c, "month", c.path("month").asText(), "transfondos");
        }
        for (JsonNode c : cartas.getOrDefault("aventuras", List.of())) {
            comprobar.lista("aventura", c, "historiaIds", "historias");
            comprobar.lista("aventura", c, "npcIds", "npcs");
            comprobar.lista("aventura", c, "villanoIds", "enemigos");
            comprobar.lista("aventura", c, "enemigoIds", "enemigos");
            comprobar.lista("aventura", c, "lootIds", "loot");
            comprobar.lista("aventura", c, "trampaIds", "trampas");
        }
        for (JsonNode c : cartas.getOrDefault("personatges", List.of())) {
            comprobar.ref("personaje", c, "claseId", c.path("claseId").asText(), "clases");
            comprobar.ref("personaje", c, "razaId", c.path("razaId").asText(), "razas");
            comprobar.ref("personaje", c, "transfonsId", c.path("transfonsId").asText(), "transfondos");
            comprobar.lista("personaje", c, "habilidadIds", "habilidades");
            comprobar.lista("personaje", c, "equipoIds", "armas");
            comprobar.lista("personaje", c, "hechizoIds", "hechizos");
        }

        long ms = (System.nanoTime() - inicio) / 1_000_000;
        ultimoInforme = new Informe(Instant.now(), ms, conteos, problemas, referencias[0]);
        return ultimoInforme;
    }

    /** El nodo startingCards no lleva id propio: le prestamos el de su clase para los mensajes. */
    private JsonNode withId(JsonNode nodo, JsonNode dueno) {
        if (nodo instanceof com.fasterxml.jackson.databind.node.ObjectNode obj && !obj.has("id")) {
            obj.put("id", dueno.path("id").asText());
        }
        return nodo;
    }

    /** Datos de entorno para el panel: memoria y sesión JVM. */
    public Map<String, String> entorno() {
        Runtime rt = Runtime.getRuntime();
        Map<String, String> datos = new HashMap<>();
        datos.put("memoriaUsadaMb", String.valueOf((rt.totalMemory() - rt.freeMemory()) / (1024 * 1024)));
        datos.put("memoriaMaxMb", String.valueOf(rt.maxMemory() / (1024 * 1024)));
        datos.put("procesadores", String.valueOf(rt.availableProcessors()));
        datos.put("javaVersion", System.getProperty("java.version"));
        return datos;
    }
}
