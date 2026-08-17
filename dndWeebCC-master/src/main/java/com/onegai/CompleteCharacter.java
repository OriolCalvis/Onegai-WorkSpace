package com.onegai;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import java.text.Normalizer;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Complete character sheet model for the legacy {@code com.onegai} API.
 *
 * <p>The original snippet used {@code org.json} and external sheet classes that are not part of
 * this repository. This implementation keeps the same public surface where practical, but uses
 * Jackson (the JSON library already present in the project) and small nested value objects so the
 * class is self-contained.</p>
 */
public class CompleteCharacter {
    private static final ObjectMapper MAPPER = new ObjectMapper();
    private static final Pattern NAMED_JSON_PATTERN = Pattern.compile("(.+):\\s*(\\{.*})");

    private String name = "";
    private String charName = "";
    private String race = "";
    private String background = "";
    private String alignment = "";
    private String prof = "";
    private List<String> weaponProficiencies = new ArrayList<>();
    private List<String> armorProficiencies = new ArrayList<>();
    public List<String> aEspecialesProficiencies = new ArrayList<>();
    public List<String> mecanicasProficiencies = new ArrayList<>();
    private Map<Integer, List<String>> hechizosProficiencies = new HashMap<>();
    private final Map<String, Integer> habilidadesEspeciales = new LinkedHashMap<>();
    private final Map<Integer, List<String>> habilidadesOcultas = new HashMap<>();
    private final List<String> GOD = new ArrayList<>();
    private String personalitat = "";
    private String virtuds = "";
    private String defectes = "";
    private String proposits = "";

    public final Atributo FUE = new Atributo("FUE", 0, 0);
    public final Atributo DES = new Atributo("DES", 0, 0);
    public final Atributo CON = new Atributo("CON", 0, 0);
    public final Atributo INT = new Atributo("INT", 0, 0);
    public final Atributo SAB = new Atributo("SAB", 0, 0);
    public final Atributo CAR = new Atributo("CAR", 0, 0);

    public final Habilitat atletismo = new Habilitat("Atl", "FUE", 0);
    public final Habilitat acrobacias = new Habilitat("Acro", "DES", 0);
    public final Habilitat juego_de_Manos = new Habilitat("JdM", "DES", 0);
    public final Habilitat sigilo = new Habilitat("Sig", "DES", 0);
    public final Habilitat arcan = new Habilitat("Arc", "INT", 0);
    public final Habilitat historia = new Habilitat("His", "INT", 0);
    public final Habilitat investigacion = new Habilitat("Inve", "INT", 0);
    public final Habilitat naturaleza = new Habilitat("Nat", "INT", 0);
    public final Habilitat religion = new Habilitat("Reg", "INT", 0);
    public final Habilitat trato_de_animales = new Habilitat("TdA", "SAB", 0);
    public final Habilitat medicina = new Habilitat("Med", "SAB", 0);
    public final Habilitat percepcion = new Habilitat("Perc", "SAB", 0);
    public final Habilitat perspicacia = new Habilitat("Pers", "SAB", 0);
    public final Habilitat supervivencia = new Habilitat("Sup", "SAB", 0);
    public final Habilitat enganio = new Habilitat("Eng", "CAR", 0);
    public final Habilitat intimidacion = new Habilitat("Inti", "CAR", 0);
    public final Habilitat interpretacion = new Habilitat("Inte", "CAR", 0);
    public final Habilitat persuasion = new Habilitat("Persua", "CAR", 0);

    private int percepcionPasiva;
    private int bonCompetencia;
    private int ca;
    private int ini;
    private int vel;
    private int pdGT;
    private int pdGA;
    private int nDdG;
    private int ddG;
    private int sdMA;
    private int sdMF;
    private int fsdM;
    private int asdM;
    private int nivel;
    private int exp;
    private String inspiracion = "0";
    private String atributo = "";
    private final List<InventoryItem> equipoReal = new ArrayList<>();

    public void setInfo(String playerName, String characterName, String characterRace,
                        String characterBackground, String characterAlignment,
                        String characterClass, int characterLevel) {
        name = valueOrEmpty(playerName);
        charName = valueOrEmpty(characterName);
        race = valueOrEmpty(characterRace);
        background = valueOrEmpty(characterBackground);
        alignment = valueOrEmpty(characterAlignment);
        prof = valueOrEmpty(characterClass);
        nivel = characterLevel;
    }

    public void setTraits(String personalitat, String virtuds, String defectes, String proposits) {
        this.personalitat = valueOrEmpty(personalitat);
        this.virtuds = valueOrEmpty(virtuds);
        this.defectes = valueOrEmpty(defectes);
        this.proposits = valueOrEmpty(proposits);
    }

    public String getPersonalitat() { return personalitat; }
    public String getVirtuds() { return virtuds; }
    public String getDefectes() { return defectes; }
    public String getProposits() { return proposits; }
    public String getName() { return name; }
    public String getRace() { return race; }
    public String getBackground() { return background; }
    public String getAlignment() { return alignment; }
    public String getLvl() { return String.valueOf(nivel); }
    public int getLvli() { return nivel; }
    public String getCharName() { return charName; }
    public String getExp() { return String.valueOf(exp); }
    public String getInsp() { return inspiracion; }
    public String getProfBonus() { return String.valueOf(bonCompetencia); }
    public String getAC() { return String.valueOf(ca); }
    public String getInitia() { return String.valueOf(ini); }
    public String getSpeed() { return String.valueOf(vel); }
    public String getProf() { return prof; }
    public String getHp() { return String.valueOf(pdGA); }
    public int getHpi() { return pdGT; }
    public String getCurrentHP() { return String.valueOf(pdGA); }
    public int getHitDiceValue() { return ddG; }
    public int getnHitDiceValue() { return nDdG; }
    public String getCastingStat() { return atributo; }

    public void validarPersonaje() {
        System.out.println("=== VALIDACIÓN DEL PERSONAJE ===");
        if (vel <= 0) {
            System.out.println("❌ Velocidad no puede ser 0");
            vel = 30;
        }
        if (pdGT <= 0) {
            System.out.println("❌ Puntos de golpe no pueden ser 0");
            pdGT = Math.max(1, 8 + CON.getMod());
            pdGA = pdGT;
        }

        List<String> competencias = new ArrayList<>();
        competencias.addAll(weaponProficiencies);
        competencias.addAll(armorProficiencies);
        cat.dnd.cc.service.SistemaHabilidades sistema = new SistemaHabilidades(
                FUE.getAt(), DES.getAt(), CON.getAt(), INT.getAt(), SAB.getAt(), CAR.getAt(), nivel, competencias)
                .setPuedeLanzarHechizos(isSpellcaster())
                .addEtiqueta("CHRONOS")
                .addEtiqueta("EROS")
                .conocerPacto("Pacto del Cometa");

        sistema.getHabilidadesDisponibles()
                .forEach(habilidad -> System.out.println(habilidad.id + " -> " + habilidad.nombre));
    }

    public ObjectNode toJsonCompleto() {
        ObjectNode json = MAPPER.createObjectNode();
        ObjectNode meta = json.putObject("meta");
        meta.put("schemaVersion", "1.0.0");
        meta.put("system", "DnD5e-custom");
        meta.put("lastUpdated", LocalDate.now().toString());

        json.put("characterName", charName);
        json.put("playerName", name);
        json.put("characterClass", prof);
        json.put("level", nivel);
        json.put("experiencePoints", exp);
        json.put("proficiencyBonus", bonCompetencia);
        json.put("inspiration", inspiracion);
        json.put("alignment", alignment);
        json.put("race", race);
        json.put("background", background);

        ObjectNode attributes = json.putObject("attributes");
        attributes.put("strength", FUE.getAt());
        attributes.put("dexterity", DES.getAt());
        attributes.put("constitution", CON.getAt());
        attributes.put("intelligence", INT.getAt());
        attributes.put("wisdom", SAB.getAt());
        attributes.put("charisma", CAR.getAt());

        ObjectNode modifiers = json.putObject("modifiers");
        modifiers.put("strength", FUE.getMod());
        modifiers.put("dexterity", DES.getMod());
        modifiers.put("constitution", CON.getMod());
        modifiers.put("intelligence", INT.getMod());
        modifiers.put("wisdom", SAB.getMod());
        modifiers.put("charisma", CAR.getMod());

        json.put("armorClass", ca);
        json.put("initiative", ini);
        json.put("speed", vel);
        json.put("hitPoints", pdGT);
        json.put("currentHitPoints", pdGA);
        json.put("hitDice", getHitDice());

        ObjectNode hitDiceDetail = json.putObject("hitDiceDetail");
        hitDiceDetail.put("type", "d" + ddG);
        hitDiceDetail.put("total", nivel);
        hitDiceDetail.put("remaining", nDdG);

        ObjectNode savingThrows = json.putObject("savingThrows");
        savingThrows.set("strength", createSavingThrowObject(FUE));
        savingThrows.set("dexterity", createSavingThrowObject(DES));
        savingThrows.set("constitution", createSavingThrowObject(CON));
        savingThrows.set("intelligence", createSavingThrowObject(INT));
        savingThrows.set("wisdom", createSavingThrowObject(SAB));
        savingThrows.set("charisma", createSavingThrowObject(CAR));

        ObjectNode skills = json.putObject("skills");
        skills.set("acrobatics", createSkillObject(acrobacias));
        skills.set("animalHandling", createSkillObject(trato_de_animales));
        skills.set("arcana", createSkillObject(arcan));
        skills.set("athletics", createSkillObject(atletismo));
        skills.set("deception", createSkillObject(enganio));
        skills.set("history", createSkillObject(historia));
        skills.set("insight", createSkillObject(perspicacia));
        skills.set("intimidation", createSkillObject(intimidacion));
        skills.set("investigation", createSkillObject(investigacion));
        skills.set("medicine", createSkillObject(medicina));
        skills.set("nature", createSkillObject(naturaleza));
        skills.set("perception", createSkillObject(percepcion));
        skills.set("performance", createSkillObject(interpretacion));
        skills.set("persuasion", createSkillObject(persuasion));
        skills.set("religion", createSkillObject(religion));
        skills.set("sleightOfHand", createSkillObject(juego_de_Manos));
        skills.set("stealth", createSkillObject(sigilo));
        skills.set("survival", createSkillObject(supervivencia));

        ObjectNode passives = json.putObject("passives");
        passives.put("passivePerception", percepcionPasiva);
        passives.put("passiveInsight", 10 + perspicacia.getMod());
        passives.put("passiveInvestigation", 10 + investigacion.getMod());

        ObjectNode proficiencies = json.putObject("proficiencies");
        addStringArray(proficiencies, "armors", armorProficiencies);
        addStringArray(proficiencies, "weapons", weaponProficiencies);
        addStringArray(proficiencies, "tools", Collections.emptyList());
        addStringArray(proficiencies, "languages", Collections.emptyList());

        json.put("equipment", listOfWeaponandArmor());
        ArrayNode inventory = json.putArray("inventory");
        for (InventoryItem item : getInventory()) {
            ObjectNode itemJson = inventory.addObject();
            itemJson.put("name", item.getName());
            itemJson.put("type", item.getTipo());
            itemJson.put("quantity", item.getQuantity());
            itemJson.put("weight", item.getWeight());
            itemJson.put("value", item.getValue());
        }

        json.put("personalityTraits", personalitat);
        json.put("ideals", virtuds);
        json.put("flaws", defectes);
        json.put("bonds", proposits);
        addStringArray(json, "specialAbilities", aEspecialesProficiencies);
        addStringArray(json, "mechanics", mecanicasProficiencies);

        ObjectNode spells = json.putObject("spells");
        hechizosProficiencies.keySet().stream().sorted().forEach(level -> addStringArray(spells, String.valueOf(level), hechizosProficiencies.get(level)));

        if (isSpellcaster()) {
            ObjectNode spellcasting = json.putObject("spellcasting");
            spellcasting.put("isCaster", true);
            spellcasting.put("castingStat", atributo);
            spellcasting.put("spellSaveDC", getSpellSaveDC());
            spellcasting.put("spellAttackBonus", getSpellAttackBonus());
            ObjectNode spellSlots = spellcasting.putObject("spellSlots");
            getSpellSlots().entrySet().stream()
                    .sorted(Map.Entry.comparingByKey((a, b) -> Integer.compare(Integer.parseInt(a), Integer.parseInt(b))))
                    .forEach(entry -> spellSlots.put(entry.getKey(), entry.getValue()));
        }

        ObjectNode deathSaves = json.putObject("deathSaves");
        deathSaves.put("successes", sdMA);
        deathSaves.put("failures", sdMF);
        return json;
    }

    private ObjectNode createSkillObject(Habilitat skill) {
        ObjectNode skillObj = MAPPER.createObjectNode();
        skillObj.put("value", skill.getMod());
        skillObj.put("proficient", "X".equals(skill.getComp()));
        skillObj.put("expertise", false);
        return skillObj;
    }

    private ObjectNode createSavingThrowObject(Atributo attribute) {
        ObjectNode saveObj = MAPPER.createObjectNode();
        boolean proficient = "X".equals(attribute.getComp());
        saveObj.put("value", attribute.getMod() + (proficient ? bonCompetencia : 0));
        saveObj.put("proficient", proficient);
        return saveObj;
    }

    public void anadirEquipo(InventoryItem item) {
        if (item == null) {
            return;
        }
        equipoReal.add(item);
        if ("armor".equals(item.getTipo()) || "shield".equals(item.getTipo())) {
            actualizarCA();
        }
    }

    /** Legacy spelling retained for callers that use the original method name. */
    public void añadirEquipo(InventoryItem item) {
        anadirEquipo(item);
    }

    private void actualizarCA() {
        int nuevaCA = 10 + DES.getMod();
        for (InventoryItem item : equipoReal) {
            if ("armor".equals(item.getTipo())) {
                if (item.getName().contains("Cuero")) {
                    nuevaCA = 11 + DES.getMod();
                } else if (item.getName().contains("Malla")) {
                    nuevaCA = 12 + Math.min(DES.getMod(), 2);
                }
            } else if ("shield".equals(item.getTipo())) {
                nuevaCA += 2;
            }
        }
        ca = nuevaCA;
    }

    public String getMod(String sts) { return String.valueOf(getModi(sts)); }

    public int getModi(String sts) {
        return switch (normalizeStatKey(sts)) {
            case "FUE" -> FUE.getMod();
            case "DES" -> DES.getMod();
            case "CON" -> CON.getMod();
            case "SAB" -> SAB.getMod();
            case "INT" -> INT.getMod();
            case "CAR" -> CAR.getMod();
            default -> 0;
        };
    }

    public void setStat(String sts, int value) {
        switch (normalizeStatKey(sts)) {
            case "FUE" -> FUE.aumValor(value);
            case "DES" -> DES.aumValor(value);
            case "CON" -> CON.aumValor(value);
            case "SAB" -> SAB.aumValor(value);
            case "INT" -> INT.aumValor(value);
            case "CAR" -> CAR.aumValor(value);
            default -> { }
        }
    }

    public String getCheckSavingThrow(String sts) {
        return switch (normalizeStatKey(sts)) {
            case "FUE" -> FUE.getComp();
            case "DES" -> DES.getComp();
            case "CON" -> CON.getComp();
            case "SAB" -> SAB.getComp();
            case "INT" -> INT.getComp();
            case "CAR" -> CAR.getComp();
            default -> "0";
        };
    }

    public String getSavingThrow(String sts) {
        return switch (normalizeStatKey(sts)) {
            case "FUE" -> String.valueOf(FUE.getSav());
            case "DES" -> String.valueOf(DES.getSav());
            case "CON" -> String.valueOf(CON.getSav());
            case "SAB" -> String.valueOf(SAB.getSav());
            case "INT" -> String.valueOf(INT.getSav());
            case "CAR" -> String.valueOf(CAR.getSav());
            default -> "0";
        };
    }

    public void setHpi(int value) {
        pdGT = value;
        pdGA = pdGT;
    }

    public String getHitDice() {
        return nDdG + "/" + ddG;
    }

    public String getLiveCheck(int index) {
        return index >= 1 && index <= sdMA ? "X" : "_";
    }

    public String getLiveUncheck(int index) {
        return index >= 1 && index <= sdMF ? "X" : "_";
    }

    public String getHabilidadesEspecialesFormateadas() {
        StringBuilder sb = new StringBuilder();
        habilidadesEspeciales.forEach((key, value) -> sb.append("- ").append(key).append(" +").append(value).append("\n"));
        return sb.toString();
    }

    public Map<String, Integer> getHabilidadesEspeciales() {
        return new LinkedHashMap<>(habilidadesEspeciales);
    }

    public void setHabilidadEspecial(String habilidad, int valor) {
        String normalized = normalizeKey(habilidad);
        habilidadesEspeciales.merge(normalized, valor, Integer::sum);
    }

    public void setSavingThrows(List<String> savingThrows) {
        for (String save : nullSafe(savingThrows)) {
            switch (normalizeKey(save)) {
                case "fuerza", "fuerca" -> FUE.setCompetencia(true);
                case "destreza", "destresa" -> DES.setCompetencia(true);
                case "constitucion" -> CON.setCompetencia(true);
                case "inteligencia" -> INT.setCompetencia(true);
                case "sabiduria", "saviesa" -> SAB.setCompetencia(true);
                case "carisma" -> CAR.setCompetencia(true);
                default -> { }
            }
        }
    }

    public void incrementStat(String attr, int value) {
        switch (normalizeKey(attr)) {
            case "fuerza", "fuerca" -> FUE.aumValor(value);
            case "destreza", "destresa" -> DES.aumValor(value);
            case "constitucion" -> CON.aumValor(value);
            case "inteligencia" -> INT.aumValor(value);
            case "sabiduria", "saviesa" -> SAB.aumValor(value);
            case "carisma" -> CAR.aumValor(value);
            default -> { }
        }
    }

    public void setHitDiceCount(int nDdG) { this.nDdG = nDdG; }
    public void setHitDiceValue(int ddG) { this.ddG = ddG; }
    public void setSpellcastingAbility(String atributo) { this.atributo = valueOrEmpty(atributo); }
    public void setMechanicsProficencies(List<String> mechanics) { this.mecanicasProficiencies = new ArrayList<>(nullSafe(mechanics)); }
    public void setHabilidadesProficiencies(Map<Integer, List<String>> habilidadesPorNivel) { this.hechizosProficiencies = copyLevelMap(habilidadesPorNivel); }
    public void setaEspecialesProficiencies(List<String> habilidades) { this.aEspecialesProficiencies = new ArrayList<>(nullSafe(habilidades)); }
    public void setWeaponProficiencies(List<String> weapons) { this.weaponProficiencies = new ArrayList<>(nullSafe(weapons)); }
    public void setArmorProficiencies(List<String> armors) { this.armorProficiencies = new ArrayList<>(nullSafe(armors)); }
    public void setAC(int ca) { this.ca = ca; }
    public void setProfBonus(int bonCompetencia) { this.bonCompetencia = bonCompetencia; }

    public void setSkills(List<String> seleccionades) {
        for (String skill : nullSafe(seleccionades)) {
            switch (normalizeKey(skill)) {
                case "atletismo" -> markSkill(atletismo);
                case "acrobacias" -> markSkill(acrobacias);
                case "juego_de_mano", "juego_de_manos" -> markSkill(juego_de_Manos);
                case "sigilo" -> markSkill(sigilo);
                case "arcana", "arcano", "arcan" -> markSkill(arcan);
                case "historia" -> markSkill(historia);
                case "investigacion" -> markSkill(investigacion);
                case "naturaleza" -> markSkill(naturaleza);
                case "religion" -> markSkill(religion);
                case "trato_con_animales", "trato_animales", "trato_de_animales" -> markSkill(trato_de_animales);
                case "medicina" -> markSkill(medicina);
                case "percepcion" -> markSkill(percepcion);
                case "perspicacia" -> markSkill(perspicacia);
                case "supervivencia" -> markSkill(supervivencia);
                case "engano", "engaio" -> markSkill(enganio);
                case "intimidacion" -> markSkill(intimidacion);
                case "interpretacion" -> markSkill(interpretacion);
                case "persuasion" -> markSkill(persuasion);
                default -> System.out.println("Competència desconeguda: " + normalizeKey(skill));
            }
        }
    }

    private void markSkill(Habilitat skill) {
        skill.setCompetencia(true);
        skill.incMod(1);
    }

    public String ListOfWeaponandArmor() { return listOfWeaponandArmor(); }

    public String listOfWeaponandArmor() {
        StringBuilder sb = new StringBuilder();
        if (!weaponProficiencies.isEmpty()) {
            sb.append("Armas: ").append(String.join(", ", weaponProficiencies));
        }
        if (!armorProficiencies.isEmpty()) {
            if (sb.length() > 0) {
                sb.append(" | ");
            }
            sb.append("Armaduras: ").append(String.join(", ", armorProficiencies));
        }
        return sb.toString();
    }

    public void recalcularHabilitats() {
        for (Habilitat skill : allSkills()) {
            int total = getModi(skill.getPadre());
            if (skill.getCompB()) {
                total += bonCompetencia;
            }
            skill.setMod(total);
        }
        percepcionPasiva = 10 + percepcion.getMod();
    }

    public void imprimirHabilidadesEspeciales() {
        System.out.println("=== HABILIDADES ESPECIALES ===");
        aEspecialesProficiencies.forEach(this::printJsonLikeEntry);
    }

    public void imprimirMecanicas() {
        System.out.println("=== MECÁNICAS ===");
        mecanicasProficiencies.forEach(this::printJsonLikeEntry);
    }

    public void imprimirHechizos() {
        System.out.println("=== HECHIZOS POR NIVEL ===");
        hechizosProficiencies.entrySet().stream().sorted(Map.Entry.comparingByKey()).forEach(entry -> {
            System.out.println("\nNivel " + entry.getKey() + ":");
            entry.getValue().forEach(this::printJsonLikeEntry);
        });
    }

    public void imprimirTodaLaInformacion() {
        imprimirHabilidadesEspeciales();
        System.out.println("\n");
        imprimirMecanicas();
        System.out.println("\n");
        imprimirHechizos();
    }

    public boolean isSpellcaster() {
        return (atributo != null && !atributo.isEmpty()) || (hechizosProficiencies != null && !hechizosProficiencies.isEmpty());
    }

    public Map<String, List<String>> getSpellsByLevel() {
        Map<String, List<String>> spellsByLevel = new LinkedHashMap<>();
        hechizosProficiencies.entrySet().stream().sorted(Map.Entry.comparingByKey()).forEach(entry -> {
            List<String> spellDescriptions = new ArrayList<>();
            for (String spell : entry.getValue()) {
                spellDescriptions.add(formatSpellDescription(spell));
            }
            spellsByLevel.put("Nivel " + entry.getKey(), spellDescriptions);
        });
        return spellsByLevel;
    }

    public InventoryItem[] getInventory() {
        List<InventoryItem> inventory = new ArrayList<>(equipoReal);
        for (String weapon : weaponProficiencies) {
            inventory.add(new InventoryItem(weapon, "weapon", 1, 1.5, "Varía"));
        }
        for (String armor : armorProficiencies) {
            inventory.add(new InventoryItem(armor, "armor", 1, 5.0, "Varía"));
        }
        return inventory.toArray(new InventoryItem[0]);
    }

    public int getSpellSaveDC() {
        if (atributo == null || atributo.isEmpty()) {
            return 0;
        }
        return 8 + bonCompetencia + getModi(atributo);
    }

    public int getSpellAttackBonus() {
        if (atributo == null || atributo.isEmpty()) {
            return 0;
        }
        return bonCompetencia + getModi(atributo);
    }

    public Map<String, Integer> getSpellSlots() {
        Map<String, Integer> spellSlots = new LinkedHashMap<>();
        if (nivel >= 1) spellSlots.put("1", 2 + (nivel > 1 ? 1 : 0));
        if (nivel >= 3) spellSlots.put("2", 2);
        if (nivel >= 5) spellSlots.put("3", 2);
        if (nivel >= 7) spellSlots.put("4", 1);
        if (nivel >= 9) spellSlots.put("5", 1);
        if (nivel >= 11) spellSlots.put("6", 1);
        if (nivel >= 13) spellSlots.put("7", 1);
        if (nivel >= 15) spellSlots.put("8", 1);
        if (nivel >= 17) spellSlots.put("9", 1);
        return spellSlots;
    }

    public List<Spell> getSpells() {
        List<Spell> spells = new ArrayList<>();
        hechizosProficiencies.forEach((level, spellEntries) -> {
            for (String spellEntry : spellEntries) {
                spells.add(parseSpell(spellEntry, level));
            }
        });
        return spells;
    }

    public void inicializarValoresBase() {
        if ("Aarakocra".equals(race)) {
            vel = 25;
            setHabilidadEspecial("Vuelo", 50);
        }
        pdGT = Math.max(1, 8 + CON.getMod() + (8 + CON.getMod()) * Math.max(0, nivel - 1));
        pdGA = pdGT;
        ddG = 8;
        nDdG = nivel;
    }

    public Map<Integer, List<String>> getHechizosDisponibles() {
        Map<Integer, List<String>> hechizosDisponibles = new LinkedHashMap<>();
        hechizosProficiencies.entrySet().stream()
                .filter(entry -> entry.getKey() <= nivel)
                .sorted(Map.Entry.comparingByKey())
                .forEach(entry -> hechizosDisponibles.put(entry.getKey(), new ArrayList<>(entry.getValue())));
        return hechizosDisponibles;
    }

    private List<Habilitat> allSkills() {
        return Arrays.asList(atletismo, acrobacias, juego_de_Manos, sigilo, arcan, historia, investigacion,
                naturaleza, religion, trato_de_animales, medicina, percepcion, perspicacia, supervivencia,
                enganio, intimidacion, interpretacion, persuasion);
    }

    private Spell parseSpell(String spellEntry, int fallbackLevel) {
        Spell spell = new Spell();
        try {
            if (spellEntry != null && spellEntry.trim().startsWith("{")) {
                JsonNode node = MAPPER.readTree(spellEntry);
                spell.setName(text(node, "nombre", "Hechizo sin nombre"));
                spell.setLevel(node.has("nivel") ? node.path("nivel").asInt(fallbackLevel) : fallbackLevel);
                spell.setSchool(text(node, "escuela", ""));
                spell.setDescription(text(node, "descripcion", text(node, "efecto", "")));
            } else {
                spell.setName(spellEntry);
                spell.setLevel(fallbackLevel);
                spell.setDescription("Hechizo obtenido al nivel " + fallbackLevel);
            }
        } catch (Exception e) {
            spell.setName(spellEntry);
            spell.setLevel(fallbackLevel);
            spell.setDescription("Hechizo obtenido al nivel " + fallbackLevel);
        }
        return spell;
    }

    private String formatSpellDescription(String spell) {
        if (spell != null && spell.trim().startsWith("{")) {
            try {
                JsonNode jsonSpell = MAPPER.readTree(spell);
                StringBuilder sb = new StringBuilder(text(jsonSpell, "nombre", ""));
                String efecto = text(jsonSpell, "efecto", "");
                if (!efecto.isBlank()) {
                    sb.append(": ").append(efecto);
                }
                String usos = text(jsonSpell, "usos", "");
                if (!usos.isBlank()) {
                    sb.append(" (Usos: ").append(usos).append(")");
                }
                return sb.toString();
            } catch (Exception ignored) {
                return spell;
            }
        }
        return spell;
    }

    private void printJsonLikeEntry(String entry) {
        Matcher matcher = NAMED_JSON_PATTERN.matcher(valueOrEmpty(entry));
        String json = matcher.find() ? matcher.group(2) : entry;
        try {
            if (json != null && json.trim().startsWith("{")) {
                JsonNode node = MAPPER.readTree(json);
                String label = matcher.matches() ? matcher.group(1) : text(node, "nombre", "");
                if (!label.isBlank()) {
                    System.out.println("• " + label + ":");
                }
                node.fields().forEachRemaining(field -> System.out.println("  - " + field.getKey() + ": " + field.getValue()));
            } else {
                System.out.println("• " + entry);
            }
        } catch (Exception e) {
            System.out.println("• " + entry);
        }
        System.out.println();
    }

    private static void addStringArray(ObjectNode parent, String key, List<String> values) {
        ArrayNode array = parent.putArray(key);
        for (String value : nullSafe(values)) {
            array.add(value);
        }
    }

    private static Map<Integer, List<String>> copyLevelMap(Map<Integer, List<String>> source) {
        Map<Integer, List<String>> copy = new LinkedHashMap<>();
        if (source != null) {
            source.forEach((key, value) -> copy.put(key, new ArrayList<>(nullSafe(value))));
        }
        return copy;
    }

    private static List<String> nullSafe(List<String> values) {
        return values == null ? Collections.emptyList() : values;
    }

    private static String valueOrEmpty(String value) {
        return value == null ? "" : value;
    }

    private static String text(JsonNode node, String key, String fallback) {
        JsonNode value = node == null ? null : node.get(key);
        return value == null || value.isNull() ? fallback : value.asText(fallback);
    }

    private static String normalizeKey(String value) {
        if (value == null) {
            return "";
        }
        return Normalizer.normalize(value, Normalizer.Form.NFD)
                .replaceAll("\\p{M}", "")
                .toLowerCase(Locale.ROOT)
                .trim()
                .replaceAll("[^a-z0-9]+", "_")
                .replaceAll("^_+|_+$", "");
    }

    private static String normalizeStatKey(String value) {
        if (value == null) {
            return "";
        }
        return switch (normalizeKey(value)) {
            case "fuerza", "fuerca", "strength", "str", "fue" -> "FUE";
            case "destreza", "destresa", "dexterity", "dex", "des" -> "DES";
            case "constitucion", "constitution", "con" -> "CON";
            case "sabiduria", "saviesa", "wisdom", "wis", "sab" -> "SAB";
            case "inteligencia", "intelligence", "int" -> "INT";
            case "carisma", "charisma", "cha", "car" -> "CAR";
            default -> value.toUpperCase(Locale.ROOT);
        };
    }

    public static class Atributo {
        private final String name;
        private int at;
        private int sav;
        private boolean competencia;

        public Atributo(String name, int at, int sav) {
            this.name = name;
            this.at = at;
            this.sav = sav;
        }

        public String getName() { return name; }
        public int getAt() { return at; }
        public int getMod() { return Math.floorDiv(at - 10, 2); }
        public int getSav() { return sav; }
        public String getComp() { return competencia ? "X" : ""; }
        public void setCompetencia(boolean competencia) { this.competencia = competencia; }
        public void aumValor(int value) { this.at += value; }
    }

    public static class Habilitat {
        private final String name;
        private final String padre;
        private int mod;
        private boolean competencia;

        public Habilitat(String name, String padre, int mod) {
            this.name = name;
            this.padre = padre;
            this.mod = mod;
        }

        public String getName() { return name; }
        public String getPadre() { return padre; }
        public int getMod() { return mod; }
        public void setMod(int mod) { this.mod = mod; }
        public void incMod(int value) { this.mod += value; }
        public String getComp() { return competencia ? "X" : ""; }
        public boolean getCompB() { return competencia; }
        public void setCompetencia(boolean competencia) { this.competencia = competencia; }
    }

    public static class InventoryItem {
        private String name = "";
        private String tipo = "";
        private int quantity;
        private double weight;
        private String value = "";

        public InventoryItem() {
        }

        public InventoryItem(String name, String tipo, int quantity, double weight, String value) {
            this.name = valueOrEmpty(name);
            this.tipo = valueOrEmpty(tipo);
            this.quantity = quantity;
            this.weight = weight;
            this.value = valueOrEmpty(value);
        }

        public String getName() { return name; }
        public void setName(String name) { this.name = valueOrEmpty(name); }
        public String getTipo() { return tipo; }
        public void setTipo(String tipo) { this.tipo = valueOrEmpty(tipo); }
        public int getQuantity() { return quantity; }
        public void setQuantity(int quantity) { this.quantity = quantity; }
        public double getWeight() { return weight; }
        public void setWeight(double weight) { this.weight = weight; }
        public String getValue() { return value; }
        public void setValue(String value) { this.value = valueOrEmpty(value); }
    }

    public static class Spell {
        private String name = "";
        private int level;
        private String school = "";
        private String description = "";

        public String getName() { return name; }
        public void setName(String name) { this.name = valueOrEmpty(name); }
        public int getLevel() { return level; }
        public void setLevel(int level) { this.level = level; }
        public String getSchool() { return school; }
        public void setSchool(String school) { this.school = valueOrEmpty(school); }
        public String getDescription() { return description; }
        public void setDescription(String description) { this.description = valueOrEmpty(description); }
    }
}
