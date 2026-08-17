package cat.dnd.cc.service;

import cat.dnd.cc.model.Aventura;
import cat.dnd.cc.model.CardKind;
import cat.dnd.cc.model.CartaAventura;
import cat.dnd.cc.model.FichaEstado;
import cat.dnd.cc.repository.CatalogoAventuraRepository;
import com.fasterxml.jackson.databind.node.JsonNodeFactory;
import com.fasterxml.jackson.databind.node.ObjectNode;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.Mockito.when;

/**
 * Tests unitarios (sin contexto Spring) de las 6 reglas duras del GDD §20.4 / prompt §18c
 * que implementa {@link ValidadorAventuraService}. Cada test aísla una regla filtrando la
 * lista de incidencias por su nombre, en vez de construir una aventura "perfecta" que
 * cumpla las seis a la vez: así cada caso es legible por separado.
 */
class ValidadorAventuraServiceTest {

    private HistoriaService historiaService;
    private CatalogoAventuraRepository catalogo;
    private ValidadorAventuraService validador;

    @BeforeEach
    void setUp() {
        historiaService = Mockito.mock(HistoriaService.class);
        catalogo = Mockito.mock(CatalogoAventuraRepository.class);
        validador = new ValidadorAventuraService(historiaService, catalogo);
    }

    // ==== helpers de construcción ====

    private static CartaAventura carta(String code, int acto, CardKind tipo) {
        CartaAventura c = new CartaAventura();
        c.setCode(code);
        c.setActo(acto);
        c.setTipo(tipo);
        c.setTitulo(code);
        return c;
    }

    private static void requiere(CartaAventura carta, String code, FichaEstado estado) {
        CartaAventura.Requisito req = new CartaAventura.Requisito();
        req.setRequiereCode(code);
        req.setEstado(estado);
        carta.setActivacion(req);
    }

    private static void rama(CartaAventura carta, String code, FichaEstado estado, String texto) {
        CartaAventura.Rama r = new CartaAventura.Rama();
        r.setCuandoCode(code);
        r.setCuandoEstado(estado);
        r.setTexto(texto);
        carta.getRamas().add(r);
    }

    private static void cadena(CartaAventura carta, int orden, int total) {
        CartaAventura.Cadena cad = new CartaAventura.Cadena();
        cad.setOrden(orden);
        cad.setTotal(total);
        carta.setCadena(cad);
    }

    private static Aventura aventuraConCartas(CartaAventura... cartas) {
        Aventura av = new Aventura();
        av.setId(1L);
        av.setNom("Aventura de prueba");
        List<CartaAventura> lista = new ArrayList<>(List.of(cartas));
        av.setCartasHistoria(lista);
        return av;
    }

    private static boolean hayIncidencia(List<ValidadorAventuraService.Incidencia> incidencias, String regla,
                                          ValidadorAventuraService.Gravedad gravedad, String code) {
        return incidencias.stream().anyMatch(i -> regla.equals(i.regla()) && gravedad == i.gravedad()
                && (code == null || code.equals(i.code())));
    }

    // ==== aventura plana ====

    @Test
    void aventuraPlanaNoTieneNadaQueValidar() {
        Aventura plana = new Aventura();
        plana.setNom("Plana");
        assertTrue(validador.validar(plana).isEmpty());
        assertTrue(validador.esValida(plana));
    }

    // ==== Regla 1 · filtro sano ====

    @Test
    void requisitoQueApuntaAActoPosteriorEsError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.CONDICIONAL);
        requiere(a1, "B2-01", FichaEstado.VERDE);
        CartaAventura b2 = carta("B2-01", 2, CardKind.BASE);

        Aventura av = aventuraConCartas(a1, b2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "filtro-sano", ValidadorAventuraService.Gravedad.ERROR, "A1-01"));
    }

    @Test
    void requisitoQueApuntaACodigoInexistenteEsError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.CONDICIONAL);
        requiere(a1, "A1-99", FichaEstado.VERDE);

        Aventura av = aventuraConCartas(a1);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "filtro-sano", ValidadorAventuraService.Gravedad.ERROR, "A1-01"));
    }

    @Test
    void requisitoACodigoDeActoAnteriorNoDaError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        CartaAventura b2 = carta("B2-01", 2, CardKind.CONDICIONAL);
        requiere(b2, "A1-01", FichaEstado.VERDE);

        Aventura av = aventuraConCartas(a1, b2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(hayIncidencia(incidencias, "filtro-sano", ValidadorAventuraService.Gravedad.ERROR, "B2-01"));
    }

    // ==== Regla 2 · efecto mariposa ====

    @Test
    void cartaDeActoUnoSinConsecuenciaEsAviso() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);

        Aventura av = aventuraConCartas(a1);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "efecto-mariposa", ValidadorAventuraService.Gravedad.AVISO, "A1-01"));
    }

    @Test
    void cartaReferenciadaPorRequisitoNoDaAviso() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        CartaAventura b2 = carta("B2-01", 2, CardKind.CONDICIONAL);
        requiere(b2, "A1-01", FichaEstado.ROJA);

        Aventura av = aventuraConCartas(a1, b2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(hayIncidencia(incidencias, "efecto-mariposa", ValidadorAventuraService.Gravedad.AVISO, "A1-01"));
    }

    @Test
    void cartaReferenciadaPorRamaInyectadaNoDaAviso() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        CartaAventura b2 = carta("B2-01", 2, CardKind.INYECTADA);
        rama(b2, "A1-01", FichaEstado.ROJA, "El bosque arde.");
        rama(b2, "A1-01", FichaEstado.VERDE, "El bosque florece.");

        Aventura av = aventuraConCartas(a1, b2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(hayIncidencia(incidencias, "efecto-mariposa", ValidadorAventuraService.Gravedad.AVISO, "A1-01"));
    }

    @Test
    void cartaDeActoTresNuncaPideConsecuencia() {
        CartaAventura c3 = carta("C3-01", 3, CardKind.BASE);

        Aventura av = aventuraConCartas(c3);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(hayIncidencia(incidencias, "efecto-mariposa", ValidadorAventuraService.Gravedad.AVISO, "C3-01"));
    }

    // ==== Regla 3 · válvula de escape ====

    @Test
    void porcentajeBaseInsuficienteEsError() {
        List<CartaAventura> cartas = new ArrayList<>();
        for (int i = 1; i <= 5; i++) {
            cartas.add(carta("B2-0" + i, 2, CardKind.CONDICIONAL)); // 0% Base
        }
        Aventura av = aventuraConCartas(cartas.toArray(new CartaAventura[0]));
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(incidencias.stream().anyMatch(i -> "valvula-escape".equals(i.regla())
                && i.gravedad() == ValidadorAventuraService.Gravedad.ERROR
                && i.mensaje().contains("Acto 2")));
    }

    @Test
    void peorCasoConMenosDeTresSupervivientesEsError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        CartaAventura b2cond = carta("B2-01", 2, CardKind.CONDICIONAL);
        requiere(b2cond, "A1-01", FichaEstado.VERDE); // no sobrevive si A1-01 sale Roja
        CartaAventura b2base = carta("B2-02", 2, CardKind.BASE); // único superviviente seguro

        Aventura av = aventuraConCartas(a1, b2cond, b2base);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(incidencias.stream().anyMatch(i -> "valvula-escape".equals(i.regla())
                && i.gravedad() == ValidadorAventuraService.Gravedad.ERROR
                && i.mensaje().contains("Acto 2") && i.mensaje().contains("sobreviven")));
    }

    @Test
    void peorCasoConSupervivientesSuficientesNoDaError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        CartaAventura b1 = carta("B2-01", 2, CardKind.BASE);
        CartaAventura b2 = carta("B2-02", 2, CardKind.BASE);
        CartaAventura b3 = carta("B2-03", 2, CardKind.BASE);

        Aventura av = aventuraConCartas(a1, b1, b2, b3);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(incidencias.stream().anyMatch(i -> "valvula-escape".equals(i.regla())
                && i.mensaje().contains("Acto 2")));
    }

    // ==== Regla 4 · cadenas ====

    @Test
    void cadenaConActivacionEsError() {
        CartaAventura c = carta("A1-01", 1, CardKind.CADENA);
        cadena(c, 1, 1);
        requiere(c, "A1-99", FichaEstado.VERDE); // una cadena nunca debería tener requisito

        Aventura av = aventuraConCartas(c);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "cadena", ValidadorAventuraService.Gravedad.ERROR, "A1-01"));
    }

    @Test
    void cadenaConHuecoEnNumeracionEsError() {
        CartaAventura c1 = carta("A1-01", 1, CardKind.CADENA);
        cadena(c1, 1, 3);
        CartaAventura c2 = carta("A1-02", 1, CardKind.CADENA);
        cadena(c2, 3, 3); // falta el 2/3

        Aventura av = aventuraConCartas(c1, c2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(incidencias.stream().anyMatch(i -> "cadena".equals(i.regla())
                && i.gravedad() == ValidadorAventuraService.Gravedad.ERROR
                && i.mensaje().contains("posición 2")));
    }

    @Test
    void cadenaCompletaYSinRequisitoNoDaError() {
        CartaAventura c1 = carta("A1-01", 1, CardKind.CADENA);
        cadena(c1, 1, 2);
        CartaAventura c2 = carta("A1-02", 1, CardKind.CADENA);
        cadena(c2, 2, 2);

        Aventura av = aventuraConCartas(c1, c2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(incidencias.stream().anyMatch(i -> "cadena".equals(i.regla())));
    }

    // ==== Regla 5 · Balance Final ====

    @Test
    void sinCartaDeBalanceFinalEsError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);

        Aventura av = aventuraConCartas(a1);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "balance-final", ValidadorAventuraService.Gravedad.ERROR, null));
    }

    @Test
    void dosCartasDeBalanceFinalEsError() {
        CartaAventura b1 = carta("C3-98", 3, CardKind.BALANCE_FINAL);
        CartaAventura.FilaBalance fila1 = new CartaAventura.FilaBalance();
        fila1.setCondicion("cond");
        fila1.setEpilogo("epi");
        b1.setBalance(List.of(fila1));
        CartaAventura b2 = carta("C3-99", 3, CardKind.BALANCE_FINAL);
        b2.setBalance(List.of(fila1));

        Aventura av = aventuraConCartas(b1, b2);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "balance-final", ValidadorAventuraService.Gravedad.ERROR, null));
    }

    @Test
    void balanceFinalSinFilasEsAviso() {
        CartaAventura balance = carta("C3-99", 3, CardKind.BALANCE_FINAL);

        Aventura av = aventuraConCartas(balance);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "balance-final", ValidadorAventuraService.Gravedad.AVISO, "C3-99"));
    }

    // ==== Regla 6 · referencias ====

    @Test
    void referenciaANpcInexistenteEsAviso() {
        when(catalogo.completa("npc_fantasma")).thenReturn(null);

        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        a1.getReferencias().getNpcIds().add("npc_fantasma");

        Aventura av = aventuraConCartas(a1);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "referencias", ValidadorAventuraService.Gravedad.AVISO, "A1-01"));
    }

    @Test
    void referenciaANpcExistenteNoDaAviso() {
        ObjectNode nodo = JsonNodeFactory.instance.objectNode();
        when(catalogo.completa("npc_real")).thenReturn(nodo);

        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        a1.getReferencias().getNpcIds().add("npc_real");

        Aventura av = aventuraConCartas(a1);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertFalse(hayIncidencia(incidencias, "referencias", ValidadorAventuraService.Gravedad.AVISO, "A1-01"));
    }

    @Test
    void referenciaAHistoriaInexistenteEsAviso() {
        when(historiaService.existe("hist_fantasma")).thenReturn(false);

        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE);
        a1.getReferencias().setStoryId("hist_fantasma");

        Aventura av = aventuraConCartas(a1);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(hayIncidencia(incidencias, "referencias", ValidadorAventuraService.Gravedad.AVISO, "A1-01"));
    }

    // ==== esValida() ====

    @Test
    void esValidaEsFalsoSiHayAlMenosUnError() {
        CartaAventura a1 = carta("A1-01", 1, CardKind.CONDICIONAL);
        requiere(a1, "A1-99", FichaEstado.VERDE); // filtro-sano: ERROR

        Aventura av = aventuraConCartas(a1);
        assertFalse(validador.esValida(av));
    }

    @Test
    void esValidaEsVerdaderoSiSoloHayAvisos() {
        // Acto III necesita cartas de mazo reales además del Balance Final (si no, el peor
        // caso de la válvula de escape se queda a 0 supervivientes y sí sería ERROR).
        CartaAventura a1 = carta("A1-01", 1, CardKind.BASE); // sin consecuencia → aviso
        CartaAventura c1 = carta("C3-01", 3, CardKind.BASE);
        CartaAventura c2 = carta("C3-02", 3, CardKind.BASE);
        CartaAventura c3 = carta("C3-03", 3, CardKind.BASE);
        CartaAventura balance = carta("C3-99", 3, CardKind.BALANCE_FINAL); // sin filas → aviso

        Aventura av = aventuraConCartas(a1, c1, c2, c3, balance);
        List<ValidadorAventuraService.Incidencia> incidencias = validador.validar(av);

        assertTrue(incidencias.stream().allMatch(i -> i.gravedad() == ValidadorAventuraService.Gravedad.AVISO));
        assertTrue(validador.esValida(av));
    }
}
