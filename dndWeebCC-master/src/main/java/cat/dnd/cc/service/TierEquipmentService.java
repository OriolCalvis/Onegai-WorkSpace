package cat.dnd.cc.service;

import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.repository.TierEquipmentRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TierEquipmentService {
    private final TierEquipmentRepository tierEquipmentRepository;

    public TierEquipmentService(TierEquipmentRepository tierEquipmentRepository) {
        this.tierEquipmentRepository = tierEquipmentRepository;
    }

    public long count() {
        return tierEquipmentRepository.count();
    }

    public List<TierEquipment> listarTodas() {
        return tierEquipmentRepository.findAll();
    }

    public TierEquipment obtenerPorId(String id) {
        return tierEquipmentRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Carta de equipo no encontrada: " + id));
    }

    public TierEquipment guardar(TierEquipment carta) {
        return tierEquipmentRepository.save(carta);
    }

    public void eliminar(String id) {
        tierEquipmentRepository.deleteById(id);
    }
}
