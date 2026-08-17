package cat.dnd.cc.web.form;

public class TierSkillForm {

    private String id;
    private String name;
    private int tier = 1;
    private String rarity = "common";
    private String classTags;
    private String roleTags;
    private String mechanicTags;
    private int reqCon;
    private int reqDes;
    private int reqCar;
    private int reqInt;
    private String requiredTags;
    private String incompatibleTags;
    /** @deprecated ver TierSkill.cost */
    @Deprecated
    private String costResource = "none";
    /** @deprecated ver TierSkill.cost */
    @Deprecated
    private int costAmount;
    private String recovery = "activa";
    private String actionType = "accion";
    private String range = "melee";
    private String duration = "instant";
    private String defenseStat;
    private String effectDescription;
    private String effectScaling = "none";
    private String limitations;
    /** @deprecated ver TierSkill.upgradePath */
    @Deprecated
    private String upgradePath;
    private String evolvesInto;
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

    public String getRoleTags() {
        return roleTags;
    }

    public void setRoleTags(String roleTags) {
        this.roleTags = roleTags;
    }

    public String getMechanicTags() {
        return mechanicTags;
    }

    public void setMechanicTags(String mechanicTags) {
        this.mechanicTags = mechanicTags;
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

    @Deprecated
    public String getCostResource() {
        return costResource;
    }

    @Deprecated
    public void setCostResource(String costResource) {
        this.costResource = costResource;
    }

    @Deprecated
    public int getCostAmount() {
        return costAmount;
    }

    @Deprecated
    public void setCostAmount(int costAmount) {
        this.costAmount = costAmount;
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

    @Deprecated
    public String getUpgradePath() {
        return upgradePath;
    }

    @Deprecated
    public void setUpgradePath(String upgradePath) {
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
