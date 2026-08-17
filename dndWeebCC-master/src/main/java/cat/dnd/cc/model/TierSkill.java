package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Carta de Habilidad del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.5 / 16.1).
 */
public class TierSkill {

    private String id;
    private String name;
    private String type = "skill";
    private int tier = 1;
    private String rarity = "common";
    private List<String> classTags = new ArrayList<>();
    private List<String> roleTags = new ArrayList<>();
    private List<String> mechanicTags = new ArrayList<>();
    private Map<String, Integer> requiredStats = new LinkedHashMap<>();
    private List<String> requiredTags = new ArrayList<>();
    private List<String> incompatibleTags = new ArrayList<>();
    /** @deprecated heredado de la edición 1 (sistema de puntos de recurso). Sustituido por {@link #recovery}
     *  (GDD sección 6). Se mantiene solo para no perder datos de cartas antiguas todavía sin migrar. */
    @Deprecated
    private Cost cost = new Cost();
    /** A qué pila va la carta tras usarse: activa | descanso_corto | descanso_largo | ninguno (GDD sección 6). */
    private String recovery = "activa";
    private String actionType;
    private String range;
    private String duration;
    /** A qué defensa apunta si es una tirada enfrentada (GDD 9.5): CA | defensa_mental | resistencia_fisica. */
    private String defenseStat;
    private Effect effect = new Effect();
    private List<String> limitations = new ArrayList<>();
    /** @deprecated heredado de la edición 1. Sustituido por {@link #evolvesInto} (GDD sección 9.12: la
     *  carta mejorada ocupa el mismo lugar que la anterior, en vez de una lista abierta de mejoras). */
    @Deprecated
    private List<String> upgradePath = new ArrayList<>();
    /** ID de la versión evolucionada de esta carta (GDD 9.12), o null si es la versión final de su línea. */
    private String evolvesInto;
    private String flavorText;

    public TierSkill() {
    }

    public static class Cost {
        private String resource = "none";
        private int amount;

        public String getResource() {
            return resource;
        }

        public void setResource(String resource) {
            this.resource = resource;
        }

        public int getAmount() {
            return amount;
        }

        public void setAmount(int amount) {
            this.amount = amount;
        }
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

    public List<String> getRoleTags() {
        return roleTags;
    }

    public void setRoleTags(List<String> roleTags) {
        this.roleTags = roleTags;
    }

    public List<String> getMechanicTags() {
        return mechanicTags;
    }

    public void setMechanicTags(List<String> mechanicTags) {
        this.mechanicTags = mechanicTags;
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

    @Deprecated
    public Cost getCost() {
        return cost;
    }

    @Deprecated
    public void setCost(Cost cost) {
        this.cost = cost;
    }

    public String getRecovery() {
        return recovery;
    }

    public void setRecovery(String recovery) {
        this.recovery = recovery;
    }

    public String getActionType() {
        return actionType;
    }

    public void setActionType(String actionType) {
        this.actionType = actionType;
    }

    public String getRange() {
        return range;
    }

    public void setRange(String range) {
        this.range = range;
    }

    public String getDuration() {
        return duration;
    }

    public void setDuration(String duration) {
        this.duration = duration;
    }

    public String getDefenseStat() {
        return defenseStat;
    }

    public void setDefenseStat(String defenseStat) {
        this.defenseStat = defenseStat;
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

    @Deprecated
    public List<String> getUpgradePath() {
        return upgradePath;
    }

    @Deprecated
    public void setUpgradePath(List<String> upgradePath) {
        this.upgradePath = upgradePath;
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
