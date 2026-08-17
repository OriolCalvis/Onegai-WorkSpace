package cat.dnd.cc.service;

import cat.dnd.cc.eines.FichaPersonatge;
import cat.dnd.cc.model.Personatge;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;

@Service
public class PdfService {
    public byte[] generarPdfPersonatge(Personatge personatge, FichaPersonatge fitxa) {
        StringBuilder linia2 = new StringBuilder();
        if (fitxa.clase() != null) {
            linia2.append(textSegur(fitxa.clase().getName()));
        }
        if (fitxa.raza() != null) {
            linia2.append(" - ").append(textSegur(fitxa.raza().getName()));
        }
        linia2.append(" - Tier ").append(personatge.getTier())
                .append(" - Vida ").append((int) fitxa.vida());

        String contingut = "%PDF-1.4\n"
                + "1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj\n"
                + "2 0 obj << /Type /Pages /Kids [3 0 R] /Count 1 >> endobj\n"
                + "3 0 obj << /Type /Page /Parent 2 0 R /Resources << >> /MediaBox [0 0 612 792] /Contents 4 0 R >> endobj\n"
                + "4 0 obj << /Length 200 >> stream\n"
                + "BT /F1 18 Tf 72 720 Td (Fitxa de personatge: " + textSegur(personatge.getNom()) + ") Tj ET\n"
                + "BT /F1 12 Tf 72 690 Td (" + linia2 + ") Tj ET\n"
                + "endstream endobj\n"
                + "trailer << /Root 1 0 R >>\n%%EOF";
        return contingut.getBytes(StandardCharsets.UTF_8);
    }

    private String textSegur(String text) {
        if (text == null || text.isBlank()) {
            return "Sense nom";
        }
        return text.replace("(", "").replace(")", "");
    }
}
