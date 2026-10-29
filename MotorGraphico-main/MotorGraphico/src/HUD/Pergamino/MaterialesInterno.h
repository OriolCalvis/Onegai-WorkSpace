// PERGAMINO-01 — Lo que comparten los dos .cpp de los materiales: el azar
// determinista, las rejillas logicas y la paleta. No es interfaz publica.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "HUD/Pergamino/Materiales.h"

namespace Pergamino {
namespace Interno {

// SplitMix64: pequeno, rapido y el mismo en cualquier maquina. Lo que
// importa no es la calidad estadistica sino que la semilla 40 de hoy sea
// la semilla 40 de manana.
class Azar {
public:
    explicit Azar(std::uint64_t semilla) : m_estado(semilla * 0x9E3779B97F4A7C15ULL + 0x1234567ULL) {}
    std::uint64_t siguiente() {
        std::uint64_t z = (m_estado += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    double real() { return static_cast<double>(siguiente() >> 11) * (1.0 / 9007199254740992.0); }
    int entero(int desde, int hasta) {  // [desde, hasta)
        if (hasta <= desde) return desde;
        return desde + static_cast<int>(siguiente() % static_cast<std::uint64_t>(hasta - desde));
    }

private:
    std::uint64_t m_estado;
};

template <typename T>
struct Rejilla {
    int ancho = 0, alto = 0;
    std::vector<T> v;
    Rejilla(int w, int h, T inicial) : ancho(w), alto(h), v(static_cast<std::size_t>(w * h), inicial) {}
    T& at(int x, int y) { return v[static_cast<std::size_t>(y * ancho + x)]; }
    const T& at(int x, int y) const { return v[static_cast<std::size_t>(y * ancho + x)]; }
};

using Color3 = std::array<std::uint8_t, 3>;

// Paleta de papel: 0 luz, 1 claro, 2 base, 3 sombra, 4 mancha, 5 canto.
extern const Color3 kPapel[6];
extern const Color3 kLineaTinta;   // contorno de tinta (74, 52, 33)
extern const Color3 kTintaAdorno;  // filetes (107, 80, 56)
extern const Color3 kMadera[10];
extern const Color3 kLaton[10];
extern const Color3 kCuero[4];

// Umbral de Bayer 4x4 en [0, 1).
float bayer(int x, int y);

// Ruido de valor con interpolacion suave en una rejilla de "celda" pixeles.
Rejilla<float> ruido(int w, int h, int celda, Azar& azar);
// Mascara de canto rasgado (lados: bits 1 arriba, 2 abajo, 4 izquierda, 8
// derecha) con las esquinas mordidas.
Rejilla<std::uint8_t> mascara(int w, int h, int rasgado, Azar& azar, int lados = 15);
// Distancia (4 vecinos, hasta 14) al exterior de la mascara.
Rejilla<int> distanciaBorde(const Rejilla<std::uint8_t>& m);
// Tono de papel antes de cuantizar.
Rejilla<float> papel(int w, int h, Azar& azar, float tono, int manchas, bool fibras,
                     float oscuro);
// Canto envejecido + tramado a la paleta + contorno de tinta. Devuelve la
// imagen logica (sin ampliar).
Imagen cuantiza(Rejilla<float>& t, const Rejilla<std::uint8_t>& m, const Rejilla<int>& d,
                int canto);
// Marco interior de tinta desvaida con rombos en las esquinas.
void filete(Imagen& img, int inset);
// Pixel logico -> 2x2 de pantalla.
Imagen amplia(const Imagen& logica, int factor = 2);
void pon(Imagen& img, int x, int y, const Color3& c, std::uint8_t alfa = 255);

// Fabricas de cada material (MaterialesPapel.cpp y MaterialesPiezas.cpp).
Imagen fabricaHoja(const PedidoMaterial& p);
Imagen fabricaPapiro(const PedidoMaterial& p);
Imagen fabricaPestana(const PedidoMaterial& p);
Imagen fabricaCuero(const PedidoMaterial& p);
Imagen fabricaAdorno(const PedidoMaterial& p);
Imagen fabricaPieza(const PedidoMaterial& p);
Imagen fabricaLomo(const PedidoMaterial& p);

}  // namespace Interno
}  // namespace Pergamino
