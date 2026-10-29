// COMBATE-01 — Que se note cada golpe (Oriol, 14/10: «algún feedback visual de
// que me peguen»).
//
// Los números que saltan sobre quien recibe («-5» en rojo, «+3» en verde) y lo
// que no hace nada («Fallo», «Roce») en tinta clara, subiendo y apagándose en
// un segundo. AV14-02: el critico, mas grande y en oro viejo; las bandas rojas
// de los bordes se han ido (Oriol: «como el marco de la hoja y no un cuadrado
// alli forzado»): lo que se tine ahora es tu hoja (HudRetales). Quien sabe dónde está cada uno en la pantalla (Application) los lanza;
// esto los guarda, los mueve y los pinta en su lienzo. Sin GL: se prueba en
// tests/HUD/GolpesEnPantallaTests.cpp.
#pragma once

#include <string>
#include <vector>

#include "Core/Math/Vector2.h"
#include "HUD/Pergamino/Lienzo.h"

namespace Pergamino {

class Pincel;

class GolpesEnPantalla {
public:
    enum class Tipo { Dano, Cura, Nada, Critico };
    static constexpr float kVida = 1.1f;  // segundos que se ve un número

    // `mundo`: encima de la cabeza de quien lo recibe, en el mundo (el número se
    // queda pegado a él aunque la cámara se mueva).
    void lanza(const Vector2& mundo, const std::string& texto, Tipo tipo);
    // La cámara de este fotograma: su centro en el mundo y su zoom.
    void ponVista(const Vector2& camara, float zoom) {
        m_camara = camara;
        m_zoom = zoom;
    }
    void avanza(float segundos);
    void olvida();

    // Repinta su lienzo (vacío si no hay nada a la vista).
    void pinta(const Pincel& p, int anchoVista, int altoVista);
    const Lienzo& lienzo() const { return m_lienzo; }

    struct Golpe {
        Vector2 mundo;
        std::string texto;
        Tipo tipo = Tipo::Dano;
        float edad = 0.0f;
    };
    const std::vector<Golpe>& golpes() const { return m_golpes; }

private:
    std::vector<Golpe> m_golpes;
    Vector2 m_camara;
    float m_zoom = 1.0f;
    Lienzo m_lienzo;
};

}  // namespace Pergamino
