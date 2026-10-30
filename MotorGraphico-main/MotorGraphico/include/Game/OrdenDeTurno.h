#pragma once
// AV13-11 — El orden de turno por iniciativa (D-COMB-03, COMBATE_BG3.md §3):
// cada uno tira 1d6 + su bono (la DES) al empezar el combate y juega en ese
// orden, enemigos incluidos, uno a uno. Se acaba lo de «todos los enemigos de
// golpe». GL-free: no sabe de la batalla, solo de puestos (heroe o enemigo i).
//
// AV13-29 (Oriol, 14/10, D0015 §1): los acompanantes tienen su puesto (aliado i > 0)
// y los juega el jugador. Las invocaciones siguen sin puesto: van con el heroe.

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace RPG { class RandomEngine; }

namespace Game {

struct Puesto {
    bool enemigo = false;
    std::size_t i = 0;  // el indice en su bando (el heroe es el aliado 0)
    int dado = 0;       // lo que salio en el d6
    int bono = 0;       // la DES
    int tirada() const { return dado + bono; }
};

class OrdenDeTurno {
public:
    // Tira para el heroe y para cada enemigo (bonos en el orden de la batalla)
    // y los ordena: mayor tirada primero; empate, mas bono; luego el heroe; luego
    // el orden de la batalla. Empieza por el primero.
    void tira(int bonoHeroe, const std::vector<int>& bonosEnemigos, RPG::RandomEngine& rng);
    // Con los acompanantes: un bono por aliado (el 0 es el heroe). Empate: el heroe, luego los
    // aliados, luego los enemigos, y dentro de cada bando el orden de la batalla.
    void tira(const std::vector<int>& bonosAliados, const std::vector<int>& bonosEnemigos, RPG::RandomEngine& rng);
    // Alguien que llega a mitad (un refuerzo): juega el ultimo de la ronda.
    void anade(Puesto p) { m_puestos.push_back(p); }
    bool vacio() const { return m_puestos.empty(); }
    const std::vector<Puesto>& puestos() const { return m_puestos; }
    std::size_t actual() const { return m_actual; }
    const Puesto* quien() const { return m_puestos.empty() ? nullptr : &m_puestos[m_actual]; }

    using Vivo = std::function<bool(const Puesto&)>;
    // Al siguiente que siga en pie. true si para llegar a el se ha dado la vuelta
    // (empieza una ronda nueva). Si no queda nadie en pie, se queda donde esta.
    bool avanza(const Vivo& vivo);
    // Si al que le toca ya no esta en pie, al siguiente (misma regla que avanza).
    bool saltaCaidos(const Vivo& vivo);

    // «Sulivan 7, Perdido saqueador 4.» (los nombres, en el orden de la batalla).
    std::string resumen(const std::string& heroe, const std::vector<std::string>& enemigos) const;
    std::string resumen(const std::vector<std::string>& aliados, const std::vector<std::string>& enemigos) const;

private:
    std::vector<Puesto> m_puestos;
    std::size_t m_actual = 0;
};

}  // namespace Game
