package cat.dnd.cc.combat.motor;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Tests d'{@link EstatPiles}: sistema de pilas en viu (regla 6 del GDD, D8 del backlog
 * WS-D).
 */
class EstatPilesTest {

    @Test
    void registrarComençaSempreAActiva() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_tajo", Recovery.DESCANSO_CORTO);
        assertEquals(Pila.ACTIVA, piles.pilaDe("skill_tajo"));
    }

    @Test
    void jugarUnaCartaAmbDescansCurtLaMouAAquellaPila() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_tajo", Recovery.DESCANSO_CORTO);
        piles.jugar("skill_tajo");
        assertEquals(Pila.DESCANSO_CORTO, piles.pilaDe("skill_tajo"));
    }

    @Test
    void jugarUnaCartaAmbDescansLlargLaMouAAquellaPila() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("spell_meteor", Recovery.DESCANSO_LARGO);
        piles.jugar("spell_meteor");
        assertEquals(Pila.DESCANSO_LARGO, piles.pilaDe("spell_meteor"));
    }

    @Test
    void jugarUnaCartaAmbRecoveryNingunoEsQuedaAActiva() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("passive_regen", Recovery.NINGUNO);
        piles.jugar("passive_regen");
        assertEquals(Pila.ACTIVA, piles.pilaDe("passive_regen"));
    }

    @Test
    void esPotJugarRepetidamentUnaCartaNingunoOActiva() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("passive_regen", Recovery.NINGUNO);
        piles.jugar("passive_regen");
        piles.jugar("passive_regen");
        assertEquals(Pila.ACTIVA, piles.pilaDe("passive_regen"));
    }

    @Test
    void jugarUnaCartaJaJugadaAbansDeDescansarLlencaExcepcio() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_tajo", Recovery.DESCANSO_CORTO);
        piles.jugar("skill_tajo");
        assertThrows(IllegalStateException.class, () -> piles.jugar("skill_tajo"));
    }

    @Test
    void jugarCartaNoRegistradaLlencaExcepcio() {
        EstatPiles piles = new EstatPiles();
        assertThrows(IllegalArgumentException.class, () -> piles.jugar("mai_registrada"));
    }

    @Test
    void pilaDeCartaNoRegistradaLlencaExcepcio() {
        EstatPiles piles = new EstatPiles();
        assertThrows(IllegalArgumentException.class, () -> piles.pilaDe("mai_registrada"));
    }

    @Test
    void descansarCurtTornaNomesLaPilaDescansCurtAActiva() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_tajo", Recovery.DESCANSO_CORTO);
        piles.registrar("spell_meteor", Recovery.DESCANSO_LARGO);
        piles.jugar("skill_tajo");
        piles.jugar("spell_meteor");

        piles.descansarCurt();

        assertEquals(Pila.ACTIVA, piles.pilaDe("skill_tajo"));
        assertEquals(Pila.DESCANSO_LARGO, piles.pilaDe("spell_meteor"), "el descans curt no toca el descans llarg");
    }

    @Test
    void descansarLlargTornaAmbduesPilesAActiva() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_tajo", Recovery.DESCANSO_CORTO);
        piles.registrar("spell_meteor", Recovery.DESCANSO_LARGO);
        piles.jugar("skill_tajo");
        piles.jugar("spell_meteor");

        piles.descansarLlarg();

        assertEquals(Pila.ACTIVA, piles.pilaDe("skill_tajo"));
        assertEquals(Pila.ACTIVA, piles.pilaDe("spell_meteor"));
    }

    @Test
    void moureAExplicitPermetRedirigirIndependentDelRecoveryDeclarat() {
        // Cas Arcanista: un hechizo de descanso_largo que, per una pasiva, torna directament
        // a Activa en lloc d'anar a la seva pila per defecte.
        EstatPiles piles = new EstatPiles();
        piles.registrar("spell_meteor", Recovery.DESCANSO_LARGO);
        piles.jugar("spell_meteor");
        assertEquals(Pila.DESCANSO_LARGO, piles.pilaDe("spell_meteor"));

        piles.moureA("spell_meteor", Pila.ACTIVA);
        assertEquals(Pila.ACTIVA, piles.pilaDe("spell_meteor"));
    }

    @Test
    void comptarICartesAReflecteixenElRecompteReal() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_a", Recovery.DESCANSO_CORTO);
        piles.registrar("skill_b", Recovery.DESCANSO_CORTO);
        piles.registrar("skill_c", Recovery.DESCANSO_LARGO);
        piles.jugar("skill_a");
        piles.jugar("skill_b");
        piles.jugar("skill_c");

        assertEquals(2, piles.comptar(Pila.DESCANSO_CORTO));
        assertEquals(1, piles.comptar(Pila.DESCANSO_LARGO));
        assertEquals(0, piles.comptar(Pila.ACTIVA));
        assertEquals(List.of("skill_a", "skill_b"), piles.cartesA(Pila.DESCANSO_CORTO));
    }

    @Test
    void registrarAmbCardIdONullLlencaExcepcio() {
        EstatPiles piles = new EstatPiles();
        assertThrows(IllegalArgumentException.class, () -> piles.registrar(null, Recovery.ACTIVA));
        assertThrows(IllegalArgumentException.class, () -> piles.registrar("x", null));
    }

    @Test
    void moureANullLlencaExcepcio() {
        EstatPiles piles = new EstatPiles();
        piles.registrar("skill_tajo", Recovery.ACTIVA);
        assertThrows(IllegalArgumentException.class, () -> piles.moureA("skill_tajo", null));
    }

    @Test
    void novesInstanciesComencenBuidesSenseCartesEnlloc() {
        EstatPiles piles = new EstatPiles();
        assertTrue(piles.cartesA(Pila.ACTIVA).isEmpty());
        assertEquals(0, piles.comptar(Pila.ACTIVA));
    }
}
