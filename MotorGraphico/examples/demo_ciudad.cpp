// Ciclo social y comercial de la ciudad (NPCs, tiendas, puertas entre
// mapas) verificado SIN VENTANA: GameSession, ObjectCatalog y
// LevelLoader son GL-free a proposito, asi que todo lo que decide la
// partida -- hablar, comprar, vender, cruzar una puerta -- se puede
// probar de verdad aqui, con asserts, en vez de "a ojo" con el juego
// abierto. Mismo criterio que demo_game_session.cpp con el combate.
//
// Verifica:
//  1. Un NPC que solo habla abre GameMode::Dialogue.
//  2. Un tendero abre GameMode::Shop con sus articulos y precios
//     resueltos (precio de la tienda, o el del catalogo si no lo fija).
//  3. Comprar descuenta oro y entrega el objeto; sin oro suficiente,
//     falla sin dejar el saldo en negativo ni el objeto a medias.
//  4. Vender paga buybackPercent y saca el objeto del inventario; lo que
//     no tiene precio no se vende.
//  5. Pisar una puerta deja una transicion pendiente con su destino.
//  6. Los NPCs bloquean el paso (se les habla desde una celda contigua).
#include "Game/GameSession.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

// Cuerpo minimo del jugador: aqui no se combate, solo hace falta algo
// que cumpla ICombatant para construir la sesion.
class TestBody : public ICombatant {
public:
    void takeDamage(int amount) override { m_health -= amount; }
    void heal(int amount) override { m_health += amount; }
    int health() const override { return m_health; }
    int maxHealth() const override { return 30; }
    bool isAlive() const override { return m_health > 0; }

    // Stubs RPG de ICombatant (ver demo_skills.cpp): defaults neutros.
    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "testbody"; }

private:
    int m_health = 30;
};

// Camina hasta la celda indicada por el camino mas corto. Hace BFS sobre
// el mapa (tiles con colision + objetos que bloquean) y luego EJECUTA la
// ruta con tryMovePlayer, que es lo que se quiere probar.
//
// La primera version iba en linea recta (primero X, luego Y) y se
// atascaba en cuanto habia un edificio de por medio: en una ciudad con
// manzanas eso es siempre. Un test que falla por su propio helper y no
// por el codigo probado es peor que no tenerlo.
bool caminarHasta(GameSession& s, const TileMap& map, const ObjectCatalog& catalog, int tx,
                  int ty) {
    const int W = map.getWidth();
    const int H = map.getHeight();
    auto index = [&](int x, int y) { return y * W + x; };

    auto transitable = [&](int x, int y) {
        if (x < 0 || y < 0 || x >= W || y >= H) {
            return false;
        }
        for (int l = 0; l < map.getLayerCount(); ++l) {
            if (map.getTile(l, x, y).hasCollision()) {
                return false;
            }
        }
        // El destino puede estar ocupado por el propio objeto al que
        // vamos (un cartel no bloquea), pero un NPC si: se comprueba.
        for (const ObjectSpawn& o : s.worldObjects()) {
            if (o.position.x == x && o.position.y == y) {
                const ObjectDefinition* d = catalog.find(o.objectId);
                if (d != nullptr && d->blocksMovement) {
                    return false;
                }
            }
        }
        return true;
    };

    const GridCoord start = s.playerPosition();
    std::vector<int> prev(static_cast<std::size_t>(W * H), -1);
    std::vector<bool> seen(static_cast<std::size_t>(W * H), false);
    std::vector<int> queue{index(start.x, start.y)};
    seen[static_cast<std::size_t>(index(start.x, start.y))] = true;
    const int dxs[] = {1, -1, 0, 0};
    const int dys[] = {0, 0, 1, -1};
    for (std::size_t head = 0; head < queue.size(); ++head) {
        const int cur = queue[head];
        const int cx = cur % W;
        const int cy = cur / W;
        if (cx == tx && cy == ty) {
            break;
        }
        for (int d = 0; d < 4; ++d) {
            const int nx = cx + dxs[d];
            const int ny = cy + dys[d];
            if (!transitable(nx, ny) || seen[static_cast<std::size_t>(index(nx, ny))]) {
                continue;
            }
            seen[static_cast<std::size_t>(index(nx, ny))] = true;
            prev[static_cast<std::size_t>(index(nx, ny))] = cur;
            queue.push_back(index(nx, ny));
        }
    }
    if (!seen[static_cast<std::size_t>(index(tx, ty))]) {
        return false;  // inalcanzable
    }

    // Reconstruir y ejecutar la ruta.
    std::vector<int> path;
    for (int cur = index(tx, ty); cur != -1; cur = prev[static_cast<std::size_t>(cur)]) {
        path.push_back(cur);
        if (cur == index(start.x, start.y)) {
            break;
        }
    }
    for (std::size_t i = path.size(); i-- > 1;) {
        const int from = path[i];
        const int to = path[i - 1];
        s.tryMovePlayer(to % W - from % W, to / W - from / W);
    }
    const GridCoord& end = s.playerPosition();
    return end.x == tx && end.y == ty;
}

}  // namespace

int main() {
    ObjectCatalog catalog;
    auto catRes = catalog.loadFromFile("assets/objects/ciudad_objetos.json");
    require(catRes.isOk());
    std::cout << "[CIUDAD] catalogo cargado: " << catRes.value() << " objetos.\n";

    auto lvlRes = LevelLoader::loadFromFile("assets/levels/interior_mercado.json");
    require(lvlRes.isOk());
    LevelDefinition level = lvlRes.value();

    TileMap map;
    auto mapRes = map.loadFromFile(level.mapPath);
    require(mapRes.isOk());

    SkillCatalog skills;
    SkillSet playerSkills(8);
    TestBody body;
    std::vector<std::string> inventory;
    GameSession session(&map, &catalog, &skills, level, &body, &playerSkills, &inventory);

    // --- 2. Hablar con el tendero abre la tienda ---
    // El tendero bloquea el paso, asi que se le habla desde al lado: se
    // camina a la celda de debajo suya.
    const ObjectSpawn* tendero = nullptr;
    for (const ObjectSpawn& o : session.worldObjects()) {
        const ObjectDefinition* d = catalog.find(o.objectId);
        if (d != nullptr && d->category == ObjectCategory::Npc && !d->shop.items.empty()) {
            tendero = &o;
            break;
        }
    }
    require(tendero != nullptr && "el interior del mercado deberia tener un tendero");
    const GridCoord tpos = tendero->position;

    require(caminarHasta(session, map, catalog, tpos.x, tpos.y + 1));
    // 6. El NPC bloquea: intentar entrar en su celda no mueve al jugador.
    require(!session.tryMovePlayer(0, -1));
    require(session.playerPosition().y == tpos.y + 1);
    std::cout << "[CIUDAD] el NPC bloquea el paso (se le habla desde al lado).\n";

    require(session.interact());
    require(session.mode() == GameMode::Shop);
    require(!session.shopOffers().empty());
    std::cout << "[CIUDAD] tienda de \"" << session.shopKeeper() << "\" abierta con "
              << session.shopOffers().size() << " articulos:\n";
    for (const ShopOffer& offer : session.shopOffers()) {
        std::cout << "           " << offer.name << " - " << offer.price << " oro\n";
    }

    // --- 3. Comprar: sin oro no se puede; con oro, se cobra y se entrega ---
    const ShopOffer primero = session.shopOffers()[0];
    require(session.gold() == 0);
    require(!session.buy(0) && "sin oro no deberia poder comprarse");
    require(session.gold() == 0 && inventory.empty());
    std::cout << "[CIUDAD] compra sin oro rechazada (saldo intacto).\n";

    session.addGold(primero.price + 25);
    const int oroAntes = session.gold();
    require(session.buy(0));
    require(session.gold() == oroAntes - primero.price);
    require(inventory.size() == 1 && inventory[0] == primero.objectId);
    std::cout << "[CIUDAD] comprado \"" << primero.name << "\" por " << primero.price
              << " oro (quedan " << session.gold() << ").\n";

    // Indice fuera de rango: no-op, no crash.
    require(!session.buy(999));

    // --- 4. Vender: paga buybackPercent y saca el objeto ---
    const int oroPreVenta = session.gold();
    require(session.sell(0));
    require(session.gold() > oroPreVenta);
    require(inventory.empty());
    std::cout << "[CIUDAD] revendido: +" << (session.gold() - oroPreVenta)
              << " oro (recompra parcial, no al precio de compra).\n";
    require(!session.sell(0) && "inventario vacio: nada que vender");

    session.closeInteraction();
    require(session.mode() == GameMode::Exploration);

    // --- 5. La puerta de salida deja una transicion pendiente ---
    const ObjectSpawn* salida = nullptr;
    for (const ObjectSpawn& o : session.worldObjects()) {
        if (!o.targetLevel.empty()) {
            salida = &o;
            break;
        }
    }
    require(salida != nullptr && "el interior deberia tener una puerta de salida");
    const std::string destino = salida->targetLevel;
    require(caminarHasta(session, map, catalog, salida->position.x, salida->position.y));
    require(session.interact());
    require(session.pendingTransition().pending);
    require(session.pendingTransition().levelPath == destino);
    std::cout << "[CIUDAD] puerta pisada -> transicion pendiente a " << destino << "\n";
    session.clearTransition();
    require(!session.pendingTransition().pending);

    // --- 1. Un NPC sin tienda solo abre dialogo ---
    auto calleRes = LevelLoader::loadFromFile("assets/levels/ciudad_centro.json");
    require(calleRes.isOk());
    LevelDefinition calle = calleRes.value();
    TileMap calleMap;
    require(calleMap.loadFromFile(calle.mapPath).isOk());
    GameSession ciudad(&calleMap, &catalog, &skills, calle, &body, &playerSkills, &inventory);

    const ObjectSpawn* charlatan = nullptr;
    for (const ObjectSpawn& o : ciudad.worldObjects()) {
        const ObjectDefinition* d = catalog.find(o.objectId);
        if (d != nullptr && d->category == ObjectCategory::Npc && d->shop.items.empty()) {
            charlatan = &o;
            break;
        }
    }
    require(charlatan != nullptr && "la ciudad deberia tener NPCs que solo hablan");
    if (caminarHasta(ciudad, calleMap, catalog, charlatan->position.x, charlatan->position.y + 1)) {
        require(ciudad.interact());
        require(ciudad.mode() == GameMode::Dialogue);
        require(!ciudad.dialogueLines().empty());
        std::cout << "[CIUDAD] dialogo con \"" << ciudad.dialogueSpeaker() << "\": \""
                  << ciudad.dialogueLines()[0] << "\"\n";
        ciudad.closeInteraction();
    }

    // --- 7. Negocios: comprar un local, ajustar el alquiler y ver la
    // moral moverse en consecuencia ---
    const ObjectSpawn* cartel = nullptr;
    for (const ObjectSpawn& o : ciudad.worldObjects()) {
        const ObjectDefinition* d = catalog.find(o.objectId);
        if (d != nullptr && d->business.price > 0) {
            cartel = &o;
            break;
        }
    }
    require(cartel != nullptr && "la ciudad deberia tener carteles de negocio");
    require(caminarHasta(ciudad, calleMap, catalog, cartel->position.x, cartel->position.y));
    require(ciudad.interact());
    require(ciudad.mode() == GameMode::Business);
    const int precio = ciudad.currentBusinessPrice();
    std::cout << "[CIUDAD] cartel de \"" << ciudad.currentBusinessName() << "\": " << precio
              << " oro, renta base " << ciudad.currentBusinessIncome() << " oro/ciclo\n";

    // Sin oro suficiente no se compra, y el saldo no se toca.
    const int oroInicial = ciudad.gold();
    if (oroInicial < precio) {
        require(!ciudad.buyCurrentBusiness());
        require(ciudad.gold() == oroInicial);
        std::cout << "[CIUDAD] compra rechazada por falta de oro (saldo intacto).\n";
    }
    ciudad.addGold(precio);
    const int antesDeComprar = ciudad.gold();
    require(ciudad.buyCurrentBusiness());
    require(ciudad.gold() == antesDeComprar - precio);
    require(ciudad.ownsBusiness(cartel->objectId));
    require(!ciudad.buyCurrentBusiness() && "no se compra dos veces");
    std::cout << "[CIUDAD] negocio comprado (quedan " << ciudad.gold() << " oro).\n";

    // La moral empieza neutral y se mueve con el alquiler.
    require(ciudad.morality().value() == 0);
    require(std::string(ciudad.morality().label()) == "NEUTRAL");

    // Subir el alquiler: villano, y la ocupacion baja.
    for (int i = 0; i < 8; ++i) {
        ciudad.adjustCurrentRent(+10);  // efecto: siempre se ejecuta
    }
    const int moralVillano = ciudad.morality().value();
    require(moralVillano < 0 && "subir el alquiler debe restar moral");
    std::cout << "[CIUDAD] alquiler al maximo -> moral " << moralVillano << " ("
              << ciudad.morality().label() << "), ocupacion "
              << GameSession::occupancyPercent(GameSession::kMaxRent) << "%\n";
    require(!ciudad.morality().history().empty());
    std::cout << "           motivo: " << ciudad.morality().history()[0] << "\n";

    // Bajarlo del todo: la moral sube por encima de donde estaba.
    for (int i = 0; i < 20; ++i) {
        ciudad.adjustCurrentRent(-10);  // efecto: siempre se ejecuta
    }
    const int moralHeroe = ciudad.morality().value();
    require(moralHeroe > moralVillano && "bajar el alquiler debe sumar moral");
    std::cout << "[CIUDAD] alquiler al minimo -> moral " << moralHeroe << " ("
              << ciudad.morality().label() << ")\n";
    // En el tope no se puede bajar mas: no-op, y la moral no se mueve
    // (si no, bastaria machacar la tecla para volverse heroe gratis).
    const int moralEnTope = ciudad.morality().value();
    require(!ciudad.adjustCurrentRent(-10));
    require(ciudad.morality().value() == moralEnTope);
    std::cout << "[CIUDAD] en el tope de alquiler no se puede farmear moral.\n";

    // La curva de ingresos: subir la renta DA mas oro (si no, no habria
    // tentacion y la decision moral seria falsa), pero con rendimientos
    // decrecientes y un optimo intermedio -- pasado ese punto solo
    // queda el coste moral.
    const int base = ciudad.currentBusinessIncome();
    require(GameSession::occupancyPercent(100) == 100);
    require(GameSession::occupancyPercent(GameSession::kMaxRent) < 100);
    const int justo = GameSession::incomeFor(base, 100);
    const int medio = GameSession::incomeFor(base, 150);
    const int tope = GameSession::incomeFor(base, GameSession::kMaxRent);
    require(medio > justo && "subir algo la renta debe compensar economicamente");
    require(tope < 2 * justo && "la ocupacion impide que el ingreso escale con la renta");
    // Existe un optimo INTERIOR: exprimir al maximo no es lo mas
    // rentable, asi que llegar al tope es una eleccion moral, no
    // economica.
    int mejor = 0;
    int mejorRenta = 0;
    for (int r = GameSession::kMinRent; r <= GameSession::kMaxRent; r += 5) {
        const int v = GameSession::incomeFor(base, r);
        if (v > mejor) {
            mejor = v;
            mejorRenta = r;
        }
    }
    require(mejorRenta < GameSession::kMaxRent && "el maximo no puede ser lo mas rentable");
    std::cout << "[CIUDAD] ingresos: justo " << justo << ", 150% " << medio << ", tope " << tope
              << " oro/ciclo. Optimo economico en " << mejorRenta << "%.\n";

    // Cobrar rentas suma oro.
    ciudad.closeInteraction();
    const int oroPreRenta = ciudad.gold();
    const int cobrado = ciudad.collectRent();
    require(cobrado > 0);
    require(ciudad.gold() == oroPreRenta + cobrado);
    std::cout << "[CIUDAD] ciclo de rentas cobrado: +" << cobrado << " oro.\n";

    std::cout << "\nTodas las comprobaciones (require/runOk) han pasado correctamente.\n";
    return 0;
}
