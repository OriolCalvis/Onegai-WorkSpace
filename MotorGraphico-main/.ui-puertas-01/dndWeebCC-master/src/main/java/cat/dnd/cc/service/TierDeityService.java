package cat.dnd.cc.service;

import cat.dnd.cc.model.TierDeity;
import cat.dnd.cc.repository.TierDeityRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierDeityService {
    private final TierDeityRepository tierDeityRepository;

    public TierDeityService(TierDeityRepository tierDeityRepository) {
        this.tierDeityRepository = tierDeityRepository;
    }

    public long count() {
        return tierDeityRepository.count();
    }

    public List<TierDeity> listarTodas() {
        return tierDeityRepository.findAll();
    }

    public TierDeity obtenerPorId(String id) {
        return tierDeityRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de deidad no encontrada: " + id));
    }

    public TierDeity guardar(TierDeity carta) {
        return tierDeityRepository.save(carta);
    }

    public void eliminar(String id) {
        tierDeityRepository.deleteById(id);
    }
}
