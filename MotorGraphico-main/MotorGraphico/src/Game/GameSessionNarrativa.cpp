// Lo que la aventura le pide a la sesion.
//
// El motor narrativo es puro: no tira dados ni invoca enemigos. Cuando
// un beat pide una tirada o un combate, lo devuelve en su resultado y
// es AQUI donde se resuelve contra el mundo de verdad.
//
// Salio de src/Game/GameSession.cpp el 23/09/2026 (EST-23): aquel fichero
// tenia 1167 lineas y lo cruzan seis paquetes del plan (C-04, C-17, D-08,
// E-04, G-12, G-14) — de los sitios donde dos agentes se pisan sin
// enterarse. Se movio, no se reescribio: ni un cuerpo tocado.
//
// El estado sigue viviendo en GameSession (40 miembros). Sacarlo a una
// clase de datos es el otro medio relevo y va aparte, porque tocaria cada
// m_ de las mil lineas y el diff dejaria de poder leerse.


#include "Game/GameSession.h"
#include "Game/MoralDeLaSesion.h"  // MORAL-01

#include "RPG/CharacterSheet.h"  // H2-04
#include "RPG/Narrative/HeroeEnLaHistoria.h"  // HEROE-01
#include "RPG/InventoryEngine.h"  // G-12

#include "Level/ObjectCatalog.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "Runtime/RuntimeTileMap.h"

#include <algorithm>
#include <map>
#include <utility>

namespace {

// HEROE-01 — Antes de cada disparo, la historia sabe quien es el heroe: la
// raza, la clase, el origen, la deidad, las siete respuestas de la historia y
// los atributos de la ficha, como flags `heroe_*`, variables `heroe_*` y los
// textos de {nombre}, {raza}... Se lee de los catalogos una vez por ficha.
void reflejaHeroe(SessionData& d) {
    if (d.m_narrativeState == nullptr || d.m_playerBody == nullptr) return;
    moral::reflejaLaReputacion(d);  // BND-04
    const RPG::CharacterSheet* ficha = d.m_playerBody->character_sheet();
    if (ficha == nullptr) return;
    static std::map<std::string, RPG::HeroeEnLaHistoria> cache;
    const std::string clave = RPG::HeroeEnLaHistoria::clave(*ficha);
    auto it = cache.find(clave);
    if (it == cache.end()) it = cache.emplace(clave, RPG::HeroeEnLaHistoria::desde(*ficha)).first;
    it->second.refleja(*d.m_narrativeState);
}


// NIVELES-01 — Los niveles y los hitos que concede un beat van a la ficha
// del heroe, con las reglas de tier del nivel. Veinte niveles por tier y un
// hito para pasar: sin el hito el heroe se queda en el nivel 20 y en Tier 1.
// Si sube de tier, su tope de vida sube y la vida ganada se le suma: subir
// no deja al heroe mas herido de lo que estaba. Devuelve las lineas del
// registro.
std::vector<std::string> concedeNiveles(SessionData& d, int puntos, int hitos) {
    std::vector<std::string> lineas;
    if ((puntos <= 0 && hitos <= 0) || d.m_playerBody == nullptr) return lineas;
    RPG::CharacterSheet* ficha = d.m_playerBody->character_sheet();
    if (ficha == nullptr) return lineas;
    static const RPG::TierRules kSinReglas;
    const RPG::TierRules& reglas = d.m_inventoryRules.tiers != nullptr ? *d.m_inventoryRules.tiers : kSinReglas;
    const int topeAntes = d.m_playerBody->maxHealth();
    const int tierAntes = ficha->tier;
    const int nivelAntes = ficha->nivel;
    ficha->grantMilestones(hitos, reglas);
    ficha->grantLevelPoints(puntos, reglas);
    if (hitos > 0) {
        std::string linea = hitos == 1 ? "Consigues un hito" : "Consigues " + std::to_string(hitos) + " hitos";
        // Los del tier en el que se queda: «1 de 2 para Tier 2».
        if (ficha->tier == tierAntes && ficha->tier < RPG::TIER_MAX) {
            linea += " (" + std::to_string(ficha->hitosTier) + " de " +
                     std::to_string(RPG::CharacterSheet::hitosParaSalirDe(ficha->tier)) + " para Tier " +
                     std::to_string(ficha->tier + 1) + ")";
        }
        lineas.push_back(linea + ".");
    }
    if (ficha->nivel != nivelAntes) lineas.push_back("Subes a nivel " + std::to_string(ficha->nivel) + ".");
    if (puntos > 0 && ficha->nivel == ficha->nivelMaximo()) {
        lineas.push_back("Nivel " + std::to_string(ficha->nivel) + ": hace falta un hito para seguir subiendo.");
    } else if (puntos > 0 && ficha->progresoNivel > 0) {
        lineas.push_back("Nivel " + std::to_string(ficha->nivel) + ", " + std::to_string(ficha->progresoNivel) +
                         " % hacia el " + std::to_string(ficha->nivel + 1) + ".");
    }
    // Los beats que piden nivel o tier lo leen del estado de la aventura:
    // al cerrar este dialogo, el tick ya ve al heroe nuevo (la necesidad
    // que surge al llegar).
    if (d.m_narrativeState != nullptr) d.m_narrativeState->ponHeroe(ficha->nivel, ficha->tier);
    if (ficha->tier != tierAntes) {
        const int ganada = d.m_playerBody->maxHealth() - topeAntes;
        if (ganada > 0) d.m_playerBody->heal(ganada);
        lineas.push_back("Subes a Tier " + std::to_string(ficha->tier) + ".");
    }
    return lineas;
}

// FASE1-BND-01 — La tabla aleatoria (el 1d4 del traje roto): se tira con el
// mismo RNG que las tiradas y se enciende la flag de la cara. Devuelve las
// lineas del registro.
std::vector<std::string> tiraTablas(SessionData& d, const std::vector<RPG::TablaAleatoriaRequest>& tablas) {
    std::vector<std::string> lineas;
    for (const RPG::TablaAleatoriaRequest& t : tablas) {
        if (d.m_narrativeState == nullptr || d.m_rngInUse == nullptr || t.dado < 1 ||
            t.flags.size() != static_cast<std::size_t>(t.dado))
            continue;
        const int cara = d.m_rngInUse->roll_in_range(1, t.dado);
        d.m_narrativeState->setFlag(t.flags[static_cast<std::size_t>(cara - 1)]);
        lineas.push_back((t.que.empty() ? std::string("Tiras") : t.que + ":") + " 1d" + std::to_string(t.dado) +
                         ", sale un " + std::to_string(cara) + ".");
    }
    return lineas;
}

// FASE1-BND-01 — Los descansos, con la regla del reglamento (Sistema de cartas,
// §7.5): el corto cura 1d8 por tier; el largo, toda la vida. Las cartas ya
// vuelven a la mano en cada combate (CartasEnCombate), asi que aqui solo es la
// vida.
std::vector<std::string> descansa(SessionData& d, const std::vector<std::string>& descansos) {
    std::vector<std::string> lineas;
    ICombatant* cuerpo = d.m_playerBody;
    for (const std::string& tipo : descansos) {
        if (cuerpo == nullptr || d.m_rngInUse == nullptr) continue;
        const int antes = cuerpo->health();
        if (tipo == "largo") {
            cuerpo->heal(cuerpo->maxHealth() - antes);
        } else {
            int curado = 0;
            for (int i = 0; i < std::max(1, cuerpo->tier()); ++i) curado += d.m_rngInUse->roll_in_range(1, 8);
            cuerpo->heal(std::min(curado, cuerpo->maxHealth() - antes));
        }
        const int ganada = cuerpo->health() - antes;
        lineas.push_back(std::string(tipo == "largo" ? "Descanso largo" : "Descanso corto") + ": " +
                         (ganada > 0 ? "recuperas " + std::to_string(ganada) + " PV." : "ya estabas entero."));
    }
    return lineas;
}

// BND-ALT-03 — Lo que hiere desde un beat (el fuego de la barricada del Ocaso):
// 1d<dado>, sin dejar al heroe por debajo de 1 PV (no es un combate: no se cae
// por leer). Devuelve las lineas del registro.
std::vector<std::string> hiere(SessionData& d, const std::vector<std::pair<std::string, int>>& heridas) {
    std::vector<std::string> lineas;
    ICombatant* cuerpo = d.m_playerBody;
    for (const auto& [que, dado] : heridas) {
        if (cuerpo == nullptr || d.m_rngInUse == nullptr || dado < 2) continue;
        const int dano = std::min(d.m_rngInUse->roll_in_range(1, dado), std::max(0, cuerpo->health() - 1));
        if (dano > 0) cuerpo->takeDamage(dano);
        lineas.push_back(que + ": 1d" + std::to_string(dado) + ", pierdes " + std::to_string(dano) + " PV.");
    }
    return lineas;
}

// MORAL-01 — Una tirada con exitos fijos (Oriol, 08/10: «superar 2 seises en tu
// tirada de carisma», a la letra): sin rival. Parcial si te falta uno como
// mucho y algo sacaste.
RPG::Degree gradoConExitos(const RPG::PoolResult& t, int exitos) {
    if (t.botch) return RPG::Degree::BOTCH;
    if (t.successes >= static_cast<float>(exitos)) return t.critical ? RPG::Degree::CRITICAL : RPG::Degree::SUCCESS;
    if (t.successes > 0.0f && t.successes >= static_cast<float>(exitos - 1)) return RPG::Degree::PARTIAL;
    return RPG::Degree::BOTCH;
}

// FASE1-BND-01 — Lo que da un beat va a la mochila, con su nombre en el
// registro («Recibes: Poción x2.»). Un id que no está en el catálogo no
// entra: sería un objeto sin ficha, que no se puede ni usar ni vender.
std::vector<std::string> daObjetos(SessionData& d, const std::vector<std::string>& objetos) {
    std::vector<std::string> lineas;
    std::vector<std::pair<std::string, int>> recibidos;  // nombre y cuántos, en orden
    for (const std::string& id : objetos) {
        const ObjectDefinition* def = d.m_catalog != nullptr ? d.m_catalog->find(id) : nullptr;
        if (def == nullptr || d.m_inventory == nullptr) {
            const std::string aviso = "(No se da «" + id + "»: no está en el catálogo de objetos.)";
            if (std::find(lineas.begin(), lineas.end(), aviso) == lineas.end()) lineas.push_back(aviso);
            continue;
        }
        d.m_inventory->push_back(id);
        auto it = std::find_if(recibidos.begin(), recibidos.end(),
                               [&](const std::pair<std::string, int>& r) { return r.first == def->name; });
        if (it == recibidos.end()) recibidos.emplace_back(def->name, 1);
        else ++it->second;
    }
    for (const auto& [nombre, n] : recibidos)
        lineas.push_back("Recibes: " + nombre + (n > 1 ? " x" + std::to_string(n) : std::string()) + ".");
    return lineas;
}

}  // namespace


void GameSession::resolveSkillChecks(const std::vector<RPG::SkillCheckRequest>& checks) {
    // Una tirada Nd6 pedida desde un beat: N dados = stat del jugador
    // segun la skill implicada, enfrentada al pool que declara el beat
    // (rivalDice, Ed. 3 — bitacora 2.17), grado -> flag. Es el MISMO
    // DicePoolEngine que usa el combate, asi que narrativa y combate
    // comparten reglas (coherencia pedida para todo el sistema).
    //
    // IMPORTANTE: esto convierte a GameSession en un segundo escritor de
    // flags narrativas (el primero es el propio NarrativeEngine con
    // setFlag/clearFlag). Es consistente con como ya pasaba con el oro
    // (GameSession es quien lo aplica, no el motor narrativo), y la regla
    // de oro sigue en pie: LAS FLAGS SON LA UNICA FUENTE DE VERDAD. Lo
    // unico que cambia es quien empuja el lapiz.
    if (sessionData()->m_narrativeState == nullptr) {
        return;  // Sin estado no hay donde escribir el resultado.
    }
    for (const RPG::SkillCheckRequest& c : checks) {
        const RPG::SkillDefinition* def =
            sessionData()->m_nd6Skills != nullptr ? sessionData()->m_nd6Skills->find(c.skillId) : nullptr;
        if (def == nullptr) {
            logLine("(skillCheck: skill Nd6 desconocida \"" + c.skillId +
                    "\", se ignora. Falta setNd6SkillCatalog?)");
            continue;
        }
        if (sessionData()->m_playerBody == nullptr) {
            logLine("(skillCheck: sin jugador para tirar \"" + c.skillId + "\", se ignora)");
            continue;
        }
        const int dice = sessionData()->m_playerBody->stat(def->casting_stat);
        const RPG::PoolResult pool =
            RPG::DicePoolEngine::roll_pool(dice, RPG::DiceMod::NORMAL, *sessionData()->m_rngInUse);
        // Ed. 3 (bitácora 2.17): la prueba es enfrentada — el rival del
        // beat tira su pool (rivalDice). Beats anteriores a Ed. 3 traen
        // "cd" en su lugar: se convierte a dados (0.5→1 … 2.5→5) para no
        // romper aventuras ya escritas.
        int rivalDice = c.rivalDice;
        if (rivalDice <= 0 && c.cd > 0.0f && c.exitos <= 0) {
            rivalDice = static_cast<int>(c.cd * 2.0f + 0.5f);
            if (rivalDice < 1) rivalDice = 1;
            if (rivalDice > 5) rivalDice = 5;
        }
        if (rivalDice < 0 || c.exitos > 0) rivalDice = 0;  // MORAL-01: con exitos fijos no hay rival
        const RPG::PoolResult rival =
            RPG::DicePoolEngine::roll_pool(rivalDice, RPG::DiceMod::NORMAL, *sessionData()->m_rngInUse);
        const RPG::Degree grado =
            c.exitos > 0 ? gradoConExitos(pool, c.exitos) : RPG::DicePoolEngine::opposed_degree(pool, rival);
        switch (grado) {
            case RPG::Degree::BOTCH:    sessionData()->m_narrativeState->setFlag(c.flagBotch);    break;
            case RPG::Degree::PARTIAL:  sessionData()->m_narrativeState->setFlag(c.flagPartial);  break;
            case RPG::Degree::SUCCESS:  sessionData()->m_narrativeState->setFlag(c.flagSuccess);  break;
            case RPG::Degree::CRITICAL: sessionData()->m_narrativeState->setFlag(c.flagCritical); break;
        }
        logLine("(enfrentada " + c.skillId + ": jugador " +
                std::to_string(pool.successes) + " exitos vs rival (" +
                std::to_string(rivalDice) + "d6) " +
                std::to_string(rival.successes) + " exitos -> " +
                std::to_string(static_cast<int>(grado)) + ")");
        // H2-01 — La foto de la tirada, para que la hoja del diálogo la enseñe.
        RPG::TiradaVisible vista;
        vista.que = def->name.empty() ? c.skillId : def->name;
        if (c.exitos > 0) vista.que += " · " + std::to_string(c.exitos) + (c.exitos == 1 ? " éxito" : " éxitos");
        const RPG::CharacterSheet* ficha = characterSheet();
        vista.quien = ficha != nullptr && !ficha->displayName.empty() ? ficha->displayName : "Tú";
        vista.tirada = pool;
        vista.rival = rival;
        vista.enfrentada = rivalDice > 0;
        vista.grado = grado;
        sessionData()->m_narrativeState->ponTirada(std::move(vista));
    }
}

// C-04 — Ver la declaracion en GameSession.h.
Engine::EntityHandle GameSession::spawnEnemyFromCatalog(const std::string& objectId,
                                                        GridCoord position, bool summoned,
                                                        int identity, int health) {
    const ObjectDefinition* def = sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(objectId) : nullptr;
    if (def == nullptr || def->category != ObjectCategory::Enemy) {
        return {};
    }

    // D-08 — Usar EnemyCatalog::createEnemy() para poblar desde el
    // MonsterCatalog cuando exista; patronMin==Max==position => quieto.
    Engine::WorldEnemy enemy = EnemyCatalog::createEnemy(
        objectId, def, sessionData()->m_monsterCatalog,
        position, position, position,
        summoned, identity, health);
    return sessionData()->m_world->createEnemy(std::move(enemy));
}

// E-04 — Ver la nota del header: envoltorio, no camino nuevo.
void GameSession::startNarrativeBattle(const RPG::BattleRequest& request) {
    resolveBattles({request});
}

void GameSession::resolveBattles(const std::vector<RPG::BattleRequest>& battles) {
    // Materializa al enemigo del catalogo y arranca el combate. Ver el
    // header para por que las flags no se encienden aqui.
    for (const RPG::BattleRequest& req : battles) {
        if (sessionData()->m_battle != nullptr || sessionData()->m_pendingBattle.has_value()) {
            sessionData()->m_battleQueue.push_back(req);  // uno de cada vez; el resto espera
            continue;
        }
        // Uno o un grupo (Oriol, 05/10): todos tienen que existir.
        const std::vector<std::string> grupo = req.grupo();
        std::string roto;
        for (const std::string& id : grupo) {
            const ObjectDefinition* def =
                sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(id) : nullptr;
            if (roto.empty() && (def == nullptr || def->category != ObjectCategory::Enemy)) roto = id;
        }
        if (!roto.empty()) {
            // Un id roto no revienta la partida (mismo criterio permisivo
            // que LevelLoader), pero SI enciende la flag de derrota: si no,
            // la aventura se queda esperando para siempre una flag que
            // nadie va a poner, y el jugador no puede avanzar.
            logLine("(startBattle: enemigo \"" + roto +
                    "\" no esta en el catalogo, se da por perdido)");
            if (sessionData()->m_narrativeState != nullptr) {
                sessionData()->m_narrativeState->setFlag(req.flagDefeat);
            }
            continue;
        }

        // El enemigo aparece junto al jugador: lo ha emboscado la narrativa,
        // no estaba patrullando el nivel. TUTORIAL-01 — Cada uno en una casilla libre
        // de alrededor (la del jugador, si no queda ninguna): todos encima
        // del heroe, en el combate no se les veia.
        //
        // C-04 — Y recibe IDENTIDAD PROPIA: summoned=true y un ordinal de
        // sesion. Antes se quedaba con el spawnIndex por defecto (-1),
        // que es el valor de "ninguno": al guardar la partida y volver a
        // cargarla, restoreSaveGame buscaba un enemigo del nivel con
        // indice -1, no lo encontraba y rechazaba el guardado entero. Si
        // huias de una emboscada y guardabas, esa partida no se abria
        // nunca mas.
        std::vector<Engine::EntityHandle> summoned;
        std::vector<GridCoord> ocupadas{sessionData()->m_playerPosition};
        const auto junto = [&]() {
            const GridCoord p = sessionData()->m_playerPosition;
            static const int dx[8] = {1, 0, -1, 0, 1, -1, 1, -1}, dy[8] = {0, 1, 0, -1, 1, 1, -1, -1};
            for (int k = 0; k < 8; ++k) {
                const GridCoord c{p.x + dx[k], p.y + dy[k]};
                const bool usada = std::any_of(ocupadas.begin(), ocupadas.end(),
                                               [&](const GridCoord& o) { return o.x == c.x && o.y == c.y; });
                if (usada || tileBlocks(c.x, c.y) || enemyAt(c.x, c.y).valid() || objectAt(c.x, c.y).valid()) continue;
                ocupadas.push_back(c);
                return c;
            }
            return p;
        };
        for (const std::string& id : grupo) {
            const Engine::EntityHandle h =
                spawnEnemyFromCatalog(id, junto(), /*summoned=*/true,
                                      sessionData()->m_nextSummonIndex, /*health=*/-1);
            if (!h.valid()) continue;  // no deberia pasar: los ids ya se han validado arriba
            ++sessionData()->m_nextSummonIndex;
            summoned.push_back(h);
        }
        if (summoned.empty()) {
            continue;
        }

        // C-17 — El invocado acaba de nacer PENDIENTE en el World (los
        // creates no participan en get()/queries hasta flush). startBattle
        // lo busca vivo por handle: hay que publicarlo antes. Aqui no se
        // esta dentro de una iteracion del World (la emboscada la dispara
        // la narrativa al hablar, fuera de update), asi que el flush es
        // seguro.
        sessionData()->m_world->flushPendingChanges();

        sessionData()->m_pendingBattle = req;
        // ACOMPANANTES-01 — Quien pelea a tu lado: los de la aventura que pide
        // el combate, cada uno si su flag esta encendida.
        std::vector<RPG::AcompananteDef> aLado;
        const RPG::AdventureScript* aventura =
            sessionData()->m_narrative != nullptr ? sessionData()->m_narrative->adventure() : nullptr;
        for (const RPG::BattleRequest::ConAcompanante& con : req.acompanantes) {
            const RPG::AcompananteDef* def = aventura != nullptr ? aventura->acompanante(con.id) : nullptr;
            const bool toca = con.siFlag.empty() ||
                              (sessionData()->m_narrativeState != nullptr && sessionData()->m_narrativeState->hasFlag(con.siFlag));
            if (def != nullptr && toca) aLado.push_back(*def);
        }
        startBattle(summoned, aLado);
        // CAER-01 — De un momento ineludible no se huye.
        if (req.ineludible && sessionData()->m_battle != nullptr) sessionData()->m_battle->prohibeHuida();
    }
}

bool GameSession::applyNarrative(const RPG::NarrativeResult& result) {
    if (!result.fired) {
        return false;
    }
    sessionData()->m_dialogueSpeaker = result.speaker;
    sessionData()->m_dialogueLines = result.lines;
    sessionData()->m_shopOffers.clear();
    sessionData()->m_mode = GameMode::Dialogue;

    addGold(result.goldDelta);
    for (const std::string& linea : concedeNiveles(*sessionData(), result.levelPoints, result.milestones)) logLine(linea);
    resolveSkillChecks(result.skillChecks);
    for (const std::string& linea : tiraTablas(*sessionData(), result.tablas)) logLine(linea);  // FASE1-BND-01
    // MORAL-01 — Lo que se hace pesa al dormir: los actos se apuntan antes del
    // descanso largo del mismo beat, que los asienta.
    for (const std::string& linea : moral::apuntaActos(*sessionData(), result.actos)) logLine(linea);
    for (const std::string& linea : descansa(*sessionData(), result.descansos)) logLine(linea);
    if (std::find(result.descansos.begin(), result.descansos.end(), "largo") != result.descansos.end())
        for (const std::string& linea : moral::asientaNoche(*sessionData())) logLine(linea);
    for (const std::string& linea : hiere(*sessionData(), result.heridas)) logLine(linea);  // BND-ALT-03
    for (const std::string& linea : daObjetos(*sessionData(), result.objetos)) logLine(linea);
    moral::reflejaEnLaAventura(*sessionData());
    resolveBattles(result.battles);
    // TUTORIAL-01 — Lo que dice el beat se lee antes de pelear: el combate queda
    // montado (los enemigos ya estan en su sitio) y empieza al cerrar el
    // dialogo (closeInteraction). Sin lineas, en el acto.
    if (sessionData()->m_mode == GameMode::Battle && !sessionData()->m_dialogueLines.empty())
        sessionData()->m_mode = GameMode::Dialogue;
    for (const std::string& entry : result.log) {
        logLine(entry);
    }
    for (const std::string& line : sessionData()->m_dialogueLines) {
        // speaker vacio = voz del narrador: la linea va sola, sin "X: ".
        logLine(sessionData()->m_dialogueSpeaker.empty() ? line : sessionData()->m_dialogueSpeaker + ": " + line);
    }
    avaluaEventosDelMundo();  // C8-03c
    return true;
}

// G4-01 — Una puerta cerrada por flags no cambia de nivel: habla su beat (como
// un talk) o, sin beat, avisa. false = abierta, se cruza como siempre.
bool GameSession::cierraPuerta(const ObjectSpawn& puerta) {
    const RPG::NarrativeState* estado = sessionData()->m_narrativeState;
    if (!puerta.estaCerrada([estado](const std::string& f) { return estado != nullptr && estado->hasFlag(f); }))
        return false;
    if (puerta.beatCerrada.empty() || !talkNarrative(puerta.beatCerrada)) logLine("La puerta está cerrada.");
    return true;
}

void GameSession::setNarrative(const RPG::NarrativeEngine* engine, RPG::NarrativeState* state) {
    m_data->m_narrative = engine;
    m_data->m_narrativeState = state;
    moral::reflejaLaReputacion(*m_data);  // BND-04: restoreSaveGame pudo ir antes
}

void GameSession::reflejaReputacion() { moral::reflejaLaReputacion(*m_data); }

bool GameSession::enterLevelNarrative(const std::string& levelId) {
    if (sessionData()->m_narrative == nullptr || sessionData()->m_narrativeState == nullptr) {
        return false;
    }
    reflejaHeroe(*sessionData());  // HEROE-01
    return applyNarrative(sessionData()->m_narrative->enterLevel(levelId, *sessionData()->m_narrativeState));
}

void GameSession::updateNarrativeTimers(float dtActivo, const std::string& nivelActual) {
    if (sessionData()->m_narrative == nullptr || sessionData()->m_narrativeState == nullptr) {
        return;
    }
    // flagCount() antes/despues detecta que el tick encendio ALGO nuevo
    // (aviso por umbral o expiracion). Re-encender una flag ya encendida
    // no cuenta — ese aviso ya se conto una vez.
    const std::size_t flagsAntes = sessionData()->m_narrativeState->flagCount();
    sessionData()->m_narrative->tickTimers(*sessionData()->m_narrativeState, dtActivo, nivelActual);
    if (sessionData()->m_narrativeState->flagCount() == flagsAntes) {
        return;
    }
    // La consecuencia sale como beat de dialogo, pero solo en Exploration:
    // no se interrumpe una tienda ni un dialogo abierto (el tick del
    // cierre de ese dialogo lo recoge igual).
    if (sessionData()->m_mode == GameMode::Exploration) {
        reflejaHeroe(*sessionData());  // HEROE-01
        applyNarrative(sessionData()->m_narrative->tick(*sessionData()->m_narrativeState));
    }
}

bool GameSession::talkNarrative(const std::string& npcId) {
    if (sessionData()->m_narrative == nullptr || sessionData()->m_narrativeState == nullptr) {
        return false;
    }
    reflejaHeroe(*sessionData());  // HEROE-01
    return applyNarrative(sessionData()->m_narrative->talkTo(npcId, *sessionData()->m_narrativeState));
}

// PROLOGO-02 — Ver la declaracion: el tick() que haria cerrar un dialogo.
bool GameSession::avanzaNarrativa() {
    if (sessionData()->m_narrative == nullptr || sessionData()->m_narrativeState == nullptr ||
        sessionData()->m_mode != GameMode::Exploration) {
        return false;
    }
    reflejaHeroe(*sessionData());  // HEROE-01
    return applyNarrative(sessionData()->m_narrative->tick(*sessionData()->m_narrativeState));
}

// OPCIONES-01 — Ver la declaracion. Lo que trae la opcion se aplica como un
// beat (oro, niveles, tiradas, combates); sin lineas no queda nada que leer y
// el dialogo se cierra, que es lo que dispara el tick de lo que venga detras.
bool GameSession::chooseOption(int indice) {
    RPG::NarrativeState* estado = sessionData()->m_narrativeState;
    if (sessionData()->m_mode != GameMode::Dialogue || sessionData()->m_narrative == nullptr ||
        estado == nullptr || !estado->eleccion().activa()) {
        return false;
    }
    moral::reflejaLaReputacion(*sessionData());  // BND-04
    const RPG::NarrativeResult elegida = sessionData()->m_narrative->elige(indice, *estado);
    if (!elegida.fired) return false;
    applyNarrative(elegida);
    if (sessionData()->m_dialogueLines.empty()) closeInteraction();
    return true;
}

