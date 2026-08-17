package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Rasgo Especial del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.9).
 * Otorgada por raza, trasfondo o hitos narrativos.
 */
public class TierSpecialTrait {

    private String id;
    private String name;
    private String type = "trait";
    private String origin;
    private int tier = 1;
    private Effect effect = new Effect();
    private List<String> limitations = new ArrayList<>();
    private String flavorText;

    public TierSpecialTrait() {
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

    public String getOrigin() {
        return origin;
    }

    public void setOrigin(String origin) {
        this.origin = origin;
    }

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public Effect getEffect() {
        return effect;
    }

    public void setEffect(Effect effect) {
        this.effect = effect;
    }

    public List<String> getLimitations() {
        return limitations;
    }

    public void setLimitations(List<String> limitations) {
        this.limitations = limitations;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
