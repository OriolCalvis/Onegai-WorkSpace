package cat.dnd.cc.service;

import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.repository.TierSkillRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierSkillService {
    private final TierSkillRepository tierSkillRepository;

    public TierSkillService(TierSkillRepository tierSkillRepository) {
        this.tierSkillRepository = tierSkillRepository;
    }

    public long count() {
        return tierSkillRepository.count();
    }

    public List<TierSkill> listarTodas() {
        return tierSkillRepository.findAll();
    }

    public TierSkill obtenerPorId(String id) {
        return tierSkillRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de habilidad no encontrada: " + id));
    }

    public TierSkill guardar(TierSkill carta) {
        return tierSkillRepository.save(carta);
    }

    public void eliminar(String id) {
        tierSkillRepository.deleteById(id);
    }
}
