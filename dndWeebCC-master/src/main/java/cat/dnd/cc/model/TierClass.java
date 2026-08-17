package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Carta de Clase del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.1 / 14).
 * Se serializa a JSON con el mismo esquema documentado, para que los ficheros generados por el
 * editor web sean directamente compatibles con la plantilla de diseño.
 */
public class TierClass {

    private String id;
    private String name;
    private String type = "class";
    private String role;
    private int tier = 1;
    private int baseHealth;
    private Map<String, Double> healthScaling = new LinkedHashMap<>();
    private String primaryStat;
    private String secondaryStat;
    private String primaryResource;
    private String secondaryResource;
    private List<String> startingEquipment = new ArrayList<>();
    private StartingCards startingCards = new StartingCards();
    private List<String> allowedEquipmentTags = new ArrayList<>();
    private List<String> restrictedTags = new ArrayList<>();
    /** Peso máximo de armadura/arma que la clase lleva sin penalización: "ligera" | "media" | "pesada". */
    private String maxArmorWeight = "media";
    private String maxWeaponWeight = "media";
    private List<String> specializations = new ArrayList<>();
    private String description;

    public TierClass() {
    }

    /**
     * Referencias (por nombre) a las cartas pasivas/habilidades/hechizos iniciales de la clase.
     * En esta primera versión del editor son texto libre, no IDs de un catálogo de cartas aparte.
     */
    public static class StartingCards {
        private List<String> passives = new ArrayList<>();
        private List<String> skills = new ArrayList<>();
        private List<String> spells = new ArrayList<>();
        /**
         * Cartas de habilidad que la clase puede llegar a aprender más adelante (Tier 2+),
         * más allá de las iniciales de {@link #skills}. Sistema modular: la clase no "posee"
         * las habilidades, solo declara cuáles son compatibles con ella — igual que en el
         * esquema de referencia (class.startingSkills / class.learnableSkills).
         */
        private List<String> learnableSkills = new ArrayList<>();

        public List<String> getPassives() {
            return passives;
        }

        public void setPassives(List<String> passives) {
            this.passives = passives;
        }

        public List<String> getSkills() {
            return skills;
        }

        public void setSkills(List<String> skills) {
            this.skills = skills;
        }

        public List<String> getSpells() {
            return spells;
        }

        public void setSpells(List<String> spells) {
            this.spells = spells;
        }

        public List<String> getLearnableSkills() {
            return learnableSkills;
        }

        public void setLearnableSkills(List<String> learnableSkills) {
            this.learnableSkills = learnableSkills;
        }
    }

    // Conveniencia: el sistema solo escala vida con CON (sección 5 del documento de diseño).

    public double getHealthScalingCon() {
        Double valor = healthScaling.get("CON");
        return valor == null ? 0d : valor;
    }

    public void setHealthScalingCon(double con) {
        this.healthScaling.put("CON", con);
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

    public String getRole() {
        return role;
    }

    public void setRole(String role) {
        this.role = role;
    }

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public int getBaseHealth() {
        return baseHealth;
    }

    public void setBaseHealth(int baseHealth) {
        this.baseHealth = baseHealth;
    }

    public Map<String, Double> getHealthScaling() {
        return healthScaling;
    }

    public void setHealthScaling(Map<String, Double> healthScaling) {
        this.healthScaling = healthScaling;
    }

    public String getPrimaryStat() {
        return primaryStat;
    }

    public void setPrimaryStat(String primaryStat) {
        this.primaryStat = primaryStat;
    }

    public String getSecondaryStat() {
        return secondaryStat;
    }

    public void setSecondaryStat(String secondaryStat) {
        this.secondaryStat = secondaryStat;
    }

    public String getPrimaryResource() {
        return primaryResource;
    }

    public void setPrimaryResource(String primaryResource) {
        this.primaryResource = primaryResource;
    }

    public String getSecondaryResource() {
        return secondaryResource;
    }

    public void setSecondaryResource(String secondaryResource) {
        this.secondaryResource = secondaryResource;
    }

    public List<String> getStartingEquipment() {
        return startingEquipment;
    }

    public void setStartingEquipment(List<String> startingEquipment) {
        this.startingEquipment = startingEquipment;
    }

    public StartingCards getStartingCards() {
        return startingCards;
    }

    public void setStartingCards(StartingCards startingCards) {
        this.startingCards = startingCards;
    }

    public List<String> getAllowedEquipmentTags() {
        return allowedEquipmentTags;
    }

    public void setAllowedEquipmentTags(List<String> allowedEquipmentTags) {
        this.allowedEquipmentTags = allowedEquipmentTags;
    }

    public List<String> getRestrictedTags() {
        return restrictedTags;
    }

    public void setRestrictedTags(List<String> restrictedTags) {
        this.restrictedTags = restrictedTags;
    }

    public String getMaxArmorWeight() {
        return maxArmorWeight;
    }

    public void setMaxArmorWeight(String maxArmorWeight) {
        this.maxArmorWeight = maxArmorWeight;
    }

    public String getMaxWeaponWeight() {
        return maxWeaponWeight;
    }

    public void setMaxWeaponWeight(String maxWeaponWeight) {
        this.maxWeaponWeight = maxWeaponWeight;
    }

    public List<String> getSpecializations() {
        return specializations;
    }

    public void setSpecializations(List<String> specializations) {
        this.specializations = specializations;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }
}
