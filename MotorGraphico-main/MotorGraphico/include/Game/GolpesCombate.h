#pragma once
// AV14-01 — Un solo evento de golpe (COMBATE_BG3.md §9).
//
// La resolucion del combate (BattleState) apunta cada golpe que hace algo, o
// que lo intenta y no entra: quien, a quien, cuanto y con que grado. De el
// cuelgan el numero sobre la cabeza, el marco de la hoja que se tine, el
// parpadeo, el retroceso, la pausa del critico, las particulas y el sonido.
// Antes la pantalla los deducia comparando vidas entre fotogramas (COMBATE-01)
// y no sabia si un -1 era un roce o un critico contra una armadura.
//
// Tambien aqui el sitio de la batalla (CampoDeBatalla): el tablero, el orden de
// turno y los golpes van juntos para que BattleState no guarde mas de lo que
// dirige (la guardia de linea_base_core: 15 miembros por clase).

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Game/OrdenDeTurno.h"
#include "Game/TableroCombate.h"

namespace Game {

struct GolpeCombate {
    enum class Grado { Fallo, Pifia, Roce, Acierto, Critico, Cura, Periodico };
    int numero = 0;         // sube con cada golpe (0 = ninguno)
    bool aAliado = false;   // lo recibe tu bando
    std::size_t i = 0;      // su indice en ese bando
    std::string de, a;      // nombres ("" de: nadie, como el veneno)
    std::string que;        // la carta o el arma («Atacar», «Lluvia de balas»)
    int cambio = 0;         // vida: negativo, dano; positivo, cura; 0, no entro
    Grado grado = Grado::Acierto;
};

const char* nombreDe(GolpeCombate::Grado g);  // «Fallo», «Roce», «Critico»...

struct CampoDeBatalla {
    static constexpr std::size_t kGolpes = 64;  // los viejos ya se han visto
    std::optional<TableroCombate> tablero;   // AV13-06
    OrdenDeTurno orden;                      // AV13-11
    std::vector<GolpeCombate> golpes;        // AV14-01, el mas nuevo al final
    // AV13-34 — El golpe mortal guionizado (la compania de Rosita): cuando el enemigo `enemigo` esta a
    // `umbral` de su vida o menos, su siguiente accion te deja a las puertas (1 PV) y `dado` se enciende.
    struct GolpeMortal {
        bool activo = false;
        std::size_t enemigo = 0;
        float umbral = 0.0f;
        bool dado = false;
    } golpeMortal;
    void apunta(GolpeCombate g) {
        g.numero = golpes.empty() ? 1 : golpes.back().numero + 1;
        if (golpes.size() >= kGolpes) golpes.erase(golpes.begin());
        golpes.push_back(std::move(g));
    }
};

}  // namespace Game
