package cat.dnd.cc.model;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta d'història del sistema de baralles per actes (GDD §20). Reflecteix l'esquema de
 * generació de {@code Plantilla_Prompt_Contenido.md §18b} perquè el contingut produït per IA
 * es pugui deserialitzar directament; també accepta claus angleses del prompt via {@link JsonAlias}
 * ({@code act}→{@code acto}, {@code title}→{@code titulo}, {@code scene}→{@code escena}…).
 * <p>
 * És additiva: viu dins de {@link Aventura#getCartasHistoria()}. Una aventura sense cartes
 * d'història és una aventura plana heretada perfectament vàlida.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class CartaAventura {

    /** Codi narratiu únic dins de l'aventura: acte + número, per exemple "A1-03". */
    private String code;

    /** Acte al qual pertany: 1, 2 o 3. */
    @JsonAlias("act")
    private int acto;

    @JsonAlias("title")
    private String titulo;

    /** BASE per defecte: sense marcador, una carta entra sempre a la baralla. */
    @JsonAlias("cardKind")
    private CardKind tipo = CardKind.BASE;

    /** Només si és CADENA: posició fixa dins de l'acte. */
    private Cadena cadena;

    /** Només si és CONDICIONAL: la fitxa que exigeix d'una carta d'un acte anterior. Màxim 1. */
    @JsonAlias("activation")
    private Requisito activacion;

    /** Text que llegeix o parafraseja el director (cara de jugador). */
    @JsonAlias("scene")
    private String escena;

    /** Branques condicionals internes; obligatori com a mínim 2 en cartes INYECTADAS. */
    @JsonAlias("branches")
    private List<Rama> ramas = new ArrayList<>();

    /** Línia "Si ignoren això..." que es llegeix en col·locar la fitxa roja (còpia del director). */
    @JsonAlias("ignoreHook")
    private String ganchoIgnorar;

    /** Enllaços a catàleg real (npc, enemic, botí, localització, història). */
    @JsonAlias("references")
    private Referencias referencias = new Referencias();

    /** Només si és BALANCE_FINAL: taula de conseqüències per recompte de fitxes. */
    @JsonAlias("balanceTable")
    private List<FilaBalance> balance = new ArrayList<>();

    public CartaAventura() {
    }

    /** Posició fixa d'una carta de cadena: "ordre de total". */
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class Cadena {
        private int orden;
        private int total;

        public int getOrden() { return orden; }
        public void setOrden(int orden) { this.orden = orden; }
        public int getTotal() { return total; }
        public void setTotal(int total) { this.total = total; }
    }

    /** Requisit d'entrada: la carta {@code requiereCode} ha de tenir la fitxa {@code estado}. */
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class Requisito {
        @JsonAlias("requires")
        private String requiereCode;
        @JsonAlias("state")
        private FichaEstado estado;

        public String getRequiereCode() { return requiereCode; }
        public void setRequiereCode(String requiereCode) { this.requiereCode = requiereCode; }
        public FichaEstado getEstado() { return estado; }
        public void setEstado(FichaEstado estado) { this.estado = estado; }
    }

    /** Branca condicional: "quan la carta {@code cuandoCode} té fitxa {@code cuandoEstado}". */
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class Rama {
        @JsonAlias("whenCode")
        private String cuandoCode;
        @JsonAlias("whenState")
        private FichaEstado cuandoEstado;
        @JsonAlias("text")
        private String texto;

        public String getCuandoCode() { return cuandoCode; }
        public void setCuandoCode(String cuandoCode) { this.cuandoCode = cuandoCode; }
        public FichaEstado getCuandoEstado() { return cuandoEstado; }
        public void setCuandoEstado(FichaEstado cuandoEstado) { this.cuandoEstado = cuandoEstado; }
        public String getTexto() { return texto; }
        public void setTexto(String texto) { this.texto = texto; }
    }

    /** Enllaços de la carta al catàleg real, tots per id. */
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class Referencias {
        private List<String> npcIds = new ArrayList<>();
        @JsonAlias("enemyIds")
        private List<String> enemigoIds = new ArrayList<>();
        @JsonAlias("locationId")
        private String localizacionId;
        private String lootTableId;
        private String storyId;

        public List<String> getNpcIds() { return npcIds; }
        public void setNpcIds(List<String> npcIds) { this.npcIds = npcIds; }
        public List<String> getEnemigoIds() { return enemigoIds; }
        public void setEnemigoIds(List<String> enemigoIds) { this.enemigoIds = enemigoIds; }
        public String getLocalizacionId() { return localizacionId; }
        public void setLocalizacionId(String localizacionId) { this.localizacionId = localizacionId; }
        public String getLootTableId() { return lootTableId; }
        public void setLootTableId(String lootTableId) { this.lootTableId = lootTableId; }
        public String getStoryId() { return storyId; }
        public void setStoryId(String storyId) { this.storyId = storyId; }
    }

    /** Fila del balanç final: si es compleix {@code condicion}, es llegeix {@code epilogo}. */
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class FilaBalance {
        @JsonAlias("condition")
        private String condicion;
        @JsonAlias("epilogue")
        private String epilogo;

        public String getCondicion() { return condicion; }
        public void setCondicion(String condicion) { this.condicion = condicion; }
        public String getEpilogo() { return epilogo; }
        public void setEpilogo(String epilogo) { this.epilogo = epilogo; }
    }

    // ==== conveniencia ====

    /** Una carta és de filtre segur (entra sempre) si és Base o Inyectada. */
    public boolean entraSiempre() {
        return tipo == CardKind.BASE || tipo == CardKind.INYECTADA;
    }

    // ==== getters / setters ====

    public String getCode() { return code; }
    public void setCode(String code) { this.code = code; }
    public int getActo() { return acto; }
    public void setActo(int acto) { this.acto = acto; }
    public String getTitulo() { return titulo; }
    public void setTitulo(String titulo) { this.titulo = titulo; }
    public CardKind getTipo() { return tipo; }
    public void setTipo(CardKind tipo) { this.tipo = tipo; }
    public Cadena getCadena() { return cadena; }
    public void setCadena(Cadena cadena) { this.cadena = cadena; }
    public Requisito getActivacion() { return activacion; }
    public void setActivacion(Requisito activacion) { this.activacion = activacion; }
    public String getEscena() { return escena; }
    public void setEscena(String escena) { this.escena = escena; }
    public List<Rama> getRamas() { return ramas; }
    public void setRamas(List<Rama> ramas) { this.ramas = ramas; }
    public String getGanchoIgnorar() { return ganchoIgnorar; }
    public void setGanchoIgnorar(String ganchoIgnorar) { this.ganchoIgnorar = ganchoIgnorar; }
    public Referencias getReferencias() { return referencias; }
    public void setReferencias(Referencias referencias) { this.referencias = referencias; }
    public List<FilaBalance> getBalance() { return balance; }
    public void setBalance(List<FilaBalance> balance) { this.balance = balance; }
}
