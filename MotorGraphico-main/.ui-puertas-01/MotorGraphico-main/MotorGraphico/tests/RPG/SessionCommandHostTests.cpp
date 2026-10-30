#include "RPG/SessionCommandHost.h"

#include "TestHarness.h"

#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

// E-04 (segunda rebanada) — Las familias que necesitan la partida.
//
// Lo que se defiende aqui: que dar y quitar objetos y cobrar oro pasen de
// verdad por la sesion, que un cobro mayor que el saldo FALLE en vez de
// recortarse en silencio, que cambiar de mapa sea terminal (DEC-30) y que
// las condiciones de inventario, que la primera rebanada no podia
// evaluar, ya se evaluen.

namespace {

using namespace RPG;

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
    std::string combatant_id() const override { return "test"; }
};

LevelDefinition nivel() {
    LevelDefinition l;
    l.name = "Prueba";
    l.playerStart = GridCoord{0, 0};
    return l;
}

struct Mundo {
    ObjectCatalog catalogo;
    SkillCatalog habilidades;
    Cuerpo cuerpo;
    SkillSet skills{10};
    std::vector<std::string> inventario;
    GameSession sesion{nullptr, &catalogo, &habilidades, nivel(), &cuerpo, &skills, &inventario};
};

MapEventCommand cmd(const std::string& tipo,
                    std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

std::size_t cuantos(const std::vector<std::string>& inv, const std::string& id) {
    return static_cast<std::size_t>(std::count(inv.begin(), inv.end(), id));
}

MOTOR_TEST_CASE(dar_y_quitar_objetos_toca_el_inventario_de_verdad) {
    Mundo m;
    SessionCommandHost host(m.sesion, m.inventario);

    MOTOR_CHECK(host.execute(cmd("inventario.dar", {{"objeto", "llave_cueva"}})).outcome ==
                CommandOutcome::Done);
    MOTOR_CHECK(cuantos(m.inventario, "llave_cueva") == 1);

    MOTOR_CHECK(host.execute(cmd("inventario.dar",
                                 {{"objeto", "pocion"}, {"cantidad", "3"}})).outcome ==
                CommandOutcome::Done);
    MOTOR_CHECK(cuantos(m.inventario, "pocion") == 3);

    MOTOR_CHECK(host.execute(cmd("inventario.quitar",
                                 {{"objeto", "pocion"}, {"cantidad", "2"}})).outcome ==
                CommandOutcome::Done);
    MOTOR_CHECK(cuantos(m.inventario, "pocion") == 1);
    MOTOR_CHECK(cuantos(m.inventario, "llave_cueva") == 1);   // no toca lo demas
}

MOTOR_TEST_CASE(quitar_mas_de_lo_que_hay_falla_y_no_quita_nada) {
    // Todo o nada: quitar tres de una llave cuando solo hay dos dejaria
    // al jugador sin las dos Y sin la puerta abierta.
    Mundo m;
    SessionCommandHost host(m.sesion, m.inventario);
    host.execute(cmd("inventario.dar", {{"objeto", "llave"}, {"cantidad", "2"}}));

    const CommandResult r = host.execute(cmd("inventario.quitar",
                                             {{"objeto", "llave"}, {"cantidad", "3"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(cuantos(m.inventario, "llave") == 2);   // intacto
}

MOTOR_TEST_CASE(un_peaje_mayor_que_el_saldo_falla_en_vez_de_recortarse) {
    // addGold() de la sesion nunca deja el oro negativo: un cobro mayor
    // que el saldo se recorta en silencio. Para un peaje de evento eso es
    // exactamente lo que NO queremos — la puerta se abriria sin haberla
    // pagado.
    Mundo m;
    SessionCommandHost host(m.sesion, m.inventario);
    host.execute(cmd("economia.oro", {{"valor", "50"}}));
    MOTOR_CHECK(m.sesion.gold() == 50);

    const CommandResult caro = host.execute(cmd("economia.oro", {{"valor", "-80"}}));
    MOTOR_CHECK(caro.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(caro.diagnostic.find("30") != std::string::npos);   // le faltan 30
    MOTOR_CHECK(m.sesion.gold() == 50);                             // no se cobro

    MOTOR_CHECK(host.execute(cmd("economia.oro", {{"valor", "-50"}})).outcome ==
                CommandOutcome::Done);
    MOTOR_CHECK(m.sesion.gold() == 0);
}

MOTOR_TEST_CASE(ir_a_otro_mapa_anota_la_transicion_y_termina_el_evento) {
    // DEC-30. Y la transicion se ANOTA en vez de ejecutarse: GameSession
    // no carga niveles a proposito, y este anfitrion sigue la misma
    // division — la sesion decide, el de fuera orquesta.
    Mundo m;
    SessionCommandHost host(m.sesion, m.inventario);
    MOTOR_CHECK(!host.requestedTransition().pending);

    const CommandResult r = host.execute(cmd("mapa.ir", {{"mapa", "assets/levels/bosque.json"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::EndEvent);
    MOTOR_CHECK(host.requestedTransition().pending);
    MOTOR_CHECK(host.requestedTransition().levelPath == "assets/levels/bosque.json");

    host.clearRequestedTransition();
    MOTOR_CHECK(!host.requestedTransition().pending);
}

MOTOR_TEST_CASE(las_condiciones_de_inventario_ya_se_evaluan) {
    // La primera rebanada devolvia Error aqui a proposito. Ahora hay
    // inventario que mirar.
    Mundo m;
    NarrativeState estado;
    m.inventario = {"llave_cueva", "pocion", "pocion"};

    MapEventCondition tiene;
    tiene.kind = EventConditionKind::Item;
    tiene.reference = "llave_cueva";
    // Sin cantidad, «tiene al menos uno» es lo que el autor quiere decir.
    MOTOR_CHECK(EvaluateConditionWithInventory(tiene, estado, m.inventario).value());

    MapEventCondition noTiene = tiene;
    noTiene.reference = "espada";
    MOTOR_CHECK(!EvaluateConditionWithInventory(noTiene, estado, m.inventario).value());

    MapEventCondition dos = tiene;
    dos.reference = "pocion";
    dos.comparison = ">=";
    dos.value = "2";
    MOTOR_CHECK(EvaluateConditionWithInventory(dos, estado, m.inventario).value());
    dos.value = "3";
    MOTOR_CHECK(!EvaluateConditionWithInventory(dos, estado, m.inventario).value());

    // Negada, y las de bandera siguen funcionando por el camino de siempre.
    MapEventCondition negada = noTiene;
    negada.negated = true;
    MOTOR_CHECK(EvaluateConditionWithInventory(negada, estado, m.inventario).value());

    estado.setFlag("prologo");
    MapEventCondition bandera;
    bandera.kind = EventConditionKind::Flag;
    bandera.reference = "prologo";
    MOTOR_CHECK(EvaluateConditionWithInventory(bandera, estado, m.inventario).value());
}

// E-04 (cierre) — comercio.abrir. Un catalogo con un tendero que vende
// una pocion con precio de catalogo y un pan con precio propio, un PNJ que
// solo habla, un tendero cuyos articulos no tienen precio y un objeto que
// no es un PNJ.
void llenarTiendas(ObjectCatalog& c) {
    ObjectDefinition pocion;
    pocion.id = "pocion";
    pocion.name = "Pocion";
    pocion.category = ObjectCategory::Pickup;
    pocion.pickup.price = 30;
    c.add(pocion);

    ObjectDefinition pan;
    pan.id = "pan";
    pan.name = "Pan";
    pan.category = ObjectCategory::Pickup;
    c.add(pan);  // sin precio de catalogo

    ObjectDefinition tendero;
    tendero.id = "tendero";
    tendero.name = "Tendero";
    tendero.category = ObjectCategory::Npc;
    tendero.dialogue.lines = {"Hola, forastero."};
    tendero.shop.items = {ShopItem{"pocion", 0}, ShopItem{"pan", 8}};
    tendero.shop.buybackPercent = 80;
    c.add(tendero);

    ObjectDefinition mudo;
    mudo.id = "mudo";
    mudo.name = "Mudo";
    mudo.category = ObjectCategory::Npc;
    c.add(mudo);

    ObjectDefinition gratis;
    gratis.id = "gratis";
    gratis.name = "Regalos";
    gratis.category = ObjectCategory::Npc;
    gratis.shop.items = {ShopItem{"pan", 0}};  // ni la tienda ni el catalogo le ponen precio
    c.add(gratis);

    ObjectDefinition barril;
    barril.id = "barril";
    barril.name = "Barril";
    barril.category = ObjectCategory::Prop;
    barril.shop.items = {ShopItem{"pocion", 0}};
    c.add(barril);
}

MOTOR_TEST_CASE(comercio_abre_la_tienda_espera_y_sigue_al_cerrarla) {
    Mundo m;
    llenarTiendas(m.catalogo);
    SessionCommandHost host(m.sesion, m.inventario);

    const CommandResult r = host.execute(cmd("comercio.abrir", {{"tienda", "tendero"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::Wait);
    MOTOR_CHECK(m.sesion.mode() == GameMode::Shop);
    MOTOR_CHECK(m.sesion.shopKeeper() == "Tendero");
    // Las ofertas salen con la misma regla que al hablar: precio de la
    // tienda si lo hay, si no el del catalogo.
    MOTOR_CHECK(m.sesion.shopOffers().size() == 2);
    MOTOR_CHECK(m.sesion.shopOffers()[0].objectId == "pocion");
    MOTOR_CHECK(m.sesion.shopOffers()[0].price == 30);
    MOTOR_CHECK(m.sesion.shopOffers()[1].price == 8);
    // Sin narrativa ni dialogo del catalogo: el evento ya es la escena.
    MOTOR_CHECK(m.sesion.dialogueLines().empty());

    // Mientras la tienda esta abierta, el evento espera.
    MOTOR_CHECK(!host.waitResolved(cmd("comercio.abrir", {{"tienda", "tendero"}})));

    // Se compra y se vende de verdad, con la recompra de ESTE tendero,
    // aunque no este en el mapa (la sesion de prueba no tiene mundo).
    m.sesion.addGold(40);
    MOTOR_CHECK(m.sesion.buy(0));
    MOTOR_CHECK(m.sesion.gold() == 10);
    MOTOR_CHECK(cuantos(m.inventario, "pocion") == 1);
    MOTOR_CHECK(m.sesion.sell(0));
    MOTOR_CHECK(m.sesion.gold() == 34);  // 30 * 80 %
    MOTOR_CHECK(m.inventario.empty());

    m.sesion.closeInteraction();
    MOTOR_CHECK(m.sesion.mode() == GameMode::Exploration);
    MOTOR_CHECK(host.waitResolved(cmd("comercio.abrir", {{"tienda", "tendero"}})));
}

MOTOR_TEST_CASE(comercio_que_no_se_puede_abrir_falla_con_motivo_y_sin_cambiar_de_modo) {
    // Esperar a una tienda que nunca se abrio colgaria el evento, y el
    // jugador se quedaria delante de un cartel que no hace nada.
    Mundo m;
    llenarTiendas(m.catalogo);
    SessionCommandHost host(m.sesion, m.inventario);

    const struct {
        const char* tienda;
        const char* motivo;
    } casos[] = {
        {"nadie", "no existe"},
        {"barril", "no es un personaje"},
        {"mudo", "nada a la venta"},
        {"gratis", "nada a la venta"},
    };
    for (const auto& c : casos) {
        const CommandResult r = host.execute(cmd("comercio.abrir", {{"tienda", c.tienda}}));
        MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
        if (r.diagnostic.find(c.motivo) == std::string::npos ||
            r.diagnostic.find(c.tienda) == std::string::npos) {
            MOTOR_CHECK_FAIL(std::string(c.tienda) + ": diagnostico inesperado: " + r.diagnostic);
        }
        MOTOR_CHECK(m.sesion.mode() == GameMode::Exploration);
        MOTOR_CHECK(m.sesion.shopOffers().empty());
    }

    // Con la partida ocupada (ya hay una tienda abierta) tampoco se abre
    // otra encima, y la que estaba sigue igual.
    MOTOR_CHECK(host.execute(cmd("comercio.abrir", {{"tienda", "tendero"}})).outcome ==
                CommandOutcome::Wait);
    const CommandResult ocupada = host.execute(cmd("comercio.abrir", {{"tienda", "tendero"}}));
    MOTOR_CHECK(ocupada.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(ocupada.diagnostic.find("no esta explorando") != std::string::npos);
    MOTOR_CHECK(m.sesion.mode() == GameMode::Shop);
    MOTOR_CHECK(m.sesion.shopOffers().size() == 2);
}

MOTOR_TEST_CASE(comercio_por_el_interprete_completo) {
    // El recorrido de verdad: texto, tienda, y lo de debajo solo corre
    // cuando el jugador cierra.
    Mundo m;
    llenarTiendas(m.catalogo);
    NarrativeState estado;
    SessionCommandHost sesionHost(m.sesion, m.inventario);
    NarrativeCommandHost host(estado, &sesionHost);

    MOTOR_CHECK(host.execute(cmd("comercio.abrir", {{"tienda", "tendero"}})).outcome ==
                CommandOutcome::Wait);
    MOTOR_CHECK(!host.waitResolved(cmd("comercio.abrir", {{"tienda", "tendero"}})));
    m.sesion.closeInteraction();
    MOTOR_CHECK(host.waitResolved(cmd("comercio.abrir", {{"tienda", "tendero"}})));
}

MOTOR_TEST_CASE(un_combate_que_no_se_puede_iniciar_falla_en_vez_de_esperar_para_siempre) {
    // Sin mundo ni catalogo de monstruos, el enemigo no se puede
    // materializar. Devolver Wait aqui dejaria al evento esperando un
    // combate que nunca empezo — y al jugador delante de un PNJ mudo.
    Mundo m;
    SessionCommandHost host(m.sesion, m.inventario);
    const CommandResult r = host.execute(cmd("combate.iniciar", {{"encuentro", "goblin"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(r.diagnostic.find("goblin") != std::string::npos);
    MOTOR_CHECK(m.sesion.mode() != GameMode::Battle);
}

MOTOR_TEST_CASE(segun_desenlace_sin_combate_delante_es_un_error) {
    // Silenciarlo daria una rama al azar: el autor veria la escena de
    // victoria despues de un combate que no ha habido.
    Mundo m;
    SessionCommandHost host(m.sesion, m.inventario);
    const CommandResult r = host.execute(cmd("control.segun_desenlace"));
    MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(r.diagnostic.find("Según desenlace") != std::string::npos);
}

MOTOR_TEST_CASE(el_catalogo_separa_lanzar_el_combate_de_ramificar_su_desenlace) {
    // Dos comandos y no uno, como el diseno de bloques (§6, B09 y B14).
    // Separarlos permite meter cosas ENTRE el combate y la ramificacion,
    // y que un combate sin ramas siga siendo legal.
    const CommandSpec* iniciar = FindCommand("combate.iniciar");
    const CommandSpec* segun = FindCommand("control.segun_desenlace");
    MOTOR_CHECK(iniciar != nullptr && segun != nullptr);
    MOTOR_CHECK(iniciar->implemented && segun->implemented);
    MOTOR_CHECK(iniciar->branches == 0);   // el que lanza NO ramifica
    MOTOR_CHECK(segun->branches == 3);     // victoria, huida, derrota
    // El comercio ya esta cableado (cierre de E-04) y su parametro es una
    // referencia a objeto: el editor lo ofrece con el selector de E-14 y
    // E-15 lo encuentra al buscar donde se usa un PNJ.
    const CommandSpec* comercio = FindCommand("comercio.abrir");
    MOTOR_CHECK(comercio != nullptr && comercio->implemented);
    MOTOR_CHECK(comercio->params.size() == 1);
    MOTOR_CHECK(comercio->params[0].type == CommandParamType::ObjectRef);
    // Ningun comando del catalogo queda declarado sin cablear.
    for (const CommandSpec& s : CommandCatalog()) {
        if (!s.implemented) MOTOR_CHECK_FAIL("sin cablear: " + s.type);
    }
}

MOTOR_TEST_CASE(el_anfitrion_narrativo_delega_en_el_de_sesion) {
    // Es el punto de costura que dejo preparado la primera rebanada: las
    // familias de estado narrativo las resuelve uno, las de partida el
    // otro, y el interprete no sabe que son dos.
    Mundo m;
    NarrativeState estado;
    SessionCommandHost sesionHost(m.sesion, m.inventario);
    NarrativeCommandHost host(estado, &sesionHost);

    MOTOR_CHECK(host.execute(cmd("bandera.encender", {{"bandera", "abierto"}})).outcome ==
                CommandOutcome::Done);
    MOTOR_CHECK(estado.hasFlag("abierto"));

    MOTOR_CHECK(host.execute(cmd("inventario.dar", {{"objeto", "llave"}})).outcome ==
                CommandOutcome::Done);
    MOTOR_CHECK(cuantos(m.inventario, "llave") == 1);
}

// ---------------------------------------------------------------------
// E-09 — El «si» que pregunta por un objeto
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(un_si_de_inventario_se_delega_a_la_sesion_y_ramifica_de_verdad) {
    // Lo encontro la mision de la llave (E-09): el anfitrion narrativo
    // resuelve el `si` el mismo y no sabe mirar el inventario, asi que
    // una puerta que pedia la llave fallaba en silencio aunque hubiera
    // sesion detras. Ahora, cuando no sabe evaluarlo, delega el comando
    // entero — la misma costura que usan las demas familias de partida.
    Mundo m;
    NarrativeState estado;
    SessionCommandHost partida(m.sesion, m.inventario);
    NarrativeCommandHost host(estado, &partida);

    const MapEventCommand si = cmd("control.si", {{"clase", "item"},
                                                  {"referencia", "llave_oxidada"}});

    // Sin la llave: rama 1 (el «si no»).
    CommandResult r = host.execute(si);
    MOTOR_CHECK(r.outcome == CommandOutcome::Branch);
    MOTOR_CHECK(r.branch == 1);

    // Con la llave: rama 0.
    MOTOR_CHECK(host.execute(cmd("inventario.dar", {{"objeto", "llave_oxidada"}})).outcome ==
                CommandOutcome::Done);
    r = host.execute(si);
    MOTOR_CHECK(r.outcome == CommandOutcome::Branch);
    MOTOR_CHECK(r.branch == 0);

    // Y con DOS llaves sigue abriendo: «lleva la llave» no es «lleva
    // exactamente una».
    MOTOR_CHECK(host.execute(cmd("inventario.dar", {{"objeto", "llave_oxidada"}})).outcome ==
                CommandOutcome::Done);
    r = host.execute(si);
    MOTOR_CHECK(r.branch == 0);
}

MOTOR_TEST_CASE(sin_sesion_detras_el_si_de_inventario_sigue_diciendo_que_le_falta) {
    // La delegacion no puede tapar el diagnostico: sin partida, el autor
    // tiene que enterarse de que esa condicion no se puede evaluar.
    NarrativeState estado;
    NarrativeCommandHost solo(estado);
    const CommandResult r = solo.execute(cmd("control.si", {{"clase", "item"},
                                                            {"referencia", "llave"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(!r.diagnostic.empty());
}

MOTOR_TEST_CASE(la_pagina_activa_de_una_puerta_mira_el_inventario) {
    // Es lo que usa el juego: la pagina «abierta» de una puerta esta
    // condicionada a llevar la llave, y sin inventario esa condicion no
    // se puede evaluar.
    MapEventCondition llave;
    llave.kind = EventConditionKind::Item;
    llave.reference = "llave_oxidada";

    MapEventPage cerrada;
    cerrada.id = ids::EventId::parse("p_cerrada").value();
    MapEventPage abierta;
    abierta.id = ids::EventId::parse("p_abierta").value();
    abierta.conditions.push_back(llave);

    MapEventDocument puerta;
    puerta.id = ids::EventId::parse("puerta").value();
    puerta.position = GridCoord{2, 2};
    puerta.pages = {cerrada, abierta};

    NarrativeState estado;
    std::vector<std::string> inventario;
    ActivePageResult activa = ActivePageWithInventory(puerta, estado, inventario);
    MOTOR_CHECK(activa.trustworthy());
    MOTOR_CHECK(activa.index == 0);

    inventario.push_back("llave_oxidada");
    activa = ActivePageWithInventory(puerta, estado, inventario);
    MOTOR_CHECK(activa.trustworthy());
    MOTOR_CHECK(activa.index == 1);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
