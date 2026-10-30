package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Carta de Trasfondo del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.3 / 16).
 * Además de lo documentado, incluye statBonuses (CON/DES/INT/CAR) porque los trasfondos originales
 * del proyecto (uno por mes de nacimiento) ya otorgaban bonificadores numéricos y queremos conservarlos
 * al migrarlos al sistema de cuatro stats.
 */
public class TierBackground {

    private String id;
    private String name;
    private String type = "background";
    private int tier = 1;
    private Map<String, Integer> statBonuses = new LinkedHashMap<>();
    private List<String> narrativeSkills = new ArrayList<>();
    private List<String> contacts = new ArrayList<>();
    private List<String> bonusEquipment = new ArrayList<>();
    private TierRace.Trait narrativePassive = new TierRace.Trait();
    private String complication;
    private List<String> unlocks = new ArrayList<>();
    private String flavorText;

    // ==== Esquema canónico de Trasfondos (Prompt Maestro §4b, decisión 2026-07) ====
    // Narrativa pura: descripción, lore, alineamiento (arquetipo) y las 7 listas de
    // characterCreation entre las que el jugador ELIGE una opción de cada al crear
    // su personaje. Los campos antiguos de arriba quedan como legado (los .md viejos).
    private String description;
    private Lore lore = new Lore();
    private CharacterCreation characterCreation = new CharacterCreation();
    private Alignment alignment = new Alignment();
    private List<String> tags = new ArrayList<>();

    public TierBackground() {
    }

    /** Una opción elegible de cualquiera de las 7 listas (personalidad usa title; el resto, name). */
    public static class Opcion {
        private String id;
        private String title;
        private String name;
        private String description;

        public String getId() { return id; }
        public void setId(String id) { this.id = id; }
        public String getTitle() { return title; }
        public void setTitle(String title) { this.title = title; }
        public String getName() { return name; }
        public void setName(String name) { this.name = name; }
        public String getDescription() { return description; }
        public void setDescription(String description) { this.description = description; }
        /** Título mostrable con independencia de si la lista usa title o name. */
        public String getEtiqueta() { return title != null ? title : name; }
    }

    public static class Lore {
        private String origin;
        private String culture;
        private String environment;
        private List<String> importantEvents = new ArrayList<>();
        private List<String> deities = new ArrayList<>();
        private List<String> symbols = new ArrayList<>();
        private List<String> traditions = new ArrayList<>();

        public String getOrigin() { return origin; }
        public void setOrigin(String origin) { this.origin = origin; }
        public String getCulture() { return culture; }
        public void setCulture(String culture) { this.culture = culture; }
        public String getEnvironment() { return environment; }
        public void setEnvironment(String environment) { this.environment = environment; }
        public List<String> getImportantEvents() { return importantEvents; }
        public void setImportantEvents(List<String> v) { this.importantEvents = v; }
        public List<String> getDeities() { return deities; }
        public void setDeities(List<String> v) { this.deities = v; }
        public List<String> getSymbols() { return symbols; }
        public void setSymbols(List<String> v) { this.symbols = v; }
        public List<String> getTraditions() { return traditions; }
        public void setTraditions(List<String> v) { this.traditions = v; }
    }

    public static class CharacterCreation {
        private int choosePersonality = 1;
        private List<Opcion> personalities = new ArrayList<>();
        private int chooseVirtue = 1;
        private List<Opcion> virtues = new ArrayList<>();
        private int chooseFlaw = 1;
        private List<Opcion> flaws = new ArrayList<>();
        private int chooseGoal = 1;
        private List<Opcion> goals = new ArrayList<>();
        private int chooseFear = 1;
        private List<Opcion> fears = new ArrayList<>();
        private int chooseIdeal = 1;
        private List<Opcion> ideals = new ArrayList<>();
        private int chooseBond = 1;
        private List<Opcion> bonds = new ArrayList<>();

        public int getChoosePersonality() { return choosePersonality; }
        public void setChoosePersonality(int v) { this.choosePersonality = v; }
        public List<Opcion> getPersonalities() { return personalities; }
        public void setPersonalities(List<Opcion> v) { this.personalities = v; }
        public int getChooseVirtue() { return chooseVirtue; }
        public void setChooseVirtue(int v) { this.chooseVirtue = v; }
        public List<Opcion> getVirtues() { return virtues; }
        public void setVirtues(List<Opcion> v) { this.virtues = v; }
        public int getChooseFlaw() { return chooseFlaw; }
        public void setChooseFlaw(int v) { this.chooseFlaw = v; }
        public List<Opcion> getFlaws() { return flaws; }
        public void setFlaws(List<Opcion> v) { this.flaws = v; }
        public int getChooseGoal() { return chooseGoal; }
        public void setChooseGoal(int v) { this.chooseGoal = v; }
        public List<Opcion> getGoals() { return goals; }
        public void setGoals(List<Opcion> v) { this.goals = v; }
        public int getChooseFear() { return chooseFear; }
        public void setChooseFear(int v) { this.chooseFear = v; }
        public List<Opcion> getFears() { return fears; }
        public void setFears(List<Opcion> v) { this.fears = v; }
        public int getChooseIdeal() { return chooseIdeal; }
        public void setChooseIdeal(int v) { this.chooseIdeal = v; }
        public List<Opcion> getIdeals() { return ideals; }
        public void setIdeals(List<Opcion> v) { this.ideals = v; }
        public int getChooseBond() { return chooseBond; }
        public void setChooseBond(int v) { this.chooseBond = v; }
        public List<Opcion> getBonds() { return bonds; }
        public void setBonds(List<Opcion> v) { this.bonds = v; }
    }

    public static class Alignment {
        private String title;
        private String description;

        public String getTitle() { return title; }
        public void setTitle(String title) { this.title = title; }
        public String getDescription() { return description; }
        public void setDescription(String description) { this.description = description; }
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

    public List<String> getNarrativeSkills() {
        return narrativeSkills;
    }

    public void setNarrativeSkills(List<String> narrativeSkills) {
        this.narrativeSkills = narrativeSkills;
    }

    public List<String> getContacts() {
        return contacts;
    }

    public void setContacts(List<String> contacts) {
        this.contacts = contacts;
    }

    public List<String> getBonusEquipment() {
        return bonusEquipment;
    }

    public void setBonusEquipment(List<String> bonusEquipment) {
        this.bonusEquipment = bonusEquipment;
    }

    public TierRace.Trait getNarrativePassive() {
        return narrativePassive;
    }

    public void setNarrativePassive(TierRace.Trait narrativePassive) {
        this.narrativePassive = narrativePassive;
    }

    public String getComplication() {
        return complication;
    }

    public void setComplication(String complication) {
        this.complication = complication;
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

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public Lore getLore() {
        return lore;
    }

    public void setLore(Lore lore) {
        this.lore = lore;
    }

    public CharacterCreation getCharacterCreation() {
        return characterCreation;
    }

    public void setCharacterCreation(CharacterCreation characterCreation) {
        this.characterCreation = characterCreation;
    }

    public Alignment getAlignment() {
        return alignment;
    }

    public void setAlignment(Alignment alignment) {
        this.alignment = alignment;
    }

    public List<String> getTags() {
        return tags;
    }

    public void setTags(List<String> tags) {
        this.tags = tags;
    }
}
