// AV13-10 — Al empezar un combate, cada uno en su casilla del nivel
// (Game::TableroCombate; planificacion_alcance_y_vision/COMBATE_BG3.md §3).
//
// El heroe y los enemigos, donde estaban; los acompanantes (Venides, Xila...),
// en la libre mas cercana al heroe. Bloquea lo que bloquea al andar: la
// colision de las capas y los objetos que no se pisan (PNJ, muebles); los
// enemigos que no pelean, tambien.
#include "Game/GameSession.h"
#include "Game/CartasEnCombate.h"

#include "Level/ObjectCatalog.h"
#include "Runtime/RuntimeTileMap.h"

namespace {
const std::string kAcompanante = "acompanante_";
bool esAcompanante(const Engine::WorldEntity& e) {
    return e.kind == Engine::EntityKind::Object && e.object.objectId.rfind(kAcompanante, 0) == 0;
}
}  // namespace

void GameSession::quitaAcompanantes() {
    if (sessionData()->m_world == nullptr) return;
    std::vector<Engine::EntityHandle> fuera;
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& e) {
        if (esAcompanante(e)) fuera.push_back(h);
    });
    for (const Engine::EntityHandle h : fuera) sessionData()->m_world->destroy(h);
}

void GameSession::montaTablero() {
    BattleState* b = sessionData()->m_battle.get();
    // AV13-34 — Si algun refuerzo espera el golpe mortal, el enemigo principal (el primero) lo dara.
    if (b != nullptr && sessionData()->m_pendingBattle.has_value())
        for (const auto& r : sessionData()->m_pendingBattle->refuerzos)
            if (r.golpeMortal) b->ponGolpeMortal(0, r.vidaEnemigo);
    const RuntimeTileMap* mapa = sessionData()->m_map;
    if (b == nullptr || mapa == nullptr || sessionData()->m_world == nullptr) return;  // sin nivel no hay sitio
    const std::vector<Engine::EntityHandle>& enCombate = sessionData()->m_battleEnemyHandles;
    std::vector<GridCoord> donde{sessionData()->m_playerPosition};
    std::vector<GridCoord> enemigos;
    for (const Engine::EntityHandle h : enCombate) {
        const Engine::WorldEntity* e = sessionData()->m_world->get(h);
        enemigos.push_back(e != nullptr ? e->position() : sessionData()->m_playerPosition);
        donde.push_back(enemigos.back());
    }
    auto bloquea = [this, enCombate](int x, int y) {
        if (tileBlocks(x, y)) return true;
        const Engine::EntityHandle obj = objectAt(x, y);
        if (obj.valid()) {
            const Engine::WorldEntity* e = sessionData()->m_world->get(obj);
            const ObjectDefinition* def = e != nullptr && sessionData()->m_catalog != nullptr
                                              ? sessionData()->m_catalog->find(e->object.objectId)
                                              : nullptr;
            if (def != nullptr && def->blocksMovement) return true;
        }
        const Engine::EntityHandle enemigo = enemyAt(x, y);
        if (!enemigo.valid()) return false;
        for (const Engine::EntityHandle h : enCombate)
            if (h == enemigo) return false;  // los que pelean ocupan su casilla, no la bloquean
        return true;
    };
    Game::TableroCombate t =
        Game::TableroCombate::alrededor(donde, mapa->getWidth(), mapa->getHeight(), bloquea);
    t.ponNiveles([mapa](int x, int y) { return mapa->alturas().en(x, y); });  // AV19-02: subir cuesta el doble
    const std::vector<BattleParticipant>& aliados = b->allies();
    for (std::size_t i = 0; i < aliados.size(); ++i) {
        const GridCoord deseada = sessionData()->m_playerPosition;
        if (i == 0 && t.pon(aliados[i].name, deseada)) continue;
        if (const auto libre = t.libreMasCercana(deseada)) t.pon(aliados[i].name, *libre);
    }
    for (std::size_t i = 0; i < b->enemies().size() && i < enemigos.size(); ++i) {
        if (t.pon(b->enemies()[i].name, enemigos[i])) continue;
        if (const auto libre = t.libreMasCercana(enemigos[i])) t.pon(b->enemies()[i].name, *libre);
    }
    // Si su casilla no valia (fuera del mapa, bloqueada), el heroe va a la libre mas cercana: ahi esta.
    if (const auto heroe = aliados.empty() ? std::nullopt : t.dondeEsta(aliados[0].name)) sessionData()->m_playerPosition = *heroe;
    // AV13-29 — Los acompanantes, en el mundo mientras dura el combate: un objeto
    // "acompanante_<id>" en su casilla (la Application le pone el dibujo del PNJ <id>).
    quitaAcompanantes();
    for (std::size_t i = 1; i < aliados.size(); ++i) {
        const auto donde = t.dondeEsta(aliados[i].name);
        if (!donde || aliados[i].combatant == nullptr) continue;
        ObjectSpawn s;
        s.objectId = kAcompanante + aliados[i].combatant->combatant_id();
        s.position = s.patrolMin = s.patrolMax = *donde;
        sessionData()->m_world->createObject(s);
    }
    sessionData()->m_world->flushPendingChanges();
    // AV13-28: con tablero, el turno a lo BG3 (la accion principal no lo cierra).
    if (auto* cartas = dynamic_cast<Game::CartasEnCombate*>(b->mesa())) cartas->ponTurnoBG3(true);
    // AV16-12 — Cuatro pasos de base, +1 con DES 3 o mas (heroe, acompanantes y enemigos).
    auto ponBonos = [&t](const std::vector<BattleParticipant>& quienes) {
        for (const BattleParticipant& p : quienes)
            if (p.combatant != nullptr)
                t.ponBono(p.name, Game::TableroCombate::bonoDeDestreza(p.combatant->stat(RPG::Stat::DES)));
    };
    ponBonos(aliados);
    ponBonos(b->enemies());
    // AV16-14 — Congelado y ralentizado andan la mitad; aturdido y agarrado, nada.
    if (auto* cartas = dynamic_cast<Game::CartasEnCombate*>(b->mesa())) {
        t.ponVelocidad([b, cartas](const std::string& quien) {
            const auto busca = [&quien](const std::vector<BattleParticipant>& v) -> const BattleParticipant* {
                for (const BattleParticipant& p : v)
                    if (p.name == quien) return &p;
                return nullptr;
            };
            if (const BattleParticipant* p = busca(b->allies()))
                return cartas->velocidadDe(true, static_cast<std::size_t>(p - b->allies().data()), p->combatant);
            if (const BattleParticipant* p = busca(b->enemies()))
                return cartas->velocidadDe(false, static_cast<std::size_t>(p - b->enemies().data()), p->combatant);
            return 0;
        });
    }
    b->ponTablero(std::move(t));
    sincronizaTablero();  // el que el tablero ha tenido que mover (su casilla no valia), ya en la suya
}

std::vector<GridCoord> GameSession::mueveEnCombate(GridCoord destino, std::size_t aliado) {
    BattleState* b = sessionData()->m_battle.get();
    if (b == nullptr || b->tablero() == nullptr || b->outcome() != BattleOutcome::InProgress || aliado >= b->allies().size())
        return {};
    std::vector<GridCoord> ruta = b->tablero()->anda(b->allies()[aliado].name, destino);
    if (!ruta.empty() && aliado == 0) sessionData()->m_playerPosition = destino;
    return ruta;
}

std::vector<std::pair<Engine::EntityHandle, std::pair<GridCoord, GridCoord>>> GameSession::sincronizaTablero() {
    std::vector<std::pair<Engine::EntityHandle, std::pair<GridCoord, GridCoord>>> movidos;
    BattleState* b = sessionData()->m_battle.get();
    if (b == nullptr || b->tablero() == nullptr || sessionData()->m_world == nullptr) return movidos;
    const std::vector<Engine::EntityHandle>& handles = sessionData()->m_battleEnemyHandles;
    for (std::size_t i = 0; i < handles.size() && i < b->enemies().size(); ++i) {
        const auto donde = b->tablero()->dondeEsta(b->enemies()[i].name);
        Engine::WorldEntity* e = sessionData()->m_world->get(handles[i]);
        if (!donde || e == nullptr || e->kind != Engine::EntityKind::Enemy) continue;
        const GridCoord antes = e->enemy.position;
        if (antes.x != donde->x || antes.y != donde->y) movidos.push_back({handles[i], {antes, *donde}});
        e->enemy.position = *donde;  // el espejo...
        if (e->enemy.brain) e->enemy.brain->setPosition(*donde);  // ...y la posicion de verdad (C-17)
    }
    // AV13-29 — Y los acompanantes, donde los ha dejado su turno.
    const std::vector<BattleParticipant>& aliados = b->allies();
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle h, Engine::WorldEntity& e) {
        if (!esAcompanante(e)) return;
        for (std::size_t i = 1; i < aliados.size(); ++i) {
            if (aliados[i].combatant == nullptr ||
                e.object.objectId != kAcompanante + aliados[i].combatant->combatant_id())
                continue;
            const auto donde = b->tablero()->dondeEsta(aliados[i].name);
            const GridCoord antes = e.object.position;
            if (!donde || (antes.x == donde->x && antes.y == donde->y)) return;
            movidos.push_back({h, {antes, *donde}});
            e.object.position = *donde;
            return;
        }
    });
    return movidos;
}
