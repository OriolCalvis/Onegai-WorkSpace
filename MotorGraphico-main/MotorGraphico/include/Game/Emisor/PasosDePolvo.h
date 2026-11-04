// U4-07.E07 — El polvo de pasos del jugador (D-O1 = B). GL-free: la parte logica del efecto.
//
// Al pasar el jugador a una celda ADYACENTE (Manhattan 1, la condicion de Player::moveTo) se levanta una nube
// de la familia `polvo` en los pies de la celda que deja. Un salto no adyacente (carga, teletransporte) o la
// celda centinela de las cinematicas olvidan todo y no levantan polvo; la primera celda tras monta/olvida solo
// se memoriza. Posee los emisores (orden de creacion), UN presupuesto de 256 y el contador de pasos. Nada
// `static`. La pintura es Render::PolvoDePasos (GL).
//
// D-O7 (Oriol): sin polvo en combate (ver avanza).
// Ticks: `avanza` se llama una vez por tick fijo de 1/60 s. Con dtPolvo == 0 (pausa, D-O6) NADA nace ni
// envejece (la celda centinela si se atiende: olvida aunque haya pausa).
#pragma once

#include <cstdint>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Game/Emisor/DefinicionEmisor.h"
#include "Game/Emisor/Emisor.h"
#include "Game/Emisor/ProyeccionParticulas.h"

class Alturas;

namespace Game::Emisor {

constexpr int kMaxEmisoresPolvo = 16;      // nubes vivas a la vez; la 17.a no se crea (no se evicta ninguna)
constexpr int kAforoPolvo = 16;            // particulas por nube
constexpr int kPresupuestoPolvo = 256;     // vivas entre todas las nubes
constexpr double kEscalaTasaReducida = 0.25;   // D-O3: movimiento reducido = tasa al 25 %
constexpr GridCoord kCeldaCentinela{-1000, -1000};   // «sin celda»: cinematicas

// La definicion del emisor de una nube (construida en codigo, no pasa por leerEmisor). `semilla` entera de 64 bits.
DefinicionEmisor definicionDePolvo(std::uint64_t semilla);
// alfaMax 0,7 y entrada 0,02 s (D-O5: ajustable al verlo).
EstiloEmisor estiloDePolvo();

class PasosDePolvo {
public:
    PasosDePolvo();

    // Un nivel nuevo: olvida todo, guarda llavor, tamano de baldosa y el puntero (no propietario, puede ser
    // nullptr = sin desfase) a las alturas del mapa, que debe vivir mientras este montado.
    void monta(std::uint32_t llavor, int tileW, int tileH, const Alturas* alturas);
    void olvida();      // sin nubes, sin celda memorizada y presupuesto a 0; conserva el montaje
    void desmonta();    // olvida y suelta el puntero a las alturas
    // D-O7: con enCombate no nace polvo ni se detectan pasos (el heroe se mueve varias casillas por tick); las
    // nubes que ya existian se agotan solas. Al salir, la primera celda solo se memoriza.
    void avanza(double dtPolvo, const GridCoord& celda, bool reducido, bool enCombate = false);

    const std::vector<Emisor>& emisores() const { return m_emisores; }
    const PresupuestoParticulas& presupuesto() const { return m_presupuesto; }
    std::uint64_t pasos() const { return m_pasos; }

    // El adaptador GL pone aqui si tiene textura blanca (es el unico estado de blanco: Render::PolvoDePasos solo
    // guarda ademas el puntero Texture*, que este modulo gl-free no puede nombrar); sin ella listo() es falso.
    void ponBlanco(bool disponible) { m_blanco = disponible; }
    bool listo() const { return m_blanco; }

private:
    void creaNube(const GridCoord& anterior);

    std::vector<Emisor> m_emisores;
    PresupuestoParticulas m_presupuesto;
    GridCoord m_ultima{0, 0};
    bool m_hayUltima = false;
    std::uint64_t m_pasos = 0;      // pasos adyacentes vistos desde monta: alimenta la semilla
    std::uint32_t m_llavor = 0;
    int m_tileW = 64;
    int m_tileH = 32;
    const Alturas* m_alturas = nullptr;
    bool m_blanco = false;
};

}  // namespace Game::Emisor
