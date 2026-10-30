package cat.dnd.cc.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Personaje del sistema de tiers y cartas. A diferencia del sistema viejo (raca/classe/transfons
 * como texto libre + nivell), un personaje SOLO puede referenciar cartas que ya existen en el
 * catálogo ({@link TierRace}, {@link TierClass}, {@link TierBackground}, {@link TierSkill},
 * {@link TierEquipment}, {@link TierFeat}) por su id. Crear una carta nueva (clase, raza, trasfondo,
 * habilidad, arma u objeto, dote) solo se puede hacer desde /cartas/*; aquí solo se seleccionan
 * cartas ya creadas para estandarizar valores y mantener el balance.
 */
public class Personatge {
    private Long id;
    private String nom;
    private String razaId;
    private String claseId;
    private String transfonsId;
    private int tier;
    private int statCon;
    private int statDes;
    private int statCar;
    private int statInt;
    private List<String> habilidadIds = new ArrayList<>();
    private List<String> equipoIds = new ArrayList<>();
    private List<String> doteIds = new ArrayList<>();
    /** Habilidades divinas elegidas (hechizos con castingStat CAR). Máximo 3 por personaje (GDD 10.10). */
    private List<String> hechizoIds = new ArrayList<>();
    /** Identidad elegida del Trasfondo (una opción por lista del characterCreation, Prompt Maestro §4b). */
    private String eleccionPersonalidad;
    private String eleccionVirtud;
    private String eleccionDefecto;
    private String eleccionObjetivo;
    private String eleccionMiedo;
    private String eleccionIdeal;
    private String eleccionVinculo;
    private String historia;

    public Personatge() {
        this.tier = 1;
        this.statCon = 1;
        this.statDes = 1;
        this.statCar = 1;
        this.statInt = 1;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getNom() {
        return nom;
    }

    public void setNom(String nom) {
        this.nom = nom;
    }

    public String getRazaId() {
        return razaId;
    }

    public void setRazaId(String razaId) {
        this.razaId = razaId;
    }

    public String getClaseId() {
        return claseId;
    }

    public void setClaseId(String claseId) {
        this.claseId = claseId;
    }

    public String getTransfonsId() {
        return transfonsId;
    }

    public void setTransfonsId(String transfonsId) {
        this.transfonsId = transfonsId;
    }

    public int getTier() {
        return tier;
    }

    public void setTier(int tier) {
        this.tier = tier;
    }

    public int getStatCon() {
        return statCon;
    }

    public void setStatCon(int statCon) {
        this.statCon = statCon;
    }

    public int getStatDes() {
        return statDes;
    }

    public void setStatDes(int statDes) {
        this.statDes = statDes;
    }

    public int getStatCar() {
        return statCar;
    }

    public void setStatCar(int statCar) {
        this.statCar = statCar;
    }

    public int getStatInt() {
        return statInt;
    }

    public void setStatInt(int statInt) {
        this.statInt = statInt;
    }

    public List<String> getHabilidadIds() {
        return habilidadIds;
    }

    public void setHabilidadIds(List<String> habilidadIds) {
        this.habilidadIds = habilidadIds;
    }

    public List<String> getEquipoIds() {
        return equipoIds;
    }

    public void setEquipoIds(List<String> equipoIds) {
        this.equipoIds = equipoIds;
    }

    public List<String> getDoteIds() {
        return doteIds;
    }

    public void setDoteIds(List<String> doteIds) {
        this.doteIds = doteIds;
    }

    public List<String> getHechizoIds() {
        return hechizoIds;
    }

    public void setHechizoIds(List<String> hechizoIds) {
        this.hechizoIds = hechizoIds;
    }

    public String getEleccionPersonalidad() { return eleccionPersonalidad; }
    public void setEleccionPersonalidad(String v) { this.eleccionPersonalidad = v; }
    public String getEleccionVirtud() { return eleccionVirtud; }
    public void setEleccionVirtud(String v) { this.eleccionVirtud = v; }
    public String getEleccionDefecto() { return eleccionDefecto; }
    public void setEleccionDefecto(String v) { this.eleccionDefecto = v; }
    public String getEleccionObjetivo() { return eleccionObjetivo; }
    public void setEleccionObjetivo(String v) { this.eleccionObjetivo = v; }
    public String getEleccionMiedo() { return eleccionMiedo; }
    public void setEleccionMiedo(String v) { this.eleccionMiedo = v; }
    public String getEleccionIdeal() { return eleccionIdeal; }
    public void setEleccionIdeal(String v) { this.eleccionIdeal = v; }
    public String getEleccionVinculo() { return eleccionVinculo; }
    public void setEleccionVinculo(String v) { this.eleccionVinculo = v; }

    public String getHistoria() {
        return historia;
    }

    public void setHistoria(String historia) {
        this.historia = historia;
    }
}
