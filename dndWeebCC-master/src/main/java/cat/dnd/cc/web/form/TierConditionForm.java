package cat.dnd.cc.web.form;

public class TierConditionForm {

    private String id;
    private String name;
    private String category = "neutro";
    private String source = "estado";
    private boolean stackable = false;
    private String duration = "1_turn";
    private String effects;
    private String cureConditions;
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

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    public boolean isStackable() {
        return stackable;
    }

    public void setStackable(boolean stackable) {
        this.stackable = stackable;
    }

    public String getDuration() {
        return duration;
    }

    public void setDuration(String duration) {
        this.duration = duration;
    }

    public String getEffects() {
        return effects;
    }

    public void setEffects(String effects) {
        this.effects = effects;
    }

    public String getCureConditions() {
        return cureConditions;
    }

    public void setCureConditions(String cureConditions) {
        this.cureConditions = cureConditions;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
