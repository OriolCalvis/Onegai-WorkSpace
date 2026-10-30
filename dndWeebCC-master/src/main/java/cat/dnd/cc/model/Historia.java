package cat.dnd.cc.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.ArrayList;
import java.util.List;

/**
 * Carta de Historia (gancho de aventura) — ver Plantilla_Prompt_Contenido.md §10.
 * Cada historia nombra a un antagonista REAL del bestiario, ocurre en un sitio,
 * premia con una tabla de loot real y pertenece a una trama madre de 5 escalones.
 */
public class Historia {

    private String id;
    private String title;
    private String type = "story";
    private String hook;
    private String location;
    private String antagonist;
    private List<Integer> tierRange = new ArrayList<>();
    private String reward;
    private String decision;
    private String complication;
    private List<String> months = new ArrayList<>();
    private String faction;
    private Chain chain = new Chain();
    private String flavorText;
    /** Qué cambia en el mundo si el grupo ignora, falla o llega tarde a este escalón —
     * opcional: no todas las historias del catálogo lo tienen todavía. */
    private String consequence;
    /** Escenas de la historia (wireframe t13): el esquema §10 plano (1 gancho + 1
     * antagonista + 1 recompensa en un único lugar implícito) se extiende con un array
     * de escenas, cada una con su lugar, su reparto propio y sus acciones→consecuencias.
     * Opcional: las historias generadas por script no lo tienen todavía. */
    private List<Escena> escenas = new ArrayList<>();

    /** Una escena: lugar + momento + contenido propio + qué pasa según lo que decidan
     * los jugadores ahí (visualizada como línea de tiempo — wireframe 13a). */
    public static class Escena {
        private String lugar;
        private String momento;
        private List<String> enemigos = new ArrayList<>();
        private List<String> aliados = new ArrayList<>();
        private List<String> eventos = new ArrayList<>();
        private List<String> trampas = new ArrayList<>();
        private List<String> loot = new ArrayList<>();
        private List<Accion> acciones = new ArrayList<>();

        public String getLugar() { return lugar; }
        public void setLugar(String lugar) { this.lugar = lugar; }
        public String getMomento() { return momento; }
        public void setMomento(String momento) { this.momento = momento; }
        public List<String> getEnemigos() { return enemigos; }
        public void setEnemigos(List<String> enemigos) { this.enemigos = enemigos; }
        public List<String> getAliados() { return aliados; }
        public void setAliados(List<String> aliados) { this.aliados = aliados; }
        public List<String> getEventos() { return eventos; }
        public void setEventos(List<String> eventos) { this.eventos = eventos; }
        public List<String> getTrampas() { return trampas; }
        public void setTrampas(List<String> trampas) { this.trampas = trampas; }
        public List<String> getLoot() { return loot; }
        public void setLoot(List<String> loot) { this.loot = loot; }
        public List<Accion> getAcciones() { return acciones; }
        public void setAcciones(List<Accion> acciones) { this.acciones = acciones; }
    }

    /** Acción posible del grupo en una escena → su consecuencia, y (opcional) a qué
     * escena lleva. */
    public static class Accion {
        private String accion;
        private String consecuencia;
        @JsonProperty("lleva_a")
        private String llevaA;

        public String getAccion() { return accion; }
        public void setAccion(String accion) { this.accion = accion; }
        public String getConsecuencia() { return consecuencia; }
        public void setConsecuencia(String consecuencia) { this.consecuencia = consecuencia; }
        public String getLlevaA() { return llevaA; }
        public void setLlevaA(String llevaA) { this.llevaA = llevaA; }
    }

    public static class Chain {
        private String trama;
        private int step = 1;
        private int of = 5;

        public String getTrama() { return trama; }
        public void setTrama(String trama) { this.trama = trama; }
        public int getStep() { return step; }
        public void setStep(int step) { this.step = step; }
        public int getOf() { return of; }
        public void setOf(int of) { this.of = of; }
    }

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }
    public String getTitle() { return title; }
    public void setTitle(String title) { this.title = title; }
    public String getType() { return type; }
    public void setType(String type) { this.type = type; }
    public String getHook() { return hook; }
    public void setHook(String hook) { this.hook = hook; }
    public String getLocation() { return location; }
    public void setLocation(String location) { this.location = location; }
    public String getAntagonist() { return antagonist; }
    public void setAntagonist(String antagonist) { this.antagonist = antagonist; }
    public List<Integer> getTierRange() { return tierRange; }
    public void setTierRange(List<Integer> tierRange) { this.tierRange = tierRange; }
    public String getReward() { return reward; }
    public void setReward(String reward) { this.reward = reward; }
    public String getDecision() { return decision; }
    public void setDecision(String decision) { this.decision = decision; }
    public String getComplication() { return complication; }
    public void setComplication(String complication) { this.complication = complication; }
    public List<String> getMonths() { return months; }
    public void setMonths(List<String> months) { this.months = months; }
    public String getFaction() { return faction; }
    public void setFaction(String faction) { this.faction = faction; }
    public Chain getChain() { return chain; }
    public void setChain(Chain chain) { this.chain = chain; }
    public String getFlavorText() { return flavorText; }
    public void setFlavorText(String flavorText) { this.flavorText = flavorText; }
    public String getConsequence() { return consequence; }
    public void setConsequence(String consequence) { this.consequence = consequence; }
    public List<Escena> getEscenas() { return escenas; }
    public void setEscenas(List<Escena> escenas) { this.escenas = escenas; }

    /** Tier mínimo de la historia (primer valor del rango), para filtros. */
    @JsonIgnore
    public int getTierMin() {
        return tierRange.isEmpty() ? 1 : tierRange.get(0);
    }

    /** Todos los tiers que cubre esta historia (expande [min,max] a la lista completa
     * de enteros), para filtros como el del Mapa Mundi — no solo el mínimo. */
    @JsonIgnore
    public List<Integer> getTierRangeExpandido() {
        if (tierRange == null || tierRange.isEmpty()) return new ArrayList<>();
        int min = tierRange.get(0);
        int max = tierRange.get(tierRange.size() - 1);
        if (min > max) { int tmp = min; min = max; max = tmp; }
        List<Integer> expandido = new ArrayList<>();
        for (int t = min; t <= max; t++) expandido.add(t);
        return expandido;
    }

    /** Nombre legible de la facción (sin guiones bajos). */
    @JsonIgnore
    public String getFactionNombre() {
        return faction == null ? "" : faction.replace('_', ' ');
    }
}
