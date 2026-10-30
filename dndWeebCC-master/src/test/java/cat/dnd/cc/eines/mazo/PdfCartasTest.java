package cat.dnd.cc.eines.mazo;

import org.junit.jupiter.api.Test;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CARTAS-CLASES-01 — El PDF del mazo físico, escrito a mano: que sea un PDF bien formado (la
 * tabla xref apunta donde debe), una hoja por cada 9 cartas, y que el texto se parta sin
 * salirse de la carta y sin caracteres que WinAnsi no tiene.
 */
class PdfCartasTest {

    private static List<CartaFisica> cartas(int n) {
        List<CartaFisica> l = new ArrayList<>();
        for (int i = 0; i < n; i++) {
            l.add(new CartaFisica(CartaFisica.Tipo.HABILIDAD, "Canción del Otoño " + i)
                    .id("carta_" + i).linea("Habilidad · Acción principal").ficha("Todos los enemigos")
                    .mecanica("Tira 1 + DES contra la salvación de CAR")
                    .texto("Lanzas una melodía que da desventaja a todos los enemigos que la oigan durante 2 turnos.")
                    .pie("Bardo de las Cuatro Estaciones"));
        }
        return l;
    }

    @Test
    void esUnPdfConUnaHojaPorCadaNueveCartas() {
        byte[] pdf = PdfCartas.generar("Mazo de prueba", cartas(10));
        String texto = new String(pdf, StandardCharsets.ISO_8859_1);
        assertTrue(texto.startsWith("%PDF-1.4"));
        assertTrue(texto.endsWith("%%EOF\n"));
        Matcher m = Pattern.compile("/Type /Page /Parent").matcher(texto);
        int paginas = 0;
        while (m.find()) paginas++;
        assertEquals(2, paginas, "10 cartas = 2 hojas de 9");
        assertTrue(texto.contains("/Count 2"));
    }

    @Test
    void laTablaXrefApuntaACadaObjeto() {
        byte[] pdf = PdfCartas.generar("Mazo de prueba", cartas(3));
        String texto = new String(pdf, StandardCharsets.ISO_8859_1);
        int startxref = Integer.parseInt(texto.substring(texto.lastIndexOf("startxref") + 10, texto.lastIndexOf("%%EOF")).trim());
        assertTrue(texto.startsWith("xref", startxref));
        Matcher m = Pattern.compile("(\\d{10}) 00000 n ").matcher(texto.substring(startxref));
        int objeto = 1;
        while (m.find()) {
            int offset = Integer.parseInt(m.group(1));
            assertTrue(texto.startsWith(objeto + " 0 obj", offset), "el objeto " + objeto + " está donde dice la xref");
            objeto++;
        }
        assertTrue(objeto > 10, "catálogo, páginas, 6 fuentes, info y la hoja");
    }

    @Test
    void elTextoSeParteSinPasarDelAncho() {
        String t = "Lanzas una melodía que da desventaja a todos los enemigos que la oigan durante 2 turnos.";
        List<String> lineas = PdfCartas.partir(t, PdfCartas.Fuente.HELVETICA, 8f, 100f);
        assertTrue(lineas.size() > 1);
        for (String l : lineas) {
            assertTrue(PdfCartas.ancho(l, PdfCartas.Fuente.HELVETICA, 8f) <= 100f, l);
        }
        assertEquals(t, String.join(" ", lineas));
    }

    @Test
    void loQueWinAnsiNoTieneSeCambiaOSeQuita() {
        assertEquals(0x80, PdfCartas.codigoWinAnsi('€'));
        assertEquals(0xf1, PdfCartas.codigoWinAnsi('ñ'));
        assertEquals(0x97, PdfCartas.codigoWinAnsi('—'));
        assertEquals(-1, PdfCartas.codigoWinAnsi('→'));
        assertEquals("1d6 -> 2d6", PdfCartas.limpiar("1d6 → 2d6"));
        assertEquals("Fuego", PdfCartas.limpiar("🔥 Fuego"));
        assertEquals("CON×3", PdfCartas.limpiar("CON×3"));
    }
}
