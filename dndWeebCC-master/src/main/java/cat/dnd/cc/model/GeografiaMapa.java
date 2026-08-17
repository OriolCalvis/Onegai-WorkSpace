package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Geografía editable del Mapa Mundi: los polígonos de cada nación y la posición de cada
 * marcador jugable. Vive como JSON en data/mapa/geografia.json (mismo patrón que el resto
 * de datos de la app) para que el editor de puntos de /mapa pueda guardarla sin tocar
 * código ni recompilar.
 */
public class GeografiaMapa {

    private List<Nacion> naciones = new ArrayList<>();
    /** facción (= id de región) → [x, y] en el sistema de coordenadas de mapamundi.png (1600×1200). */
    private Map<String, int[]> marcadores = new LinkedHashMap<>();
    /** Puntos personalizados: localizaciones que el director crea a mano y vincula a
     * historias y/o aventuras que no encajan (o no encajan solas) en una de las facciones. */
    private List<Punto> puntos = new ArrayList<>();
    /** Ciudades del mundo: asentamientos con nación, tamaño, población y rasgos, situados
     * a mano (o generados al azar) desde el Creador de ciudades del editor del mapa. */
    private List<Ciudad> ciudades = new ArrayList<>();
    /** Zonas y accidentes del terreno: montañas, bosques, lagos, ruinas, fortalezas...
     * Mismo flujo que las ciudades (colocar con clic, Alt+clic para generar al azar). */
    private List<Zona> zonas = new ArrayList<>();
    /** Cronología del mundo: eventos históricos con año, era y lugar. Se editan desde la
     * sección "Cronología" de /mapa y viajan en el mismo geografia.json. */
    private List<EventoHistorico> eventosHistoricos = new ArrayList<>();

    public List<Nacion> getNaciones() { return naciones; }
    public void setNaciones(List<Nacion> naciones) { this.naciones = naciones; }
    public Map<String, int[]> getMarcadores() { return marcadores; }
    public void setMarcadores(Map<String, int[]> marcadores) { this.marcadores = marcadores; }
    public List<Punto> getPuntos() { return puntos; }
    public void setPuntos(List<Punto> puntos) { this.puntos = puntos; }
    public List<Ciudad> getCiudades() { return ciudades; }
    public void setCiudades(List<Ciudad> ciudades) { this.ciudades = ciudades; }
    public List<Zona> getZonas() { return zonas; }
    public void setZonas(List<Zona> zonas) { this.zonas = zonas; }
    public List<EventoHistorico> getEventosHistoricos() { return eventosHistoricos; }
    public void setEventosHistoricos(List<EventoHistorico> eventosHistoricos) { this.eventosHistoricos = eventosHistoricos; }

    /**
     * Un lugar o accidente del terreno: montaña, bosque, lago, río, desierto, pantano,
     * ruina, fortaleza, santuario, puerto, isla o paso. El tipo determina el icono; el
     * peligro es texto de mesa ("nidos de arpía", "avalanchas en deshielo").
     */
    public static class Zona {
        private String id;
        private String nombre;
        private String tipo = "montana";
        private String nacion;
        private String peligro;
        private String descripcion;
        private int x;
        private int y;
        private List<String> historiaIds = new ArrayList<>();

        public String getId() { return id; }
        public void setId(String id) { this.id = id; }
        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
        public String getTipo() { return tipo; }
        public void setTipo(String tipo) { this.tipo = tipo; }
        public String getNacion() { return nacion; }
        public void setNacion(String nacion) { this.nacion = nacion; }
        public String getPeligro() { return peligro; }
        public void setPeligro(String peligro) { this.peligro = peligro; }
        public String getDescripcion() { return descripcion; }
        public void setDescripcion(String descripcion) { this.descripcion = descripcion; }
        public int getX() { return x; }
        public void setX(int x) { this.x = x; }
        public int getY() { return y; }
        public void setY(int y) { this.y = y; }
        public List<String> getHistoriaIds() { return historiaIds; }
        public void setHistoriaIds(List<String> historiaIds) { this.historiaIds = historiaIds; }
    }

    /**
     * Un evento histórico del mundo: qué pasó, cuándo (año + era) y dónde (texto libre
     * con datalist de ciudades/zonas/naciones). Las consecuencias son lo jugable: qué
     * quedó del suceso que la mesa aún pueda tocar.
     */
    public static class EventoHistorico {
        private String id;
        private String titulo;
        private int ano;
        private String era = "";
        private String lugar = "";
        private String descripcion = "";
        private String consecuencias = "";

        public String getId() { return id; }
        public void setId(String id) { this.id = id; }
        public String getTitulo() { return titulo; }
        public void setTitulo(String titulo) { this.titulo = titulo; }
        public int getAno() { return ano; }
        public void setAno(int ano) { this.ano = ano; }
        public String getEra() { return era; }
        public void setEra(String era) { this.era = era; }
        public String getLugar() { return lugar; }
        public void setLugar(String lugar) { this.lugar = lugar; }
        public String getDescripcion() { return descripcion; }
        public void setDescripcion(String descripcion) { this.descripcion = descripcion; }
        public String getConsecuencias() { return consecuencias; }
        public void setConsecuencias(String consecuencias) { this.consecuencias = consecuencias; }
    }

    /**
     * Una ciudad del mundo. El tamaño determina el icono y da un orden de magnitud de
     * población; el resto (rasgos, gobernante, historias/pnjs vinculados) es sabor de mesa.
     */
    public static class Ciudad {
        private String id;
        private String nombre;
        private String nacion;               // nombre de la nación (coincide con Nacion.nombre)
        private String tamano = "ciudad";    // capital | ciudad | pueblo | aldea
        private String poblacion;            // texto libre ("~12.000", "unas 300 almas")
        private String gobernante;
        private String rasgo;                // rasgo distintivo ("puerto de contrabando", "ruinas bajo la plaza")
        private String descripcion;
        private int x;
        private int y;
        private List<String> historiaIds = new ArrayList<>();
        private List<String> npcIds = new ArrayList<>();

        public Ciudad() { }

        public String getId() { return id; }
        public void setId(String id) { this.id = id; }
        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
        public String getNacion() { return nacion; }
        public void setNacion(String nacion) { this.nacion = nacion; }
        public String getTamano() { return tamano; }
        public void setTamano(String tamano) { this.tamano = tamano; }
        public String getPoblacion() { return poblacion; }
        public void setPoblacion(String poblacion) { this.poblacion = poblacion; }
        public String getGobernante() { return gobernante; }
        public void setGobernante(String gobernante) { this.gobernante = gobernante; }
        public String getRasgo() { return rasgo; }
        public void setRasgo(String rasgo) { this.rasgo = rasgo; }
        public String getDescripcion() { return descripcion; }
        public void setDescripcion(String descripcion) { this.descripcion = descripcion; }
        public int getX() { return x; }
        public void setX(int x) { this.x = x; }
        public int getY() { return y; }
        public void setY(int y) { this.y = y; }
        public List<String> getHistoriaIds() { return historiaIds; }
        public void setHistoriaIds(List<String> historiaIds) { this.historiaIds = historiaIds; }
        public List<String> getNpcIds() { return npcIds; }
        public void setNpcIds(List<String> npcIds) { this.npcIds = npcIds; }

        /** Icono según el tamaño — decisión visual, no se guarda en el JSON. */
        public String getIcono() {
            if (tamano == null) return "🏘";      // 🏘
            return switch (tamano) {
                case "capital" -> "🏰";           // 🏰
                case "ciudad" -> "🏙";            // 🏙
                case "pueblo" -> "🏘";            // 🏘
                case "aldea" -> "🏡";             // 🏡
                default -> "🏘";
            };
        }
    }

    public static class Punto {
        private String id;
        private String nombre;
        private String icono;
        private int x;
        private int y;
        private List<String> historiaIds = new ArrayList<>();
        private List<String> aventuraIds = new ArrayList<>();
        private List<String> eventoIds = new ArrayList<>();

        public Punto() { }

        public String getId() { return id; }
        public void setId(String id) { this.id = id; }
        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
        public String getIcono() { return icono; }
        public void setIcono(String icono) { this.icono = icono; }
        public int getX() { return x; }
        public void setX(int x) { this.x = x; }
        public int getY() { return y; }
        public void setY(int y) { this.y = y; }
        public List<String> getHistoriaIds() { return historiaIds; }
        public void setHistoriaIds(List<String> historiaIds) { this.historiaIds = historiaIds; }
        public List<String> getAventuraIds() { return aventuraIds; }
        public void setAventuraIds(List<String> aventuraIds) { this.aventuraIds = aventuraIds; }
        public List<String> getEventoIds() { return eventoIds; }
        public void setEventoIds(List<String> eventoIds) { this.eventoIds = eventoIds; }
    }

    public static class Nacion {
        private String nombre;
        private String color;
        /** "x1,y1 x2,y2 ..." — mismo formato que el atributo points de un <polygon> SVG. */
        private String points;
        // ── Identidad de worldbuilding (opcional, editable desde el panel de /mapa) ──
        private String cultura = "";
        private String alineacion = "";
        private String curiosidades = "";
        /** Deidades veneradas aquí — ids del catálogo de deidades, nunca texto libre. */
        private List<String> deidadIds = new ArrayList<>();
        /** Razas predominantes — ids del catálogo de razas, nunca texto libre. */
        private List<String> razaIds = new ArrayList<>();

        public Nacion() { }

        public Nacion(String nombre, String color, String points) {
            this.nombre = nombre;
            this.color = color;
            this.points = points;
        }

        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
        public String getColor() { return color; }
        public void setColor(String color) { this.color = color; }
        public String getPoints() { return points; }
        public void setPoints(String points) { this.points = points; }
        public String getCultura() { return cultura; }
        public void setCultura(String cultura) { this.cultura = cultura; }
        public String getAlineacion() { return alineacion; }
        public void setAlineacion(String alineacion) { this.alineacion = alineacion; }
        public String getCuriosidades() { return curiosidades; }
        public void setCuriosidades(String curiosidades) { this.curiosidades = curiosidades; }
        public List<String> getDeidadIds() { return deidadIds; }
        public void setDeidadIds(List<String> deidadIds) { this.deidadIds = deidadIds; }
        public List<String> getRazaIds() { return razaIds; }
        public void setRazaIds(List<String> razaIds) { this.razaIds = razaIds; }
    }
}
