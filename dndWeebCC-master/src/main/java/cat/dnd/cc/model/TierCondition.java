package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Condición del sistema de tiers y cartas: unifica Estado, Bendición y Maldición
 * (ver docs/Sistema_Cartas_Tiers.md, sección 7.4, y docs/Arquitectura_Datos_Onegai.md, sección 5.12).
 * Las tres son mecánicamente lo mismo — un modificador temporal —, solo cambia el origen y si es
 * positivo o negativo. Cualquier otra carta (habilidad, hechizo, objeto, monstruo) puede referenciar
 * una Condición por su id en su lista de tags requeridos/aplicados: la mecánica existe una única vez
 * y es reutilizable por cualquier clase, enemigo, jefe u objeto mágico, en vez de vivir escondida
 * dentro del texto de una carta.
 */
public class TierCondition {

    private String id;
    private String name;
    private String type = "condition";
    private String category = "neutro";
    private String source = "estado";
    private boolean stackable = false;
    private String duration = "1_turn";
    private List<Effect> effects = new ArrayList<>();
    private List<String> cureConditions = new ArrayList<>();
    private String flavorText;

    public TierCondition() {
    }

    public static class Effect {
        private String description;
        private String mechanicHook;

        public Effect() {
        }

        public Effect(String description, String mechanicHook) {
            this.description = description;
            this.mechanicHook = mechanicHook;
        }

        public String getDescription() {
            return description;
        }

        public void setDescription(String description) {
            this.description = description;
        }

        public String getMechanicHook() {
            return mechanicHook;
        }

        public void setMechanicHook(String mechanicHook) {
            this.mechanicHook = mechanicHook;
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

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    public boolean isStackable() {
        return stackable;
    }

    public void setStackable(boolean stackable) {
        this.stackable = stackable;
    }

    public String getDuration() {
        return duration;
    }

    public void setDuration(String duration) {
        this.duration = duration;
    }

    public List<Effect> getEffects() {
        return effects;
    }

    public void setEffects(List<Effect> effects) {
        this.effects = effects;
    }

    public List<String> getCureConditions() {
        return cureConditions;
    }

    public void setCureConditions(List<String> cureConditions) {
        this.cureConditions = cureConditions;
    }

    public String getFlavorText() {
        return flavorText;
    }

    public void setFlavorText(String flavorText) {
        this.flavorText = flavorText;
    }
}
