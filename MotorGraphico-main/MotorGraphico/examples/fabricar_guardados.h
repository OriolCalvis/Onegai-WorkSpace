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

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "Game/Personajes/Vestuario.h"
#include "Game/NewGameFlow.h"
#include "Game/SaveGame.h"
#include "Game/GameSession.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "Project/ProjectIndex.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
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

// Fotografia de la partida del arnes en `momento`.
inline SaveGameData fotografia(const demo_bnd::Jugador& j, const Heroe& heroe, const Momento& momento) {
    auto montado = montaNivel(momento.nivel, heroe.ficha);
    montado->sesion->setNarrative(&j.e, const_cast<RPG::NarrativeState*>(&j.s));
    SaveGameData data = montado->sesion->captureSaveGame(montado->ruta);
    // Los objetos que comparten casilla (ciudad_centro: la puerta de la herreria y aigren_carro en (6,59), y otros
    // dos) van enteros: restoreSaveGame los distingue por id desde E-GUION-02b.
    data.playerPosition = casillaJuntoA(*montado->sesion, montado->nivel, momento);
    data.hero = heroe.guardado;
    data.characterName = heroe.ficha.displayName;
    data.playerHealth = data.playerMaxHealth = heroe.ficha.healthCap();
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

}  // namespace fabricar
