package cat.dnd.cc.web.form;

/**
 * Objeto de formulario para crear/editar una carta de clase del sistema de tiers.
 * Las listas (equipo, tags, habilidades...) se capturan como texto separado por comas
 * para mantener el formulario simple; el controlador se encarga de partirlas.
 */
public class TierClassForm {

    private String id;
    private String name;
    private String role = "balanced";
    private int tier = 1;
    private int baseHealth = 10;
    private double healthScalingCon = 2;
    private String primaryStat = "CON";
    private String secondaryStat;
    private String primaryResource = "energia";
    private String secondaryResource;
    private String startingEquipment;
    private String startingPassives;
    private String startingSkills;
    private String startingSpells;
    private String learnableSkills;
    private String allowedEquipmentTags;
    private String restrictedTags;
    private String maxArmorWeight = "media";
    private String maxWeaponWeight = "media";
    private String specializations;
    private String description;

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

    public double getHealthScalingCon() {
        return healthScalingCon;
    }

    public void setHealthScalingCon(double healthScalingCon) {
        this.healthScalingCon = healthScalingCon;
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

    public String getStartingEquipment() {
        return startingEquipment;
    }

    public void setStartingEquipment(String startingEquipment) {
        this.startingEquipment = startingEquipment;
    }

    public String getStartingPassives() {
        return startingPassives;
    }

    public void setStartingPassives(String startingPassives) {
        this.startingPassives = startingPassives;
    }

    public String getStartingSkills() {
        return startingSkills;
    }

    public void setStartingSkills(String startingSkills) {
        this.startingSkills = startingSkills;
    }

    public String getStartingSpells() {
        return startingSpells;
    }

    public void setStartingSpells(String startingSpells) {
        this.startingSpells = startingSpells;
    }

    public String getLearnableSkills() {
        return learnableSkills;
    }

    public void setLearnableSkills(String learnableSkills) {
        this.learnableSkills = learnableSkills;
    }

    public String getAllowedEquipmentTags() {
        return allowedEquipmentTags;
    }

    public void setAllowedEquipmentTags(String allowedEquipmentTags) {
        this.allowedEquipmentTags = allowedEquipmentTags;
    }

    public String getRestrictedTags() {
        return restrictedTags;
    }

    public void setRestrictedTags(String restrictedTags) {
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

    public String getSpecializations() {
        return specializations;
    }

    public void setSpecializations(String specializations) {
        this.specializations = specializations;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }
}
