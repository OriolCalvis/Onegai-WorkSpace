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
#include "RPG/CharacterCreationScreen.h"
#include "RPG/TierRules.h"

using RPG::CharacterCreation;
using RPG::CharacterSheet;
using RPG::CreationChoice;
using RPG::CharacterCreationScreen;
using Paso = RPG::CharacterCreationScreen::Paso;

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

    // =================================================================
    // LA PANTALLA, paso a paso y a golpe de tecla. Va aqui y no en un
    // fichero GL por el mismo motivo que ProjectHub: lo que se mete en un
    // .cpp con OpenGL delante solo se puede pasar por el compilador.
    // =================================================================
    std::printf("\n[la pantalla, paso a paso]\n");
    CharacterCreationScreen p(cc);
    require(p.paso() == Paso::Raza);
    require(p.opciones().size() == 43);
    std::printf("  paso %d/%d  %s  (%d opciones)\n", p.numeroDePaso(),
                CharacterCreationScreen::totalDePasos(), p.tituloPaso(),
                (int)p.opciones().size());

    // La lista viene ORDENADA y estable: Catalog guarda en unordered_map,
    // asi que sin ordenar la pantalla se barajaria sola entre ejecuciones.
    for (std::size_t i = 1; i < p.etiquetas().size(); ++i) {
        require(p.etiquetas()[i - 1] <= p.etiquetas()[i]);
    }
    std::printf("  lista ordenada por nombre. OK\n");

    // --- Filtrar: 43 razas pasan a las que importan ---
    for (char c : std::string("enan")) p.tecla(c);
    require(p.filtro() == "enan");
    require(p.opciones().size() < 43 && !p.opciones().empty());
    require(p.totalSinFiltrar() == 43);
    std::printf("  filtro 'enan': %d de %d razas -> %s\n", (int)p.opciones().size(),
                (int)p.totalSinFiltrar(), p.etiquetas()[0].c_str());
    // El filtro busca por NOMBRE y por id: "Enano de las Fraguas
    // Profundas" tiene el id 'race_dwarf_deepforge', asi que buscar por
    // el id solo dejaria fuera media lista. Se guarda el id que estaba
    // marcado, sea cual sea.
    const std::string razaMarcada = p.opciones()[p.marcado()];
    p.tecla('\n');
    require(p.paso() == Paso::Clase);
    require(p.filtro().empty());      // el filtro no se arrastra al paso siguiente
    require(p.eleccion().raceId == razaMarcada);

    // --- No se puede avanzar sin resolver el paso ---
    for (char c : std::string("zzzz")) p.tecla(c);
    require(p.opciones().empty());
    require(!p.siguiente());
    require(!p.aviso().empty());
    std::printf("  con filtro sin resultados no avanza: \"%s\"\n", p.aviso().c_str());
    p.limpiarFiltro();
    p.tecla('\n');
    require(p.paso() == Paso::Trasfondo);

    p.tecla('\n');   // primer trasfondo
    require(p.paso() == Paso::Reparto);
    const std::string fondo = p.eleccion().backgroundId;
    require(!fondo.empty());

    // --- Reparto: no avanza hasta gastar los 12 puntos ---
    require(p.puntosLibres() == 12);
    require(!p.siguiente());
    std::printf("  reparto: no avanza con %d puntos sin gastar\n", p.puntosLibres());
    // CON empieza en 1: cinco '+' lo dejan en 6, que es el tope. El SEXTO
    // es el que tiene que avisar, no el quinto.
    for (int i = 0; i < 5; ++i) p.tecla('+');
    require(p.aviso().empty());
    p.tecla('+');
    require(!p.aviso().empty());
    std::printf("  tope de stat en tier 1: \"%s\"\n", p.aviso().c_str());
    p.tecla('s');
    for (int i = 0; i < 5; ++i) p.tecla('+');       // DES
    p.tecla('s');
    for (int i = 0; i < 2; ++i) p.tecla('+');       // INT
    require(p.puntosLibres() == 0);
    require(p.siguiente());
    require(p.paso() == Paso::Vinculo);
    std::printf("  12 puntos repartidos, avanza. OK\n");

    // --- Las siete narrativas salen del trasfondo elegido ---
    int narrativas = 0;
    while (p.paso() >= Paso::Vinculo && p.paso() <= Paso::Virtud) {
        require(p.opciones().size() == 5);   // cinco opciones por lista
        require(p.tecla('\n') == CharacterCreationScreen::Accion::Ninguna);
        ++narrativas;
    }
    require(narrativas == 7);
    require(p.paso() == Paso::Deidad);
    std::printf("  las 7 listas narrativas, 5 opciones cada una. OK\n");

    // --- La deidad se puede no elegir, y "(ninguna)" va la primera ---
    require(p.etiquetas()[0] == "(ninguna)");
    require(p.opciones()[0].empty());
    p.tecla('\n');
    require(p.eleccion().deityId.empty());
    require(p.paso() == Paso::Nombre);
    std::printf("  deidad opcional, '(ninguna)' la primera. OK\n");

    // --- Nombre ---
    require(!p.siguiente());             // vacio no cuela
    for (char c : std::string("Bruna la Forjadora")) p.tecla(c);
    require(p.eleccion().displayName == "Bruna la Forjadora");
    p.tecla('\n');
    require(p.paso() == Paso::Resumen);

    // --- Resumen: sin problemas y construye ---
    const auto probs = p.problemas();
    if (!probs.empty()) {
        for (const auto& s : probs) std::printf("    PROBLEMA: %s\n", s.c_str());
    }
    require(probs.empty());
    require(p.tecla('\n') == CharacterCreationScreen::Accion::Terminado);
    auto hechaPantalla = p.construir();
    require(hechaPantalla);
    RPG::CharacterSheet f = hechaPantalla.value();
    cc.recalcular(f, reglas);
    std::printf("\n  %s: %s / %s / %s\n", f.displayName.c_str(), f.raceId.c_str(),
                f.classId.c_str(), f.backgroundId.c_str());
    std::printf("  CON %d DES %d INT %d CAR %d   vida %d   %d habilidades\n",
                f.con(), f.des(), f.int_(), f.car(), f.healthCap(),
                (int)f.knownSkillIds.size());

    // --- Volver atras y cambiar de trasfondo limpia las narrativas ---
    // Es el fallo que se cuela solo: los ids elegidos eran de las listas
    // del trasfondo VIEJO y siguen puestos.
    CharacterCreationScreen q(cc);
    q.tecla('\n'); q.tecla('\n');                    // raza, clase
    q.tecla('\n');                                   // trasfondo #0
    const std::string primero = q.eleccion().backgroundId;
    q.anterior();                                    // volver al trasfondo
    q.tecla('s');                                    // otro distinto
    q.tecla('\n');
    require(q.eleccion().backgroundId != primero);
    require(q.eleccion().bondId.empty());
    std::printf("\n  cambiar de trasfondo limpia las 7 narrativas. OK\n");

    // ESC en el primer paso cancela; en los demas, vuelve.
    CharacterCreationScreen r(cc);
    require(r.tecla(27) == CharacterCreationScreen::Accion::Cancelado);
    r.tecla('\n');
    require(r.paso() == Paso::Clase);
    require(r.tecla(27) == CharacterCreationScreen::Accion::Ninguna);
    require(r.paso() == Paso::Raza);
    std::printf("  ESC: cancela en el primer paso, vuelve en los demas. OK\n");


    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
