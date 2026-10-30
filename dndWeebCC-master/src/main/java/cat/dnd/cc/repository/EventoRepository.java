package cat.dnd.cc.repository;

import cat.dnd.cc.model.Evento;
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
 * Persiste los eventos como un JSON por evento en data/eventos, con id de texto
 * (slug generado del nombre) — mismo enfoque que AventuraRepository, pero con id de
 * texto en vez de numérico porque otros eventos/historias pueden referenciarlo
 * (campo continuesTo) y un id legible es más fácil de reconocer.
 */
@Repository
public class EventoRepository {

    private static final Path CARPETA = Paths.get("data", "eventos");

    private final ObjectMapper objectMapper;
    private final Map<String, Evento> eventos = new LinkedHashMap<>();

    public EventoRepository(ObjectMapper objectMapper) {
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
            Evento evento = objectMapper.readValue(fichero.toFile(), Evento.class);
            if (evento.getId() != null) {
                eventos.put(evento.getId(), evento);
            }
        } catch (IOException e) {
            System.err.println("No se pudo leer el evento " + fichero + ": " + e.getMessage());
        }
    }

    public long count() {
        return eventos.size();
    }

    public List<Evento> findAll() {
        return eventos.values().stream()
                .sorted(Comparator.comparing(Evento::getName, Comparator.nullsLast(String::compareTo)))
                .collect(Collectors.toList());
    }

    public Optional<Evento> findById(String id) {
        return Optional.ofNullable(eventos.get(id));
    }

    public Evento save(Evento evento) {
        if (evento.getId() == null || evento.getId().isBlank()) {
            evento.setId(generarId(evento.getName()));
        }
        eventos.put(evento.getId(), evento);
        escribirFichero(evento);
        return evento;
    }

    public void deleteById(String id) {
        eventos.remove(id);
        try {
            Files.deleteIfExists(CARPETA.resolve(id + ".json"));
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo borrar el evento " + id, e);
        }
    }

    /** "El Ataque Nocturno" → "evento_el_ataque_nocturno" (con sufijo numérico si ya existe). */
    private String generarId(String nombre) {
        String normalizado = Normalizer.normalize(nombre == null ? "" : nombre, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "");
        String limpio = normalizado.toLowerCase().replaceAll("[^a-z0-9]+", "_").replaceAll("^_+|_+$", "");
        String base = "evento_" + (limpio.isBlank() ? "sin_nombre" : limpio);
        String id = base;
        int n = 1;
        while (eventos.containsKey(id)) {
            id = base + "_" + (++n);
        }
        return id;
    }

    private void escribirFichero(Evento evento) {
        try {
            Files.createDirectories(CARPETA);
            objectMapper.writerWithDefaultPrettyPrinter()
                    .writeValue(CARPETA.resolve(evento.getId() + ".json").toFile(), evento);
        } catch (IOException e) {
            throw new IllegalStateException("No se pudo guardar el evento " + evento.getId(), e);
        }
    }
}
