package cat.dnd.cc.service;

import cat.dnd.cc.model.TierFeat;
import cat.dnd.cc.repository.TierFeatRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierFeatService {
    private final TierFeatRepository tierFeatRepository;

    public TierFeatService(TierFeatRepository tierFeatRepository) {
        this.tierFeatRepository = tierFeatRepository;
    }

    public long count() {
        return tierFeatRepository.count();
    }

    public List<TierFeat> listarTodas() {
        return tierFeatRepository.findAll();
    }

    public TierFeat obtenerPorId(String id) {
        return tierFeatRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de dote no encontrada: " + id));
    }

    public TierFeat guardar(TierFeat carta) {
        return tierFeatRepository.save(carta);
    }

    public void eliminar(String id) {
        tierFeatRepository.deleteById(id);
    }
}
