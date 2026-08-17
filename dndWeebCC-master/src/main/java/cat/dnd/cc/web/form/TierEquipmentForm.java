package cat.dnd.cc.web.form;

public class TierEquipmentForm {

    private String id;
    private String name;
    private String slot = "arma_principal";
    private int tier = 1;
    private String rarity = "common";
    private String weightCategory = "ligera";
    private int bonusCon;
    private int bonusDes;
    private int bonusCar;
    private int bonusInt;
    private int bonusHealth;
    private int bonusArmor;
    private int penaltyCon;
    private int penaltyDes;
    private int penaltyCar;
    private int penaltyInt;
    private String grantedTags;
    private String linkedSkill;
    private int reqCon;
    private int reqDes;
    private int reqCar;
    private int reqInt;
    private String restrictions;
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

    public int getBonusCon() {
        return bonusCon;
    }

    public void setBonusCon(int bonusCon) {
        this.bonusCon = bonusCon;
    }

    public int getBonusDes() {
        return bonusDes;
    }

    public void setBonusDes(int bonusDes) {
        this.bonusDes = bonusDes;
    }

    public int getBonusCar() {
        return bonusCar;
    }

    public void setBonusCar(int bonusCar) {
        this.bonusCar = bonusCar;
    }

    public int getBonusInt() {
        return bonusInt;
    }

    public void setBonusInt(int bonusInt) {
        this.bonusInt = bonusInt;
    }

    public int getBonusHealth() {
        return bonusHealth;
    }

    public void setBonusHealth(int bonusHealth) {
        this.bonusHealth = bonusHealth;
    }

    public int getBonusArmor() {
        return bonusArmor;
    }

    public void setBonusArmor(int bonusArmor) {
        this.bonusArmor = bonusArmor;
    }

    public int getPenaltyCon() {
        return penaltyCon;
    }

    public void setPenaltyCon(int penaltyCon) {
        this.penaltyCon = penaltyCon;
    }

    public int getPenaltyDes() {
        return penaltyDes;
    }

    public void setPenaltyDes(int penaltyDes) {
        this.penaltyDes = penaltyDes;
    }

    public int getPenaltyCar() {
        return penaltyCar;
    }

    public void setPenaltyCar(int penaltyCar) {
        this.penaltyCar = penaltyCar;
    }

    public int getPenaltyInt() {
        return penaltyInt;
    }

    public void setPenaltyInt(int penaltyInt) {
        this.penaltyInt = penaltyInt;
    }

    public String getGrantedTags() {
        return grantedTags;
    }

    public void setGrantedTags(String grantedTags) {
        this.grantedTags = grantedTags;
    }

    public String getLinkedSkill() {
        return linkedSkill;
    }

    public void setLinkedSkill(String linkedSkill) {
        this.linkedSkill = linkedSkill;
    }

    public int getReqCon() {
        return reqCon;
    }

    public void setReqCon(int reqCon) {
        this.reqCon = reqCon;
    }

    public int getReqDes() {
        return reqDes;
    }

    public void setReqDes(int reqDes) {
        this.reqDes = reqDes;
    }

    public int getReqCar() {
        return reqCar;
    }

    public void setReqCar(int reqCar) {
        this.reqCar = reqCar;
    }

    public int getReqInt() {
        return reqInt;
    }

    public void setReqInt(int reqInt) {
        this.reqInt = reqInt;
    }

    public String getRestrictions() {
        return restrictions;
    }

    public void setRestrictions(String restrictions) {
        this.restrictions = restrictions;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
