package cat.dnd.cc.service;

import cat.dnd.cc.model.TierSummon;
import cat.dnd.cc.repository.TierSummonRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierSummonService {
    private final TierSummonRepository tierSummonRepository;

    public TierSummonService(TierSummonRepository tierSummonRepository) {
        this.tierSummonRepository = tierSummonRepository;
    }

    public long count() {
        return tierSummonRepository.count();
    }

    public List<TierSummon> listarTodas() {
        return tierSummonRepository.findAll();
    }

    public TierSummon obtenerPorId(String id) {
        return tierSummonRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de invocación no encontrada: " + id));
    }

    public TierSummon guardar(TierSummon carta) {
        return tierSummonRepository.save(carta);
    }

    public void eliminar(String id) {
        tierSummonRepository.deleteById(id);
    }
}
