package cat.dnd.cc.eines;

import cat.dnd.cc.model.TierBackground;
import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.model.TierFeat;
import cat.dnd.cc.model.TierRace;
import cat.dnd.cc.model.TierSkill;

import java.util.List;

/**
 * Resultado de resolver un {@link cat.dnd.cc.model.Personatge} contra el catálogo de cartas:
 * las cartas concretas que ha elegido (clase, raza, trasfondo, habilidades, equipo, dotes),
 * sus stats finales (base + raza + trasfondo + equipo) y la vida calculada con
 * {@link CalculadoraVida} siguiendo la fórmula del sistema
 * (Vida = Vida Base de Clase + CON × Multiplicador + Bono de Tier + Bono de Equipo).
 *
 * <p>También expone las defensas derivadas del Manual Sagrado (Edición 2):
 * Clase de Armadura = 10 + DES + bono de armadura equipada,
 * Defensa mental = 10 + CAR, Resistencia física = 10 + CON,
 * Iniciativa = DES + modificadores de equipo/dotes.</p>
 */
public record FichaPersonatge(
        TierClass clase,
        TierRace raza,
        TierBackground transfons,
        List<TierSkill> habilidades,
        List<TierEquipment> equipo,
        List<TierFeat> dotes,
        int finalCon,
        int finalDes,
        int finalCar,
        int finalInt,
        double vida,
        int claseArmadura,
        int defensaMental,
        int resistenciaFisica,
        int iniciativa
) {
}
