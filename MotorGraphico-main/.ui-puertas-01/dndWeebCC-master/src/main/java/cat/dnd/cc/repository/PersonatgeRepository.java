package cat.dnd.cc.repository;

import cat.dnd.cc.model.Personatge;
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
import java.util.concurrent.atomic.AtomicLong;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Persiste los personajes como un fichero JSON por personaje en data/personatges/ (relativa
 * al directorio de trabajo del proceso, igual que los repositorios de cartas bajo data/cartas/).
 * Antes los personajes solo vivían en memoria y se perdían al reiniciar la aplicación; ahora
 * siguen el mismo enfoque "todo en disco, sin base de datos" que el resto del sistema.
 */
@Repository
public class PersonatgeRepository {

    private static final Path CARPETA = Paths.get("data", "personatges");

    private final ObjectMapper objectMapper;
    private final Map<Long, Personatge> personatges = new LinkedHashMap<>();
    private final AtomicLong sequence = new AtomicLong(1);

    public PersonatgeRepository(ObjectMapper objectMapper) {
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
        long maxId = personatges.keySet().stream().mapToLong(Long::longValue).max().orElse(0);
        sequence.set(maxId + 1);
    }

    private void cargarFichero(Path fichero) {
        try {
            Personatge personatge = objectMapper.readValue(fichero.toFile(), Personatge.class);
            if (personatge.getId() == null) {
                String nombreFichero = fichero.getFileName().toString().replace(".json", "");
                try {
                    personatge.setId(Long.parseLong(nombreFichero));
                } catch (NumberFormatException e) {
                    System.err.println("Nombre de fichero de personaje inválido, se ignora: " + fichero);
                    return;
                }
            }
            personatges.put(personatge.getId(), personatge);
        } catch (IOException e) {
            System.err.println("No se pudo leer el personaje " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return personatges.size();
    }

    public List<Personatge> findAll() {
        return personatges.values().stream()
                .sorted(Comparator.comparing(
                        p -> p.getNom() == null ? "" : p.getNom(),
                        String.CASE_INSENSITIVE_ORDER))
                .collect(Collectors.toList());
    }

    public Optional<Personatge> findById(Long id) {
        return Optional.ofNullable(personatges.get(id));
    }

    public Personatge save(Personatge personatge) {
        if (personatge.getId() == null) {
            personatge.setId(sequence.getAndIncrement());
        }
        personatges.put(personatge.getId(), personatge);
        escribirFichero(personatge);
        return personatge;
    }

    public void deleteById(Long id) {
        personatges.remove(id);
        try {
            Files.deleteIfExists(CARPETA.resolve(id + ".json"));
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo borrar el personaje " + id, e);
        }
    }

    private void escribirFichero(Personatge personatge) {
        try {
            Files.createDirectories(CARPETA);
            objectMapper.writerWithDefaultPrettyPrinter()
                    .writeValue(CARPETA.resolve(personatge.getId() + ".json").toFile(), personatge);
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo guardar el personaje " + personatge.getId(), e);
        }
    }
}
