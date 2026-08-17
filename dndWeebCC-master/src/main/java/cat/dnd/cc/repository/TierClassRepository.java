package cat.dnd.cc.repository;

import cat.dnd.cc.model.TierClass;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Repository;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.Normalizer;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Persiste las cartas de clase del sistema de tiers como un fichero JSON por carta,
 * en la carpeta data/cartas/clases (relativa al directorio de trabajo del proceso, es decir,
 * la raíz del proyecto cuando se ejecuta con mvn/spring-boot:run). No usa base de datos,
 * siguiendo el mismo enfoque "todo en disco" del resto de la aplicación.
 */
@Repository
public class TierClassRepository {

    private static final Path CARPETA = Paths.get("data", "cartas", "clases");

    private final ObjectMapper objectMapper;
    private final Map<String, TierClass> cartas = new LinkedHashMap<>();

    public TierClassRepository(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
        cargarDesdeDisco();
    }

    private void cargarDesdeDisco() {
        try {
            Files.createDirectories(CARPETA);
            try (Stream<Path> ficheros = Files.list(CARPETA)) {
                ficheros
                        .filter(p -> p.toString().endsWith(".json"))
                        .sorted()
                        .forEach(this::cargarFichero);
            }
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo leer la carpeta " + CARPETA, e);
        }
    }

    private void cargarFichero(Path fichero) {
        try {
            TierClass carta = objectMapper.readValue(fichero.toFile(), TierClass.class);
            if (carta.getId() == null || carta.getId().isBlank()) {
                carta.setId(fichero.getFileName().toString().replace(".json", ""));
            }
            cartas.put(carta.getId(), carta);
        } catch (IOException e) {
            System.err.println("No se pudo leer la carta de clase " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return cartas.size();
    }

    public List<TierClass> findAll() {
        return cartas.values().stream()
                .sorted(Comparator.comparing(TierClass::getName, String.CASE_INSENSITIVE_ORDER))
                .collect(Collectors.toList());
    }

    public Optional<TierClass> findById(String id) {
        return Optional.ofNullable(cartas.get(id));
    }

    public TierClass save(TierClass carta) {
        if (carta.getId() == null || carta.getId().isBlank()) {
            carta.setId(generarId(carta.getName()));
        }
        cartas.put(carta.getId(), carta);
        escribirFichero(carta);
        return carta;
    }

    public void deleteById(String id) {
        cartas.remove(id);
        try {
            Files.deleteIfExists(CARPETA.resolve(id + ".json"));
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo borrar la carta " + id, e);
        }
    }

    private void escribirFichero(TierClass carta) {
        try {
            Files.createDirectories(CARPETA);
            objectMapper.writerWithDefaultPrettyPrinter()
                    .writeValue(CARPETA.resolve(carta.getId() + ".json").toFile(), carta);
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo guardar la carta " + carta.getId(), e);
        }
    }

    private String generarId(String nombre) {
        String base = slug(nombre);
        if (base.isBlank()) {
            base = "clase";
        }
        String candidato = base;
        int sufijo = 2;
        while (cartas.containsKey(candidato)) {
            candidato = base + "_" + sufijo++;
        }
        return candidato;
    }

    public static String slug(String texto) {
        if (texto == null) {
            return "";
        }
        String sinAcentos = Normalizer.normalize(texto, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "");
        return sinAcentos.toLowerCase()
                .replaceAll("[^a-z0-9]+", "_")
                .replaceAll("^_+|_+$", "");
    }
}
