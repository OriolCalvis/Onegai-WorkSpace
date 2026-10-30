package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Carta de Raza del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.2 / 15).
 */
public class TierRace {

    private String id;
    private String name;
    private String type = "race";
    private int tier = 1;
    private Map<String, Integer> statBonuses = new LinkedHashMap<>();
    private Trait passiveTrait = new Trait();
    private Trait activeTrait = new Trait();
    private List<String> affinities = new ArrayList<>();
    private List<String> limitations = new ArrayList<>();
    private List<String> narrativeTags = new ArrayList<>();
    private List<String> unlocks = new ArrayList<>();
    private String flavorText;

    public TierRace() {
    }

    public static class Trait {
        private String name;
        private String description;

        public Trait() {
        }

        public Trait(String name, String description) {
            this.name = name;
            this.description = description;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getDescription() {
            return description;
        }

        public void setDescription(String description) {
            this.description = description;
        }
    }

    // Conveniencia para los cuatro stats del sistema (CON/DES/INT/CAR).

    public int getStatBonusCon() {
        return statBonuses.getOrDefault("CON", 0);
    }

    public void setStatBonusCon(int valor) {
        statBonuses.put("CON", valor);
    }

    public int getStatBonusDes() {
        return statBonuses.getOrDefault("DES", 0);
    }

    public void setStatBonusDes(int valor) {
        statBonuses.put("DES", valor);
    }

    public int getStatBonusCar() {
        return statBonuses.getOrDefault("CAR", 0);
    }

    public void setStatBonusCar(int valor) {
        statBonuses.put("CAR", valor);
    }

    public int getStatBonusInt() {
        return statBonuses.getOrDefault("INT", 0);
    }

    public void setStatBonusInt(int valor) {
        statBonuses.put("INT", valor);
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

    public Map<String, Integer> getStatBonuses() {
        return statBonuses;
    }

    public void setStatBonuses(Map<String, Integer> statBonuses) {
        this.statBonuses = statBonuses;
    }

    public Trait getPassiveTrait() {
        return passiveTrait;
    }

    public void setPassiveTrait(Trait passiveTrait) {
        this.passiveTrait = passiveTrait;
    }

    public Trait getActiveTrait() {
        return activeTrait;
    }

    public void setActiveTrait(Trait activeTrait) {
        this.activeTrait = activeTrait;
    }

    public List<String> getAffinities() {
        return affinities;
    }

    public void setAffinities(List<String> affinities) {
        this.affinities = affinities;
    }

    public List<String> getLimitations() {
        return limitations;
    }

    public void setLimitations(List<String> limitations) {
        this.limitations = limitations;
    }

    public List<String> getNarrativeTags() {
        return narrativeTags;
    }

    public void setNarrativeTags(List<String> narrativeTags) {
        this.narrativeTags = narrativeTags;
    }

    public List<String> getUnlocks() {
        return unlocks;
    }

    public void setUnlocks(List<String> unlocks) {
        this.unlocks = unlocks;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
