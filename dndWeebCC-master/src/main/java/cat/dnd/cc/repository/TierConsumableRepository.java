package cat.dnd.cc.repository;

import cat.dnd.cc.model.TierConsumable;
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
 * Persiste las cartas de consumible como un fichero JSON por carta en data/cartas/consumibles.
 */
@Repository
public class TierConsumableRepository {

    private static final Path CARPETA = Paths.get("data", "cartas", "consumibles");

    private final ObjectMapper objectMapper;
    private final Map<String, TierConsumable> cartas = new LinkedHashMap<>();

    public TierConsumableRepository(ObjectMapper objectMapper) {
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
            TierConsumable carta = objectMapper.readValue(fichero.toFile(), TierConsumable.class);
            if (carta.getId() == null || carta.getId().isBlank()) {
                carta.setId(fichero.getFileName().toString().replace(".json", ""));
            }
            cartas.put(carta.getId(), carta);
        } catch (IOException e) {
            System.err.println("No se pudo leer la carta de consumible " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return cartas.size();
    }

    public List<TierConsumable> findAll() {
        return cartas.values().stream()
                .sorted(Comparator.comparing(TierConsumable::getName, String.CASE_INSENSITIVE_ORDER))
                .collect(Collectors.toList());
    }

    public Optional<TierConsumable> findById(String id) {
        return Optional.ofNullable(cartas.get(id));
    }

    public TierConsumable save(TierConsumable carta) {
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

    private void escribirFichero(TierConsumable carta) {
        try {
            Files.createDirectories(CARPETA);
            objectMapper.writerWithDefaultPrettyPrinter()
                    .writeValue(CARPETA.resolve(carta.getId() + ".json").toFile(), carta);
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo guardar la carta " + carta.getId(), e);
        }
    }

    private String generarId(String nombre) {
        String base = TierClassRepository.slug(nombre);
        if (base.isBlank()) {
            base = "consumible";
        }
        String candidato = base;
        int sufijo = 2;
        while (cartas.containsKey(candidato)) {
            candidato = base + "_" + sufijo++;
        }
        return candidato;
    }
}
