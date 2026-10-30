package cat.dnd.cc.web.form;

/**
 * Objeto de formulario para crear/editar una carta de raza del sistema de tiers.
 */
public class TierRaceForm {

    private String id;
    private String name;
    private int tier = 1;
    private int statBonusCon;
    private int statBonusDes;
    private int statBonusCar;
    private int statBonusInt;
    private String passiveTraitName;
    private String passiveTraitDescription;
    private String activeTraitName;
    private String activeTraitDescription;
    private String affinities;
    private String limitations;
    private String narrativeTags;
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

    public String getPassiveTraitName() {
        return passiveTraitName;
    }

    public void setPassiveTraitName(String passiveTraitName) {
        this.passiveTraitName = passiveTraitName;
    }

    public String getPassiveTraitDescription() {
        return passiveTraitDescription;
    }

    public void setPassiveTraitDescription(String passiveTraitDescription) {
        this.passiveTraitDescription = passiveTraitDescription;
    }

    public String getActiveTraitName() {
        return activeTraitName;
    }

    public void setActiveTraitName(String activeTraitName) {
        this.activeTraitName = activeTraitName;
    }

    public String getActiveTraitDescription() {
        return activeTraitDescription;
    }

    public void setActiveTraitDescription(String activeTraitDescription) {
        this.activeTraitDescription = activeTraitDescription;
    }

    public String getAffinities() {
        return affinities;
    }

    public void setAffinities(String affinities) {
        this.affinities = affinities;
    }

    public String getLimitations() {
        return limitations;
    }

    public void setLimitations(String limitations) {
        this.limitations = limitations;
    }

    public String getNarrativeTags() {
        return narrativeTags;
    }

    public void setNarrativeTags(String narrativeTags) {
        this.narrativeTags = narrativeTags;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
