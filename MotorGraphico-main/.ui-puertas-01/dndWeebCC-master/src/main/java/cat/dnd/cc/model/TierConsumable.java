package cat.dnd.cc.model;

/**
 * Carta de Objeto Consumible del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.8).
 * Máximo 3 cartas simultáneas en la ranura de consumibles, sin importar tier (ver GDD 10).
 */
public class TierConsumable {

    private String id;
    private String name;
    private String type = "consumable";
    private int tier = 1;
    private String rarity = "common";
    private String actionType;
    private Effect effect = new Effect();
    private int uses = 1;
    private String flavorText;

    public TierConsumable() {
    }

    public static class Effect {
        private String description;

        public String getDescription() {
            return description;
        }

        public void setDescription(String description) {
            this.description = description;
        }
    }

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

    public String getRarity() {
        return rarity;
    }

    public void setRarity(String rarity) {
        this.rarity = rarity;
    }

    public String getActionType() {
        return actionType;
    }

    public void setActionType(String actionType) {
        this.actionType = actionType;
    }

    public Effect getEffect() {
        return effect;
    }

    public void setEffect(Effect effect) {
        this.effect = effect;
    }

    public int getUses() {
        return uses;
    }

    public void setUses(int uses) {
        this.uses = uses;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
