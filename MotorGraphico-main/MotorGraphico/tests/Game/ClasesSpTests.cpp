// SP-02 — Las SP de NosTale como clases nuevas: cada una carga con sus 7
// cartas, sus 3 de Creador y su pasiva, se elige al crear personaje, y sus
// cartas hacen en combate lo que dice su texto (los ganchos nuevos siempre:,
// propio: y limpia incluidos). Las reglas de la adaptacion, en
// planificacion/CANON_SP_CLASES.md; la carta, tools/importar_clases_sp.py.
//
// La mesa es la de H2-06 (tests/Game/CartasPrueba.h): dado fijo en 5, el
// heroe ataca con 6 dados (3 exitos) y el lobo se defiende con 5 (2,5); con
// DES 1 el lobo se defiende con un dado (0,5). Las cuentas, en
// tests/Game/CartasEnCombateTests.cpp.
#include "CartasPrueba.h"
#include "Game/EstadosCombate.h"
#include "RPG/CharacterCreation.h"
#include "RPG/ConditionEngine.h"
#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

// Las doce SP del Espadachin (la enciclopedia de Oriol, en su orden).
const char* const kEspadachin[] = {"guerrero",           "ninja",       "cruzado",
                                   "berserker",          "gladiador",   "monje_de_lucha",
                                   "cosechador_de_almas", "renegado",   "hachero_de_la_cascada",
                                   "caballero_draconiano", "romperrocas", "aquiles"};

// Las doce del Arquero.
const char* const kArquero[] = {"cazador",       "asesino",      "destructor",    "guardabosques",
                                "servidor_del_canon_de_fuego", "espia", "cazademonios", "angel_vengador",
                                "cazasoles",     "granadero",    "cazanieblas",   "almirante_yi"};

// Las doce del Mago.
const char* const kMago[] = {"mago_de_fuego", "mago_sagrado",        "mago_azul", "mago_de_las_tinieblas",
                             "volcano",       "senor_de_las_mareas", "vidente",   "archimago",
                             "sacerdote_vudu", "gravitas",           "tormenta_de_fuego", "merlin"};

// Las ocho del Artista Marcial.
const char* const kArtista[] = {"puno_del_dragon",  "artes_misticas",  "maestro_lobo", "guerrero_demoniaco",
                                "druida_flamigero", "puno_hidraulico", "tronador",     "thor"};

// Todas las SP hechas (las clases base, en su orden).
std::vector<const char*> todas() {
    std::vector<const char*> v(std::begin(kEspadachin), std::end(kEspadachin));
    v.insert(v.end(), std::begin(kArquero), std::end(kArquero));
    v.insert(v.end(), std::begin(kMago), std::end(kMago));
    v.insert(v.end(), std::begin(kArtista), std::end(kArtista));
    return v;
}

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

// Para las cartas que limpian: el heroe con miedo en la ficha (una condicion
// negativa del catalogo) y la batalla con el catalogo de condiciones.
void conMiedo(Mesa& m) {
    m.batalla->setConditionCatalog(&catalogo().condiciones());
    const RPG::ConditionDefinition* miedo = catalogo().condiciones().find("asustado");
    MOTOR_CHECK(miedo != nullptr && miedo->isNegative);
    if (miedo != nullptr) MOTOR_CHECK(RPG::ConditionEngine::apply(m.ficha, *miedo, catalogo().condiciones()).isOk());
}

bool tieneCondicion(const Mesa& m, const std::string& id) {
    for (const RPG::ActiveCondition& a : m.ficha.conditions)
        if (a.conditionId == id) return true;
    return false;
}

}  // namespace

MOTOR_TEST_CASE(cada_sp_es_una_clase_con_siete_cartas_tres_de_creador_y_su_pasiva) {
    RPG::Catalogs::SkillCatalog skills;
    RPG::Catalogs::PassiveCatalog pasivas;
    MOTOR_CHECK(skills.loadFromFile("assets/catalogs/skills.json").isOk());
    MOTOR_CHECK(pasivas.loadFromFile("assets/catalogs/passives.json").isOk());
    MOTOR_CHECK(todas().size() == 44);
    for (const char* id : todas()) {
        const RPG::ClassDefinition* c = clases().find(id);
        MOTOR_CHECK(c != nullptr);
        if (c == nullptr) continue;
        MOTOR_CHECK(c->learnableSkillIds.size() == 10);
        MOTOR_CHECK(c->startingSkillIds.size() == 3);
        MOTOR_CHECK(!c->passiveId.empty() && pasivas.find(c->passiveId) != nullptr);
        // Oriol (07/10): la clase no lleva su set; empieza con un arma normal de su tipo.
        MOTOR_CHECK(c->startingEquipmentIds.size() == 1);
        int creador = 0;
        for (const std::string& s : c->learnableSkillIds) {
            const RPG::SkillDefinition* d = skills.find(s);
            MOTOR_CHECK(d != nullptr);
            if (d == nullptr) continue;
            const bool esCreador = std::find(d->tags.begin(), d->tags.end(), "creador") != d->tags.end();
            creador += esCreador ? 1 : 0;
            // Jugable: tiene ganchos y ninguno queda sin entender.
            const Game::CatalogoCartas::Carta* carta = catalogo().carta(s);
            MOTOR_CHECK(carta != nullptr && !carta->ganchos.empty());
            if (carta != nullptr && !carta->bloqueo.empty())
                MOTOR_CHECK(carta->bloqueo.rfind("[pendiente]", 0) == 0 ||
                            carta->bloqueo == "necesita posiciones en el combate");
        }
        MOTOR_CHECK(creador == 3);
    }
    const auto reglas = RPG::CharacterCreation::load("assets/catalogs");
    MOTOR_CHECK(reglas.isOk());
    if (!reglas.isOk()) return;
    for (const char* id : todas()) MOTOR_CHECK(reglas.value().classes().find(id) != nullptr);
}

// «De momento un set entero por SP»: el arma y las cuatro piezas, raras de
// tier 2, con la etiqueta del set; ninguna es equipo inicial de nadie.
MOTOR_TEST_CASE(cada_sp_tiene_un_set_entero_que_no_es_equipo_inicial) {
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    const struct {
        const char* clase;
        const char* etiqueta;
    } kSets[] = {{"guerrero", "Set del Guerrero"},
                 {"ninja", "Set del Ninja"},
                 {"cruzado", "Set del Cruzado"},
                 {"berserker", "Set del Berserker"},
                 {"gladiador", "Set del Gladiador"},
                 {"monje_de_lucha", "Set del Monje de lucha"},
                 {"cosechador_de_almas", "Set del Cosechador de almas"},
                 {"renegado", "Set del Renegado"},
                 {"hachero_de_la_cascada", "Set del Hachero de la cascada"},
                 {"caballero_draconiano", "Set del Caballero Draconiano"},
                 {"romperrocas", "Set del Romperrocas"},
                 {"aquiles", "Set de Aquiles"},
                 {"cazador", "Set del Cazador"},
                 {"asesino", "Set del Asesino"},
                 {"destructor", "Set del Destructor"},
                 {"guardabosques", "Set del Guardabosques"},
                 {"servidor_del_canon_de_fuego", "Set del Cañonero"},  // set_de: el nombre corto
                 {"espia", "Set del Espía"},
                 {"cazademonios", "Set del Cazademonios"},
                 {"angel_vengador", "Set del Ángel vengador"},
                 {"cazasoles", "Set del Cazasoles"},
                 {"granadero", "Set del Granadero"},
                 {"cazanieblas", "Set del Cazanieblas"},
                 {"almirante_yi", "Set del Almirante Yi"},
                 {"mago_de_fuego", "Set del Mago de fuego"},
                 {"mago_sagrado", "Set del Mago sagrado"},
                 {"mago_azul", "Set del Mago azul"},
                 {"mago_de_las_tinieblas", "Set del Mago de las tinieblas"},
                 {"volcano", "Set de Volcano"},  // nombre propio, sin articulo
                 {"senor_de_las_mareas", "Set del Señor de las mareas"},
                 {"vidente", "Set del Vidente"},
                 {"archimago", "Set del Archimago"},
                 {"sacerdote_vudu", "Set del Sacerdote vudú"},
                 {"gravitas", "Set de Grávitas"},
                 {"tormenta_de_fuego", "Set de la Tormenta de fuego"},  // femenino
                 {"merlin", "Set de Merlín"},
                 {"puno_del_dragon", "Set del Puño del dragón"},
                 {"artes_misticas", "Set de las Artes Místicas"},  // femenino plural
                 {"maestro_lobo", "Set del Maestro Lobo"},
                 {"guerrero_demoniaco", "Set del Guerrero demoníaco"},
                 {"druida_flamigero", "Set del Druida flamígero"},
                 {"puno_hidraulico", "Set del Puño Hidráulico"},
                 {"tronador", "Set del Tronador"},
                 {"thor", "Set de Thor"}};
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

MOTOR_TEST_CASE(los_ganchos_nuevos_se_leen) {
    const Game::Gancho siempre = Game::leerGancho("siempre:bono_dano_arma:1");
    MOTOR_CHECK(siempre.tipo == T::Siempre);
    MOTOR_CHECK(siempre.rasgo.size() == 1 && siempre.rasgo[0].tipo == T::BonoDanoArma);
    const Game::Gancho propio = Game::leerGancho("propio:2:bono_ca:1");
    MOTOR_CHECK(propio.tipo == T::Propio && propio.rondas == 2);
    MOTOR_CHECK(propio.rasgo.size() == 1 && propio.rasgo[0].tipo == T::BonoCa && propio.rasgo[0].numero == 1);
    MOTOR_CHECK(Game::leerGancho("propio:combate:ventaja:ataque").rondas == -1);
    MOTOR_CHECK(Game::leerGancho("limpia").tipo == T::Limpia);
    // Lo que el combate no simula no vale como rasgo de una pasiva.
    MOTOR_CHECK(Game::leerGancho("siempre:velocidad_0").tipo == T::Desconocido);
    MOTOR_CHECK(Game::leerGancho("propio:2:nada").tipo == T::Desconocido);
    // A favor o en contra (el icono del estado y «limpia»).
    MOTOR_CHECK(Game::esPerjuicio(Game::leerGancho("penal_ca:1")));
    MOTOR_CHECK(!Game::esPerjuicio(Game::leerGancho("bono_ca:1")));
    MOTOR_CHECK(Game::esPerjuicio(Game::leerGancho("incapacitado")));
    MOTOR_CHECK(!Game::esPerjuicio(Game::leerGancho("ventaja:ataque")));
}

// --- Guerrero (SP1, fuego) -----------------------------------------------------

MOTOR_TEST_CASE(guerrero_espadon_a_dos_manos_suma_uno_a_cada_golpe) {
    Mesa m("guerrero");
    m.atacar();  // 3 contra 2,5: 1d6 = 5, y la pasiva +1
    MOTOR_CHECK(m.lobo.health() == 54);
    m.ronda();
    m.atacar();
    MOTOR_CHECK(m.lobo.health() == 48);
}

// Oriol (07/10): de arranque, «un provocar molt bàsic i fàcil d'esquivar».
// Un enemigo, con salvacion de defensa mental; nada de rigidez ni de CA.
MOTOR_TEST_CASE(guerrero_provocacion_basica_reta_a_uno_y_se_puede_esquivar) {
    Mesa m("guerrero", 1, 5, 2);  // dos lobos: solo el elegido
    const RPG::ClassDefinition* g = clases().find("guerrero");
    MOTOR_CHECK(std::find(g->startingSkillIds.begin(), g->startingSkillIds.end(), "guerrero_provocacion") !=
                g->startingSkillIds.end());
    // El lobo se salva con CAR 3 (1,5 contra 3) y falla.
    MOTOR_CHECK(m.jugar("Provocación"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Provocado));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Provocado) == 1);
    MOTOR_CHECK(!tieneRasgo(m.situLobo(), T::Incapacitado));
    MOTOR_CHECK(m.cartas->situacion(false, 1)->estados.empty());  // el otro lobo, nada
    MOTOR_CHECK(!tieneRasgo(*m.cartas->situacion(true, 0), T::BonoCa));
}

MOTOR_TEST_CASE(guerrero_provocacion_basica_falla_si_el_enemigo_se_resiste) {
    Mesa m("guerrero");
    m.dado.cara = 4;  // el Guerrero no saca exitos: el lobo se resiste
    MOTOR_CHECK(m.jugar("Provocación"));
    MOTOR_CHECK(!tieneRasgo(m.situLobo(), T::Provocado));
    MOTOR_CHECK(m.carta("guerrero_provocacion").gastada);  // la carta se juega igual
}

MOTOR_TEST_CASE(guerrero_intimidacion_es_magia_y_deja_shock_y_menos_ca) {
    Mesa m("guerrero");
    // Defensa mental: el lobo se salva con CAR 3 (1,5 contra 3) y falla.
    MOTOR_CHECK(m.jugar("Intimidación"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Incapacitado));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoCa, -1));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::BonoCa) == 3);
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
}

MOTOR_TEST_CASE(guerrero_piel_de_acero_da_ca_y_mas_dano_al_arma) {
    Mesa m("guerrero");
    MOTOR_CHECK(m.jugar("Piel de acero"));
    const Game::SituacionCombate& heroe = *m.cartas->situacion(true, 0);
    MOTOR_CHECK(rondasDe(heroe, T::BonoCa) == 3);
    MOTOR_CHECK(rondasDe(heroe, T::BonoDanoArma) == 5);
    m.ronda();
    m.atacar();  // 1d6 + 1 (pasiva) + 1 (Ira de hierro)
    MOTOR_CHECK(m.lobo.health() == 53);
}

MOTOR_TEST_CASE(guerrero_las_de_creador_esperan_al_tier_2) {
    Mesa t1("guerrero");
    MOTOR_CHECK(t1.fila("Terremoto (tier 2)") >= 0);
    MOTOR_CHECK(!t1.jugar("Terremoto"));

    Mesa t2("guerrero", 2, 1, 2);  // dos lobos con DES 1
    MOTOR_CHECK(t2.jugar("Terremoto"));
    MOTOR_CHECK(t2.lobo.health() < 60 && t2.lobo2.health() < 60);  // a todos
    MOTOR_CHECK(t2.lobo.health() == t2.lobo2.health());
    MOTOR_CHECK(tieneRasgo(t2.situLobo(), T::Incapacitado));
}

MOTOR_TEST_CASE(guerrero_estimulacion_da_ventaja_todo_el_combate) {
    Mesa m("guerrero", 2);
    MOTOR_CHECK(m.jugar("Estimulación"));
    const Game::SituacionCombate& heroe = *m.cartas->situacion(true, 0);
    MOTOR_CHECK(tieneRasgo(heroe, T::VentajaAtaque));
    MOTOR_CHECK(rondasDe(heroe, T::VentajaAtaque) == -1);
}

// Un estado que baja la CA va en contra de quien lo lleva (antes salia como
// «mejora» por ser un bono_ca en negativo).
MOTOR_TEST_CASE(un_penal_ca_se_ve_como_perjuicio) {
    Mesa m("guerrero", 1, 1);
    MOTOR_CHECK(m.jugar("Golpe último"));
    bool visto = false;
    for (const Game::EstadoVisible& e : Game::estadosDe(*m.batalla, false, 0)) {
        if (e.nombre != "Golpe último") continue;
        visto = true;
        MOTOR_CHECK(e.signo == Game::EstadoVisible::Signo::Contra);
        MOTOR_CHECK(e.icono == "guardia_rota");
    }
    MOTOR_CHECK(visto);
}

// --- El resto del Espadachin: lo propio de cada una ------------------------------
// Todas llevan la pasiva de +1 al arma (el Monje, su espiritu; el Angel
// vengador, su energia; el Puno del dragon y el Druida flamigero, su forma,
// que se elige en combate): el golpe basico contra el lobo es 1d6 = 5, y 1 mas.

MOTOR_TEST_CASE(todas_menos_las_de_forma_y_el_angel_suman_uno_al_golpe_con_arma) {
    const std::vector<std::string> deForma = {"monje_de_lucha", "angel_vengador", "puno_del_dragon",
                                              "druida_flamigero"};
    for (const char* id : todas()) {
        const std::string clase = id;
        if (std::find(deForma.begin(), deForma.end(), clase) != deForma.end()) continue;
        Mesa m(id);
        m.atacar();
        MOTOR_CHECK(m.lobo.health() == 54);
    }
    for (const char* id : {"monje_de_lucha", "angel_vengador", "puno_del_dragon", "druida_flamigero"}) {
        Mesa m(id);
        m.atacar();
        MOTOR_CHECK(m.lobo.health() == 55);
    }
}

// §3: las de arranque se pueden jugar. Si una de las tres primeras sale
// bloqueada, entra la siguiente jugable; el Cazasoles empieza con su lobo.
MOTOR_TEST_CASE(ninguna_carta_de_arranque_sale_bloqueada) {
    for (const char* id : todas()) {
        const RPG::ClassDefinition* c = clases().find(id);
        MOTOR_CHECK(c != nullptr);
        if (c == nullptr) continue;
        for (const std::string& s : c->startingSkillIds) {
            const Game::CatalogoCartas::Carta* carta = catalogo().carta(s);
            MOTOR_CHECK(carta != nullptr && carta->bloqueo.empty());
        }
    }
    MOTOR_CHECK((clases().find("destructor")->startingSkillIds ==
                 std::vector<std::string>{"destructor_tiro_rapido", "destructor_tiro_lejano",
                                          "destructor_bomba_de_gas_toxico"}));
    const std::vector<std::string>& cazasoles = clases().find("cazasoles")->startingSkillIds;
    MOTOR_CHECK(std::find(cazasoles.begin(), cazasoles.end(), "cazasoles_curacion_natural") != cazasoles.end());
}

// Primero se libra del miedo y despues se cura (con miedo, la cura saldria a
// medias: la tirada lleva su penalizacion).
MOTOR_TEST_CASE(ninja_recuperacion_espiritual_quita_el_miedo_y_cura) {
    Mesa m("ninja");
    conMiedo(m);
    MOTOR_CHECK(tieneCondicion(m, "asustado"));
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Recuperación espiritual"));
    MOTOR_CHECK(m.heroe.health() == 25);
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
}

MOTOR_TEST_CASE(ninja_protector_de_cuchillas_para_el_golpe_y_afila_la_espada) {
    Mesa m("ninja");
    MOTOR_CHECK(m.jugar("Protector de cuchillas (reaccion)"));
    m.atacar();  // 5 + 1
    m.ronda();   // el lobo ataca: la reaccion lo para y el heroe gana 1d6 al arma 2 rondas
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoDanoArma));
    m.atacar();  // 5 + 1 + 5
    MOTOR_CHECK(m.lobo.health() == 43);
}

MOTOR_TEST_CASE(cruzado_escudo_creciente_y_efecto_deslumbrador) {
    Mesa m("cruzado");
    MOTOR_CHECK(m.jugar("Escudo creciente"));
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCa, 1));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoCa) == 2);
    m.ronda();
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Efecto deslumbrador"));
    MOTOR_CHECK(m.heroe.health() == 25);
}

MOTOR_TEST_CASE(berserker_se_enfurece_pega_mas_y_se_cubre_menos) {
    Mesa m("berserker");
    MOTOR_CHECK(m.jugar("Berserker"));
    const Game::SituacionCombate& h = situHeroe(m);
    MOTOR_CHECK(tieneRasgo(h, T::VentajaAtaque));
    MOTOR_CHECK(tieneRasgo(h, T::BonoDanoArma));
    MOTOR_CHECK(tieneRasgo(h, T::BonoCa, -1));
    MOTOR_CHECK(rondasDe(h, T::BonoDanoArma) == -1);
    m.ronda();
    m.atacar();  // 5 + 1 + 1d6
    MOTOR_CHECK(m.lobo.health() == 49);
}

MOTOR_TEST_CASE(berserker_ruptura_de_la_armadura_es_magia_contra_el_cuerpo) {
    Mesa m("berserker");
    // Resistencia fisica: el lobo se salva con CON 1 (0,5 contra 3) y falla.
    MOTOR_CHECK(m.jugar("Ruptura de la armadura"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoCa, -2));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::BonoCa) == 3);
    MOTOR_CHECK(m.lobo.health() == 60);  // no hace dano
}

MOTOR_TEST_CASE(gladiador_gangrena_cura_y_da_ventaja_todo_el_combate) {
    Mesa m("gladiador", 2);  // de Creador
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Gangrena"));
    MOTOR_CHECK(m.heroe.health() == 25);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaAtaque) == -1);
}

MOTOR_TEST_CASE(monje_elige_su_espiritu_y_palabras_de_buda_limpian) {
    Mesa m("monje_de_lucha", 2);
    MOTOR_CHECK(m.fila("Elegir Fuerza") >= 0);
    MOTOR_CHECK(m.fila("Elegir Iluminado") >= 0);
    MOTOR_CHECK(m.fila("Elegir Mesura") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Iluminado"));
    MOTOR_CHECK(m.cartas->pasiva().elegida == "iluminado");
    MOTOR_CHECK(m.fila("Elegir Fuerza") < 0);  // una vez por combate
    conMiedo(m);
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Palabras de Buda"));
    MOTOR_CHECK(m.heroe.health() == 25);
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
}

MOTOR_TEST_CASE(cosechador_aspirasangre_cura_lo_que_quita) {
    Mesa m("cosechador_de_almas");
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Aspirasangre"));
    const int quitado = 60 - m.lobo.health();
    MOTOR_CHECK(quitado > 0);
    MOTOR_CHECK(m.heroe.health() == 10 + quitado);
}

MOTOR_TEST_CASE(cosechador_convertidor_temporal_devuelve_una_estandar) {
    Mesa m("cosechador_de_almas", 2);  // de Creador
    MOTOR_CHECK(m.jugar("Cuchilla"));
    MOTOR_CHECK(m.carta("cosechador_de_almas_cuchilla").gastada);
    m.ronda();
    MOTOR_CHECK(m.jugar("Convertidor temporal: Cuchilla"));
    MOTOR_CHECK(!m.carta("cosechador_de_almas_cuchilla").gastada);
    MOTOR_CHECK(m.cartas->turnoAbierto());  // accion libre
}

MOTOR_TEST_CASE(renegado_ignicion_suma_un_dado_al_arma) {
    Mesa m("renegado");
    MOTOR_CHECK(m.jugar("Ignición"));
    m.ronda();
    m.atacar();  // 5 + 1 + 1d6
    MOTOR_CHECK(m.lobo.health() == 49);
}

MOTOR_TEST_CASE(hachero_energia_runica_cura_y_da_ca_y_la_cascada_para_y_limpia) {
    Mesa m("hachero_de_la_cascada");
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Energía rúnica del agua"));
    MOTOR_CHECK(m.heroe.health() == 25);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoCa) == 5);
    m.ronda();
    conMiedo(m);
    MOTOR_CHECK(m.jugar("Protección de la cascada (reaccion)"));
    m.atacar();
    const int antes = m.heroe.health();
    m.ronda();  // el lobo ataca: parado, y fuera el miedo
    MOTOR_CHECK(m.heroe.health() == antes);
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
}

MOTOR_TEST_CASE(caballero_escudo_de_dragonio_para_un_ataque) {
    Mesa m("caballero_draconiano");
    MOTOR_CHECK(m.jugar("Escudo de Dragonio (reaccion)"));
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() < 30);
}

MOTOR_TEST_CASE(romperrocas_salto_con_cohete_da_ca_a_quien_salta) {
    Mesa m("romperrocas", 1, 1);
    MOTOR_CHECK(m.jugar("Salto con cohete"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCa, 1));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoCa) == 2);
}

MOTOR_TEST_CASE(romperrocas_respiracion_llameante_quema_cada_ronda) {
    Mesa m("romperrocas", 1, 1);
    MOTOR_CHECK(m.jugar("Respiración llameante"));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::DanoPorTurno));
    const int tras = m.lobo.health();
    m.ronda();
    MOTOR_CHECK(m.lobo.health() == tras - 1);
}

MOTOR_TEST_CASE(aquiles_niebla_impenetrable_le_cubre_y_marca_a_los_enemigos) {
    Mesa m("aquiles");
    MOTOR_CHECK(m.jugar("Niebla impenetrable"));
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::VentajaDefensa));
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::VentajaAtaque));
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoDanoContra));
    m.ronda();
    m.atacar();  // 5 + 1 + 1 (los golpes de Aquiles contra el marcado)
    MOTOR_CHECK(m.lobo.health() == 53);
}

// --- Arquero (SP-02): lo propio de cada una ----------------------------------------

MOTOR_TEST_CASE(cazador_ataque_de_falcon_da_ventaja_y_mas_dano_todo_el_combate) {
    Mesa m("cazador");
    MOTOR_CHECK(m.jugar("Ataque de falcón"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaAtaque) == -1);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoDanoArma) == -1);
    MOTOR_CHECK(m.fila("Corredor de viento (pendiente)") >= 0);  // velocidad: sin posiciones
    MOTOR_CHECK(!m.jugar("Corredor de viento"));
}

MOTOR_TEST_CASE(asesino_tambalearse_para_el_golpe_y_afila_el_punal) {
    Mesa m("asesino");
    MOTOR_CHECK(m.jugar("Tambalearse (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: la reaccion lo para y el heroe gana 1d6 al arma
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoDanoArma));
}

MOTOR_TEST_CASE(destructor_cura_milagrosa_limpia_y_refuerza_todo_el_combate) {
    Mesa m("destructor");
    conMiedo(m);
    MOTOR_CHECK(m.jugar("Cura milagrosa"));
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoCa) == -1);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoDanoArma) == -1);
    MOTOR_CHECK(!m.jugar("Colocabombas"));  // la ficha no publica el dano de la bomba
}

MOTOR_TEST_CASE(guardabosques_espiritu_de_lobo_da_un_dado_al_grupo_todo_el_combate) {
    Mesa m("guardabosques");
    MOTOR_CHECK(m.jugar("Espíritu de lobo"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoDanoArma) == -1);
    m.ronda();
    m.atacar();  // 5 + 1 + 1d6
    MOTOR_CHECK(m.lobo.health() == 49);
}

MOTOR_TEST_CASE(espia_emboscada_te_esconde_y_te_deja_atacar_con_ventaja) {
    Mesa m("espia", 2);  // de Creador
    MOTOR_CHECK(m.jugar("Emboscada"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaDefensa) == 2);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaAtaque) == 2);
}

MOTOR_TEST_CASE(cazademonios_hoja_de_tiro_cura_lo_que_quita_y_rompe_la_guardia) {
    Mesa m("cazademonios");
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Hoja de tiro"));
    const int quitado = 60 - m.lobo.health();
    MOTOR_CHECK(quitado > 0);
    MOTOR_CHECK(m.heroe.health() == 10 + quitado);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoCa, -2));
}

// La energia del Angel se elige una vez por combate, como el espiritu del
// Monje, y dura poco desde que se elige.
MOTOR_TEST_CASE(angel_vengador_elige_hielo_y_su_espada_pega_un_dado_mas) {
    Mesa m("angel_vengador");
    MOTOR_CHECK(m.fila("Elegir Llama") >= 0);
    MOTOR_CHECK(m.fila("Elegir Sin elemento") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Hielo"));
    MOTOR_CHECK(m.cartas->pasiva().elegida == "hielo");
    MOTOR_CHECK(m.fila("Elegir Llama") < 0);  // una vez por combate
    m.atacar();  // 5 + 1d6 del Hielo
    MOTOR_CHECK(m.lobo.health() == 50);
}

MOTOR_TEST_CASE(angel_vengador_llama_devuelve_los_golpes_fisicos) {
    Mesa m("angel_vengador");
    MOTOR_CHECK(m.jugar("Elegir Llama"));
    m.atacar();  // 5
    m.ronda();   // el lobo ataca y su golpe rebota
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.lobo.health() < 55);
}

MOTOR_TEST_CASE(cazasoles_curacion_natural_trae_al_lobo_solar_todo_el_combate) {
    Mesa m("cazasoles", 1, 1);  // lobo con DES 1: se defiende con un dado
    conMiedo(m);
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.fila("Curación natural: Lobo solar") >= 0);
    MOTOR_CHECK(m.jugar("Curación natural"));
    MOTOR_CHECK(m.heroe.health() == 25);  // primero se libra del miedo y despues se cura 1d6
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    MOTOR_CHECK(m.batalla->allies()[1].name == "Lobo solar");
    MOTOR_CHECK(m.lobo.health() < 60);  // al cerrar el turno, el Lobo solar ataca
    for (int i = 0; i < 20; ++i) m.batalla->tickRound();
    MOTOR_CHECK(m.batalla->allies()[1].combatant->isAlive());  // no se va: todo el combate
}

MOTOR_TEST_CASE(granadero_escudo_energetico_para_el_golpe_y_da_ca) {
    Mesa m("granadero");
    MOTOR_CHECK(m.jugar("Escudo energético de Dragonio (reaccion)"));
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoCa, 1));
    MOTOR_CHECK(m.fila("Granada gravitatoria (pendiente)") >= 0);  // sujetar: sin posiciones
}

MOTOR_TEST_CASE(cazanieblas_bloqueo_de_vision_ciega_y_te_refuerza_todo_el_combate) {
    Mesa m("cazanieblas", 2);  // de Creador
    MOTOR_CHECK(m.jugar("Bloqueo de visión"));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DesventajaAtaque) == 2);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DesventajaDefensa) == 2);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaAtaque) == -1);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoDanoArma) == -1);
}

MOTOR_TEST_CASE(almirante_yi_doble_marea_atrae_y_deja_grogui) {
    Mesa m("almirante_yi");
    MOTOR_CHECK(m.jugar("Doble marea"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::Provocado));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DesventajaAtaque) == 1);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DesventajaDefensa) == 1);
}

// --- Mago (SP-02): lo propio de cada una -------------------------------------------
// §7: con que se tira lo dice la ficha de NosTale. El heroe de la mesa tiene
// INT 1 (dos dados: 1 exito, que no pasa los 2,5 del lobo); con INT 5 tira seis,
// como con DES.

MOTOR_TEST_CASE(las_cartas_del_mago_tiran_int_salvo_las_del_arma_magica) {
    for (const char* id : todas()) {
        const RPG::ClassDefinition* c = clases().find(id);
        MOTOR_CHECK(c != nullptr);
        if (c == nullptr) continue;
        const std::string clase = id;
        const bool mago = std::find_if(std::begin(kMago), std::end(kMago), [&](const char* m) {
                              return clase == m;
                          }) != std::end(kMago);
        for (const std::string& s : c->learnableSkillIds) {
            const Game::CatalogoCartas::Carta* carta = catalogo().carta(s);
            MOTOR_CHECK(carta != nullptr);
            if (carta == nullptr) continue;
            // El Espadachin y el Arquero, DES (DANO-02); los magos, INT, salvo
            // los que disparan el arma magica y el lanzallamas de la Tormenta.
            RPG::Stat esperada = mago ? RPG::Stat::INT : RPG::Stat::DES;
            if (clase == "mago_de_las_tinieblas" || clase == "merlin") esperada = RPG::Stat::DES;
            if (clase == "tormenta_de_fuego")
                esperada = s == "tormenta_de_fuego_circulo_de_fuego" || s == "tormenta_de_fuego_remolino_llameante"
                               ? RPG::Stat::INT
                               : RPG::Stat::DES;
            MOTOR_CHECK(carta->def->casting_stat == esperada);
        }
    }
}

MOTOR_TEST_CASE(mago_de_fuego_tira_int_y_su_aliento_quema_cada_ronda) {
    Mesa torpe("mago_de_fuego");  // INT 1: 1 exito contra 2,5
    MOTOR_CHECK(torpe.jugar("Aliento de fuego"));
    MOTOR_CHECK(torpe.lobo.health() == 60);
    MOTOR_CHECK(!tieneRasgo(torpe.situLobo(), T::DanoPorTurno));

    Mesa m("mago_de_fuego");
    m.heroe.ponerStat(RPG::Stat::INT, 5);  // seis dados: 3 contra 2,5
    MOTOR_CHECK(m.jugar("Aliento de fuego"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DanoPorTurno) == 3);
    const int tras = m.lobo.health();
    m.ronda();
    MOTOR_CHECK(m.lobo.health() == tras - 1);
}

MOTOR_TEST_CASE(mago_sagrado_efecto_cura_limpia_y_la_cura_de_grupo_cura_dos_dados) {
    Mesa m("mago_sagrado");
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    conMiedo(m);
    MOTOR_CHECK(m.jugar("Efecto cura"));
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
    m.ronda();
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Cura de grupo"));
    MOTOR_CHECK(m.heroe.health() == 20);  // 2d6 = 10
    MOTOR_CHECK(m.fila("Tótem sagrado (pendiente)") >= 0);  // la ficha no dice que hace el totem
    MOTOR_CHECK(!m.jugar("Tótem sagrado"));
}

MOTOR_TEST_CASE(mago_azul_cadena_helada_deja_al_lobo_dos_turnos_sin_atacar) {
    Mesa m("mago_azul");
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(m.jugar("Cadena helada"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 2);
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);  // helado: no ataca
}

MOTOR_TEST_CASE(mago_de_las_tinieblas_dispara_con_des_y_la_posesion_es_magia) {
    Mesa m("mago_de_las_tinieblas");  // INT 1: sus disparos tiran DES 5
    MOTOR_CHECK(m.jugar("Tiro combinado"));
    MOTOR_CHECK(m.lobo.health() < 60);
    m.ronda();
    // Defensa mental: el lobo se salva con CAR 3 (1,5 contra 3) y falla.
    MOTOR_CHECK(m.jugar("Posesión"));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 2);
}

MOTOR_TEST_CASE(volcano_tanque_de_magma_cubre_todo_el_combate_y_mas_dos_turnos) {
    Mesa m("volcano");
    MOTOR_CHECK(m.jugar("Tanque de magma"));
    std::vector<int> rondas;
    for (const Game::EstadoCarta& e : situHeroe(m).estados)
        for (const Game::Gancho& g : e.rasgos)
            if (g.tipo == T::BonoCa && g.numero == 1) rondas.push_back(e.rondas);
    std::sort(rondas.begin(), rondas.end());
    MOTOR_CHECK((rondas == std::vector<int>{-1, 2}));  // el escudo de magma y el tanque
}

MOTOR_TEST_CASE(senor_de_las_mareas_el_canto_de_las_sirenas_duerme_tres_turnos) {
    Mesa m("senor_de_las_mareas");
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    // Defensa mental: el lobo se salva con CAR 3 (1,5 contra 3) y falla.
    MOTOR_CHECK(m.jugar("El canto de las sirenas"));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 3);
    MOTOR_CHECK(m.lobo.health() == 60);  // no hace dano
    MOTOR_CHECK(m.fila("Remolino gigante (pendiente)") >= 0);  // atrapar y frenar: sin posiciones
}

MOTOR_TEST_CASE(vidente_superabracadabra_trae_cuatro_clones_que_atacan_y_se_van) {
    Mesa m("vidente", 2, 1);  // de Creador; el lobo, con DES 1
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(m.fila("Súperabracadabra: Clon oscuro") >= 0);
    MOTOR_CHECK(m.jugar("Súperabracadabra"));
    MOTOR_CHECK(m.batalla->allies().size() == 5);
    MOTOR_CHECK(m.batalla->allies()[4].name == "Clon oscuro");
    MOTOR_CHECK(m.lobo.health() < 60);  // al cerrar el turno, los clones atacan
    m.ronda();
    for (std::size_t i = 1; i < m.batalla->allies().size(); ++i)
        MOTOR_CHECK(!m.batalla->allies()[i].combatant->isAlive());  // 1 turno: se desvanecen
}

MOTOR_TEST_CASE(archimago_catalizador_marca_y_ciega_y_dos_de_creador_esperan_cifras) {
    Mesa m("archimago", 2);
    m.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(m.jugar("Catalizador lumínico"));
    MOTOR_CHECK(rondasDe(m.situLobo(), T::DesventajaAtaque) == 3);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoDanoContra));  // marcado
    MOTOR_CHECK(m.fila("Teleportador en masa (pendiente)") >= 0);
    MOTOR_CHECK(m.fila("Tormenta de meteoritos (pendiente)") >= 0);
}

MOTOR_TEST_CASE(sacerdote_vudu_tres_aranas_atacan_una_vez_y_estallan) {
    Mesa m("sacerdote_vudu", 1, 1);  // el lobo, con DES 1
    MOTOR_CHECK(m.jugar("Araña espiritual"));
    MOTOR_CHECK(m.batalla->allies().size() == 4);
    MOTOR_CHECK(m.lobo.health() < 60);
    m.ronda();
    for (std::size_t i = 1; i < m.batalla->allies().size(); ++i)
        MOTOR_CHECK(!m.batalla->allies()[i].combatant->isAlive());
}

MOTOR_TEST_CASE(gravitas_atrae_y_su_reflexion_para_el_golpe_y_lo_devuelve) {
    Mesa a("gravitas");
    a.heroe.ponerStat(RPG::Stat::INT, 5);
    MOTOR_CHECK(a.jugar("Atracción"));
    MOTOR_CHECK(tieneRasgo(a.situLobo(), T::Provocado));

    Mesa m("gravitas", 2);  // de Creador
    MOTOR_CHECK(m.jugar("Reflexión por antigravedad (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: la reaccion lo para y el heroe refleja los golpes 2 rondas
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::Refleja));
}

MOTOR_TEST_CASE(tormenta_de_fuego_el_circulo_es_magia_y_el_lanzallamas_dispara_con_des) {
    Mesa m("tormenta_de_fuego");  // DES 5, INT 1
    MOTOR_CHECK(m.jugar("Infierno llameante reforzado"));  // DES: 3 contra 2,5
    const int tras = m.lobo.health();
    MOTOR_CHECK(tras < 60);
    m.ronda();
    MOTOR_CHECK(m.jugar("Círculo de fuego"));  // INT 1: 1 contra 2,5
    MOTOR_CHECK(m.lobo.health() == tras);
    MOTOR_CHECK(!tieneRasgo(m.situLobo(), T::Incapacitado));
}

MOTOR_TEST_CASE(merlin_adivinacion_para_el_golpe_y_te_deja_atacar_con_ventaja) {
    Mesa m("merlin");
    MOTOR_CHECK(m.jugar("Adivinación (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: la reaccion lo para
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::VentajaAtaque));
}

// --- Artista Marcial (SP-02): lo propio de cada una -------------------------------
// Todas sus tecnicas son cuerpo a cuerpo o a distancia: tiran DES (lo comprueba
// las_cartas_del_mago_tiran_int_salvo_las_del_arma_magica). Empiezan con los
// guanteletes de Viaje (comunes, tier 1; «A dos manos» en sus restricciones).

MOTOR_TEST_CASE(el_artista_marcial_empieza_con_los_guanteletes_de_viaje) {
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    const RPG::EquipmentDefinition* guantes = equipo.find("guanteletes_de_viaje");
    MOTOR_CHECK(guantes != nullptr);
    if (guantes == nullptr) return;
    MOTOR_CHECK(guantes->slot == "MainHand");
    MOTOR_CHECK(guantes->rarity == RPG::ItemRarity::Common);
    MOTOR_CHECK(guantes->tier == 1);
    for (const char* id : kArtista) {
        const RPG::ClassDefinition* c = clases().find(id);
        MOTOR_CHECK(c != nullptr);
        if (c != nullptr) MOTOR_CHECK((c->startingEquipmentIds == std::vector<std::string>{"guanteletes_de_viaje"}));
    }
}

MOTOR_TEST_CASE(puno_del_dragon_elige_su_estilo_y_el_agarre_deja_sin_atacar) {
    Mesa m("puno_del_dragon");
    MOTOR_CHECK(m.fila("Elegir Haetae") >= 0);
    MOTOR_CHECK(m.fila("Elegir Dragon") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Dragon"));
    MOTOR_CHECK(m.cartas->pasiva().elegida == "dragon");
    m.ronda();
    m.atacar();
    MOTOR_CHECK(m.lobo.health() == 54);  // estilo dragon: +1 al golpe
    m.ronda();
    const int antes = m.lobo.health();
    MOTOR_CHECK(m.jugar("Agarre"));
    MOTOR_CHECK(m.lobo.health() < antes);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 1);
    MOTOR_CHECK(tieneRasgo(m.situLobo(), T::BonoDanoContra));
}

MOTOR_TEST_CASE(artes_misticas_aguante_para_el_golpe_y_la_luz_lunar_da_ventaja) {
    Mesa m("artes_misticas", 2);  // Aguante es de Creador
    MOTOR_CHECK(m.jugar("Aguante (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: la reaccion lo para
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.jugar("Entrada de luz lunar"));
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::VentajaAtaque));
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::BonoDanoArma));
}

MOTOR_TEST_CASE(maestro_lobo_el_estilo_definitivo_da_ventaja_dos_turnos_y_la_patada_noquea) {
    Mesa m("maestro_lobo", 2);
    MOTOR_CHECK(m.jugar("Estilo de combate definitivo"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::VentajaAtaque) == 2);
    m.ronda();
    MOTOR_CHECK(m.jugar("Patada con salto"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 1);
}

MOTOR_TEST_CASE(guerrero_demoniaco_contraalas_para_el_golpe_y_lo_devuelve) {
    Mesa m("guerrero_demoniaco");
    MOTOR_CHECK(m.jugar("Contraalas (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: la reaccion lo para y el heroe refleja los golpes 2 rondas
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(tieneRasgo(situHeroe(m), T::Refleja));
}

MOTOR_TEST_CASE(druida_flamigero_elige_figura_y_la_llama_sanadora_limpia_y_cura) {
    Mesa m("druida_flamigero", 2);
    MOTOR_CHECK(m.fila("Elegir Druida") >= 0);
    MOTOR_CHECK(m.fila("Elegir Oso") >= 0);
    MOTOR_CHECK(m.fila("Elegir Leopardo") >= 0);
    MOTOR_CHECK(m.jugar("Elegir Oso"));
    MOTOR_CHECK(m.cartas->pasiva().elegida == "oso");
    m.ronda();
    conMiedo(m);
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Llama sanadora"));
    MOTOR_CHECK(!tieneCondicion(m, "asustado"));
    MOTOR_CHECK(m.heroe.health() == 20);  // 2d6 = 10
}

MOTOR_TEST_CASE(puno_hidraulico_tanquear_espera_al_combustible_y_el_protocolo_cura) {
    Mesa m("puno_hidraulico");
    MOTOR_CHECK(m.fila("Tanquear (pendiente)") >= 0);
    MOTOR_CHECK(!m.jugar("Tanquear"));
    m.heroe.ponerVida(10);
    MOTOR_CHECK(m.jugar("Protocolo de recuperación"));
    MOTOR_CHECK(m.heroe.health() == 15);  // 1d6 = 5
    MOTOR_CHECK((clases().find("puno_hidraulico")->startingSkillIds ==
                 std::vector<std::string>{"puno_hidraulico_ataque_en_salto", "puno_hidraulico_machacar",
                                          "puno_hidraulico_protocolo_de_recuperacion"}));
}

MOTOR_TEST_CASE(tronador_la_onda_electrica_deja_sin_atacar_y_le_cubre_dos_turnos) {
    Mesa m("tronador");
    MOTOR_CHECK(m.fila("Amplificador de batería (pendiente)") >= 0);  // las marcas no se simulan
    MOTOR_CHECK(m.jugar("Onda eléctrica"));
    MOTOR_CHECK(m.lobo.health() < 60);
    MOTOR_CHECK(rondasDe(m.situLobo(), T::Incapacitado) == 1);
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoCa) == 2);
}

MOTOR_TEST_CASE(thor_escudo_helado_para_el_golpe_y_el_trueno_refuerza_al_grupo) {
    Mesa m("thor", 2);  // Trueno creciente es de Creador
    MOTOR_CHECK(m.jugar("Escudo helado (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo ataca: la reaccion lo para
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(m.jugar("Trueno creciente"));
    MOTOR_CHECK(rondasDe(situHeroe(m), T::BonoDanoArma) == -1);  // todo el combate
}

int main() { return motor_test::runAllTests(); }
