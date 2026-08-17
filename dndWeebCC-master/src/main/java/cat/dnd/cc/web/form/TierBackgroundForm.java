package cat.dnd.cc.web.form;

/**
 * Objeto de formulario para crear/editar una carta de trasfondo del sistema de tiers.
 */
public class TierBackgroundForm {

    private String id;
    private String name;
    private int tier = 1;
    private int statBonusCon;
    private int statBonusDes;
    private int statBonusCar;
    private int statBonusInt;
    private String narrativeSkills;
    private String contacts;
    private String bonusEquipment;
    private String narrativePassiveName;
    private String narrativePassiveDescription;
    private String complication;
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

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public int getStatBonusCon() {
        return statBonusCon;
    }

    public void setStatBonusCon(int statBonusCon) {
        this.statBonusCon = statBonusCon;
    }

    public int getStatBonusDes() {
        return statBonusDes;
    }

    public void setStatBonusDes(int statBonusDes) {
        this.statBonusDes = statBonusDes;
    }

    public int getStatBonusCar() {
        return statBonusCar;
    }

    public void setStatBonusCar(int statBonusCar) {
        this.statBonusCar = statBonusCar;
    }

    public int getStatBonusInt() {
        return statBonusInt;
    }

    public void setStatBonusInt(int statBonusInt) {
        this.statBonusInt = statBonusInt;
    }

    public String getNarrativeSkills() {
        return narrativeSkills;
    }

    public void setNarrativeSkills(String narrativeSkills) {
        this.narrativeSkills = narrativeSkills;
    }

    public String getContacts() {
        return contacts;
    }

    public void setContacts(String contacts) {
        this.contacts = contacts;
    }

    public String getBonusEquipment() {
        return bonusEquipment;
    }

    public void setBonusEquipment(String bonusEquipment) {
        this.bonusEquipment = bonusEquipment;
    }

    public String getNarrativePassiveName() {
        return narrativePassiveName;
    }

    public void setNarrativePassiveName(String narrativePassiveName) {
        this.narrativePassiveName = narrativePassiveName;
    }

    public String getNarrativePassiveDescription() {
        return narrativePassiveDescription;
    }

    public void setNarrativePassiveDescription(String narrativePassiveDescription) {
        this.narrativePassiveDescription = narrativePassiveDescription;
    }

    public String getComplication() {
        return complication;
    }

    public void setComplication(String complication) {
        this.complication = complication;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
