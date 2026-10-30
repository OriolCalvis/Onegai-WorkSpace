package cat.dnd.cc.repository;

import cat.dnd.cc.model.TierFeat;
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
 * Persiste las cartas de dote como un fichero JSON por carta en data/cartas/dotes.
 */
@Repository
public class TierFeatRepository {

    private static final Path CARPETA = Paths.get("data", "cartas", "dotes");

    private final ObjectMapper objectMapper;
    private final Map<String, TierFeat> cartas = new LinkedHashMap<>();

    public TierFeatRepository(ObjectMapper objectMapper) {
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
            TierFeat carta = objectMapper.readValue(fichero.toFile(), TierFeat.class);
            if (carta.getId() == null || carta.getId().isBlank()) {
                carta.setId(fichero.getFileName().toString().replace(".json", ""));
            }
            cartas.put(carta.getId(), carta);
        } catch (IOException e) {
            System.err.println("No se pudo leer la carta de dote " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return cartas.size();
    }

    public List<TierFeat> findAll() {
        return cartas.values().stream()
                .sorted(Comparator.comparing(TierFeat::getName, String.CASE_INSENSITIVE_ORDER))
                .collect(Collectors.toList());
    }

    public Optional<TierFeat> findById(String id) {
        return Optional.ofNullable(cartas.get(id));
    }

    public TierFeat save(TierFeat carta) {
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

    private void escribirFichero(TierFeat carta) {
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
            base = "dote";
        }
        String candidato = base;
        int sufijo = 2;
        while (cartas.containsKey(candidato)) {
            candidato = base + "_" + sufijo++;
        }
        return candidato;
    }
}
