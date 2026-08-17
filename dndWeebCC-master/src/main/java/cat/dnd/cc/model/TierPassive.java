package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta Pasiva del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.4).
 * La pasiva de clase es la identidad mecánica de la clase: única, nunca se elimina y nunca hay
 * una segunda pasiva de clase activa a la vez (ver GDD 9.4 y 11.1).
 */
public class TierPassive {

    private String id;
    private String name;
    private String type = "passive";
    private int tier = 1;
    private List<String> classTags = new ArrayList<>();
    private String trigger;
    private Effect effect = new Effect();
    private List<String> limitations = new ArrayList<>();
    private List<String> synergyTags = new ArrayList<>();
    private boolean unique = true;
    private String flavorText;

    public TierPassive() {
    }

    public static class Effect {
        private String description;
        private String scaling = "none";

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

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public List<String> getClassTags() {
        return classTags;
    }

    public void setClassTags(List<String> classTags) {
        this.classTags = classTags;
    }

    public String getTrigger() {
        return trigger;
    }

    public void setTrigger(String trigger) {
        this.trigger = trigger;
    }

    public Effect getEffect() {
        return effect;
    }

    public void setEffect(Effect effect) {
        this.effect = effect;
    }

    public List<String> getLimitations() {
        return limitations;
    }

    public void setLimitations(List<String> limitations) {
        this.limitations = limitations;
    }

    public List<String> getSynergyTags() {
        return synergyTags;
    }

    public void setSynergyTags(List<String> synergyTags) {
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
