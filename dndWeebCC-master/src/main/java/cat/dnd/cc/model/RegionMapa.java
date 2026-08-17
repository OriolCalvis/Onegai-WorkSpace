package cat.dnd.cc.model;

import cat.dnd.cc.repository.CatalogoAventuraRepository;

import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Una región del mapa mundi: agrupa todo lo que ya existe en el catálogo (historias,
 * monstruos, tesoros, pnjs, aventuras, eventos) bajo la misma localización/facción, para
 * el panel lateral del mapa interactivo (/mapa). No es una carta nueva — es una vista
 * sobre datos que ya viven en data/historias, data/cartas/enemigos, data/loot, data/npcs,
 * data/aventuras y data/eventos.
 */
public class RegionMapa {

    private final String id;
    private final String location;
    private final String faccion;
    private final String faccionNombre;
    private final String icono;
    private final int x;
    private final int y;
    private final Map<String, List<Historia>> historiasPorTrama;
    private final List<CatalogoAventuraRepository.Resumen> monstruos;
    private final List<CatalogoAventuraRepository.Resumen> tesoros;
    private final List<CatalogoAventuraRepository.Resumen> npcs;
    private final List<Aventura> aventuras;
    private final List<Evento> eventos;
    private final String tiersCsv;

    public RegionMapa(String id, String location, String faccion, String faccionNombre, String icono, int x, int y,
                       Map<String, List<Historia>> historiasPorTrama,
                       List<CatalogoAventuraRepository.Resumen> monstruos,
                       List<CatalogoAventuraRepository.Resumen> tesoros,
                       List<CatalogoAventuraRepository.Resumen> npcs,
                       List<Aventura> aventuras,
                       List<Evento> eventos) {
        this.id = id;
        this.location = location;
        this.faccion = faccion;
        this.faccionNombre = faccionNombre;
        this.icono = icono;
        this.x = x;
        this.y = y;
        this.historiasPorTrama = historiasPorTrama;
        this.monstruos = monstruos;
        this.tesoros = tesoros;
        this.npcs = npcs;
        this.aventuras = aventuras;
        this.eventos = eventos;
        this.tiersCsv = Stream.concat(
                        historiasPorTrama.values().stream().flatMap(List::stream).flatMap(h -> h.getTierRangeExpandido().stream()),
                        eventos.stream().map(Evento::getTier))
                .collect(Collectors.toCollection(TreeSet::new))
                .stream().map(String::valueOf).collect(Collectors.joining(","));
    }

    public String getId() { return id; }
    public String getLocation() { return location; }
    public String getFaccion() { return faccion; }
    public String getFaccionNombre() { return faccionNombre; }
    public String getIcono() { return icono; }
    public int getX() { return x; }
    public int getY() { return y; }
    public Map<String, List<Historia>> getHistoriasPorTrama() { return historiasPorTrama; }
    public List<CatalogoAventuraRepository.Resumen> getMonstruos() { return monstruos; }
    public List<CatalogoAventuraRepository.Resumen> getTesoros() { return tesoros; }
    public List<CatalogoAventuraRepository.Resumen> getNpcs() { return npcs; }
    public List<Aventura> getAventuras() { return aventuras; }
    public List<Evento> getEventos() { return eventos; }
    public String getTiersCsv() { return tiersCsv; }

    /** Nº total de historias de la región, para el resumen del marcador. */
    public int getTotalHistorias() {
        int total = 0;
        for (List<Historia> h : historiasPorTrama.values()) total += h.size();
        return total;
    }
}
