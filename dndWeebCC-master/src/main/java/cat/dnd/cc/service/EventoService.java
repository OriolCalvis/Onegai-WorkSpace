package cat.dnd.cc.service;

import cat.dnd.cc.model.Evento;
import cat.dnd.cc.repository.EventoRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class EventoService {

    private final EventoRepository eventoRepository;

    public EventoService(EventoRepository eventoRepository) {
        this.eventoRepository = eventoRepository;
    }

    public long count() {
        return eventoRepository.count();
    }

    public List<Evento> listarTodos() {
        return eventoRepository.findAll();
    }

    public Evento obtenerPorId(String id) {
        return eventoRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Evento no encontrado: " + id));
    }

    public Evento guardar(Evento evento) {
        return eventoRepository.save(evento);
    }

    public void eliminar(String id) {
        eventoRepository.deleteById(id);
    }
}
