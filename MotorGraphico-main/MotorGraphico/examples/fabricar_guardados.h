// fabricar_guardados.h — E-GUION-02. Guardados "justo antes" de tres escenas del juego, para que el modo
// guion (`juego --guardado g.json --guion s.txt`) llegue a ellas sin jugar el prologo ni el dia 1.
//
// Se juega con el arnes de las demos (demo_boundington_partida.h: un Jugador sobre un NarrativeState) hasta
// el momento y se fotografia con GameSession::captureSaveGame sobre el nivel real, como hace el guardado
// rapido. Lo que el arnes no tiene (heroe, posicion) se pone aqui:
//   - el heroe: humano de las Marcas con el aspecto por defecto (Apariencia::deRaza) y su equipo inicial, para
//     que al cargar salga vestido (PantallaNuevaPartida::restaurarHeroe);
//   - la posicion: la casilla de al lado del PNJ (rutaAndando) o, sin PNJ, el inicio del nivel.
//
// Los tres momentos:
//   sectarios  dia 1, en la plaza, con los sectarios ya resueltos: el siguiente "hablar" con
//              sectario_perdido dispara b1_sectarios_seguir ("Seguirles de lejos [Sigilo]").
//   ocaso      el Ocaso, en la base: el aviso ya dado (bnd3_avisado) y el evento comienza_ocaso sin evaluar,
//              asi que al cargar suenan las campanas (cinematica base_campanas, beat b3_campanas).
//   combate5   dia 1, el barrio de la nina del gato: bnd1_verdaderos_pifia puesta, asi que al cargar salta
//              b1_verdaderos_pelea, el combate con mas enemigos del dia 1 (cinco perdido_devoto).
//
// GL-free. Lo usan la herramienta (fabricar_guardados.cpp) y su prueba (tests/Engine/GuardadosDeCapturaTests.cpp).
#pragma once

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <cstdlib>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "Game/Personajes/Vestuario.h"
#include "Game/NewGameFlow.h"
#include "Game/SaveGame.h"
#include "Game/BattleState.h"
#include "Game/GameSession.h"
#include "Game/MoralDeLaSesion.h"
#include "Game/SessionData.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "Project/ProjectIndex.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/Narrative/HeroeEnLaHistoria.h"
#include "RPG/TierRules.h"
#include "Render/ICombatant.h"
#include "Render/TileMap.h"
#include "demo_boundington_partida.h"

namespace fabricar {

// Cuerpo del jugador para una GameSession sin Player (GL-free): lleva la ficha, que es lo que captureSaveGame lee.
class CuerpoConFicha final : public ICombatant {
public:
    explicit CuerpoConFicha(RPG::CharacterSheet ficha) : m_ficha(std::move(ficha)) { m_vida = m_ficha.healthCap(); }
    RPG::CharacterSheet* character_sheet() override { return &m_ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &m_ficha; }
    void takeDamage(int n) override { m_vida = std::max(0, m_vida - n); }
    void heal(int n) override { m_vida = std::min(m_ficha.healthCap(), m_vida + n); }
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_ficha.healthCap(); }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat s) const override { return m_ficha.stat(s); }
    RPG::DefenseBlock defenses() const override { return m_ficha.defenseValues(); }
    int tier() const override { return m_ficha.tier; }
    std::string combatant_id() const override { return "heroe_guardado"; }

private:
    RPG::CharacterSheet m_ficha;
    int m_vida = 1;
};

// El heroe de los guardados: humano de las Marcas, su ficha con el equipo inicial y su aspecto de raza.
struct Heroe {
    RPG::CharacterSheet ficha;
    SavedHero guardado;
    std::vector<std::string> flags;  // heroe_* que la sesion pone antes de cada disparo
    RPG::HeroeEnLaHistoria historia;
};

inline Heroe creaHeroe() {
    Heroe h;
    auto creacion = RPG::CharacterCreation::load("assets/catalogs");
    require(creacion.isOk());
    const RPG::CharacterCreation& cc = creacion.value();
    RPG::CreationChoice e;
    e.displayName = "Ilse";
    e.raceId = "race_human_marches";
    const auto sirve = [](const RPG::ClassDefinition& c) {
        return c.playable && c.tier <= 1 && !c.startingSkillIds.empty() && !c.startingEquipmentIds.empty();
    };
    if (const RPG::ClassDefinition* guerrero = cc.classes().find("guerrero"); guerrero != nullptr && sirve(*guerrero))
        e.classId = guerrero->id;  // un guerrero: arma y armadura a la vista
    cc.classes().forEach([&](const RPG::ClassDefinition& c) {
        if (e.classId.empty() && sirve(c)) e.classId = c.id;
    });
    cc.backgrounds().forEach([&](const RPG::BackgroundDefinition& b) {
        const auto* o = cc.opcionesDe(b.id);
        if (e.backgroundId.empty() && o != nullptr && !o->bonds.empty() && !o->fears.empty() && !o->flaws.empty() &&
            !o->goals.empty() && !o->ideals.empty() && !o->personalities.empty() && !o->virtues.empty()) {
            e.backgroundId = b.id;
            e.bondId = o->bonds.front().id;
            e.fearId = o->fears.front().id;
            e.flawId = o->flaws.front().id;
            e.goalId = o->goals.front().id;
            e.idealId = o->ideals.front().id;
            e.personalityId = o->personalities.front().id;
            e.virtueId = o->virtues.front().id;
        }
    });
    require(!e.classId.empty() && !e.backgroundId.empty());
    const RPG::TierRules tiers;
    RPG::Catalogs::EquipmentCatalog equipo;
    require(equipo.loadFromFile("assets/catalogs/equipment.json").isOk());
    auto ficha = Game::NewGameFlow::fichaSinEquipo(cc, e, tiers);
    require(ficha.isOk());
    h.ficha = ficha.value();
    // Como el cuaderno de creacion (NewGameFlow::finalizar): con el equipo inicial puesto.
    // fichaSinEquipo vacia la mochila, asi que se vuelve a construir con ella.
    auto completa = cc.construir(e);
    require(completa.isOk());
    h.ficha = completa.value();
    cc.recalcular(h.ficha, tiers);
    const RPG::ClassDefinition* clase = cc.classes().find(e.classId);
    require(clase != nullptr);
    Game::Vestuario::equiparInicial(h.ficha, *clase, tiers, equipo);

    SavedHero& s = h.guardado;
    s.raceId = h.ficha.raceId;
    s.classId = h.ficha.classId;
    s.backgroundId = h.ficha.backgroundId;
    s.deityId = h.ficha.deityId;
    s.bondId = h.ficha.bondId;
    s.fearId = h.ficha.fearId;
    s.flawId = h.ficha.flawId;
    s.goalId = h.ficha.goalId;
    s.idealId = h.ficha.idealId;
    s.personalityId = h.ficha.personalityId;
    s.virtueId = h.ficha.virtueId;
    s.equipped.assign(h.ficha.equippedIds.begin(), h.ficha.equippedIds.end());
    // El aspecto por defecto: el de la raza tal cual la viste personajes.json (lo que hace vestirPorDefecto).
    auto manifiesto = Game::ManifiestoPersonajes::cargar("assets/textures");
    require(manifiesto.isOk());
    const Game::RazaVisual* raza = manifiesto.value().raza("race_human_marches");
    require(raza != nullptr);
    Game::NewGameFlow::guardaApariencia(Game::Apariencia::deRaza(*raza), s);

    h.historia = RPG::HeroeEnLaHistoria::desde(h.ficha);
    h.flags = h.historia.flags;
    return h;
}

// Un nivel montado sin ventana: el mapa, el catalogo de la campana y la sesion con el heroe.
struct NivelMontado {
    std::string ruta;
    LevelDefinition nivel;
    TileMap mapa;
    ObjectCatalog catalogo;
    SkillCatalog habilidades;
    SkillSet habilidadesDelJugador{8};
    std::vector<std::string> inventario;
    std::unique_ptr<CuerpoConFicha> cuerpo;
    RPG::Catalogs::MonsterCatalog monstruos;
    std::unique_ptr<GameSession> sesion;
};

inline std::unique_ptr<NivelMontado> montaNivel(const std::string& nombre, const RPG::CharacterSheet& ficha) {
    auto n = std::make_unique<NivelMontado>();
    n->ruta = "assets/levels/" + nombre + ".json";
    // Los mismos catalogos que carga Application::init con titulo: el de siempre y los del manifiesto de la campana.
    require(n->catalogo.loadFromFile("assets/objects/boundington_npcs.json").isOk());
    auto indice = Editor::ProjectIndex::scan("assets");
    require(indice.isOk() && !indice.value().projects().empty());
    const auto& campana = indice.value().projects().front();
    for (const auto& f : campana.catalogs)
        require(n->catalogo.loadFromFile("assets/" + campana.settings.catalogsDirectory + "/" + f).isOk());
    auto lvl = LevelLoader::loadFromFile(n->ruta);
    require(lvl.isOk());
    n->nivel = lvl.value();
    require(n->mapa.loadFromFile(n->nivel.mapPath).isOk());
    require(n->monstruos.loadFromFile("assets/catalogs/monsters.json").isOk());
    n->cuerpo = std::make_unique<CuerpoConFicha>(ficha);
    n->sesion = std::make_unique<GameSession>(&n->mapa, &n->catalogo, &n->habilidades, n->nivel, n->cuerpo.get(),
                                              &n->habilidadesDelJugador, &n->inventario, nullptr, &n->monstruos);
    return n;
}

// El momento que se quiere guardar: nombre del fichero, nivel y el PNJ junto al que se queda el jugador.
struct Momento {
    const char* archivo;  // sin carpeta
    const char* nivel;    // sin "assets/levels/" ni ".json"
    const char* pnj;      // objectId junto al que se pone; "" = el inicio del nivel
    int dx, dy;           // la casilla de al lado que se prefiere (la que se ve bien en la camara); si no se pisa, otra
    int px = -1, py = -1; // E-MATRIZ-02: casilla exacta (si px >= 0 manda sobre el PNJ)
};

inline constexpr Momento kSectarios{"sectarios.json", "ciudad_centro", "sectario_perdido", 0, -1};
inline constexpr Momento kOcaso{"ocaso.json", "interior_cuartel", "venides_base", 0, 1};
inline constexpr Momento kCombate5{"combate5.json", "ciudad_oeste", "nina_del_gato", 1, 0};

// Un paso mas de la historia, para la mutacion de la prueba: el momento "sectarios" fabricado cuando el beat
// que tenia que quedar pendiente ya se ha disparado.
struct Opciones {
    bool sectariosUnPasoTarde = false;
};

// La casilla de al lado del PNJ en la que se pone el heroe: la preferida del momento (la que se ve bien en la camara)
// si se puede pisar (hay camino hasta ella) y, si no, la vecina pisable de mayor x+y.
inline GridCoord casillaJuntoA(GameSession& sesion, const LevelDefinition& nivel, const Momento& momento) {
    const std::string pnj = momento.pnj;
    if (momento.px >= 0) return GridCoord{momento.px, momento.py};
    if (pnj.empty()) return nivel.playerStart;
    for (const ObjectSpawn& o : nivel.objects) {
        if (o.objectId != pnj) continue;
        const GridCoord preferida{o.position.x + momento.dx, o.position.y + momento.dy};
        const std::vector<GridCoord> hasta = sesion.rutaAndando(preferida);
        if (!hasta.empty() && hasta.back().x == preferida.x && hasta.back().y == preferida.y) return preferida;
        GridCoord mejor = nivel.playerStart;
        int mejorPeso = -1;
        const int dx[4] = {1, 0, -1, 0}, dy[4] = {0, 1, 0, -1};
        for (int i = 0; i < 4; ++i) {
            const GridCoord c{o.position.x + dx[i], o.position.y + dy[i]};
            const std::vector<GridCoord> ruta = sesion.rutaAndando(c);
            const bool pisable = (!ruta.empty() && ruta.back().x == c.x && ruta.back().y == c.y) ||
                                 (c.x == nivel.playerStart.x && c.y == nivel.playerStart.y);
            if (pisable && c.x + c.y > mejorPeso) {
                mejor = c;
                mejorPeso = c.x + c.y;
            }
        }
        return mejor;
    }
    std::fprintf(stderr, "fabricar_guardados: %s no esta en el nivel\n", pnj.c_str());
    require(false);
    return {};
}

// E-MATRIZ-01b — Vuelca en la sesion montada lo que el arnes apunto (demo_bnd::Cuentas), en el mismo orden y con las piezas
// que usa GameSession::applyNarrative (privada): el oro (addGold: nunca por debajo de 0), los hitos y niveles de la ficha
// (concedeNiveles -> CharacterSheet::grantMilestones / grantLevelPoints), los actos (moral::apuntaActos) y, con un descanso
// largo, la noche que los asienta (moral::asientaNoche), y los objetos (daObjetos: los que no estan en el catalogo no entran).
// La moral se calcula sobre un SessionData de ida y vuelta (lo que pide moral::) y se copia a la sesion.
inline void aplicaLasCuentas(NivelMontado& n, RPG::NarrativeState& estado, const demo_bnd::Cuentas& cuentas) {
    SessionData d;
    d.m_playerBody = n.cuerpo.get();
    d.m_inventory = &n.inventario;
    d.m_catalog = &n.catalogo;
    d.m_narrativeState = &estado;
    d.m_gold = demo_bnd::Cuentas::kBolsaInicial;
    static const RPG::TierRules reglasDeNivel;  // las de por defecto = assets/rules/tier_rules.json
    RPG::CharacterSheet* ficha = n.cuerpo->character_sheet();
    for (const demo_bnd::Cuentas::Paso& p : cuentas.pasos) {
        if (std::getenv("MATRIZ_CUENTAS") != nullptr) {  // diagnostico: que beat da que
            std::fprintf(stderr, "    [cuentas] %s oro%+d niv%+d hit%+d", p.beat.c_str(), p.oro, p.puntos, p.hitos);
            for (const std::string& a : p.actos) std::fprintf(stderr, " acto:%s", a.c_str());
            for (const std::string& o : p.objetos) std::fprintf(stderr, " obj:%s", o.c_str());
            for (const std::string& o : p.descansos) std::fprintf(stderr, " descanso:%s", o.c_str());
            std::fprintf(stderr, "\n");
        }
        d.m_gold = std::max(0, d.m_gold + p.oro);
        if (p.puntos > 0 || p.hitos > 0) {
            ficha->grantMilestones(p.hitos, reglasDeNivel);
            ficha->grantLevelPoints(p.puntos, reglasDeNivel);
        }
        moral::apuntaActos(d, p.actos);
        if (std::find(p.descansos.begin(), p.descansos.end(), "largo") != p.descansos.end()) moral::asientaNoche(d);
        for (const std::string& id : p.objetos)
            if (n.catalogo.find(id) != nullptr) n.inventario.push_back(id);
    }
    moral::reflejaEnLaAventura(d);
    n.sesion->addGold(d.m_gold);
    n.sesion->morality() = d.m_morality;
}

// Fotografia de la partida del arnes en `momento`.
inline SaveGameData fotografia(const demo_bnd::Jugador& j, const Heroe& heroe, const Momento& momento) {
    auto montado = montaNivel(momento.nivel, heroe.ficha);
    montado->sesion->setNarrative(&j.e, const_cast<RPG::NarrativeState*>(&j.s));
    aplicaLasCuentas(*montado, const_cast<RPG::NarrativeState&>(j.s), j.cuentas);  // E-MATRIZ-01b: oro, nivel, moral, objetos
    SaveGameData data = montado->sesion->captureSaveGame(montado->ruta);
    // Los objetos que comparten casilla (ciudad_centro: la puerta de la herreria y aigren_carro en (6,59), y otros
    // dos) van enteros: restoreSaveGame los distingue por id desde E-GUION-02b.
    data.playerPosition = casillaJuntoA(*montado->sesion, montado->nivel, momento);
    data.hero = heroe.guardado;
    data.characterName = heroe.ficha.displayName;
    data.playerHealth = data.playerMaxHealth = montado->cuerpo->maxHealth();  // (el tope sube si el heroe sube de tier)
    return data;
}

// Una partida con el heroe: el arnes con las flags heroe_* puestas (las pone la sesion en el juego).
struct Partida {
    demo_bnd::Jugador j;
    Partida(const demo_bnd::Guiones& g, const Heroe& h, demo_bnd::Suerte suerte) : j(g, suerte, h.flags) {
        h.historia.refleja(j.s);
        j.ruido = std::getenv("BND_RUIDO") != nullptr;  // para diagnosticar: cuenta la partida paso a paso
    }
};

// Cambia de aventura como la Application (ApplicationBucle.cpp, «transiciones de AVENTURA»): ademas de dar el libreto,
// el juego enciende una flag privada por capitulo para no reentrar en la transicion cada fotograma. El arnes de las
// demos no las pone, y un guardado sin ellas hace que el juego repita la transicion (el dia 1 mandaba al heroe a la
// taberna otra vez).
inline void fase(demo_bnd::Jugador& j, const std::string& nombre) {
    j.fase(nombre);
    if (nombre == "dia1") j.s.setFlag("bnd_dia1_empezado");
    if (nombre == "dia_gorran") j.s.setFlag("bnd_diag_empezado");
    if (nombre == "dia2") j.s.setFlag("bnd_dia2_empezado");
    if (nombre == "ocaso") j.s.setFlag("bnd_ocaso_empezado");
}

// --- Los tres momentos ---

inline SaveGameData sectarios(const demo_bnd::Guiones& g, const Heroe& h, const Opciones& op = {}) {
    // Suerte regular: la tirada de «Mostrar interes [Persuasion]» sale a medias, los sectarios no sueltan la pista
    // (bnd_pista_sectarios) y quedan resueltos sin invitarte a la cripta: el siguiente hablar es «Seguirles».
    Partida p(g, h, demo_bnd::Suerte::Regular);
    demo_bnd::Jugador& j = p.j;
    j.prefiere = {"Pagar la cama", "Preguntar por los Perdidos", "Mostrar interés [Persuasión]", "Subir a dormir"};
    j.prologo();
    fase(j, "dia1");
    j.entra("interior_taberna");
    j.habla("skilla");  // la cama y el desayuno con Venides
    j.entra("ciudad_centro");
    // Los sectarios de la plaza: la charla se juega hasta que quedan resueltos.
    for (int vez = 0; vez < 8 && !j.s.hasFlag("bnd1_sectarios_resueltos"); ++vez) j.habla("sectario_perdido");
    require(j.s.hasFlag("bnd1_sectarios_resueltos"));
    require(!j.s.hasFlag("bnd1_sectarios_seguir_visto"));
    if (op.sectariosUnPasoTarde) {
        j.habla("sectario_perdido");  // b1_sectarios_seguir ya disparado
        require(j.s.hasFlag("bnd1_sectarios_seguir_visto"));
    }
    return fotografia(j, h, kSectarios);
}

// Lo que contesta el heroe de la ruta A de demo_campana_boundington (el que lo hace todo): la misma lista, para
// llegar al Ocaso con las pistas del dia 3 (sin ellas la plaza esta vacia y Venides «no tiene plan»).
inline const std::vector<std::string> kRutaDelHeroe = {
    "Mirar alrededor [Percepción]", "Observarle [Conocimiento arcano]", "Callar y escuchar", "¿Qué pasó en las montañas?",
    "Pagar la cama", "Subir a dormir", "Anoche soñé con una barca.",
    "Mostrar interés [Persuasión]", "Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]",
    "Busco antorchas.", "Volver a sacar las antorchas", "Insistir, de malos modos [Intimidación]",
    "Escuchar lo que te pide", "No diré nada. Tranquila.", "Sentarte con él, sin decir nada",
    "Invitarle a desayunar [Persuasión]", "Hablar de lo que has averiguado", "Contárselo todo",
    "Separarlos [pelea sin matar]", "Hacerle hablar", "¿Quién te manda?",
    "Aceptar el encargo", "Pagar el paso (5)", "Cogerla sin que te oigan [Sigilo]", "Suelta a los presos y estamos en paz.",
    "Devolvérsela y contarle todo", "¿Qué fue de los otros tres?", "Vengo con el caballero Venides.",
    "Ir con Venides a ver al capitan", "Soy el que te trajo al camorrista anoche.",
    "De noble excéntrico de Ascaria", "Vengo a preguntar si necesitáis algo", "Preguntarle qué necesitan",
    "Me quedo hasta que vengan.", "Plantarte delante.", "Hablar con el vigilante",
    "Convencerle de que te deje pasar [Persuasion]", "Entrar por la puerta, con disfraz [Interpretacion]",
    "Mezclarte con los invitados", "Prefiero hacerle preguntas", "Sonsacar al noble que brinda [Engano]",
    "Senalarselo a Venides", "Contarselo todo", "Subir a dormir",
    "Correr a avisar a Venides", "Al oeste", "Vamos a por el.", "Ven con nosotros.", "Liberarle",
};

// Los tres dias de Boundington, por la ruta del heroe (demo_campana_boundington, juegaHeroe), hasta el amanecer
// del Ocaso: el sermon de Sandes en la plaza.
inline void hastaElOcaso(demo_bnd::Jugador& j) {
    j.prefiere = kRutaDelHeroe;
    j.prologo();
    fase(j, "dia1");
    j.entra("interior_taberna");
    j.habla("skilla");
    j.entra("ciudad_centro");
    j.habla("sectario_perdido");
    j.entra("interior_herreria");
    j.habla("aigren");
    j.entra("interior_sastreria");
    j.habla("luisarda");
    j.entra("ciudad_oeste");
    j.habla("xila");
    j.entra("interior_iglesia");
    j.entra("interior_taberna");
    j.anochece();
    j.habla("venides");
    j.duerme();
    require(j.s.hasFlag("bnd_dia_cerrado"));
    fase(j, "dia_gorran");
    j.drena();
    j.laEspadaDeRosita();
    j.entra("interior_taberna");
    j.habla("skilla");
    require(j.s.hasFlag("bndg_dia_cerrado"));
    fase(j, "dia2");
    j.drena();
    j.habla("skilla");
    j.entra("interior_castillo");
    j.entra("interior_castillo_p1");
    j.habla("capitan_guardia");
    j.habla("capitan_guardia");
    j.entra("ciudad_oeste");
    j.entra("tienda_abandonada");
    j.entra("ciudad_surysal");
    j.habla("viejo_surysal");
    j.anochece();
    j.entra("ciudad_este");
    j.entra("mansion_barrio_alto");
    j.habla("seguidor_brinda");
    j.habla("seguidor_brinda");
    j.entra("interior_taberna");
    j.habla("venides");
    j.duermeDia3();
    require(j.s.hasFlag("bnd2_dia_cerrado"));
    fase(j, "ocaso");
}

inline SaveGameData ocaso(const demo_bnd::Guiones& g, const Heroe& h) {
    Partida p(g, h, demo_bnd::Suerte::Buena);
    demo_bnd::Jugador& j = p.j;
    hastaElOcaso(j);
    j.entra("ciudad_centro");  // el sermon de Sandes: «Correr a avisar a Venides»
    j.entra("interior_cuartel");
    require(!j.s.hasFlag("bnd3_campanas"));
    // Se habla con Venides pero sin dejar que el mundo evalue sus eventos (en el juego lo hace cada fotograma): el
    // aviso queda dado y comienza_ocaso sin disparar. Al cargar, el primer fotograma hace sonar las campanas.
    j.cinesAlEntrar = false;
    j.habla("venides_base");
    j.cinesAlEntrar = true;
    require(j.s.hasFlag("bnd3_avisado") && !j.s.hasFlag("bnd3_campanas"));
    return fotografia(j, h, kOcaso);
}

inline SaveGameData combate5(const demo_bnd::Guiones& g, const Heroe& h) {
    Partida p(g, h, demo_bnd::Suerte::Buena);
    demo_bnd::Jugador& j = p.j;
    j.prefiere = {"Pagar la cama", "Subir a dormir"};
    j.prologo();
    fase(j, "dia1");
    j.entra("interior_taberna");
    j.habla("skilla");
    j.entra("ciudad_oeste");  // Pico Dragon: la nina del gato y la puerta azul
    // «Calmarlos [Persuasion]» sale mal (pifia): los Verdaderos se echan encima. La flag la enciende el juego al
    // resolver la tirada y el beat b1_verdaderos_pelea (auto) lanza el combate de cinco en el fotograma siguiente.
    // No se drena: el combate queda por empezar.
    j.s.setFlag("bnd1_verdaderos_intentado");
    j.s.setFlag("bnd1_verdaderos_pifia");
    return fotografia(j, h, kCombate5);
}


// ===========================================================================================================
// E-MATRIZ-01 — Un guardado por camino del dia 1.
//
// Cada CaminoDia1 juega el prologo y el dia 1 DE VERDAD con el arnes (ruta + respuestas), hasta cerrar el dia
// (bnd_dia_cerrado, sin dar el paso a Gorran), y fotografia la partida en el cuarto de la taberna. La prueba
// (tests/Engine/MatrizDia1Tests.cpp) los restaura en una sesion limpia, juega el inicio del dia siguiente y mira
// las consecuencias que inventaria tools/matriz_decisiones.py. Las ranuras salen de fabricar_guardados --matriz.
// ===========================================================================================================

// Un beat de un dia posterior que lee la decision y si, con los hechos del dia 1 que trae el guardado, se puede ver.
// (Los requisitos propios de ese dia, bnd2_/bnd3_/..., se dan por cumplidos: lo que se mide es el efecto de la decision.)
struct Lector {
    const char* dia;    // "dia_gorran" | "dia2" (= boundington_segundo_dia) | "ocaso"
    const char* beat;
    const char* opcion; // "" = el requires del beat; si no, el de esa opcion (ademas del beat)
    bool seVe;
};

// Mutacion: en una COPIA del JSON del dia 1 (temporal), la primera aparicion de `cadena` despues de la opcion
// (`beat`, `opcion`) pasa a ser otra: la opcion deja de hacer su efecto.
// `opcion` vacia = desde el principio del beat (un efecto del beat, no de una opcion). `hacia`: por que se cambia (por
// defecto un nombre que no existe: el efecto no hace nada; "log" convierte un grantGold en un log; un acto distinto
// mueve la moral a otro valor).
struct Mutacion {
    const char* beat;
    const char* opcion;
    const char* cadena;
    const char* hacia = "zz_mutada";
    int vez = 1;   // la n-esima opcion con ese texto (b1_pelea tiene dos «Proteger al borracho dormido»)
    int salta = 0; // apariciones de `cadena` que se dejan como estan antes de la que se cambia (la 1.ª puede ser un requires)
    const char* dia = "primer_dia";  // E-DIA1-CONSEC: el JSON que se muta (primer_dia | dia_gorran | segundo_dia | ocaso)
};

// Lo que el guardado trae ademas de las flags (E-MATRIZ-01b): el oro (150 de la bolsa inicial mas lo que se cobra y se paga),
// el valor de la moral asentada (el eje de siete tramos: moral::tramoDe), el nivel y el avance hacia el siguiente, y la
// mochila (ids, ordenados).
struct Cuenta {
    int oro = 0;
    int moral = 0;
    int nivel = 1;
    int progreso = 0;
    std::vector<std::string> mochila;
};

struct CaminoDia1 {
    const char* id;    // dia1_<id>.json
    const char* ruta;  // sectarios | aigren | verdaderos | luisarda | pelea
    std::vector<std::string> dice;
    demo_bnd::Suerte suerte;
    std::vector<std::string> beats;             // beats que se tienen que haber visto
    std::vector<std::string> ve, noVe;          // flags
    std::vector<std::pair<std::string, int>> minimos;  // variable >= n
    std::vector<Lector> lectores;
    Mutacion mutacion;
    const char* nota;
    Cuenta cuenta;       // E-MATRIZ-01b: lo que tiene que traer la partida
    Mutacion mutOro;     // ... y una copia del JSON en la que el oro de este camino deja de ser el que es
    Mutacion mutMoral;   // ... y otra en la que cambia la moral (beat vacio = sin acto que mover: no se prueba)
    std::vector<std::pair<std::string, demo_bnd::Suerte>> fuerza;  // tiradas (por su flagBotch) que salen con esta suerte aunque las demas salgan bien
    std::vector<std::pair<std::string, int>> igual;                // variable == n
    std::vector<std::string> pierde, huye;                         // E-DIA1-CONSEC: combates (por su flagVictory) que se pierden / de los que se huye
};

inline bool cierraElDia1(demo_bnd::Jugador& j) {
    if (j.s.hasFlag("bnd_dia_cerrado")) return true;
    j.entra("interior_taberna");
    j.anochece();
    j.habla("venides");
    j.duerme();
    return j.s.hasFlag("bnd_dia_cerrado");
}

// El recorrido del dia 1 hasta la decision. Cada ruta es la de la escena de demo_boundington que la juega.
inline void recorre(demo_bnd::Jugador& j, const std::string& ruta) {
    if (ruta != "xila_borracho") {  // (xila_borracho se suma a otra ruta del dia 1: la cama ya esta pagada y no se duerme antes de tiempo)
        j.entra("interior_taberna");
        j.habla("skilla");  // la cama
    }
    if (ruta == "sandes" || ruta == "sandes_luisarda") {  // la cripta con la pifia de «Hablar con alguien»: Sandes sale a hablarte
        if (ruta == "sandes_luisarda") {  // ... habiendo pasado antes por la sastreria (bnd1_luisarda_sala)
            j.entra("interior_sastreria");
            j.habla("luisarda");
        }
        j.entra("ciudad_centro");
        j.habla("sectario_perdido");
        require(j.s.hasFlag("bnd1_invitado_guarida"));
        j.entra("interior_iglesia_s1");
        j.entra("santuario_subterraneo");
    } else if (ruta == "aigren_convence") {  // «Convencerle de que son el mal»: sale bien o mal, y por la noche Aigren ya sabe
        j.entra("interior_herreria");
        j.habla("aigren");
        j.anochece();
        j.entra("interior_herreria");
    } else if (ruta == "luisarda_pelea") {  // Luisarda cenando con Rosita y, de noche, la pelea de la taberna
        j.entra("interior_herreria");
        j.habla("aigren");
        j.habla("aigren");
        j.entra("interior_sastreria");
        j.habla("luisarda");
        j.entra("interior_taberna");
        j.habla("skilla");
        j.duerme();
    } else if (ruta == "sectarios") {
        j.entra("ciudad_centro");
        j.habla("sectario_perdido");
        require(j.s.hasFlag("bnd1_invitado_guarida"));
        j.entra("interior_iglesia_s1");
        j.entra("santuario_subterraneo");
    } else if (ruta == "completa") {  // la cripta de los sectarios y, ademas, los PNJ de la ruta del heroe: pistas de sobra
        j.entra("ciudad_centro");
        j.habla("sectario_perdido");
        require(j.s.hasFlag("bnd1_invitado_guarida"));
        j.entra("interior_iglesia_s1");
        j.entra("santuario_subterraneo");
        j.entra("interior_herreria");
        j.habla("aigren");
        j.entra("interior_sastreria");
        j.habla("luisarda");
        j.entra("ciudad_oeste");
        j.habla("xila");
        j.entra("interior_iglesia");
    } else if (ruta == "xila_borracho") {  // D0025.1: Xila despreciado y convidado (b1_xila_resentido -> b1_xila_habla): se duerme borracho
        j.prefiere.insert(j.prefiere.begin(), {"Tratarle con desprecio", "Invitarle a un trago (1) y hacer las paces"});
        j.entra("ciudad_oeste");
        j.habla("xila");
        j.habla("xila");
        require(j.s.hasFlag("bnd1_xila_borracho") && !j.s.hasFlag("bnd1_xila_honor"));
    } else if (ruta == "aigren") {
        j.entra("interior_herreria");
        j.habla("aigren");
        require(j.s.hasFlag("bnd1_invitado_por_aigren") || j.s.hasFlag("bnd1_aigren_tema_perdidos") || j.s.hasFlag("bnd1_aigren_resuelto"));
        j.entra("interior_iglesia_s1");
        j.entra("santuario_subterraneo");
    } else if (ruta == "verdaderos") {
        j.entra("interior_taberna");
        j.habla("skilla");  // pasa la mañana
        j.entra("ciudad_oeste");
        j.habla("nina_del_gato");
    } else if (ruta == "luisarda") {
        j.entra("interior_herreria");
        j.habla("aigren");
        j.habla("aigren");
        j.entra("interior_sastreria");
        j.habla("luisarda");
        j.entra("interior_taberna");
        j.habla("skilla");
    } else if (ruta == "pelea") {
        j.entra("interior_taberna");
        j.duerme();  // anochece, «Subir a dormir» y b1_pelea
    } else if (ruta == "camara") {  // E-DIA1-CONSEC: la novicia copista cuenta lo de Killville y se baja a la camara honda de Sandes
        j.habla("copista_cripta");
        j.entra("camara_profunda");
    } else {
        require(false);
    }
}

// Juega el camino y devuelve el guardado del final del dia. `jugado` (opcional) recibe los beats vistos.
// `exigeCierre` = false para las mutaciones, que pueden dejar el dia sin poder cerrarse (la comprobacion lo dira).
inline SaveGameData dia1(const demo_bnd::Guiones& g, const Heroe& h, const CaminoDia1& c, std::vector<std::string>* jugado = nullptr,
                         bool exigeCierre = true) {
    Partida p(g, h, c.suerte);
    demo_bnd::Jugador& j = p.j;
    j.prefiere = c.dice;
    j.prefiere.insert(j.prefiere.end(), {"Pagar la cama", "Subir a dormir"});
    for (const auto& f : c.fuerza) j.tiradas[f.first] = f.second;
    for (const std::string& f : c.pierde) j.pierde.insert(f);
    for (const std::string& f : c.huye) j.huyen.insert(f);
    j.prologo();
    fase(j, "dia1");
    recorre(j, c.ruta);
    j.suerte = demo_bnd::Suerte::Buena;
    const bool cerrado = cierraElDia1(j);
    if (exigeCierre) require(cerrado);
    if (jugado != nullptr) *jugado = j.vistos;
    static const Momento kDia1{"dia1.json", "interior_taberna_p1", "", 0, 0};
    return fotografia(j, h, kDia1);
}

// Copia del dia 1 con la opcion neutralizada: se lee el JSON real, se cambia en memoria y se escribe en un temporal.
inline demo_bnd::Guiones conMutacion(const demo_bnd::Guiones& g, const Mutacion& m, const std::string& temporal) {
    std::string t;
    {
        FILE* f = std::fopen((std::string("assets/adventures/boundington_") + m.dia + ".json").c_str(), "rb");
        require(f != nullptr);
        char buf[65536];
        std::size_t n;
        while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) t.append(buf, n);
        std::fclose(f);
    }
    const std::size_t b = t.find(std::string("\"id\": \"") + m.beat + "\"");
    require(b != std::string::npos);
    std::size_t o = std::string(m.opcion).empty() ? b : t.find(std::string("\"texto\": \"") + m.opcion + "\"", b);
    for (int v = 1; v < m.vez && o != std::string::npos; ++v) o = t.find(std::string("\"texto\": \"") + m.opcion + "\"", o + 1);
    require(o != std::string::npos);
    const std::string quiero = std::string("\"") + m.cadena + "\"";
    std::size_t x = t.find(quiero, o);
    for (int v = 0; v < m.salta && x != std::string::npos; ++v) x = t.find(quiero, x + 1);
    require(x != std::string::npos);
    t.replace(x, quiero.size(), std::string("\"") + m.hacia + "\"");
    FILE* f = std::fopen(temporal.c_str(), "wb");
    require(f != nullptr);
    std::fwrite(t.data(), 1, t.size(), f);
    std::fclose(f);
    auto mutada = RPG::AdventureScript::loadFromFile(temporal);
    std::remove(temporal.c_str());
    require(mutada.isOk());
    demo_bnd::Guiones r = g;
    const std::string dia = m.dia;
    (dia == "primer_dia" ? r.dia1 : dia == "dia_gorran" ? r.gorran : dia == "segundo_dia" ? r.dia2 : r.ocaso) = mutada.value();
    return r;
}

// Un guardado restaurado en una sesion limpia: el nivel real, la sesion, y un Jugador cuyo estado narrativo ES el de la sesion.
struct Restaurada {
    SaveGameData leido;
    std::unique_ptr<NivelMontado> nivel;
    std::unique_ptr<demo_bnd::Jugador> j;
};

inline Restaurada restaura(const demo_bnd::Guiones& g, const Heroe& h, const SaveGameData& data, const std::string& archivo) {
    Restaurada r;
    const std::string ruta = (std::filesystem::temp_directory_path() / ("matriz_dia1_" + archivo)).string();
    std::string error;
    require(writeSaveGameFile(ruta, data, &error));
    require(readSaveGameFile(ruta, &r.leido, &error));
    std::filesystem::remove(ruta);
    const std::string nombre = r.leido.levelPath.substr(std::string("assets/levels/").size());
    r.nivel = montaNivel(nombre.substr(0, nombre.size() - 5), h.ficha);
    r.j = std::make_unique<demo_bnd::Jugador>(g, demo_bnd::Suerte::Buena);
    r.nivel->sesion->setNarrative(&r.j->e, &r.j->s);
    require(r.nivel->sesion->restoreSaveGame(r.leido, &error));
    r.j->s.ponLugar(r.leido.levelPath);
    return r;
}

// ¿Se puede ver el beat (u opcion) lector con los hechos del dia 1 de `base`? Los requisitos de los dias posteriores
// (flags que no son bnd0_/bnd1_, variables ajenas al dia 1) se dan por cumplidos; los del dia 1 se leen del guardado.
inline bool lectorSeVe(const demo_bnd::Guiones& g, const Lector& l, const RPG::NarrativeState& base, bool tambienGorran = false, bool tambienDia3 = false) {
    const RPG::AdventureScript& guion = g.por(l.dia);
    const RPG::NarrativeBeat* beat = nullptr;
    for (const RPG::NarrativeBeat& b : guion.beats)
        if (b.id == l.beat) beat = &b;
    require(beat != nullptr);
    // Los hechos del dia 1: todo nombre que el JSON del dia 1 enciende, apaga o suma (flags, salidas de tiradas y
    // combates, variables). Lo demas lo ponen los dias posteriores y se da por cumplido.
    std::set<std::string> deldia1;
    const auto recoge = [&](const std::vector<RPG::NarrativeEffect>& efs) {
        for (const RPG::NarrativeEffect& e : efs) {
            if (e.type == RPG::EffectType::SetFlag || e.type == RPG::EffectType::ClearFlag ||
                e.type == RPG::EffectType::SetVariable || e.type == RPG::EffectType::AddVariable)
                deldia1.insert(e.arg);
            if (e.type == RPG::EffectType::SkillCheck)
                for (const std::string& f : {e.skillCheck.flagBotch, e.skillCheck.flagPartial, e.skillCheck.flagSuccess, e.skillCheck.flagCritical})
                    deldia1.insert(f);
            if (e.type == RPG::EffectType::StartBattle)
                for (const std::string& f : {e.battle.flagVictory, e.battle.flagDefeat, e.battle.flagHuida}) deldia1.insert(f);
        }
    };
    for (const RPG::NarrativeBeat& b : g.dia1.beats) {
        recoge(b.effects);
        for (const RPG::OpcionDeDialogo& o : b.opciones) recoge(o.efectos);
    }
    if (tambienGorran) {  // E-MATRIZ-02: el guardado es del final del dia de Gorran: tambien son hechos suyos los del dia 2
        for (const RPG::NarrativeBeat& b : g.gorran.beats) {
            recoge(b.effects);
            for (const RPG::OpcionDeDialogo& o : b.opciones) recoge(o.efectos);
        }
    }
    if (tambienDia3) {  // E-MATRIZ-03: el guardado es del final del dia 3: tambien son hechos suyos los de boundington_segundo_dia
        for (const RPG::NarrativeBeat& b : g.dia2.beats) {
            recoge(b.effects);
            for (const RPG::OpcionDeDialogo& o : b.opciones) recoge(o.efectos);
        }
    }
    for (const char* v : {"dia", "franja", "bnd_dia_cerrado", "bndg_dia_cerrado"}) deldia1.erase(v);  // el reloj y el cierre los mueve cada dia
    const auto propio = [&](const std::string& f) { return deldia1.count(f) > 0; };
    const auto variableDelDia1 = propio;
    const auto visible = [&](const RPG::NarrativeCondition& c) {
        RPG::NarrativeState s = base;
        for (const std::string& f : c.allFlags)
            if (!propio(f)) s.setFlag(f);
        for (const std::string& f : c.notFlags)
            if (!propio(f)) s.clearFlag(f);
        // anyFlags: si mezcla flags del dia 1 con otras (bgd_capucha o bnd1_unido_al_culto), mandan las del dia 1: las
        // otras podrian darse sin la decision y no la distinguen. Si no tiene ninguna del dia 1, se da la primera.
        bool hayPropia = false;
        for (const std::string& f : c.anyFlags) hayPropia = hayPropia || propio(f);
        if (!c.anyFlags.empty() && !hayPropia) s.setFlag(c.anyFlags.front());
        for (const auto& v : c.variables)
            if (!variableDelDia1(v.nombre)) s.setVariable(v.nombre, v.min != std::numeric_limits<int>::min() ? v.min : 0);
        if (!c.lugares.empty()) s.ponLugar(c.lugares.front());
        return c.isSatisfiedBy(s);
    };
    bool ve = visible(beat->requirements);
    if (std::string(l.opcion).empty()) return ve;
    // E-DIA1-CONSEC: una opcion puede desdoblarse en variantes con el mismo texto (b2_venides_castillo_s1_a, b3_sermon_s3_a):
    // se ve si se ve alguna.
    bool hay = false;
    for (const RPG::OpcionDeDialogo& o : beat->opciones)
        if (o.texto.rfind(l.opcion, 0) == 0) {
            hay = true;
            if (ve && visible(o.requisitos)) return true;
        }
    require(hay);
    return false;
}

// Lo que tiene que cumplir el camino al restaurarse. Devuelve las faltas ("" = ninguna).
inline std::vector<std::string> compruebaDia1(const demo_bnd::Guiones& g, Restaurada& r, const CaminoDia1& c) {
    std::vector<std::string> fallos;
    demo_bnd::Jugador& j = *r.j;
    const auto falta = [&](const std::string& s) { fallos.push_back(s); };
    for (const std::string& f : c.ve)
        if (!j.s.hasFlag(f)) falta("falta " + f);
    for (const std::string& f : c.noVe)
        if (j.s.hasFlag(f)) falta("sobra " + f);
    for (const auto& v : c.minimos)
        if (j.s.variable(v.first) < v.second) falta(v.first + "=" + std::to_string(j.s.variable(v.first)) + " < " + std::to_string(v.second));
    for (const auto& v : c.igual)
        if (j.s.variable(v.first) != v.second) falta(v.first + "=" + std::to_string(j.s.variable(v.first)) + " != " + std::to_string(v.second));
    for (const std::string& b : c.beats)
        if (!j.s.beatFired(b)) falta("no disparado " + b);
    // El oro, la moral, el nivel y la mochila: restaurados por el juego (restoreSaveGame), no por el arnes.
    {
        GameSession& sesion = *r.nivel->sesion;
        const RPG::CharacterSheet* ficha = r.nivel->cuerpo->character_sheet();
        if (sesion.gold() != c.cuenta.oro) falta("oro=" + std::to_string(sesion.gold()) + " != " + std::to_string(c.cuenta.oro));
        if (sesion.morality().value() != c.cuenta.moral)
            falta("moral=" + std::to_string(sesion.morality().value()) + " != " + std::to_string(c.cuenta.moral));
        if (sesion.morality().tramo() != moral::tramoDe(c.cuenta.moral))
            falta(std::string("tramo de moral ") + sesion.morality().label() + " != " + moral::nombreTramo(moral::tramoDe(c.cuenta.moral)));
        if (!sesion.morality().pendientes().empty()) falta("moral sin asentar: " + std::to_string(sesion.morality().pendientes().size()) + " actos pendientes");
        if (ficha->nivel != c.cuenta.nivel || ficha->progresoNivel != c.cuenta.progreso)
            falta("nivel " + std::to_string(ficha->nivel) + " (" + std::to_string(ficha->progresoNivel) + " %) != " + std::to_string(c.cuenta.nivel) + " (" +
                  std::to_string(c.cuenta.progreso) + " %)");
        std::vector<std::string> mochila = *sesion.inventory();
        std::sort(mochila.begin(), mochila.end());
        if (mochila != c.cuenta.mochila) falta("mochila de " + std::to_string(mochila.size()) + " objetos != la esperada de " + std::to_string(c.cuenta.mochila.size()));
    }
    if (!j.s.hasFlag("bnd_dia_cerrado")) falta("el dia no esta cerrado");
    if (j.s.hasFlag("bnd_diag_empezado")) falta("el guardado ya es de Gorran");
    // El inicio del dia siguiente, como lo hace el juego al cargar: cambio de aventura y primer fotograma.
    // El HUD lee el dia del estado (MinimapaTinta::rellena -> franjas::rotulo): el guardado trae «Día 1 · Noche» y el reloj
    // pasa a «Día 2 · Mañana» cuando salta bg_amanece (pasarFranja), que en el juego es al acabar la cinematica dia1_despertar.
    if (RPG::franjas::rotulo(RPG::franjas::momentoDe(j.s)) != "Día 1 · Noche") falta("el guardado no trae «Día 1 · Noche»");
    fase(j, "dia_gorran");
    j.drena();
    if (!j.visto("bg_amanece")) falta("no amanece Gorran (bg_amanece)");
    if (RPG::franjas::rotulo(RPG::franjas::momentoDe(j.s)) != "Día 2 · Mañana") falta("al amanecer el HUD no dice «Día 2 · Mañana»");
    if (!j.s.hasFlag("bnd_diag_empezado")) falta("Gorran no ha empezado");
    // Que la decision siga en pie tras el primer fotograma del dia siguiente.
    for (const std::string& f : c.ve)
        if (!j.s.hasFlag(f)) falta("tras amanecer ya no esta " + f);
    for (const Lector& l : c.lectores) {
        const bool ve = lectorSeVe(g, l, j.s);
        if (ve != l.seVe)
            falta(std::string(l.dia) + ":" + l.beat + (std::string(l.opcion).empty() ? "" : std::string("/") + l.opcion) +
                  (l.seVe ? " deberia verse" : " no deberia verse"));
    }
    return fallos;
}

// --- Los caminos del dia 1 (la tabla de la prueba y de las ranuras) ---
// Respuestas comunes y recorridos, como en demo_boundington («DIA 1: la cripta, ...»). Los lectores y los efectos
// esperados salen de out/fabrica/UNI-02-11/MATRIZ_DIA1.md (tools/matriz_decisiones.py).

// Respuestas comunes de la cripta, como en demo_boundington («DIA 1: la cripta, ...»).
inline const std::vector<std::string> kCriptaSectarios = {
    "Mostrar interés [Persuasión]", "Cuenta conmigo.", "Dejar las armas", "Mirar la estatua [Conocimiento arcano]",
    "Escuchar el canto", "Hablar con alguien [Persuasión]", "¿Qué mezclas?", "¿Conoces a un caballero llamado Venides?",
    "Callar"};
inline const std::vector<std::string> kCriptaAigren = {"Preguntarle por los Perdidos", "Decirle que te interesan",
                                                "Vengo de parte del herrero.", "Ir a por lo tuyo"};

inline const std::vector<std::string> kLuisarda = {"¿Trabajas solo?", "Pues suena muy sano.", "Presionarle con su hijo [Intimidación]", "Busco antorchas.",
                                            "Volver a sacar las antorchas", "Hacerle creer que ya lo sabes todo [Engaño]", "Escuchar lo que te pide"};

inline std::vector<std::string> junta(std::vector<std::string> a, const std::vector<std::string>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

inline const std::vector<CaminoDia1>& caminosDia1() {
    // Orden de campos: id, ruta, dice, suerte, beats, ve, noVe, minimos, lectores, mutacion, nota.
    using demo_bnd::Suerte;
    static const std::vector<CaminoDia1> c = {
        // --- b1_edul (la cripta, invitado por los sectarios) + b1_venides_informe_bastan ---
        {"edul_unirse", "sectarios", junta({"Unirte de verdad", "Contárselo todo"}, kCriptaSectarios), Suerte::Buena,
         {"b1_edul"}, {"bnd1_unido_al_culto", "bnd1_guarida_resuelta", "bnd1_sabe_barricadas"}, {"bnd1_dudas_verina"},
         {{"puntos_gurkazaal", 1}},
         {{"dia_gorran", "bgd_velas_hermano", "", true}, {"dia2", "b2_po54_sandes", "", true}, {"ocaso", "b3_verina_salvada_dudas", "", false}},
         {"b1_edul", "Unirte de verdad", "bnd1_unido_al_culto"}, "b1_edul: unirse al culto",
         {165, 0, 2, 50, {}}, {"b1_edul", "Unirte de verdad", "grantGold", "log"}, {"b1_edul", "Unirte de verdad", "unirse_perdidos", "dudas_en_verina"}},
        {"edul_dudas", "sectarios", junta({"Sembrar dudas en Verina", "Contárselo todo"}, kCriptaSectarios), Suerte::Buena,
         {"b1_edul"}, {"bnd1_dudas_verina", "bnd1_verina_vista", "bnd1_guarida_resuelta"}, {"bnd1_unido_al_culto"}, {},
         {{"ocaso", "b3_verina_salvada_dudas", "", true}, {"dia_gorran", "bgd_velas_hermano", "", false}, {"dia2", "b2_po54_sandes", "", false}},
         {"b1_edul", "Sembrar dudas en Verina", "bnd1_dudas_verina"}, "b1_edul: sembrar dudas en Verina",
         {125, 2, 2, 50, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_edul", "Sembrar dudas en Verina", "dudas_en_verina"}},
        {"informe_cuenta_todo", "completa", junta({"Sembrar dudas en Verina", "Contárselo todo"}, junta(kCriptaSectarios, kRutaDelHeroe)), Suerte::Buena,
         {"b1_venides_informe_bastan"}, {"bnd1_venides_informe_hecho", "bnd_venides_20", "bnd_aviso_dado"}, {"bnd1_venides_desconfianza"}, {},
         {{"dia2", "b2_venides_castillo_v0", "", true}, {"dia2", "b2_noche_rosita_informa", "Contarselo todo y ensenarle", false}},
         {"b1_venides_informe_bastan", "Contárselo todo", "bnd_venides_20"}, "b1_venides_informe_bastan: contárselo todo a Venides",
         {165, 5, 3, 38, {"vendas", "vendas"}}, {"b1_venides_informe_bastan", "", "grantGold", "log"}, {"b1_aigren_redimido", "", "aigren_redimido"}},
        {"informe_desconfia", "completa", junta({"Sembrar dudas en Verina", "Desconfiar de él"}, junta(kCriptaSectarios, kRutaDelHeroe)), Suerte::Buena,
         {"b1_venides_informe_bastan"}, {"bnd1_venides_informe_hecho", "bnd1_venides_desconfianza"}, {"bnd_venides_20"}, {},
         {{"dia2", "b2_venides_castillo_v0", "", false}, {"dia2", "b2_noche_rosita_informa", "Contarselo todo y ensenarle", true}},
         {"b1_venides_informe_bastan", "Desconfiar de él", "bnd1_venides_desconfianza"}, "b1_venides_informe_bastan: desconfiar de Venides",
         {125, 5, 3, 10, {"vendas", "vendas"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_aigren_redimido", "", "aigren_redimido"}},
        // --- b1_guarida_por_aigren (la cripta, de parte del herrero) ---
        {"aigren_verina", "aigren", junta({"Sacar a Verina de aquí", "Contárselo todo"}, kCriptaAigren), Suerte::Buena,
         {"b1_guarida_por_aigren"}, {"bnd1_verina_salvada_guarida", "bnd1_guarida_resuelta", "bnd1_invitado_por_aigren"}, {"bnd1_unido_al_culto", "bnd1_pista_guarida"}, {},
         {{"dia2", "b2_herreria_cripta", "", true}, {"dia_gorran", "bgd_velas_hermano", "", false}, {"dia2", "b2_rosita_verina", "", false}},
         {"b1_guarida_por_aigren", "Sacar a Verina de aquí", "bnd1_verina_salvada_guarida"}, "b1_guarida_por_aigren: sacar a Verina",
         {125, 5, 2, 50, {"pocion"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_guarida_por_aigren", "Sacar a Verina de aquí", "verina_sacada_de_la_guarida"}},
        {"aigren_sonsacar", "aigren", junta({"Sonsacar para Venides", "Contárselo todo"}, kCriptaAigren), Suerte::Buena,
         {"b1_guarida_por_aigren"}, {"bnd1_pista_guarida", "bnd1_sabe_barricadas", "bnd1_guarida_resuelta"}, {"bnd1_verina_salvada_guarida", "bnd1_unido_al_culto"}, {{"pistas", 1}},
         {{"dia2", "b2_castillo_prueba", "", true}, {"dia2", "b2_rosita_verina_arriba", "", false}},
         {"b1_guarida_por_aigren", "Sonsacar para Venides", "bnd1_pista_guarida"}, "b1_guarida_por_aigren: sonsacar para Venides",
         {125, 0, 2, 50, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_guarida_ceremonia", "", "ver_ceremonia_gurkazaal", "verina_sacada_de_la_guarida"}},
        {"aigren_unirse", "aigren", junta({"Unirte", "Contárselo todo"}, kCriptaAigren), Suerte::Buena,
         {"b1_guarida_por_aigren"}, {"bnd1_unido_al_culto", "bnd1_sabe_barricadas", "bnd1_guarida_resuelta"}, {"bnd1_verina_salvada_guarida", "bnd1_pista_guarida"}, {{"puntos_gurkazaal", 1}},
         {{"dia_gorran", "bgd_velas_hermano", "", true}, {"dia2", "b2_rosita_verina_arriba", "", false}},
         {"b1_guarida_por_aigren", "Unirte", "bnd1_unido_al_culto"}, "b1_guarida_por_aigren: unirse al culto",
         {165, 0, 2, 50, {}}, {"b1_guarida_por_aigren", "Unirte", "grantGold", "log"}, {"b1_guarida_por_aigren", "Unirte", "unirse_perdidos", "verina_sacada_de_la_guarida"}},
        // --- b1_luisarda_pide ---
        {"luisarda_silencio", "luisarda", junta({"No diré nada. Tranquila.", "Contárselo todo"}, kLuisarda), Suerte::Buena,
         {"b1_luisarda_pide"}, {"bnd1_luisarda_silencio"}, {"bnd1_luisarda_con_rosita"}, {},
         {{"dia2", "b2_luisarda_pociones", "", true}, {"dia2", "b2_luisarda", "", false}},
         {"b1_luisarda_pide", "No diré nada. Tranquila.", "bnd1_luisarda_silencio"}, "b1_luisarda_pide: guardar silencio",
         {125, -5, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_aigren_hijo", "", "aigren_presionado"}},
        {"luisarda_cena", "luisarda", junta({"Ven a cenar a la taberna. Rosita te espera.", "Contárselo todo"}, kLuisarda), Suerte::Buena,
         {"b1_luisarda_pide"}, {"bnd1_luisarda_con_rosita"}, {"bnd1_luisarda_silencio"}, {{"confianza_rosita", 1}},
         {{"dia2", "b2_luisarda", "", true}, {"dia2", "b2_luisarda_pociones", "", false}},
         {"b1_luisarda_pide", "Ven a cenar a la taberna. Rosita te espera.", "bnd1_luisarda_con_rosita"}, "b1_luisarda_pide: Luisarda a cenar con Rosita",
         {125, -3, 2, 20, {"pocion", "pocion"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_luisarda_pide", "Ven a cenar a la taberna. Rosita te espera.", "luisarda_con_rosita"}},
        // --- b1_verdaderos (Pico Dragon) ---
        {"verdaderos_sumarse", "verdaderos", {"Comer algo mientras pasa la mañana (5)", "Sumarte a ellos", "Contárselo todo"}, Suerte::Buena,
         {"b1_verdaderos"}, {"bnd1_verdaderos_unido", "bnd1_familia_resuelta"}, {"bnd1_verdaderos_vencidos", "bnd1_familia_defendida"}, {},
         {{"dia2", "b2_po54_sandes", "", true}, {"ocaso", "b3_sermon_po54", "", true}},
         {"b1_verdaderos", "Sumarte a ellos", "bnd1_verdaderos_unido"}, "b1_verdaderos: sumarse a los Verdaderos",
         {120, -5, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_verdaderos", "Sumarte a ellos", "sumarse_verdaderos"}},
        {"verdaderos_cara", "verdaderos", {"Comer algo mientras pasa la mañana (5)", "Plantarles cara", "Contárselo todo"}, Suerte::Buena,
         {"b1_verdaderos"}, {"bnd1_verdaderos_vencidos", "bnd1_familia_defendida"}, {"bnd1_verdaderos_unido"}, {},
         {{"dia2", "b2_po54_sandes", "", false}, {"ocaso", "b3_sermon_po54", "", false}},
         {"b1_verdaderos", "Plantarles cara", "bnd1_verdaderos_vencidos"}, "b1_verdaderos: plantarles cara (y ganar)",
         {140, 5, 2, 45, {"pastel", "pastel"}}, {"b1_verdaderos_vencidos", "", "grantGold", "log"}, {"b1_verdaderos", "Plantarles cara", "verdaderos_enfrentados"}},
        // --- b1_pelea (la noche en la taberna) ---
        {"pelea_separar", "pelea", {"Separarlos [pelea sin matar]", "Contárselo todo"}, Suerte::Buena,
         {"b1_pelea"}, {"bnd1_pelea_lanzada", "bnd1_pelea_ganada"}, {"bnd1_pelea_rosita"}, {},
         {{"dia2", "b2_venides_castillo_s1_a", "¿Anoche escribiste", true}},
         {"b1_pelea", "Separarlos [pelea sin matar]", "bnd1_pelea_ganada"}, "b1_pelea: separarlos a golpes",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""}},
        {"pelea_proteger", "pelea", {"Proteger al borracho dormido", "Contárselo todo"}, Suerte::Buena,
         {"b1_pelea"}, {"bnd1_pelea_lanzada", "bnd1_pelea_rosita", "bnd1_rosita_fuerza_vista"}, {"bnd1_pelea_ganada"}, {{"confianza_rosita", 1}},
         {{"dia2", "b2_venides_castillo_s1_a", "¿Anoche escribiste", true}},
         {"b1_pelea", "Proteger al borracho dormido", "bnd1_pelea_rosita"}, "b1_pelea: proteger al borracho dormido",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""}},
        // --- E-MATRIZ-01b: los caminos que faltaban ---
        // b1_charla_sandes: la pifia de «Hablar con alguien [Persuasion]» en la ceremonia (la fuerza el arnes) y no ir con Edul.
        {"sandes_solo", "sandes", junta({"Solo miraba.", "Salir con él"}, kCriptaSectarios), Suerte::Buena,
         {"b1_charla_sandes", "b1_edul"}, {"bnd1_charla_pifia", "bnd1_con_edul", "bnd1_sandes_conocido", "bnd1_sandes_enfadado", "bnd1_sandes_te_conoce"}, {"bnd1_unido_al_culto", "bnd1_luisarda_delatada"}, {},
         {{"dia2", "b2_fiesta_puerta", "Buscar a Edul para entrar como", true}},
         {"b1_charla_sandes", "Solo miraba.", "bnd1_con_edul"}, "b1_charla_sandes: «Solo miraba.»",
         {125, 0, 2, 50, {"pocion"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""},
         {{"bnd1_charla_pifia", Suerte::Mala}}},
        {"sandes_mandar", "sandes", junta({"¿Y tú quién eres para mandar?", "Salir con él"}, kCriptaSectarios), Suerte::Buena,
         {"b1_charla_sandes", "b1_edul"}, {"bnd1_charla_pifia", "bnd1_con_edul", "bnd1_sandes_conocido", "bnd1_sandes_enfadado", "bnd1_sandes_te_conoce"}, {"bnd1_unido_al_culto", "bnd1_luisarda_delatada"}, {},
         {{"dia2", "b2_fiesta_puerta", "Buscar a Edul para entrar como", true}},
         {"b1_charla_sandes", "¿Y tú quién eres para mandar?", "bnd1_con_edul"}, "b1_charla_sandes: «¿Y tú quién eres para mandar?»",
         {125, 0, 2, 50, {"pocion"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""},
         {{"bnd1_charla_pifia", Suerte::Mala}}},
        {"sandes_luisarda", "sandes_luisarda", junta({"Contarle lo de Luisarda", "Salir con él"}, junta(kCriptaSectarios, kLuisarda)), Suerte::Buena,
         {"b1_charla_sandes", "b1_edul"}, {"bnd1_charla_pifia", "bnd1_con_edul", "bnd1_sandes_conocido", "bnd1_luisarda_delatada", "bnd1_sandes_te_conoce"}, {"bnd1_unido_al_culto", "bnd1_sandes_enfadado"}, {},
         {{"dia2", "b2_cartel_luisarda", "", true}, {"dia2", "b2_sastre_delatada", "", true}, {"dia2", "b2_fiesta_puerta", "Buscar a Edul para entrar como", true}},
         {"b1_charla_sandes", "Contarle lo de Luisarda", "bnd1_luisarda_delatada", "zz_mutada", 1, 1}, "b1_charla_sandes: contarle a Sandes lo de Luisarda",
         {125, -5, 2, 50, {"pocion"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_charla_sandes", "Contarle lo de Luisarda", "luisarda_delatada"},
         {{"bnd1_charla_pifia", Suerte::Mala}}},
        // b1_aigren_perdidos: la tirada de «Convencerle de que son el mal [Persuasion]», con su exito y su pifia.
        {"aigren_convence_exito", "aigren_convence", {"Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]"}, Suerte::Buena,
         {"b1_aigren_perdidos", "b1_aigren_redimido"}, {"bnd1_aigren_c_exito", "bnd1_aigren_redimido", "bnd1_aigren_resuelto", "bnd_pista_aigren"}, {"bnd1_aigren_c_pifia", "bnd1_pista_espadas", "bnd1_aigren_orden_pedida", "bnd1_invitado_por_aigren"}, {{"pistas", 1}},
         {{"dia2", "b2_herreria_redimido", "", true}, {"dia2", "b2_capitan_orden", "", false}},
         {"b1_aigren_perdidos", "Convencerle de que son el mal [Persuasión]", "bnd1_aigren_c_exito"}, "b1_aigren_perdidos: la tirada de persuasion sale bien",
         {125, 5, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_aigren_redimido", "", "aigren_redimido"}},
        {"aigren_convence_pifia", "aigren_convence", {"Preguntarle por los Perdidos", "Convencerle de que son el mal [Persuasión]"}, Suerte::Buena,
         {"b1_aigren_perdidos", "b1_aigren_papel", "b1_aigren_noche"}, {"bnd1_aigren_c_pifia", "bnd1_aigren_orden_pedida", "bnd1_pista_espadas"}, {"bnd1_aigren_c_exito", "bnd1_aigren_redimido", "bnd_pista_aigren", "bnd1_invitado_por_aigren"}, {},
         {{"dia2", "b2_capitan_orden", "", true}, {"dia2", "b2_herreria_redimido", "", false}},
         {"b1_aigren_perdidos", "Convencerle de que son el mal [Persuasión]", "bnd1_aigren_c_pifia"}, "b1_aigren_perdidos: la tirada de persuasion sale mal",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""},
         {{"bnd1_aigren_c_pifia", Suerte::Mala}}},
        // b1_pelea: «Quedarte mirando» y la variante de «Proteger» con Luisarda (las mismas flags que la probada; ver MATRIZ_DIA1.md).
        {"pelea_mirar", "pelea", {"Quedarte mirando", "Contárselo todo"}, Suerte::Buena,
         {"b1_pelea"}, {"bnd1_pelea_lanzada", "bnd1_pelea_rosita", "bnd1_rosita_fuerza_vista"}, {"bnd1_pelea_ganada"}, {},
         {{"dia2", "b2_venides_castillo_s1_a", "¿Anoche escribiste", true}},
         {"b1_pelea", "Quedarte mirando", "bnd1_pelea_rosita"}, "b1_pelea: quedarte mirando",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""}, {}, {{"confianza_rosita", 0}}},
        {"pelea_proteger_luisarda", "luisarda_pelea", junta({"Ven a cenar a la taberna. Rosita te espera.", "Proteger al borracho dormido", "Contárselo todo"}, kLuisarda), Suerte::Buena,
         {"b1_pelea", "b1_luisarda_pide"}, {"bnd1_pelea_lanzada", "bnd1_pelea_rosita", "bnd1_rosita_fuerza_vista", "bnd1_luisarda_con_rosita"}, {"bnd1_pelea_ganada"}, {},
         {{"dia2", "b2_venides_castillo_s1_a", "¿Anoche escribiste", true}},
         {"b1_pelea", "Proteger al borracho dormido", "bnd1_pelea_rosita", "zz_mutada", 2}, "b1_pelea: proteger al borracho dormido (con Luisarda en la taberna)",
         {125, -3, 2, 20, {"pocion", "pocion"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"},
         {"b1_luisarda_pide", "Ven a cenar a la taberna. Rosita te espera.", "luisarda_con_rosita"}, {}, {{"confianza_rosita", 1}}},

        // --- E-DIA1-CONSEC: los caminos que dan su eco a Gorran, al dia 3 y al Ocaso (ver MATRIZ_DIA1.md y out/fabrica/UNI-02-11/fable/F-DIA1_CONSECUENCIAS.md) ---
        {"informe_confia", "completa", junta({"Sembrar dudas en Verina", "Confiar en él: «Bebe conmigo»"}, junta(kCriptaSectarios, kRutaDelHeroe)), Suerte::Buena,
         {"b1_venides_informe_bastan"}, {"bnd1_venides_informe_hecho", "bnd1_venides_confianza"}, {"bnd1_venides_desconfianza", "bnd_venides_20"}, {},
         {{"dia2", "b2_noche_rosita_informa_confianza", "Contarselo todo", true}, {"dia2", "b2_noche_rosita_informa", "Contarselo todo", false},
          {"dia2", "b2_venides_castillo_v0", "", false}},
         {"b1_venides_informe_bastan", "Confiar en él: «Bebe conmigo»", "bnd1_venides_confianza"}, "b1_venides_informe_bastan: confiar en Venides (bebe conmigo)",
         {125, 5, 3, 10, {"vendas", "vendas"}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_aigren_redimido", "", "aigren_redimido"}},
        {"camorrista_muerto", "pelea", {"Separarlos [pelea sin matar]", "Rematarle", "Sacarlo y matarlo en la calle", "Contárselo todo"}, Suerte::Buena,
         {"b1_pelea", "b1_pelea_apaleado", "b1_pelea_rosita"}, {"bnd1_pelea_ganada", "bnd1_camorrista_muerto"}, {"bnd1_camorrista_echado", "bnd1_camorrista_entregado", "bnd1_capitan_te_conoce"}, {},
         {{"dia2", "b2_capitan_muerto", "Pedirle que prepare a la guardia", true}, {"dia2", "b2_capitan", "Pedirle que prepare a la guardia", false},
          {"dia2", "b2_capitan", "Soy el que te trajo", false}},
         {"b1_pelea_rosita", "Sacarlo y matarlo en la calle", "bnd1_camorrista_muerto"}, "b1_pelea_rosita: sacar al camorrista y matarlo en la calle",
         {125, -12, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_pelea_rosita", "Sacarlo y matarlo en la calle", "camorrista_ejecutado", "camorrista_a_la_guardia"}},
        {"camara_perdido", "camara", {"¿Quién es Sandes?", "Me he perdido.", "Contárselo todo"}, Suerte::Buena,
         {"b1_copista", "b1_camara_sandes"}, {"bnd1_sandes_killville_oido", "bnd1_sandes_te_conoce", "bnd1_sandes_se_va"}, {"bnd1_sandes_carta_dicha", "bnd1_sandes_enfadado"}, {},
         {{"ocaso", "b3_edul_killville", "", false}},
         {"b1_camara_sandes", "Me he perdido.", "bnd1_sandes_se_va"}, "b1_camara_sandes: «Me he perdido.»",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""}},
        {"camara_carta", "camara", {"¿Quién es Sandes?", "«Tu padre te escribe desde Killville.»", "Contárselo todo"}, Suerte::Buena,
         {"b1_copista", "b1_camara_sandes"}, {"bnd1_sandes_killville_oido", "bnd1_sandes_te_conoce", "bnd1_sandes_se_va", "bnd1_sandes_carta_dicha"}, {"bnd1_sandes_enfadado"}, {},
         {{"ocaso", "b3_edul_killville", "", true}},
         {"b1_camara_sandes", "«Tu padre te escribe desde Killville.»", "bnd1_sandes_carta_dicha"}, "b1_camara_sandes: «Tu padre te escribe desde Killville.»",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""}},
        {"edul_cara", "sectarios", junta({"Plantarles cara", "Contárselo todo"}, kCriptaSectarios), Suerte::Buena,
         {"b1_edul"}, {"bnd1_guardias_avisados", "bnd1_guarida_pelea"}, {"bnd1_unido_al_culto"}, {},
         {{"dia2", "b2_capitan_ronda", "", true}},
         {"b1_edul", "Plantarles cara", "bnd1_guardias_avisados"}, "b1_edul: plantarles cara a los sectarios (alguien llama a la guardia)",
         {165, 0, 2, 50, {}}, {"b1_guarida_ganada", "", "grantGold", "log"}, {"", "", ""}},
        {"verdaderos_ganan", "verdaderos", {"Comer algo mientras pasa la mañana (5)", "Plantarles cara", "Contárselo todo"}, Suerte::Buena,
         {"b1_verdaderos"}, {"bnd1_verdaderos_ganan", "bnd1_verdaderos_intentado"}, {"bnd1_verdaderos_vencidos", "bnd1_verdaderos_huido", "bnd1_verdaderos_unido"}, {},
         {{"ocaso", "b3_cabecilla_pan", "", true}, {"ocaso", "b3_rufo_pan", "", false}},
         {"b1_verdaderos", "Plantarles cara", "bnd1_verdaderos_ganan"}, "b1_verdaderos: plantarles cara y perder",
         {120, 5, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_verdaderos", "Plantarles cara", "verdaderos_enfrentados", "sumarse_verdaderos"},
         {}, {}, {"bnd1_verdaderos_vencidos"}},
        {"verdaderos_huido", "verdaderos", {"Comer algo mientras pasa la mañana (5)", "Plantarles cara", "Contárselo todo"}, Suerte::Buena,
         {"b1_verdaderos"}, {"bnd1_verdaderos_huido", "bnd1_verdaderos_intentado"}, {"bnd1_verdaderos_vencidos", "bnd1_verdaderos_ganan", "bnd1_verdaderos_unido"}, {},
         {{"ocaso", "b3_cabecilla_pan", "", true}, {"ocaso", "b3_rufo_pan", "", false}},
         {"b1_verdaderos", "Plantarles cara", "bnd1_verdaderos_huido"}, "b1_verdaderos: plantarles cara y huir",
         {120, 5, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"b1_verdaderos", "Plantarles cara", "verdaderos_enfrentados", "sumarse_verdaderos"},
         {}, {}, {}, {"bnd1_verdaderos_vencidos"}},
        {"pelea_perdida", "pelea", {"Separarlos [pelea sin matar]", "Contárselo todo"}, Suerte::Buena,
         {"b1_pelea"}, {"bnd1_pelea_lanzada", "bnd1_pelea_perdida"}, {"bnd1_pelea_ganada", "bnd1_pelea_huida", "bnd1_pelea_rosita"}, {},
         {{"dia2", "b2_venides_castillo_s1_a", "¿Anoche escribiste", true}},
         {"b1_pelea", "Separarlos [pelea sin matar]", "bnd1_pelea_perdida"}, "b1_pelea: separarlos a golpes y perder",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""},
         {}, {}, {"bnd1_pelea_ganada"}},
        {"pelea_huida", "pelea", {"Separarlos [pelea sin matar]", "Contárselo todo"}, Suerte::Buena,
         {"b1_pelea"}, {"bnd1_pelea_lanzada", "bnd1_pelea_huida"}, {"bnd1_pelea_ganada", "bnd1_pelea_perdida", "bnd1_pelea_rosita"}, {},
         {{"dia2", "b2_venides_castillo_s1_a", "¿Anoche escribiste", true}},
         {"b1_pelea", "Separarlos [pelea sin matar]", "bnd1_pelea_huida"}, "b1_pelea: separarlos a golpes y huir",
         {125, 0, 2, 20, {}}, {"b0_rosita", "Pagar la cama", "grantGold", "log"}, {"", "", ""},
         {}, {}, {}, {"bnd1_pelea_ganada"}},
    };
    return c;
}


// ===========================================================================================================
// E-DIA1-CONSEC — Lo que el resto de la campana hace con lo decidido en el dia 1.
//
// Desde un guardado del dia 1 cerrado (restaura()) se juega lo que sigue COMO LA RUTA DEL HEROE de demo_campana_boundington
// (Gorran con sus cuatro lios, el dia 3 y el Ocaso) hasta la etapa que se pide, con las respuestas `dice` por delante de las de
// la ruta, y el Jugador apunta lo que lee (Jugador::dicho) y los dados rivales de las tiradas (Jugador::dados).
// ===========================================================================================================
enum class Hasta { Gorran, Dia3, Sermon, Ocaso };

// `cajasLlegan`: el cargamento gris del Cojo llega a las Afueras (camino C de Gorran: nadie lo hunde ni lo cambia).
// `edulEnLasAfueras`: el heroe no pasa por Edul retenido en el Casco Antiguo, asi que le encuentra contra el abrevadero (b3_edul).
inline void sigueLaCampana(demo_bnd::Jugador& j, const std::vector<std::string>& dice, Hasta hasta, bool cajasLlegan = false,
                           bool edulEnLasAfueras = false) {
    j.ruido = std::getenv("BND_RUIDO_CAMPANA") != nullptr;  // para diagnosticar: cuenta lo que sigue paso a paso
    j.prefiere = dice;
    for (const std::string& r : kRutaDelHeroe)
        if (r != "Correr a avisar a Venides" && r != "Ir con Venides a ver al capitan")
            j.prefiere.push_back(r);  // el sermon y la charla del castillo se recorren pagina a pagina; al final sale la despedida
    fase(j, "dia_gorran");
    j.drena();                    // amanece Gorran
    j.laEspadaDeRosita();         // el encargo, la Gola Fosca, los cuatro lios, el pedestal
    j.entra("interior_taberna");
    for (int vez = 0; vez < 3 && !j.s.hasFlag("bndg_dia_cerrado"); ++vez) j.habla("skilla");  // a dormir (tras la reaccion de Rosita, si hay)
    if (hasta == Hasta::Gorran) return;
    fase(j, "dia2");
    j.drena();                    // amanece el tercero
    j.habla("skilla");
    j.entra("interior_castillo");
    j.entra("interior_castillo_p1");
    for (int vez = 0; vez < 5 && !j.s.hasFlag("bnd2_capitan_hablado"); ++vez) j.habla("capitan_guardia");  // (las reacciones del capitan van delante)
    j.entra("ciudad_oeste");
    j.entra("tienda_abandonada");
    j.entra("ciudad_surysal");
    j.habla("viejo_surysal");
    j.anochece();
    j.entra("ciudad_este");
    j.entra("mansion_barrio_alto");
    j.habla("seguidor_brinda");
    j.habla("seguidor_brinda");
    j.entra("interior_taberna");
    j.habla("venides");           // el informe de la noche
    j.duermeDia3();
    if (hasta == Hasta::Dia3) return;
    if (cajasLlegan) {
        j.s.setFlag("bgc_cajas_llegan");
        for (const char* f : {"bgc_cajas_robadas", "bgc_cajas_hundidas", "bgc_cajas_cambiadas", "bgc_media_carga"}) j.s.clearFlag(f);
    }
    fase(j, "ocaso");
    if (hasta == Hasta::Sermon) {
        j.entra("ciudad_centro");   // la plaza: el sermon de Sandes, pagina a pagina
        return;
    }
    j.ocasoBase();
    if (edulEnLasAfueras) j.zonasEvita = {"edul_casco_retenido"};
    else j.zonasPrimero = {"edul_casco_retenido"};
    j.ocasoRecorrido();
}

inline const CaminoDia1& caminoDia1(const std::string& id) {
    for (const CaminoDia1& c : caminosDia1())
        if (id == c.id) return c;
    std::fprintf(stderr, "fabricar_guardados: no hay camino %s\n", id.c_str());
    require(false);
    return caminosDia1().front();
}


// ===========================================================================================================
// E-MATRIZ-02 — Un guardado por camino del dia de Gorran.
//
// Cada CaminoGorran parte de un camino del dia 1 (CaminoDia1: el de unirse al culto o uno neutral), lo juega con el arnes,
// cierra el dia 1 y sigue con el dia de Gorran DE VERDAD (Jugador::juegaElDiaDeGorran: los lios, la subasta, dormir) hasta
// su cierre (bndg_dia_cerrado, sin dar el paso al tercer dia). Fotografia la partida en el cuarto de la taberna. La prueba
// (tests/Engine/MatrizGorranTests.cpp) la restaura en una sesion limpia, deja que amanezca el dia 3, juega su comienzo y
// mira lo que inventaria tools/matriz_decisiones.py --dia dia_gorran. Las ranuras salen de fabricar_guardados --matriz-gorran.
// ===========================================================================================================
struct CaminoGorran {
    const char* id;     // gorran_<id>.json
    const char* desde;  // id del CaminoDia1 con el que llega al dia de Gorran
    demo_bnd::Jugador::DiaDeGorran dia;
    std::vector<std::string> beats;                    // beats que se tienen que haber visto en el dia
    std::vector<std::string> ve, noVe;                 // flags al cerrar el dia (y tras amanecer el tercero)
    std::vector<std::pair<std::string, int>> igual;    // variable == n
    std::vector<Lector> lectores;                      // beats del dia 3 / Ocaso que leen la decision: se ven o no
    std::vector<std::string> sale, noSale;             // lo que se LEE al empezar el tercer dia (hablando con Rosita)
    Mutacion mutacion;                                 // una copia del JSON de Gorran en la que la decision no hace su efecto
    const char* nota;
    Cuenta cuenta;                                     // oro, moral, nivel y mochila al cerrar el dia
    Mutacion mutOro;
    Mutacion mutMoral;
};

inline const std::vector<CaminoGorran>& caminosGorran();

// Una mutacion sobre boundington_dia_gorran.json (ver Mutacion).
inline Mutacion mg(const char* beat, const char* opcion, const char* cadena, const char* hacia = "zz_mutada") {
    return Mutacion{beat, opcion, cadena, hacia, 1, 0, "dia_gorran"};
}

// Las decisiones del dia de Gorran de un camino (ver Jugador::DiaDeGorran).
inline demo_bnd::Jugador::DiaDeGorran gd(const char* lios, bool aval = false, bool tregua = false,
                                         std::vector<std::string> subasta = {"Dejar que estalle"},
                                         const char* final = "Suelta a los presos y estamos en paz.", std::vector<std::string> antes = {},
                                         bool trato = false, std::vector<std::pair<std::string, demo_bnd::Suerte>> tiradas = {}) {
    demo_bnd::Jugador::DiaDeGorran d;
    d.lios = lios;
    d.aval = aval;
    d.tregua = tregua;
    d.subasta = std::move(subasta);
    d.final = final;
    d.antes = std::move(antes);
    d.trato = trato;
    d.tiradas = std::move(tiradas);
    return d;
}

inline SaveGameData gorran(const demo_bnd::Guiones& g, const Heroe& h, const CaminoGorran& c, std::vector<std::string>* jugado = nullptr,
                           bool exigeCierre = true) {
    const CaminoDia1& d1 = caminoDia1(c.desde);
    Partida p(g, h, d1.suerte);
    demo_bnd::Jugador& j = p.j;
    j.prefiere = d1.dice;
    j.prefiere.insert(j.prefiere.end(), {"Pagar la cama", "Subir a dormir"});
    for (const auto& f : d1.fuerza) j.tiradas[f.first] = f.second;
    for (const std::string& f : d1.pierde) j.pierde.insert(f);
    for (const std::string& f : d1.huye) j.huyen.insert(f);
    j.prologo();
    fase(j, "dia1");
    recorre(j, d1.ruta);
    j.suerte = demo_bnd::Suerte::Buena;
    require(cierraElDia1(j));
    // El dia de Gorran, con sus propias respuestas (las del dia 1 ya no pintan nada).
    j.prefiere.clear();
    j.suerte = demo_bnd::Suerte::Buena;
    demo_bnd::Jugador::DiaDeGorran dg = c.dia;
    dg.amaneceElTercero = false;
    dg.exige = exigeCierre;
    fase(j, "dia_gorran");  // (con la flag privada del capitulo, como el juego)
    j.juegaElDiaDeGorran(dg);
    if (exigeCierre) require(j.s.hasFlag("bndg_dia_cerrado"));
    if (jugado != nullptr) *jugado = j.vistos;
    static const Momento kGorran{"gorran.json", "interior_taberna_p1", "", 0, 0};
    return fotografia(j, h, kGorran);
}

// E-MATRIZ-02 (capturas): el mismo camino, parado JUSTO ANTES de un momento clave del dia de Gorran ("subasta": en el salon del recinto con
// el cuerno sonado (con el camino `tregua`, la subasta con tregua, D0024); "balance": al bajar a la taberna, antes del hito y del balance de la aldea;
// "dormir": en la taberna, antes de «Subir a dormir»).
inline SaveGameData gorranParada(const demo_bnd::Guiones& g, const Heroe& h, const CaminoGorran& c, const std::string& parada) {
    CaminoGorran p = c;
    p.dia.parada = parada;
    const CaminoDia1& d1 = caminoDia1(c.desde);
    Partida pa(g, h, d1.suerte);
    demo_bnd::Jugador& j = pa.j;
    j.prefiere = d1.dice;
    j.prefiere.insert(j.prefiere.end(), {"Pagar la cama", "Subir a dormir"});
    for (const auto& f : d1.fuerza) j.tiradas[f.first] = f.second;
    for (const std::string& f : d1.pierde) j.pierde.insert(f);
    for (const std::string& f : d1.huye) j.huyen.insert(f);
    j.prologo();
    fase(j, "dia1");
    recorre(j, d1.ruta);
    j.suerte = demo_bnd::Suerte::Buena;
    require(cierraElDia1(j));
    j.prefiere.clear();
    demo_bnd::Jugador::DiaDeGorran dg = p.dia;
    dg.amaneceElTercero = false;
    fase(j, "dia_gorran");
    j.juegaElDiaDeGorran(dg);
    // Subasta: en el borde del salon (zona [54,25,66,37] de la cinematica recinto_subasta).
    static const Momento kSubasta{"m2_subasta.json", "recinto_gorran", "", 0, 0, 55, 31};
    static const Momento kDormir{"m2_dormir.json", "interior_taberna", "skilla", 0, -1};
    static const Momento kBalance{"m2_balance.json", "interior_taberna", "", 0, 0};  // al cargar, el heroe «entra» en la taberna: hito y balance de la aldea
    return fotografia(j, h, parada == "subasta" ? kSubasta : parada == "balance" ? kBalance : kDormir);
}

// Lo que tiene que cumplir el camino al restaurarse, al amanecer el tercer dia y en su comienzo. Devuelve las faltas.
inline std::vector<std::string> compruebaGorran(const demo_bnd::Guiones& g, Restaurada& r, const CaminoGorran& c) {
    std::vector<std::string> fallos;
    demo_bnd::Jugador& j = *r.j;
    const auto falta = [&](const std::string& s) { fallos.push_back(s); };
    for (const std::string& f : c.ve)
        if (!j.s.hasFlag(f)) falta("falta " + f);
    for (const std::string& f : c.noVe)
        if (j.s.hasFlag(f)) falta("sobra " + f);
    for (const auto& v : c.igual)
        if (j.s.variable(v.first) != v.second) falta(v.first + "=" + std::to_string(j.s.variable(v.first)) + " != " + std::to_string(v.second));
    for (const std::string& b : c.beats)
        if (!j.s.beatFired(b)) falta("no disparado " + b);
    {
        GameSession& sesion = *r.nivel->sesion;
        const RPG::CharacterSheet* ficha = r.nivel->cuerpo->character_sheet();
        if (sesion.gold() != c.cuenta.oro) falta("oro=" + std::to_string(sesion.gold()) + " != " + std::to_string(c.cuenta.oro));
        if (sesion.morality().value() != c.cuenta.moral)
            falta("moral=" + std::to_string(sesion.morality().value()) + " != " + std::to_string(c.cuenta.moral));
        if (sesion.morality().tramo() != moral::tramoDe(c.cuenta.moral))
            falta(std::string("tramo de moral ") + sesion.morality().label() + " != " + moral::nombreTramo(moral::tramoDe(c.cuenta.moral)));
        if (!sesion.morality().pendientes().empty()) falta("moral sin asentar: " + std::to_string(sesion.morality().pendientes().size()) + " actos pendientes");
        if (ficha->nivel != c.cuenta.nivel || ficha->progresoNivel != c.cuenta.progreso)
            falta("nivel " + std::to_string(ficha->nivel) + " (" + std::to_string(ficha->progresoNivel) + " %) != " + std::to_string(c.cuenta.nivel) + " (" +
                  std::to_string(c.cuenta.progreso) + " %)");
        std::vector<std::string> mochila = *sesion.inventory();
        std::sort(mochila.begin(), mochila.end());
        if (mochila != c.cuenta.mochila) {
            std::string m;
            for (const std::string& x : mochila) m += x + ",";
            falta("mochila [" + m + "] != la esperada de " + std::to_string(c.cuenta.mochila.size()) + " objetos");
        }
    }
    if (!j.s.hasFlag("bndg_dia_cerrado")) falta("el dia de Gorran no esta cerrado");
    if (j.s.hasFlag("bnd2_dia_empezado")) falta("el guardado ya es del tercer dia");
    // (El reloj del dia de Gorran lo lleva aldea_tiempo, no la franja: al cerrar el dia el HUD sigue en «Día 2 · Mañana»; hallazgo en MATRIZ_GORRAN.md.)
    if (RPG::franjas::rotulo(RPG::franjas::momentoDe(j.s)).rfind("Día 2", 0) != 0) falta("el guardado no trae el «Día 2»");
    // El comienzo del tercer dia, como lo hace el juego al cargar: cambio de aventura y primer fotograma (b2_amanece).
    fase(j, "dia2");
    j.drena();
    if (!j.visto("b2_amanece")) falta("no amanece el tercer dia (b2_amanece)");
    if (!j.s.hasFlag("bnd2_encargo")) falta("falta bnd2_encargo al amanecer");
    if (RPG::franjas::rotulo(RPG::franjas::momentoDe(j.s)) != "Día 3 · Mañana") falta("al amanecer el HUD no dice «Día 3 · Mañana»");
    for (const std::string& f : c.ve)
        if (!j.s.hasFlag(f)) falta("tras amanecer ya no esta " + f);
    for (const Lector& l : c.lectores) {
        const bool ve = lectorSeVe(g, l, j.s, true);
        if (ve != l.seVe)
            falta(std::string(l.dia) + ":" + l.beat + (std::string(l.opcion).empty() ? "" : std::string("/") + l.opcion) +
                  (l.seVe ? " deberia verse" : " no deberia verse"));
    }
    // Lo que se lee al empezar el tercer dia: Rosita en la taberna (la espada devuelta o su ausencia).
    j.entra("interior_taberna");
    j.habla("skilla");
    for (const std::string& t : c.sale)
        if (!j.dijo(t)) falta("no sale «" + t + "»");
    for (const std::string& t : c.noSale)
        if (j.dijo(t)) falta("sobra «" + t + "»");
    return fallos;
}

inline const CaminoGorran& caminoGorran(const std::string& id) {
    for (const CaminoGorran& c : caminosGorran())
        if (id == c.id) return c;
    std::fprintf(stderr, "fabricar_guardados: no hay camino de Gorran %s\n", id.c_str());
    require(false);
    return caminosGorran().front();
}

// --- Los caminos del dia de Gorran (la tabla de la prueba y de las ranuras) ---
// Los lectores y los efectos esperados salen de out/fabrica/UNI-02-11/MATRIZ_GORRAN.md (tools/matriz_decisiones.py --dia dia_gorran).
inline const std::vector<CaminoGorran>& caminosGorran() {
    using demo_bnd::Suerte;
    // Orden de campos: id, desde, decisiones, beats, ve, noVe, igual, lectores, sale, noSale, mutacion, nota, cuenta, mutOro, mutMoral.
    static const std::vector<CaminoGorran> c = {
        // --- los cuatro lios (D0023.1): todos caben; luego «Dejar que estalle» y «Suelta a los presos» ---
        {"cuatro_lios", "pelea_separar", gd("BADC"),
         {"bgc_hundir_exito", "bgd_sacar_exito", "b2_subasta_aviso", "b2_gorran_victoria"},
         {"bgb_cerrada", "bga_cerrada", "bgc_cerrada", "bgd_cerrada", "bgc_cajas_hundidas", "bgd_ninos_salvados", "bnd2_subasta_ya", "bnd2_gorran_perdonado",
          "bnd2_furia_alaro", "bnd2_documentos", "bnd2_preso_libre", "bndg_dia_cerrado"},
         {"bgd_mia_perdida", "bnd2_gorran_muerto", "bnd2_gorran_tregua"}, {{"gorran_misiones", 4}, {"aldea_tiempo", 6}},
         {{"ocaso", "b3_gorran", "", true}, {"ocaso", "b3_rev_teobaldo", "", true}, {"ocaso", "b3_barricada_floja", "", true}, {"ocaso", "b3_gorran_sin_jefe", "", false},
          {"ocaso", "b3_epi_iris", "", true}, {"dia2", "b2_skilla_espada_devuelta", "", true}},
         {"Le pones la espada sobre la barra"}, {"Rosita lee el trato"},
         mg("bgc_hundir_exito", "", "bgc_cajas_hundidas"), "los 4 lios cerrados, estalla la subasta y se perdona a Gorran",
         {140, 2, 3, 80, {"furia_de_alaro"}}, mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "grantGold", "log"),
         mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "gorran_perdonado", "gorran_rematado")},
        // --- solo tres lios: el de las Galerias (C) queda sin jugar; el cuerno suena igual (min 3) ---
        {"tres_lios", "pelea_separar", gd("BAD"),
         {"b2_subasta_aviso", "b2_gorran_victoria"},
         {"bgb_cerrada", "bga_cerrada", "bgd_cerrada", "bnd2_subasta_ya", "bnd2_gorran_perdonado", "bndg_dia_cerrado"},
         {"bgc_cerrada", "bgc_cajas_hundidas", "bgc_gancho", "bgd_mia_perdida"}, {{"gorran_misiones", 3}, {"aldea_tiempo", 4}},
         {{"ocaso", "b3_barricada_floja", "", false}, {"ocaso", "b3_gorran", "", true}, {"ocaso", "b3_rev_viga", "", true}},
         {"Le pones la espada sobre la barra"}, {},
         Mutacion{"b2_subasta_aviso", "", "bnd2_subasta_ya", "zz_mutada", 1, 1, "dia_gorran"}, "solo 3 lios (B, A, D): el cuerno de la subasta suena a la tercera",
         {140, 2, 3, 50, {"furia_de_alaro"}}, mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "grantGold", "log"),
         mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "gorran_perdonado", "gorran_rematado")},
        // --- Mia perdida (D0023.2): la mina vieja se hace con el reloj en 5: solo se salva a Tinet ---
        {"mia_perdida", "pelea_separar", gd("BACD"),
         {"bgd_sacar_exito_tinet"},
         {"bgd_mia_perdida", "bgd_tinet_salvado", "bgd_solo_tinet", "bgd_cerrada", "bnd2_gorran_perdonado"},
         {"bgd_ninos_salvados"}, {{"gorran_misiones", 4}},
         {{"ocaso", "b3_orfeu", "", true}, {"ocaso", "b3_gorran", "", true}},
         {"Le pones la espada sobre la barra"}, {},
         mg("bgd_sacar_exito_tinet", "", "bgd_solo_tinet"), "la mina vieja tarde: Mia perdida, solo Tinet",
         {140, 2, 3, 80, {"furia_de_alaro"}}, mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "grantGold", "log"),
         mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "gorran_perdonado", "gorran_rematado")},
        // --- tregua con Gorran (D0024): el aval de Serra, tres lios y, EN LA SUBASTA, «Proponerle una tregua» (b2_subasta_aval); la tregua le protege y se cumple salvandole la cara ---
        {"tregua", "pelea_separar", gd("BADC", true, true, {"Salvarle la cara a Gorran"}),
         {"bga_despues_bien", "b2_subasta_aval_cojo_pobre", "b2_subasta_tregua", "b2_honor_exito"},
         {"bnd2_aval_serra", "bnd2_gorran_tregua", "bnd2_tregua_cerrada", "bnd2_gorran_honor", "bnd2_gorran_redimido", "bnd2_tiene_aliados", "bnd2_joel_rescatado", "bndg_dia_cerrado"},
         {"bnd2_tregua_rota", "bnd2_comedia_pelea", "bnd2_gorran_lanzado", "bnd2_gorran_muerto", "bnd2_gorran_perdonado", "bnd2_furia_alaro", "bnd2_documentos"}, {{"gorran_misiones", 4}},
         {{"ocaso", "b3_gorran", "", false}, {"ocaso", "b3_amigos_afueras", "", true}, {"dia2", "b2_skilla_espada_devuelta", "", false}},
         {}, {"Le pones la espada sobre la barra"},
         mg("b2_subasta_aval_cojo_pobre", "Proponerle una tregua", "bnd2_gorran_tregua"), "tregua con Gorran (aval de Serra + 3 lios) pactada en la subasta, que se cierra salvandole la cara",
         {120, 2, 4, 10, {}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"),
         mg("b2_honor_exito", "", "gorran_perdonado", "gorran_rematado")},
        // --- subasta: «Dejar que estalle» y luego «Acabar con él» ---
        {"estalle_acaba", "pelea_separar", gd("BADC", false, false, {"Dejar que estalle"}, "Acabar con él (no volverá a ayudarte)"),
         {"b2_gorran_victoria"},
         {"bnd2_gorran_muerto", "bnd2_furia_alaro", "bnd2_documentos", "bnd2_gorran_decidido", "bnd2_cueva_cerrada", "bndg_dia_cerrado"},
         {"bnd2_gorran_perdonado", "bnd2_gorran_redimido", "bnd2_preso_libre", "bnd2_tiene_aliados"}, {{"gorran_misiones", 4}},
         {{"ocaso", "b3_gorran_sin_jefe", "", true}, {"ocaso", "b3_gorran", "", false}, {"ocaso", "b3_rev_teobaldo", "", false}, {"dia2", "b2_skilla_espada_devuelta", "", true}},
         {"Le pones la espada sobre la barra"}, {},
         mg("b2_gorran_victoria", "Acabar con él (no volverá a ayudarte)", "bnd2_gorran_muerto"), "la subasta estalla y se acaba con Gorran",
         {160, -5, 3, 80, {"furia_de_alaro"}}, mg("b2_gorran_victoria", "Acabar con él (no volverá a ayudarte)", "grantGold", "log"),
         mg("b2_gorran_victoria", "Acabar con él (no volverá a ayudarte)", "gorran_rematado", "gorran_perdonado")},
        // --- el culto: llega unido a la secta del dia 1 y en la mina vieja «Yo enciendo la vela» ---
        {"culto_velas", "edul_unirse", gd("BADC", false, false, {"Dejar que estalle"}, "Suelta a los presos y estamos en paz.", {"Unirte: «Yo enciendo la vela.»"}),
         {"bgd_velas_hermano"},
         {"bgd_unido", "bnd1_unido_al_culto", "bgd_ninos_perdidos", "bgd_orfeu_huido", "bgd_cerrada", "bnd2_gorran_perdonado"},
         {"bgd_ninos_salvados", "bgd_mia_perdida"}, {{"gorran_misiones", 4}, {"aldea_sospecha", 3}},
         {{"ocaso", "b3_orfeu", "", false}, {"dia2", "b2_po54_sandes", "", true}, {"ocaso", "b3_gorran", "", true}},
         {"Le pones la espada sobre la barra"}, {},
         mg("bgd_velas_hermano", "Unirte: «Yo enciendo la vela.»", "bgd_unido"), "culto: unido en el dia 1 y «Yo enciendo la vela» con los crios de la mina",
         {180, 2, 3, 80, {"furia_de_alaro"}}, mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "grantGold", "log"),
         mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "gorran_perdonado", "gorran_rematado")},
        // --- subasta: «Llevarte la Furia en el caos [Sigilo]» (sin pelea con Gorran) ---
        {"caos", "pelea_separar", gd("BADC", false, false, {"Llevarte la Furia en el caos [Sigilo]", "Sacar a Joel en el caos [Sigilo]"}),
         {"b2_caos_exito", "b2_caos_furia"},
         {"bnd2_caos_exito", "bnd2_joel_rescatado", "bnd2_furia_alaro", "bnd2_documentos", "bnd2_cueva_cerrada", "bndg_dia_cerrado"},
         {"bnd2_gorran_perdonado", "bnd2_gorran_redimido", "bnd2_gorran_vencido", "bnd2_gorran_muerto"}, {{"gorran_misiones", 4}},
         {{"ocaso", "b3_amigos_afueras", "", true}, {"ocaso", "b3_gorran", "", false}, {"dia2", "b2_skilla_espada_devuelta", "", true}},
         {"Le pones la espada sobre la barra"}, {},
         mg("b2_caos_exito", "", "bnd2_joel_rescatado"), "subasta: llevarse la Furia en el caos",
         {120, 0, 4, 10, {"furia_de_alaro"}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"), {"", "", ""}},
        // --- subasta: «Unirte a la farsa [Interpretación]» (la comedia: Joel a salvo, sin la Furia) ---
        {"farsa", "pelea_separar", gd("BADC", false, false, {"Unirte a la farsa [Interpretación]"}),
         {"b2_farsa_exito"},
         {"bnd2_farsa_exito", "bnd2_joel_rescatado", "bnd2_subasta_vista", "bndg_dia_cerrado"},
         {"bnd2_furia_alaro", "bnd2_gorran_resuelto", "bnd2_documentos", "bnd2_gorran_perdonado"}, {{"gorran_misiones", 4}},
         {{"ocaso", "b3_amigos_afueras", "", true}, {"dia2", "b2_skilla_espada_devuelta", "", false}, {"ocaso", "b3_gorran", "", false}},
         {}, {"Le pones la espada sobre la barra"},
         mg("b2_farsa_exito", "", "bnd2_joel_rescatado"), "subasta: unirse a la farsa (Joel a salvo, sin la Furia)",
         {120, 0, 4, 10, {}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"), {"", "", ""}},
        // --- el trato del Cojo: se lleva de su casa y se le lee a Gorran en voz alta ---
        {"trato_cojo", "pelea_separar", gd("BADC", false, false, {"Unirte a la farsa [Interpretación]"}, "Suelta a los presos y estamos en paz.", {}, true, {{"bnd2_farsa_pifia", Suerte::Mala}}),
         {"b2_libros_cojo", "b2_farsa_pifia", "b2_calabozo"},
         {"bnd2_trato_venta", "bnd2_gorran_contra_cojo", "bnd2_calabozo_resuelto", "bnd2_gorran_trato_visto", "bnd2_gorran_redimido", "bnd2_furia_alaro", "bnd2_tiene_aliados"},
         {"bnd2_gorran_perdonado", "bnd2_documentos", "bnd2_trato_copia"}, {{"gorran_misiones", 4}},
         {{"dia2", "b2_rosita_trato", "", true}, {"ocaso", "b3_gorran", "", false}, {"dia2", "b2_skilla_espada_devuelta", "", true}},
         {"Rosita lee el trato dos veces"}, {"Le pones la espada sobre la barra"},
         mg("b2_libros_cojo", "Llevarte el trato", "bnd2_trato_venta"), "Gorran contra el Cojo: la farsa sale mal, te llevan al calabozo y le lees el trato (el beat b2_gorran_trato no se alcanza: Gorran esta oculto)",
         {120, 2, 3, 80, {"furia_de_alaro"}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"),
         mg("b2_calabozo", "Enseñarle el trato y leérselo", "gorran_perdonado", "gorran_rematado")},
        // --- Cal Serra con mala suerte: se llevan a Iris; luego Gorran perdonado la suelta (b2_gorran_suelta_iris) ---
        {"iris_llevada", "pelea_separar", gd("BADC", false, false, {"Dejar que estalle"}, "Suelta a los presos y estamos en paz.", {"Dejar que se la lleven"}, false, {{"bga_garsa_pifia", Suerte::Mala}}),
         {"bga_ultimo", "b2_gorran_suelta_iris"},
         {"bga_ultimo_visto", "bga_serra_ofendida", "b2_gorran_suelta_iris_visto", "bga_iris_salvada", "bnd2_gorran_perdonado", "bga_cerrada"},
         {"bga_iris_llevada", "bga_garsa_exito", "bga_morvan_huido"}, {{"gorran_misiones", 4}},
         {{"ocaso", "b3_epi_iris", "", true}, {"ocaso", "b3_rev_casa", "", true}, {"ocaso", "b3_garsa", "", false}, {"ocaso", "b3_morvan", "", false}},
         {"Le pones la espada sobre la barra"}, {},
         Mutacion{"b2_gorran_suelta_iris", "", "bga_iris_salvada", "zz_mutada", 1, 1, "dia_gorran"}, "se llevan a Iris (mala suerte con la Garsa) y Gorran perdonado la suelta",
         {140, 2, 3, 50, {"furia_de_alaro"}}, mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "grantGold", "log"),
         mg("b2_gorran_victoria", "Suelta a los presos y estamos en paz.", "gorran_perdonado", "gorran_rematado")},
    };
    return c;
}



// ===========================================================================================================
// E-MATRIZ-03 — Un guardado por camino del dia 3 (boundington_segundo_dia).
//
// Cada CaminoDia3 parte de un final de Gorran (CaminoGorran: la tregua o la pelea), opcionalmente con otro dia 1 por detras (el de
// Verina), lo juega con el arnes DE VERDAD (la espada de Rosita, el castillo, la Barriada, la fiesta del Barrio Alto, el informe a
// Venides, dormir), fotografia la partida al cerrar el dia (bnd2_dia_cerrado) en el cuarto de la taberna, y la prueba
// (tests/Engine/MatrizDia3Tests.cpp) la restaura en una sesion limpia, empieza el Ocaso y mira lo que inventaria
// tools/matriz_decisiones.py --dia segundo_dia. Las ranuras salen de fabricar_guardados --matriz-dia3.
// ===========================================================================================================
struct DiaTres {
    std::vector<std::string> antes;                                 // respuestas que van por delante de las de siempre
    std::vector<std::pair<std::string, demo_bnd::Suerte>> tiradas;  // tiradas (por su flagBotch) con esta suerte aunque las demas salgan bien
    bool fiesta = true;     // false = no sube al Barrio Alto de noche (va derecho a la taberna)
    bool resaca = false;    // D0025.1: el dia 3 despierta a Xila borracho (b2_xila_resaca: desayuno caliente) en ciudad_oeste
    bool corre = false;     // D0025.3: el Ocaso empieza corriendo a avisar a Venides (ocasoBase) y despues se deja pasar la tarde
    bool delata = false;    // tras la Barriada vuelve a la plaza y habla con el sectario (b2_sectarios: «La Barriada se arma»)
    bool exige = true;      // false (las mutaciones): lo que no sale no aborta
    std::string parada;     // capturas: "fiesta" = en ciudad_este JUSTO ANTES de pisar la calle; "mansion" = dentro, junto al noble que brinda; "informe" = en la taberna antes de hablar con Venides
};

struct CaminoDia3 {
    const char* id;      // dia3_<id>.json
    const char* gorran;  // id del CaminoGorran del que parte (el final de Gorran)
    const char* desde1;  // "" = el dia 1 de ese camino; si no, el id del CaminoDia1 que lo sustituye
    std::vector<std::string> mas1, dice1;  // rutas del dia 1 que se juegan ademas (recorre) y respuestas por delante
    DiaTres dia;
    std::vector<std::string> beats;                  // beats que se tienen que haber visto en el dia 3
    std::vector<std::string> ve, noVe;               // flags al cerrar el dia
    std::vector<std::pair<std::string, int>> igual;  // variable == n
    std::vector<Lector> lectores;                    // beats del Ocaso que leen la decision: se ven o no
    std::vector<std::string> ocasoVe, ocasoNoVe;     // beats que SALTAN al empezar el Ocaso (ocasoBase)
    Mutacion mutacion;                               // una copia de boundington_segundo_dia.json en la que la decision no hace su efecto
    const char* nota;
    Cuenta cuenta;
    Mutacion mutOro, mutMoral;
    bool espera = false;  // el Ocaso empieza sin avisar a Venides: se espera a la tarde en la taberna (el aviso de Xila, evento xila_avisa_a_venides)
    std::vector<std::string> ocasoFlags;  // D0025: flags que tiene que haber puesto el comienzo del Ocaso (p. ej. bnd3_xila_avisa del evento xila_avisa_a_venides)
};

inline const std::vector<CaminoDia3>& caminosDia3();

inline Mutacion m3(const char* beat, const char* opcion, const char* cadena, const char* hacia = "zz_mutada", int salta = 0) {
    return Mutacion{beat, opcion, cadena, hacia, 1, salta, "segundo_dia"};
}

// Lo que contesta el heroe en el dia 3 cuando no hay nada propio (la ruta del heroe de demo_campana_boundington, del encargo de Rosita al Ocaso).
inline const std::vector<std::string>& baseDia3() {
    static const std::vector<std::string> b = [] {
        std::vector<std::string> r;
        bool dentro = false;
        for (const std::string& x : kRutaDelHeroe) {
            if (x == "Devolvérsela y contarle todo") dentro = true;
            if (dentro && x != "Correr a avisar a Venides") r.push_back(x);
        }
        r.push_back("Venides hablaria por mi");
        return r;
    }();
    return b;
}

// El dia 3 como lo juega el jugador (demo_campana_boundington: juegaHeroe), con las decisiones de `d`. Se llega con el tercer dia ya amanecido.
inline void juegaElDia3(demo_bnd::Jugador& j, const DiaTres& d) {
    const std::vector<std::string> antes = j.prefiere;
    j.prefiere = d.antes;
    j.prefiere.insert(j.prefiere.end(), baseDia3().begin(), baseDia3().end());
    for (const auto& t : d.tiradas) j.tiradas[t.first] = t.second;
    j.entra("interior_taberna");
    j.habla("skilla");  // la espada devuelta (o no) y lo que Rosita cuenta
    j.entra("interior_castillo");
    j.entra("interior_castillo_p1");
    for (int vez = 0; vez < 5 && !j.s.hasFlag("bnd2_capitan_hablado"); ++vez) j.habla("capitan_guardia");
    for (int vez = 0; vez < 2 && !j.s.hasFlag("bnd2_guardia_preparada") && !j.s.hasFlag("b2_castillo_prueba_cerrada"); ++vez) j.habla("capitan_guardia");  // b2_castillo_prueba
    j.entra("ciudad_oeste");
    if (d.resaca) {
        require(j.seVe("xila"));  // D0025.1: borracho y sin hablarle, se ve el dia 3
        j.prefiere.insert(j.prefiere.begin(), "Pagarle un desayuno caliente (6)");
        j.habla("xila");
        j.prefiere.erase(j.prefiere.begin());
        require(!d.exige || (j.s.hasFlag("bnd1_xila_honor") && !j.s.hasFlag("bnd1_xila_borracho")));
        require(!j.seVe("xila"));  // ya recuperado, vuelve a esconderse como siempre
    }
    j.entra("tienda_abandonada");  // el disfraz
    j.entra("ciudad_surysal");
    j.habla("viejo_surysal");  // la Barriada: Zarela, los saqueadores, el ritual (y la noche)
    if (d.delata) {
        j.entra("ciudad_centro");
        j.habla("sectario_perdido");
    }
    j.anochece();
    if (d.fiesta) {
        if (d.parada == "fiesta") {
            j.prefiere = antes;
            return;
        }
        j.entra("ciudad_este");  // b2_fiesta_puerta: el vigilante, el cobrador, Edul
        j.entra("mansion_barrio_alto");
        if (d.parada == "mansion") {
            j.prefiere = antes;
            return;
        }
        if (j.seVe("seguidor_brinda")) {  // dentro de la fiesta (bnd2_fiesta_dentro): el noble que brinda
            j.habla("seguidor_brinda");
            j.habla("seguidor_brinda");
        } else {
            require(!d.exige || false);
        }
    }
    j.entra("interior_taberna");
    if (d.parada == "informe") {
        j.prefiere = antes;
        return;
    }
    for (int vez = 0; vez < 3 && !j.s.hasFlag("bnd2_informe_dado") && !j.s.hasFlag("bnd2_informe_callado"); ++vez) j.habla("venides");  // el informe de la noche
    j.duermeDia3();
    j.prefiere = antes;
}

inline const CaminoDia3& caminoDia3(const std::string& id) {
    for (const CaminoDia3& c : caminosDia3())
        if (id == c.id) return c;
    std::fprintf(stderr, "fabricar_guardados: no hay camino del dia 3 %s\n", id.c_str());
    require(false);
    return caminosDia3().front();
}

// Juega el dia 1 (el de `c`), el dia de Gorran (el de su CaminoGorran) y el dia 3, y fotografia el cierre del tercer dia.
inline SaveGameData dia3(const demo_bnd::Guiones& g, const Heroe& h, const CaminoDia3& c, std::vector<std::string>* jugado = nullptr, bool exigeCierre = true,
                         const std::string& parada = "") {
    const CaminoGorran& cg = caminoGorran(c.gorran);
    const CaminoDia1& d1 = caminoDia1(std::string(c.desde1).empty() ? cg.desde : c.desde1);
    Partida p(g, h, d1.suerte);
    demo_bnd::Jugador& j = p.j;
    j.prefiere = c.dice1;
    j.prefiere.insert(j.prefiere.end(), d1.dice.begin(), d1.dice.end());
    j.prefiere.insert(j.prefiere.end(), {"Pagar la cama", "Subir a dormir"});
    for (const auto& f : d1.fuerza) j.tiradas[f.first] = f.second;
    for (const std::string& f : d1.pierde) j.pierde.insert(f);
    for (const std::string& f : d1.huye) j.huyen.insert(f);
    j.prologo();
    fase(j, "dia1");
    // D0025.1: Xila despreciado y emborrachado se hace por la manana, antes que la ruta del dia 1 (que acaba de noche).
    const bool xilaBorracho = std::find(c.mas1.begin(), c.mas1.end(), "xila_borracho") != c.mas1.end();
    if (xilaBorracho) {
        j.entra("interior_taberna");
        j.habla("skilla");  // la cama
        recorre(j, "xila_borracho");
    }
    recorre(j, d1.ruta);
    for (const std::string& r : c.mas1)
        if (r != "xila_borracho") recorre(j, r);
    j.suerte = demo_bnd::Suerte::Buena;
    require(cierraElDia1(j) || !exigeCierre);
    j.prefiere.clear();
    demo_bnd::Jugador::DiaDeGorran dg = cg.dia;
    dg.amaneceElTercero = false;
    dg.exige = exigeCierre;
    fase(j, "dia_gorran");
    j.juegaElDiaDeGorran(dg);
    require(j.s.hasFlag("bndg_dia_cerrado") || !exigeCierre);
    fase(j, "dia2");
    j.drena();  // b2_amanece
    DiaTres dt = c.dia;
    dt.exige = exigeCierre;
    if (!parada.empty()) dt.parada = parada;
    juegaElDia3(j, dt);
    if (exigeCierre && dt.parada.empty()) require(j.s.hasFlag("bnd2_dia_cerrado"));
    if (jugado != nullptr) *jugado = j.vistos;
    static const Momento kDia3{"dia3.json", "interior_taberna_p1", "", 0, 0};
    static const Momento kFiesta{"m3_fiesta.json", "ciudad_este", "", 0, 0};
    static const Momento kInforme{"m3_informe.json", "interior_taberna", "venides", 0, 1};
    static const Momento kMansion{"m3_mansion.json", "mansion_barrio_alto", "seguidor_brinda", 0, 1};
    return fotografia(j, h, dt.parada == "fiesta" ? kFiesta : dt.parada == "informe" ? kInforme : dt.parada == "mansion" ? kMansion : kDia3);
}

// El comienzo del Ocaso, como lo hace el juego al cargar: cambio de aventura y la base (el sermon, Venides, las campanas) o, con `espera`,
// sin avisar a Venides: la taberna y «Esperar a la tarde» (cruza ocaso_tarde y salta el evento de Xila).
inline void empiezaElOcaso(demo_bnd::Jugador& j, const CaminoDia3& c) {
    fase(j, "ocaso");
    if (c.espera) {
        j.entra("ciudad_centro");
        j.entra("interior_taberna");
        j.prefiere.insert(j.prefiere.begin(), "Esperar a la tarde");
        j.habla("skilla");
        j.entra("interior_cuartel");
        j.drena();
    } else {
        j.ocasoBase();
        if (c.dia.corre) {  // D0025.3: se corrio a avisar a Venides (bnd3_avisado, campanas) y la tarde se cruza despues (sin Venides, solo cruza)
            j.s.setVariable("franja", 1);
            j.drena();
        }
    }
}

// Lo que tiene que cumplir el camino al restaurarse y al empezar el Ocaso. Devuelve las faltas.
inline std::vector<std::string> compruebaDia3(const demo_bnd::Guiones& g, Restaurada& r, const CaminoDia3& c) {
    std::vector<std::string> fallos;
    demo_bnd::Jugador& j = *r.j;
    const auto falta = [&](const std::string& s) { fallos.push_back(s); };
    for (const std::string& f : c.ve)
        if (!j.s.hasFlag(f)) falta("falta " + f);
    for (const std::string& f : c.noVe)
        if (j.s.hasFlag(f)) falta("sobra " + f);
    for (const auto& v : c.igual)
        if (j.s.variable(v.first) != v.second) falta(v.first + "=" + std::to_string(j.s.variable(v.first)) + " != " + std::to_string(v.second));
    for (const std::string& b : c.beats)
        if (!j.s.beatFired(b)) falta("no disparado " + b);
    {
        GameSession& sesion = *r.nivel->sesion;
        const RPG::CharacterSheet* ficha = r.nivel->cuerpo->character_sheet();
        if (sesion.gold() != c.cuenta.oro) falta("oro=" + std::to_string(sesion.gold()) + " != " + std::to_string(c.cuenta.oro));
        if (sesion.morality().value() != c.cuenta.moral)
            falta("moral=" + std::to_string(sesion.morality().value()) + " != " + std::to_string(c.cuenta.moral));
        if (!sesion.morality().pendientes().empty()) falta("moral sin asentar: " + std::to_string(sesion.morality().pendientes().size()) + " actos pendientes");
        if (ficha->nivel != c.cuenta.nivel || ficha->progresoNivel != c.cuenta.progreso)
            falta("nivel " + std::to_string(ficha->nivel) + " (" + std::to_string(ficha->progresoNivel) + " %) != " + std::to_string(c.cuenta.nivel) + " (" +
                  std::to_string(c.cuenta.progreso) + " %)");
        std::vector<std::string> mochila = *sesion.inventory();
        std::sort(mochila.begin(), mochila.end());
        if (mochila != c.cuenta.mochila) {
            std::string m;
            for (const std::string& x : mochila) m += x + ",";
            falta("mochila [" + m + "] != la esperada de " + std::to_string(c.cuenta.mochila.size()) + " objetos");
        }
    }
    if (!j.s.hasFlag("bnd2_dia_cerrado")) falta("el dia 3 no esta cerrado");
    if (j.s.hasFlag("bnd_ocaso_empezado")) falta("el guardado ya es del Ocaso");
    for (const Lector& l : c.lectores) {
        const bool ve = lectorSeVe(g, l, j.s, true, true);
        if (ve != l.seVe)
            falta(std::string(l.dia) + ":" + l.beat + (std::string(l.opcion).empty() ? "" : std::string("/") + l.opcion) +
                  (l.seVe ? " deberia verse" : " no deberia verse"));
    }
    // El comienzo del Ocaso, como lo hace el juego al cargar: cambio de aventura y la base (el sermon, Venides, las campanas).
    empiezaElOcaso(j, c);
    for (const std::string& b : c.ocasoVe)
        if (!j.visto(b)) falta("el Ocaso no cuenta " + b);
    for (const std::string& b : c.ocasoNoVe)
        if (j.visto(b)) falta("el Ocaso cuenta " + b);
    for (const std::string& f : c.ocasoFlags)
        if (!j.s.hasFlag(f)) falta("el Ocaso no pone " + f);
    return fallos;
}

// Diagnostico (MATRIZ_DIAG=1): lo que deja el camino, para escribir la tabla.
inline void describeDia3(const demo_bnd::Guiones& g, Restaurada& r, const CaminoDia3& c) {
    demo_bnd::Jugador& j = *r.j;
    std::fprintf(stderr, "=== %s\n flags:", c.id);
    for (const std::string& f : j.s.flags())
        if (f.rfind("bnd2_", 0) == 0 || f.rfind("bnd1_", 0) == 0 || f == "bnd_venides_20" || f == "bnd_venides_10") std::fprintf(stderr, " %s", f.c_str());
    std::fprintf(stderr, "\n vars:");
    for (const auto& v : j.s.variables()) std::fprintf(stderr, " %s=%d", v.first.c_str(), v.second);
    GameSession& sesion = *r.nivel->sesion;
    const RPG::CharacterSheet* ficha = r.nivel->cuerpo->character_sheet();
    std::vector<std::string> mochila = *sesion.inventory();
    std::sort(mochila.begin(), mochila.end());
    std::fprintf(stderr, "\n oro=%d moral=%d nivel=%d prog=%d mochila:", sesion.gold(), sesion.morality().value(), ficha->nivel, ficha->progresoNivel);
    for (const std::string& m : mochila) std::fprintf(stderr, " %s", m.c_str());
    empiezaElOcaso(j, c);
    std::fprintf(stderr, "\n ocaso vistos:");
    for (const std::string& b : j.vistos) std::fprintf(stderr, " %s", b.c_str());
    std::fprintf(stderr, "\n ocaso flags:");
    for (const std::string& f : j.s.flags())
        if (f.rfind("bnd3_", 0) == 0) std::fprintf(stderr, " %s", f.c_str());
    std::fprintf(stderr, "\n");
    (void)g;
}

// --- Los caminos del dia 3 (la tabla de la prueba y de las ranuras) ---
// Los lectores y los efectos esperados salen de out/fabrica/UNI-02-11/MATRIZ_DIA3.md (tools/matriz_decisiones.py --dia segundo_dia).
inline const std::vector<CaminoDia3>& caminosDia3() {
    using demo_bnd::Suerte;
    static const std::vector<CaminoDia3> c = {
        // Orden de campos: id, gorran, desde1, mas1, dice1, dia, beats, ve, noVe, igual, lectores, ocasoVe, ocasoNoVe, mutacion, nota, cuenta, mutOro, mutMoral, espera.
        // --- la fiesta del Barrio Alto: se entra (vigilante, mansion, el noble que brinda) y se le senala a Venides ---
        {"fiesta_dentro", "tregua", "", {}, {}, DiaTres{},
         {"b2_fiesta_puerta", "b2_dentro", "b2_sonsaca_exito", "b2_venides_noche"},
         {"bnd2_fiesta_dentro", "bnd2_fiesta_nobles", "bnd2_noble_senalado", "bnd2_conspiradores", "bnd2_informe_dado", "bnd2_venides_leal", "bnd_venides_10", "bnd2_dia_cerrado"},
         {"bnd2_noble_chantajeado", "bnd2_venides_sin_aviso", "bnd2_barriada_delatada", "bnd2_rosita_verina"}, {},
         {{"ocaso", "b3_plaza_vacia", "", false}, {"ocaso", "b3_sermon_s3_a", "Buscar al noble del brindis", true}, {"ocaso", "b3_zarela", "", true}},
         {"b3_plaza", "b3_avisar_venides", "b3_venides_arenga_10", "b3_capitan_ronda"}, {"b3_plaza_vacia"},
         m3("b2_sonsaca_exito", "Senalarselo a Venides", "bnd2_noble_senalado"), "la fiesta: se entra, se sonsaca al noble y se le senala a Venides",
         {170, 11, 5, 47, {"amuleto_gitano", "entrada_opera", "vendas"}}, m3("b2_venides_noche", "Contarselo todo", "grantGold", "log"),
         m3("b2_sonsaca_exito", "Senalarselo a Venides", "noble_senalado", "noble_chantajeado")},
        // --- no entrar en la fiesta: el heroe no sube al Barrio Alto; la plaza del Ocaso queda vacia (nada que Sandes tema) ---
        {"sin_fiesta", "tregua", "", {}, {}, [] { DiaTres d; d.fiesta = false; return d; }(),
         {"b2_venides_noche", "b2_cierre"},
         {"bnd2_informe_dado", "bnd2_dia_cerrado"},
         {"bnd2_fiesta_dentro", "bnd2_fiesta_calle", "bnd2_fiesta_nobles", "bnd2_noble_senalado", "bnd2_conspiradores"}, {},
         {{"ocaso", "b3_plaza_vacia", "", true}, {"ocaso", "b3_sermon_s3_a", "Buscar al noble del brindis", false}},
         {"b3_plaza_vacia", "b3_campanas"}, {"b3_plaza"},
         Mutacion{"b3_plaza_vacia", "", "bnd3_esc04_sermon", "zz_mutada", 1, 0, "ocaso"}, "no ir a la fiesta: la plaza del Ocaso sale vacia (b3_plaza_vacia)",
         {170, 9, 5, 19, {"amuleto_gitano", "entrada_opera", "vendas"}}, m3("b2_venides_noche", "Contarselo todo", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado")},
        // --- Verina: confianza_rosita >= 3 (Verina salvada de la guarida + proteger al borracho + devolver la espada y contarlo todo) ---
        {"verina_arriba", "cuatro_lios", "aigren_verina", {"pelea"}, {"Proteger al borracho dormido"}, [] { DiaTres d; d.antes = {"Devolvérsela y contarle todo"}; return d; }(),
         {"b2_skilla_espada_devuelta", "b2_rosita_revela", "b2_rosita_verina_arriba"},
         {"bnd1_verina_salvada_guarida", "bnd1_pelea_rosita", "bnd2_rosita_revelada", "bnd2_rosita_verina", "bnd2_skilla_pagada", "bnd2_aliado_skilla"}, {"bnd2_rosita_desconfia"},
         {{"confianza_rosita", 3}}, {}, {"b3_avisar_venides"}, {},
         m3("b2_rosita_verina_arriba", "", "bnd1_verina_salvada_guarida", "bnd1_zz_mutada"), "Verina arriba: solo se alcanza con Verina salvada + proteger al borracho + devolver y contarlo todo (confianza 3)",
         {240, 16, 5, 76, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "pocion", "vendas"}}, m3("b2_noche_rosita_informa", "Contarselo todo", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado")},
        // --- decirle a Rosita que no encontraste la espada ---
        {"espada_mentira", "cuatro_lios", "", {}, {}, [] { DiaTres d; d.antes = {"Decirle que no la encontraste"}; return d; }(),
         {"b2_skilla_espada_devuelta", "b2_venides_noche_todo"},
         {"bnd2_rosita_desconfia", "bnd2_skilla_pagada", "bnd2_furia_alaro", "bnd2_informe_dado"},
         {"bnd2_aliado_skilla", "bnd2_rosita_revelada", "bnd2_rosita_verina"}, {},
         {{"ocaso", "b3_rosita_calle", "", false}, {"ocaso", "b3_taberna_cerrada", "", true}}, {"b3_avisar_venides"}, {},
         m3("b2_skilla_espada_devuelta", "Decirle que no la encontraste", "bnd2_rosita_desconfia"), "mentir a Rosita sobre la espada: desconfia (taberna cerrada en el Ocaso)",
         {189, 6, 5, 17, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "vendas"}}, m3("b2_venides_noche_todo", "Contarselo todo", "grantGold", "log"),
         m3("b2_skilla_espada_devuelta", "Decirle que no la encontraste", "furia_de_alaro_robada", "furia_de_alaro_devuelta")},
        // --- no contarle nada a Venides: no esta en la base el dia del Ocaso (A0) ---
        {"informe_nada", "tregua", "", {}, {}, [] { DiaTres d; d.antes = {"No contarle nada"}; return d; }(),
         {"b2_venides_noche"},
         {"bnd2_informe_callado", "bnd2_venides_sin_aviso"}, {"bnd2_informe_dado", "bnd2_venides_leal", "bnd_venides_10"}, {},
         {{"ocaso", "b3_venides_retenido", "", true}, {"ocaso", "b3_aviso_en_base", "", false}}, {"b3_venides_retenido", "b3_campanas_solas"}, {"b3_avisar_venides", "b3_venides_arenga_10"},
         m3("b2_venides_noche", "No contarle nada", "bnd2_venides_sin_aviso"), "no contarle nada a Venides: el Ocaso empieza sin el (b3_venides_retenido)",
         {120, 9, 5, 47, {"amuleto_gitano", "entrada_opera", "vendas"}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado")},
        // --- el aviso de Xila: Xila aliado (dia 1) corre a avisar a Venides la tarde del Ocaso... si Venides esta en la base ---
        {"xila_avisa", "cuatro_lios", "informe_cuenta_todo", {}, {}, DiaTres{},
         {"b2_noche_xila", "b2_noche_rosita_informa"},
         {"bnd1_xila_honor", "bnd2_informe_dado", "bnd2_venides_leal"}, {"bnd2_venides_sin_aviso"}, {},
         {}, {"b3_tarde_aviso", "b3_xila_avisa", "b3_aviso_en_base", "b3_xila_vendas"}, {},
         m3("b2_noche_rosita_informa", "Contarselo todo", "bnd2_venides_leal", "bnd2_venides_sin_aviso"), "el aviso de Xila: aliado + informe dado, Xila avisa a Venides la tarde del Ocaso",
         {280, 16, 6, 64, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "vendas", "vendas", "vendas"}}, m3("b2_noche_rosita_informa", "Contarselo todo", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado"), true},
        // --- ... y si Venides no sabe nada, nadie avisa (xila_avisa_a_venides exige no bnd2_venides_sin_aviso) ---
        {"xila_sin_aviso", "cuatro_lios", "informe_cuenta_todo", {}, {}, [] { DiaTres d; d.antes = {"No contarle nada"}; return d; }(),
         {"b2_noche_xila", "b2_noche_rosita_informa"},
         {"bnd1_xila_honor", "bnd2_informe_callado", "bnd2_venides_sin_aviso"}, {"bnd2_informe_dado"}, {},
         {}, {"b3_tarde_aviso"}, {"b3_xila_avisa", "b3_aviso_en_base"},
         m3("b2_noche_rosita_informa", "No contarle nada", "bnd2_venides_sin_aviso"), "Xila aliado pero Venides sin aviso: el evento del aviso de Xila no salta",
         {230, 15, 6, 64, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "vendas", "vendas", "vendas"}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado"), true},
        // --- D0025.1: Xila borracho el dia 1 se ve el dia 3, se le despierta (desayuno caliente) y avisa a Venides la tarde del Ocaso ---
        {"xila_resaca", "tregua", "", {"xila_borracho"}, {}, [] { DiaTres d; d.resaca = true; return d; }(),
         {"b2_xila_resaca", "b2_xila_resaca_bien", "b2_venides_noche"},
         {"bnd1_xila_honor", "bnd1_aliado_xila", "bnd2_xila_resaca_visto", "bnd2_xila_resaca_cerrada", "bnd2_informe_dado", "bnd2_venides_leal"}, {"bnd1_xila_borracho", "bnd2_venides_sin_aviso"}, {},
         {}, {"b3_tarde_aviso", "b3_xila_avisa", "b3_aviso_en_base"}, {},
         m3("b2_xila_resaca", "Pagarle un desayuno caliente (6)", "bnd2_resaca_exito"), "Xila borracho: se ve el dia 3, el desayuno le recupera y avisa a Venides la tarde del Ocaso",
         {163, 8, 5, 47, {"amuleto_gitano", "entrada_opera", "vendas"}}, m3("b2_venides_noche", "Contarselo todo", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado"), true, {"bnd3_xila_avisa"}},
        // --- D0025.2: Verina arriba con confianza_rosita 2 (Verina salvada + devolver la espada y contarlo todo, sin proteger al borracho) ---
        {"verina_arriba_2", "cuatro_lios", "aigren_verina", {}, {}, [] { DiaTres d; d.antes = {"Devolvérsela y contarle todo"}; return d; }(),
         {"b2_skilla_espada_devuelta", "b2_rosita_revela", "b2_rosita_verina_arriba"},
         {"bnd1_verina_salvada_guarida", "bnd2_rosita_revelada", "bnd2_rosita_verina"}, {"bnd2_rosita_desconfia"},
         {{"confianza_rosita", 2}}, {}, {}, {},
         m3("b2_rosita_verina_arriba", "", "bnd1_verina_salvada_guarida", "bnd1_zz_mutada"), "Verina arriba con confianza 2: Verina salvada + devolver y contarlo todo",
         {190, 15, 5, 76, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "pocion", "vendas"}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado")},
        // --- D0025.3: Xila aliado (dia 1), el jugador corre a avisar a Venides por la manana (campanas) y la tarde Xila avisa igual ---
        {"xila_corre", "cuatro_lios", "informe_cuenta_todo", {}, {}, [] { DiaTres d; d.corre = true; return d; }(),
         {"b2_noche_xila", "b2_noche_rosita_informa"},
         {"bnd1_xila_honor", "bnd2_informe_dado", "bnd2_venides_leal"}, {"bnd2_venides_sin_aviso"}, {},
         {}, {"b3_avisar_venides"}, {},
         m3("b2_noche_rosita_informa", "Contarselo todo", "bnd2_venides_leal", "bnd2_venides_sin_aviso"), "Xila aliado y el jugador corre a avisar a Venides: Xila avisa igual la tarde (bnd3_xila_avisa)",
         {280, 16, 6, 64, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "vendas", "vendas", "vendas"}}, m3("b2_noche_rosita_informa", "Contarselo todo", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado"), false, {"bnd3_xila_avisa", "bnd3_avisado"}},
        // --- chantajear al noble de la fiesta en vez de senalarlo ---
        {"noble_chantaje", "tregua", "", {}, {}, [] { DiaTres d; d.antes = {"Chantajearle"}; return d; }(),
         {"b2_sonsaca_exito"},
         {"bnd2_noble_chantajeado", "bnd2_fiesta_dentro"}, {"bnd2_noble_senalado"}, {},
         {{"ocaso", "b3_sermon_s3_a", "Buscar al noble del brindis", false}}, {"b3_plaza"}, {},
         m3("b2_sonsaca_exito", "Chantajearle", "bnd2_noble_chantajeado"), "chantajear al noble: +30 de oro, moral a la baja y sin noble que senalar",
         {200, 7, 5, 47, {"amuleto_gitano", "entrada_opera", "vendas"}}, m3("b2_sonsaca_exito", "Chantajearle", "grantGold", "log"),
         m3("b2_sonsaca_exito", "Chantajearle", "noble_chantajeado", "noble_senalado")},
        // --- delatar la Barriada a los sectarios ---
        {"barriada_delata", "cuatro_lios", "", {}, {}, [] { DiaTres d; d.delata = true; d.antes = {"La Barriada se arma: empalizada y guerreros"}; return d; }(),
         {"b2_sectarios"},
         {"bnd2_barriada_delatada", "bnd2_aliado_gitanos"}, {}, {},
         {{"ocaso", "b3_zarela", "", false}, {"ocaso", "b3_huida_gitanos", "", false}}, {"b3_avisar_venides"}, {},
         m3("b2_sectarios", "La Barriada se arma: empalizada y guerreros", "bnd2_barriada_delatada"), "delatar a la Barriada: los gitanos no llegan a la huida (b3_zarela, b3_huida_gitanos)",
         {240, 8, 5, 46, {"amuleto_gitano", "entrada_opera", "furia_de_alaro", "vendas"}}, m3("b2_noche_rosita_informa", "Contarselo todo", "grantGold", "log"),
         m3("b2_sectarios", "La Barriada se arma: empalizada y guerreros", "barriada_delatada", "guardia_preparada")},
        // --- retirarse ante el capitan: la guardia no se prepara ---
        {"castillo_retirada", "tregua", "", {}, {}, [] { DiaTres d; d.antes = {"Retirarse"}; return d; }(),
         {"b2_castillo_prueba"},
         {"b2_castillo_prueba_cerrada"}, {"bnd2_guardia_preparada"}, {},
         {{"ocaso", "b3_capitan_ronda", "", false}}, {"b3_avisar_venides"}, {"b3_capitan_ronda"},
         m3("b2_castillo_prueba", "Retirarse", "b2_castillo_prueba_cerrada"), "retirarse ante el capitan: la guardia no se prepara (sin b3_capitan_ronda)",
         {170, 9, 5, 47, {"amuleto_gitano", "entrada_opera", "vendas"}}, mg("b2_cueva_llegada", "Pagar el paso (5)", "grantGold", "log"),
         m3("b2_saqueadores_vencidos", "", "barriada_defendida", "noble_chantajeado")},
    };
    return c;
}

}  // namespace fabricar
