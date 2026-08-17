package cat.dnd.cc.service;

import cat.dnd.cc.model.Aventura;
import cat.dnd.cc.model.Historia;
import cat.dnd.cc.repository.AventuraRepository;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import org.springframework.stereotype.Service;

import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

@Service
public class AventuraService {

    private final AventuraRepository aventuraRepository;
    private final HistoriaService historiaService;
    private final CatalogoAventuraRepository catalogo;

    public AventuraService(AventuraRepository aventuraRepository, HistoriaService historiaService,
                           CatalogoAventuraRepository catalogo) {
        this.aventuraRepository = aventuraRepository;
        this.historiaService = historiaService;
        this.catalogo = catalogo;
    }

    public CatalogoAventuraRepository catalogo() {
        return catalogo;
    }

    /** Nombre legible de cualquier id (npc, enemigo, loot, trampa...), para las plantillas. */
    public String nombreDe(String id) {
        return catalogo.nombreDe(id);
    }

    public long count() {
        return aventuraRepository.count();
    }

    public List<Aventura> llistarTotes() {
        return aventuraRepository.findAll();
    }

    public Aventura obtenirPerId(Long id) {
        return aventuraRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Aventura no encontrada: " + id));
    }

    public Aventura guardar(Aventura aventura) {
        return aventuraRepository.save(aventura);
    }

    public void eliminar(Long id) {
        aventuraRepository.deleteById(id);
    }

    /**
     * Resuelve las historias de una aventura contra el catálogo, en orden narrativo:
     * facción → trama madre → escalón (las cadenas de 5 pasos se leen en su orden).
     * Los ids que ya no existan en el catálogo se ignoran sin romper la aventura.
     */
    public List<Historia> historiasDe(Aventura aventura) {
        return aventura.getHistoriaIds().stream()
                .map(id -> {
                    try {
                        return historiaService.obtenerPorId(id);
                    } catch (IllegalArgumentException e) {
                        return null;
                    }
                })
                .filter(Objects::nonNull)
                .sorted(Comparator.comparing(Historia::getFaction, Comparator.nullsLast(String::compareTo))
                        .thenComparing(h -> h.getChain() != null ? h.getChain().getTrama() : "",
                                Comparator.nullsLast(String::compareTo))
                        .thenComparingInt(h -> h.getChain() != null ? h.getChain().getStep() : 0))
                .collect(Collectors.toList());
    }
}
