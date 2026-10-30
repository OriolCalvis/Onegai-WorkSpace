// BLOQUES-02 — Pajaros de ciudad: una bandada que vuela entre azoteas.
//
// Logica sin OpenGL. Cada pajaro esta posado en una azotea (una celda de
// bloque, a 32 * plantas px del suelo) o volando hacia otra, a kAlturaVuelo.
// Despega cuando el jugador se le acerca o cuando se le acaba la paciencia,
// elige otra azotea lejos y planea hasta ella. Todo determinista desde la
// semilla (la misma ciudad, los mismos pajaros al cargar), y el azar solo
// decide destinos y esperas. Quien dibuja es Render/BandadaNivel.
#pragma once

#include <cstdint>
#include <vector>

#include "Core/Math/GridCoord.h"

namespace Game {

struct Azotea {
    GridCoord celda;
    int plantas = 1;
    // MASC-01 — los tejados del kit: el punto exacto del caballete dentro de
    // la celda, su altura en px (si >= 0 manda sobre las plantas) y la celda
    // que ordena el dibujo (la esquina delantera de la casa: el pajaro va
    // despues de todo el tejado). Sin ellos, el centro de la azotea de bloque.
    float dx = 0.5f, dy = 0.5f;
    float altura = -1.0f;
    GridCoord orden{-1, -1};
};

struct Pajaro {
    float x = 0.0f, y = 0.0f;   // posicion continua en celdas (centro de celda = .5)
    float altura = 0.0f;        // px sobre el suelo
    float reloj = 0.0f;         // segundos en el estado actual
    float espera = 5.0f;        // cuanto se queda posado
    float x0 = 0.0f, y0 = 0.0f, altura0 = 0.0f;  // de donde salio (para el vuelo)
    int destino = 0;            // indice de azotea
    int especie = 0;
    bool volando = false;
    bool mirandoDerecha = true;
};

class Bandada {
public:
    static constexpr float kAlturaVuelo = 128.0f;   // por encima de cualquier bloque de 3 plantas
    static constexpr float kVelocidad = 4.0f;       // celdas por segundo
    static constexpr float kDistanciaSusto = 2.5f;  // celdas: el jugador los espanta

    // Reparte «cuantos» pajaros por las azoteas; sin azoteas no hay bandada.
    void prepara(const std::vector<Azotea>& azoteas, std::uint32_t semilla, int cuantos, int especies = 1);
    void avanza(float dt, const GridCoord& jugador);
    const std::vector<Pajaro>& pajaros() const { return m_pajaros; }
    const std::vector<Azotea>& azoteas() const { return m_azoteas; }
    bool vacia() const { return m_pajaros.empty(); }
    // Frame de la hoja: 0 posado, 1 alas arriba, 2 alas abajo.
    static int frameDe(const Pajaro& p);

private:
    std::uint32_t siguiente();
    float azar() { return static_cast<float>(siguiente() % 10000u) / 10000.0f; }
    void despega(Pajaro& p);
    static float alturaAzotea(const Azotea& a) {
        return a.altura >= 0.0f ? a.altura : 32.0f * static_cast<float>(a.plantas) + 6.0f;
    }

    std::vector<Azotea> m_azoteas;
    std::vector<Pajaro> m_pajaros;
    std::uint32_t m_estado = 1;
};

}  // namespace Game
