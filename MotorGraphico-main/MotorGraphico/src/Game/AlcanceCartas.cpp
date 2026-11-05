// AV13-13 — ver include/Game/AlcanceCartas.h.
#include "Game/AlcanceCartas.h"
#include "Game/AreaDeCarta.h"
#include <cstdlib>
#include <cctype>

#include "Game/BattleState.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>

namespace Game {

int casillasDeAlcance(RPG::SkillRange rango, RPG::SkillTarget objetivo) {
    switch (rango) {
        case RPG::SkillRange::SELF: return 0;
        case RPG::SkillRange::MELEE: return 1;
        case RPG::SkillRange::SHORT: return 4;
        case RPG::SkillRange::MEDIUM: return 8;
        case RPG::SkillRange::LONG: return 12;
        case RPG::SkillRange::UNKNOWN: break;
    }
    return objetivo == RPG::SkillTarget::SELF ? 0 : 4;
}

namespace {

bool enPie(const BattleParticipant& p) { return p.combatant != nullptr && p.combatant->isAlive(); }

// Si `desde` llega a `hasta` con `casillas`: a esa distancia, y viendole si no esta al lado.
bool llega(const TableroCombate& t, const std::string& desde, const std::string& hasta, int casillas) {
    const auto a = t.dondeEsta(desde), b = t.dondeEsta(hasta);
    if (!a || !b) return false;
    const int d = TableroCombate::distancia(*a, *b);
    return d <= casillas && (d <= 1 || t.seVen(*a, *b));
}

void ponUna(CartaBarra& c, const BattleState& batalla, const TableroCombate& t) {
    if (c.id == "#pocion" || c.id == "#huir") return;
    c.alcance = casillasDeAlcance(c.rango, c.objetivo);
    const std::string& heroe = batalla.allies()[static_cast<std::size_t>(std::max(0, batalla.aliadoEnTurno()))].id;  // AV13-29: el que juega
    for (const BattleParticipant& p : batalla.allies())
        c.aliadosATiro.push_back(enPie(p) && (p.id == heroe || llega(t, heroe, p.id, c.alcance)));
    for (const BattleParticipant& p : batalla.enemies())
        c.enemigosATiro.push_back(enPie(p) && llega(t, heroe, p.id, c.alcance));
    // Una carta contra enemigos que no llega a ninguno no se puede jugar: dice por que.
    const bool contraEnemigos = c.objetivo == RPG::SkillTarget::SINGLE_ENEMY ||
                                c.objetivo == RPG::SkillTarget::ZONE_RADIUS ||
                                c.objetivo == RPG::SkillTarget::ALL_ENEMIES;
    const bool alguno = std::find(c.enemigosATiro.begin(), c.enemigosATiro.end(), true) != c.enemigosATiro.end();
    if (!contraEnemigos || alguno || c.estado != CartaBarra::Estado::Lista) return;
    c.estado = CartaBarra::Estado::Bloqueada;
    c.motivo = c.alcance <= 1 ? "nadie al lado: acércate"
                              : "nadie a tiro (" + std::to_string(c.alcance) + " casillas, viéndole)";
}

}  // namespace

std::vector<GridCoord> casillasATiro(const TableroCombate& t, const std::string& quien, int casillas) {
    std::vector<GridCoord> salida;
    const auto desde = t.dondeEsta(quien);
    if (!desde || casillas < 1) return salida;
    for (int y = t.origen().y; y < t.origen().y + t.alto(); ++y) {
        for (int x = t.origen().x; x < t.origen().x + t.ancho(); ++x) {
            const GridCoord c{x, y};
            const int d = TableroCombate::distancia(*desde, c);
            if (d == 0 || d > casillas || !t.transitable(c)) continue;
            if (d <= 1 || t.seVen(*desde, c)) salida.push_back(c);
        }
    }
    return salida;
}

void ponAlcances(VistaBarra& v, const BattleState& batalla) {
    const TableroCombate* t = batalla.tablero();
    if (t == nullptr || batalla.allies().empty()) return;
    for (CartaBarra& c : v.basicas) ponUna(c, batalla, *t);
    for (CartaBarra& c : v.mano) ponUna(c, batalla, *t);
}

bool esMuro(const TableroCombate& t, GridCoord c) { return !t.transitable(c) && t.quienEsta(c) == nullptr; }

// AV13-14 — El area de la carta (ver Game/AreaDeCarta.h).
std::optional<AreaDeCarta> areaDe(const RPG::SkillDefinition& def) {
    using T = RPG::SkillTarget;
    if (def.target != T::ZONE_RADIUS && def.target != T::ALL_ENEMIES) return std::nullopt;
    std::string texto = def.description;
    std::transform(texto.begin(), texto.end(), texto.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    const auto dice = [&](const char* trozo) { return texto.find(trozo) != std::string::npos; };
    AreaDeCarta a;
    const int alcance = std::max(4, casillasDeAlcance(def.range, def.target));
    if (dice("línea") || dice("linea")) {
        a.plantilla = PlantillaArea{PlantillaArea::Forma::Linea, alcance};
        a.enTi = true;
    } else if (dice("cono")) {
        a.plantilla = PlantillaArea{PlantillaArea::Forma::Cono, alcance};
        a.enTi = true;
    } else {
        int radio = def.target == T::ALL_ENEMIES ? 2 : 1;
        if (const std::size_t i = texto.find("radio de "); i != std::string::npos) {
            const int pies = std::atoi(texto.c_str() + i + 9);
            if (pies > 0) radio = std::max(1, (pies + 2) / 5);  // 5 pies, una casilla
        }
        a.plantilla = PlantillaArea{PlantillaArea::Forma::Circulo, radio};
        a.enTi = dice("a tu alrededor") || dice("alrededor de ti");
    }
    return a;
}

std::vector<GridCoord> casillasDelArea(const AreaDeCarta& area, GridCoord lanzador, GridCoord objetivo,
                                       const std::function<bool(int x, int y)>& muro) {
    return casillas(area.plantilla, area.enTi ? lanzador : objetivo, objetivo, muro);
}

}  // namespace Game

