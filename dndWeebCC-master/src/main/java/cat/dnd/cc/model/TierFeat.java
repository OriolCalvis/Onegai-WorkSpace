package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Carta de Dote del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.6).
 * Una dote es una mejora narrativa/mecánica puntual (no consume recurso por turno, a diferencia
 * de una habilidad): un rasgo permanente que se activa al elegirla.
 */
public class TierFeat {

    private String id;
    private String name;
    private String type = "feat";
    private int tier = 1;
    private String rarity = "common";
    private List<String> classTags = new ArrayList<>();
    private Map<String, Integer> requiredStats = new LinkedHashMap<>();
    private List<String> requiredTags = new ArrayList<>();
    private List<String> incompatibleTags = new ArrayList<>();
    private List<String> grantedTags = new ArrayList<>();
    private Effect effect = new Effect();
    private List<String> limitations = new ArrayList<>();
    private String flavorText;

    public TierFeat() {
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

    // Conveniencia para requiredStats

    public int getReqCon() {
        return requiredStats.getOrDefault("CON", 0);
    }

    public void setReqCon(int v) {
        requiredStats.put("CON", v);
    }

    public int getReqDes() {
        return requiredStats.getOrDefault("DES", 0);
    }

    public void setReqDes(int v) {
        requiredStats.put("DES", v);
    }

    public int getReqCar() {
        return requiredStats.getOrDefault("CAR", 0);
    }

    public void setReqCar(int v) {
        requiredStats.put("CAR", v);
    }

    public int getReqInt() {
        return requiredStats.getOrDefault("INT", 0);
    }

    public void setReqInt(int v) {
        requiredStats.put("INT", v);
    }

    // Getters / setters

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

    public Map<String, Integer> getRequiredStats() {
        return requiredStats;
    }

    public void setRequiredStats(Map<String, Integer> requiredStats) {
        this.requiredStats = requiredStats;
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

    public List<String> getGrantedTags() {
        return grantedTags;
    }

    public void setGrantedTags(List<String> grantedTags) {
        this.grantedTags = grantedTags;
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

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
