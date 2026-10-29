// CP-01 — Las clases propias (planificacion/CANON_CLASES_PROPIAS.md): los
// cuatro bardos, los dos clerigos, los dos paladines, el ingeniero, el
// mercader, el alquimista, la cartomante, el druida, el domador, los dos
// chamanes, el tempestario, el necromago y los dos piratas de Vinclap. Cada
// una carga con sus 7 cartas, sus 3 de Creador y su pasiva, todas jugables,
// tiene su set de tier 2 y hace en combate lo que dice su texto. Y el gancho
// nuevo azar_una (la lectura de la Cartomante).
//
// La mesa es la de H2-06 (tests/Game/CartasPrueba.h): dado fijo en 5, el
// heroe con CON 2, DES 5, INT 1 y CAR 3 contra un lobo de DES 5. Las clases
// que tiran CAR o INT suben ese atributo a 5 (6 dados, 3 exitos, como la DES
// del heroe) para que sus golpes entren contra el lobo (2,5).
#include "CartasPrueba.h"
#include "Game/BarraCombate.h"
#include "RPG/CharacterCreation.h"
#include "RPG/ConditionEngine.h"
#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

const char* const kPropias[] = {"bardo_de_las_dos_mascaras", "bardo_del_cantar_de_gesta",
                                "bardo_del_tambor_de_guerra", "bardo_de_la_feria",
                                "clerigo_de_la_espiga_dorada", "clerigo_del_caminante_gris",
                                "paladin_del_juramento",     "caballero_del_rey_lobo",
                                "ingeniero_de_asedio",       "mercader_errante",
                                "alquimista",                "cartomante",
                                "druida_del_roble",          "domador_de_bestias",
                                "chaman_de_los_elementos",   "chaman_de_los_espiritus",
                                "tempestario",               "necromago_de_las_animas",
                                "corsario_vampirico",        "forajido_de_vinclap"};

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

bool enElLog(const Mesa& m, const std::string& texto) {
    for (const std::string& l : m.batalla->log())
        if (l.find(texto) != std::string::npos) return true;
    return false;
}

}  // namespace

MOTOR_TEST_CASE(cada_clase_propia_tiene_diez_cartas_jugables_y_su_pasiva) {
    RPG::Catalogs::SkillCatalog skills;
    RPG::Catalogs::PassiveCatalog pasivas;
    MOTOR_CHECK(skills.loadFromFile("assets/catalogs/skills.json").isOk());
    MOTOR_CHECK(pasivas.loadFromFile("assets/catalogs/passives.json").isOk());
    for (const char* id : kPropias) {
        const RPG::ClassDefinition* c = clases().find(id);
        MOTOR_CHECK(c != nullptr);
        if (c == nullptr) continue;
        MOTOR_CHECK(c->learnableSkillIds.size() == 10);
        MOTOR_CHECK(c->startingSkillIds.size() == 3);
        MOTOR_CHECK(!c->passiveId.empty() && pasivas.find(c->passiveId) != nullptr);
        MOTOR_CHECK(c->startingEquipmentIds.size() == 1);  // un arma normal: el set no
        int creador = 0;
        for (const std::string& s : c->learnableSkillIds) {
            const RPG::SkillDefinition* d = skills.find(s);
            MOTOR_CHECK(d != nullptr);
            if (d == nullptr) continue;
            creador += std::find(d->tags.begin(), d->tags.end(), "creador") != d->tags.end() ? 1 : 0;
            // Todas jugables: con ganchos, todos entendidos y ninguna pendiente.
            const Game::CatalogoCartas::Carta* carta = catalogo().carta(s);
            MOTOR_CHECK(carta != nullptr && !carta->ganchos.empty());
            if (carta != nullptr) MOTOR_CHECK(carta->bloqueo.empty());
        }
        MOTOR_CHECK(creador == 3);
        // El arranque, de las estandar.
        for (const std::string& s : c->startingSkillIds) {
            const RPG::SkillDefinition* d = skills.find(s);
            MOTOR_CHECK(d != nullptr && d->tier_min_clamped() == 1);
        }
    }
    const auto reglas = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(reglas.isOk());
    if (!reglas.isOk()) return;
    for (const char* id : kPropias) MOTOR_CHECK(reglas.value().classes().find(id) != nullptr);
}

// «Una armadura de tier dos para cada uno»: el arma y las cuatro piezas, raras
// de tier 2, con la etiqueta del set; ninguna es equipo inicial de nadie.
MOTOR_TEST_CASE(cada_clase_propia_tiene_su_set_de_tier_dos) {
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    const struct {
        const char* clase;
        const char* etiqueta;
    } kSets[] = {{"bardo_de_las_dos_mascaras", "Set del Comediante"},
                 {"bardo_del_cantar_de_gesta", "Set del Juglar"},
                 {"bardo_del_tambor_de_guerra", "Set del Tamborilero"},
                 {"bardo_de_la_feria", "Set del Feriante"},
                 {"clerigo_de_la_espiga_dorada", "Set de la Espiga Dorada"},  // femenino
                 {"clerigo_del_caminante_gris", "Set del Caminante Gris"},
                 {"paladin_del_juramento", "Set del Juramento"},
                 {"caballero_del_rey_lobo", "Set del Rey Lobo"},
                 {"ingeniero_de_asedio", "Set del Ingeniero"},
                 {"mercader_errante", "Set del Mercader"},
                 {"alquimista", "Set del Alquimista"},
                 {"cartomante", "Set del Cartomante"},
                 {"druida_del_roble", "Set del Druida"},
                 {"domador_de_bestias", "Set del Domador"},
                 {"chaman_de_los_elementos", "Set del Chamán de los Elementos"},
                 {"chaman_de_los_espiritus", "Set del Chamán de los Espíritus"},
                 {"tempestario", "Set del Tempestario"},
                 {"necromago_de_las_animas", "Set del Necromago"},
                 {"corsario_vampirico", "Set del Corsario Vampírico"},
                 {"forajido_de_vinclap", "Set del Forajido"}};
    for (const auto& s : kSets) {
        std::vector<std::string> ranuras;
        equipo.forEach([&](const RPG::EquipmentDefinition& e) {
            if (std::find(e.grantedTags.begin(), e.grantedTags.end(), s.etiqueta) == e.grantedTags.end()) return;
            ranuras.push_back(e.slot);
            MOTOR_CHECK(e.rarity == RPG::ItemRarity::Rare);
            MOTOR_CHECK(e.tier == 2);
            clases().forEach([&](const RPG::ClassDefinition& c) {
                MOTOR_CHECK(std::find(c.startingEquipmentIds.begin(), c.startingEquipmentIds.end(), e.id) ==
                            c.startingEquipmentIds.end());
            });
        });
        std::sort(ranuras.begin(), ranuras.end());
        MOTOR_CHECK((ranuras == std::vector<std::string>{"Feet", "Head", "Legs", "MainHand", "Torso"}));
        MOTOR_CHECK(clases().find(s.clase) != nullptr);
    }
}

// --- azar_una (la lectura de la Cartomante) -------------------------------------

MOTOR_TEST_CASE(azar_una_se_lee_como_elige_una_pero_al_azar) {
    const Game::Gancho g = Game::leerGancho("azar_una:sol,luna,estrella,torre");
    MOTOR_CHECK(g.tipo == T::AzarUna);
    MOTOR_CHECK((g.opciones == std::vector<std::string>{"sol", "luna", "estrella", "torre"}));
}

MOTOR_TEST_CASE(cartomante_echa_las_cartas_y_la_suerte_saca_su_arcano) {
    Mesa m("cartomante");
    // Una sola fila y ninguna de elegir: decide la suerte.
    MOTOR_CHECK(m.fila("Echar las cartas") == 0);
    MOTOR_CHECK(m.fila("Elegir") < 0);
    const Game::VistaBarra v = Game::vistaBarra(*m.cartas, 0);
    MOTOR_CHECK(v.pasivaAlAzar && v.opcionesPasiva.size() == 1 && v.filasPasiva.size() == 1);
    MOTOR_CHECK(m.jugar("Echar las cartas"));
    // El dado fijo en 5, entre 1 y 4: la cuarta, la Torre (+1 a los golpes con arma).
    MOTOR_CHECK(m.cartas->pasiva().elegida == "torre");
    MOTOR_CHECK(enElLog(m, "La suerte saca Torre"));
    MOTOR_CHECK(m.fila("Echar las cartas") < 0);  // una vez por combate
    MOTOR_CHECK(m.cartas->turnoAbierto());        // accion libre
    m.atacar();
    MOTOR_CHECK(m.lobo.health() == 54);  // 5 y la Torre +1
}

MOTOR_TEST_CASE(cartomante_la_rueda_de_la_fortuna_suma_dos_dados) {
    Mesa m("cartomante");
    MOTOR_CHECK(m.jugar("La Rueda de la Fortuna (reaccion)"));
    MOTOR_CHECK(m.cartas->turnoAbierto());
    m.atacar();  // la proxima tirada del heroe: 8 dados en vez de 6
    MOTOR_CHECK(enElLog(m, "La Rueda de la Fortuna: +2 dados"));
    MOTOR_CHECK(m.carta("cartomante_la_rueda_de_la_fortuna").gastada);
}

// --- Bardos -----------------------------------------------------------------------

MOTOR_TEST_CASE(dos_mascaras_elige_mascara_y_el_monologo_deja_sin_atacar) {
    Mesa m("bardo_de_las_dos_mascaras");
    MOTOR_CHECK(m.fila("Elegir Tragedia") >= 0 && m.fila("Elegir Comedia") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Comedia"));
    MOTOR_CHECK(m.cartas->pasiva().elegida == "comedia");
    // Defensa mental: el lobo se salva con CAR 3 (1,5) contra el CAR 3 del bardo (2).
    MOTOR_CHECK(m.jugar("Monólogo"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Incapacitado));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 1);
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.lobo.health() == 60);  // no hace dano
}

MOTOR_TEST_CASE(dos_mascaras_morir_en_escena_para_el_golpe_y_da_ventaja) {
    Mesa m("bardo_de_las_dos_mascaras");
    MOTOR_CHECK(m.jugar("Morir en escena (reaccion)"));
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::VentajaAtaque));
}

MOTOR_TEST_CASE(cantar_de_gesta_la_balada_cura_uno_mas_y_el_ultimo_cantar_levanta) {
    Mesa m("bardo_del_cantar_de_gesta", 2);
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Balada de los caídos"));
    MOTOR_CHECK(m.heroe.health() == 26);  // 1d6 = 5, y la pasiva +1
    m.ronda();
    m.heroe.ponerVida(1);
    MOTOR_CHECK(m.jugar("El último cantar (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo lo tumba y el cantar lo levanta
    MOTOR_CHECK(m.heroe.isAlive());
    MOTOR_CHECK(m.carta("bardo_del_cantar_de_gesta_el_ultimo_cantar").gastada);
}

MOTOR_TEST_CASE(tambor_de_guerra_toca_carga_y_el_contratiempo_asusta) {
    Mesa m("bardo_del_tambor_de_guerra");
    MOTOR_CHECK(m.fila("Elegir Marcha") >= 0 && m.fila("Elegir Retirada") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Carga"));
    m.atacar();
    MOTOR_CHECK(m.lobo.health() == 54);  // 5 y la carga +1
    m.ronda();
    MOTOR_CHECK(m.jugar("Contratiempo (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();  // el lobo ataca: parado, y queda asustado
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::DesventajaAtaque));
}

MOTOR_TEST_CASE(feria_lanza_tres_cuchillos_y_el_pregon_le_atrae) {
    Mesa m("bardo_de_la_feria");
    MOTOR_CHECK(m.jugar("Lanzar cuchillos"));
    MOTOR_CHECK(m.lobo.health() == 45);  // tres golpes de 5
    m.ronda();
    Mesa p("bardo_de_la_feria", 1, 5, 2);
    MOTOR_CHECK(p.jugar("¡Pasen y vean!"));  // CAR 3 (2) contra la defensa mental del lobo (1,5)
    MOTOR_CHECK(tieneRasgo(p.situLobo(), T::Provocado));
    MOTOR_CHECK(rondasDe(situHeroe(p), T::BonoCa) == 2);
}

// --- Clerigos y paladines ---------------------------------------------------------

MOTOR_TEST_CASE(espiga_dorada_la_hogaza_cura_uno_mas_y_la_semilla_para_el_golpe) {
    Mesa m("clerigo_de_la_espiga_dorada");
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Hogaza compartida"));
    MOTOR_CHECK(m.heroe.health() == 16);  // 1d6 = 5, y la paciencia +1
    m.ronda();
    MOTOR_CHECK(m.jugar("Semilla de paciencia (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCura));
}

MOTOR_TEST_CASE(caminante_gris_destierra_y_la_fuente_limpia_y_cura) {
    Mesa m("clerigo_del_caminante_gris");
    MOTOR_CHECK(m.jugar("Destierro gris"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Incapacitado));
    MOTOR_CHECK(m.lobo.health() == 60);
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    conMiedo(m);
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Fuente del cruce"));
    MOTOR_CHECK(m.heroe.health() == 25);  // primero limpia, luego 1d6
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
}

MOTOR_TEST_CASE(juramento_jura_proteger_y_el_desafio_reta_a_uno) {
    Mesa m("paladin_del_juramento", 1, 5, 2);
    MOTOR_CHECK(m.fila("Elegir Castigar") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Proteger"));
    MOTOR_CHECK(m.jugar("Desafío jurado"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Provocado));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Provocado) == 1);
    MOTOR_CHECK(m.cartas->situacion(false, 1)->estados.empty());  // el otro lobo, nada
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoCa) == 2);
}

MOTOR_TEST_CASE(juramento_firme_como_el_yunque_para_y_templa) {
    Mesa m("paladin_del_juramento");
    MOTOR_CHECK(m.jugar("Firme como el yunque (reaccion)"));
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCa, 1));
}

MOTOR_TEST_CASE(rey_lobo_el_colmillo_cura_lo_que_quita_y_el_grunido_asusta) {
    Mesa m("caballero_del_rey_lobo");
    m.heroe.ponerStat(RPG::Stat::CAR, 5);
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Colmillo de escarcha"));
    const int quitado = 60 - m.lobo.health();
    MOTOR_CHECK(quitado > 0);
    MOTOR_CHECK(m.heroe.health() == 10 + quitado);
    m.ronda();
    MOTOR_CHECK(m.jugar("Gruñido (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::DesventajaAtaque));
}

// --- Ingenio y oficios ------------------------------------------------------------

MOTOR_TEST_CASE(ingeniero_monta_la_torreta_tres_turnos_y_el_cepo_tumba) {
    Mesa m("ingeniero_de_asedio", 1, 1);  // lobo con DES 1: la torreta (DES 3) le acierta
    MOTOR_CHECK(m.fila("Torreta de ballesta: Torreta de ballesta") >= 0);
    MOTOR_CHECK(m.jugar("Torreta de ballesta"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    MOTOR_CHECK(m.batalla->allies()[1].name == "Torreta de ballesta");
    MOTOR_CHECK(m.lobo.health() < 60);  // dispara al cerrar el turno
    for (int i = 0; i < 4; ++i) m.batalla->tickRound();
    MOTOR_CHECK(!m.batalla->allies()[1].combatant->isAlive());  // se desmonta
    Mesa c("ingeniero_de_asedio");
    MOTOR_CHECK(c.jugar("Cepo (reaccion)"));
    c.atacar();
    c.ronda();
    MOTOR_CHECK(c.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(c.situLobo(), T::VentajaContraMelee));  // caido
}

MOTOR_TEST_CASE(mercader_soborna_contrata_y_el_seguro_de_vida_levanta) {
    Mesa m("mercader_errante", 2);
    MOTOR_CHECK(m.jugar("Soborno"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Incapacitado));
    m.ronda();
    MOTOR_CHECK(m.jugar("Contratar escolta"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    MOTOR_CHECK(m.batalla->allies()[1].name == "Escolta a sueldo");
    m.ronda();
    m.heroe.ponerVida(1);
    MOTOR_CHECK(m.jugar("Seguro de vida (reaccion)"));
    m.atacar();
    m.heroe.ponerStat(RPG::Stat::DES, 1);  // con la cota (+1 a la CA) el lobo empataba: que le acierte
    m.ronda();  // el lobo lo tumba y la poliza lo levanta
    MOTOR_CHECK(m.heroe.isAlive());
    MOTOR_CHECK(m.carta("mercader_errante_seguro_de_vida").gastada);
}

MOTOR_TEST_CASE(alquimista_prepara_mercurio_y_la_escarcha_deja_sin_defensa) {
    Mesa m("alquimista");
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(m.fila("Elegir Sal") >= 0 && m.fila("Elegir Azufre") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Mercurio"));
    MOTOR_CHECK(m.jugar("Bomba de escarcha"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::DesventajaDefensa));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DesventajaDefensa) == 2);
    Mesa p("alquimista");
    p.heroe.ponerStat(RPG::Stat::INT, 5);
    // Resistencia fisica: el lobo se salva con CON 1 (0,5) y falla.
    MOTOR_CHECK(p.jugar("Transmutar en plomo"));
    MOTOR_CHECK(tieneRasgo(p.situLobo(), T::Incapacitado));
    MOTOR_CHECK(p.lobo.health() == 60);
}

// --- Naturaleza y espiritus -------------------------------------------------------

MOTOR_TEST_CASE(druida_se_encomienda_al_sauce_y_la_savia_limpia_y_cura) {
    Mesa m("druida_del_roble");
    MOTOR_CHECK(m.fila("Elegir Roble") >= 0 && m.fila("Elegir Espino") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Sauce"));
    conMiedo(m);
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Savia viva"));
    MOTOR_CHECK(m.heroe.health() == 21);  // 2d6 = 10, y el sauce +1
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
    m.ronda();
    MOTOR_CHECK(m.jugar("Zarzal (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::DanoPorTurno));  // sangra
}

MOTOR_TEST_CASE(domador_llama_a_su_bestia_para_todo_el_combate) {
    Mesa m("domador_de_bestias", 1, 1);  // lobo con DES 1
    const RPG::ClassDefinition* c = clases().find("domador_de_bestias");
    MOTOR_CHECK(c->startingSkillIds.front() == "domador_de_bestias_llamar_a_la_bestia");
    MOTOR_CHECK(m.fila("Llamar a la bestia: Bestia domada") >= 0);
    MOTOR_CHECK(m.jugar("Llamar a la bestia"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    MOTOR_CHECK(m.batalla->allies()[1].name == "Bestia domada");
    MOTOR_CHECK(m.lobo.health() < 60);  // ataca al cerrar el turno
    for (int i = 0; i < 20; ++i) m.batalla->tickRound();
    MOTOR_CHECK(m.batalla->allies()[1].combatant->isAlive());  // todo el combate
}

MOTOR_TEST_CASE(chaman_de_los_elementos_planta_el_totem_y_la_piedra_lo_cubre) {
    Mesa m("chaman_de_los_elementos");
    MOTOR_CHECK(m.fila("Elegir Tierra") >= 0 && m.fila("Elegir Aire") >= 0);
    MOTOR_CHECK(m.jugar("Tótem abrasador"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    MOTOR_CHECK(m.batalla->allies()[1].name == "Tótem abrasador");
    m.ronda();
    MOTOR_CHECK(m.jugar("Tótem de piel de piedra"));
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCa, 1));
    MOTOR_CHECK(tieneRasgo(*m.cartas->situacion(true, 1), T::BonoCa, 1));  // tambien el totem
    for (int i = 0; i < 4; ++i) m.batalla->tickRound();
    MOTOR_CHECK(!m.batalla->allies()[1].combatant->isAlive());  // se apaga
}

MOTOR_TEST_CASE(chaman_de_los_espiritus_imbuye_fuego_y_da_dos_golpes) {
    Mesa m("chaman_de_los_espiritus");
    MOTOR_CHECK(m.fila("Elegir Viento furioso") >= 0 && m.fila("Elegir Escarcha") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Lengua de fuego"));
    MOTOR_CHECK(m.jugar("Ráfaga del vendaval"));
    MOTOR_CHECK(m.lobo.health() == 48);  // dos golpes de 5 + 1
    m.ronda();
    MOTOR_CHECK(m.jugar("Escudo de relámpagos (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoDanoArma));
}

// --- Tormenta, animas y Vinclap ---------------------------------------------------

MOTOR_TEST_CASE(tempestario_el_pararrayos_para_el_golpe_y_marca_al_atacante) {
    Mesa m("tempestario");
    MOTOR_CHECK(m.jugar("Pararrayos (reaccion)"));
    m.atacar();
    MOTOR_CHECK(m.lobo.health() == 54);  // 5 y la vara cargada +1
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoDanoContra));  // marcado
    m.atacar();
    MOTOR_CHECK(m.lobo.health() == 47);  // 5 + 1 + la marca
}

MOTOR_TEST_CASE(necromago_llama_a_un_alma_en_pena_tres_turnos) {
    Mesa m("necromago_de_las_animas", 1, 1);
    MOTOR_CHECK(m.jugar("Llamar a un alma en pena"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    MOTOR_CHECK(m.batalla->allies()[1].name == "Alma en pena");
    MOTOR_CHECK(m.lobo.health() < 60);
    for (int i = 0; i < 4; ++i) m.batalla->tickRound();
    MOTOR_CHECK(!m.batalla->allies()[1].combatant->isAlive());  // se desvanece
}

MOTOR_TEST_CASE(corsario_vampirico_el_mordisco_bebe_lo_que_quita) {
    Mesa m("corsario_vampirico");
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Mordisco"));
    const int quitado = 60 - m.lobo.health();
    MOTOR_CHECK(quitado > 0);
    MOTOR_CHECK(m.heroe.health() == 10 + quitado);
    m.ronda();
    MOTOR_CHECK(m.jugar("Forma de niebla (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::VentajaDefensa));
}

MOTOR_TEST_CASE(forajido_estocada_y_disparo_y_el_barril_de_ron_limpia) {
    Mesa m("forajido_de_vinclap");
    MOTOR_CHECK(m.jugar("Estocada y disparo"));
    MOTOR_CHECK(m.lobo.health() == 48);  // dos golpes de 5 + 1
    m.ronda();
    conMiedo(m);
    MOTOR_CHECK(m.jugar("Barril de ron (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
}

int main() { return motor_test::runAllTests(); }
