// MASC-01 — Mascotes amb trets magics: animals de companyia que passegen pel
// voltant de casa seva, descansen, dormen, i saluden el jugador (les
// afectuoses) o se n'aparten (les esquives).
//
// Logica sense OpenGL. Cada mascota te una casa (la cel·la on la posa el
// nivel, objecte mascota_<especie>) i camina per cel·les transitables, sempre
// en eix (+x, +y, -x, -y: les quatre diagonals de la pantalla), sense trepitjar
// el jugador ni una altra mascota. Tot es determinista des de la llavor: el
// mateix nivell, les mateixes passejades. Qui les dibuixa (i la seva magia:
// llum a terra, espurnes, anells, brases) es Render/MascotesNivel.
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"

namespace Game {

// Com es cada especie (de la fulla, assets/textures/pixellab_mascotas.json).
struct EspecieMascota {
    std::string id;           // "gat_alquimia"
    float velocitat = 1.4f;   // cel·les per segon caminant
    int radi = 4;             // fins on passeja des de casa
    bool afectuosa = true;    // ve a saludar el jugador; false: se n'aparta
    bool dorm = true;         // te animacio de dormir (si no, descansa dreta)
};

enum class EstatMascota { Repos, Camina, Saluda, Dorm, Fuig };

// Direccio: 0 = SE (+x), 1 = SO (+y), 2 = NO (-x), 3 = NE (-y).
struct Mascota {
    int especie = 0;
    GridCoord casa;
    float x = 0.5f, y = 0.5f;      // posicio continua en cel·les (centre = .5)
    int direccio = 0;
    int variant = 0;               // en repos: 0 quieta, 1 l'altra (pasturar, llepar-se...)
    EstatMascota estat = EstatMascota::Repos;
    float rellotge = 0.0f;         // segons en l'estat
    float espera = 3.0f;           // quant dura el repos o la salutacio
    float recorregut = 0.0f;       // cel·les caminades (cicle de caminar)
    float calma = 0.0f;            // segons que no fa cas del jugador
    std::vector<GridCoord> cami;   // cel·les que li falten, la primera es la seguent
};

class Mascotes {
public:
    using Transitable = std::function<bool(int, int)>;
    static constexpr float kDistanciaInteres = 3.5f;  // cel·les: el jugador li crida l'atencio
    static constexpr float kDistanciaFuga = 2.5f;     // cel·les: l'esquiva se n'aparta
    static constexpr float kSalutacio = 5.0f;         // segons asseguda davant del jugador
    static constexpr float kCalmaDespres = 18.0f;     // segons sense tornar-hi

    // Una mascota per (especie, casa). Una especie fora de rang s'ignora; una
    // casa no transitable tambe (la mascota no tindria on posar els peus).
    void prepara(const std::vector<std::pair<int, GridCoord>>& llista, const std::vector<EspecieMascota>& especies,
                 Transitable transitable, std::uint32_t llavor);
    void avanza(float dt, const GridCoord& jugador);
    const std::vector<Mascota>& mascotes() const { return m_mascotes; }
    const std::vector<EspecieMascota>& especies() const { return m_especies; }
    bool buida() const { return m_mascotes.empty(); }

    // La cel·la on es (la que la sosté per a l'ordre de dibuix).
    static GridCoord cella(const Mascota& m);
    // Direccio d'un pas (dx, dy) o cap a un punt.
    static int direccioDe(int dx, int dy);
    static int direccioCap(float dx, float dy);

private:
    std::uint32_t seguent();
    float atzar() { return static_cast<float>(seguent() % 10000u) / 10000.0f; }
    bool lliure(int x, int y, std::size_t qui, const GridCoord& jugador) const;
    // Cel·les abastables des d'on es (cerca en amplada, fins a «passos»),
    // amb el pare de cadascuna per refer el cami.
    std::vector<std::pair<GridCoord, int>> abast(std::size_t qui, const GridCoord& jugador, int passos) const;
    static std::vector<GridCoord> camiFins(const std::vector<std::pair<GridCoord, int>>& nodes, int desti);
    void reposa(Mascota& m, float durada);
    void passeja(std::size_t qui, const GridCoord& jugador);
    void saluda(std::size_t qui, const GridCoord& jugador);
    void fuig(std::size_t qui, const GridCoord& jugador);
    void camina(std::size_t qui, float dt, const GridCoord& jugador);

    std::vector<EspecieMascota> m_especies;
    std::vector<Mascota> m_mascotes;
    Transitable m_transitable;
    std::uint32_t m_estat = 1;
};

}  // namespace Game
