package cat.dnd.cc.service;

import cat.dnd.cc.model.TierPassive;
import cat.dnd.cc.repository.TierPassiveRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierPassiveService {
    private final TierPassiveRepository tierPassiveRepository;

    public TierPassiveService(TierPassiveRepository tierPassiveRepository) {
        this.tierPassiveRepository = tierPassiveRepository;
    }

    public long count() {
        return tierPassiveRepository.count();
    }

    public List<TierPassive> listarTodas() {
        return tierPassiveRepository.findAll();
    }

    public TierPassive obtenerPorId(String id) {
        return tierPassiveRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de pasiva no encontrada: " + id));
    }

    public TierPassive guardar(TierPassive carta) {
        return tierPassiveRepository.save(carta);
    }

    public void eliminar(String id) {
        tierPassiveRepository.deleteById(id);
    }
}
