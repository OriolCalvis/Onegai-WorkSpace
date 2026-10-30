package cat.dnd.cc.service;

import cat.dnd.cc.model.TierCondition;
import cat.dnd.cc.repository.TierConditionRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierConditionService {
    private final TierConditionRepository tierConditionRepository;

    public TierConditionService(TierConditionRepository tierConditionRepository) {
        this.tierConditionRepository = tierConditionRepository;
    }

    public long count() {
        return tierConditionRepository.count();
    }

    public List<TierCondition> listarTodas() {
        return tierConditionRepository.findAll();
    }

    public TierCondition obtenerPorId(String id) {
        return tierConditionRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de condición no encontrada: " + id));
    }

    public TierCondition guardar(TierCondition carta) {
        return tierConditionRepository.save(carta);
    }

    public void eliminar(String id) {
        tierConditionRepository.deleteById(id);
    }
}
