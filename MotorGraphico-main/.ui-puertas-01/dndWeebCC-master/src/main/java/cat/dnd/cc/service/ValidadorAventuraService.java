package cat.dnd.cc.service;

import cat.dnd.cc.model.Aventura;
import cat.dnd.cc.model.CardKind;
import cat.dnd.cc.model.CartaAventura;
import cat.dnd.cc.model.FichaEstado;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

/**
 * Reglas duras de la baralla d'història per actes (GDD §20.4 i el prompt de generació
 * {@code Plantilla_Prompt_Contenido.md §18c}), backlog WS-E fase 2 (E10).
 *
 * <p>És un servei de <b>lectura</b>: no modifica l'{@link Aventura}, només l'inspecciona i
 * retorna una llista d'{@link Incidencia}. Una aventura plana (sense {@code cartasHistoria})
 * no té res a validar aquí — {@link #validar} retorna una llista buida a l'instant.</p>
 *
 * <p>Es crida en obrir la pantalla de dependències (wireframe 13b) i, opcionalment, abans
 * d'exportar. El simulador de "pitjor cas" de la regla de la vàlvula d'escapament reutilitza
 * la mateixa lògica de filtratge que descriu el GDD §20.3 (Base/Injectada/Cadena entren sempre,
 * Condicional entra si la fitxa coincideix) sense dependre encara de {@code FiltroActoService}
 * (Fase 3, WS-E E15): aquell motor de partida en viu és per a l'estat real de sessió; aquí
 * només interessa un escenari pitjor-cas de disseny, calculat un sol cop.</p>
 */
@Service
public class ValidadorAventuraService {

    /** Mínim de cartes que han de sobreviure el pitjor cas perquè un acte no es quedi buit (§20.4.4). */
    private static final int MINIM_SUPERVIVENTS_PITJOR_CAS = 3;

    /** Percentatge mínim de cartes "segures" (Base/Injectada) per acte exigit per la vàlvula d'escapament. */
    private static final int PERCENTATGE_BASE_MINIM = 40;

    private final HistoriaService historiaService;
    private final CatalogoAventuraRepository catalogo;

    public ValidadorAventuraService(HistoriaService historiaService, CatalogoAventuraRepository catalogo) {
        this.historiaService = historiaService;
        this.catalogo = catalogo;
    }

    /** Gravetat d'una incidència: {@code ERROR} bloqueja, {@code AVISO} és una recomanació de disseny. */
    public enum Gravedad { ERROR, AVISO }

    /**
     * Una incidència detectada en validar l'aventura. {@code code} és el codi de la carta
     * afectada ({@code "A1-03"}) o {@code null} si la incidència és de l'aventura sencera
     * (p. ex. "falta la carta de Balance Final").
     */
    public record Incidencia(String code, String regla, Gravedad gravedad, String mensaje) {
    }

    /**
     * Executa totes les regles del GDD §20.4 / prompt §18c sobre una aventura. Ordre: filtre
     * sa → efecte papallona → vàlvula d'escapament → cadenes → balanç final → referències.
     *
     * @param aventura l'aventura a validar; si no és per actes ({@link Aventura#esPorActos()}
     *                 fals) es retorna una llista buida
     * @return incidències trobades, mai {@code null}; buida si tot és correcte
     */
    public List<Incidencia> validar(Aventura aventura) {
        List<Incidencia> incidencias = new ArrayList<>();
        if (aventura == null || !aventura.esPorActos()) {
            return incidencias;
        }
        validarFiltroSano(aventura, incidencias);
        validarEfectoMariposa(aventura, incidencias);
        validarValvulaDeEscape(aventura, incidencias);
        validarCadenas(aventura, incidencias);
        validarBalanceFinal(aventura, incidencias);
        validarReferencias(aventura, incidencias);
        return incidencias;
    }

    /** {@code true} si no hay ninguna incidencia de gravedad {@link Gravedad#ERROR}. */
    public boolean esValida(Aventura aventura) {
        return validar(aventura).stream().noneMatch(i -> i.gravedad() == Gravedad.ERROR);
    }

    // ==== Regla 1 · Filtro sano (§20.3 · 13b "A1-09 no existe") ====

    private void validarFiltroSano(Aventura aventura, List<Incidencia> incidencias) {
        Map<String, CartaAventura> porCodigo = indexarPorCodigo(aventura);
        for (CartaAventura carta : aventura.getCartasHistoria()) {
            CartaAventura.Requisito req = carta.getActivacion();
            if (req == null) {
                continue;
            }
            if (esVacio(req.getRequiereCode())) {
                incidencias.add(error(carta, "filtro-sano",
                        "tiene un requisito de activación sin carta de referencia (requiereCode vacío)"));
                continue;
            }
            if (req.getEstado() == null) {
                incidencias.add(error(carta, "filtro-sano",
                        "el requisito de activación no indica el estado exigido (verde/roja)"));
            }
            CartaAventura destino = porCodigo.get(req.getRequiereCode());
            if (destino == null) {
                incidencias.add(error(carta, "filtro-sano",
                        "el requisito apunta a '" + req.getRequiereCode() + "', que no existe en la aventura"));
            } else if (destino.getActo() >= carta.getActo()) {
                incidencias.add(error(carta, "filtro-sano",
                        "el requisito apunta a '" + req.getRequiereCode() + "' (acto " + destino.getActo()
                                + "), que no es un acto anterior al suyo (acto " + carta.getActo() + ")"));
            }
        }
    }

    // ==== Regla 2 · Efecto mariposa (§20.4.2) ====

    private void validarEfectoMariposa(Aventura aventura, List<Incidencia> incidencias) {
        TreeSet<String> referenciados = new TreeSet<>();
        for (CartaAventura carta : aventura.getCartasHistoria()) {
            if (carta.getActivacion() != null && !esVacio(carta.getActivacion().getRequiereCode())) {
                referenciados.add(carta.getActivacion().getRequiereCode());
            }
            for (CartaAventura.Rama rama : carta.getRamas()) {
                if (!esVacio(rama.getCuandoCode())) {
                    referenciados.add(rama.getCuandoCode());
                }
            }
        }
        for (CartaAventura carta : aventura.getCartasHistoria()) {
            // Només les cartes de l'Acte I o II poden tenir conseqüència en un acte posterior;
            // el Balance Final és en si mateix el tancament, no cal que el referenciïn.
            if (carta.getActo() >= 3 || carta.getTipo() == CardKind.BALANCE_FINAL) {
                continue;
            }
            if (esVacio(carta.getCode()) || !referenciados.contains(carta.getCode())) {
                incidencias.add(aviso(carta, "efecto-mariposa",
                        "no se detecta ninguna carta posterior que la referencie (requisito o rama); "
                                + "si puede acabar en Roja, la regla §20.4.2 exige una consecuencia"));
            }
        }
    }

    // ==== Regla 3 · Válvula de escape (§20.4.4 · 13b "Acto III sin Base") ====

    private void validarValvulaDeEscape(Aventura aventura, List<Incidencia> incidencias) {
        for (int acto = 1; acto <= 3; acto++) {
            List<CartaAventura> cartas = aventura.cartasDeActo(acto);
            if (cartas.isEmpty()) {
                continue;
            }
            int porcentaje = porcentajeBaseDelMazo(cartas);
            if (porcentaje < PERCENTATGE_BASE_MINIM) {
                incidencias.add(errorAventura("valvula-escape",
                        "Acto " + acto + ": solo el " + porcentaje + "% de sus cartas son Base/Inyectada "
                                + "(mínimo " + PERCENTATGE_BASE_MINIM + "% exigido por la válvula de escape)"));
            }
        }
        // Pitjor cas: si tot el que ve abans surt Roja, sobreviuen prou cartes per jugar?
        List<CartaAventura> acto1 = aventura.cartasDeActo(1);
        List<CartaAventura> acto2 = aventura.cartasDeActo(2);
        List<CartaAventura> acto3 = aventura.cartasDeActo(3);

        if (!acto2.isEmpty()) {
            comprobarPeorCaso(2, acto2, todoRojas(acto1), incidencias);
        }
        if (!acto3.isEmpty()) {
            Map<String, FichaEstado> previoTotal = todoRojas(acto1);
            previoTotal.putAll(todoRojas(acto2));
            comprobarPeorCaso(3, acto3, previoTotal, incidencias);
        }
    }

    private void comprobarPeorCaso(int acto, List<CartaAventura> cartasDelActo,
                                    Map<String, FichaEstado> fichasPeorCaso, List<Incidencia> incidencias) {
        int supervivientes = simularSupervivientes(cartasDelActo, fichasPeorCaso);
        if (supervivientes < MINIM_SUPERVIVENTS_PITJOR_CAS) {
            incidencias.add(errorAventura("valvula-escape",
                    "Acto " + acto + ": si todas las fichas previas salen Rojas, solo sobreviven "
                            + supervivientes + " carta(s) al filtro (mínimo " + MINIM_SUPERVIVENTS_PITJOR_CAS
                            + " para que el acto no se quede en blanco)"));
        }
    }

    /**
     * % de cartas "seguras" (Base/Inyectada) sobre el mazo real de un acto, excluyendo la
     * carta de Balance Final: esta última no se baraja ni se roba con el resto (§20.3, "se
     * juega siempre la última"), así que no debe contar ni en el numerador ni en el
     * denominador de la válvula de escape. Un acto que solo contenga la carta de Balance
     * Final devuelve 100 (no hay mazo que evaluar todavía; esa ausencia ya se marca aparte
     * si además faltan cartas jugables).
     */
    private int porcentajeBaseDelMazo(List<CartaAventura> cartasDelActo) {
        List<CartaAventura> mazo = new ArrayList<>();
        for (CartaAventura carta : cartasDelActo) {
            if (carta.getTipo() != CardKind.BALANCE_FINAL) {
                mazo.add(carta);
            }
        }
        if (mazo.isEmpty()) {
            return 100;
        }
        long base = mazo.stream().filter(CartaAventura::entraSiempre).count();
        return (int) Math.round(100.0 * base / mazo.size());
    }

    /** Cuenta cuántas cartas de un acto pasarían el filtro con un mapa de fichas ya resuelto (§20.3). */
    private int simularSupervivientes(List<CartaAventura> cartasDelActo, Map<String, FichaEstado> fichasPrevias) {
        int supervivientes = 0;
        for (CartaAventura carta : cartasDelActo) {
            if (carta.getTipo() == CardKind.BALANCE_FINAL) {
                continue; // no és una carta normal de la baralla, es juga a part
            }
            if (carta.getTipo() == CardKind.CADENA || carta.entraSiempre()) {
                supervivientes++;
                continue;
            }
            CartaAventura.Requisito req = carta.getActivacion();
            if (req != null && req.getRequiereCode() != null
                    && req.getEstado() == fichasPrevias.get(req.getRequiereCode())) {
                supervivientes++;
            }
        }
        return supervivientes;
    }

    private Map<String, FichaEstado> todoRojas(List<CartaAventura> cartas) {
        Map<String, FichaEstado> fichas = new HashMap<>();
        for (CartaAventura carta : cartas) {
            if (!esVacio(carta.getCode())) {
                fichas.put(carta.getCode(), FichaEstado.ROJA);
            }
        }
        return fichas;
    }

    // ==== Regla 4 · Cadenas (§20.3) ====

    private void validarCadenas(Aventura aventura, List<Incidencia> incidencias) {
        for (int acto = 1; acto <= 3; acto++) {
            List<CartaAventura> cadenas = new ArrayList<>();
            for (CartaAventura carta : aventura.cartasDeActo(acto)) {
                if (carta.getTipo() != CardKind.CADENA) {
                    continue;
                }
                if (carta.getActivacion() != null) {
                    incidencias.add(error(carta, "cadena",
                            "es una carta de cadena (🔗) y no puede tener requisito de activación"));
                }
                if (carta.getCadena() == null) {
                    incidencias.add(error(carta, "cadena", "es de cadena (🔗) pero no define orden/total"));
                    continue;
                }
                cadenas.add(carta);
            }
            if (cadenas.isEmpty()) {
                continue;
            }
            TreeSet<Integer> totales = new TreeSet<>();
            for (CartaAventura carta : cadenas) {
                totales.add(carta.getCadena().getTotal());
            }
            if (totales.size() > 1) {
                incidencias.add(errorAventura("cadena",
                        "Acto " + acto + ": las cartas de cadena declaran totales distintos " + totales));
                continue;
            }
            int total = totales.first();
            if (total != cadenas.size()) {
                incidencias.add(errorAventura("cadena",
                        "Acto " + acto + ": la cadena declara total=" + total + " pero hay " + cadenas.size()
                                + " carta(s) de cadena en el acto"));
            }
            TreeSet<Integer> ordenes = new TreeSet<>();
            for (CartaAventura carta : cadenas) {
                if (!ordenes.add(carta.getCadena().getOrden())) {
                    incidencias.add(error(carta, "cadena",
                            "orden de cadena duplicado (" + carta.getCadena().getOrden() + ")"));
                }
            }
            for (int esperado = 1; esperado <= total; esperado++) {
                if (!ordenes.contains(esperado)) {
                    incidencias.add(errorAventura("cadena",
                            "Acto " + acto + ": falta la posición " + esperado + "/" + total + " en la cadena"));
                }
            }
        }
    }

    // ==== Regla 5 · Balance Final (§20.3) ====

    private void validarBalanceFinal(Aventura aventura, List<Incidencia> incidencias) {
        List<CartaAventura> balances = new ArrayList<>();
        for (CartaAventura carta : aventura.getCartasHistoria()) {
            if (carta.getTipo() == CardKind.BALANCE_FINAL) {
                balances.add(carta);
            }
        }
        if (balances.isEmpty()) {
            incidencias.add(errorAventura("balance-final",
                    "la aventura no tiene ninguna carta de Balance Final (debe haber exactamente 1)"));
            return;
        }
        if (balances.size() > 1) {
            incidencias.add(errorAventura("balance-final",
                    "la aventura tiene " + balances.size() + " cartas de Balance Final (debe haber exactamente 1)"));
        }
        for (CartaAventura balance : balances) {
            if (balance.getBalance() == null || balance.getBalance().isEmpty()) {
                incidencias.add(aviso(balance, "balance-final", "no tiene ninguna fila de consecuencias"));
                continue;
            }
            for (CartaAventura.FilaBalance fila : balance.getBalance()) {
                if (esVacio(fila.getCondicion()) || esVacio(fila.getEpilogo())) {
                    incidencias.add(aviso(balance, "balance-final",
                            "tiene una fila de consecuencias incompleta (falta condición o epílogo)"));
                }
            }
        }
    }

    // ==== Regla 6 · Referencias (§18c.6) ====

    private void validarReferencias(Aventura aventura, List<Incidencia> incidencias) {
        for (CartaAventura carta : aventura.getCartasHistoria()) {
            CartaAventura.Referencias ref = carta.getReferencias();
            if (ref == null) {
                continue;
            }
            for (String npcId : ref.getNpcIds()) {
                if (!esVacio(npcId) && catalogo.completa(npcId) == null) {
                    incidencias.add(aviso(carta, "referencias", "el npc '" + npcId + "' no existe en el catálogo"));
                }
            }
            for (String enemigoId : ref.getEnemigoIds()) {
                if (!esVacio(enemigoId) && catalogo.completa(enemigoId) == null) {
                    incidencias.add(aviso(carta, "referencias",
                            "el enemigo '" + enemigoId + "' no existe en el catálogo"));
                }
            }
            if (!esVacio(ref.getLootTableId()) && catalogo.completa(ref.getLootTableId()) == null) {
                incidencias.add(aviso(carta, "referencias",
                        "la tabla de botín '" + ref.getLootTableId() + "' no existe en el catálogo"));
            }
            if (!esVacio(ref.getStoryId()) && !historiaService.existe(ref.getStoryId())) {
                incidencias.add(aviso(carta, "referencias",
                        "la historia '" + ref.getStoryId() + "' no existe en el catálogo"));
            }
            // localizacionId no es valida: el mapa del món encara no exposa un catàleg
            // indexable d'ubicacions (fora de l'abast d'E10; vegeu WS-J al backlog).
        }
    }

    // ==== ayudantes ====

    private Map<String, CartaAventura> indexarPorCodigo(Aventura aventura) {
        Map<String, CartaAventura> porCodigo = new LinkedHashMap<>();
        for (CartaAventura carta : aventura.getCartasHistoria()) {
            if (!esVacio(carta.getCode())) {
                porCodigo.put(carta.getCode(), carta);
            }
        }
        return porCodigo;
    }

    private static boolean esVacio(String s) {
        return s == null || s.isBlank();
    }

    private static Incidencia error(CartaAventura carta, String regla, String mensaje) {
        return new Incidencia(carta.getCode(), regla, Gravedad.ERROR, describir(carta) + " " + mensaje);
    }

    private static Incidencia aviso(CartaAventura carta, String regla, String mensaje) {
        return new Incidencia(carta.getCode(), regla, Gravedad.AVISO, describir(carta) + " " + mensaje);
    }

    private static Incidencia errorAventura(String regla, String mensaje) {
        return new Incidencia(null, regla, Gravedad.ERROR, mensaje);
    }

    private static String describir(CartaAventura carta) {
        String code = esVacio(carta.getCode()) ? "(sin código)" : carta.getCode();
        return "La carta " + code + ":";
    }
}
