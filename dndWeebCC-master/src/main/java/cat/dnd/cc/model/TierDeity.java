package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Deidad/Afinidad del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.11).
 * Vincula con CAR (afinidad divina) y con compatibleDeities de la carta de Raza.
 */
public class TierDeity {

    private String id;
    private String name;
    private String type = "deity";
    private String domain;
    private Favor favor = new Favor();
    private List<String> compatibleWith = new ArrayList<>();
    private List<String> obligations = new ArrayList<>();
    private String flavorText;

    public TierDeity() {
    }

    public static class Favor {
        private String description;
        private String scaling = "CAR";

        public String getDescription() {
            return description;
        }

        public void setDescription(String description) {
            this.description = description;
        }

        public String getScaling() {
            return scaling;
        }

        public void setScaling(String scaling) {
            this.scaling = scaling;
        }
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getDomain() {
        return domain;
    }

    public void setDomain(String domain) {
        this.domain = domain;
    }

    public Favor getFavor() {
        return favor;
    }

    public void setFavor(Favor favor) {
        this.favor = favor;
    }

    public List<String> getCompatibleWith() {
        return compatibleWith;
    }

    public void setCompatibleWith(List<String> compatibleWith) {
        this.compatibleWith = compatibleWith;
    }

    public List<String> getObligations() {
        return obligations;
    }

    public void setObligations(List<String> obligations) {
        this.obligations = obligations;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
