package cat.dnd.cc.model;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class Subraca {
    private final String nom;
    private final String descripcio;
    private final Map<String, Integer> bonificadors;
    private final List<String> competencies;
    private final List<String> coneixementAncestral;
    private final String edat;
    private final List<String> idiomes;
    private final String alineament;
    private final String mida;
    private final Map<String, Integer> velocitats;
    private final String restriccionsVelocitat;
    private final String religio;
    private final String altres;

    public Subraca(String nom, String descripcio, Map<String, Integer> bonificadors,
                   List<String> competencies, List<String> coneixementAncestral,
                   String edat, List<String> idiomes, String alineament, String mida,
                   Map<String, Integer> velocitats, String restriccionsVelocitat,
                   String religio, String altres) {
        this.nom = valueOrEmpty(nom);
        this.descripcio = valueOrEmpty(descripcio);
        this.bonificadors = Collections.unmodifiableMap(new LinkedHashMap<>(bonificadors == null ? Map.of() : bonificadors));
        this.competencies = List.copyOf(competencies == null ? List.of() : competencies);
        this.coneixementAncestral = List.copyOf(coneixementAncestral == null ? List.of() : coneixementAncestral);
        this.edat = valueOrEmpty(edat);
        this.idiomes = List.copyOf(idiomes == null ? List.of() : idiomes);
        this.alineament = valueOrEmpty(alineament);
        this.mida = valueOrEmpty(mida);
        this.velocitats = Collections.unmodifiableMap(new LinkedHashMap<>(velocitats == null ? Map.of() : velocitats));
        this.restriccionsVelocitat = valueOrEmpty(restriccionsVelocitat);
        this.religio = valueOrEmpty(religio);
        this.altres = valueOrEmpty(altres);
    }

    public String getNom() {
        return nom;
    }

    public String getDescripcio() {
        return descripcio;
    }

    public Map<String, Integer> getBonificadors() {
        return bonificadors;
    }

    public List<String> getCompetencies() {
        return competencies;
    }

    public List<String> getConeixementAncestral() {
        return coneixementAncestral;
    }

    public String getEdat() {
        return edat;
    }

    public List<String> getIdiomes() {
        return idiomes;
    }

    public String getAlineament() {
        return alineament;
    }

    public String getMida() {
        return mida;
    }

    public Map<String, Integer> getVelocitats() {
        return velocitats;
    }

    public String getRestriccionsVelocitat() {
        return restriccionsVelocitat;
    }

    public String getReligio() {
        return religio;
    }

    public String getAltres() {
        return altres;
    }

    public String getBonificadorsResum() {
        if (bonificadors.isEmpty()) {
            return "Sense bonificadors";
        }
        return bonificadors.entrySet().stream()
                .map(entry -> entry.getKey() + " +" + entry.getValue())
                .reduce((left, right) -> left + ", " + right)
                .orElse("Sense bonificadors");
    }

    public String getVelocitatsResum() {
        if (velocitats.isEmpty()) {
            return "Sense velocitat definida";
        }
        String resum = velocitats.entrySet().stream()
                .map(entry -> entry.getKey() + " " + entry.getValue())
                .reduce((left, right) -> left + ", " + right)
                .orElse("Sense velocitat definida");
        return restriccionsVelocitat.isBlank() ? resum : resum + " (" + restriccionsVelocitat + ")";
    }

    public String getCompetenciesResum() {
        return competencies.isEmpty() ? "Sense competències" : String.join(", ", competencies);
    }

    public String getIdiomesResum() {
        return idiomes.isEmpty() ? "Sense idiomes" : String.join(", ", idiomes);
    }

    private static String valueOrEmpty(String value) {
        return value == null ? "" : value;
    }
}
