package cat.dnd.cc.service;

import cat.dnd.cc.model.TierBackground;
import cat.dnd.cc.repository.TierBackgroundRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierBackgroundService {
    private final TierBackgroundRepository tierBackgroundRepository;

    public TierBackgroundService(TierBackgroundRepository tierBackgroundRepository) {
        this.tierBackgroundRepository = tierBackgroundRepository;
    }

    public long count() {
        return tierBackgroundRepository.count();
    }

    public List<TierBackground> listarTodas() {
        return tierBackgroundRepository.findAll();
    }

    public TierBackground obtenerPorId(String id) {
        return tierBackgroundRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de trasfondo no encontrada: " + id));
    }

    public TierBackground guardar(TierBackground carta) {
        return tierBackgroundRepository.save(carta);
    }

    public void eliminar(String id) {
        tierBackgroundRepository.deleteById(id);
    }
}
