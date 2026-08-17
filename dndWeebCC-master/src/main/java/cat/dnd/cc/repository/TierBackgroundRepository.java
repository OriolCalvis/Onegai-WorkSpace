package cat.dnd.cc.repository;

import cat.dnd.cc.model.TierBackground;
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
 * Persiste las cartas de trasfondo del sistema de tiers como un fichero JSON por carta,
 * en la carpeta data/cartas/transfondos. Mismo enfoque que TierClassRepository/TierRaceRepository.
 */
@Repository
public class TierBackgroundRepository {

    private static final Path CARPETA = Paths.get("data", "cartas", "transfondos");

    private final ObjectMapper objectMapper;
    private final Map<String, TierBackground> cartas = new LinkedHashMap<>();

    public TierBackgroundRepository(ObjectMapper objectMapper) {
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
            TierBackground carta = objectMapper.readValue(fichero.toFile(), TierBackground.class);
            if (carta.getId() == null || carta.getId().isBlank()) {
                carta.setId(fichero.getFileName().toString().replace(".json", ""));
            }
            cartas.put(carta.getId(), carta);
        } catch (IOException e) {
            System.err.println("No se pudo leer la carta de trasfondo " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return cartas.size();
    }

    public List<TierBackground> findAll() {
        return cartas.values().stream()
                .sorted(Comparator.comparing(TierBackground::getName, String.CASE_INSENSITIVE_ORDER))
                .collect(Collectors.toList());
    }

    public Optional<TierBackground> findById(String id) {
        return Optional.ofNullable(cartas.get(id));
    }

    public TierBackground save(TierBackground carta) {
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

    private void escribirFichero(TierBackground carta) {
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
            base = "transfons";
        }
        String candidato = base;
        int sufijo = 2;
        while (cartas.containsKey(candidato)) {
            candidato = base + "_" + sufijo++;
        }
        return candidato;
    }
}
