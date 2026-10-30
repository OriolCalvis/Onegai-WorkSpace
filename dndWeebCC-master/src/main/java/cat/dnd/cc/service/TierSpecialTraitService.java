package cat.dnd.cc.service;

import cat.dnd.cc.model.TierSpecialTrait;
import cat.dnd.cc.repository.TierSpecialTraitRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierSpecialTraitService {
    private final TierSpecialTraitRepository tierSpecialTraitRepository;

    public TierSpecialTraitService(TierSpecialTraitRepository tierSpecialTraitRepository) {
        this.tierSpecialTraitRepository = tierSpecialTraitRepository;
    }

    public long count() {
        return tierSpecialTraitRepository.count();
    }

    public List<TierSpecialTrait> listarTodas() {
        return tierSpecialTraitRepository.findAll();
    }

    public TierSpecialTrait obtenerPorId(String id) {
        return tierSpecialTraitRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de rasgo no encontrada: " + id));
    }

    public TierSpecialTrait guardar(TierSpecialTrait carta) {
        return tierSpecialTraitRepository.save(carta);
    }

    public void eliminar(String id) {
        tierSpecialTraitRepository.deleteById(id);
    }
}
