#pragma once
// AV14-02 — Lo que hace cada golpe en el cuerpo y en tu hoja (COMBATE_BG3.md §9).
//
// Oriol (14/10): «no son suficientes, hay que integrarlo todo artisticamente:
// el recuadro rojo podria ser como el marco de la hoja y no un cuadrado alli
// forzado». De cada GolpeCombate (AV14-01) sale:
//
//   - el cuerpo que lo recibe parpadea en carmin y retrocede, alejandose de
//     quien pega (el roce, poco; el critico, mas, y el mundo se para 70 ms);
//   - el que esquiva (fallo, pifia) se aparta de lado, sin parpadear;
//   - la cura, un pulso verde;
//   - si te pegan a ti, tu hoja (el retal de arriba a la izquierda) se tine
//     por el borde y tiembla: ni bandas rojas superpuestas, ni cuadrados.
//
// GL-free: guarda los efectos en curso y dice, cada fotograma, el tinte y el
// desplazamiento de cada cuerpo. Quien tiene los dibujos (ApplicationCinematica)
// se los pone. Se prueba en tests/Game/EfectosDeGolpeTests.cpp.

#include <cstddef>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Game/GolpesCombate.h"

namespace Game {

class EfectosDeGolpe {
public:
    static constexpr float kPausaCritico = 0.07f;  // el mundo, parado
    static constexpr float kHoja = 0.9f;           // lo que tarda la hoja en volver

    // Un golpe nuevo. `direccion`: de quien pega hacia quien recibe, en pantalla
    // (se normaliza; {0,0}: hacia atras de la camara, abajo). `fuerzaHoja`: si es
    // a ti, cuanto se nota en tu hoja (0..1; lo que quita respecto a tu vida).
    void golpe(const GolpeCombate& g, Vector2 direccion, float fuerzaHoja = 0.0f);
    void avanza(float segundos);
    void olvida();

    // Para cada cuerpo (bando e indice), este fotograma.
    bool activo(bool aliado, std::size_t i) const;
    Vector4 tinte(bool aliado, std::size_t i) const;       // multiplica (blanco: nada)
    Vector2 desplazamiento(bool aliado, std::size_t i) const;  // en pixeles del mundo
    float pausa() const { return m_pausa; }               // > 0: el mundo, parado
    // Tu hoja: 0 nada; 1 recien pegado (se tine el borde y tiembla).
    float hoja() const { return m_hoja; }
    // Su temblor este fotograma, en pixeles de pantalla.
    Vector2 temblorHoja() const;
    // Consume la pausa del critico con el tiempo de verdad; true mientras dura.
    bool pausaMundo(float segundos);

private:
    struct Efecto {
        bool aliado = false;
        std::size_t i = 0;
        GolpeCombate::Grado grado = GolpeCombate::Grado::Acierto;
        Vector2 direccion;
        float edad = 0.0f;
        float vida = 0.3f;
    };
    const Efecto* de(bool aliado, std::size_t i) const;
    std::vector<Efecto> m_efectos;
    float m_pausa = 0.0f;
    float m_hoja = 0.0f;
    float m_reloj = 0.0f;
};

}  // namespace Game
