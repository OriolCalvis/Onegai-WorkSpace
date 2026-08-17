package cat.dnd.cc.service;

import cat.dnd.cc.model.TierRace;
import cat.dnd.cc.repository.TierRaceRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierRaceService {
    private final TierRaceRepository tierRaceRepository;

    public TierRaceService(TierRaceRepository tierRaceRepository) {
        this.tierRaceRepository = tierRaceRepository;
    }

    public long count() {
        return tierRaceRepository.count();
    }

    public List<TierRace> listarTodas() {
        return tierRaceRepository.findAll();
    }

    public TierRace obtenerPorId(String id) {
        return tierRaceRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de raza no encontrada: " + id));
    }

    public TierRace guardar(TierRace carta) {
        return tierRaceRepository.save(carta);
    }

    public void eliminar(String id) {
        tierRaceRepository.deleteById(id);
    }
}
