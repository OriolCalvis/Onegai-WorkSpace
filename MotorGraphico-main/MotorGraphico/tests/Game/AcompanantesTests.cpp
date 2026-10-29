// ACOMPANANTES-01 — Los acompañantes de la historia en la mesa (Venides,
// Xila): actuan solos tras el turno del heroe, los enemigos van a por quien
// les ha hecho daño, a Xila no se le ataca y cura a quien esta en apuros,
// Venides se retira a 0 PV, y si cae el heroe la batalla se pierde.
//
// El dado es fijo en 5 (medio exito por dado) y las cuentas salen a mano:
//   heroe   CON 2, DES 5: ataca con 6 dados (3) y defiende con 5 (2,5);
//   Venides CON 3, DES 2: ataca con 3 dados (1,5) y defiende con 3 (1,5);
//   lobos   CON 0, DES 0: atacan con 1 dado (0,5) y no defienden (0).
// Un heroe o un Venides sin atributos (1 dado, sin defensa) recibe todos
// los golpes de los lobos.
#include "CartasPrueba.h"
#include "Game/BarraCombate.h"  // AV13-28
#include "Game/Acompanante.h"
#include "Game/Refuerzos.h"  // ROSITA-01
#include "Game/TableroCombate.h"  // AV13-29
#include "RPG/Narrative/NarrativeState.h"
#include "RPG/Narrative/AdventureScript.h"
#include "TestHarness.h"

#include <array>
#include <string>
#include <vector>

namespace {

RPG::AcompananteDef venides(int vida = 16, bool flojo = false) {
    RPG::AcompananteDef a;
    a.id = "venides";
    a.nombre = "Venides";
    a.vida = vida;
    a.stats = flojo ? std::array<int, 4>{0, 0, 0, 0} : std::array<int, 4>{3, 2, 0, 1};
    a.ataques = 2;
    a.seRetira = true;
    return a;
}

RPG::AcompananteDef xila() {
    RPG::AcompananteDef a;
    a.id = "xila";
    a.nombre = "Xila";
    a.vida = 6;
    a.stats = {1, 2, 0, 2};
    a.ataques = 0;
    a.atacable = false;
    a.cura = {4, 1, 2, 0.34f};
    return a;
}

// Un heroe bardo con su mesa, sus acompañantes detras y dos lobos de 60 PV.
struct Grupo {
    RPG::CharacterSheet ficha;
    Luchador heroe;
    Luchador lobo;
    Luchador lobo2;
    SkillCatalog legado;
    DadoFijo dado{5};
    std::unique_ptr<BattleState> batalla;
    Game::CartasEnCombate* cartas = nullptr;

    explicit Grupo(const std::vector<RPG::AcompananteDef>& aLado, bool heroeFlojo = false)
        : heroe(30, heroeFlojo ? 0 : 2, heroeFlojo ? 0 : 5), lobo(60, 0, 0), lobo2(60, 0, 0) {
        const RPG::ClassDefinition* def = clases().find("bardo_de_las_cuatro_estaciones");
        ficha.knownSkillIds = def->learnableSkillIds;
        ficha.passiveIds = {def->passiveId};
        ficha.tier = 1;
        heroe.ficha = &ficha;
        auto mesa = Game::CartasEnCombate::paraHeroe(catalogo(), heroe);
        cartas = mesa.get();
        std::vector<BattleParticipant> aliados{{"Heroe", &heroe, nullptr, nullptr}};
        for (const RPG::AcompananteDef& a : aLado)
            aliados.push_back({a.nombre, mesa->sumaAcompanante(a, a.cura.veces), nullptr, nullptr});
        batalla = std::make_unique<BattleState>(
            std::move(aliados),
            std::vector<BattleParticipant>{{"Lobo", &lobo, nullptr, nullptr}, {"Lobo gris", &lobo2, nullptr, nullptr}},
            &legado);
        batalla->setRandomEngine(dado);
        batalla->setMesa(std::move(mesa));
    }
    void atacar(int objetivo = 0) {
        batalla->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", objetivo, ""});
    }
    void ronda() {
        batalla->resolveEnemyTurn();
        batalla->tickRound();
    }
    int enLog(const std::string& texto) const {
        int n = 0;
        for (const std::string& l : batalla->log()) n += l.find(texto) != std::string::npos ? 1 : 0;
        return n;
    }
    const Game::Acompanante& aliado(std::size_t i) const {
        return *static_cast<const Game::Acompanante*>(batalla->allies()[i].combatant);
    }
};

}  // namespace

MOTOR_TEST_CASE(venides_golpea_dos_veces_al_ultimo_enemigo_tras_el_heroe) {
    Grupo g({venides()});
    MOTOR_CHECK(g.batalla->allies().size() == 2);
    g.atacar(0);
    // El heroe acierta al primero (3 contra 0: 3d6) y Venides, solo, dos veces
    // al ultimo (1,5 contra 0: 2d6 y 2d6). Los enemigos aun no han actuado.
    MOTOR_CHECK(g.lobo.health() == 45);
    MOTOR_CHECK(g.lobo2.health() == 40);
    MOTOR_CHECK(g.enLog("Venides ataca a Lobo gris") == 2);
    // Sus dos golpes salen como una accion aparte de la del heroe.
    const auto& t = g.cartas->tiradas();
    MOTOR_CHECK(t.size() == 3);
    if (t.size() == 3) {
        MOTOR_CHECK(t[0].quien == "Heroe" && t[1].quien == "Venides" && t[2].quien == "Venides");
        MOTOR_CHECK(t[0].accion != t[1].accion && t[1].accion == t[2].accion);
    }
    // AV14-01: la batalla lo apunta como golpes, con quien pego segun su tirada (los
    // dos de Venides, juntos: un golpe por quien lo recibe y por accion).
    const auto& golpes = g.batalla->golpes();
    MOTOR_CHECK(golpes.size() == 2);
    if (golpes.size() == 2) {
        MOTOR_CHECK(golpes[0].a == "Lobo" && golpes[0].de == "Heroe" && golpes[0].cambio == -15 && !golpes[0].aAliado);
        MOTOR_CHECK(golpes[1].a == "Lobo gris" && golpes[1].de == "Venides" && golpes[1].cambio == -20);
        MOTOR_CHECK(golpes[0].grado != Game::GolpeCombate::Grado::Fallo && golpes[0].que == g.cartas->tiradas()[0].que && !golpes[0].que.empty());
    }
}

MOTOR_TEST_CASE(cada_enemigo_va_a_por_quien_le_hizo_dano) {
    Grupo g({venides()});
    g.atacar(0);
    g.ronda();
    // El lobo al que pego el heroe va a por el; el que golpeo Venides, a por
    // Venides. Los dos fallan (0,5 contra 2,5 y contra 1,5).
    MOTOR_CHECK(g.enLog("Lobo ataca a Heroe") == 1);
    MOTOR_CHECK(g.enLog("Lobo gris ataca a Venides") == 1);
    MOTOR_CHECK(g.heroe.health() == 30 && g.aliado(1).health() == 16);
    // Si el heroe le pega al gris, el gris vuelve a por el heroe... hasta que
    // Venides le pega otra vez: va a por el ultimo que le hizo dano.
    g.atacar(1);
    g.ronda();
    MOTOR_CHECK(g.enLog("Lobo gris ataca a Venides") == 2);
}

MOTOR_TEST_CASE(a_xila_no_le_atacan_y_cura_a_quien_esta_en_apuros) {
    Grupo g({xila()});
    MOTOR_CHECK(g.aliado(1).cura() && g.aliado(1).curasQueQuedan() == 2);
    g.heroe.ponerVida(8);  // 8 de 30: por debajo del umbral (0,34)
    g.atacar(0);
    // Cura 1d4 + 1: con el dado fijo, el d4 da 4.
    MOTOR_CHECK(g.heroe.health() == 13);
    MOTOR_CHECK(g.enLog("Xila cura a Heroe (+5 PV; le quedan 1)") == 1);
    MOTOR_CHECK(g.aliado(1).curasQueQuedan() == 1);
    // Los lobos van a por el heroe, nunca a por Xila.
    g.ronda();
    MOTOR_CHECK(g.enLog("ataca a Xila") == 0);
    MOTOR_CHECK(g.enLog("Lobo ataca a Heroe") == 1 && g.enLog("Lobo gris ataca a Heroe") == 1);
    // Fuera de apuros no cura; con 8 o menos, la ultima cura.
    g.heroe.ponerVida(20);
    g.atacar(0);
    MOTOR_CHECK(g.aliado(1).curasQueQuedan() == 1);
    g.heroe.ponerVida(3);
    g.atacar(0);
    g.heroe.ponerVida(3);
    g.atacar(0);
    MOTOR_CHECK(g.aliado(1).curasQueQuedan() == 0);
    MOTOR_CHECK(g.enLog("Xila cura a Heroe") == 2);
}

MOTOR_TEST_CASE(venides_se_retira_a_cero_y_si_cae_el_heroe_se_pierde) {
    Grupo g({venides(5, true), xila()}, true);
    g.atacar(0);
    g.ronda();
    // Sin defensa, los dos reciben (0,5 contra 0, 1d6): el lobo le da 5 al
    // heroe y el gris, que es a quien pego Venides, 5 a Venides: se retira.
    // El combate sigue porque el heroe sigue en pie.
    MOTOR_CHECK(g.heroe.health() == 25);
    MOTOR_CHECK(!g.aliado(1).isAlive());
    MOTOR_CHECK(g.enLog("Venides se retira del combate.") == 1);
    MOTOR_CHECK(g.batalla->outcome() == BattleOutcome::InProgress);
    // Retirado, ya no actua. Cae el heroe (Xila le cura 5 antes): Xila se
    // retira con el y la batalla se pierde.
    g.heroe.ponerVida(5);
    g.atacar(0);
    MOTOR_CHECK(g.enLog("Venides ataca") == 2);
    MOTOR_CHECK(g.heroe.health() == 10);
    g.ronda();
    MOTOR_CHECK(!g.heroe.isAlive());
    MOTOR_CHECK(!g.aliado(2).isAlive());
    MOTOR_CHECK(g.batalla->outcome() == BattleOutcome::Defeat);
}

MOTOR_TEST_CASE(los_acompanantes_se_leen_se_escriben_y_se_validan) {
    const char* json = R"({
      "id": "ocaso",
      "acompanantes": [
        {"id": "venides", "nombre": "Venides", "vida": 16, "stats": {"CON": 3, "DES": 2, "CAR": 1},
         "ataques": 2, "alCaer": "se_retira"},
        {"id": "xila", "nombre": "Xila", "vida": 6, "ataques": 0, "atacable": false,
         "cura": {"dado": 4, "mas": 1, "veces": 2, "umbral": 0.34}}
      ],
      "beats": [
        {"id": "puerta", "trigger": {"type": "auto"}, "lines": ["."],
         "effects": [{"type": "startBattle", "startBattle": {"monsterId": "perdido_devoto", "cantidad": 3,
            "acompanantes": [{"id": "venides"}, {"id": "xila", "siFlag": "bnd_pista_xila"}],
            "flagVictory": "v", "flagDefeat": "d"}}]}
      ]
    })";
    const auto leida = RPG::AdventureScript::loadFromString(json);
    MOTOR_CHECK(leida.isOk());
    if (!leida.isOk()) return;
    const RPG::AdventureScript& a = leida.value();
    MOTOR_CHECK(a.acompanantes.size() == 2);
    const RPG::AcompananteDef* v = a.acompanante("venides");
    MOTOR_CHECK(v != nullptr && v->vida == 16 && v->stats[0] == 3 && v->stats[1] == 2 && v->stats[3] == 1 &&
                v->ataques == 2 && v->seRetira && v->atacable && v->cura.veces == 0);
    const RPG::AcompananteDef* x = a.acompanante("xila");
    MOTOR_CHECK(x != nullptr && !x->atacable && x->ataques == 0 && x->cura.dado == 4 && x->cura.mas == 1 &&
                x->cura.veces == 2 && !x->seRetira);
    const RPG::BattleRequest& b = a.beats[0].effects[0].battle;
    MOTOR_CHECK(b.acompanantes.size() == 2 && b.acompanantes[0].id == "venides" && b.acompanantes[0].siFlag.empty() &&
                b.acompanantes[1].siFlag == "bnd_pista_xila");
    // De ida y vuelta, el mismo texto.
    const std::string texto = RPG::serializeAdventure(a);
    const auto otra = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto);
    MOTOR_CHECK(otra.isOk() && otra.value().acompanantes.size() == 2 &&
                otra.value().beats[0].effects[0].battle.acompanantes.size() == 2);
    // Sin acompañantes no se escribe nada nuevo.
    const auto sola = RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": []})");
    MOTOR_CHECK(sola.isOk() && RPG::serializeAdventure(sola.value()).find("acompanantes") == std::string::npos);
    // Errores con nombre: un combate que pide a alguien que no esta, un id
    // repetido, sin vida o un alCaer que no existe.
    const auto falla = [](const char* j) { return !RPG::AdventureScript::loadFromString(j).isOk(); };
    MOTOR_CHECK(falla(R"({"id": "x", "beats": [{"id": "b", "trigger": {"type": "auto"},
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "corb", "acompanantes": [{"id": "nadie"}],
        "flagVictory": "v", "flagDefeat": "d"}}]}]})"));
    MOTOR_CHECK(falla(R"({"id": "x", "acompanantes": [{"id": "a", "vida": 3}, {"id": "a", "vida": 3}], "beats": []})"));
    MOTOR_CHECK(falla(R"({"id": "x", "acompanantes": [{"id": "a"}], "beats": []})"));
    MOTOR_CHECK(falla(R"({"id": "x", "acompanantes": [{"id": "a", "vida": 3, "alCaer": "huye"}], "beats": []})"));
    MOTOR_CHECK(falla(R"({"id": "x", "acompanantes": [{"id": "a", "vida": 3, "cura": {"dado": 4}}], "beats": []})"));
}

// ROSITA-01 — La compañía de Rosita llega a mitad del combate: cuando el
// enemigo principal baja a un cuarto de su vida (o el heroe), si se cumple su
// "requires" (aqui, una flag y una variable), una sola vez; enciende sus
// flags, anota sus lineas y se pone al final del bando con su ficha.
MOTOR_TEST_CASE(la_compania_llega_a_mitad_del_combate_una_vez) {
    const auto aventura = RPG::AdventureScript::loadFromString(R"({
      "id": "ocaso",
      "acompanantes": [{"id": "guardabosques", "nombre": "El guardabosques", "vida": 14, "stats": {"DES": 4}},
                       {"id": "mago", "nombre": "El mago de las tinieblas", "vida": 10, "stats": {"INT": 4}}],
      "beats": [{"id": "final", "trigger": {"type": "auto"}, "lines": ["."],
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "sandes", "flagVictory": "v", "flagDefeat": "d",
          "refuerzos": [{"requires": {"allFlags": ["rosita_semilla"], "variables": {"confianza_rosita": {"min": 2}}},
                         "cuando": {"vidaEnemigo": 0.25, "vidaHeroe": 0.25},
                         "flags": ["compania_vista", "compania"],
                         "lineas": ["Un trueno sin nube."],
                         "acompanantes": [{"id": "guardabosques"}, {"id": "mago", "siFlag": "con_mago"}]}]}}]}]
    })");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    RPG::BattleRequest pedido = aventura.value().beats[0].effects[0].battle;
    MOTOR_CHECK(pedido.refuerzos.size() == 1 && pedido.refuerzos[0].acompanantes.size() == 2);

    Grupo g({venides()});
    RPG::NarrativeState trama;
    const auto llegan = [&] { return Game::llegaRefuerzos(*g.batalla, pedido, trama, &aventura.value()); };
    // Con el lobo entero no llega nadie, aunque se cumpla el "requires".
    trama.setFlag("rosita_semilla");
    trama.setVariable("confianza_rosita", 2);
    MOTOR_CHECK(llegan() == 0 && g.batalla->allies().size() == 2);
    // A un cuarto (15 de 60), sin la confianza: tampoco.
    g.lobo.ponerVida(15);
    trama.setVariable("confianza_rosita", 1);
    MOTOR_CHECK(llegan() == 0 && g.batalla->allies().size() == 2);
    // Con ella, llegan: el guardabosques (el mago no, sin su flag).
    trama.setVariable("confianza_rosita", 2);
    MOTOR_CHECK(llegan() == 1);
    MOTOR_CHECK(g.batalla->allies().size() == 3 && g.batalla->allies()[2].name == "El guardabosques");
    MOTOR_CHECK(g.aliado(2).def().id == "guardabosques" && g.aliado(2).health() == 14);
    MOTOR_CHECK(trama.hasFlag("compania_vista") && trama.hasFlag("compania"));
    MOTOR_CHECK(g.enLog("Un trueno sin nube.") == 1 && g.enLog("Llegan a tu lado: El guardabosques.") == 1);
    MOTOR_CHECK(pedido.refuerzos.empty());
    // Una sola vez.
    MOTOR_CHECK(llegan() == 0 && g.batalla->allies().size() == 3);
    // Y pelea: tras el heroe, como los demas.
    g.atacar(0);
    MOTOR_CHECK(g.enLog("El guardabosques") >= 2);
}

MOTOR_TEST_CASE(la_compania_llega_si_el_heroe_va_a_caer_y_se_lee_y_escribe) {
    const char* json = R"({
      "id": "ocaso",
      "acompanantes": [{"id": "guardabosques", "nombre": "El guardabosques", "vida": 14}],
      "beats": [{"id": "final", "trigger": {"type": "auto"}, "lines": ["."],
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "sandes", "flagVictory": "v", "flagDefeat": "d",
          "refuerzos": [{"cuando": {"vidaHeroe": 0.25}, "flags": ["compania"],
                         "acompanantes": [{"id": "guardabosques"}]}]}}]}]
    })";
    const auto aventura = RPG::AdventureScript::loadFromString(json);
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    RPG::BattleRequest pedido = aventura.value().beats[0].effects[0].battle;
    Grupo g({});
    RPG::NarrativeState trama;
    g.lobo.ponerVida(1);  // el enemigo no cuenta: solo mira al heroe
    MOTOR_CHECK(Game::llegaRefuerzos(*g.batalla, pedido, trama, &aventura.value()) == 0);
    g.heroe.ponerVida(7);  // 7 de 30, menos de un cuarto
    MOTOR_CHECK(Game::llegaRefuerzos(*g.batalla, pedido, trama, &aventura.value()) == 1);
    MOTOR_CHECK(g.batalla->allies().size() == 2 && trama.hasFlag("compania"));
    // De ida y vuelta, el mismo texto, con sus refuerzos.
    const std::string texto = RPG::serializeAdventure(aventura.value());
    MOTOR_CHECK(texto.find("\"refuerzos\"") != std::string::npos && texto.find("\"vidaHeroe\"") != std::string::npos);
    const auto otra = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto);
    // Errores con nombre: sin "cuando", fuera de (0, 1], o alguien que no esta.
    const auto falla = [](const char* refuerzo) {
        const std::string j = std::string(R"({"id": "x", "acompanantes": [{"id": "a", "vida": 3}], "beats": [{"id": "b",
            "trigger": {"type": "auto"}, "effects": [{"type": "startBattle", "startBattle": {"monsterId": "m",
            "flagVictory": "v", "flagDefeat": "d", "refuerzos": [)") + refuerzo + "]}}]}]}";
        return !RPG::AdventureScript::loadFromString(j).isOk();
    };
    MOTOR_CHECK(!falla(R"({"cuando": {"vidaEnemigo": 0.5}, "acompanantes": [{"id": "a"}]})"));
    MOTOR_CHECK(falla(R"({"acompanantes": [{"id": "a"}]})"));
    MOTOR_CHECK(falla(R"({"cuando": {"vidaEnemigo": 1.5}, "acompanantes": [{"id": "a"}]})"));
    MOTOR_CHECK(falla(R"({"cuando": {"vidaHeroe": 0.2}, "acompanantes": [{"id": "nadie"}]})"));
    MOTOR_CHECK(falla(R"({"cuando": {"vidaHeroe": 0.2}})"));
}
// AV13-34 (Oriol, 14/10, D0015 §13) — La compania llega cuando Sandes, a un cuarto de su vida o
// menos, te da un golpe mortal; guionizado: su siguiente accion te deja a las puertas (1 PV).
MOTOR_TEST_CASE(la_compania_llega_con_el_golpe_mortal_de_sandes_a_un_cuarto) {
    const char* json = R"({
      "id": "ocaso",
      "acompanantes": [{"id": "guardabosques", "nombre": "El guardabosques", "vida": 14}],
      "beats": [{"id": "final", "trigger": {"type": "auto"}, "lines": ["."],
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "sandes", "flagVictory": "v", "flagDefeat": "d",
          "refuerzos": [{"cuando": {"vidaEnemigo": 0.25, "golpeMortal": true}, "flags": ["compania"],
                         "acompanantes": [{"id": "guardabosques"}]}]}}]}]
    })";
    const auto aventura = RPG::AdventureScript::loadFromString(json);
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    RPG::BattleRequest pedido = aventura.value().beats[0].effects[0].battle;
    MOTOR_CHECK(pedido.refuerzos.size() == 1 && pedido.refuerzos[0].golpeMortal);
    Grupo g({});
    RPG::NarrativeState trama;
    g.batalla->ponGolpeMortal(0, 0.25f);
    g.heroe.ponerVida(5);  // el heroe muy bajo no basta: hace falta el golpe mortal
    MOTOR_CHECK(Game::llegaRefuerzos(*g.batalla, pedido, trama, &aventura.value()) == 0);
    g.heroe.ponerVida(30);
    g.batalla->resolveEnemyTurn();  // el lobo a mas de un cuarto: aun no
    MOTOR_CHECK(!g.batalla->golpeMortalDado());
    g.lobo.ponerVida(15);           // 15 de 60: un cuarto
    g.heroe.ponerVida(30);
    g.batalla->resolveEnemyTurn();
    MOTOR_CHECK(g.batalla->golpeMortalDado());
    MOTOR_CHECK(g.heroe.health() == 1);
    MOTOR_CHECK(g.enLog("te da un golpe mortal") == 1);
    MOTOR_CHECK(g.batalla->outcome() == BattleOutcome::InProgress);
    MOTOR_CHECK(Game::llegaRefuerzos(*g.batalla, pedido, trama, &aventura.value()) == 1);
    MOTOR_CHECK(g.batalla->allies().size() == 2 && trama.hasFlag("compania"));
    // Sin vidaEnemigo, golpeMortal es un error con nombre; y se escribe de vuelta.
    MOTOR_CHECK(RPG::serializeAdventure(aventura.value()).find("\"golpeMortal\"") != std::string::npos);
    const std::string mal = std::string(json).replace(std::string(json).find("\"vidaEnemigo\": 0.25, "), 21, "");
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(mal).isOk());
}

// AV13-29 — La iniciativa con los acompañantes: cada uno su puesto y su tirada; el de IA juega solo.
MOTOR_TEST_CASE(los_acompanantes_tiran_iniciativa_y_el_de_ia_juega_solo) {
    RPG::AcompananteDef solo = venides();
    solo.soloIA = true;
    Grupo g({venides(), solo});
    g.cartas->ponTurnoBG3(true);
    g.batalla->tiraIniciativa();
    const auto& puestos = g.batalla->orden().puestos();
    int aliados = 0;
    for (const auto& p : puestos) aliados += !p.enemigo;
    MOTOR_CHECK(aliados == 3);                       // el heroe y los dos acompañantes
    MOTOR_CHECK(g.batalla->loJuegaElJugador(1));     // Venides: lo juegas tu
    MOTOR_CHECK(!g.batalla->loJuegaElJugador(2));    // el de "ia": true, solo
    const int antes = g.lobo2.health();
    g.batalla->resuelveAliadoSolo(2);                // su IA: golpea al ultimo enemigo en pie
    MOTOR_CHECK(g.lobo2.health() < antes);
    MOTOR_CHECK(g.batalla->log().front().find("Iniciativa (1d6 + DES): ") == 0);
}

// AV13-28 (Oriol, 14/10, D0014 §8) — A lo BG3, atacar no cierra el turno: el heroe puede seguir
// (andar, lo libre), no puede volver a atacar, y Venides actua cuando el heroe termina su turno.
MOTOR_TEST_CASE(a_lo_bg3_atacar_no_cierra_el_turno_y_los_suyos_actuan_al_terminar) {
    Grupo g({venides()});
    g.cartas->ponTurnoBG3(true);
    g.atacar(0);
    MOTOR_CHECK(g.cartas->turnoAbierto());
    MOTOR_CHECK(g.lobo.health() == 45);
    MOTOR_CHECK(g.enLog("Venides ataca") == 0);  // aun no: el turno sigue abierto
    g.atacar(0);                                 // una sola principal por turno
    MOTOR_CHECK(g.lobo.health() == 45);
    MOTOR_CHECK(g.enLog("ya ha actuado este turno") == 1);
    const Game::VistaBarra v = Game::vistaDeBatalla(*g.batalla);
    MOTOR_CHECK(v.basicas[0].estado == Game::CartaBarra::Estado::Bloqueada);  // Atacar, en gris
    g.batalla->resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, Game::kAccionPasar, 0, ""});
    MOTOR_CHECK(!g.cartas->turnoAbierto());
    MOTOR_CHECK(g.enLog("Heroe termina su turno.") == 1);
    // AV13-29: a lo BG3 Venides ya no actua al terminar el heroe: tiene su turno y lo juegas tu.
    MOTOR_CHECK(g.enLog("Venides ataca") == 0);
    MOTOR_CHECK(g.batalla->loJuegaElJugador(0) && g.batalla->loJuegaElJugador(1));
    g.batalla->resolveAllyAction(1, BattleAction{BattleActionType::Attack, "", 1, ""});  // al lobo gris
    MOTOR_CHECK(g.enLog("Venides ataca a Lobo gris") == 2);  // sus dos golpes (Multiataque)
    g.batalla->resolveAllyAction(1, BattleAction{BattleActionType::Attack, "", 1, ""});
    MOTOR_CHECK(g.enLog("Venides ya ha actuado este turno.") == 1);
    g.batalla->resolveAllyAction(1, BattleAction{BattleActionType::UseSkill, Game::kAccionPasar, 0, ""});
    MOTOR_CHECK(g.enLog("Venides termina su turno.") == 1);
}


// AV13-29 — Con tablero, el de IA anda antes de actuar (hasta el enemigo, si pega) y quien cae deja
// libre su casilla.
MOTOR_TEST_CASE(con_tablero_el_de_ia_anda_hasta_el_enemigo_y_quien_cae_deja_su_casilla) {
    RPG::AcompananteDef solo = venides();
    solo.soloIA = true;
    Grupo g({solo});
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 5}, {7, 5}}, 20, 20, [](int, int) { return false; });
    MOTOR_CHECK(t.pon("Heroe", {2, 5}) && t.pon("Venides", {2, 6}) && t.pon("Lobo", {7, 5}) && t.pon("Lobo gris", {7, 6}));
    g.batalla->ponTablero(std::move(t));
    const Game::TableroCombate& tb = *g.batalla->tablero();
    MOTOR_CHECK(!tb.aTiro("Venides", "Lobo", 1) && !tb.aTiro("Venides", "Lobo gris", 1));
    const int antes = g.lobo.health() + g.lobo2.health();
    g.batalla->resuelveAliadoSolo(1);
    MOTOR_CHECK(tb.aTiro("Venides", "Lobo", 1) || tb.aTiro("Venides", "Lobo gris", 1));  // ha andado hasta ellos
    MOTOR_CHECK(g.lobo.health() + g.lobo2.health() < antes);                           // y ha pegado
    g.lobo.ponerVida(0);
    g.batalla->resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, Game::kAccionPasar, 0, ""});
    MOTOR_CHECK(!tb.dondeEsta("Lobo").has_value());   // cayo: su casilla, libre
    MOTOR_CHECK(tb.dondeEsta("Lobo gris").has_value());
}

// AV13-29 — Si el primer golpe de Venides tumba a uno, el segundo va a otro que tenga al lado.
MOTOR_TEST_CASE(el_segundo_golpe_de_venides_va_a_otro_si_el_primero_mata) {
    Grupo g({venides()});
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 5}, {3, 6}}, 20, 20, [](int, int) { return false; });
    MOTOR_CHECK(t.pon("Heroe", {1, 5}) && t.pon("Venides", {2, 6}) && t.pon("Lobo", {3, 6}) && t.pon("Lobo gris", {2, 7}));
    g.batalla->ponTablero(std::move(t));
    g.lobo.ponerVida(1);
    g.batalla->resolveAllyAction(1, BattleAction{BattleActionType::Attack, "", 0, ""});
    MOTOR_CHECK(!g.lobo.isAlive());
    MOTOR_CHECK(g.lobo2.health() < 60);  // el segundo golpe, al gris
}

// AV13-14 — Una carta de area pilla a todos los de dentro, aliados incluidos: la bola de fuego de la
// Cancion del Verano («radio de 5 pies»: radio 1) contra el lobo quema tambien a Venides, que esta a su
// lado, y no al lobo gris, que esta lejos.
MOTOR_TEST_CASE(un_area_pilla_a_todos_los_de_dentro_aliados_incluidos) {
    Grupo g({venides()});
    Game::TableroCombate t = Game::TableroCombate::alrededor({{1, 5}, {7, 9}}, 20, 20, [](int, int) { return false; });
    MOTOR_CHECK(t.pon("Heroe", {3, 5}) && t.pon("Venides", {6, 5}) && t.pon("Lobo", {7, 5}) && t.pon("Lobo gris", {7, 9}));
    g.batalla->ponTablero(std::move(t));
    int fila = -1;
    const auto filas = g.cartas->filas();
    for (std::size_t i = 0; i < filas.size(); ++i)
        if (fila < 0 && filas[i].rfind("Canción del Verano", 0) == 0) fila = static_cast<int>(i);
    MOTOR_CHECK(fila >= 0);
    if (fila < 0) return;
    BattleAction a;
    MOTOR_CHECK(g.cartas->accionDeFila(static_cast<std::size_t>(fila), a));
    a.targetIndex = 0;  // al lobo
    const int venidesAntes = g.aliado(1).health(), grisAntes = g.lobo2.health();
    g.batalla->resolveAllyAction(0, a);
    MOTOR_CHECK(g.enLog("sobre Lobo:") >= 1);
    MOTOR_CHECK(g.enLog("sobre Venides:") >= 1);    // dentro: tambien le pilla
    MOTOR_CHECK(g.enLog("sobre Lobo gris") == 0);   // fuera
    MOTOR_CHECK(g.lobo2.health() == grisAntes);
    MOTOR_CHECK(g.aliado(1).health() <= venidesAntes);
}
