// D0027, D0030 — Morir en un combate: el heroe revive al inicio de ese mismo combate (en su primera
// oleada), con una cicatriz mas y el coste moral de que lo vean caer. Ver decisiones/D0027-*.md.
//
// La foto se toma al empezar el combate (startBattle -> tomaFoto) y se devuelve al morir
// (reviveAlInicio): el heroe (vida, mana, mochila, oro, casilla), los enemigos (vida, mana,
// casilla), el pedido de la historia con sus refuerzos y las flags. Las curas
// de los acompanantes y los estados de la mesa no hacen falta: las primeras solo se apuntan al
// cerrar el combate, y la mesa se monta de nuevo en cada startBattle.
#include "Game/GameSession.h"

#include <algorithm>
#include <set>

#include "Game/Cicatrices.h"
#include "Game/MoralDeLaSesion.h"
#include "Game/ReglasMoral.h"
#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "RPG/CharacterSheet.h"
#include "../RPG/Narrative/BeatsAventura.h"
#include "RPG/RandomEngine.h"
#include "RPG/Narrative/NarrativeState.h"

namespace {

// Lleva una vida a un valor exacto por la interfaz de siempre (heal / takeDamage).
void ponVida(ICombatant& quien, int vida) {
    const int ahora = quien.health();
    if (vida > ahora) quien.heal(vida - ahora);
    else if (vida < ahora) quien.takeDamage(ahora - vida);
}

}  // namespace

namespace Game {

// CAER-01 — Quien cae en un combate de la historia en el que nadie muere se levanta con un cuarto de
// su vida (1 como poco). Devuelve con cuanta.
int levantaHeroe(ICombatant& heroe) {
    const int objetivo = std::max(1, heroe.maxHealth() / 4);
    if (heroe.health() < objetivo) heroe.heal(objetivo - heroe.health());
    return heroe.health();
}

std::string pagaLaHuida(int& oro, RPG::RandomEngine& dado) {
    if (oro <= 0) return "";
    const int cara = dado.roll_d6();
    const int perdido = cara <= 3 ? std::max(1, oro * (4 - cara) / 10) : 0;
    oro -= perdido;
    return "Tiras un dado al huir: sale un " + std::to_string(cara) +
           (perdido > 0 ? ". Se te caen " + std::to_string(perdido) + " de oro." : ". No pierdes nada.");
}

}  // namespace Game

void GameSession::ponRutaCicatrices(const std::string& ruta) {
    m_muerte.ruta = ruta;
    m_muerte.cicatrices = ruta.empty() ? Game::Cicatrices{} : Game::Cicatrices::carga(ruta);
}

static std::string quienEs(const RPG::CharacterSheet* ficha) {
    if (ficha == nullptr) return "heroe";
    return ficha->id.empty() ? ficha->displayName : ficha->id;
}

int GameSession::cicatrices() const { return m_muerte.cicatrices.de(quienEs(characterSheet())); }

void GameSession::tomaFoto(const std::vector<Engine::EntityHandle>& enemies,
                           const std::vector<RPG::AcompananteDef>& aLado) {
    SessionData& d = *sessionData();
    if (m_muerte.sigueLaOleada && m_muerte.foto) return;
    Game::FotoCombate foto;
    foto.handles = enemies;
    foto.aLado = aLado;
    if (d.m_playerBody != nullptr) foto.vida = d.m_playerBody->health();
    if (d.m_playerSkills != nullptr) foto.mana = d.m_playerSkills->mana();
    if (d.m_inventory != nullptr) foto.mochila = *d.m_inventory;
    foto.oro = d.m_gold;
    foto.casilla = d.m_playerPosition;
    for (const Engine::EntityHandle h : enemies) {
        Game::FotoCombate::Enemigo e;
        const Engine::WorldEntity* ent = d.m_world->get(h);
        if (ent != nullptr && ent->kind == Engine::EntityKind::Enemy) {
            if (ent->enemy.brain) e.vida = ent->enemy.brain->health();
            if (ent->enemy.skills) e.mana = ent->enemy.skills->mana();
            e.casilla = ent->enemy.position;
        }
        foto.enemigos.push_back(e);
    }
    foto.pedido = d.m_pendingBattle;
    foto.guardado = captureSaveGame("");
    for (const Engine::EntityHandle h : enemies) {
        const Engine::WorldEntity* ent = d.m_world->get(h);
        if (ent != nullptr && ent->kind == Engine::EntityKind::Enemy)
            foto.guardado.combate.enemigos.push_back({ent->enemy.summoned, ent->enemy.spawnIndex, ent->enemy.objectId});
    }
    for (const RPG::AcompananteDef& def : aLado) foto.guardado.combate.aLado.push_back(def.id);
    if (d.m_narrativeState != nullptr) {
        foto.flags = d.m_narrativeState->flags();
        foto.variables = d.m_narrativeState->variables();
    }
    m_muerte.foto = std::move(foto);
}

bool GameSession::reviveAlInicio() {
    SessionData& d = *sessionData();
    if (!m_muerte.foto || d.m_battle == nullptr || d.m_playerBody == nullptr) return false;
    // D0027 §6: las peleas en las que nadie muere siguen como siempre (te levantas y sigue la historia).
    if (d.m_pendingBattle.has_value() && d.m_pendingBattle->sinMatar) return false;
    // D0031: si con la derrota ya saltaria el beat de rescate, no se reinicia: el combate se cierra
    // como una derrota de las que se levantan y el beat sigue la historia. Se prueba en una copia.
    if (d.m_pendingBattle.has_value() && !d.m_pendingBattle->rescateAlMorir.empty() && d.m_narrative != nullptr &&
        d.m_narrativeState != nullptr) {
        RPG::NarrativeState copia = *d.m_narrativeState;
        copia.setFlag(d.m_pendingBattle->flagDefeat);
        if (d.m_narrative->tick(copia).beatId == d.m_pendingBattle->rescateAlMorir) {
            d.m_pendingBattle->ineludible = false;
            return false;
        }
    }
    // Los enemigos tienen que seguir ahi para volver a montar el mismo combate; los de una oleada
    // ya ganada no siguen, pero se vuelven a invocar con su pedido.
    if (m_muerte.foto->oleadaGanada && !m_muerte.foto->pedido.has_value()) return false;
    for (const Engine::EntityHandle h : m_muerte.foto->handles) {
        const Engine::WorldEntity* ent = d.m_world->get(h);
        if (!m_muerte.foto->oleadaGanada && (ent == nullptr || ent->kind != Engine::EntityKind::Enemy)) return false;
    }
    Game::FotoCombate foto = std::move(*m_muerte.foto);
    m_muerte.foto.reset();

    // La muerte: una cicatriz (fuera de la partida guardada) y un acto en la moral.
    ++m_muerte.n;
    const int cuantas = m_muerte.cicatrices.suma(quienEs(characterSheet()));
    if (!m_muerte.ruta.empty() && !m_muerte.cicatrices.guarda(m_muerte.ruta))
        logLine("(No se pudo guardar la cicatriz en " + m_muerte.ruta + ")");
    // Las tres notas cortas van tambien al registro del combate nuevo (el jugador lee ese, no el de la partida).
    const std::vector<std::string> notas{"Has caído derrotado.", Game::Cicatrices::nota(cuantas), "Vuelves al inicio del combate."};
    logLine(notas[0]);
    logLine(notas[1]);
    const std::string& acto = moral::reglas().parametros.actoAlMorir;
    if (!acto.empty())
        for (const std::string& linea : moral::apuntaActos(d, {acto})) logLine(linea);
    logLine(notas[2]);

    // El estado de entonces.
    ponVida(*d.m_playerBody, foto.vida);
    if (d.m_playerSkills != nullptr) d.m_playerSkills->ponMana(foto.mana);
    if (d.m_inventory != nullptr) *d.m_inventory = foto.mochila;
    d.m_gold = foto.oro;
    d.m_playerPosition = foto.casilla;
    for (std::size_t i = 0; i < foto.handles.size() && i < foto.enemigos.size(); ++i) {
        Engine::WorldEntity* ent = d.m_world->get(foto.handles[i]);
        if (ent == nullptr) continue;
        if (ent->enemy.brain) {
            ponVida(*ent->enemy.brain, foto.enemigos[i].vida);
            ent->enemy.brain->setPosition(foto.enemigos[i].casilla);
        }
        if (ent->enemy.skills) ent->enemy.skills->ponMana(foto.enemigos[i].mana);
        ent->enemy.position = foto.enemigos[i].casilla;
    }
    d.m_pendingBattle = foto.pedido;
    if (d.m_narrativeState != nullptr) {
        const std::set<std::string> antes(foto.flags.begin(), foto.flags.end());
        for (const std::string& flag : d.m_narrativeState->flags())
            if (antes.count(flag) == 0) d.m_narrativeState->clearFlag(flag);  // las de un refuerzo que ya no ha llegado
        for (const auto& [nombre, valor] : d.m_narrativeState->variables()) {
            const auto entonces = foto.variables.find(nombre);
            const int vuelve = entonces != foto.variables.end() ? entonces->second : 0;
            if (valor != vuelve) d.m_narrativeState->setVariable(nombre, vuelve);
        }
    }

    // Y el combate, desde el principio (el BattleState nuevo sustituye al viejo).
    if (foto.oleadaGanada) {
        // Se cayo en una oleada posterior: sus enemigos se van y vuelve la primera, con las que le seguian.
        for (const Engine::EntityHandle h : d.m_battleEnemyHandles) d.m_world->destroy(h);
        d.m_battleEnemyHandles.clear();
        d.m_battle.reset();
        d.m_pendingBattle.reset();
        quitaAcompanantes();
        d.m_battleQueue = foto.cola;
        resolveBattles({*foto.pedido});
    } else {
        startBattle(foto.handles, foto.aLado);
        if (d.m_battle != nullptr && foto.pedido.has_value() && foto.pedido->ineludible) d.m_battle->prohibeHuida();
    }
    // En combate el jugador lee el registro de la batalla, no el de la partida: que ahi tambien se entienda.
    if (d.m_battle != nullptr)
        for (const std::string& linea : notas) d.m_battle->anota(linea);
    return true;
}

// D0033 — Guardar en combate guarda su inicio: un combate no se guarda a medias, se repite (como al morir).
std::optional<SaveGameData> GameSession::guardadoDelCombate(const std::string& levelPath) const {
    const SessionData& d = *sessionData();
    if (d.m_battle == nullptr || !m_muerte.foto || !m_muerte.foto->guardado.combate.hay()) return std::nullopt;
    const Game::FotoCombate& foto = *m_muerte.foto;
    SaveGameData data = foto.guardado;
    data.levelPath = levelPath;
    if (foto.pedido.has_value()) {
        std::vector<RPG::NarrativeEffect> pedidos(1);
        pedidos[0].type = RPG::EffectType::StartBattle;
        pedidos[0].battle = *foto.pedido;
        for (const RPG::BattleRequest& siguiente : foto.oleadaGanada ? foto.cola : d.m_battleQueue) {
            pedidos.emplace_back();
            pedidos.back().type = RPG::EffectType::StartBattle;
            pedidos.back().battle = siguiente;
        }
        JsonWriter w;
        w.beginObject();
        RPG::beats::escribeEfectos(w, pedidos);
        w.endObject();
        data.combate.pedidos = w.str();
    }
    return data;
}

bool GameSession::reanudaCombate(const SaveGameData& data) {
    SessionData& d = *sessionData();
    if (!data.combate.hay()) return false;
    const RPG::AdventureScript* aventura = d.m_narrative != nullptr ? d.m_narrative->adventure() : nullptr;
    std::vector<RPG::BattleRequest> pedidos;
    if (!data.combate.pedidos.empty()) {
        const auto json = JsonValue::parse(data.combate.pedidos);
        if (!json.isOk()) return false;
        const auto efectos = RPG::beats::leeEfectos(json.value()["effects"], "guardado.combate",
                                                    aventura != nullptr ? aventura->acompanantes : std::vector<RPG::AcompananteDef>{});
        if (!efectos.isOk()) return false;
        for (const RPG::NarrativeEffect& e : efectos.value()) pedidos.push_back(e.battle);
    }
    std::vector<Engine::EntityHandle> handles;
    for (const CombateGuardado::Enemigo& quien : data.combate.enemigos) {
        Engine::EntityHandle hallado;
        d.m_world->eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& ent) {
            if (!hallado.valid() && ent.kind == Engine::EntityKind::Enemy && ent.enemy.summoned == quien.summoned &&
                ent.enemy.spawnIndex == quien.spawnIndex && ent.enemy.objectId == quien.objectId &&
                std::find(handles.begin(), handles.end(), h) == handles.end())
                hallado = h;
        });
        if (!hallado.valid()) return false;
        handles.push_back(hallado);
    }
    std::vector<RPG::AcompananteDef> aLado;
    for (const std::string& id : data.combate.aLado) {
        const RPG::AcompananteDef* def = aventura != nullptr ? aventura->acompanante(id) : nullptr;
        if (def != nullptr) aLado.push_back(*def);
    }
    if (!pedidos.empty()) {
        d.m_pendingBattle = pedidos.front();
        d.m_battleQueue.assign(pedidos.begin() + 1, pedidos.end());
    }
    startBattle(handles, aLado);
    if (d.m_battle == nullptr) return false;
    if (d.m_pendingBattle.has_value() && d.m_pendingBattle->ineludible) d.m_battle->prohibeHuida();
    d.m_battle->anota("Vuelves al inicio del combate.");
    return true;
}
