package cat.dnd.cc.model;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Blinda la compatibilidad del modelo de aventura por actos (GDD §20) con las 52 aventuras
 * "planas" ya en disco, y comprueba que una aventura por actos —con las claves inglesas del
 * prompt de generación §18b— deserializa a los tipos correctos.
 */
class CartaAventuraTest {

    private final ObjectMapper mapper = new ObjectMapper();

    @Test
    void aventuraPlanaLegacyCargaSinActos() throws Exception {
        // Igual que data/aventuras/1.json: solo historiaIds, sin cartasHistoria.
        String json = """
            {
              "id": 1,
              "nom": "jklkbbjkbkj",
              "descripcion": "",
              "historiaIds": ["hist_uno", "hist_dos"],
              "notas": ""
            }
            """;

        Aventura av = mapper.readValue(json, Aventura.class);

        assertEquals("jklkbbjkbkj", av.getNom());
        assertEquals(2, av.getHistoriaIds().size());
        assertNotNull(av.getCartasHistoria());
        assertTrue(av.getCartasHistoria().isEmpty(), "una aventura legacy no tiene cartas de historia");
        assertFalse(av.esPorActos(), "sin cartas de historia no es una aventura por actos");
        assertEquals("borrador", av.getEstado(), "estado por defecto");
    }

    @Test
    void aventuraPorActosDeserializaConClavesDelPrompt() throws Exception {
        // Claves inglesas tal como las emite el prompt §18b (act, title, cardKind, activation,
        // scene, branches, ignoreHook, references, balanceTable) + un campo extra ignorado.
        String json = """
            {
              "id": 7,
              "nom": "Las Cenizas de Brumal",
              "tema": "Fantasía",
              "estado": "en_curso",
              "arquitectura": "espina_de_pescado",
              "cartasHistoria": [
                {
                  "code": "A1-01", "act": 1, "title": "El Rumor", "cardKind": "base",
                  "scene": "En la taberna corre un rumor.",
                  "ignoreHook": "Si ignoran esto, el rumor se apaga solo.",
                  "adventureId": "adventure_brumal"
                },
                {
                  "code": "B2-05", "act": 2, "title": "Camino Seguro", "cardKind": "condicional",
                  "activation": { "requires": "A1-03", "state": "verde" },
                  "scene": "El sendero está despejado."
                },
                {
                  "code": "B2-09", "act": 2, "title": "La Emboscada", "cardKind": "inyectada",
                  "branches": [
                    { "whenCode": "A1-03", "whenState": "roja", "text": "Los bandidos atacan." },
                    { "whenCode": "A1-03", "whenState": "verde", "text": "El camino está en calma." }
                  ],
                  "references": { "enemyIds": ["enemigo_bandido"], "locationId": "bosque_de_vael" }
                },
                {
                  "code": "C3-99", "act": 3, "title": "Balance", "cardKind": "balance_final",
                  "balanceTable": [ { "condition": "≥3 rojas en Acto I", "epilogue": "El reino arde." } ]
                }
              ]
            }
            """;

        Aventura av = mapper.readValue(json, Aventura.class);

        assertTrue(av.esPorActos());
        assertEquals(4, av.getCartasHistoria().size());
        assertEquals(1, av.cartasDeActo(1).size());
        assertEquals(2, av.cartasDeActo(2).size());

        CartaAventura rumor = av.cartasDeActo(1).get(0);
        assertEquals("A1-01", rumor.getCode());
        assertEquals("El Rumor", rumor.getTitulo());          // desde "title"
        assertEquals(CardKind.BASE, rumor.getTipo());
        assertTrue(rumor.entraSiempre());

        CartaAventura camino = av.getCartasHistoria().get(1);
        assertEquals(CardKind.CONDICIONAL, camino.getTipo());
        assertEquals("A1-03", camino.getActivacion().getRequiereCode());
        assertEquals(FichaEstado.VERDE, camino.getActivacion().getEstado());

        CartaAventura emboscada = av.getCartasHistoria().get(2);
        assertEquals(CardKind.INYECTADA, emboscada.getTipo());
        assertTrue(emboscada.entraSiempre(), "una inyectada entra siempre en el mazo");
        assertEquals(2, emboscada.getRamas().size());
        assertEquals(FichaEstado.ROJA, emboscada.getRamas().get(0).getCuandoEstado());
        assertEquals("enemigo_bandido", emboscada.getReferencias().getEnemigoIds().get(0));

        CartaAventura balance = av.getCartasHistoria().get(3);
        assertEquals(CardKind.BALANCE_FINAL, balance.getTipo());
        assertEquals(1, balance.getBalance().size());

        // Válvula de escape (GDD §20.4): Acto II tiene 1 Base de 2 → 50 %.
        assertEquals(50, av.porcentajeBase(2));
        assertEquals(100, av.porcentajeBase(1));
    }

    @Test
    void enumsRedondeanAMinusculasAlSerializar() throws Exception {
        assertEquals("\"balance_final\"", mapper.writeValueAsString(CardKind.BALANCE_FINAL));
        assertEquals("\"roja\"", mapper.writeValueAsString(FichaEstado.ROJA));
    }
}
