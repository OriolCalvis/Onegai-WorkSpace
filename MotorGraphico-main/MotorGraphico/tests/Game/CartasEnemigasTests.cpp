// AV18-06 — Los enemigos juegan las cartas nativas de su repertorio (CartasEnemigas.cpp): las de
// monstruos_boundington_cartas.json, cargadas de skills.json por CatalogoCartas. Cada turno, la
// siguiente; cada una hace lo que dice su gancho.
//
// Las cuentas, con el dado fijo en 5 (medio exito por dado, ver CartasEnCombateTests.cpp):
//   heroe     CON 1, DES 1 -> se defiende con 1 dado: 0,5;
//   enemigo   DES 5         -> tira 6 dados: 3. Gana siempre (exito, 3 contra 0,5).
// El repertorio va en el orden de SkillSet::knownSkillIds() (alfabetico): _control, _golpe e
// _impulso. Las cartas son las publicadas: si alguien las cambia, esto lo dice.
#include "CartasPrueba.h"
#include "TestHarness.h"

#include <string>

namespace {

// El heroe (con las cartas de una clase, para que haya mesa) contra un enemigo de Boundington
// que solo conoce su repertorio nativo: nada del catalogo legado.
struct MesaEnemiga {
    RPG::CharacterSheet ficha;
    Luchador heroe{40, 1, 1};
    Luchador malo{60, 1, 5};
    SkillSet repertorio{0};
    SkillCatalog legado;
    DadoFijo dado{5};
    std::unique_ptr<BattleState> batalla;
    Game::CartasEnCombate* cartas = nullptr;

    MesaEnemiga(const char* nombre, std::initializer_list<const char*> ids) {
        const RPG::ClassDefinition* def = clases().find("bardo_de_las_cuatro_estaciones");
        ficha.knownSkillIds = def->startingSkillIds;
        ficha.passiveIds = {def->passiveId};
        ficha.tier = 1;
        heroe.ficha = &ficha;
        for (const char* id : ids) repertorio.learn(id);
        batalla = std::make_unique<BattleState>(
            std::vector<BattleParticipant>{{"Heroe", &heroe, nullptr, nullptr}},
            std::vector<BattleParticipant>{{nombre, &malo, &repertorio, nullptr}}, &legado);
        batalla->setRandomEngine(dado);
        auto mesa = Game::CartasEnCombate::paraHeroe(catalogo(), heroe);
        cartas = mesa.get();
        batalla->setMesa(std::move(mesa));
    }

    // El turno del enemigo; true si en lo que ha escrito sale "texto".
    bool turno(const std::string& texto) {
        const std::size_t antes = batalla->log().size();
        batalla->resolveEnemyTurn();
        bool sale = false;
        for (std::size_t i = antes; i < batalla->log().size(); ++i) sale |= batalla->log()[i].find(texto) != std::string::npos;
        return sale;
    }

    const Game::EstadoCarta* estado(bool aliado, const std::string& id) const {
        for (const Game::EstadoCarta& e : cartas->situacion(aliado, 0)->estados) {
            if (e.id == id) return &e;
        }
        return nullptr;
    }
};

bool tieneRasgo(const Game::EstadoCarta& e, T tipo) {
    for (const Game::Gancho& g : e.rasgos) {
        if (g.tipo == tipo) return true;
    }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(un_enemigo_juega_dos_cartas_distintas_de_su_repertorio_con_sus_efectos) {
    MesaEnemiga m("Perdido saqueador", {"bnd_enemigo_perdido_saqueador_golpe", "bnd_enemigo_perdido_saqueador_control"});
    MOTOR_CHECK(m.cartas != nullptr);

    // Turno 1: Corte sin limpiar (condicion:sangrado:1). Gana la tirada y deja Sangrado en el
    // heroe, sin quitarle vida todavia; dura hasta el final de la ronda siguiente (1 + 1).
    MOTOR_CHECK(m.turno("usa Corte sin limpiar contra Heroe"));
    const Game::EstadoCarta* sangrado = m.estado(true, "sangrado");
    MOTOR_CHECK(sangrado != nullptr);
    MOTOR_CHECK(sangrado != nullptr && sangrado->rondas == 2 && !sangrado->origenAliado);
    MOTOR_CHECK(sangrado != nullptr && tieneRasgo(*sangrado, T::DanoPorTurno));
    MOTOR_CHECK(m.heroe.health() == 40);

    // Fin de ronda: el sangrado hace su 1 y sigue (le queda una ronda).
    m.batalla->tickRound();
    MOTOR_CHECK(m.heroe.health() == 39);
    MOTOR_CHECK(m.estado(true, "sangrado") != nullptr);

    // Turno 2: la otra carta, Cuchillada del saqueador (ataque_arma): golpe de arma con DANO-02.
    MOTOR_CHECK(m.turno("usa Cuchillada del saqueador contra Heroe"));
    MOTOR_CHECK(m.heroe.health() < 39);

    // Y se ve: dos cartas jugadas, una por turno, las dos sobre el heroe.
    const auto& jugadas = m.cartas->cartasJugadas();
    MOTOR_CHECK(jugadas.size() == 2);
    MOTOR_CHECK(jugadas.size() == 2 && jugadas[0].carta == "bnd_enemigo_perdido_saqueador_control" &&
                jugadas[1].carta == "bnd_enemigo_perdido_saqueador_golpe");
    MOTOR_CHECK(jugadas.size() == 2 && !jugadas[0].quien.aliado && jugadas[0].sobre.size() == 1 && jugadas[0].sobre[0].aliado);
}

MOTOR_TEST_CASE(gorran_se_turna_las_tres_y_su_impulso_le_da_ventaja_mientras_dura) {
    MesaEnemiga m("Gorran", {"bnd_enemigo_gorran_bandido_golpe", "bnd_enemigo_gorran_bandido_control",
                             "bnd_enemigo_gorran_bandido_impulso"});
    MOTOR_CHECK(m.turno("usa Finta del bandido contra Heroe"));
    const Game::EstadoCarta* guardia = m.estado(true, "guardia_rota");
    MOTOR_CHECK(guardia != nullptr && guardia->rondas == 2);
    m.batalla->tickRound();
    MOTOR_CHECK(m.turno("usa Tajo de Gorran contra Heroe"));
    m.batalla->tickRound();

    // propio:2:ventaja:ataque, sobre el mismo: sin tirada, 2 + 1 rondas.
    MOTOR_CHECK(m.turno("Gorran usa Tomar la iniciativa"));
    const Game::EstadoCarta* impulso = m.estado(false, "bnd_enemigo_gorran_bandido_impulso#propio");
    MOTOR_CHECK(impulso != nullptr && impulso->rondas == 3 && tieneRasgo(*impulso, T::VentajaAtaque));
    m.batalla->tickRound();

    // Vuelve a empezar por la Finta; y el impulso, que aun le dura, no se repite: tras la Finta,
    // el Tajo, y luego otra vez la Finta en lugar del impulso.
    MOTOR_CHECK(m.turno("usa Finta del bandido"));
    m.batalla->tickRound();
    MOTOR_CHECK(m.estado(false, "bnd_enemigo_gorran_bandido_impulso#propio") != nullptr);
    MOTOR_CHECK(m.turno("usa Tajo de Gorran"));
    MOTOR_CHECK(m.estado(false, "bnd_enemigo_gorran_bandido_impulso#propio") != nullptr);
    MOTOR_CHECK(!m.turno("usa Tomar la iniciativa"));
}
