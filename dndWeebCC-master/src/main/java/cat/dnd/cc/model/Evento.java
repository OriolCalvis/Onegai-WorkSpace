package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Evento — ver docs/Arquitectura_Datos_Onegai.md §5.6. Un evento es algo que
 * pasa en el mundo (no un gancho de aventura como Historia): tiene un disparador, unos
 * efectos automáticos y, opcionalmente, opciones para que el grupo reaccione.
 * <p>
 * Simplificación deliberada respecto al esquema original de los documentos: los
 * "requirements"/"consequences" de cada PlayerOption viven como texto libre
 * (requirementText / consequenceText) en vez de objetos estructurados
 * (stat/minValue/itemId, grantsItem/appliesCondition/startsQuest) — así el formulario
 * de creación es una lista de filas simples en vez de un editor anidado de tres
 * niveles. Se puede estructurar más adelante si hace falta.
 */
public class Evento {

    private String id;
    private String name;
    private String description;
    private int tier = 1;
    /** Opcional: si se rellena, el evento aparece automáticamente en el panel de esa
     * región del Mapa Mundi, igual que las historias y las aventuras. */
    private String faction;
    private Trigger trigger = new Trigger();
    /** Una línea de texto por efecto (se edita como textarea, una línea = un efecto). */
    private List<String> effects = new ArrayList<>();
    private List<PlayerOption> playerOptions = new ArrayList<>();
    /** Id de otro evento o de una historia al que este evento encadena, si procede. */
    private String continuesTo;
    private boolean oneTime;

    public Evento() { }

    public static class Trigger {
        private String type = "";
        private String condition = "";

        public String getType() { return type; }
        public void setType(String type) { this.type = type; }
        public String getCondition() { return condition; }
        public void setCondition(String condition) { this.condition = condition; }
    }

    public static class PlayerOption {
        private String id = "";
        private String text = "";
        private String requirementText = "";
        private String consequenceText = "";

        public String getId() { return id; }
        public void setId(String id) { this.id = id; }
        public String getText() { return text; }
        public void setText(String text) { this.text = text; }
        public String getRequirementText() { return requirementText; }
        public void setRequirementText(String requirementText) { this.requirementText = requirementText; }
        public String getConsequenceText() { return consequenceText; }
        public void setConsequenceText(String consequenceText) { this.consequenceText = consequenceText; }
    }

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }
    public String getDescription() { return description; }
    public void setDescription(String description) { this.description = description; }
    public int getTier() { return tier; }
    public void setTier(int tier) { this.tier = tier; }
    public String getFaction() { return faction; }
    public void setFaction(String faction) { this.faction = faction; }
    public Trigger getTrigger() { return trigger; }
    public void setTrigger(Trigger trigger) { this.trigger = trigger; }
    public List<String> getEffects() { return effects; }
    public void setEffects(List<String> effects) { this.effects = effects; }
    public List<PlayerOption> getPlayerOptions() { return playerOptions; }
    public void setPlayerOptions(List<PlayerOption> playerOptions) { this.playerOptions = playerOptions; }
    public String getContinuesTo() { return continuesTo; }
    public void setContinuesTo(String continuesTo) { this.continuesTo = continuesTo; }
    public boolean isOneTime() { return oneTime; }
    public void setOneTime(boolean oneTime) { this.oneTime = oneTime; }

    /** Nombre legible de la facción (sin guiones bajos), igual que Historia. */
    public String getFactionNombre() {
        return faction == null ? "" : faction.replace('_', ' ');
    }

    /** Texto de effects unido por saltos de línea, para precargar el <textarea> del
     * formulario de edición (Spring no sabe bindear un List<String> multilínea solo). */
    public String getEffectsTexto() {
        return effects == null ? "" : String.join("\n", effects);
    }
}
