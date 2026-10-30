// AV13-06 — El tablero del combate: las casillas del nivel donde se pelea.
//
// Oriol (14/10/2026, D0014): «el objetivo final es practicamente ser Baldur's
// Gate 3 en cuanto a combate», empezando por «moverse por casillas en combate,
// como en un tactics, con las areas visibles». Hasta hoy el combate no tenia
// sitio: BattleState es un turno de Dragon Quest, sin posiciones
// (planificacion_alcance_y_vision/COMBATE_BG3.md §2-3).
//
// El combate no cambia de pantalla: se juega en las casillas del nivel donde
// empieza, dentro de un recuadro de kLado x kLado (11) alrededor de los que pelean
// (recortado al nivel). Aqui se sabe que casillas cuentan, quien esta en cada
// una, cuanto distan y si se ven. La ruta y el area son de otras piezas
// (Game::RutaCasillas, Game::PlantillaArea): este tablero les da los
// callbacks de transitable y ocupada.
//
// Medidas (1 casilla = 1,5 m, como los 9 m de base de BG3 en 6 casillas):
//   - distancia(): en casillas, con la diagonal contando 1 (Chebyshev). Es la
//     del alcance: «al lado» incluye las cuatro diagonales.
//   - moverse va por los cuatro vecinos (como tryMovePlayer); lo que cuesta
//     andar lo da la ruta, no la distancia.
//   - seVen(): la linea entre los centros (Bresenham) no cruza una casilla
//     que el nivel bloquea. Las personas no tapan la vista.
//
// GL-free y sin el nivel dentro: el nivel se consulta con un callback.
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/RutaCasillas.h"

namespace Game {

class TableroCombate {
public:
    static constexpr int kLado = 11;  // casillas por lado del recuadro (16,5 m; cabe casi entero con el zoom del combate)
    static constexpr float kMetrosPorCasilla = 1.5f;
    static constexpr int kPasos = 4;          // AV16-12: casillas por ronda de base (6 m); eran 6 y Oriol las vio de mas
    static constexpr int kDesParaBono = 3;    // AV16-12: con DES 3 o mas, +1 casilla por ronda
    static constexpr int kBonoPorDes = 1;

    // true si el nivel no deja pisar esa casilla (pared, mueble, agua...).
    using Bloquea = std::function<bool(int x, int y)>;

    // AV19-02 — El nivel de altura de una casilla. Subir un nivel cuesta el
    // doble de esfuerzo (2 pasos en vez de 1); bajar o andar en llano, lo normal.
    using Nivel = std::function<int(int x, int y)>;
    void ponNiveles(Nivel nivel) { m_nivel = std::move(nivel); }
    // Lo que cuesta pasar de una casilla a su vecina: 1, o 2 si sube de nivel.
    int costeDePaso(GridCoord de, GridCoord a) const;

    TableroCombate() = default;
    // El recuadro de `lado` x `lado` centrado en el medio de `quienes`,
    // desplazado (no recortado) para que quepa en el nivel si el nivel es mas
    // grande; si el nivel es mas pequeno, el nivel entero.
    static TableroCombate alrededor(const std::vector<GridCoord>& quienes, int anchoNivel, int altoNivel,
                                    Bloquea bloquea, int lado = kLado);

    GridCoord origen() const { return m_origen; }
    int ancho() const { return m_ancho; }
    int alto() const { return m_alto; }

    bool dentro(GridCoord c) const;
    bool transitable(GridCoord c) const;  // dentro y el nivel la deja pisar
    bool libre(GridCoord c) const;        // transitable y sin nadie

    // Quien esta donde. Un id por combatiente (el nombre que usa la batalla).
    bool pon(const std::string& quien, GridCoord c);  // false si no es libre (y no se pone)
    void quita(const std::string& quien);
    bool mueve(const std::string& quien, GridCoord a);  // false si no esta o la casilla no es libre
    std::optional<GridCoord> dondeEsta(const std::string& quien) const;
    const std::string* quienEsta(GridCoord c) const;
    const std::vector<std::pair<std::string, GridCoord>>& ocupantes() const { return m_ocupantes; }

    static int distancia(GridCoord a, GridCoord b);  // Chebyshev, en casillas
    static bool alLado(GridCoord a, GridCoord b) { return distancia(a, b) == 1; }
    bool seVen(GridCoord a, GridCoord b) const;

    // La casilla libre mas cercana a `desde` (por anillos; a igual distancia,
    // la primera en orden de lectura, para que salga siempre la misma).
    std::optional<GridCoord> libreMasCercana(GridCoord desde) const;
    // Las casillas del borde del recuadro que se pueden pisar: por donde
    // entra un refuerzo o se sale huyendo.
    std::vector<GridCoord> borde() const;

    // AV13-12 — Moverse en el combate. Cada uno tiene kPasos casillas por
    // ronda (reponPasos() al cerrarla); andar gasta lo que cuesta la ruta.
    int pasos(const std::string& quien) const;
    // AV16-12 — Los pasos que tiene `quien` cada ronda: kPasos, mas su bono (DES 3+).
    int pasosMax(const std::string& quien) const;
    void ponBono(const std::string& quien, int bono);
    // AV16-14 — Estados que frenan: 0 = normal, 1 = la mitad de los pasos (congelado),
    // 2 = ninguno (aturdido, agarrado). Se pregunta cada vez, asi que un estado que
    // llega o se va en mitad del combate cuenta al momento.
    using Velocidad = std::function<int(const std::string& quien)>;
    void ponVelocidad(Velocidad v) { m_velocidad = std::move(v); }
    // El bono que da una destreza: kBonoPorDes si llega a kDesParaBono, si no 0.
    static int bonoDeDestreza(int destreza) { return destreza >= kDesParaBono ? kBonoPorDes : 0; }
    void reponPasos() { m_gastados.clear(); }
    // Las casillas a las que llega `quien` con los pasos que le quedan.
    std::vector<RutaCasillas::Alcanzable> alcanzables(const std::string& quien) const;
    // La ruta de `quien` hasta `destino` (vacia si no llega con lo que le queda).
    std::vector<GridCoord> rutaHasta(const std::string& quien, GridCoord destino) const;
    // Anda: se pone en `destino` y gasta la ruta. Devuelve la ruta andada
    // (vacia si no podia: nada cambia).
    std::vector<GridCoord> anda(const std::string& quien, GridCoord destino);
    // `quien` llega a `objetivo` con un ataque de este alcance: 1 = al lado
    // (diagonal incluida); mas de 1, a esa distancia y viendole.
    bool aTiro(const std::string& quien, const std::string& objetivo, int alcance) const;
    // El que no esta a tiro de ninguno de `objetivos` se acerca con sus pasos
    // a la casilla desde la que llega (si no, a la mas cercana). Devuelve si
    // ha quedado a tiro, y en `ruta` lo que ha andado.
    bool acerca(const std::string& quien, const std::vector<std::string>& objetivos, int alcance,
                std::vector<GridCoord>* ruta = nullptr);

private:
    GridCoord m_origen{};
    int m_ancho = 0, m_alto = 0;
    Bloquea m_bloquea;
    Nivel m_nivel;
    std::vector<std::pair<std::string, GridCoord>> m_ocupantes;
    std::vector<std::pair<std::string, int>> m_bonos;
    Velocidad m_velocidad;     // AV16-12: pasos extra por ronda
    std::vector<std::pair<std::string, int>> m_gastados;  // AV13-12: pasos gastados esta ronda

    RutaCasillas rutasDe(const std::string& quien) const;  // pisa lo libre (y su propia casilla)
    void gasta(const std::string& quien, int n);
};

}  // namespace Game
