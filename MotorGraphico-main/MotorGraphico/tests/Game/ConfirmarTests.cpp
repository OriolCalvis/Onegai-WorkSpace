// AV15-05a (EH-22, EH-23, EH-29, EH-30) — Lo caro e irreversible se confirma.
//
//   1  la pausa pregunta antes de salir, volver al título, cargar y guardar
//      encima, y no pregunta cuando no hay nada que perder;
//   2  la pregunta de la pausa va en el mismo menú: «No» primero y con el
//      foco, el título es la pregunta, y al olvidarla se vuelve donde estaba;
//   3  dos partidas son la misma si solo cambia la fecha de guardado;
//   4  el negocio: COMPRAR ya no paga al primer ENTER (pregunta con el precio,
//      el oro que queda y la renta), «No» no compra, «Sí» sí;
//   5  lo tuyo se revende con la pérdida a la vista, también con pregunta;
//   6  las filas dicen lo que hacen y, apagadas, por qué; ESC cierra antes la
//      pregunta que el negocio;
//   7  la hoja del negocio y su pregunta se pintan (dos papiros, «No» con el foco).

#include "Game/Confirmacion.h"
#include "Game/GameSession.h"
#include "Game/NegocioEstado.h"
#include "Game/PauseMenu.h"
#include "Game/SaveGame.h"
#include "HUD/Iconos.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/HojaNegocio.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/Papiro.h"
#include "HUD/Pergamino/Pincel.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"
#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <vector>

using Game::PauseAction;

namespace {

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
};

struct Ciudad {
    ObjectCatalog catalogo;
    SkillCatalog skills;
    Cuerpo cuerpo;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    Ciudad() {
        ObjectDefinition cartel;
        cartel.id = "posada";
        cartel.name = "Posada";
        cartel.category = ObjectCategory::Prop;
        cartel.business.price = 120;
        cartel.business.baseIncome = 12;
        cartel.business.businessName = "Posada del Cuervo";
        catalogo.add(cartel);
    }
    GameSession sesion(int oro) {
        LevelDefinition nivel;
        nivel.name = "Ciudad";
        nivel.playerStart = GridCoord{0, 0};
        ObjectSpawn spawn;
        spawn.objectId = "posada";
        spawn.position = GridCoord{0, 0};
        nivel.objects.push_back(spawn);
        GameSession s(nullptr, &catalogo, &skills, nivel, &cuerpo, &habilidades, &mochila);
        s.addGold(oro);
        return s;
    }
};

const char* const kHoy = "2026-10-14 13:05";

}  // namespace

MOTOR_TEST_CASE(la_pausa_pregunta_solo_cuando_hay_algo_que_perder) {
    Game::UltimaPartida nada;  // nunca has guardado
    Game::UltimaPartida igual{true, "2026-10-14 12:31", true};
    Game::UltimaPartida distinta{true, "2026-10-14 12:31", false};
    Game::UltimaPartida ayer{true, "2026-09-14 06:58", false};

    // Salir y volver al titulo: preguntan salvo que este igual que lo guardado.
    for (PauseAction a : {PauseAction::Quit, PauseAction::Title}) {
        MOTOR_CHECK(!Game::preguntaDePausa(a, nada, kHoy).vacia());
        MOTOR_CHECK(Game::preguntaDePausa(a, igual, kHoy).vacia());
        MOTOR_CHECK(!Game::preguntaDePausa(a, distinta, kHoy).vacia());
    }
    const Game::Confirmacion salir = Game::preguntaDePausa(PauseAction::Quit, distinta, kHoy);
    MOTOR_CHECK(salir.titulo == "¿SALIR DEL JUEGO?");
    MOTOR_CHECK(salir.detalle == "Perderás lo jugado desde las 12:31.");
    MOTOR_CHECK(salir.no == "No, seguir jugando" && salir.si == "Sí, salir sin guardar");
    MOTOR_CHECK(salir.foco == 0 && !salir.siElegido());
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Quit, nada, kHoy).detalle ==
                "Perderás todo lo que no hayas guardado.");
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Title, ayer, kHoy).detalle ==
                "Perderás lo jugado desde el 14/09 a las 06:58.");

    // Cargar: solo si hay partida y algo cambia.
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Load, nada, kHoy).vacia());
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Load, igual, kHoy).vacia());
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Load, distinta, kHoy).detalle ==
                "Vuelves a la de las 12:31: pierdes lo de después.");
    // Guardar: solo si se sobrescribe otra distinta.
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Save, nada, kHoy).vacia());
    MOTOR_CHECK(Game::preguntaDePausa(PauseAction::Save, igual, kHoy).vacia());
    const Game::Confirmacion encima = Game::preguntaDePausa(PauseAction::Save, distinta, kHoy);
    MOTOR_CHECK(encima.titulo == "¿GUARDAR ENCIMA?" && encima.detalle == "Sustituye la partida de las 12:31.");
    // Lo demas nunca pregunta.
    for (PauseAction a : {PauseAction::Resume, PauseAction::Options, PauseAction::Character})
        MOTOR_CHECK(Game::preguntaDePausa(a, distinta, kHoy).vacia());
}

MOTOR_TEST_CASE(la_pregunta_de_la_pausa_va_en_el_mismo_menu) {
    Game::PauseMenu m = Game::PauseMenu::create();
    const std::size_t filas = m.entries().size();
    MOTOR_CHECK(m.titulo() == "EN PAUSA" && !m.preguntando());
    while (m.confirm() != PauseAction::Quit) m.moveDown();
    m.pregunta(PauseAction::Quit, "¿SALIR DEL JUEGO?", "Perderás lo jugado desde las 12:31.", "No, seguir jugando",
               "Sí, salir sin guardar");
    MOTOR_CHECK(m.preguntando() && m.pendiente() == PauseAction::Quit);
    MOTOR_CHECK(m.entries().size() == 2);
    MOTOR_CHECK(m.entries()[0].label == "No, seguir jugando" && m.entries()[0].action == PauseAction::None);
    MOTOR_CHECK(m.entries()[1].label == "Sí, salir sin guardar" && m.entries()[1].action == PauseAction::None);
    MOTOR_CHECK(m.selected() == 0 && !m.siElegido());  // «No» con el foco
    MOTOR_CHECK(m.titulo() == "¿SALIR DEL JUEGO?" && m.subtitulo() == "Perderás lo jugado desde las 12:31.");
    m.moveDown();
    MOTOR_CHECK(m.siElegido());
    m.moveDown();  // da la vuelta entre las dos
    MOTOR_CHECK(!m.siElegido());
    m.olvidaPregunta();
    MOTOR_CHECK(!m.preguntando() && m.entries().size() == filas);
    MOTOR_CHECK(m.confirm() == PauseAction::Quit);  // el foco, donde estaba
    MOTOR_CHECK(m.titulo() == "EN PAUSA" && m.subtitulo().empty());
}

MOTOR_TEST_CASE(dos_partidas_son_la_misma_si_solo_cambia_la_fecha) {
    SaveGameData a;
    a.levelPath = "assets/levels/ciudad_centro.json";
    a.gold = 30;
    a.savedAt = "2026-10-14 12:31";
    SaveGameData b = a;
    b.savedAt = "2026-10-14 13:00";
    MOTOR_CHECK(Game::mismaPartida(a, b));
    b.gold = 31;
    MOTOR_CHECK(!Game::mismaPartida(a, b));
    b = a;
    b.playerPosition = GridCoord{3, 4};
    MOTOR_CHECK(!Game::mismaPartida(a, b));
    MOTOR_CHECK(Game::cuandoSeGuardo("2026-10-14 09:05", kHoy) == "las 09:05");
    MOTOR_CHECK(Game::cuandoSeGuardo("", kHoy) == "la última vez que guardaste");
}

MOTOR_TEST_CASE(comprar_un_negocio_pide_confirmacion) {
    Ciudad c;
    GameSession s = c.sesion(150);
    MOTOR_CHECK(s.interact() && s.mode() == GameMode::Business);
    Game::NegocioEstado n;
    auto filas = Game::NegocioEstado::filas(s);
    MOTOR_CHECK(filas.size() == 2);
    MOTOR_CHECK(filas[0].accion == Game::AccionNegocio::Comprar && filas[0].valor == "120 ORO");
    MOTOR_CHECK(filas[0].motivo == "Te dará 12 de oro por ciclo.");

    // Primer ENTER: la pregunta, no la compra.
    MOTOR_CHECK(!n.confirma(s));
    MOTOR_CHECK(!s.ownsBusiness("posada") && s.gold() == 150);
    const Game::Confirmacion* q = n.pregunta();
    MOTOR_CHECK(q != nullptr && q->titulo == "¿COMPRARLO?");
    MOTOR_CHECK(q->detalle == "Posada del Cuervo por 120 de oro.");
    MOTOR_CHECK(q->notaSi == "Te quedarán 30 de oro. Te dará 12 de oro por ciclo.");
    MOTOR_CHECK(q->foco == 0);
    // ENTER sobre «No»: nada.
    MOTOR_CHECK(!n.confirma(s));
    MOTOR_CHECK(n.pregunta() == nullptr && !s.ownsBusiness("posada") && s.gold() == 150);
    // Otra vez, ahora «Sí».
    n.confirma(s);
    n.mueve(+1, filas.size());
    MOTOR_CHECK(n.pregunta()->siElegido());
    n.confirma(s);
    MOTOR_CHECK(s.ownsBusiness("posada") && s.gold() == 30);
    MOTOR_CHECK(n.estado() == "Has comprado Posada del Cuervo." && n.estadoDetalle() == "Te quedan 30 de oro.");
    MOTOR_CHECK(!n.estadoError());
}

MOTOR_TEST_CASE(sin_oro_comprar_esta_apagado_y_dice_por_que) {
    Ciudad c;
    GameSession s = c.sesion(100);
    MOTOR_CHECK(s.interact());
    const auto filas = Game::NegocioEstado::filas(s);
    MOTOR_CHECK(filas[0].apagada && filas[0].motivo == "Te faltan 20 de oro.");
    Game::NegocioEstado n;
    n.confirma(s);
    MOTOR_CHECK(n.pregunta() == nullptr && n.estadoError());
    MOTOR_CHECK(!s.ownsBusiness("posada") && s.gold() == 100);
}

MOTOR_TEST_CASE(lo_tuyo_se_revende_con_la_perdida_a_la_vista) {
    Ciudad c;
    GameSession s = c.sesion(150);
    MOTOR_CHECK(s.interact());
    MOTOR_CHECK(s.buyCurrentBusiness() && s.gold() == 30);
    auto filas = Game::NegocioEstado::filas(s);
    MOTOR_CHECK(filas.size() == 4);  // subir, bajar, revender, salir: el orden del menu de siempre
    MOTOR_CHECK(filas[0].accion == Game::AccionNegocio::SubirAlquiler && filas[0].valor == "110 %");
    MOTOR_CHECK(filas[0].motivo.find("moral -2") != std::string::npos);
    MOTOR_CHECK(filas[1].accion == Game::AccionNegocio::BajarAlquiler && filas[1].motivo.find("moral +2") != std::string::npos);
    MOTOR_CHECK(filas[2].accion == Game::AccionNegocio::Revender && filas[2].valor == "60 ORO");
    MOTOR_CHECK(filas[2].motivo == "Recibes 60 de los 120 que pagaste.");
    MOTOR_CHECK(filas[3].accion == Game::AccionNegocio::Salir);

    Game::NegocioEstado n;
    n.ponFoco(2);
    n.confirma(s);
    MOTOR_CHECK(n.pregunta() != nullptr && n.pregunta()->titulo == "¿REVENDERLO?");
    MOTOR_CHECK(n.pregunta()->notaSi == "Recibes 60 de los 120 que pagaste.");
    MOTOR_CHECK(s.ownsBusiness("posada"));
    // ESC cierra la pregunta, no el negocio.
    MOTOR_CHECK(n.cancela());
    MOTOR_CHECK(n.pregunta() == nullptr && s.mode() == GameMode::Business);
    // Y «Sí»: se revende.
    n.ponFoco(2);
    n.confirma(s);
    n.mueve(+1, filas.size());
    n.confirma(s);
    MOTOR_CHECK(!s.ownsBusiness("posada") && s.gold() == 90);
    MOTOR_CHECK(n.estado() == "Has revendido Posada del Cuervo.");
    MOTOR_CHECK(n.estadoDetalle() == "Recibes 60 de oro; ahora tienes 90 de oro.");
    // Vuelve a estar en venta.
    MOTOR_CHECK(Game::NegocioEstado::filas(s)[0].accion == Game::AccionNegocio::Comprar);
    // Sin pregunta, ESC olvida y deja cerrar.
    MOTOR_CHECK(!n.cancela());
    // Revender lo que no es tuyo: -1, y no da oro.
    MOTOR_CHECK(s.sellCurrentBusiness(50) == -1 && s.gold() == 90);
}

MOTOR_TEST_CASE(subir_el_alquiler_no_pregunta_y_salir_cierra) {
    Ciudad c;
    GameSession s = c.sesion(150);
    MOTOR_CHECK(s.interact() && s.buyCurrentBusiness());
    Game::NegocioEstado n;
    n.ponFoco(0);
    MOTOR_CHECK(!n.confirma(s));
    MOTOR_CHECK(n.pregunta() == nullptr && n.estado() == "Alquiler al 110 %.");
    n.ponFoco(3);
    MOTOR_CHECK(n.confirma(s));
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
}

MOTOR_TEST_CASE(la_hoja_del_negocio_y_su_pregunta_se_pintan) {
    Pergamino::FuenteAtlas letras;
    Hud::Iconos iconos;
    MOTOR_CHECK(letras.carga("assets/textures/letras_pergamino.json").isOk());
    MOTOR_CHECK(iconos.cargar("assets/textures/iconos.json").isOk());
    const Pergamino::Pincel p(letras, iconos);
    Ciudad c;
    GameSession s = c.sesion(150);
    MOTOR_CHECK(s.interact());
    Game::NegocioEstado n;
    Pergamino::Lienzo sola;
    Pergamino::pintaNegocio(sola, p, s, n, 1280, 720);
    MOTOR_CHECK(!sola.vacio());
    int filasSola = 0;
    for (const Pergamino::Zona& z : sola.zonas()) filasSola += z.tipo == Pergamino::kZonaEntradaPapiro;
    MOTOR_CHECK(filasSola == 2);  // Comprar y Salir
    n.confirma(s);  // la pregunta
    Pergamino::Lienzo conPregunta;
    Pergamino::pintaNegocio(conPregunta, p, s, n, 1280, 720);
    int filas = 0;
    for (const Pergamino::Zona& z : conPregunta.zonas()) filas += z.tipo == Pergamino::kZonaEntradaPapiro;
    MOTOR_CHECK(filas == 4);  // las dos del negocio y las dos de la pregunta, encima
    MOTOR_CHECK(conPregunta.zonas().back().indice == 1);  // la ultima registrada: el «Si» de la pregunta
    MOTOR_CHECK(conPregunta.cuantos() > sola.cuantos());
}

int main() { return motor_test::runAllTests(); }
