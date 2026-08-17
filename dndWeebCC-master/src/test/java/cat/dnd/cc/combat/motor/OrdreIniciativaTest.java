package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests d'{@link OrdreIniciativa}: ordenació determinista de l'ordre de torn (regla 7.3
 * del GDD, D5 del backlog WS-D). No calcula Iniciativa: rep valors ja resolts.
 */
class OrdreIniciativaTest {

    @Test
    void ordenaPerIniciativaDescendent() {
        List<OrdreIniciativa.Participant> participants = List.of(
                new OrdreIniciativa.Participant("bandit", 8.0, 3),
                new OrdreIniciativa.Participant("heroi", 12.0, 5),
                new OrdreIniciativa.Participant("mag", 10.0, 4)
        );
        assertEquals(List.of("heroi", "mag", "bandit"), OrdreIniciativa.ordenar(participants));
    }

    @Test
    void empatEnIniciativaEsDecideixPerDesBase() {
        List<OrdreIniciativa.Participant> participants = List.of(
                new OrdreIniciativa.Participant("des-baixa", 10.0, 2),
                new OrdreIniciativa.Participant("des-alta", 10.0, 6)
        );
        assertEquals(List.of("des-alta", "des-baixa"), OrdreIniciativa.ordenar(participants));
    }

    @Test
    void empatTotalMantéLOrdreRelatiuDEntrada() {
        // Mateixa Iniciativa i mateix DES base: cap criteri automàtic més enllà de 7.3,
        // l'ordenació és estable i respecta l'ordre d'entrada tal qual.
        List<OrdreIniciativa.Participant> participants = List.of(
                new OrdreIniciativa.Participant("segon-en-entrar", 10.0, 4),
                new OrdreIniciativa.Participant("primer-en-entrar", 10.0, 4)
        );
        assertEquals(List.of("segon-en-entrar", "primer-en-entrar"), OrdreIniciativa.ordenar(participants));
    }

    @Test
    void llistaBuidaDonaOrdreBuit() {
        assertTrue(OrdreIniciativa.ordenar(List.of()).isEmpty());
    }

    @Test
    void participantsNullEnOrdenarLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> OrdreIniciativa.ordenar(null));
    }

    @Test
    void idNullEnCrearParticipantLlencaExcepcio() {
        assertThrows(NullPointerException.class, () -> new OrdreIniciativa.Participant(null, 5.0, 1));
    }

    @Test
    void grupsEmpatatsDetectaNomesElsQueSeguixenEmpatatsDelTot() {
        List<OrdreIniciativa.Participant> participants = List.of(
                new OrdreIniciativa.Participant("empatat-1", 10.0, 4),
                new OrdreIniciativa.Participant("unic", 12.0, 5),
                new OrdreIniciativa.Participant("empatat-2", 10.0, 4),
                new OrdreIniciativa.Participant("desempatat-per-des", 10.0, 6)
        );
        List<List<String>> empats = OrdreIniciativa.grupsEmpatatsSenseResoldre(participants);
        assertEquals(1, empats.size());
        assertEquals(List.of("empatat-1", "empatat-2"), empats.get(0));
    }

    @Test
    void capEmpatDonaLlistaDeGrupsBuida() {
        List<OrdreIniciativa.Participant> participants = List.of(
                new OrdreIniciativa.Participant("a", 10.0, 4),
                new OrdreIniciativa.Participant("b", 8.0, 4)
        );
        assertTrue(OrdreIniciativa.grupsEmpatatsSenseResoldre(participants).isEmpty());
    }

    @Test
    void participantsNullEnGrupsEmpatatsLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> OrdreIniciativa.grupsEmpatatsSenseResoldre(null));
    }
}
