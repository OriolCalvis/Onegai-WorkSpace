package cat.dnd.cc.service;

import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.repository.TierSpellRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierSpellService {
    private final TierSpellRepository tierSpellRepository;

    public TierSpellService(TierSpellRepository tierSpellRepository) {
        this.tierSpellRepository = tierSpellRepository;
    }

    public long count() {
        return tierSpellRepository.count();
    }

    public List<TierSpell> listarTodas() {
        return tierSpellRepository.findAll();
    }

    public TierSpell obtenerPorId(String id) {
        return tierSpellRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de hechizo no encontrada: " + id));
    }

    public TierSpell guardar(TierSpell carta) {
        return tierSpellRepository.save(carta);
    }

    public void eliminar(String id) {
        tierSpellRepository.deleteById(id);
    }
}
