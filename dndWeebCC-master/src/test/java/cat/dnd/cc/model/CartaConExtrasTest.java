package cat.dnd.cc.model;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CARTAS-CLASES-01 — Las cartas que suben del motor traen campos que los modelos no tienen
 * (ganchos, con qué tira, a quién apunta, la familia de la clase...). Se leen, se pueden
 * consultar y vuelven a salir igual al guardar: el editor ya no los borra.
 */
class CartaConExtrasTest {

    private final ObjectMapper mapper = new ObjectMapper();

    @Test
    void lasClavesQueElModeloNoConoceSeGuardanYVuelvenASalir() throws Exception {
        String json = """
            {
              "id": "cancion_del_otono",
              "name": "Canción del Otoño",
              "type": "skill",
              "tier": 1,
              "target": "all_enemies",
              "castingStat": null,
              "defenseStat": "defensa_mental",
              "iteracion": 3,
              "effects": [
                { "description": "Desventaja 2 turnos.", "mechanicHook": "estado:2:desventaja:todo" }
              ]
            }
            """;

        TierSkill carta = mapper.readValue(json, TierSkill.class);

        assertEquals("defensa_mental", carta.getDefenseStat(), "lo que el modelo conoce, en su campo");
        assertEquals("all_enemies", carta.extraTexto("target"));
        assertEquals(List.of("estado:2:desventaja:todo"), carta.ganchos());
        assertTrue(carta.getExtras().containsKey("castingStat"), "un null también se conserva");

        JsonNode escrito = mapper.readTree(mapper.writeValueAsString(carta));
        assertEquals("all_enemies", escrito.path("target").asText());
        assertEquals(3, escrito.path("iteracion").asInt());
        assertEquals("estado:2:desventaja:todo", escrito.path("effects").get(0).path("mechanicHook").asText());
        assertFalse(escrito.has("extras"), "el mapa no sale como una clave más");
    }

    @Test
    void copiarExtrasNoPisaLosQueYaTiene() {
        TierClass guardada = new TierClass();
        guardada.setExtra("rarity", "uncommon");
        guardada.setExtra("classSet", "Set del Ninja");
        TierClass editada = new TierClass();
        editada.setExtra("rarity", "rare");

        editada.copiarExtrasDe(guardada);

        assertEquals("rare", editada.extraTexto("rarity"));
        assertEquals("Set del Ninja", editada.extraTexto("classSet"));
    }
}
