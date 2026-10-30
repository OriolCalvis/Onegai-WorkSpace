package cat.dnd.cc.web.form;

public class TierSpellForm {

    private String id;
    private String name;
    private String school;
    private int tier = 1;
    private String rarity = "common";
    private String classTags;
    private String castingStat = "INT";
    private String recovery = "activa";
    private String range;
    private String area;
    private String duration;
    private String mechanicTags;
    private String requiredTags;
    private String incompatibleTags;
    private String effectDescription;
    private String effectScaling = "INT";
    private String upgradeConditions;
    private String limitations;
    private String evolvesInto;
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

    public String getSchool() {
        return school;
    }

    public void setSchool(String school) {
        this.school = school;
    }

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public String getRarity() {
        return rarity;
    }

    public void setRarity(String rarity) {
        this.rarity = rarity;
    }

    public String getClassTags() {
        return classTags;
    }

    public void setClassTags(String classTags) {
        this.classTags = classTags;
    }

    public String getCastingStat() {
        return castingStat;
    }

    public void setCastingStat(String castingStat) {
        this.castingStat = castingStat;
    }

    public String getRecovery() {
        return recovery;
    }

    public void setRecovery(String recovery) {
        this.recovery = recovery;
    }

    public String getRange() {
        return range;
    }

    public void setRange(String range) {
        this.range = range;
    }

    public String getArea() {
        return area;
    }

    public void setArea(String area) {
        this.area = area;
    }

    public String getDuration() {
        return duration;
    }

    public void setDuration(String duration) {
        this.duration = duration;
    }

    public String getMechanicTags() {
        return mechanicTags;
    }

    public void setMechanicTags(String mechanicTags) {
        this.mechanicTags = mechanicTags;
    }

    public String getRequiredTags() {
        return requiredTags;
    }

    public void setRequiredTags(String requiredTags) {
        this.requiredTags = requiredTags;
    }

    public String getIncompatibleTags() {
        return incompatibleTags;
    }

    public void setIncompatibleTags(String incompatibleTags) {
        this.incompatibleTags = incompatibleTags;
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

    public String getUpgradeConditions() {
        return upgradeConditions;
    }

    public void setUpgradeConditions(String upgradeConditions) {
        this.upgradeConditions = upgradeConditions;
    }

    public String getLimitations() {
        return limitations;
    }

    public void setLimitations(String limitations) {
        this.limitations = limitations;
    }

    public String getEvolvesInto() {
        return evolvesInto;
    }

    public void setEvolvesInto(String evolvesInto) {
        this.evolvesInto = evolvesInto;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
