package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Invocación del sistema de tiers y cartas (ver docs/Sistema_Cartas_Tiers.md, sección 9.10).
 * Una habilidad de invocación no describe a la criatura: solo la invoca. La criatura es su propia
 * carta independiente, con toda su información a mano.
 */
public class TierSummon {

    private String id;
    private String name;
    private String type = "summon";
    private String summonedBy;
    private int tier = 1;
    private int health;
    private List<Attack> attacks = new ArrayList<>();
    private int movement;
    private Passive passive = new Passive();
    private String duration;
    private String control = "automatica";
    private String flavorText;

    public TierSummon() {
    }

    public static class Attack {
        private String name;
        private String effect;

        public Attack() {
        }

        public Attack(String name, String effect) {
            this.name = name;
            this.effect = effect;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getEffect() {
            return effect;
        }

        public void setEffect(String effect) {
            this.effect = effect;
        }
    }

    public static class Passive {
        private String name;
        private String description;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

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

    public List<Attack> getAttacks() {
        return attacks;
    }

    public void setAttacks(List<Attack> attacks) {
        this.attacks = attacks;
    }

    public int getMovement() {
        return movement;
    }

    public void setMovement(int movement) {
        this.movement = movement;
    }

    public Passive getPassive() {
        return passive;
    }

    public void setPassive(Passive passive) {
        this.passive = passive;
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
