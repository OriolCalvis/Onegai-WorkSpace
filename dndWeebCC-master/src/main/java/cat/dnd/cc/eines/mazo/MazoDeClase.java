package cat.dnd.cc.eines.mazo;

import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.model.TierPassive;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.model.TierSummon;
import cat.dnd.cc.eines.CalculadoraVida;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

/**
 * El mazo físico de una clase: la carta de clase, su pasiva, sus cartas estándar, las de
 * Creador, lo que invocan, y (si se pide) su equipo y las cartas de reglas de mesa.
 * <p>
 * CARTAS-CLASES-01 (08/10/2026). Sin Spring y sin Jackson: recibe las cartas ya leídas por
 * un {@link Catalogo}, así se puede probar con mapas en memoria.
 * <p>
 * Las reglas que imprime en cada carta son las de la Ed. 4 (docs/Sistema_Cartas_Tiers.md §7):
 * el que actúa tira 1 + su atributo; un golpe físico se defiende con CON o DES (la mayor) y
 * hace tantos d6 como la diferencia de éxitos; la magia se salva con CON (resistencia
 * física) o CAR (defensa mental): si falla, la carta entera; si salva, la mitad y sin estados.
 */
public final class MazoDeClase {

    /** De dónde saca el mazo las cartas que nombra la clase. null = no existe. */
    public interface Catalogo {
        TierSkill habilidad(String id);

        TierSpell hechizo(String id);

        TierPassive pasiva(String id);

        TierSummon invocacion(String id);

        TierEquipment equipo(String id);

        /** Las invocaciones que dicen salir de esta carta ({@code summonedBy}). */
        List<TierSummon> invocacionesDe(String cartaId);

        /** Las piezas del set con esta etiqueta («Set del Ninja»), en su orden. */
        List<TierEquipment> piezasDelSet(String etiqueta);
    }

    /** Qué entra en el mazo además de la clase, su pasiva, sus cartas y sus invocaciones. */
    public record Opciones(boolean creador, boolean equipo, boolean set, boolean reglas) {
        public static Opciones porDefecto() {
            return new Opciones(true, true, false, true);
        }
    }

    // ------------------------------------------------------------------ familias

    /** La familia de la clase, para filtrar y para el pie: "sp-mago", "propia", "it3" o "vieja". */
    public static String claveFamilia(TierClass clase) {
        Map<String, Object> sp = clase.extraMapa("especialista");
        if (!sp.isEmpty()) {
            Object base = sp.get("claseBase");
            return "sp-" + (base == null ? "otras" : base.toString());
        }
        if (!clase.extraMapa("propia").isEmpty()) {
            return "propia";
        }
        Object iteracion = clase.getExtras().get("iteracion");
        if (iteracion instanceof Number n && n.intValue() == 3) {
            return "it3";
        }
        return "vieja";
    }

    /** Las familias, en el orden en que se enseñan, con su nombre. */
    public static final Map<String, String> FAMILIAS = familias();

    private static Map<String, String> familias() {
        Map<String, String> f = new LinkedHashMap<>();
        f.put("it3", "3.ª iteración (Oriol)");
        f.put("propia", "Clases propias");
        f.put("sp-espadachin", "Especialistas · Espadachín");
        f.put("sp-arquero", "Especialistas · Arquero");
        f.put("sp-mago", "Especialistas · Mago");
        f.put("sp-artista_marcial", "Especialistas · Artista marcial");
        f.put("vieja", "2.ª iteración (sin adaptar)");
        return f;
    }

    public static String nombreFamilia(TierClass clase) {
        String clave = claveFamilia(clase);
        return FAMILIAS.getOrDefault(clave, clave);
    }

    /** ¿Tiene ya las 7 + 3 cartas con sus ganchos (3.ª iteración, SP y propias)? */
    public static boolean esEd4(TierClass clase) {
        return !"vieja".equals(claveFamilia(clase));
    }

    // ------------------------------------------------------------------ el mazo

    public static List<CartaFisica> construir(TierClass clase, Catalogo cat, Opciones op,
                                              List<CartaFisica> reglasDeMesa) {
        List<CartaFisica> mazo = new ArrayList<>();
        TierClass.StartingCards sc = clase.getStartingCards();
        String pie = clase.getName();

        // Las cartas de la clase: primero las de arranque, luego el resto de aprendibles
        Set<String> ids = new LinkedHashSet<>();
        if (sc != null) {
            ids.addAll(sc.getSkills());
            ids.addAll(sc.getLearnableSkills());
        }
        List<TierSkill> estandar = new ArrayList<>();
        List<TierSkill> creador = new ArrayList<>();
        for (String id : ids) {
            TierSkill s = cat.habilidad(id);
            if (s == null) {
                continue;
            }
            (esCreador(s) ? creador : estandar).add(s);
        }
        List<TierPassive> pasivas = new ArrayList<>();
        List<TierSpell> hechizos = new ArrayList<>();
        if (sc != null) {
            for (String id : sc.getPassives()) {
                TierPassive p = cat.pasiva(id);
                if (p != null) pasivas.add(p);
            }
            for (String id : sc.getSpells()) {
                TierSpell h = cat.hechizo(id);
                if (h != null) hechizos.add(h);
            }
        }

        mazo.add(cartaDeClase(clase, cat, pasivas, estandar.size() + hechizos.size(), creador.size()));
        for (TierPassive p : pasivas) mazo.add(cartaDePasiva(p, pie));
        for (TierSkill s : estandar) mazo.add(cartaDeHabilidad(s, pie, false));
        for (TierSpell h : hechizos) mazo.add(cartaDeHechizo(h, pie));
        if (op.creador()) {
            for (TierSkill s : creador) mazo.add(cartaDeHabilidad(s, pie, true));
        }

        // Lo que invocan sus cartas (gancho invoca:... o summonedBy)
        Set<String> vistas = new LinkedHashSet<>();
        List<TierSummon> invocaciones = new ArrayList<>();
        List<TierSkill> todas = new ArrayList<>(estandar);
        if (op.creador()) todas.addAll(creador);
        for (TierSkill s : todas) {
            for (String g : s.ganchos()) {
                String[] p = g.split(":");
                if (p.length > 2 && p[0].equals("invoca")) {
                    for (String inv : p[2].split(",")) {
                        TierSummon t = cat.invocacion(inv.strip());
                        if (t != null && vistas.add(t.getId())) invocaciones.add(t);
                    }
                }
            }
            for (TierSummon t : cat.invocacionesDe(s.getId())) {
                if (vistas.add(t.getId())) invocaciones.add(t);
            }
        }
        for (TierSummon t : invocaciones) mazo.add(cartaDeInvocacion(t, pie));

        if (op.equipo()) {
            for (String id : clase.getStartingEquipment()) {
                TierEquipment e = cat.equipo(id);
                if (e != null) mazo.add(cartaDeEquipo(e, pie, "Equipo inicial"));
            }
        }
        String set = clase.extraTexto("classSet");
        if (op.set() && set != null) {
            for (TierEquipment e : cat.piezasDelSet(set)) mazo.add(cartaDeEquipo(e, pie, set));
        }
        if (op.reglas() && reglasDeMesa != null) {
            mazo.addAll(reglasDeMesa);
        }
        return mazo;
    }

    static boolean esCreador(TierSkill s) {
        for (Object t : s.extraLista("tags")) {
            if ("creador".equalsIgnoreCase(String.valueOf(t))) return true;
        }
        for (String t : s.getMechanicTags()) {
            if ("creador".equalsIgnoreCase(t)) return true;
        }
        return false;
    }

    // ------------------------------------------------------------------ cada carta

    static CartaFisica cartaDeClase(TierClass c, Catalogo cat, List<TierPassive> pasivas, int estandar, int creador) {
        String familia = nombreFamilia(c);
        CartaFisica carta = new CartaFisica(CartaFisica.Tipo.CLASE, c.getName())
                .id(c.getId())
                .tier(c.getTier())
                .linea("Clase · " + rol(c.getRole()) + " · " + familia)
                .pie(familia);
        String elemento = elementoDeClase(c);
        if (elemento != null) carta.elemento(elemento, COLORES.get(elemento));

        String p = c.getPrimaryStat();
        String s = c.getSecondaryStat();
        carta.dato("Atributos: " + p + " principal" + (s != null ? " · " + s + " secundario" : ""));
        int techo = CalculadoraVida.techoTier(Math.max(1, c.getTier()));
        carta.dato("Vida: " + c.getBaseHealth() + " + CON×3 + CAR (máx. " + techo + " en Tier "
                + Math.max(1, c.getTier()) + ")");
        carta.dato("Kit del cofre: " + kit(p, s));
        carta.dato("Pesos: armadura hasta " + peso(c.getMaxArmorWeight()) + " · arma hasta " + peso(c.getMaxWeaponWeight()));
        List<String> equipo = new ArrayList<>();
        for (String id : c.getStartingEquipment()) {
            TierEquipment e = cat.equipo(id);
            equipo.add(e != null ? e.getName() : id);
        }
        if (!equipo.isEmpty()) carta.dato("Equipo inicial: " + String.join(", ", equipo));
        if (!pasivas.isEmpty()) {
            List<String> nombres = new ArrayList<>();
            for (TierPassive pa : pasivas) nombres.add(pa.getName());
            carta.dato("Pasiva: " + String.join(", ", nombres));
        }
        carta.dato("Cartas: " + estandar + " estándar" + (creador > 0 ? " + " + creador + " de Creador (Tier 2)" : ""));
        String set = c.extraTexto("classSet");
        if (set != null) carta.dato(set + " (Tier 2, no se empieza con él)");
        carta.texto(c.getDescription());
        if (!esEd4(c)) {
            carta.aviso("2.ª iteración: sus cartas aún no se han adaptado a la Ed. 4");
        }
        return carta;
    }

    static CartaFisica cartaDePasiva(TierPassive p, String pie) {
        String disparo = p.getTrigger();
        String linea = "Pasiva de clase · siempre activa";
        if (disparo != null && !disparo.isBlank() && !disparo.equalsIgnoreCase("Constante")) {
            linea = "Pasiva de clase · " + disparo;
        }
        String texto = p.extraTexto("description");
        if (texto == null && p.getEffect() != null) texto = p.getEffect().getDescription();
        return new CartaFisica(CartaFisica.Tipo.PASIVA, p.getName())
                .id(p.getId())
                .tier(p.getTier())
                .linea(linea)
                .texto(texto)
                .ambientacion(p.getFlavorText())
                .aviso(aviso(p.ganchos()))
                .pie(pie);
    }

    static CartaFisica cartaDeHabilidad(TierSkill s, String pie, boolean creador) {
        CartaFisica.Tipo tipo = creador ? CartaFisica.Tipo.CREADOR : CartaFisica.Tipo.HABILIDAD;
        String accion = accion(s.getActionType());
        String linea = creador ? "Creador · Tier " + Math.max(2, s.getTier()) + " · " + accion
                : (s.getTier() >= 2 ? "Aprendible en Tier " + s.getTier() + " · " + accion : "Habilidad · " + accion);
        CartaFisica carta = new CartaFisica(tipo, s.getName())
                .id(s.getId())
                .tier(creador ? Math.max(2, s.getTier()) : s.getTier())
                .linea(linea)
                .ficha(objetivo(s.extraTexto("target")))
                .ficha(alcance(s.getRange()))
                .ficha(duracion(s.getDuration()))
                .ficha(recuperacion(s.getRecovery()))
                .mecanica(mecanica(s.extraTexto("target"), s.extraTexto("castingStat"), s.getDefenseStat(), s.ganchos()))
                .texto(textoDe(s.extraTexto("description"), s.getEffect() != null ? s.getEffect().getDescription() : null))
                .ambientacion(s.getFlavorText())
                .aviso(aviso(s.ganchos()))
                .pie(pie);
        String elemento = elementoDeEtiquetas(s.getMechanicTags(), s.extraLista("tags"));
        if (elemento != null) carta.elemento(elemento, COLORES.get(elemento));
        return carta;
    }

    static CartaFisica cartaDeHechizo(TierSpell h, String pie) {
        CartaFisica carta = new CartaFisica(CartaFisica.Tipo.HECHIZO, h.getName())
                .id(h.getId())
                .tier(h.getTier())
                .linea("Hechizo" + (h.getSchool() != null ? " · " + h.getSchool() : ""))
                .ficha(alcance(h.getRange()))
                .ficha(duracion(h.getDuration()))
                .ficha(recuperacion(h.getRecovery()))
                .mecanica(h.getCastingStat() != null ? "Tira 1 + " + h.getCastingStat() : null)
                .texto(textoDe(h.extraTexto("description"), h.getEffect() != null ? h.getEffect().getDescription() : null))
                .ambientacion(h.getFlavorText())
                .pie(pie);
        String elemento = elementoDeEtiquetas(h.getMechanicTags(), h.extraLista("tags"));
        if (elemento != null) carta.elemento(elemento, COLORES.get(elemento));
        return carta;
    }

    static CartaFisica cartaDeInvocacion(TierSummon t, String pie) {
        CartaFisica carta = new CartaFisica(CartaFisica.Tipo.INVOCACION, t.getName())
                .id(t.getId())
                .tier(t.getTier())
                .linea("Invocación" + (t.getDuration() != null ? " · " + duracionLarga(t.getDuration()) : ""))
                .pie(pie);
        Map<String, Object> stats = t.extraMapa("stats");
        if (!stats.isEmpty()) {
            carta.dato("CON " + num(stats.get("CON")) + " · DES " + num(stats.get("DES")) + " · INT "
                    + num(stats.get("INT")) + " · CAR " + num(stats.get("CAR")));
        }
        carta.dato(t.getHealth() + " PV" + (t.getMovement() > 0 ? " · Movimiento " + t.getMovement() : "")
                + " · " + control(t.getControl()));
        StringBuilder texto = new StringBuilder();
        for (TierSummon.Attack a : t.getAttacks()) {
            if (a.getName() == null && a.getEffect() == null) continue;
            texto.append(a.getName() != null ? a.getName() + ": " : "").append(nz(a.getEffect())).append('\n');
        }
        if (t.getPassive() != null && t.getPassive().getName() != null) {
            texto.append(t.getPassive().getName()).append(": ").append(nz(t.getPassive().getDescription()));
        }
        carta.texto(texto.toString());
        carta.ambientacion(t.getFlavorText());
        return carta;
    }

    static CartaFisica cartaDeEquipo(TierEquipment e, String pie, String origen) {
        CartaFisica carta = new CartaFisica(CartaFisica.Tipo.EQUIPO, e.getName())
                .id(e.getId())
                .tier(e.getTier())
                .linea("Equipo · " + ranura(e.getSlot()) + " · " + peso(e.getWeightCategory()) + " · " + rareza(e.getRarity()))
                .pie(origen + " · " + pie);
        List<String> bonos = new ArrayList<>();
        for (String stat : List.of("CON", "DES", "INT", "CAR")) {
            int b = e.getStatBonuses().getOrDefault(stat, 0);
            int m = e.getPenalties().getOrDefault(stat, 0);
            if (b != 0) bonos.add((b > 0 ? "+" : "") + b + " " + stat);
            if (m != 0) bonos.add("-" + Math.abs(m) + " " + stat);
        }
        if (!bonos.isEmpty()) carta.dato(String.join(" · ", bonos));
        String texto = e.extraTexto("description");
        carta.texto(texto != null ? texto : e.getFlavorText());
        if (texto != null && !texto.equals(e.getFlavorText())) carta.ambientacion(e.getFlavorText());
        return carta;
    }

    // ------------------------------------------------------------------ la mecánica

    /**
     * La línea de lo que se tira, sacada de los campos y los ganchos de la carta:
     * «Tira 1 + DES contra CON o DES · +2d6 si gana», «Tira 1 + INT contra salvación de CAR ·
     * 3d6, mitad si salva». Las cartas que no apuntan a un enemigo no tiran.
     */
    static String mecanica(String objetivo, String castingStat, String defenseStat, List<String> ganchos) {
        boolean contraEnemigo = objetivo == null
                || objetivo.equals("single_enemy") || objetivo.equals("all_enemies") || objetivo.equals("zone_radius");
        String dados = null;
        boolean armas = false;
        boolean afectaEnemigo = ganchos.isEmpty();
        for (String g : ganchos) {
            String[] p = g.split(":");
            switch (p[0]) {
                case "dano" -> {
                    afectaEnemigo = true;
                    if (dados == null && p.length > 1 && !p[1].equals("escudo")) dados = p[1];
                }
                case "ataque_arma", "ataques_arma" -> {
                    afectaEnemigo = true;
                    armas = true;
                }
                case "estado", "condicion" -> afectaEnemigo = true;
                default -> {
                }
            }
        }
        if (!contraEnemigo || !afectaEnemigo) {
            return null;
        }
        String stat = (castingStat == null || castingStat.isBlank()) ? "DES" : castingStat.toUpperCase(Locale.ROOT);
        boolean magia = "defensa_mental".equals(defenseStat) || "resistencia_fisica".equals(defenseStat);
        StringBuilder m = new StringBuilder("Tira 1 + ").append(stat);
        if (magia) {
            m.append(" contra la salvación de ").append("defensa_mental".equals(defenseStat) ? "CAR" : "CON");
            if (dados != null) m.append(" · ").append(dados).append(", la mitad si salva");
        } else {
            m.append(" contra CON o DES (la mayor)");
            if (armas) m.append(" · golpe de arma");
            if (dados != null) m.append(" · +").append(dados).append(" si gana");
        }
        return m.toString();
    }

    /** «Por definir: ...» para lo que la carta deja [pendiente] (no se inventa la regla). */
    static String aviso(List<String> ganchos) {
        List<String> faltan = new ArrayList<>();
        List<String> parcial = new ArrayList<>();
        for (String g : ganchos) {
            if (g.startsWith("pendiente_parcial:")) parcial.add(g.substring("pendiente_parcial:".length()));
            else if (g.startsWith("pendiente:")) faltan.add(g.substring("pendiente:".length()));
        }
        if (!faltan.isEmpty()) return "Por definir: " + String.join("; ", faltan);
        if (!parcial.isEmpty()) return "En parte por definir: " + String.join("; ", parcial);
        return null;
    }

    private static String textoDe(String a, String b) {
        return (a != null && !a.isBlank()) ? a : b;
    }

    // ------------------------------------------------------------------ los nombres

    /** Los 13 elementos de CANON_PARTICULAS (motor, PART-01) y su color. */
    public static final Map<String, Integer> COLORES = colores();

    private static Map<String, Integer> colores() {
        Map<String, Integer> c = new LinkedHashMap<>();
        c.put("Fuego", 0xff8c3c);
        c.put("Agua", 0x5ab4ff);
        c.put("Hielo", 0xbff0ff);
        c.put("Rayo", 0xfff27a);
        c.put("Tierra", 0xc89a5a);
        c.put("Viento", 0xd8f5e6);
        c.put("Luz", 0xfff0b4);
        c.put("Tinieblas", 0x7a4ab4);
        c.put("Naturaleza", 0x8cd25a);
        c.put("Veneno", 0xa0e650);
        c.put("Sangre", 0xd22828);
        c.put("Arcano", 0xc88cff);
        c.put("Vacío", 0x5a3c8c);
        return c;
    }

    private static final Map<String, String> ALIAS_ELEMENTO = Map.of(
            "Sagrado", "Luz", "Oscuro", "Tinieblas", "Vacio", "Vacío");

    static String normalizaElemento(String etiqueta) {
        if (etiqueta == null) return null;
        String e = etiqueta.strip();
        if (COLORES.containsKey(e)) return e;
        return ALIAS_ELEMENTO.get(e);
    }

    static String elementoDeEtiquetas(List<String> mecanicas, List<Object> tags) {
        for (String t : mecanicas) {
            String e = normalizaElemento(t);
            if (e != null) return e;
        }
        for (Object t : tags) {
            String e = normalizaElemento(String.valueOf(t));
            if (e != null) return e;
        }
        return null;
    }

    static String elementoDeClase(TierClass c) {
        Object e = c.extraMapa("especialista").get("elemento");
        if (e == null) e = c.extraMapa("propia").get("elemento");
        return e == null ? null : normalizaElemento(e.toString());
    }

    static String kit(String principal, String secundario) {
        List<String> resto = new ArrayList<>(List.of("CON", "DES", "INT", "CAR"));
        resto.remove(principal);
        resto.remove(secundario);
        StringBuilder k = new StringBuilder("+2 ").append(principal);
        if (secundario != null) k.append(", +2 ").append(secundario);
        for (String r : resto) k.append(", +1 ").append(r);
        return k.toString();
    }

    static String rol(String rol) {
        if (rol == null) return "Sin rol";
        return switch (rol) {
            case "tank" -> "Tanque";
            case "balanced" -> "Equilibrado";
            case "agile" -> "Ágil";
            case "caster" -> "Lanzador";
            case "support" -> "Apoyo";
            case "striker" -> "Atacante";
            default -> rol;
        };
    }

    static String accion(String tipo) {
        if (tipo == null) return "Acción principal";
        return switch (tipo) {
            case "accion_principal", "accion" -> "Acción principal";
            case "accion_menor" -> "Acción menor";
            case "reaccion" -> "Reacción";
            case "movimiento" -> "Movimiento";
            case "canalizacion" -> "Canalización";
            case "libre" -> "Acción libre";
            case "preparacion" -> "Preparación";
            case "pasiva" -> "Pasiva";
            case "consumible" -> "Consumible";
            default -> tipo;
        };
    }

    static String objetivo(String target) {
        if (target == null) return null;
        return switch (target) {
            case "single_enemy" -> "Un enemigo";
            case "all_enemies" -> "Todos los enemigos";
            case "self" -> "Tú";
            case "zone_radius" -> "Una zona";
            case "all_allies" -> "Tu grupo";
            case "single_ally" -> "Un aliado";
            case "single_target" -> "Un objetivo";
            default -> target;
        };
    }

    static String alcance(String range) {
        if (range == null) return null;
        return switch (range) {
            case "melee" -> "Cuerpo a cuerpo";
            case "short" -> "Alcance corto";
            case "medium" -> "Alcance medio";
            case "long" -> "Alcance largo";
            case "self" -> null;
            default -> range;
        };
    }

    static String duracion(String duration) {
        if (duration == null) return null;
        return switch (duration) {
            case "instant" -> null;
            default -> duracionLarga(duration);
        };
    }

    static String duracionLarga(String duration) {
        return switch (duration) {
            case "instant" -> "Instantánea";
            case "1_turn" -> "1 turno";
            case "1_min" -> "1 minuto";
            case "concentration" -> "Concentración";
            case "permanent" -> "Permanente";
            case "combate" -> "Todo el combate";
            default -> duration;
        };
    }

    static String recuperacion(String recovery) {
        if (recovery == null) return null;
        return switch (recovery) {
            case "descanso_corto" -> "Descanso corto";
            case "descanso_largo" -> "Descanso largo";
            case "activo", "activa", "ninguno" -> "Siempre lista";
            default -> recovery;
        };
    }

    static String peso(String peso) {
        if (peso == null) return "—";
        return switch (peso.toLowerCase(Locale.ROOT)) {
            case "ligera", "light" -> "ligera";
            case "media", "medium" -> "media";
            case "pesada", "heavy" -> "pesada";
            default -> peso;
        };
    }

    static String ranura(String slot) {
        if (slot == null) return "Sin ranura";
        return switch (slot) {
            case "MainHand", "arma_principal" -> "Arma";
            case "OffHand", "arma_secundaria" -> "Mano secundaria";
            case "Head", "cabeza" -> "Cabeza";
            case "Torso", "torso" -> "Torso";
            case "Legs", "piernas" -> "Piernas";
            case "Feet", "pies" -> "Pies";
            case "Neck", "cuello" -> "Cuello";
            default -> slot;
        };
    }

    static String rareza(String rareza) {
        if (rareza == null) return "común";
        return switch (rareza) {
            case "common" -> "común";
            case "uncommon" -> "poco común";
            case "rare" -> "rara";
            case "epic" -> "épica";
            case "legendary" -> "legendaria";
            case "mythic" -> "mítica";
            default -> rareza;
        };
    }

    static String control(String control) {
        if (control == null) return "La controlas tú";
        return switch (control) {
            case "jugador" -> "La controlas tú";
            case "car_invocador" -> "Obedece según tu CAR";
            case "automatica" -> "Actúa sola";
            default -> control;
        };
    }

    private static String num(Object o) {
        return o instanceof Number n ? String.valueOf(n.intValue()) : (o == null ? "0" : o.toString());
    }

    private static String nz(String s) {
        return s == null ? "" : s;
    }

    private MazoDeClase() {
    }
}
