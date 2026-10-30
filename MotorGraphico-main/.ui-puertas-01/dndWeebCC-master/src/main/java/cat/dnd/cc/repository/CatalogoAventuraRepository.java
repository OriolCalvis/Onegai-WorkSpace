package cat.dnd.cc.repository;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Repository;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Stream;

/**
 * Catálogo LIGERO para el constructor de aventuras: en vez de cargar cientos de cartas
 * completas (430 enemigos, 200 tablas de loot...), indexa solo los campos que la mesa
 * del director necesita (id, nombre, tier, rango, facción). Los tipos con modelo propio
 * (historias, deidades) siguen usando sus repositorios completos.
 */
@Repository
public class CatalogoAventuraRepository {

    /** Resumen mínimo de una carta para pickers y generación aleatoria. */
    public static class Resumen {
        private final String id;
        private final String name;
        private final int tier;
        private final String rank;
        private final String faction;
        private final String extra;

        public Resumen(String id, String name, int tier, String rank, String faction, String extra) {
            this.id = id;
            this.name = name;
            this.tier = tier;
            this.rank = rank;
            this.faction = faction;
            this.extra = extra;
        }

        public String getId() { return id; }
        public String getName() { return name; }
        public int getTier() { return tier; }
        public String getRank() { return rank; }
        public String getFaction() { return faction; }
        public String getExtra() { return extra; }
        public String getFactionNombre() { return faction == null ? "" : faction.replace('_', ' '); }
    }

    private final ObjectMapper objectMapper;
    private final List<Resumen> npcs = new ArrayList<>();
    private final List<Resumen> villanos = new ArrayList<>();
    private final List<Resumen> enemigosRegulares = new ArrayList<>();
    private final List<Resumen> lootTables = new ArrayList<>();
    private final List<Resumen> trampas = new ArrayList<>();
    private final Map<String, String> nombres = new LinkedHashMap<>();
    /** id → fichero, para leer la carta completa bajo demanda (impresión). */
    private final Map<String, Path> rutas = new LinkedHashMap<>();
    // Carta completa (JSON crudo) por id, solo para quien necesite el detalle real
    // (p. ej. la impresión de todas las cartas de una aventura). El resto de la app
    // sigue usando el Resumen ligero para no cargar cientos de cartas de más.
    private final Map<String, JsonNode> completas = new LinkedHashMap<>();

    public CatalogoAventuraRepository(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
        cargarNpcs();
        cargarEnemigos();
        cargarLoot();
        cargarTrampas();
        // solo para resolver nombres en el detalle (el picker de deidades usa TierDeityService)
        recorrer(Paths.get("data", "cartas", "deidades"),
                n -> nombres.put(n.path("id").asText(), n.path("name").asText()));
        // solo nombres: los drops de las tablas de loot (data/loot/*.json → drops[].item)
        // apuntan a equipo/consumibles reales y hasta ahora no había forma de resolver su
        // nombre legible — la impresión de aventuras los mostraba como id en bruto.
        recorrer(Paths.get("data", "cartas", "armas"),
                n -> nombres.put(n.path("id").asText(), n.path("name").asText()));
        recorrer(Paths.get("data", "cartas", "consumibles"),
                n -> nombres.put(n.path("id").asText(), n.path("name").asText()));
    }

    private void recorrer(Path carpeta, java.util.function.Consumer<JsonNode> consumidor) {
        if (!Files.isDirectory(carpeta)) {
            return;
        }
        try (Stream<Path> ficheros = Files.list(carpeta)) {
            ficheros.filter(p -> p.toString().endsWith(".json")).sorted().forEach(p -> {
                try {
                    consumidor.accept(objectMapper.readTree(p.toFile()));
                } catch (IOException ignorada) {
                    // una carta ilegible no tumba el catálogo
                }
            });
        } catch (IOException e) {
            System.err.println("No se pudo indexar " + carpeta + ": " + e.getMessage());
        }
    }

    private void cargarNpcs() {
        recorrer(Paths.get("data", "npcs"), n -> {
            Resumen r = new Resumen(n.path("id").asText(), n.path("name").asText(), 1,
                    n.path("role").asText(""), null, n.path("location").asText(""));
            npcs.add(r);
            nombres.put(r.getId(), r.getName());
            completas.put(r.getId(), n);
        });
        npcs.sort(Comparator.comparing(Resumen::getName, String.CASE_INSENSITIVE_ORDER));
    }

    private void cargarEnemigos() {
        recorrer(Paths.get("data", "cartas", "enemigos"), n -> {
            Resumen r = new Resumen(n.path("id").asText(), n.path("name").asText(),
                    n.path("tier").asInt(1), n.path("rank").asText(""),
                    n.path("faction").asText(""), n.path("role").asText(""));
            if (n.has("villainProfile")) {
                villanos.add(r);
            } else {
                enemigosRegulares.add(r);
            }
            nombres.put(r.getId(), r.getName());
            completas.put(r.getId(), n);
        });
        Comparator<Resumen> orden = Comparator.comparingInt(Resumen::getTier)
                .thenComparing(Resumen::getName, String.CASE_INSENSITIVE_ORDER);
        villanos.sort(orden);
        enemigosRegulares.sort(orden);
    }

    private void cargarLoot() {
        recorrer(Paths.get("data", "loot"), n -> {
            int tier = n.path("tierRange").has(0) ? n.path("tierRange").get(0).asInt(1) : 1;
            Resumen r = new Resumen(n.path("id").asText(), n.path("name").asText(), tier, "loot", null, "");
            lootTables.add(r);
            nombres.put(r.getId(), r.getName());
            completas.put(r.getId(), n);
        });
        lootTables.sort(Comparator.comparingInt(Resumen::getTier)
                .thenComparing(Resumen::getName, String.CASE_INSENSITIVE_ORDER));
    }

    private void cargarTrampas() {
        recorrer(Paths.get("data", "cartas", "trampas"), n -> {
            Resumen r = new Resumen(n.path("id").asText(), n.path("name").asText(),
                    n.path("tier").asInt(1), "trampa", null, n.path("category").asText(""));
            trampas.add(r);
            nombres.put(r.getId(), r.getName());
            completas.put(r.getId(), n);
        });
        trampas.sort(Comparator.comparingInt(Resumen::getTier)
                .thenComparing(Resumen::getName, String.CASE_INSENSITIVE_ORDER));
    }

    public List<Resumen> npcs() { return npcs; }
    public List<Resumen> villanos() { return villanos; }
    public List<Resumen> enemigosRegulares() { return enemigosRegulares; }
    public List<Resumen> lootTables() { return lootTables; }
    public List<Resumen> trampas() { return trampas; }

    /** Nombre legible de cualquier carta indexada, o el propio id si no se conoce. */
    public String nombreDe(String id) {
        return nombres.getOrDefault(id, id);
    }

    /**
     * Carta completa (JSON crudo tal cual está en disco) de un npc/enemigo/loot/trampa,
     * o {@code null} si el id no está indexado. Pensada para vistas de impresión que
     * necesitan más que el Resumen (texto de reglas, ataques, drops...).
     */
    public JsonNode completa(String id) {
        return completas.get(id);
    }

    /**
     * Guarda un PNJ nuevo en data/npcs/&lt;id&gt;.json y lo incorpora al índice en
     * memoria, para que el álbum, los pickers y el constructor de aventuras lo vean
     * sin reiniciar la aplicación (formulario de creación — wireframe 10a).
     */
    public void guardarNpc(JsonNode npc) throws IOException {
        String id = npc.path("id").asText();
        if (id.isBlank()) {
            throw new IllegalArgumentException("El PNJ necesita un id");
        }
        Path carpeta = Paths.get("data", "npcs");
        Files.createDirectories(carpeta);
        objectMapper.writerWithDefaultPrettyPrinter().writeValue(carpeta.resolve(id + ".json").toFile(), npc);

        // refresca el índice en memoria (sustituye si ya existía)
        npcs.removeIf(r -> r.getId().equals(id));
        Resumen r = new Resumen(id, npc.path("name").asText(), 1,
                npc.path("role").asText(""), null, npc.path("location").asText(""));
        npcs.add(r);
        npcs.sort(Comparator.comparing(Resumen::getName, String.CASE_INSENSITIVE_ORDER));
        nombres.put(id, r.getName());
        completas.put(id, npc);
    }
}
