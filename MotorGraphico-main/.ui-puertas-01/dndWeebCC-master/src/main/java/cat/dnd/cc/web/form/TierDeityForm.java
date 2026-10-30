package cat.dnd.cc.web.form;

public class TierDeityForm {

    private String id;
    private String name;
    private String domain;
    private String favorDescription;
    private String favorScaling = "CAR";
    private String compatibleWith;
    private String obligations;
    private String flavorText;

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

    public String getDomain() {
        return domain;
    }

    public void setDomain(String domain) {
        this.domain = domain;
    }

    public String getFavorDescription() {
        return favorDescription;
    }

    public void setFavorDescription(String favorDescription) {
        this.favorDescription = favorDescription;
    }

    public String getFavorScaling() {
        return favorScaling;
    }

    public void setFavorScaling(String favorScaling) {
        this.favorScaling = favorScaling;
    }

    public String getCompatibleWith() {
        return compatibleWith;
    }

    public void setCompatibleWith(String compatibleWith) {
        this.compatibleWith = compatibleWith;
    }

    public String getObligations() {
        return obligations;
    }

    public void setObligations(String obligations) {
        this.obligations = obligations;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
