// CLASES-03 — Companeros: los NPC de npcs.json con bloque `companion`
// (recruitable) que el jugador puede sumar a su equipo. El reclutamiento es
// una flag del NarrativeState (`joinFlag`, por defecto companero_<npcId>):
// la enciende un beat de aventura (setFlag) o reclutar(); asi sobrevive al
// cambio de nivel y al guardado sin inventar un almacen nuevo. Sin GL.
//
// Lo que NO hace todavia: meter al companero en el combate (BattleState solo
// lleva al jugador) ni darle ficha propia; usesClassCards dice que, cuando
// se haga, jugara con las cartas de su clase.
#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace RPG { class NarrativeState; }

namespace Game {

struct Companero {
    std::string npcId, nombre, claseId, razaId, descripcion;
    std::string joinFlag;
    bool usaCartasDeClase = true;
};

class Companeros {
public:
    // Lee <ruta> (npcs.json) y se queda solo con los que tienen
    // companion.recruitable = true.
    Result<int> cargar(const std::string& rutaNpcs);
    const std::vector<Companero>& reclutables() const { return m_todos; }
    const Companero* buscar(const std::string& npcId) const;

    bool reclutado(const Companero& c, const RPG::NarrativeState& estado) const;
    std::vector<const Companero*> reclutados(const RPG::NarrativeState& estado) const;
    // Enciende la flag; false si el id no es reclutable.
    bool reclutar(const std::string& npcId, RPG::NarrativeState& estado) const;
    void despedir(const std::string& npcId, RPG::NarrativeState& estado) const;

private:
    std::vector<Companero> m_todos;
};

}  // namespace Game
