package cat.dnd.cc.eines.mazo;

import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.zip.Deflater;

/**
 * El PDF de los mazos físicos: hojas A4 con 9 cartas de 63,5 × 88,9 mm (3 × 3, sin
 * separación, para cortar de un tajo entre carta y carta) y marcas de corte en el margen.
 * <p>
 * CARTAS-CLASES-01 (08/10/2026). Escrito a mano, sin librerías: el proyecto no tenía ninguna
 * de PDF ({@code PdfService} escribe un PDF mínimo de dos líneas) y una dependencia nueva solo
 * para esto no merecía la pena. Usa las fuentes base del PDF (Helvetica y Times), que no se
 * incrustan y todo lector tiene; por eso el texto va en WinAnsi (castellano y catalán sin
 * problema; lo que no cabe en WinAnsi, como las flechas o los emoji, se cambia por texto).
 * <p>
 * Cada mazo empieza en hoja nueva, así se pueden imprimir varias clases de una vez y separar
 * luego las hojas de cada una.
 */
public final class PdfCartas {

    /** Un mazo: el nombre que va en el pie de sus hojas y sus cartas, en orden. */
    public record Mazo(String titulo, List<CartaFisica> cartas) {
    }

    // A4 y la carta, en puntos (1 mm = 72 / 25,4 pt)
    static final float ANCHO_PAGINA = 595.276f;
    static final float ALTO_PAGINA = 841.89f;
    static final float ANCHO_CARTA = 180f;   // 63,5 mm
    static final float ALTO_CARTA = 252f;    // 88,9 mm
    static final int COLUMNAS = 3;
    static final int FILAS = 3;
    public static final int CARTAS_POR_HOJA = COLUMNAS * FILAS;

    private static final float X0 = (ANCHO_PAGINA - COLUMNAS * ANCHO_CARTA) / 2f;
    private static final float Y0 = (ALTO_PAGINA - FILAS * ALTO_CARTA) / 2f;

    // Fuentes base (nombre del recurso en la página -> métricas)
    enum Fuente {
        HELVETICA("F1", "Helvetica", MetricasPdf.HELVETICA),
        HELVETICA_NEGRITA("F2", "Helvetica-Bold", MetricasPdf.HELVETICA_BOLD),
        HELVETICA_CURSIVA("F3", "Helvetica-Oblique", MetricasPdf.HELVETICA_OBLIQUE),
        TIMES_NEGRITA("F4", "Times-Bold", MetricasPdf.TIMES_BOLD),
        TIMES_CURSIVA("F5", "Times-Italic", MetricasPdf.TIMES_ITALIC),
        TIMES("F6", "Times-Roman", MetricasPdf.TIMES_ROMAN);

        final String recurso;
        final String base;
        final int[] anchos;

        Fuente(String recurso, String base, int[] anchos) {
            this.recurso = recurso;
            this.base = base;
            this.anchos = anchos;
        }
    }

    // Colores fijos
    private static final int PERGAMINO = 0xf6ecd2;
    private static final int PAPEL = 0xfffaf0;
    private static final int TINTA = 0x2b1d10;
    private static final int TINTA_SUAVE = 0x6b5a45;
    private static final int CREMA = 0xfbf5e6;
    private static final int AVISO = 0xa01818;
    private static final int ORO = 0xc9a227;
    private static final int[] COLOR_TIER = {0x7a6a55, 0x5f8f4e, 0x3d7fc4, 0xc9861a, 0x8b3fd1, 0xc62839};

    private PdfCartas() {
    }

    // ================================================================ API

    public static byte[] generar(List<Mazo> mazos) {
        List<byte[]> paginas = new ArrayList<>();
        for (Mazo mazo : mazos) {
            List<CartaFisica> cartas = mazo.cartas();
            int hojas = Math.max(1, (cartas.size() + CARTAS_POR_HOJA - 1) / CARTAS_POR_HOJA);
            for (int h = 0; h < hojas; h++) {
                Lienzo l = new Lienzo();
                int desde = h * CARTAS_POR_HOJA;
                int hasta = Math.min(cartas.size(), desde + CARTAS_POR_HOJA);
                for (int i = desde; i < hasta; i++) {
                    int pos = i - desde;
                    int col = pos % COLUMNAS;
                    int fila = pos / COLUMNAS;
                    float x = X0 + col * ANCHO_CARTA;
                    float y = Y0 + (FILAS - 1 - fila) * ALTO_CARTA;
                    dibujaCarta(l, cartas.get(i), x, y);
                }
                marcasDeCorte(l);
                pieDePagina(l, mazo.titulo(), h + 1, hojas);
                paginas.add(l.bytes());
            }
        }
        return documento(paginas, mazos.size() == 1 ? mazos.get(0).titulo() : "Mazos de clase");
    }

    public static byte[] generar(String titulo, List<CartaFisica> cartas) {
        return generar(List.of(new Mazo(titulo, cartas)));
    }

    // ================================================================ la carta

    private static void dibujaCarta(Lienzo l, CartaFisica c, float x, float y) {
        final float w = ANCHO_CARTA;
        final float h = ALTO_CARTA;
        final int color = c.getTipo().getColor();
        final boolean creador = c.getTipo() == CartaFisica.Tipo.CREADOR;
        final boolean regla = c.getTipo() == CartaFisica.Tipo.REGLA;

        // Fondo y línea de corte
        l.relleno(PERGAMINO);
        l.rect(x, y, w, h);
        l.f();
        l.trazo(0xbdb4a2, 0.35f);
        l.rect(x, y, w, h);
        l.s();

        // Marco
        final float m = 6f;
        final float mx = x + m;
        final float my = y + m;
        final float mw = w - 2 * m;
        final float mh = h - 2 * m;
        l.trazo(creador ? ORO : color, creador ? 2.2f : 1.4f);
        l.rectRedondo(mx, my, mw, mh, 6f);
        l.s();

        // Cabecera: el título en una banda del color del tipo
        final float pad = 6f;
        final float gema = regla ? 0f : 19f;
        final float anchoTitulo = mw - 2 * pad - (gema > 0 ? gema + 3f : 0f);
        Ajuste titulo = ajustar(c.getTitulo(), Fuente.TIMES_NEGRITA, anchoTitulo, 2, 11.5f, 7.5f, 1.08f);
        final float altoBanda = Math.max(22f, titulo.lineas.size() * titulo.tamano * titulo.interlineado + 10f);
        final float topeBanda = my + mh;
        l.relleno(color);
        l.rectRedondoArriba(mx, topeBanda - altoBanda, mw, altoBanda, 6f);
        l.f();
        if (creador) {
            l.relleno(ORO);
            l.rect(mx, topeBanda - altoBanda - 1.6f, mw, 1.6f);
            l.f();
        }
        float ty = topeBanda - (altoBanda - titulo.lineas.size() * titulo.tamano * titulo.interlineado) / 2f
                - titulo.tamano * 0.82f;
        for (String linea : titulo.lineas) {
            l.texto(Fuente.TIMES_NEGRITA, titulo.tamano, CREMA, mx + pad, ty, linea);
            ty -= titulo.tamano * titulo.interlineado;
        }
        if (gema > 0) {
            float cx = mx + mw - pad - gema / 2f;
            float cy = topeBanda - altoBanda / 2f;
            int t = Math.max(0, Math.min(5, c.getTier()));
            l.relleno(COLOR_TIER[t]);
            l.trazo(CREMA, 0.9f);
            l.circulo(cx, cy, gema / 2f);
            l.b();
            String et = "T" + t;
            float tw = ancho(et, Fuente.HELVETICA_NEGRITA, 7.5f);
            l.texto(Fuente.HELVETICA_NEGRITA, 7.5f, CREMA, cx - tw / 2f, cy - 2.6f, et);
        }

        // Línea de tipo (con el elemento a la derecha, si lo tiene)
        float cursor = topeBanda - altoBanda - (creador ? 1.6f : 0f);
        final float altoLinea = 11f;
        l.relleno(mezcla(color, PERGAMINO, 0.18f));
        l.rect(mx, cursor - altoLinea, mw, altoLinea);
        l.f();
        float anchoElemento = 0f;
        if (c.getElemento() != null) {
            String el = c.getElemento();
            float ew = ancho(el, Fuente.HELVETICA_CURSIVA, 5.6f);
            anchoElemento = ew + 11f;
            float ex = mx + mw - pad - ew;
            l.texto(Fuente.HELVETICA_CURSIVA, 5.6f, TINTA, ex, cursor - altoLinea + 3.6f, el);
            l.relleno(c.getColorElemento() != null ? c.getColorElemento() : 0x999999);
            l.trazo(TINTA, 0.4f);
            l.circulo(ex - 5.5f, cursor - altoLinea / 2f, 2.8f);
            l.b();
        }
        String tipoLinea = recorta(c.getLinea(), Fuente.HELVETICA_NEGRITA, 5.9f, mw - 2 * pad - anchoElemento);
        l.texto(Fuente.HELVETICA_NEGRITA, 5.9f, TINTA, mx + pad, cursor - altoLinea + 3.6f, tipoLinea);
        cursor -= altoLinea + 4f;

        // Fichas
        if (!c.getFichas().isEmpty()) {
            cursor = fichas(l, c.getFichas(), color, mx + pad - 1f, cursor, mw - 2 * pad + 2f);
        }

        // Mecánica
        if (c.getMecanica() != null) {
            Ajuste mec = ajustar(c.getMecanica(), Fuente.HELVETICA_NEGRITA, mw - 2 * pad - 6f, 3, 6.3f, 5.2f, 1.18f);
            float alto = mec.lineas.size() * mec.tamano * mec.interlineado + 5f;
            l.relleno(mezcla(color, PAPEL, 0.12f));
            l.trazo(mezcla(color, PAPEL, 0.55f), 0.5f);
            l.rect(mx + pad - 1f, cursor - alto, mw - 2 * pad + 2f, alto);
            l.b();
            float yy = cursor - 2.5f - mec.tamano * 0.82f;
            for (String linea : mec.lineas) {
                l.texto(Fuente.HELVETICA_NEGRITA, mec.tamano, TINTA, mx + pad + 2f, yy, linea);
                yy -= mec.tamano * mec.interlineado;
            }
            cursor -= alto + 4f;
        }

        // Pie: de qué clase es y el id
        final float altoPie = 9f;
        final float suelo = my + altoPie;
        String id = c.getId();
        float anchoId = 0f;
        if (id != null && !id.isBlank()) {
            String idCorto = recorta(id, Fuente.HELVETICA, 4.4f, mw * 0.45f);
            anchoId = ancho(idCorto, Fuente.HELVETICA, 4.4f);
            l.texto(Fuente.HELVETICA, 4.4f, TINTA_SUAVE, mx + mw - pad - anchoId, my + 3.4f, idCorto);
        }
        String pie = recorta(c.getPie(), Fuente.HELVETICA_CURSIVA, 5f, mw - 2 * pad - anchoId - 6f);
        l.texto(Fuente.HELVETICA_CURSIVA, 5f, TINTA_SUAVE, mx + pad, my + 3.4f, pie);

        // Caja de reglas: los datos, el texto, la ambientación y el aviso
        float cajaArriba = cursor;
        float cajaAbajo = suelo + 1f;
        float anchoCaja = mw - 2 * pad + 2f;
        l.relleno(PAPEL);
        l.trazo(mezcla(color, PAPEL, 0.5f), 0.5f);
        l.rect(mx + pad - 1f, cajaAbajo, anchoCaja, cajaArriba - cajaAbajo);
        l.b();
        cajaReglas(l, c, mx + pad + 2.5f, cajaArriba - 3f, anchoCaja - 7f, cajaArriba - cajaAbajo - 6f);
    }

    /** Las fichas en filas; devuelve dónde queda el cursor. */
    private static float fichas(Lienzo l, List<String> fichas, int color, float x, float y, float ancho) {
        final float tam = 5.6f;
        final float alto = 9f;
        final float hueco = 2.5f;
        float cx = x;
        float cy = y;
        int filas = 1;
        for (String f : fichas) {
            String txt = recorta(f, Fuente.HELVETICA, tam, ancho - 8f);
            float fw = ancho(txt, Fuente.HELVETICA, tam) + 7f;
            if (cx + fw > x + ancho && cx > x) {
                if (filas == 2) break;
                filas++;
                cx = x;
                cy -= alto + hueco;
            }
            l.relleno(PAPEL);
            l.trazo(color, 0.5f);
            l.rectRedondo(cx, cy - alto, fw, alto, 3.5f);
            l.b();
            l.texto(Fuente.HELVETICA, tam, TINTA, cx + 3.5f, cy - alto + 2.9f, txt);
            cx += fw + hueco;
        }
        return cy - alto - 4f;
    }

    /**
     * Lo que va dentro de la caja de reglas, encogiendo la letra hasta que quepa todo:
     * primero los datos (una línea cada uno), luego el texto, y la ambientación y el aviso
     * debajo. La ambientación es lo primero que se deja fuera si no hay sitio.
     */
    private static void cajaReglas(Lienzo l, CartaFisica c, float x, float arriba, float ancho, float alto) {
        List<String> datos = c.getDatos();
        String texto = c.getTexto();
        String amb = c.getAmbientacion();
        String aviso = c.getAviso();

        for (int intento = 0; intento < 2; intento++) {
            boolean conAmbientacion = intento == 0 && amb != null;
            for (float tam = 9.4f; tam >= 5.0f; tam -= 0.2f) {
                Bloques b = componer(datos, texto, conAmbientacion ? amb : null, aviso, ancho, tam);
                if (b.alto <= alto) {
                    pinta(l, b, x, arriba);
                    return;
                }
            }
        }
        // No cabe ni a 5 pt: se corta el texto con puntos suspensivos
        Bloques b = componer(datos, texto, null, aviso, ancho, 5.0f);
        while (b.alto > alto && !b.lineas.isEmpty()) {
            b.recortarTexto();
        }
        pinta(l, b, x, arriba);
    }

    private static final class Linea {
        final Fuente fuente;
        final float tam;
        final int color;
        final String texto;
        final float salto;
        final boolean deTexto;
        final String prefijo;   // la etiqueta en negrita de un dato («• Vida:»), o null
        final float sangria;    // cuánto entra la línea (las de continuación de un dato)

        Linea(Fuente fuente, float tam, int color, String texto, float salto, boolean deTexto) {
            this(fuente, tam, color, texto, salto, deTexto, null, 0f);
        }

        Linea(Fuente fuente, float tam, int color, String texto, float salto, boolean deTexto,
              String prefijo, float sangria) {
            this.fuente = fuente;
            this.tam = tam;
            this.color = color;
            this.texto = texto;
            this.salto = salto;
            this.deTexto = deTexto;
            this.prefijo = prefijo;
            this.sangria = sangria;
        }
    }

    private static final class Bloques {
        final List<Linea> lineas = new ArrayList<>();
        float alto;

        void add(Linea l) {
            lineas.add(l);
            alto += l.salto;
        }

        /** Quita la última línea del texto de reglas y pone «…» a la anterior. */
        void recortarTexto() {
            for (int i = lineas.size() - 1; i >= 0; i--) {
                if (lineas.get(i).deTexto) {
                    Linea quitada = lineas.remove(i);
                    alto -= quitada.salto;
                    for (int j = i - 1; j >= 0; j--) {
                        Linea prev = lineas.get(j);
                        if (prev.deTexto) {
                            String t = prev.texto.endsWith("…") ? prev.texto : prev.texto + "…";
                            lineas.set(j, new Linea(prev.fuente, prev.tam, prev.color, t, prev.salto, true));
                            break;
                        }
                    }
                    return;
                }
            }
            Linea quitada = lineas.remove(lineas.size() - 1);
            alto -= quitada.salto;
        }
    }

    private static Bloques componer(List<String> datos, String texto, String amb, String aviso, float ancho, float tam) {
        Bloques b = new Bloques();
        float tamDato = Math.min(7.6f, tam);
        for (String d : datos) {
            dato(b, d, tamDato, ancho);
        }
        if (!datos.isEmpty() && texto != null && !texto.isBlank()) {
            b.alto += tam * 0.45f;
            b.lineas.add(new Linea(Fuente.HELVETICA, tam, TINTA, "", tam * 0.45f, false));
        }
        if (texto != null && !texto.isBlank()) {
            for (String p : texto.split("\n")) {
                if (p.isBlank()) continue;
                for (String s : partir(p.strip(), Fuente.HELVETICA, tam, ancho)) {
                    b.add(new Linea(Fuente.HELVETICA, tam, TINTA, s, tam * 1.2f, true));
                }
                b.alto += tam * 0.25f;
                b.lineas.add(new Linea(Fuente.HELVETICA, tam, TINTA, "", tam * 0.25f, false));
            }
        }
        if (amb != null) {
            float ta = Math.max(5f, tam - 0.6f);
            for (String s : partir(amb, Fuente.TIMES_CURSIVA, ta, ancho)) {
                b.add(new Linea(Fuente.TIMES_CURSIVA, ta, TINTA_SUAVE, s, ta * 1.18f, false));
            }
        }
        if (aviso != null) {
            float tv = Math.max(5f, Math.min(6f, tam - 0.6f));
            b.alto += 2f;
            b.lineas.add(new Linea(Fuente.HELVETICA, tv, TINTA, "", 2f, false));
            for (String s : partir(aviso, Fuente.HELVETICA_NEGRITA, tv, ancho)) {
                b.add(new Linea(Fuente.HELVETICA_NEGRITA, tv, AVISO, s, tv * 1.2f, false));
            }
        }
        return b;
    }

    /**
     * Una viñeta de datos: «• Vida: 5 + CON×3...», con la etiqueta (lo que va antes de los dos
     * puntos, si es corta) en negrita, y las líneas siguientes con sangría.
     */
    private static void dato(Bloques b, String d, float tam, float ancho) {
        String limpio = limpiar(d);
        int dosPuntos = limpio.indexOf(':');
        boolean conEtiqueta = dosPuntos > 0 && dosPuntos <= 28;
        String etiqueta = conEtiqueta ? "• " + limpio.substring(0, dosPuntos + 1) : null;
        String resto = conEtiqueta ? limpio.substring(dosPuntos + 1).strip() : "• " + limpio;
        final float salto = tam * 1.2f;
        final float sangria = ancho("• ", Fuente.HELVETICA, tam);
        float anchoPrimera = conEtiqueta ? ancho - ancho(etiqueta + " ", Fuente.HELVETICA_NEGRITA, tam) : ancho;
        List<String> primera = partir(resto, Fuente.HELVETICA, tam, Math.max(20f, anchoPrimera));
        if (primera.isEmpty()) {
            b.add(new Linea(Fuente.HELVETICA, tam, TINTA, "", salto, false, etiqueta, 0f));
        } else {
            String l1 = primera.get(0);
            b.add(new Linea(Fuente.HELVETICA, tam, TINTA, l1, salto, false, etiqueta, 0f));
            // Lo que no cupo en la primera línea, partido otra vez con la sangría
            String[] palabras = resto.split(" ");
            int usadas = l1.split(" ").length;
            List<String> siguientes;
            if (usadas < palabras.length && !l1.endsWith("-")) {
                String siguiente = String.join(" ", java.util.Arrays.copyOfRange(palabras, usadas, palabras.length));
                siguientes = partir(siguiente, Fuente.HELVETICA, tam, ancho - sangria);
            } else {
                siguientes = primera.subList(1, primera.size());
            }
            for (String s : siguientes) {
                b.add(new Linea(Fuente.HELVETICA, tam, TINTA, s, salto, false, null, sangria));
            }
        }
        b.alto += tam * 0.3f;
        b.lineas.add(new Linea(Fuente.HELVETICA, tam, TINTA, "", tam * 0.3f, false));
    }

    private static void pinta(Lienzo l, Bloques b, float x, float arriba) {
        float y = arriba;
        for (Linea ln : b.lineas) {
            float xx = x + ln.sangria;
            if (ln.prefijo != null) {
                l.texto(Fuente.HELVETICA_NEGRITA, ln.tam, ln.color, xx, y - ln.tam * 0.85f, ln.prefijo);
                xx += ancho(ln.prefijo + " ", Fuente.HELVETICA_NEGRITA, ln.tam);
            }
            if (!ln.texto.isEmpty()) {
                l.texto(ln.fuente, ln.tam, ln.color, xx, y - ln.tam * 0.85f, ln.texto);
            }
            y -= ln.salto;
        }
    }

    // ================================================================ la hoja

    private static void marcasDeCorte(Lienzo l) {
        l.trazo(0x8a8a8a, 0.4f);
        float ancho = COLUMNAS * ANCHO_CARTA;
        float alto = FILAS * ALTO_CARTA;
        for (int i = 0; i <= COLUMNAS; i++) {
            float x = X0 + i * ANCHO_CARTA;
            l.linea(x, Y0 + alto + 3f, x, Y0 + alto + 14f);
            l.linea(x, Y0 - 3f, x, Y0 - 14f);
        }
        for (int j = 0; j <= FILAS; j++) {
            float y = Y0 + j * ALTO_CARTA;
            l.linea(X0 - 3f, y, X0 - 14f, y);
            l.linea(X0 + ancho + 3f, y, X0 + ancho + 14f, y);
        }
        l.s();
    }

    private static void pieDePagina(Lienzo l, String titulo, int hoja, int hojas) {
        String t = "ONEGAI · " + titulo + " · hoja " + hoja + " de " + hojas
                + " · Imprime a tamaño real (100 %, sin «ajustar a la página») en A4: cartas de 63,5 × 88,9 mm";
        t = recorta(t, Fuente.HELVETICA, 6.5f, ANCHO_PAGINA - 2 * X0);
        float w = ancho(t, Fuente.HELVETICA, 6.5f);
        l.texto(Fuente.HELVETICA, 6.5f, 0x777777, (ANCHO_PAGINA - w) / 2f, 14f, t);
    }

    // ================================================================ texto

    static final class Ajuste {
        final List<String> lineas;
        final float tamano;
        final float interlineado;

        Ajuste(List<String> lineas, float tamano, float interlineado) {
            this.lineas = lineas;
            this.tamano = tamano;
            this.interlineado = interlineado;
        }
    }

    /** El tamaño más grande (de tamMax a tamMin) con el que el texto cabe en maxLineas. */
    static Ajuste ajustar(String texto, Fuente f, float ancho, int maxLineas, float tamMax, float tamMin, float inter) {
        for (float t = tamMax; t >= tamMin - 0.001f; t -= 0.25f) {
            List<String> ls = partir(texto, f, t, ancho);
            if (ls.size() <= maxLineas) return new Ajuste(ls, t, inter);
        }
        List<String> ls = partir(texto, f, tamMin, ancho);
        List<String> corte = new ArrayList<>(ls.subList(0, Math.min(maxLineas, ls.size())));
        if (ls.size() > maxLineas && !corte.isEmpty()) {
            int u = corte.size() - 1;
            corte.set(u, recorta(corte.get(u) + "…", f, tamMin, ancho));
        }
        return new Ajuste(corte, tamMin, inter);
    }

    /** Parte en líneas que no pasen de {@code ancho}; una palabra que no cabe sola se corta. */
    static List<String> partir(String texto, Fuente f, float tam, float ancho) {
        List<String> lineas = new ArrayList<>();
        if (texto == null || texto.isBlank()) return lineas;
        String limpio = limpiar(texto);
        StringBuilder actual = new StringBuilder();
        for (String palabra : limpio.split(" ")) {
            if (palabra.isEmpty()) continue;
            String prueba = actual.length() == 0 ? palabra : actual + " " + palabra;
            if (ancho(prueba, f, tam) <= ancho) {
                actual.setLength(0);
                actual.append(prueba);
                continue;
            }
            if (actual.length() > 0) {
                lineas.add(actual.toString());
                actual.setLength(0);
            }
            // La palabra sola no cabe: se parte por letras
            String resto = palabra;
            while (ancho(resto, f, tam) > ancho && resto.length() > 1) {
                int corte = resto.length() - 1;
                while (corte > 1 && ancho(resto.substring(0, corte) + "-", f, tam) > ancho) corte--;
                lineas.add(resto.substring(0, corte) + "-");
                resto = resto.substring(corte);
            }
            actual.append(resto);
        }
        if (actual.length() > 0) lineas.add(actual.toString());
        return lineas;
    }

    /** El texto que cabe en {@code ancho}, con «…» si se corta. */
    static String recorta(String texto, Fuente f, float tam, float ancho) {
        if (texto == null) return "";
        String t = limpiar(texto);
        if (ancho(t, f, tam) <= ancho) return t;
        while (t.length() > 1 && ancho(t + "…", f, tam) > ancho) t = t.substring(0, t.length() - 1);
        return t.strip() + "…";
    }

    static float ancho(String texto, Fuente f, float tam) {
        int total = 0;
        for (int i = 0; i < texto.length(); i++) {
            int c = codigoWinAnsi(texto.charAt(i));
            if (c >= 0) total += f.anchos[c];
        }
        return total * tam / 1000f;
    }

    /**
     * El código WinAnsi (cp1252) de un carácter, o -1 si no lo tiene. De 0x20 a 0x7E y de
     * 0xA0 a 0xFF coincide con Unicode; los 27 de 0x80-0x9F son las comillas, guiones, el
     * euro, los puntos suspensivos...
     */
    static int codigoWinAnsi(char ch) {
        if (ch >= 0x20 && ch < 0x7f) return ch;
        if (ch >= 0xa0 && ch <= 0xff) return ch;
        return switch (ch) {
            case '\u20ac' -> 0x80;
            case '\u201a' -> 0x82;
            case '\u0192' -> 0x83;
            case '\u201e' -> 0x84;
            case '\u2026' -> 0x85;
            case '\u2020' -> 0x86;
            case '\u2021' -> 0x87;
            case '\u02c6' -> 0x88;
            case '\u2030' -> 0x89;
            case '\u0160' -> 0x8a;
            case '\u2039' -> 0x8b;
            case '\u0152' -> 0x8c;
            case '\u017d' -> 0x8e;
            case '\u2018' -> 0x91;
            case '\u2019' -> 0x92;
            case '\u201c' -> 0x93;
            case '\u201d' -> 0x94;
            case '\u2022' -> 0x95;
            case '\u2013' -> 0x96;
            case '\u2014' -> 0x97;
            case '\u02dc' -> 0x98;
            case '\u2122' -> 0x99;
            case '\u0161' -> 0x9a;
            case '\u203a' -> 0x9b;
            case '\u0153' -> 0x9c;
            case '\u017e' -> 0x9e;
            case '\u0178' -> 0x9f;
            default -> -1;
        };
    }

    /** Espacios y caracteres que WinAnsi no tiene, cambiados por los que sí (o quitados). */
    static String limpiar(String texto) {
        String t = Normalizer.normalize(texto, Normalizer.Form.NFC)
                .replace("\u2192", "->").replace("\u2190", "<-").replace("\u2265", ">=").replace("\u2264", "<=")
                .replace('\u2212', '-').replace('\u2010', '-').replace('\u2011', '-');
        StringBuilder sb = new StringBuilder(t.length());
        boolean espacio = false;
        for (int i = 0; i < t.length(); i++) {
            char ch = t.charAt(i);
            if (Character.isWhitespace(ch) || ch == '\u00a0' || ch == '\u202f') {
                if (!espacio && sb.length() > 0) sb.append(' ');
                espacio = true;
                continue;
            }
            espacio = false;
            if (codigoWinAnsi(ch) >= 0) {
                sb.append(ch);
            } else if (Character.isLetter(ch)) {
                // Una letra con un acento que WinAnsi no tiene (ā, ő...): sin el acento
                String base = Normalizer.normalize(String.valueOf(ch), Normalizer.Form.NFD).replaceAll("\\p{M}", "");
                for (int k = 0; k < base.length(); k++) {
                    if (codigoWinAnsi(base.charAt(k)) >= 0) sb.append(base.charAt(k));
                }
            }
            // Lo demás (emoji, símbolos, pictogramas) se quita
        }
        int fin = sb.length();
        while (fin > 0 && sb.charAt(fin - 1) == ' ') fin--;
        return sb.substring(0, fin);
    }

    static byte[] winAnsi(String texto) {
        String t = limpiar(texto);
        byte[] b = new byte[t.length()];
        for (int i = 0; i < t.length(); i++) b[i] = (byte) codigoWinAnsi(t.charAt(i));
        return b;
    }

    // ================================================================ colores

    static int mezcla(int a, int b, float t) {
        int r = Math.round(((a >> 16) & 0xff) * t + ((b >> 16) & 0xff) * (1 - t));
        int g = Math.round(((a >> 8) & 0xff) * t + ((b >> 8) & 0xff) * (1 - t));
        int bl = Math.round((a & 0xff) * t + (b & 0xff) * (1 - t));
        return (r << 16) | (g << 8) | bl;
    }

    // ================================================================ el lienzo de una página

    /** Los operadores de dibujo de una página (el flujo de contenido del PDF). */
    static final class Lienzo {
        private final StringBuilder sb = new StringBuilder(16_384);
        private final ByteArrayOutputStream bytes = new ByteArrayOutputStream(16_384);

        private static String n(float v) {
            if (Math.abs(v - Math.round(v)) < 0.0005f) return Integer.toString(Math.round(v));
            return String.format(Locale.ROOT, "%.3f", v).replaceAll("0+$", "");
        }

        private static String rgb(int c) {
            return n(((c >> 16) & 0xff) / 255f) + " " + n(((c >> 8) & 0xff) / 255f) + " " + n((c & 0xff) / 255f);
        }

        private void op(String s) {
            sb.append(s).append('\n');
        }

        void relleno(int color) {
            op(rgb(color) + " rg");
        }

        void trazo(int color, float grosor) {
            op(rgb(color) + " RG " + n(grosor) + " w");
        }

        void rect(float x, float y, float w, float h) {
            op(n(x) + " " + n(y) + " " + n(w) + " " + n(h) + " re");
        }

        void linea(float x1, float y1, float x2, float y2) {
            op(n(x1) + " " + n(y1) + " m " + n(x2) + " " + n(y2) + " l");
        }

        void f() {
            op("f");
        }

        void s() {
            op("S");
        }

        void b() {
            op("B");
        }

        private static final float K = 0.5523f; // curva de Bézier para un cuarto de círculo

        void rectRedondo(float x, float y, float w, float h, float r) {
            float k = r * K;
            op(n(x + r) + " " + n(y) + " m");
            op(n(x + w - r) + " " + n(y) + " l");
            op(n(x + w - r + k) + " " + n(y) + " " + n(x + w) + " " + n(y + r - k) + " " + n(x + w) + " " + n(y + r) + " c");
            op(n(x + w) + " " + n(y + h - r) + " l");
            op(n(x + w) + " " + n(y + h - r + k) + " " + n(x + w - r + k) + " " + n(y + h) + " " + n(x + w - r) + " " + n(y + h) + " c");
            op(n(x + r) + " " + n(y + h) + " l");
            op(n(x + r - k) + " " + n(y + h) + " " + n(x) + " " + n(y + h - r + k) + " " + n(x) + " " + n(y + h - r) + " c");
            op(n(x) + " " + n(y + r) + " l");
            op(n(x) + " " + n(y + r - k) + " " + n(x + r - k) + " " + n(y) + " " + n(x + r) + " " + n(y) + " c");
            op("h");
        }

        /** Rectángulo con solo las esquinas de arriba redondeadas (la banda del título). */
        void rectRedondoArriba(float x, float y, float w, float h, float r) {
            float k = r * K;
            op(n(x) + " " + n(y) + " m");
            op(n(x + w) + " " + n(y) + " l");
            op(n(x + w) + " " + n(y + h - r) + " l");
            op(n(x + w) + " " + n(y + h - r + k) + " " + n(x + w - r + k) + " " + n(y + h) + " " + n(x + w - r) + " " + n(y + h) + " c");
            op(n(x + r) + " " + n(y + h) + " l");
            op(n(x + r - k) + " " + n(y + h) + " " + n(x) + " " + n(y + h - r + k) + " " + n(x) + " " + n(y + h - r) + " c");
            op("h");
        }

        void circulo(float cx, float cy, float r) {
            float k = r * K;
            op(n(cx + r) + " " + n(cy) + " m");
            op(n(cx + r) + " " + n(cy + k) + " " + n(cx + k) + " " + n(cy + r) + " " + n(cx) + " " + n(cy + r) + " c");
            op(n(cx - k) + " " + n(cy + r) + " " + n(cx - r) + " " + n(cy + k) + " " + n(cx - r) + " " + n(cy) + " c");
            op(n(cx - r) + " " + n(cy - k) + " " + n(cx - k) + " " + n(cy - r) + " " + n(cx) + " " + n(cy - r) + " c");
            op(n(cx + k) + " " + n(cy - r) + " " + n(cx + r) + " " + n(cy - k) + " " + n(cx + r) + " " + n(cy) + " c");
            op("h");
        }

        void texto(Fuente f, float tam, int color, float x, float y, String texto) {
            byte[] b = winAnsi(texto);
            if (b.length == 0) return;
            // El texto va en bytes (WinAnsi): se vuelca lo que hay en sb y se escribe en crudo.
            op(rgb(color) + " rg BT /" + f.recurso + " " + n(tam) + " Tf " + n(x) + " " + n(y) + " Td");
            vuelca();
            bytes.write('(');
            for (byte x1 : b) {
                int v = x1 & 0xff;
                if (v == '(' || v == ')' || v == '\\') {
                    bytes.write('\\');
                    bytes.write(v);
                } else if (v < 32 || v > 126) {
                    String oct = Integer.toOctalString(v);
                    bytes.write('\\');
                    for (int i = oct.length(); i < 3; i++) bytes.write('0');
                    bytes.writeBytes(oct.getBytes(StandardCharsets.US_ASCII));
                } else {
                    bytes.write(v);
                }
            }
            bytes.writeBytes(") Tj ET\n".getBytes(StandardCharsets.US_ASCII));
        }

        private void vuelca() {
            bytes.writeBytes(sb.toString().getBytes(StandardCharsets.US_ASCII));
            sb.setLength(0);
        }

        byte[] bytes() {
            vuelca();
            return bytes.toByteArray();
        }
    }

    // ================================================================ el documento

    private static byte[] documento(List<byte[]> paginas, String titulo) {
        Escritor w = new Escritor();
        int nPaginas = paginas.size();
        int primeraPagina = 10;
        // 1 catálogo, 2 páginas, 3..8 fuentes, 9 info; luego página + contenido por hoja
        w.objeto(1, "<< /Type /Catalog /Pages 2 0 R >>");
        StringBuilder kids = new StringBuilder();
        for (int i = 0; i < nPaginas; i++) kids.append(primeraPagina + 2 * i).append(" 0 R ");
        w.objeto(2, "<< /Type /Pages /Kids [" + kids.toString().trim() + "] /Count " + nPaginas + " >>");
        int obj = 3;
        StringBuilder fuentes = new StringBuilder();
        for (Fuente f : Fuente.values()) {
            w.objeto(obj, "<< /Type /Font /Subtype /Type1 /BaseFont /" + f.base + " /Encoding /WinAnsiEncoding >>");
            fuentes.append('/').append(f.recurso).append(' ').append(obj).append(" 0 R ");
            obj++;
        }
        w.objetoBytes(9, concatena("<< /Title ".getBytes(StandardCharsets.US_ASCII), cadenaPdf(titulo),
                " /Producer (ONEGAI dndWeebCC - mazo fisico) >>".getBytes(StandardCharsets.US_ASCII)));
        String recursos = "<< /Font << " + fuentes.toString().trim() + " >> >>";
        for (int i = 0; i < nPaginas; i++) {
            int pag = primeraPagina + 2 * i;
            int cont = pag + 1;
            w.objeto(pag, "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 " + Lienzo.n(ANCHO_PAGINA) + " "
                    + Lienzo.n(ALTO_PAGINA) + "] /Resources " + recursos + " /Contents " + cont + " 0 R >>");
            w.flujo(cont, comprime(paginas.get(i)));
        }
        return w.cierra(primeraPagina + 2 * nPaginas, 1, 9);
    }

    private static byte[] cadenaPdf(String texto) {
        ByteArrayOutputStream o = new ByteArrayOutputStream();
        o.write('(');
        for (byte x : winAnsi(texto)) {
            int v = x & 0xff;
            if (v == '(' || v == ')' || v == '\\') {
                o.write('\\');
                o.write(v);
            } else if (v < 32 || v > 126) {
                String oct = Integer.toOctalString(v);
                o.write('\\');
                for (int i = oct.length(); i < 3; i++) o.write('0');
                o.writeBytes(oct.getBytes(StandardCharsets.US_ASCII));
            } else {
                o.write(v);
            }
        }
        o.write(')');
        return o.toByteArray();
    }

    private static byte[] concatena(byte[]... partes) {
        ByteArrayOutputStream o = new ByteArrayOutputStream();
        for (byte[] p : partes) o.writeBytes(p);
        return o.toByteArray();
    }

    private static byte[] comprime(byte[] datos) {
        Deflater d = new Deflater(Deflater.BEST_COMPRESSION);
        d.setInput(datos);
        d.finish();
        ByteArrayOutputStream o = new ByteArrayOutputStream(datos.length / 3 + 64);
        byte[] buf = new byte[8192];
        while (!d.finished()) {
            int n = d.deflate(buf);
            o.write(buf, 0, n);
        }
        d.end();
        return o.toByteArray();
    }

    /** Escribe los objetos del PDF y apunta dónde empieza cada uno (la tabla xref). */
    private static final class Escritor {
        private final ByteArrayOutputStream o = new ByteArrayOutputStream(64 * 1024);
        private final java.util.Map<Integer, Integer> offsets = new java.util.TreeMap<>();

        Escritor() {
            ascii("%PDF-1.4\n");
            o.write('%');
            o.writeBytes(new byte[]{(byte) 0xe2, (byte) 0xe3, (byte) 0xcf, (byte) 0xd3});
            ascii("\n");
        }

        private void ascii(String s) {
            o.writeBytes(s.getBytes(StandardCharsets.US_ASCII));
        }

        void objeto(int n, String cuerpo) {
            offsets.put(n, o.size());
            ascii(n + " 0 obj\n" + cuerpo + "\nendobj\n");
        }

        void objetoBytes(int n, byte[] cuerpo) {
            offsets.put(n, o.size());
            ascii(n + " 0 obj\n");
            o.writeBytes(cuerpo);
            ascii("\nendobj\n");
        }

        void flujo(int n, byte[] comprimido) {
            offsets.put(n, o.size());
            ascii(n + " 0 obj\n<< /Length " + comprimido.length + " /Filter /FlateDecode >>\nstream\n");
            o.writeBytes(comprimido);
            ascii("\nendstream\nendobj\n");
        }

        byte[] cierra(int tamano, int raiz, int info) {
            int xref = o.size();
            ascii("xref\n0 " + tamano + "\n");
            ascii("0000000000 65535 f \n");
            for (int i = 1; i < tamano; i++) {
                Integer off = offsets.get(i);
                if (off == null) ascii("0000000000 65535 f \n");
                else ascii(String.format(Locale.ROOT, "%010d 00000 n \n", off));
            }
            ascii("trailer\n<< /Size " + tamano + " /Root " + raiz + " 0 R /Info " + info + " 0 R >>\nstartxref\n"
                    + xref + "\n%%EOF\n");
            return o.toByteArray();
        }
    }
}
