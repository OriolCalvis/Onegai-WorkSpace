package cat.dnd.cc.eines.mazo;

import java.util.ArrayList;
import java.util.List;

/**
 * UNA carta del mazo físico de una clase, ya traducida a lo que se imprime: título, línea de
 * tipo, fichas, la línea de mecánica, el texto de reglas y el pie. Es la misma carta para el
 * PDF ({@link PdfCartas}) y para la lista de la ficha de la clase en la web, así que lo que se
 * ve en pantalla es lo que sale en papel.
 * <p>
 * CARTAS-CLASES-01 (08/10/2026): Oriol quiere descargar las cartas de habilidad para jugar las
 * clases en el juego de rol de mesa.
 */
public final class CartaFisica {

    /** De qué es la carta: decide el color del marco y la etiqueta. */
    public enum Tipo {
        CLASE("Clase", 0x5b4526),
        PASIVA("Pasiva de clase", 0x4b2e83),
        HABILIDAD("Habilidad", 0x2f4f6f),
        CREADOR("Carta de Creador", 0x8a1c1c),
        HECHIZO("Hechizo", 0x1f5f5b),
        INVOCACION("Invocación", 0x2e5d34),
        EQUIPO("Equipo", 0x6b4a2b),
        REGLA("Reglas de mesa", 0x3a3a3a);

        private final String etiqueta;
        private final int color;

        Tipo(String etiqueta, int color) {
            this.etiqueta = etiqueta;
            this.color = color;
        }

        public String getEtiqueta() {
            return etiqueta;
        }

        /** Color del marco, 0xRRGGBB. */
        public int getColor() {
            return color;
        }

        /** El mismo color en CSS (#rrggbb), para la web. */
        public String getColorCss() {
            return String.format("#%06x", color);
        }
    }

    private final Tipo tipo;
    private final String titulo;
    private String id = "";
    private int tier = 1;
    private String linea = "";
    private String elemento;
    private Integer colorElemento;
    private final List<String> fichas = new ArrayList<>();
    private String mecanica;
    private final List<String> datos = new ArrayList<>();
    private String texto = "";
    private String ambientacion;
    private String aviso;
    private String pie = "";

    public CartaFisica(Tipo tipo, String titulo) {
        this.tipo = tipo;
        this.titulo = titulo == null ? "" : titulo;
    }

    public Tipo getTipo() {
        return tipo;
    }

    public String getTitulo() {
        return titulo;
    }

    public String getId() {
        return id;
    }

    public CartaFisica id(String id) {
        this.id = id == null ? "" : id;
        return this;
    }

    /** Tier mínimo de la carta (0..5); las de reglas llevan 0 y no pintan la gema. */
    public int getTier() {
        return tier;
    }

    public CartaFisica tier(int tier) {
        this.tier = tier;
        return this;
    }

    /** La línea bajo el título: «Habilidad · Acción principal», «Creador · Tier 2 · Reacción»... */
    public String getLinea() {
        return linea;
    }

    public CartaFisica linea(String linea) {
        this.linea = linea == null ? "" : linea;
        return this;
    }

    /** El elemento de la carta (Fuego, Agua, Rayo...), o null si es física, de apoyo o utilidad. */
    public String getElemento() {
        return elemento;
    }

    public Integer getColorElemento() {
        return colorElemento;
    }

    public String getColorElementoCss() {
        return colorElemento == null ? null : String.format("#%06x", colorElemento);
    }

    public CartaFisica elemento(String elemento, Integer color) {
        this.elemento = elemento;
        this.colorElemento = color;
        return this;
    }

    /** Fichas cortas: a quién apunta, alcance, duración, a qué pila va. */
    public List<String> getFichas() {
        return fichas;
    }

    public CartaFisica ficha(String ficha) {
        if (ficha != null && !ficha.isBlank()) {
            fichas.add(ficha);
        }
        return this;
    }

    /** Una línea con lo que se tira: «Tira 1 + DES · Salvación CAR · 2d6 · 2 rondas». */
    public String getMecanica() {
        return mecanica;
    }

    public CartaFisica mecanica(String mecanica) {
        this.mecanica = (mecanica == null || mecanica.isBlank()) ? null : mecanica;
        return this;
    }

    /** Líneas de datos (la carta de clase, la invocación, el equipo): «Vida base 5 · ...». */
    public List<String> getDatos() {
        return datos;
    }

    public CartaFisica dato(String dato) {
        if (dato != null && !dato.isBlank()) {
            datos.add(dato);
        }
        return this;
    }

    /** El texto de reglas, tal cual lo dice la carta. */
    public String getTexto() {
        return texto;
    }

    public CartaFisica texto(String texto) {
        this.texto = texto == null ? "" : texto.strip();
        return this;
    }

    public String getAmbientacion() {
        return ambientacion;
    }

    public CartaFisica ambientacion(String ambientacion) {
        this.ambientacion = (ambientacion == null || ambientacion.isBlank()) ? null : ambientacion.strip();
        return this;
    }

    /** Lo que la carta todavía no cierra («Por definir: ...»), o null. */
    public String getAviso() {
        return aviso;
    }

    public CartaFisica aviso(String aviso) {
        this.aviso = (aviso == null || aviso.isBlank()) ? null : aviso;
        return this;
    }

    /** El pie: de qué clase es. */
    public String getPie() {
        return pie;
    }

    public CartaFisica pie(String pie) {
        this.pie = pie == null ? "" : pie;
        return this;
    }
}
