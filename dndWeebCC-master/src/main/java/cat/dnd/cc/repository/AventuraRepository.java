package cat.dnd.cc.repository;

import cat.dnd.cc.model.Aventura;
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
 * Persiste las aventuras (recopilaciones de historias) como un JSON por aventura en
 * data/aventuras, con id numérico — mismo enfoque que PersonatgeRepository.
 */
@Repository
public class AventuraRepository {

    private static final Path CARPETA = Paths.get("data", "aventuras");

    private final ObjectMapper objectMapper;
    private final Map<Long, Aventura> aventuras = new LinkedHashMap<>();

    public AventuraRepository(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
        cargarDesdeDisco();
    }

    private void cargarDesdeDisco() {
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
            Aventura aventura = objectMapper.readValue(fichero.toFile(), Aventura.class);
            if (aventura.getId() != null) {
                aventuras.put(aventura.getId(), aventura);
            }
        } catch (IOException e) {
            System.err.println("No se pudo leer la aventura " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return aventuras.size();
    }

    public List<Aventura> findAll() {
        return aventuras.values().stream()
                .sorted(Comparator.comparing(Aventura::getId))
                .collect(Collectors.toList());
    }

    public Optional<Aventura> findById(Long id) {
        return Optional.ofNullable(aventuras.get(id));
    }

    public Aventura save(Aventura aventura) {
        if (aventura.getId() == null) {
            aventura.setId(siguienteId());
        }
        aventuras.put(aventura.getId(), aventura);
        escribirFichero(aventura);
        return aventura;
    }

    public void deleteById(Long id) {
        aventuras.remove(id);
        try {
            Files.deleteIfExists(CARPETA.resolve(id + ".json"));
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo borrar la aventura " + id, e);
        }
    }

    private long siguienteId() {
        return aventuras.keySet().stream().mapToLong(Long::longValue).max().orElse(0L) + 1;
    }

    private void escribirFichero(Aventura aventura) {
        try {
            Files.createDirectories(CARPETA);
            objectMapper.writerWithDefaultPrettyPrinter()
                    .writeValue(CARPETA.resolve(aventura.getId() + ".json").toFile(), aventura);
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo guardar la aventura " + aventura.getId(), e);
        }
    }
}
