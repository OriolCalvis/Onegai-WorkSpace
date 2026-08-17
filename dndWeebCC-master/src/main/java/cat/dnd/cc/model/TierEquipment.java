package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Carta de Equipo (armas, armaduras, objetos) del sistema de tiers y cartas
 * (ver docs/Sistema_Cartas_Tiers.md, sección 9.7 / 16.4).
 */
public class TierEquipment {

    private String id;
    private String name;
    private String type = "equipment";
    private String slot;
    private int tier = 1;
    private String rarity = "common";
    /** Categoría de peso: "ligera" | "media" | "pesada" (símbolos △ ○ □, ver GDD sección 8/9.7). */
    private String weightCategory = "ligera";
    private Map<String, Integer> statBonuses = new LinkedHashMap<>();
    private Map<String, Integer> penalties = new LinkedHashMap<>();
    private List<String> grantedTags = new ArrayList<>();
    private String linkedSkill;
    private Map<String, Integer> requiredStats = new LinkedHashMap<>();
    private List<String> restrictions = new ArrayList<>();
    private String flavorText;

    public TierEquipment() {
    }

    // Conveniencia statBonuses (incluye "health", bono plano de vida)

    public int getBonusCon() {
        return statBonuses.getOrDefault("CON", 0);
    }

    public void setBonusCon(int v) {
        statBonuses.put("CON", v);
    }

    public int getBonusDes() {
        return statBonuses.getOrDefault("DES", 0);
    }

    public void setBonusDes(int v) {
        statBonuses.put("DES", v);
    }

    public int getBonusCar() {
        return statBonuses.getOrDefault("CAR", 0);
    }

    public void setBonusCar(int v) {
        statBonuses.put("CAR", v);
    }

    public int getBonusInt() {
        return statBonuses.getOrDefault("INT", 0);
    }

    public void setBonusInt(int v) {
        statBonuses.put("INT", v);
    }

    public int getBonusHealth() {
        return statBonuses.getOrDefault("health", 0);
    }

    public void setBonusHealth(int v) {
        statBonuses.put("health", v);
    }

    /** Bono de Clase de Armadura (CA) que aporta esta pieza al equiparla (ver sección 5 del GDD). */
    public int getBonusArmor() {
        return statBonuses.getOrDefault("armor", 0);
    }

    public void setBonusArmor(int v) {
        statBonuses.put("armor", v);
    }

    // Conveniencia penalties

    public int getPenaltyCon() {
        return penalties.getOrDefault("CON", 0);
    }

    public void setPenaltyCon(int v) {
        penalties.put("CON", v);
    }

    public int getPenaltyDes() {
        return penalties.getOrDefault("DES", 0);
    }

    public void setPenaltyDes(int v) {
        penalties.put("DES", v);
    }

    public int getPenaltyCar() {
        return penalties.getOrDefault("CAR", 0);
    }

    public void setPenaltyCar(int v) {
        penalties.put("CAR", v);
    }

    public int getPenaltyInt() {
        return penalties.getOrDefault("INT", 0);
    }

    public void setPenaltyInt(int v) {
        penalties.put("INT", v);
    }

    // Conveniencia requiredStats

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

    public String getSlot() {
        return slot;
    }

    public void setSlot(String slot) {
        this.slot = slot;
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

    public String getWeightCategory() {
        return weightCategory;
    }

    public void setWeightCategory(String weightCategory) {
        this.weightCategory = weightCategory;
    }

    public Map<String, Integer> getStatBonuses() {
        return statBonuses;
    }

    public void setStatBonuses(Map<String, Integer> statBonuses) {
        this.statBonuses = statBonuses;
    }

    public Map<String, Integer> getPenalties() {
        return penalties;
    }

    public void setPenalties(Map<String, Integer> penalties) {
        this.penalties = penalties;
    }

    public List<String> getGrantedTags() {
        return grantedTags;
    }

    public void setGrantedTags(List<String> grantedTags) {
        this.grantedTags = grantedTags;
    }

    public String getLinkedSkill() {
        return linkedSkill;
    }

    public void setLinkedSkill(String linkedSkill) {
        this.linkedSkill = linkedSkill;
    }

    public Map<String, Integer> getRequiredStats() {
        return requiredStats;
    }

    public void setRequiredStats(Map<String, Integer> requiredStats) {
        this.requiredStats = requiredStats;
    }

    public List<String> getRestrictions() {
        return restrictions;
    }

    public void setRestrictions(List<String> restrictions) {
        this.restrictions = restrictions;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
