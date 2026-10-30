package cat.dnd.cc.web.form;

public class TierPassiveForm {

    private String id;
    private String name;
    private int tier = 1;
    private String classTags;
    private String trigger;
    private String effectDescription;
    private String effectScaling = "none";
    private String limitations;
    private String synergyTags;
    private boolean unique = true;
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

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public String getClassTags() {
        return classTags;
    }

    public void setClassTags(String classTags) {
        this.classTags = classTags;
    }

    public String getTrigger() {
        return trigger;
    }

    public void setTrigger(String trigger) {
        this.trigger = trigger;
    }

    public String getEffectDescription() {
        return effectDescription;
    }

    public void setEffectDescription(String effectDescription) {
        this.effectDescription = effectDescription;
    }

    public String getEffectScaling() {
        return effectScaling;
    }

    public void setEffectScaling(String effectScaling) {
        this.effectScaling = effectScaling;
    }

    public String getLimitations() {
        return limitations;
    }

    public void setLimitations(String limitations) {
        this.limitations = limitations;
    }

    public String getSynergyTags() {
        return synergyTags;
    }

    public void setSynergyTags(String synergyTags) {
        this.synergyTags = synergyTags;
    }

    public boolean isUnique() {
        return unique;
    }

    public void setUnique(boolean unique) {
        this.unique = unique;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
