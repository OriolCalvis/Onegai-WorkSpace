// MASC-01 — La fulla de les mascotes com a dades: quines especies hi ha, com
// es mouen, on comencen les seves animacions per direccio i quina magia
// porten. Ho llegeix del manifest de la fulla (assets/textures/
// pixellab_mascotas.json, que escriu tools/pixellab/importar_mascotas.py) i
// tria el frame de cada mascota segons el seu estat. Sense OpenGL: qui puja
// la textura es Render/BancoAtlas i qui dibuixa, Render/MascotesNivel.
#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Game/Mascotes.h"

namespace Game {

// Una animacio: per a cada direccio (SE, SO, NO, NE), el primer frame
// (1-based a la fulla) i quants; 0 frames = aquesta direccio no hi es.
struct AnimMascota {
    std::array<std::pair<int, int>, 4> dirs{};
    float fps = 8.0f;
    bool aguanta = false;  // es queda a l'ultim frame (asseure's), no fa bucle
    bool te(int direccio) const { return direccio >= 0 && direccio < 4 && dirs[static_cast<std::size_t>(direccio)].second > 0; }
};

// La magia de l'especie, que dibuixa Render/MascotesNivel amb els frames d'efecte.
struct MagiaMascota {
    std::string color;       // cristall, llum, llauto, foc, vent, boira, espora
    std::string particula;   // espurna, hexagon, octagon, mota, brasa, espora, boira
    int quantes = 0;         // particules alhora
    float alcada = 10.0f;    // px: d'on surten
    bool anell = false;      // anell de vent al voltant
    bool rastre = false;     // les particules es queden on passa (boira)
};

struct FullaEspecie {
    EspecieMascota especie;
    float ombra = 24.0f;  // px: amplada de la sombra sota les potes
    std::map<std::string, AnimMascota> anims;  // camina, repos, repos2, saluda, dorm, fuig
    MagiaMascota magia;
};

class FullaMascotes {
public:
    // Llegeix el manifest. Sense manifest no es un error: no hi ha mascotes.
    Result<bool> carga(const std::string& rutaManifest);
    bool carregada() const { return !m_especies.empty(); }
    const std::vector<FullaEspecie>& especies() const { return m_especies; }
    std::vector<EspecieMascota> nomesEspecies() const;
    int indexDe(const std::string& id) const;  // -1 si no hi es
    // Primer frame d'un efecte (els que tenen animacio en fan 4 seguits); 0 si no hi es.
    int efecte(const std::string& nom) const;
    int peuX() const { return m_peuX; }
    int peuY() const { return m_peuY; }

    // Frame (1-based) que toca a una mascota segons el seu estat; 0 si
    // l'especie no te cap animacio per a aquesta direccio.
    int frameDe(const Mascota& m) const;
    // Nom de l'animacio que fa servir cada estat (amb les seves alternatives).
    static std::string animDe(const Mascota& m, const FullaEspecie& e);

private:
    std::vector<FullaEspecie> m_especies;
    std::map<std::string, int> m_efectes;
    int m_peuX = 32, m_peuY = 60;  // on cau el peu a la cel·la
};

}  // namespace Game
