package cat.dnd.cc.model;

public class Exportacio {
    private String nom;
    private String ruta;
    private String tipus;

    public Exportacio(String nom, String ruta, String tipus) {
        this.nom = nom;
        this.ruta = ruta;
        this.tipus = tipus;
    }

    public String getNom() {
        return nom;
    }

    public String getRuta() {
        return ruta;
    }

    public String getTipus() {
        return tipus;
    }
}
