package cat.dnd.cc.web.form;

public class TierFeatForm {

    private String id;
    private String name;
    private int tier = 1;
    private String rarity = "common";
    private String classTags;
    private int reqCon;
    private int reqDes;
    private int reqCar;
    private int reqInt;
    private String requiredTags;
    private String incompatibleTags;
    private String grantedTags;
    private String effectDescription;
    private String effectScaling = "none";
    private String limitations;
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

    public String getRarity() {
        return rarity;
    }

    public void setRarity(String rarity) {
        this.rarity = rarity;
    }

    public String getClassTags() {
        return classTags;
    }

    public void setClassTags(String classTags) {
        this.classTags = classTags;
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

    public String getRequiredTags() {
        return requiredTags;
    }

    public void setRequiredTags(String requiredTags) {
        this.requiredTags = requiredTags;
    }

    public String getIncompatibleTags() {
        return incompatibleTags;
    }

    public void setIncompatibleTags(String incompatibleTags) {
        this.incompatibleTags = incompatibleTags;
    }

    public String getGrantedTags() {
        return grantedTags;
    }

    public void setGrantedTags(String grantedTags) {
        this.grantedTags = grantedTags;
    }

    public String getEffectDescription() {
        return effectDescription;
    }

    public void setEffectDescription(String effectDescription) {
        this.effectDescription = effectDescription;
    }

    public String getEffectScaling() {
        return effectScaling;
    }

    public void setEffectScaling(String effectScaling) {
        this.effectScaling = effectScaling;
    }

    public String getLimitations() {
        return limitations;
    }

    public void setLimitations(String limitations) {
        this.limitations = limitations;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
