package cat.dnd.cc.web.form;

/**
 * DTO plano para crear/editar una Carta de Historia desde el Constructor de aventuras
 * (GDD §20). Thymeleaf lo bindea con {@code th:field}; {@code ConstructorAventuraService} lo
 * convierte a {@link cat.dnd.cc.model.CartaAventura} y viceversa.
 * <p>
 * Cubre el caso común del wireframe 12a (título, escena, requisito, inyección de 2 ramas). El
 * Balance Final y las ramas múltiples arbitrarias se editan por importación JSON (§18b); esta
 * primera versión del formulario no las expone.
 */
public class CartaAventuraForm {

    private String code;                 // vacío al crear → lo autogenera el servicio
    private int acto = 1;
    private String titulo;
    private String tipo = "base";        // base|condicional|inyectada|cadena|balance_final

    // Cadena (🔗)
    private int cadenaOrden;
    private int cadenaTotal;

    // Requisito de entrada (solo condicional)
    private String requiereCode;
    private String requiereEstado;       // "" | verde | roja

    private String escena;
    private String ganchoIgnorar;

    // Inyección (solo inyectada): una carta anterior + textos de cada ficha
    private String inyeccionCode;
    private String inyeccionTextoRoja;
    private String inyeccionTextoVerde;

    // Referencias al catálogo (CSV de ids)
    private String npcIdsCsv;
    private String enemigoIdsCsv;
    private String localizacionId;
    private String lootTableId;
    private String storyId;

    public String getCode() { return code; }
    public void setCode(String code) { this.code = code; }
    public int getActo() { return acto; }
    public void setActo(int acto) { this.acto = acto; }
    public String getTitulo() { return titulo; }
    public void setTitulo(String titulo) { this.titulo = titulo; }
    public String getTipo() { return tipo; }
    public void setTipo(String tipo) { this.tipo = tipo; }
    public int getCadenaOrden() { return cadenaOrden; }
    public void setCadenaOrden(int cadenaOrden) { this.cadenaOrden = cadenaOrden; }
    public int getCadenaTotal() { return cadenaTotal; }
    public void setCadenaTotal(int cadenaTotal) { this.cadenaTotal = cadenaTotal; }
    public String getRequiereCode() { return requiereCode; }
    public void setRequiereCode(String requiereCode) { this.requiereCode = requiereCode; }
    public String getRequiereEstado() { return requiereEstado; }
    public void setRequiereEstado(String requiereEstado) { this.requiereEstado = requiereEstado; }
    public String getEscena() { return escena; }
    public void setEscena(String escena) { this.escena = escena; }
    public String getGanchoIgnorar() { return ganchoIgnorar; }
    public void setGanchoIgnorar(String ganchoIgnorar) { this.ganchoIgnorar = ganchoIgnorar; }
    public String getInyeccionCode() { return inyeccionCode; }
    public void setInyeccionCode(String inyeccionCode) { this.inyeccionCode = inyeccionCode; }
    public String getInyeccionTextoRoja() { return inyeccionTextoRoja; }
    public void setInyeccionTextoRoja(String inyeccionTextoRoja) { this.inyeccionTextoRoja = inyeccionTextoRoja; }
    public String getInyeccionTextoVerde() { return inyeccionTextoVerde; }
    public void setInyeccionTextoVerde(String inyeccionTextoVerde) { this.inyeccionTextoVerde = inyeccionTextoVerde; }
    public String getNpcIdsCsv() { return npcIdsCsv; }
    public void setNpcIdsCsv(String npcIdsCsv) { this.npcIdsCsv = npcIdsCsv; }
    public String getEnemigoIdsCsv() { return enemigoIdsCsv; }
    public void setEnemigoIdsCsv(String enemigoIdsCsv) { this.enemigoIdsCsv = enemigoIdsCsv; }
    public String getLocalizacionId() { return localizacionId; }
    public void setLocalizacionId(String localizacionId) { this.localizacionId = localizacionId; }
    public String getLootTableId() { return lootTableId; }
    public void setLootTableId(String lootTableId) { this.lootTableId = lootTableId; }
    public String getStoryId() { return storyId; }
    public void setStoryId(String storyId) { this.storyId = storyId; }
}
