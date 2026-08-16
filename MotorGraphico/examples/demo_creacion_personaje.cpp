// Creacion de personaje, de "quiero ser un enano forjador" a ficha jugable.
//
// GL-free: las reglas no saben que existe una pantalla, asi que se pueden
// probar enteras. Lo que se comprueba aqui no es que "funcione", sino que
// RECHACE lo que tiene que rechazar: una pantalla de creacion que deja
// pasar una eleccion ilegal produce un personaje que revienta tres horas
// despues, en combate.
#include <cstdio>
#include <string>

#include "Check.h"
#include "RPG/CharacterCreation.h"
#include "RPG/TierRules.h"

using RPG::CharacterCreation;
using RPG::CharacterSheet;
using RPG::CreationChoice;

namespace {
// Un reparto legal: 12 puntos sobre el minimo de 1.
RPG::StatBlock reparto(int con, int des, int inte, int car) {
    return RPG::StatBlock{con, des, inte, car};
}
}  // namespace

int main() {
    std::printf("=== creacion de personaje ===\n\n");

    auto carga = CharacterCreation::load("assets/catalogs");
    require(carga);
    // Catalog no es copiable (posee sus entradas), asi que se usa por
    // referencia. Es lo correcto: no hay motivo para duplicar 5.000
    // definiciones en memoria.
    const CharacterCreation& cc = carga.value();
    std::printf("  catalogos: %d razas, %d clases, %d trasfondos, %d deidades\n",
                (int)cc.races().size(), (int)cc.classes().size(),
                (int)cc.backgrounds().size(), (int)cc.deities().size());
    require(cc.races().size() == 43);
    require(cc.classes().size() == 61);
    require(cc.backgrounds().size() == 12);

    // Las siete listas de eleccion del trasfondo se leen de verdad.
    const auto* op = cc.opcionesDe("mes_de_la_cosecha");
    require(op != nullptr);
    require(op->bonds.size() == 5 && op->fears.size() == 5 && op->virtues.size() == 5);
    std::printf("  'mes_de_la_cosecha': %d vinculos, %d miedos, %d virtudes\n",
                (int)op->bonds.size(), (int)op->fears.size(), (int)op->virtues.size());

    // --- Una eleccion completa y legal ---
    CreationChoice e;
    e.displayName = "Bruna la Forjadora";
    e.raceId = "enanos_forjadores";
    e.classId = "acechador_de_tejados";
    e.backgroundId = "mes_de_la_cosecha";
    e.tier = 1;
    e.baseStats = reparto(4, 5, 3, 4);   // 3+4+2+3 = 12 sobre el minimo
    require(CharacterCreation::puntosGastados(e.baseStats) == 12);
    e.bondId = op->bonds[0];
    e.fearId = op->fears[1];
    e.flawId = op->flaws[2];
    e.goalId = op->goals[0];
    e.idealId = op->ideals[3];
    e.personalityId = op->personalities[1];
    e.virtueId = op->virtues[4];

    require(cc.problemas(e).empty());
    auto hecha = cc.construir(e);
    require(hecha);
    CharacterSheet ficha = hecha.value();
    std::printf("\n  creada: %s (%s / %s)\n", ficha.displayName.c_str(),
                ficha.raceId.c_str(), ficha.classId.c_str());

    // Los bonos raciales se guardan APARTE del reparto base, no sumados
    // encima: cambiar de raza tiene que poder restar el bono viejo.
    require(ficha.baseStats == e.baseStats);
    const RPG::StatBlock fin = cc.statsFinales(e);
    require(ficha.con() == fin[RPG::kCON]);
    require(ficha.car() == fin[RPG::kCAR]);
    std::printf("  stats base %d/%d/%d/%d  +raza -> %d/%d/%d/%d\n",
                e.baseStats[0], e.baseStats[1], e.baseStats[2], e.baseStats[3],
                ficha.con(), ficha.des(), ficha.int_(), ficha.car());

    // Empieza con las cartas y el equipo de su clase: "nadie empieza con
    // una hoja en blanco" (GDD).
    require(!ficha.knownSkillIds.empty());
    require(!ficha.inventoryEquipment.empty());
    std::printf("  arranca con %d habilidades y %d objetos\n",
                (int)ficha.knownSkillIds.size(), (int)ficha.inventoryEquipment.size());

    // Vida y defensas las calcula la FICHA, no la creacion.
    RPG::TierRules reglas;
    cc.recalcular(ficha, reglas);
    require(ficha.healthCap() > 0);
    require(ficha.healthCap() <= 35);          // techo de tier 1 (GDD)
    const auto& def = ficha.defenseValues();
    require(def.get(RPG::Defense::ARMOR_CLASS) == 10 + ficha.des());
    require(def.get(RPG::Defense::PHYSICAL_SAVE) == 10 + ficha.con());
    std::printf("  vida %d (techo de tier 1: 35)   CA %d   ResFis %d\n",
                ficha.healthCap(), def.get(RPG::Defense::ARMOR_CLASS),
                def.get(RPG::Defense::PHYSICAL_SAVE));

    // --- Y ahora lo que importa: lo que tiene que RECHAZAR ---
    std::printf("\n[lo que no cuela]\n");

    auto falla = [&](CreationChoice mala, const char* porque) {
        const auto p = cc.problemas(mala);
        require(!p.empty());
        require(!cc.construir(mala).isOk());
        std::printf("    %-34s -> \"%s\"\n", porque, p.front().c_str());
    };

    { CreationChoice m = e; m.displayName = ""; falla(m, "sin nombre"); }
    { CreationChoice m = e; m.raceId = "elfo_inventado"; falla(m, "raza que no existe"); }
    { CreationChoice m = e; m.classId = "clase_inventada"; falla(m, "clase que no existe"); }
    { CreationChoice m = e; m.deityId = "dios_inventado"; falla(m, "deidad que no existe"); }
    { CreationChoice m = e; m.baseStats = reparto(6, 6, 6, 6); falla(m, "reparte de mas"); }
    { CreationChoice m = e; m.baseStats = reparto(1, 1, 1, 1); falla(m, "no reparte nada"); }
    { CreationChoice m = e; m.baseStats = reparto(9, 2, 1, 1); falla(m, "pasa de 6 en tier 1"); }
    { CreationChoice m = e; m.tier = 9; falla(m, "tier fuera de rango"); }
    { CreationChoice m = e; m.bondId = ""; falla(m, "sin elegir vinculo"); }
    // El fallo interesante: un id que EXISTE pero es de otra lista.
    { CreationChoice m = e; m.bondId = op->fears[0]; falla(m, "vinculo que es un miedo"); }
    // Y de otro trasfondo distinto.
    const auto* otro = cc.opcionesDe("mes_de_la_siembra");
    if (otro != nullptr && !otro->bonds.empty()) {
        CreationChoice m = e; m.bondId = otro->bonds[0];
        falla(m, "vinculo de otro trasfondo");
    }

    // Todos los problemas de golpe, no de uno en uno: una pantalla que
    // corrige un error por intento es una pantalla que se abandona.
    CreationChoice vacia;
    const auto muchos = cc.problemas(vacia);
    require(muchos.size() >= 5);
    std::printf("\n  una eleccion vacia da %d problemas de golpe, no uno\n", (int)muchos.size());

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
