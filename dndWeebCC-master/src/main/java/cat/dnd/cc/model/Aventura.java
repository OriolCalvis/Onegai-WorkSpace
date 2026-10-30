package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Una aventura és una recopilació ordenada de cartes d'història: el director tria
 * els ganxos que formaran la campanya o sessió. L'ordre narratiu real el donen les
 * trames mare (chain.trama + chain.step) de les històries triades.
 */
public class Aventura {

    private Long id;
    private String nom;
    private String descripcion;
    private List<String> historiaIds = new ArrayList<>();
    // ==== La taula del director ====
    private List<String> npcIds = new ArrayList<>();        // repartiment (data/npcs)
    private List<String> deidadIds = new ArrayList<>();     // déus presents (data/cartas/deidades)
    private List<String> villanoIds = new ArrayList<>();    // enemics importants (amb villainProfile)
    private List<String> enemigoIds = new ArrayList<>();    // enemics menors i elits soltes
    private List<String> lootIds = new ArrayList<>();       // taules de botí (data/loot)
    private List<String> trampaIds = new ArrayList<>();     // trampes (data/cartas/trampas)
    private List<Combate> combates = new ArrayList<>();     // encontres preparats
    private String notas;

    // ==== Baralla d'història per actes (GDD §20), additiva i opcional ====
    // Buit = aventura plana heretada. Amb contingut = aventura per actes.
    private List<CartaAventura> cartasHistoria = new ArrayList<>();
    private String tema;                 // etiqueta de galeria (Fantasia / Misteri / ...)
    private String estado = "borrador";  // borrador | en_curso | completa
    private String arquitectura;         // espina_de_pescado | facciones | crawler (GDD §20.5)

    public Aventura() {
    }

    /** Un encontre preparat: nom, tier objectiu i enemics (ids separats per coma). */
    public static class Combate {
        private String nom;
        private int tier = 1;
        private String enemigoIdsCsv = "";
        private String notas;

        public String getNom() { return nom; }
        public void setNom(String nom) { this.nom = nom; }
        public int getTier() { return tier; }
        public void setTier(int tier) { this.tier = tier; }
        public String getEnemigoIdsCsv() { return enemigoIdsCsv; }
        public void setEnemigoIdsCsv(String enemigoIdsCsv) { this.enemigoIdsCsv = enemigoIdsCsv; }
        public String getNotas() { return notas; }
        public void setNotas(String notas) { this.notas = notas; }

        /** Ids d'enemics de l'encontre, ja netejats. */
        public List<String> getEnemigoIds() {
            List<String> ids = new ArrayList<>();
            if (enemigoIdsCsv != null) {
                for (String id : enemigoIdsCsv.split(",")) {
                    if (!id.isBlank()) {
                        ids.add(id.trim());
                    }
                }
            }
            return ids;
        }
    }

    public Long getId() { return id; }
    public void setId(Long id) { this.id = id; }
    public String getNom() { return nom; }
    public void setNom(String nom) { this.nom = nom; }
    public String getDescripcion() { return descripcion; }
    public void setDescripcion(String descripcion) { this.descripcion = descripcion; }
    public List<String> getHistoriaIds() { return historiaIds; }
    public void setHistoriaIds(List<String> historiaIds) { this.historiaIds = historiaIds; }
    public List<String> getNpcIds() { return npcIds; }
    public void setNpcIds(List<String> npcIds) { this.npcIds = npcIds; }
    public List<String> getDeidadIds() { return deidadIds; }
    public void setDeidadIds(List<String> deidadIds) { this.deidadIds = deidadIds; }
    public List<String> getVillanoIds() { return villanoIds; }
    public void setVillanoIds(List<String> villanoIds) { this.villanoIds = villanoIds; }
    public List<String> getEnemigoIds() { return enemigoIds; }
    public void setEnemigoIds(List<String> enemigoIds) { this.enemigoIds = enemigoIds; }
    public List<String> getLootIds() { return lootIds; }
    public void setLootIds(List<String> lootIds) { this.lootIds = lootIds; }
    public List<String> getTrampaIds() { return trampaIds; }
    public void setTrampaIds(List<String> trampaIds) { this.trampaIds = trampaIds; }
    public List<Combate> getCombates() { return combates; }
    public void setCombates(List<Combate> combates) { this.combates = combates; }
    public String getNotas() { return notas; }
    public void setNotas(String notas) { this.notas = notas; }

    public List<CartaAventura> getCartasHistoria() { return cartasHistoria; }
    public void setCartasHistoria(List<CartaAventura> cartasHistoria) { this.cartasHistoria = cartasHistoria; }
    public String getTema() { return tema; }
    public void setTema(String tema) { this.tema = tema; }
    public String getEstado() { return estado; }
    public void setEstado(String estado) { this.estado = estado; }
    public String getArquitectura() { return arquitectura; }
    public void setArquitectura(String arquitectura) { this.arquitectura = arquitectura; }

    // ==== Conveniència per a plantilles i validador (GDD §20) ====

    /** true si l'aventura usa el sistema de baralla per actes (té cartes d'història). */
    public boolean esPorActos() {
        return cartasHistoria != null && !cartasHistoria.isEmpty();
    }

    /** Cartes d'un acte concret (1, 2 o 3), en l'ordre en què estan guardades. */
    public List<CartaAventura> cartasDeActo(int acto) {
        List<CartaAventura> resultado = new ArrayList<>();
        if (cartasHistoria != null) {
            for (CartaAventura c : cartasHistoria) {
                if (c.getActo() == acto) {
                    resultado.add(c);
                }
            }
        }
        return resultado;
    }

    /**
     * Percentatge de cartes que entren sempre en un acte (0-100). La regla de "vàlvula de sortida"
     * del GDD §20.4 demana com a mínim un 40 % de cartes segures per acte; el validador ho usa.
     * Un acte buit retorna 0.
     */
    public int porcentajeBase(int acto) {
        List<CartaAventura> cartas = cartasDeActo(acto);
        if (cartas.isEmpty()) {
            return 0;
        }
        long base = cartas.stream().filter(CartaAventura::entraSiempre).count();
        return (int) Math.round(100.0 * base / cartas.size());
    }
}
