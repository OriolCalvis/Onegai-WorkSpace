package cat.dnd.cc.web.form;

public class TierSummonForm {

    private String id;
    private String name;
    private String summonedBy;
    private int tier = 1;
    private int health;
    private String attacks;
    private int movement;
    private String passiveName;
    private String passiveDescription;
    private String duration;
    private String control = "automatica";
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

    public String getSummonedBy() {
        return summonedBy;
    }

    public void setSummonedBy(String summonedBy) {
        this.summonedBy = summonedBy;
    }

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public int getHealth() {
        return health;
    }

    public void setHealth(int health) {
        this.health = health;
    }

    public String getAttacks() {
        return attacks;
    }

    public void setAttacks(String attacks) {
        this.attacks = attacks;
    }

    public int getMovement() {
        return movement;
    }

    public void setMovement(int movement) {
        this.movement = movement;
    }

    public String getPassiveName() {
        return passiveName;
    }

    public void setPassiveName(String passiveName) {
        this.passiveName = passiveName;
    }

    public String getPassiveDescription() {
        return passiveDescription;
    }

    public void setPassiveDescription(String passiveDescription) {
        this.passiveDescription = passiveDescription;
    }

    public String getDuration() {
        return duration;
    }

    public void setDuration(String duration) {
        this.duration = duration;
    }

    public String getControl() {
        return control;
    }

    public void setControl(String control) {
        this.control = control;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
