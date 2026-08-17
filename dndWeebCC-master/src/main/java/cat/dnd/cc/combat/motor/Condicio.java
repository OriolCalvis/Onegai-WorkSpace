package cat.dnd.cc.combat.motor;

/**
 * Catàleg de les 10 condicions aplicables en joc (regla 7.4 del GDD).
 *
 * <p>Una condició modifica temporalment les capacitats d'un personatge. La carta o efecte
 * que la provoca n'especifica la durada; si no ho fa, dura fins al final de l'escena. Aquesta
 * classe només catalòga el nom i l'efecte de cada condició (per mostrar-la a la fitxa/UI,
 * D9/D10): no en resol l'aplicació mecànica (avantatge/desavantatge, dany, etc.), que
 * correspon a qui gestioni el combat en cada moment, combinant-ho amb {@link Avantatge} i
 * {@link TiradaD6}.</p>
 *
 * <p>{@link #FATIGA} és l'única condició amb graus (0-6, no un simple actiu/inactiu): vegeu
 * {@link NivellFatiga} per al seu seguiment.</p>
 */
public enum Condicio {

    SANGRADO("En començar el teu torn, reps 1d4 de dany (o el que indiqui la font). "
            + "Es cura amb un descans curt o una habilitat de curació."),

    CAIDO("Desavantatge en atacs. Els atacs cos a cos contra tu tenen avantatge. "
            + "Aixecar-te costa la teva Acció de Moviment."),

    AGARRADO("La teva velocitat baixa a 0 i no pots esquivar. Per soltar-te, has de superar "
            + "una tirada de CON contra la CD de qui t'agafa."),

    CEGADO("Desavantatge en atacs; les habilitats que requereixen veure l'objectiu fallen "
            + "automàticament."),

    ENCANTADO("No pots atacar a qui t'ha encantat; les seves interaccions socials amb tu "
            + "tenen avantatge."),

    ASUSTADO("Desavantatge en atacs i tirades d'habilitat mentre vegis la font de la por; "
            + "no pots acostar-t'hi voluntàriament."),

    INCONSCIENTE("Caus (condició Caído). No pots actuar ni moure't. Els atacs contra "
            + "tu tenen avantatge i són crítics si t'encerten."),

    PARALIZADO("No pots moure't. Desavantatge en salvacions de DES i CON. Els atacs cos a cos "
            + "contra tu tenen avantatge."),

    ENVENENADO("Desavantatge en tirades d'atac i d'habilitat. Pot evolucionar a dany continu "
            + "si no es tracta."),

    FATIGA("Cada nivell resta 1 dau a totes les tirades de característica (mínim 1 dau mentre "
            + "la característica sigui > 0). En arribar a nivell 6, el personatge mor. Es "
            + "recupera amb un descans llarg (2 nivells per descans). Vegeu NivellFatiga.");

    private final String descripcio;

    Condicio(String descripcio) {
        this.descripcio = descripcio;
    }

    /**
     * @return l'efecte mecànic de la condició, en català, tal com apareix a la regla 7.4
     */
    public String descripcio() {
        return descripcio;
    }

    /**
     * @return {@code true} si aquesta condició és {@link #FATIGA}, l'única amb graus en lloc
     *         d'un simple actiu/inactiu (vegeu {@link NivellFatiga})
     */
    public boolean esGraduable() {
        return this == FATIGA;
    }
}
