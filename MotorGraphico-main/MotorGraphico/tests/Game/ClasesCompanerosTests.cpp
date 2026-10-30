// CLASES-03 — Las seis clases de la 3.ª iteracion cargan con sus cartas y sus
// pasivas, y los companeros reclutables se reclutan por flag.
//
// SETS-03 (Oriol, 08/10/2026) — Cada una tiene su set de tier 2 (como los de
// las SP y las clases propias: raro, entero, que la clase no lleva; SETS-04: el
// Señor de las Armas tiene tres, el Duque, el Soberano sin Rostro y el Rey de la
// Forja, que sustituyen al de SETS-03), y las
// cartas del Monje del Despertar y del Mago Chasqueador («solo les puse la
// pasiva y el titulo») hacen en combate lo que dice su texto. La mesa es la
// de H2-06 (tests/Game/CartasPrueba.h): el dado fijo en 5, el heroe con CON 2,
// DES 5, INT 1 y CAR 3 contra un lobo de DES 5 (las cuentas, en la cabecera
// de tests/Game/CartasEnCombateTests.cpp: un golpe que gana por medio exito
// son 1d6 = 5, mas los dados de la carta).
#include "CartasPrueba.h"
#include "Game/Companeros.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterCreation.h"
#include "RPG/ConditionEngine.h"
#include "RPG/InventoryEngine.h"
#include "RPG/Narrative/NarrativeState.h"
#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {
const char* const kClases[] = {"monje_del_despertar", "senor_de_las_armas", "bardo_de_las_cuatro_estaciones",
                               "el_apostata", "furia_de_los_creadores", "mago_chasqueador"};

bool tieneRasgo(const Game::SituacionCombate& s, T tipo, int numero = 0) {
    for (const Game::EstadoCarta& e : s.estados)
        for (const Game::Gancho& g : e.rasgos)
            if (g.tipo == tipo && (numero == 0 || g.numero == numero)) return true;
    return false;
}

int rondasDe(const Game::SituacionCombate& s, T tipo) {
    for (const Game::EstadoCarta& e : s.estados)
        for (const Game::Gancho& g : e.rasgos)
            if (g.tipo == tipo) return e.rondas;
    return 0;
}

const Game::SituacionCombate& situHeroe(const Mesa& m) { return *m.cartas->situacion(true, 0); }

void conMiedo(Mesa& m) {
    m.batalla->setConditionCatalog(&catalogo().condiciones());
    const RPG::ConditionDefinition* miedo = catalogo().condiciones().find("asustado");
    MOTOR_CHECK(miedo != nullptr);
    if (miedo != nullptr) MOTOR_CHECK(RPG::ConditionEngine::apply(m.ficha, *miedo, catalogo().condiciones()).isOk());
}

bool tieneCondicion(const Mesa& m, const std::string& id) {
    for (const RPG::ActiveCondition& a : m.ficha.conditions)
        if (a.conditionId == id) return true;
    return false;
}
}  // namespace

MOTOR_TEST_CASE(las_seis_clases_cargan_con_siete_cartas_tres_de_creador_y_pasiva) {
    RPG::Catalogs::ClassCatalog clases;
    RPG::Catalogs::SkillCatalog skills;
    RPG::Catalogs::PassiveCatalog pasivas;
    MOTOR_CHECK(clases.loadFromFile("assets/catalogs/classes.json").isOk());
    MOTOR_CHECK(skills.loadFromFile("assets/catalogs/skills.json").isOk());
    MOTOR_CHECK(pasivas.loadFromFile("assets/catalogs/passives.json").isOk());
    for (const char* id : kClases) {
        const RPG::ClassDefinition* c = clases.find(id);
        MOTOR_CHECK(c != nullptr);
        MOTOR_CHECK(c->learnableSkillIds.size() == 10);
        MOTOR_CHECK(c->startingSkillIds.size() == 3);
        MOTOR_CHECK(!c->passiveId.empty() && pasivas.find(c->passiveId) != nullptr);
        for (const std::string& s : c->learnableSkillIds) MOTOR_CHECK(skills.find(s) != nullptr);
        int creador = 0;
        for (const std::string& s : c->learnableSkillIds) {
            const auto& tags = skills.find(s)->tags;
            if (std::find(tags.begin(), tags.end(), "creador") != tags.end()) ++creador;
        }
        MOTOR_CHECK(creador == 3);
    }
    // Lo que Oriol dejo como [pendiente] sigue marcado, no inventado.
    MOTOR_CHECK(skills.find("senor_de_las_armas_pendiente_sexta_habilidad") != nullptr);
    MOTOR_CHECK(pasivas.find("senor_de_las_armas_pasiva")->description.find("[pendiente]") != std::string::npos);
    MOTOR_CHECK(skills.find("corte_real")->description.find("2d6") != std::string::npos);
}

MOTOR_TEST_CASE(las_clases_nuevas_se_pueden_elegir_al_crear_personaje) {
    const auto reglas = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(reglas.isOk());
    for (const char* id : kClases) MOTOR_CHECK(reglas.value().classes().find(id) != nullptr);
}

MOTOR_TEST_CASE(companeros_reclutables_por_flag) {
    Game::Companeros companeros;
    const auto r = companeros.cargar("assets/catalogs/npcs.json");
    MOTOR_CHECK(r.isOk() && r.value() >= 6);
    RPG::NarrativeState estado;
    MOTOR_CHECK(companeros.reclutados(estado).empty());
    MOTOR_CHECK(!companeros.reclutar("npc_que_no_existe", estado));
    MOTOR_CHECK(companeros.reclutar("companero_el_apostata", estado));
    MOTOR_CHECK(estado.hasFlag("companero_companero_el_apostata") == false);
    MOTOR_CHECK(estado.hasFlag("companero_el_apostata"));
    const auto lista = companeros.reclutados(estado);
    MOTOR_CHECK(lista.size() == 1 && lista[0]->claseId == "el_apostata" && lista[0]->usaCartasDeClase);
    companeros.despedir("companero_el_apostata", estado);
    MOTOR_CHECK(companeros.reclutados(estado).empty());
}

// --- SETS-03: el set de tier 2 de cada una -----------------------------------------------

// Como los de las SP y las clases propias: el arma y las cuatro piezas, raras
// de tier 2, con la etiqueta del set; ninguna es equipo inicial de nadie, y la
// clase se las puede poner todas (el peso de su rol). SETS-04: el Señor de las
// Armas tiene tres sets, y el de antes ya no esta en el catalogo.
MOTOR_TEST_CASE(las_seis_clases_tienen_su_set_de_tier_dos_que_no_llevan) {
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    const struct {
        const char* clase;
        const char* etiqueta;
    } kSets[] = {{"el_apostata", "Set del Apóstata"},
                 {"senor_de_las_armas", "Set del Duque"},                     // SETS-04: los tres
                 {"senor_de_las_armas", "Set del Soberano sin Rostro"},
                 {"senor_de_las_armas", "Set del Rey de la Forja"},
                 {"furia_de_los_creadores", "Set de la Furia"},                  // femenino
                 {"bardo_de_las_cuatro_estaciones", "Set de las Cuatro Estaciones"},  // femenino plural
                 {"mago_chasqueador", "Set del Chasqueador"},
                 {"monje_del_despertar", "Set del Despertar"}};
    for (const auto& s : kSets) {
        const RPG::ClassDefinition* clase = clases().find(s.clase);
        MOTOR_CHECK(clase != nullptr);
        if (clase == nullptr) continue;
        std::vector<std::string> ranuras;
        equipo.forEach([&](const RPG::EquipmentDefinition& e) {
            if (std::find(e.grantedTags.begin(), e.grantedTags.end(), s.etiqueta) == e.grantedTags.end()) return;
            ranuras.push_back(e.slot);
            MOTOR_CHECK(e.rarity == RPG::ItemRarity::Rare);
            MOTOR_CHECK(e.tier == 2);
            MOTOR_CHECK(RPG::InventoryEngine::check_weight(*clase, e));
            clases().forEach([&](const RPG::ClassDefinition& c) {
                MOTOR_CHECK(std::find(c.startingEquipmentIds.begin(), c.startingEquipmentIds.end(), e.id) ==
                            c.startingEquipmentIds.end());
            });
        });
        std::sort(ranuras.begin(), ranuras.end());
        MOTOR_CHECK((ranuras == std::vector<std::string>{"Feet", "Head", "Legs", "MainHand", "Torso"}));
    }
    // SETS-04: el set de SETS-03 del Señor de las Armas ya no esta
    int retirados = 0;
    equipo.forEach([&](const RPG::EquipmentDefinition& e) {
        if (std::find(e.grantedTags.begin(), e.grantedTags.end(), "Set del Señor de las Armas") != e.grantedTags.end())
            ++retirados;
    });
    MOTOR_CHECK(retirados == 0);
}

// --- SETS-03: las cartas del Monje del Despertar y del Mago Chasqueador --------------------

MOTOR_TEST_CASE(monje_y_chasqueador_tienen_sus_diez_cartas_escritas_y_jugables) {
    for (const char* id : {"monje_del_despertar", "mago_chasqueador"}) {
        for (const std::string& s : clases().find(id)->learnableSkillIds) {
            const Game::CatalogoCartas::Carta* c = catalogo().carta(s);
            MOTOR_CHECK(c != nullptr && !c->ganchos.empty());
            if (c == nullptr) continue;
            MOTOR_CHECK(c->bloqueo.empty());
            MOTOR_CHECK(c->def->description.find("[pendiente]") == std::string::npos);
        }
    }
    // Las del Chasqueador tiran INT; las del Monje, DES (lo de siempre).
    MOTOR_CHECK(catalogo().carta("bola_de_fuego")->def->casting_stat == RPG::Stat::INT);
    MOTOR_CHECK(catalogo().carta("tres_golpes_de_puro_dolor")->def->casting_stat == RPG::Stat::DES);
}

MOTOR_TEST_CASE(monje_empieza_pegando_y_tres_golpes_son_tres_ataques) {
    MOTOR_CHECK((clases().find("monje_del_despertar")->startingSkillIds ==
                 std::vector<std::string>{"tres_golpes_de_puro_dolor", "abdomen_de_marfil",
                                          "rejuvenecimiento_de_jade"}));
    Mesa m("monje_del_despertar");
    MOTOR_CHECK(m.jugar("Tres Golpes de Puro Dolor"));
    MOTOR_CHECK(m.lobo.health() == 45);  // tres golpes de 5
    Mesa r("monje_del_despertar");
    MOTOR_CHECK(r.jugar("Ráfaga Elemental"));
    MOTOR_CHECK(r.lobo.health() == 40);  // 1d6 de la diferencia y 3d6 de la carta
}

MOTOR_TEST_CASE(monje_abdomen_de_marfil_para_el_golpe_y_templa) {
    Mesa m("monje_del_despertar");
    MOTOR_CHECK(m.jugar("Abdomen de Marfil (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: rebota en el abdomen
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCa, 1));
    MOTOR_CHECK(m.carta("abdomen_de_marfil").gastada);
}

MOTOR_TEST_CASE(monje_puno_ensordecedor_aturde_y_el_golpe_sanguineo_sangra) {
    Mesa m("monje_del_despertar");
    MOTOR_CHECK(m.jugar("Puño Ensordecedor"));
    MOTOR_CHECK(m.lobo.health() == 50);  // 1d6 y 1d6
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 1);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::DesventajaDefensa));
    m.ronda();  // aturdido: no ataca
    MOTOR_CHECK(m.heroe.health() == 30);
    Mesa s("monje_del_despertar");
    MOTOR_CHECK(s.jugar("Golpe Sanguíneo"));
    MOTOR_CHECK(s.lobo.health() == 45);  // 1d6 y 2d6
    MOTOR_CHECK(!s.situLobo().estados.empty());
    s.batalla->tickRound();
    MOTOR_CHECK(s.lobo.health() == 44);  // sangra 1 por turno
}

MOTOR_TEST_CASE(monje_purificacion_limpia_y_el_jade_cura) {
    Mesa m("monje_del_despertar");
    conMiedo(m);
    MOTOR_CHECK(m.jugar("Purificación Corporal"));
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaDefensa) == 2);
    Mesa j("monje_del_despertar");
    j.heroe.ponerVida(15);
    MOTOR_CHECK(j.jugar("Rejuvenecimiento de Jade"));
    MOTOR_CHECK(j.heroe.health() == 25);  // 2d6
}

MOTOR_TEST_CASE(monje_de_creador_estampida_sentidos_de_alerta_y_cancion_de_dios) {
    Mesa m("monje_del_despertar", 2, 5, 2);
    MOTOR_CHECK(m.jugar("Estampida"));
    MOTOR_CHECK(m.lobo.health() == 40 && m.lobo2.health() == 40);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::VentajaContraMelee));  // caidos
    MOTOR_CHECK(tieneRasgo(*m.cartas->situacion(false, 1), T::VentajaContraMelee));
    Mesa s("monje_del_despertar", 2);
    MOTOR_CHECK(s.jugar("Sentidos de Alerta"));
    MOTOR_CHECK(rondasDe(situHeroe(s), T::VentajaAtaque) == -1);  // hasta el final del combate
    MOTOR_CHECK(rondasDe(situHeroe(s), T::VentajaDefensa) == -1);
    Mesa c("monje_del_despertar", 2);
    conMiedo(c);
    c.heroe.ponerVida(10);
    MOTOR_CHECK(c.jugar("Canción de Dios"));
    MOTOR_CHECK(!tieneCondicion(c, "asustado"));
    MOTOR_CHECK(c.heroe.health() == 20);  // 2d6
}

MOTOR_TEST_CASE(chasqueador_lanza_con_int_y_recupera_la_bola_de_fuego) {
    Mesa m("mago_chasqueador");
    MOTOR_CHECK(m.fila("Recuperación Prestidigitadora (nada que devolver)") >= 0);
    // INT 1: 2 dados (1 exito) contra la defensa del lobo (2,5): la bola no entra
    MOTOR_CHECK(m.jugar("Bola de Fuego"));
    MOTOR_CHECK(m.lobo.health() == 60);
    m.ronda();
    // un juego de manos (accion libre: el turno sigue) y la bola vuelve a la mano
    MOTOR_CHECK(m.jugar("Recuperación Prestidigitadora: Bola de Fuego"));
    MOTOR_CHECK(m.cartas->turnoAbierto());
    MOTOR_CHECK(!m.carta("bola_de_fuego").gastada);
    // INT 5: 6 dados (3 exitos) ganan por medio: 1d6 y 3d6
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(m.jugar("Bola de Fuego"));
    MOTOR_CHECK(m.lobo.health() == 40);
}

MOTOR_TEST_CASE(chasqueador_paso_instantaneo_es_movimiento_y_la_sombra_ciega) {
    Mesa m("mago_chasqueador");
    MOTOR_CHECK(m.jugar("Paso Instantáneo (movimiento)"));
    MOTOR_CHECK(m.cartas->turnoAbierto());  // no gasta la accion principal
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaDefensa) == 1);
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    // Magia (resistencia fisica): el lobo (CON 1) no se salva
    MOTOR_CHECK(m.jugar("Sombra Cegadora"));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::VentajaContraTodo) == 2);  // cegado
    MOTOR_CHECK(m.lobo.health() == 60);                               // no hace dano
    Mesa o("mago_chasqueador");
    conMiedo(o);
    MOTOR_CHECK(o.jugar("Ocupa mi Lugar"));
    MOTOR_CHECK(!tieneCondicion(o, "asustado"));
    MOTOR_CHECK(rondasDe(situHeroe(o), T::VentajaDefensa) == 2);
}

MOTOR_TEST_CASE(chasqueador_de_creador_sin_sentido_y_repliegue_magico) {
    Mesa m("mago_chasqueador", 2);
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(m.jugar("Sin Sentido"));  // defensa mental: CAR 3 del lobo contra INT 5
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 2);
    Mesa r("mago_chasqueador", 2);
    MOTOR_CHECK(r.jugar("Repliegue Mágico (reaccion)"));
    r.atacar();
    r.ronda();  // el lobo ataca: se repliega
    MOTOR_CHECK(r.heroe.health() == 30);
    MOTOR_CHECK(rondasDe(situHeroe(r), T::VentajaDefensa) == -1);  // hasta el final del combate
    Mesa t("mago_chasqueador", 2);
    t.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(t.jugar("Sin Tregua"));
    MOTOR_CHECK(t.lobo.health() == 35);  // 1d6 y 4d6
    MOTOR_CHECK(rondasDe(t.situLobo(), T::DesventajaDefensa) == 2);
}

int main() { return motor_test::runAllTests(); }
