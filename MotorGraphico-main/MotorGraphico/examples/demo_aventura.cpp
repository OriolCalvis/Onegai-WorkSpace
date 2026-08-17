// La LINEA DE EVENTOS del juego, verificada SIN VENTANA.
//
// Este demo es la prueba de que el sistema narrativo (RPG/NarrativeEngine.h)
// funciona y, sobre todo, la MEJOR DOCUMENTACION de como se escribe una
// aventura: si quieres entender el sistema, lee este archivo junto a
// assets/adventures/aventura_silbido_medianoche.json.
//
// La aventura de ejemplo es deliberadamente la mas simple que sigue
// siendo interesante: hablar con tres PNJs de la plaza (guardia, anciana,
// bardo) EN CUALQUIER ORDEN, y cuando se tienen las tres pistas, un beat
// automatico cierra la historia. Ni combate, ni ramas, ni objetos.
//
// Verifica:
//   PARTE A -- el motor narrativo, aislado:
//     1. La aventura carga y sus objetivos empiezan todos sin cumplir.
//     2. Hablar con un PNJ dispara su beat y enciende su flag.
//     3. Volver a hablarle da el beat de "ya me lo has contado" (el
//        primero que encaja gana) y NO vuelve a tocar el estado.
//     4. El cierre NO salta con dos de tres pistas.
//     5. El cierre salta con las tres, EN LAS 6 PERMUTACIONES de orden.
//     6. El cierre se dispara UNA sola vez y paga su oro una sola vez.
//     7. Un JSON con un trigger o un efecto desconocido es Error de
//        carga, no un beat que nunca se dispara en silencio.
//
//   PARTE B -- enchufado a una partida real (GameSession + ciudad_centro):
//     8. Un PNJ con beat activo dice las lineas de la AVENTURA en vez de
//        las lineas fijas de su ObjectDefinition.
//     9. Al cerrar el ultimo dialogo, el cierre automatico salta solo y
//        deja la sesion en Dialogue con la voz del narrador.
//    10. El oro del cierre llega de verdad a la partida.
#include "Game/GameSession.h"
#include "Game/Skill.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "RPG/NarrativeEngine.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char* kAventura = "assets/adventures/aventura_silbido_medianoche.json";

// Cuerpo minimo del jugador (ver demo_ciudad.cpp): aqui no se combate.
class TestBody : public ICombatant {
public:
    void takeDamage(int amount) override { m_health -= amount; }
    void heal(int amount) override { m_health += amount; }
    int health() const override { return m_health; }
    int maxHealth() const override { return 30; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "testbody"; }

private:
    int m_health = 30;
};

// Camina hasta (tx,ty) por el camino mas corto (BFS sobre colision de
// tiles + objetos que bloquean) y EJECUTA la ruta con tryMovePlayer.
// Mismo helper que demo_ciudad.cpp: en una ciudad con manzanas, ir en
// linea recta se atasca contra el primer edificio.
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
        return false;
    }

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

// Posicion del spawn de un objectId en el nivel cargado.
bool posicionDe(const GameSession& s, const std::string& objectId, GridCoord& out) {
    for (const ObjectSpawn& o : s.worldObjects()) {
        if (o.objectId == objectId) {
            out = o.position;
            return true;
        }
    }
    return false;
}

// Habla con el NPC de "objectId": camina a una celda contigua libre y
// pulsa interactuar. Los NPCs bloquean el paso, por eso se prueban las
// cuatro vecinas hasta que una sea alcanzable.
bool hablarCon(GameSession& s, const TileMap& map, const ObjectCatalog& catalog,
               const std::string& objectId) {
    GridCoord pos{};
    if (!posicionDe(s, objectId, pos)) {
        return false;
    }
    const int dxs[] = {0, 0, -1, 1};
    const int dys[] = {1, -1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        if (caminarHasta(s, map, catalog, pos.x + dxs[i], pos.y + dys[i])) {
            return s.interact();
        }
    }
    return false;
}

// --- PARTE A: el motor narrativo aislado -------------------------------

// Recorre las tres pistas en el orden dado y devuelve cuanto oro ha
// pagado la aventura por el camino. Cada "hablar" va seguido de un
// tick(), que es como lo hace la partida real (GameSession lo llama al
// cerrar el dialogo).
int recorrerEnOrden(const RPG::NarrativeEngine& engine, const std::vector<std::string>& orden,
                    RPG::NarrativeState& state) {
    int oro = 0;
    for (const std::string& npc : orden) {
        const RPG::NarrativeResult hablado = engine.talkTo(npc, state);
        require(hablado.fired && "cada PNJ de la aventura debe tener algo que decir");
        oro += hablado.goldDelta;
        oro += engine.tick(state).goldDelta;
    }
    return oro;
}

void parteA(const RPG::NarrativeEngine& engine) {
    std::cout << "\n=== PARTE A: el motor narrativo aislado ===\n";

    // 1. Estado inicial: ningun objetivo cumplido.
    {
        RPG::NarrativeState state;
        const std::vector<RPG::ObjectiveStatus> objetivos = engine.objectives(state);
        require(objetivos.size() == 3);
        for (const RPG::ObjectiveStatus& o : objetivos) {
            require(!o.done);
            std::cout << "  [ ] " << o.text << "\n";
        }
        require(!engine.allObjectivesDone(state));
    }

    // 2 y 3. Hablar enciende la flag; volver a hablar da el beat de
    // repeticion y NO cambia el estado.
    {
        RPG::NarrativeState state;
        const RPG::NarrativeResult primera = engine.talkTo("guardia", state);
        require(primera.fired);
        require(primera.beatId == "beat_guardia_pista");
        require(state.hasFlag("silbido_pista_guardia"));
        const std::size_t flagsTrasPrimera = state.flagCount();

        const RPG::NarrativeResult segunda = engine.talkTo("guardia", state);
        require(segunda.fired);
        require(segunda.beatId == "beat_guardia_repite" &&
                "el segundo beat del guardia debe ganar cuando su flag ya esta encendida");
        require(state.flagCount() == flagsTrasPrimera && "repetir no debe cambiar el estado");
        require(segunda.goldDelta == 0);
        std::cout << "  hablar dos veces -> \"" << primera.beatId << "\" y luego \""
                  << segunda.beatId << "\".\n";

        // 4. Con dos de tres pistas, el cierre NO salta.
        require(engine.talkTo("anciana", state).fired);
        require(!engine.tick(state).fired && "el cierre no debe saltar con 2 de 3 pistas");
        require(!engine.allObjectivesDone(state));
        std::cout << "  con 2 de 3 pistas el cierre sigue callado.\n";
    }

    // 5. Las 6 permutaciones de orden resuelven la aventura igual. Esto
    // es LO QUE SE ESTA PROBANDO de verdad: que la historia no depende
    // del orden en que el jugador se encuentre a la gente.
    {
        std::vector<std::string> orden{"anciana", "bardo", "guardia"};
        std::sort(orden.begin(), orden.end());
        int permutaciones = 0;
        do {
            RPG::NarrativeState state;
            const int oro = recorrerEnOrden(engine, orden, state);

            require(engine.allObjectivesDone(state));
            require(state.hasFlag("silbido_resuelta") && "el cierre debe haber saltado");
            require(oro == 30 && "el cierre paga 30 de oro, exactamente una vez");

            // 6. Y no se repite: mas ticks no vuelven a pagar.
            require(!engine.tick(state).fired);
            require(!engine.tick(state).fired);

            std::cout << "  orden [" << orden[0] << ", " << orden[1] << ", " << orden[2]
                      << "] -> resuelta, +" << oro << " oro.\n";
            ++permutaciones;
        } while (std::next_permutation(orden.begin(), orden.end()));
        require(permutaciones == 6);
    }

    // 7. Contenido invalido = Error de carga ruidoso, no beat mudo.
    {
        const std::string triggerMalo =
            R"({"id":"x","beats":[{"id":"b","trigger":{"type":"tak","target":"guardia"}}]})";
        auto r1 = RPG::AdventureScript::loadFromString(triggerMalo);
        require(!r1.isOk());

        const std::string efectoMalo =
            R"({"id":"x","beats":[{"id":"b","trigger":{"type":"auto"},)"
            R"("effects":[{"type":"setFlagg","arg":"f"}]}]})";
        auto r2 = RPG::AdventureScript::loadFromString(efectoMalo);
        require(!r2.isOk());

        const std::string objetivoSinFlag =
            R"({"id":"x","objectives":[{"id":"o","text":"t"}],"beats":[]})";
        auto r3 = RPG::AdventureScript::loadFromString(objetivoSinFlag);
        require(!r3.isOk());

        std::cout << "  contenido invalido rechazado: \"" << r1.errorMessage() << "\"\n";
    }
}

// --- PARTE B: enchufado a una partida real -----------------------------

void parteB(const RPG::NarrativeEngine& engine) {
    std::cout << "\n=== PARTE B: la aventura dentro de una partida real ===\n";

    ObjectCatalog catalog;
    auto catRes = catalog.loadFromFile("assets/objects/ciudad_objetos.json");
    require(catRes.isOk());

    auto lvlRes = LevelLoader::loadFromFile("assets/levels/ciudad_centro.json");
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

    RPG::NarrativeState state;
    session.setNarrative(&engine, &state);

    // El beat de apertura se dispara al entrar en el nivel. Lo lanza
    // quien carga los niveles, no la sesion (ver enterLevelNarrative).
    require(session.enterLevelNarrative("assets/levels/ciudad_centro.json"));
    require(session.mode() == GameMode::Dialogue);
    require(session.dialogueSpeaker().empty() && "la apertura la cuenta el narrador, no un PNJ");
    std::cout << "  apertura: \"" << session.dialogueLines().front() << "\"\n";
    session.closeInteraction();
    require(session.mode() == GameMode::Exploration);

    // 8. El PNJ dice lo de la AVENTURA, no lo de su ObjectDefinition.
    const ObjectDefinition* guardiaDef = catalog.find("guardia");
    require(guardiaDef != nullptr);
    const std::string lineaDeCatalogo = guardiaDef->dialogue.lines.front();

    require(hablarCon(session, map, catalog, "guardia"));
    require(session.mode() == GameMode::Dialogue);
    require(!session.dialogueLines().empty());
    require(session.dialogueLines().front() != lineaDeCatalogo &&
            "con una aventura activa, la narrativa manda sobre el dialogo fijo");
    require(state.hasFlag("silbido_pista_guardia"));
    std::cout << "  guardia (catalogo) : \"" << lineaDeCatalogo << "\"\n";
    std::cout << "  guardia (aventura) : \"" << session.dialogueLines().front() << "\"\n";
    session.closeInteraction();
    require(session.mode() == GameMode::Exploration && "aun faltan pistas: nada automatico");

    require(hablarCon(session, map, catalog, "anciana"));
    require(state.hasFlag("silbido_pista_anciana"));
    session.closeInteraction();
    require(session.mode() == GameMode::Exploration);

    const int oroAntes = session.gold();
    require(hablarCon(session, map, catalog, "bardo"));
    require(state.hasFlag("silbido_pista_bardo"));

    // 9. Al cerrar el ULTIMO dialogo, el cierre salta solo: la sesion no
    // vuelve a Exploration, se queda en Dialogue con la voz del narrador.
    session.closeInteraction();
    require(session.mode() == GameMode::Dialogue &&
            "el cierre automatico deberia haber abierto su propio dialogo");
    require(session.dialogueSpeaker().empty());
    require(state.hasFlag("silbido_resuelta"));
    std::cout << "  cierre automatico:\n";
    for (const std::string& linea : session.dialogueLines()) {
        std::cout << "      " << linea << "\n";
    }

    // 10. Y el oro llega a la partida.
    require(session.gold() == oroAntes + 30);
    std::cout << "  oro de la partida: " << oroAntes << " -> " << session.gold() << ".\n";

    // Cerrar otra vez ya no dispara nada.
    session.closeInteraction();
    require(session.mode() == GameMode::Exploration);

    require(engine.allObjectivesDone(state));
    std::cout << "  diario de misiones final:\n";
    for (const RPG::ObjectiveStatus& o : engine.objectives(state)) {
        std::cout << "      [" << (o.done ? "x" : " ") << "] " << o.text << "\n";
    }
}

}  // namespace

int main() {
    auto advRes = RPG::AdventureScript::loadFromFile(kAventura);
    if (!advRes.isOk()) {
        std::cerr << "No se pudo cargar la aventura: " << advRes.errorMessage() << "\n";
        return 1;
    }
    const RPG::AdventureScript aventura = advRes.value();
    std::cout << "[AVENTURA] \"" << aventura.name << "\" cargada: " << aventura.beats.size()
              << " beats, " << aventura.objectives.size() << " objetivos.\n";
    std::cout << "           " << aventura.description << "\n";

    RPG::NarrativeEngine engine;
    engine.setAdventure(&aventura);

    parteA(engine);
    parteB(engine);

    std::cout << "\n[AVENTURA] todas las comprobaciones han pasado.\n";
    return 0;
}
