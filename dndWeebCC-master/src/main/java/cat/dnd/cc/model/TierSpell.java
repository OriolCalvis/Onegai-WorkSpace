package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Hechizo del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.6).
 * Mismo sistema de pilas (recovery) que la Habilidad, pero declara una castingStat de dos posibles
 * orígenes de poder: arcano (INT) o divino/afinidad (CAR).
 */
public class TierSpell {

    private String id;
    private String name;
    private String type = "spell";
    private String school;
    private int tier = 1;
    private String rarity = "common";
    private List<String> classTags = new ArrayList<>();
    private String castingStat;
    private String recovery = "activa";
    private String range;
    private String area;
    private String duration;
    private List<String> mechanicTags = new ArrayList<>();
    private List<String> requiredTags = new ArrayList<>();
    private List<String> incompatibleTags = new ArrayList<>();
    private Effect effect = new Effect();
    private String upgradeConditions;
    private List<String> limitations = new ArrayList<>();
    private String evolvesInto;
    private String flavorText;

    public TierSpell() {
    }

    public static class Effect {
        private String description;
        private String scaling = "INT";

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

    public List<String> getClassTags() {
        return classTags;
    }

    public void setClassTags(List<String> classTags) {
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

    public List<String> getMechanicTags() {
        return mechanicTags;
    }

    public void setMechanicTags(List<String> mechanicTags) {
        this.mechanicTags = mechanicTags;
    }

    public List<String> getRequiredTags() {
        return requiredTags;
    }

    public void setRequiredTags(List<String> requiredTags) {
        this.requiredTags = requiredTags;
    }

    public List<String> getIncompatibleTags() {
        return incompatibleTags;
    }

    public void setIncompatibleTags(List<String> incompatibleTags) {
        this.incompatibleTags = incompatibleTags;
    }

    public Effect getEffect() {
        return effect;
    }

    public void setEffect(Effect effect) {
        this.effect = effect;
    }

    public String getUpgradeConditions() {
        return upgradeConditions;
    }

    public void setUpgradeConditions(String upgradeConditions) {
        this.upgradeConditions = upgradeConditions;
    }

    public List<String> getLimitations() {
        return limitations;
    }

    public void setLimitations(List<String> limitations) {
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
