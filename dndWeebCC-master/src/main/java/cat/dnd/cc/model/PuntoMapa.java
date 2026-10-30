package cat.dnd.cc.model;

import java.util.List;
import java.util.TreeSet;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Vista resuelta de un GeografiaMapa.Punto para el panel lateral de /mapa: los ids de
 * historia/aventura/evento del punto ya están cambiados por los objetos reales (o
 * descartados si el id ya no existe en el catálogo). También lleva los ids ya unidos en
 * CSV (historiaIdsCsv / aventuraIdsCsv / eventoIdsCsv) para los atributos data-* del
 * marcador — se calculan aquí, no en la plantilla, para no depender de proyecciones
 * SpringEL sobre listas.
 */
public class PuntoMapa {

    private final String id;
    private final String nombre;
    private final String icono;
    private final int x;
    private final int y;
    private final List<Historia> historias;
    private final List<Aventura> aventuras;
    private final List<Evento> eventos;
    private final String historiaIdsCsv;
    private final String aventuraIdsCsv;
    private final String eventoIdsCsv;
    private final String tiersCsv;

    public PuntoMapa(String id, String nombre, String icono, int x, int y,
                      List<Historia> historias, List<Aventura> aventuras, List<Evento> eventos) {
        this.id = id;
        this.nombre = nombre;
        this.icono = icono;
        this.x = x;
        this.y = y;
        this.historias = historias;
        this.aventuras = aventuras;
        this.eventos = eventos;
        this.historiaIdsCsv = historias.stream().map(Historia::getId).collect(Collectors.joining(","));
        this.aventuraIdsCsv = aventuras.stream().map(a -> String.valueOf(a.getId())).collect(Collectors.joining(","));
        this.eventoIdsCsv = eventos.stream().map(Evento::getId).collect(Collectors.joining(","));
        this.tiersCsv = Stream.concat(
                        historias.stream().flatMap(h -> h.getTierRangeExpandido().stream()),
                        eventos.stream().map(Evento::getTier))
                .collect(Collectors.toCollection(TreeSet::new))
                .stream().map(String::valueOf).collect(Collectors.joining(","));
    }

    public String getId() { return id; }
    public String getNombre() { return nombre; }
    public String getIcono() { return icono; }
    public int getX() { return x; }
    public int getY() { return y; }
    public List<Historia> getHistorias() { return historias; }
    public List<Aventura> getAventuras() { return aventuras; }
    public List<Evento> getEventos() { return eventos; }
    public String getHistoriaIdsCsv() { return historiaIdsCsv; }
    public String getAventuraIdsCsv() { return aventuraIdsCsv; }
    public String getEventoIdsCsv() { return eventoIdsCsv; }
    public String getTiersCsv() { return tiersCsv; }
}
