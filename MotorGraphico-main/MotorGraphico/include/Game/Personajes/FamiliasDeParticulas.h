// PART-01 — Las familias de particulas del juego: su forma, como se mueven y sus colores.
//
// Las de los trece elementos y las de los temas de las clases (tools/particulas.py, la misma lista;
// planificacion/CANON_PARTICULAS.md). Las usan el brillo del equipo y los abalorios (BrilloEquipo) y las
// cartas al jugarse en el combate (PART-02d). Sin OpenGL: formas en pixeles del sprite y cifras.
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Game {

// PART-01: las formas de las particulas de los elementos y de los temas de las clases (las llamas, los
// copos, las notas, los naipes, los dragones...). La lista es la de tools/particulas.py.
enum class FormaDeParticula {
    Punto, Cruz, Burbuja, Voluta, Hoja, Rayo, Gota,
    Llama, Copo, Piedra, Estela, EstelaVertical, Runa, Nota, Naipe, Moneda, Engranaje, Pluma, Murcielago,
    Dragon, Petalo,
    // Oriol, revisando la lista: el veneno, calaveras verdes (las burbujas parecian curar); las estrellas y el
    // oro, la estrella de cuatro puntas de siempre; las chispas, trazos (uno por hacia donde vuelan); el
    // engranaje y la moneda giran (su otra cara).
    Calavera, Estrella, TrazoNE, TrazoNO, TrazoSE, TrazoSO, EngranajeGirado, MonedaDeCanto
};
// PART-01: como se mueve: sube o cae meciendose, deriva de lado, cae hacia el centro de la pieza
// (converge), gira alrededor de ella (orbita), se abre como un aro (expande), salta un instante (chispazo)
// o salpica: sale disparada hacia arriba y a un lado y cae (las chispas de la fragua).
enum class MovimientoDeParticula { Sube, Cae, Deriva, Converge, Orbita, Expande, Chispazo, Salpica };
struct TipoDeParticula {
    float vy = 0.0f, vx = 0.0f;  // pixeles por segundo (vy < 0 sube); vx, cuanto se mece o deriva
    float vida = 1.0f;           // segundos
    FormaDeParticula forma = FormaDeParticula::Punto;
    bool suma = true;            // luz que se suma; false: tapa (sombra, hojas)
    MovimientoDeParticula movimiento = MovimientoDeParticula::Sube;
    std::vector<std::array<std::uint8_t, 3>> colores;  // los suyos (el fuego, el confeti); vacio = el de la pieza
    float densidad = 1.0f;       // cuantas, respecto a lo normal (el oro, mas intenso)
    FormaDeParticula otraForma = FormaDeParticula::Punto;  // la de algunas (las cruces doradas entre las monedas)
    float conOtraForma = 0.0f;   // que parte lleva la otra forma (0: ninguna)
    bool gira = false;           // gira sobre si misma: alterna con su otra cara (el engranaje, la moneda)
};
// Las familias de tools/particulas.py: las de los trece elementos ("llamas", "agua", "escarcha", "rayos",
// "piedras", "viento", "luz", "sombra", "hojas", "veneno", "sangre", "arcano", "vacio") y las de los temas
// de las clases ("dragones", "petalos", "notas", "naipes", "monedas", "animas"...). Otro nombre = chispas.
TipoDeParticula tipoDeParticula(const std::string& nombre);
// Los nombres de todas las familias (para comprobar que son las de tools/particulas.py).
std::vector<std::string> familiasDeParticulas();

// Los pixeles de una forma: (dx, dy, peso).
struct PixelDeForma {
    int dx, dy;
    float peso;
};
const std::vector<PixelDeForma>& pixelesDeForma(FormaDeParticula forma);
// PART-01: el contorno de las formas de tema (llama, copo, runa, nota, naipe, moneda, engranaje, pluma,
// murcielago, dragon y petalo): los pixeles de alrededor, que se pintan antes, oscuros y con la mezcla
// normal, para que se lean encima del cuerpo como el resto del dibujo. Las demas, sin contorno (vacio).
const std::vector<PixelDeForma>& contornoDeForma(FormaDeParticula forma);

}  // namespace Game
