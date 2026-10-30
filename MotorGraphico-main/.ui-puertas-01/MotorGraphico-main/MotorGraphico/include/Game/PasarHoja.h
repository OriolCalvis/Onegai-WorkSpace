// PERGAMINO-01 — La vuelta de hoja del cuaderno, sin OpenGL.
//
// Especificacion aprobada (MODELO_VISUAL.md §13.3 y prototipo_pasar_hoja.
// html): la bisagra esta en el borde de abajo; hacia delante la hoja actual
// cae hacia el jugador (0 -> 180 grados) acelerando como con gravedad
// (0,25·t + 0,75·t²); hacia atras la hoja anterior sube y tapa la actual,
// frenando con un rebote de unos 5 grados. 360 ms. Una pulsacion durante la
// vuelta la completa y encadena la siguiente: no se pierden pulsaciones ni
// se bloquea la entrada. Con animaciones reducidas, fundido de 160 ms.
//
// Aqui vive el estado (de donde a donde, cuanto va) y la geometria de la
// hoja en perspectiva, cortada en tiras. Es aritmetica pura: el dibujo lo
// hace el cuaderno (HUD/Pergamino/Cuaderno) y la textura, Render/PergaminoGL.
#pragma once

#include <vector>

#include "Core/Math/Vector2.h"

namespace Game {

// Una tira de la hoja proyectada: esquinas en pantalla (arriba-izq,
// arriba-der, abajo-der, abajo-izq, en el orden de la TEXTURA) y el tramo
// vertical de la textura que le toca (v normalizada, 0 = arriba).
struct TiraHoja {
    Vector2 p[4];
    float v0 = 0.0f;  // v de las esquinas 0-1
    float v1 = 1.0f;  // v de las esquinas 2-3
};

class PasarHoja {
public:
    static constexpr float kDuracion = 0.36f;
    static constexpr float kFundido = 0.16f;

    // Empieza una vuelta de `desde` a `hacia`. Si ya habia una en curso, se
    // completa al instante y la nueva sale de donde aquella iba.
    void empieza(int desde, int hacia, bool reducido = false);
    void avanza(float segundos);
    // Termina la vuelta en curso (la deja en reposo en su destino).
    void completa() { m_activa = false; m_t = 1.0f; }

    bool enCurso() const { return m_activa; }
    bool haciaDelante() const { return m_hacia > m_desde; }
    bool reducido() const { return m_reducido; }
    int desde() const { return m_desde; }
    int hacia() const { return m_hacia; }
    float progreso() const { return m_t; }

    // Angulo de la hoja que se mueve: 0 plana, 180 caida bajo la bisagra.
    float angulo() const;
    static float anguloEn(bool haciaDelante, float t);
    // Brillo de las dos caras (se oscurecen al levantarse) y opacidad de la
    // sombra que la hoja proyecta sobre la de debajo.
    float brilloFrente() const;
    float brilloDorso() const;
    float sombraDebajo() const;
    bool muestraDorso() const { return angulo() > 90.0f; }
    // Opacidad de la hoja que se mueve en el modo de animaciones reducidas.
    float opacidadReducida() const { return haciaDelante() ? 1.0f - m_t : m_t; }

    // La hoja (x, y, ancho, alto; bisagra abajo) a `anguloGrados`, vista con
    // la perspectiva del prototipo (2800 px, origen al 50% / 20% de la hoja),
    // en `tiras` tramos horizontales. Por debajo de `recorteY` (el borde de
    // las tapas) no se dibuja: las tiras se recortan ahi.
    static std::vector<TiraHoja> tiras(float x, float y, float ancho, float alto,
                                       float anguloGrados, int numTiras, float recorteY);

private:
    int m_desde = 0;
    int m_hacia = 0;
    float m_t = 1.0f;
    bool m_activa = false;
    bool m_reducido = false;
};

}  // namespace Game
