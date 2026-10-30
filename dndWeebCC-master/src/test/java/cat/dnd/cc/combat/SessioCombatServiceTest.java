package cat.dnd.cc.combat;

import cat.dnd.cc.combat.motor.EstatPiles;
import cat.dnd.cc.combat.motor.Pila;
import cat.dnd.cc.model.Personatge;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.service.TierSkillService;
import cat.dnd.cc.service.TierSpellService;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.Mockito;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

/**
 * Tests unitaris (sense contexte Spring, mateix patró que {@code PersonatgeServiceTest}) de
 * {@link SessioCombatService}: resolució inicial de l'{@link EstatPiles} d'un personatge
 * contra el catàleg real (D9 del backlog WS-D).
 */
class SessioCombatServiceTest {

    private TierSkillService tierSkillService;
    private TierSpellService tierSpellService;
    private SessioCombatService service;

    @BeforeEach
    void setUp() {
        tierSkillService = Mockito.mock(TierSkillService.class);
        tierSpellService = Mockito.mock(TierSpellService.class);
        service = new SessioCombatService(tierSkillService, tierSpellService);

        when(tierSkillService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
        when(tierSpellService.obtenerPorId(anyString())).thenThrow(new IllegalArgumentException("no existe"));
    }

    private static Personatge personatgeAmb(Long id, List<String> habilidadIds, List<String> hechizoIds) {
        Personatge p = new Personatge();
        p.setId(id);
        p.setHabilidadIds(habilidadIds);
        p.setHechizoIds(hechizoIds);
        return p;
    }

    private static TierSkill skillAmbRecovery(String id, String recovery) {
        TierSkill s = new TierSkill();
        s.setId(id);
        s.setRecovery(recovery);
        return s;
    }

    private static TierSpell spellAmbRecovery(String id, String recovery) {
        TierSpell s = new TierSpell();
        s.setId(id);
        s.setRecovery(recovery);
        return s;
    }

    @Test
    void personatgeSenseCartesDonaEstatBuit() {
        Personatge p = personatgeAmb(1L, List.of(), List.of());
        EstatPiles piles = service.estatPilesDe(p);
        assertEquals(0, piles.comptar(Pila.ACTIVA));
    }

    @Test
    void habilidadIdsIHechizoIdsNullsNoPeten() {
        Personatge p = personatgeAmb(1L, null, null);
        EstatPiles piles = service.estatPilesDe(p);
        assertEquals(0, piles.comptar(Pila.ACTIVA));
    }

    @Test
    void resolCadaCartaContraElCatalegIElsRegistraTotsAActiva() {
        when(tierSkillService.obtenerPorId("skill_tajo")).thenReturn(skillAmbRecovery("skill_tajo", "descanso_corto"));
        when(tierSpellService.obtenerPorId("spell_meteor")).thenReturn(spellAmbRecovery("spell_meteor", "descanso_largo"));

        Personatge p = personatgeAmb(1L, List.of("skill_tajo"), List.of("spell_meteor"));
        EstatPiles piles = service.estatPilesDe(p);

        assertEquals(Pila.ACTIVA, piles.pilaDe("skill_tajo"));
        assertEquals(Pila.ACTIVA, piles.pilaDe("spell_meteor"));
        assertEquals(2, piles.comptar(Pila.ACTIVA));
    }

    @Test
    void unIdOrfeQueJaNoExisteixAlCatalegNoEsRegistraNiPeta() {
        // tierSkillService ja llença IllegalArgumentException per defecte (setUp): simula una
        // habilidadId que apuntava a una carta esborrada del catàleg.
        Personatge p = personatgeAmb(1L, List.of("skill_esborrada"), List.of());
        EstatPiles piles = service.estatPilesDe(p);
        assertEquals(0, piles.comptar(Pila.ACTIVA));
        assertThrows(IllegalArgumentException.class, () -> piles.pilaDe("skill_esborrada"));
    }

    @Test
    void demanarElMateixPersonatgeDuesVegadesRetornaLaMateixaInstanciaEnViu() {
        when(tierSkillService.obtenerPorId("skill_tajo")).thenReturn(skillAmbRecovery("skill_tajo", "descanso_corto"));
        Personatge p = personatgeAmb(1L, List.of("skill_tajo"), List.of());

        EstatPiles primera = service.estatPilesDe(p);
        primera.jugar("skill_tajo");
        EstatPiles segona = service.estatPilesDe(p);

        assertSame(primera, segona, "no s'ha de reconstruir l'estat cada cop que es consulta la fitxa");
        assertEquals(Pila.DESCANSO_CORTO, segona.pilaDe("skill_tajo"), "l'estat de la partida s'ha de mantenir");
    }

    @Test
    void reiniciarDescartaLEstatIElProximAccesElReconstrueix() {
        when(tierSkillService.obtenerPorId("skill_tajo")).thenReturn(skillAmbRecovery("skill_tajo", "descanso_corto"));
        Personatge p = personatgeAmb(1L, List.of("skill_tajo"), List.of());

        EstatPiles primera = service.estatPilesDe(p);
        primera.jugar("skill_tajo");

        service.reiniciar(1L);
        EstatPiles despresDeReiniciar = service.estatPilesDe(p);

        assertEquals(Pila.ACTIVA, despresDeReiniciar.pilaDe("skill_tajo"), "reiniciar torna a Activa");
    }

    @Test
    void personatgeSenseIdLlencaExcepcio() {
        Personatge p = personatgeAmb(null, List.of(), List.of());
        assertThrows(IllegalArgumentException.class, () -> service.estatPilesDe(p));
    }

    @Test
    void personatgeNullLlencaExcepcio() {
        assertThrows(IllegalArgumentException.class, () -> service.estatPilesDe(null));
    }
}
