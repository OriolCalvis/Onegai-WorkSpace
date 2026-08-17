package cat.dnd.cc.model;

import java.util.List;

/**
 * Representación uniforme de UNA carta para la vista de impresión de una aventura
 * (aventuras/imprimir.html), sea cual sea su tipo real de origen (historia, deidad,
 * npc, villano, enemigo, loot o trampa).
 * <p>
 * Existe para poder mezclar los siete tipos en UNA sola lista y partirla en páginas
 * de tamaño fijo (9 cartas) en el controlador — con una lista ya troceada, la
 * plantilla no necesita saber de qué tipo es cada carta ni el motor de impresión
 * tiene que fragmentar un contenedor largo entre hojas (la causa real de los cortes
 * y páginas en blanco: grid/flex no paginan bien cuando su contenido no cabe en una
 * sola hoja). Cada página generada aquí siempre cabe en una hoja A4 por construcción.
 */
public class CartaImpresion {

    private final String tipoIcono;
    private final String tier;
    private final String titulo;
    private final String rol;
    private final String meta;
    private final String desc;
    private final List<String> badges;
    private final String idLabel;

    public CartaImpresion(String tipoIcono, String tier, String titulo, String rol,
                           String meta, String desc, List<String> badges, String idLabel) {
        this.tipoIcono = tipoIcono;
        this.tier = tier;
        this.titulo = titulo;
        this.rol = rol;
        this.meta = meta;
        this.desc = desc;
        this.badges = badges;
        this.idLabel = idLabel;
    }

    public String getTipoIcono() { return tipoIcono; }
    public String getTier() { return tier; }
    public String getTitulo() { return titulo; }
    public String getRol() { return rol; }
    public String getMeta() { return meta; }
    public String getDesc() { return desc; }
    public List<String> getBadges() { return badges; }
    public String getIdLabel() { return idLabel; }
}
