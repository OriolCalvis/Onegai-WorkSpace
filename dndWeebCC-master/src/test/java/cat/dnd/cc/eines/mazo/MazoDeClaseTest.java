package cat.dnd.cc.eines.mazo;

import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.model.TierPassive;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.model.TierSummon;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * CARTAS-CLASES-01 — El mazo físico de una clase: qué cartas lleva, en qué orden, y la línea de
 * lo que se tira (las reglas Ed. 4 del GDD §7) sacada de los campos y los ganchos de cada carta.
 */
class MazoDeClaseTest {

    private final Map<String, TierSkill> habilidades = new HashMap<>();
    private final Map<String, TierPassive> pasivas = new HashMap<>();
    private final Map<String, TierSummon> invocaciones = new HashMap<>();
    private final Map<String, TierEquipment> equipo = new HashMap<>();

    private final MazoDeClase.Catalogo catalogo = new MazoDeClase.Catalogo() {
        @Override public TierSkill habilidad(String id) { return habilidades.get(id); }
        @Override public TierSpell hechizo(String id) { return null; }
        @Override public TierPassive pasiva(String id) { return pasivas.get(id); }
        @Override public TierSummon invocacion(String id) { return invocaciones.get(id); }
        @Override public TierEquipment equipo(String id) { return equipo.get(id); }
        @Override public List<TierSummon> invocacionesDe(String cartaId) { return List.of(); }
        @Override public List<TierEquipment> piezasDelSet(String etiqueta) {
            return equipo.values().stream().filter(e -> e.getGrantedTags().contains(etiqueta)).toList();
        }
    };

    private static Map<String, Object> gancho(String hook) {
        Map<String, Object> e = new LinkedHashMap<>();
        e.put("description", hook);
        e.put("mechanicHook", hook);
        return e;
    }

    private TierSkill habilidad(String id, String target, String defensa, boolean creador, String... ganchos) {
        TierSkill s = new TierSkill();
        s.setId(id);
        s.setName("Carta " + id);
        s.setTier(creador ? 2 : 1);
        s.setActionType("accion_principal");
        s.setRange("short");
        s.setDuration("instant");
        s.setRecovery(creador ? "descanso_largo" : "descanso_corto");
        s.setDefenseStat(defensa);
        s.setExtra("target", target);
        s.setExtra("description", "Texto de " + id);
        s.setExtra("tags", creador ? List.of("prueba", "creador") : List.of("prueba", "estandar"));
        List<Object> efectos = new ArrayList<>();
        for (String g : ganchos) efectos.add(gancho(g));
        s.setExtra("effects", efectos);
        habilidades.put(id, s);
        return s;
    }

    private TierClass claseDePrueba() {
        habilidad("golpe", "single_enemy", null, false, "dano:2d6");
        habilidad("cancion", "all_enemies", "defensa_mental", false, "estado:2:desventaja:todo");
        habilidad("bola", "zone_radius", "resistencia_fisica", false, "dano:3d6");
        habilidad("cura", "all_allies", null, false, "cura:1d6");
        habilidad("llamada", "self", null, false, "invoca:10:espectro");
        habilidad("escudo", "self", null, false, "escudo:2:10");
        habilidad("raro", "single_enemy", null, false, "pendiente:que hace el contador");
        habilidad("c1", "single_enemy", null, true, "dano:4d6");
        habilidad("c2", "single_ally", null, true, "revive:1");
        habilidad("c3", "single_ally", null, true, "dados_extra:2");

        TierPassive p = new TierPassive();
        p.setId("pasiva");
        p.setName("La pasiva");
        p.setTrigger("Constante");
        p.setExtra("description", "Lo que hace la pasiva.");
        pasivas.put("pasiva", p);

        TierSummon espectro = new TierSummon();
        espectro.setId("espectro");
        espectro.setName("Espectro");
        espectro.setHealth(5);
        espectro.setDuration("10 turnos");
        espectro.setExtra("stats", Map.of("CON", 1, "DES", 4, "INT", 0, "CAR", 1));
        invocaciones.put("espectro", espectro);

        TierEquipment sable = new TierEquipment();
        sable.setId("sable");
        sable.setName("Sable");
        sable.setSlot("MainHand");
        sable.getStatBonuses().put("DES", 1);
        equipo.put("sable", sable);
        TierEquipment pieza = new TierEquipment();
        pieza.setId("capa_del_set");
        pieza.setName("Capa del set");
        pieza.setSlot("Torso");
        pieza.setTier(2);
        pieza.getGrantedTags().add("Set de Prueba");
        equipo.put("capa_del_set", pieza);

        TierClass c = new TierClass();
        c.setId("clase_prueba");
        c.setName("Clase de Prueba");
        c.setRole("agile");
        c.setBaseHealth(5);
        c.setPrimaryStat("CAR");
        c.setSecondaryStat("DES");
        c.setStartingEquipment(List.of("sable"));
        TierClass.StartingCards sc = new TierClass.StartingCards();
        sc.setPassives(List.of("pasiva"));
        sc.setSkills(List.of("golpe", "cancion", "bola"));
        sc.setLearnableSkills(List.of("golpe", "cancion", "bola", "cura", "llamada", "escudo", "raro", "c1", "c2", "c3"));
        c.setStartingCards(sc);
        c.setExtra("iteracion", 3);
        c.setExtra("classSet", "Set de Prueba");
        return c;
    }

    private static List<CartaFisica.Tipo> tipos(List<CartaFisica> mazo) {
        return mazo.stream().map(CartaFisica::getTipo).toList();
    }

    @Test
    void elMazoLlevaClasePasivaSieteMasTresInvocacionYEquipoEnEseOrden() {
        List<CartaFisica> reglas = List.of(new CartaFisica(CartaFisica.Tipo.REGLA, "La tirada").tier(0));
        List<CartaFisica> mazo = MazoDeClase.construir(claseDePrueba(), catalogo,
                new MazoDeClase.Opciones(true, true, true, true), reglas);

        List<CartaFisica.Tipo> t = tipos(mazo);
        assertEquals(CartaFisica.Tipo.CLASE, t.get(0));
        assertEquals(CartaFisica.Tipo.PASIVA, t.get(1));
        assertEquals(7, t.stream().filter(x -> x == CartaFisica.Tipo.HABILIDAD).count());
        assertEquals(3, t.stream().filter(x -> x == CartaFisica.Tipo.CREADOR).count());
        assertEquals(1, t.stream().filter(x -> x == CartaFisica.Tipo.INVOCACION).count());
        assertEquals(2, t.stream().filter(x -> x == CartaFisica.Tipo.EQUIPO).count(), "el sable y la pieza del set");
        assertEquals(CartaFisica.Tipo.REGLA, t.get(t.size() - 1));
        // Las de Creador van después de las estándar y llevan tier 2
        assertTrue(t.indexOf(CartaFisica.Tipo.CREADOR) > t.lastIndexOf(CartaFisica.Tipo.HABILIDAD));
        assertEquals(2, mazo.stream().filter(c -> c.getTipo() == CartaFisica.Tipo.CREADOR).findFirst().get().getTier());
    }

    @Test
    void lasOpcionesQuitanCreadorEquipoYReglas() {
        List<CartaFisica> mazo = MazoDeClase.construir(claseDePrueba(), catalogo,
                new MazoDeClase.Opciones(false, false, false, false),
                List.of(new CartaFisica(CartaFisica.Tipo.REGLA, "La tirada")));
        List<CartaFisica.Tipo> t = tipos(mazo);
        assertFalse(t.contains(CartaFisica.Tipo.CREADOR));
        assertFalse(t.contains(CartaFisica.Tipo.EQUIPO));
        assertFalse(t.contains(CartaFisica.Tipo.REGLA));
        assertEquals(1 + 1 + 7 + 1, mazo.size(), "clase, pasiva, 7 estándar y la invocación");
    }

    @Test
    void laMecanicaDiceQueSeTiraContraQue() {
        // Golpe físico: 1 + DES contra la mayor de CON y DES; los dados de la carta, si gana
        assertEquals("Tira 1 + DES contra CON o DES (la mayor) · +2d6 si gana",
                MazoDeClase.mecanica("single_enemy", null, null, List.of("dano:2d6")));
        // Magia contra la mente: salvación de CAR, la mitad si salva
        assertEquals("Tira 1 + INT contra la salvación de CAR · 3d6, la mitad si salva",
                MazoDeClase.mecanica("all_enemies", "INT", "defensa_mental", List.of("dano:3d6")));
        // Contra el cuerpo, sin daño (solo estados)
        assertEquals("Tira 1 + DES contra la salvación de CON",
                MazoDeClase.mecanica("single_enemy", null, "resistencia_fisica", List.of("estado:2:desventaja:todo")));
        // Lo que no apunta a un enemigo no tira
        assertNull(MazoDeClase.mecanica("all_allies", null, null, List.of("cura:1d6")));
        assertNull(MazoDeClase.mecanica("self", null, null, List.of("escudo:2:10")));
    }

    @Test
    void loPendienteSaleComoAviso() {
        assertEquals("Por definir: que hace el contador", MazoDeClase.aviso(List.of("dano:1d6", "pendiente:que hace el contador")));
        assertEquals("En parte por definir: los 2 PA", MazoDeClase.aviso(List.of("pendiente_parcial:los 2 PA")));
        assertNull(MazoDeClase.aviso(List.of("dano:1d6")));
    }

    @Test
    void laCartaDeClaseTraeVidaKitYFamilia() {
        CartaFisica clase = MazoDeClase.construir(claseDePrueba(), catalogo, MazoDeClase.Opciones.porDefecto(), List.of()).get(0);
        assertTrue(clase.getDatos().contains("Vida: 5 + CON×3 + CAR (máx. 18 en Tier 1)"), clase.getDatos().toString());
        assertTrue(clase.getDatos().contains("Kit del cofre: +2 CAR, +2 DES, +1 CON, +1 INT"), clase.getDatos().toString());
        assertTrue(clase.getLinea().contains("3.ª iteración"));
        assertNull(clase.getAviso(), "una clase de la 3.ª iteración no lleva aviso");
    }

    @Test
    void laFamiliaSaleDeLosCamposDelMotor() {
        TierClass sp = new TierClass();
        sp.setExtra("especialista", Map.of("claseBase", "mago", "elemento", "Fuego"));
        assertEquals("sp-mago", MazoDeClase.claveFamilia(sp));
        TierClass propia = new TierClass();
        propia.setExtra("propia", Map.of("familia", "cartomante", "elemento", "Arcano"));
        assertEquals("propia", MazoDeClase.claveFamilia(propia));
        TierClass vieja = new TierClass();
        assertEquals("vieja", MazoDeClase.claveFamilia(vieja));
        assertFalse(MazoDeClase.esEd4(vieja));
    }
}
