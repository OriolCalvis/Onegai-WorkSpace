package cat.dnd.cc.service;

import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.repository.TierClassRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierClassService {
    private final TierClassRepository tierClassRepository;

    public TierClassService(TierClassRepository tierClassRepository) {
        this.tierClassRepository = tierClassRepository;
    }

    public long count() {
        return tierClassRepository.count();
    }

    public List<TierClass> listarTodas() {
        return tierClassRepository.findAll();
    }

    public TierClass obtenerPorId(String id) {
        return tierClassRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de clase no encontrada: " + id));
    }

    public TierClass guardar(TierClass carta) {
        return tierClassRepository.save(carta);
    }

    public void eliminar(String id) {
        tierClassRepository.deleteById(id);
    }
}
