package cat.dnd.cc.service;

import cat.dnd.cc.model.TierConsumable;
import cat.dnd.cc.repository.TierConsumableRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierConsumableService {
    private final TierConsumableRepository tierConsumableRepository;

    public TierConsumableService(TierConsumableRepository tierConsumableRepository) {
        this.tierConsumableRepository = tierConsumableRepository;
    }

    public long count() {
        return tierConsumableRepository.count();
    }

    public List<TierConsumable> listarTodas() {
        return tierConsumableRepository.findAll();
    }

    public TierConsumable obtenerPorId(String id) {
        return tierConsumableRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de consumible no encontrada: " + id));
    }

    public TierConsumable guardar(TierConsumable carta) {
        return tierConsumableRepository.save(carta);
    }

    public void eliminar(String id) {
        tierConsumableRepository.deleteById(id);
    }
}
