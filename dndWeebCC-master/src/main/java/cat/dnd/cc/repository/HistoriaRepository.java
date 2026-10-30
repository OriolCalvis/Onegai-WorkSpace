package cat.dnd.cc.repository;

import cat.dnd.cc.model.Historia;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Repository;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Lee las cartas de Historia de data/historias (solo lectura: el catálogo se genera
 * con scripts/generar_historias_npcs.py). Además carga un índice ligero id→nombre del
 * bestiario (data/cartas/enemigos) para mostrar el antagonista sin cargar 430 enemigos.
 */
@Repository
public class HistoriaRepository {

    private static final Path CARPETA = Paths.get("data", "historias");
    private static final Path ENEMIGOS = Paths.get("data", "cartas", "enemigos");

    private final ObjectMapper objectMapper;
    private final Map<String, Historia> historias = new LinkedHashMap<>();
    private final Map<String, String> nombresEnemigos = new LinkedHashMap<>();

    public HistoriaRepository(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
        cargarHistorias();
        cargarNombresEnemigos();
    }

    private void cargarHistorias() {
        try {
            Files.createDirectories(CARPETA);
            try (Stream<Path> ficheros = Files.list(CARPETA)) {
                ficheros.filter(p -> p.toString().endsWith(".json")).sorted().forEach(this::cargarFichero);
            }
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo leer la carpeta " + CARPETA, e);
        }
    }

    private void cargarFichero(Path fichero) {
        try {
            Historia historia = objectMapper.readValue(fichero.toFile(), Historia.class);
            if (historia.getId() == null || historia.getId().isBlank()) {
                historia.setId(fichero.getFileName().toString().replace(".json", ""));
            }
            historias.put(historia.getId(), historia);
        } catch (IOException e) {
            System.err.println("No se pudo leer la historia " + fichero + ": " + e.getMessage());
        }
    }

    private void cargarNombresEnemigos() {
        if (!Files.isDirectory(ENEMIGOS)) {
            return;
        }
        try (Stream<Path> ficheros = Files.list(ENEMIGOS)) {
            ficheros.filter(p -> p.toString().endsWith(".json")).forEach(p -> {
                try {
                    JsonNode nodo = objectMapper.readTree(p.toFile());
                    nombresEnemigos.put(nodo.path("id").asText(), nodo.path("name").asText());
                } catch (IOException ignorada) {
                    // un enemigo ilegible no debe tumbar el catálogo de historias
                }
            });
        } catch (IOException e) {
            System.err.println("No se pudo indexar el bestiario: " + e.getMessage());
        }
    }

    public long count() {
        return historias.size();
    }

    public List<Historia> findAll() {
        return historias.values().stream()
                .sorted(Comparator.comparing(Historia::getFaction, Comparator.nullsLast(String::compareTo))
                        .thenComparing(h -> h.getChain() != null ? h.getChain().getTrama() : "",
                                Comparator.nullsLast(String::compareTo))
                        .thenComparingInt(h -> h.getChain() != null ? h.getChain().getStep() : 0))
                .collect(Collectors.toList());
    }

    public Optional<Historia> findById(String id) {
        return Optional.ofNullable(historias.get(id));
    }

    /** Nombre legible del antagonista, o su id si el bestiario no lo conoce. */
    public String nombreAntagonista(String enemigoId) {
        return nombresEnemigos.getOrDefault(enemigoId, enemigoId);
    }

    /**
     * Guarda una historia creada a mano en data/historias/&lt;id&gt;.json y la
     * incorpora al índice en memoria (formulario de creación — wireframe 12a).
     * Hasta ahora el catálogo solo se poblaba en lote vía
     * scripts/generar_historias_npcs.py; esta vía cubre el caso de UNA historia
     * suelta hecha a medida por el director.
     */
    public Historia guardar(Historia historia) {
        if (historia.getId() == null || historia.getId().isBlank()) {
            throw new IllegalArgumentException("La historia necesita un id");
        }
        try {
            Files.createDirectories(CARPETA);
            objectMapper.writerWithDefaultPrettyPrinter()
                    .writeValue(CARPETA.resolve(historia.getId() + ".json").toFile(), historia);
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo guardar la historia " + historia.getId(), e);
        }
        historias.put(historia.getId(), historia);
        return historia;
    }
}
