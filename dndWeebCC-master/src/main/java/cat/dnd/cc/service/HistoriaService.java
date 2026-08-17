package cat.dnd.cc.service;

import cat.dnd.cc.model.Historia;
import cat.dnd.cc.repository.HistoriaRepository;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

@Service
public class HistoriaService {

    private final HistoriaRepository historiaRepository;

    public HistoriaService(HistoriaRepository historiaRepository) {
        this.historiaRepository = historiaRepository;
    }

    public long count() {
        return historiaRepository.count();
    }

    public List<Historia> listarTodas() {
        return historiaRepository.findAll();
    }

    public Historia obtenerPorId(String id) {
        return historiaRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Historia no encontrada: " + id));
    }

    /** {@code true} si el id corresponde a una historia real del catálogo, sin lanzar excepción. */
    public boolean existe(String id) {
        return id != null && historiaRepository.findById(id).isPresent();
    }

    /** Guarda una historia creada a mano desde la web (wireframe 12a). */
    public Historia guardar(Historia historia) {
        return historiaRepository.guardar(historia);
    }

    /** Facciones distintas presentes en el catálogo, para el filtro de la lista. */
    public List<String> facciones() {
        return historiaRepository.findAll().stream()
                .map(Historia::getFaction)
                .filter(f -> f != null && !f.isBlank())
                .distinct()
                .sorted()
                .collect(Collectors.toList());
    }

    public String nombreAntagonista(String enemigoId) {
        return historiaRepository.nombreAntagonista(enemigoId);
    }
}
