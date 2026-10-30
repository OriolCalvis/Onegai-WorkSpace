package cat.dnd.cc.service;

import cat.dnd.cc.model.Aventura;
import cat.dnd.cc.model.CardKind;
import cat.dnd.cc.model.CartaAventura;
import cat.dnd.cc.model.FichaEstado;
import cat.dnd.cc.web.form.CartaAventuraForm;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * Lógica del Constructor de aventuras por actos (GDD §20, Fase 1): añade, edita, mueve y borra
 * Cartas de Historia dentro de una {@link Aventura}, autogenerando su código y persistiendo con
 * {@link AventuraService#guardar}. No reimplementa el CRUD base de la aventura: se apoya en él.
 */
@Service
public class ConstructorAventuraService {

    private final AventuraService aventuraService;

    public ConstructorAventuraService(AventuraService aventuraService) {
        this.aventuraService = aventuraService;
    }

    /** Prefijo de código por acto: 1→"A1-", 2→"B2-", 3→"C3-" (los del GDD §20). */
    private String prefijo(int acto) {
        return switch (acto) {
            case 1 -> "A1-";
            case 2 -> "B2-";
            case 3 -> "C3-";
            default -> "X" + acto + "-";
        };
    }

    /** Siguiente código libre de un acto: A1-01, A1-02… sin colisionar con los existentes. */
    public String siguienteCodigo(Aventura aventura, int acto) {
        String pref = prefijo(acto);
        int max = 0;
        for (CartaAventura c : aventura.getCartasHistoria()) {
            String code = c.getCode();
            if (code != null && code.startsWith(pref)) {
                try {
                    max = Math.max(max, Integer.parseInt(code.substring(pref.length())));
                } catch (NumberFormatException ignored) {
                    // un código con sufijo no numérico no cuenta para la numeración
                }
            }
        }
        return String.format("%s%02d", pref, max + 1);
    }

    public void anadirCarta(Long aventuraId, CartaAventuraForm form) {
        Aventura aventura = aventuraService.obtenirPerId(aventuraId);
        if (form.getCode() == null || form.getCode().isBlank()) {
            form.setCode(siguienteCodigo(aventura, form.getActo()));
        }
        aventura.getCartasHistoria().add(aModelo(form));
        aventuraService.guardar(aventura);
    }

    public void editarCarta(Long aventuraId, String code, CartaAventuraForm form) {
        Aventura aventura = aventuraService.obtenirPerId(aventuraId);
        List<CartaAventura> cartas = aventura.getCartasHistoria();
        for (int i = 0; i < cartas.size(); i++) {
            if (code.equals(cartas.get(i).getCode())) {
                CartaAventura nueva = aModelo(form);
                nueva.setCode(code);                 // el código no cambia al editar
                cartas.set(i, nueva);
                aventuraService.guardar(aventura);
                return;
            }
        }
        throw new IllegalArgumentException("La carta " + code + " no existe en la aventura " + aventuraId);
    }

    public void eliminarCarta(Long aventuraId, String code) {
        Aventura aventura = aventuraService.obtenirPerId(aventuraId);
        aventura.getCartasHistoria().removeIf(c -> code.equals(c.getCode()));
        aventuraService.guardar(aventura);
    }

    /** Mueve una carta a otro acto y le reasigna un código coherente con el destino. */
    public void moverDeActo(Long aventuraId, String code, int actoDestino) {
        Aventura aventura = aventuraService.obtenirPerId(aventuraId);
        for (CartaAventura c : aventura.getCartasHistoria()) {
            if (code.equals(c.getCode())) {
                c.setActo(actoDestino);
                c.setCode(siguienteCodigo(aventura, actoDestino));
                aventuraService.guardar(aventura);
                return;
            }
        }
        throw new IllegalArgumentException("La carta " + code + " no existe en la aventura " + aventuraId);
    }

    // ==== conversión form ↔ modelo ====

    private CartaAventura aModelo(CartaAventuraForm f) {
        CartaAventura c = new CartaAventura();
        c.setCode(f.getCode());
        c.setActo(f.getActo());
        c.setTitulo(f.getTitulo());
        c.setTipo(CardKind.desde(f.getTipo()));
        c.setEscena(f.getEscena());
        c.setGanchoIgnorar(f.getGanchoIgnorar());

        if (c.getTipo() == CardKind.CADENA && (f.getCadenaOrden() > 0 || f.getCadenaTotal() > 0)) {
            CartaAventura.Cadena cad = new CartaAventura.Cadena();
            cad.setOrden(f.getCadenaOrden());
            cad.setTotal(f.getCadenaTotal());
            c.setCadena(cad);
        }

        if (c.getTipo() == CardKind.CONDICIONAL && noVacio(f.getRequiereCode()) && noVacio(f.getRequiereEstado())) {
            CartaAventura.Requisito req = new CartaAventura.Requisito();
            req.setRequiereCode(f.getRequiereCode().trim());
            req.setEstado(FichaEstado.desde(f.getRequiereEstado()));
            c.setActivacion(req);
        }

        if (c.getTipo() == CardKind.INYECTADA && noVacio(f.getInyeccionCode())) {
            List<CartaAventura.Rama> ramas = new ArrayList<>();
            if (noVacio(f.getInyeccionTextoRoja())) {
                ramas.add(rama(f.getInyeccionCode(), FichaEstado.ROJA, f.getInyeccionTextoRoja()));
            }
            if (noVacio(f.getInyeccionTextoVerde())) {
                ramas.add(rama(f.getInyeccionCode(), FichaEstado.VERDE, f.getInyeccionTextoVerde()));
            }
            c.setRamas(ramas);
        }

        CartaAventura.Referencias ref = new CartaAventura.Referencias();
        ref.setNpcIds(desdeCsv(f.getNpcIdsCsv()));
        ref.setEnemigoIds(desdeCsv(f.getEnemigoIdsCsv()));
        ref.setLocalizacionId(vacioANull(f.getLocalizacionId()));
        ref.setLootTableId(vacioANull(f.getLootTableId()));
        ref.setStoryId(vacioANull(f.getStoryId()));
        c.setReferencias(ref);
        return c;
    }

    public CartaAventuraForm aFormulario(CartaAventura c) {
        CartaAventuraForm f = new CartaAventuraForm();
        f.setCode(c.getCode());
        f.setActo(c.getActo());
        f.setTitulo(c.getTitulo());
        f.setTipo(c.getTipo() == null ? "base" : c.getTipo().json());
        f.setEscena(c.getEscena());
        f.setGanchoIgnorar(c.getGanchoIgnorar());
        if (c.getCadena() != null) {
            f.setCadenaOrden(c.getCadena().getOrden());
            f.setCadenaTotal(c.getCadena().getTotal());
        }
        if (c.getActivacion() != null) {
            f.setRequiereCode(c.getActivacion().getRequiereCode());
            f.setRequiereEstado(c.getActivacion().getEstado() == null ? "" : c.getActivacion().getEstado().json());
        }
        for (CartaAventura.Rama r : c.getRamas()) {
            f.setInyeccionCode(r.getCuandoCode());
            if (r.getCuandoEstado() == FichaEstado.ROJA) f.setInyeccionTextoRoja(r.getTexto());
            if (r.getCuandoEstado() == FichaEstado.VERDE) f.setInyeccionTextoVerde(r.getTexto());
        }
        if (c.getReferencias() != null) {
            f.setNpcIdsCsv(aCsv(c.getReferencias().getNpcIds()));
            f.setEnemigoIdsCsv(aCsv(c.getReferencias().getEnemigoIds()));
            f.setLocalizacionId(c.getReferencias().getLocalizacionId());
            f.setLootTableId(c.getReferencias().getLootTableId());
            f.setStoryId(c.getReferencias().getStoryId());
        }
        return f;
    }

    private CartaAventura.Rama rama(String code, FichaEstado estado, String texto) {
        CartaAventura.Rama r = new CartaAventura.Rama();
        r.setCuandoCode(code.trim());
        r.setCuandoEstado(estado);
        r.setTexto(texto.trim());
        return r;
    }

    private static boolean noVacio(String s) { return s != null && !s.isBlank(); }
    private static String vacioANull(String s) { return noVacio(s) ? s.trim() : null; }

    private static List<String> desdeCsv(String csv) {
        List<String> ids = new ArrayList<>();
        if (csv != null) {
            for (String id : csv.split(",")) {
                if (!id.isBlank()) ids.add(id.trim());
            }
        }
        return ids;
    }

    private static String aCsv(List<String> ids) {
        return ids == null ? "" : String.join(", ", ids);
    }
}
