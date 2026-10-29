// PERGAMINO-01 — Las dos voces del tema: medir, partir y colocar texto.
//
// Lee assets/textures/letras_pergamino.json (tools/gen_letras_pergamino.py):
// la fuente pixel 5x7 del motor con los ajustes de MODELO_VISUAL.md §4.2
// (avance 6 y tildes en el interlineado) y Lora a los tamanos que usan las
// maquetas. Sin OpenGL: devuelve glifos colocados (rectangulo en pantalla +
// region del atlas) y el ejecutor GL los dibuja. Por eso la regla que mas
// fallaba en el HUD viejo --que un texto no pise a su vecino-- se prueba
// aqui midiendo, sin ventana.
//
// El texto entra en UTF-8. La voz pixel siempre va en mayusculas (el atlas
// no tiene minusculas): coloca() las convierte, tildes incluidas.
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Errors/Result.h"
#include "HUD/Pergamino/Lienzo.h"

namespace Pergamino {

enum class Letra : std::uint8_t {
    Pixel,
    Lora16, Lora18, Lora20, Lora22, Lora24,
    Lora18b, Lora20b, Lora22b, Lora24b, Lora28b,
    Lora16i, Lora18i,
    Lora13,  // UI-BG3-01: los nombres de las cartas de la barra de combate
    Total
};

struct Glifo {
    int x = 0, y = 0, w = 0, h = 0;   // region del atlas, en pixeles
    int apoyoX = 0, apoyoY = 0;       // desde la pluma y la linea base
    int avance64 = 0;                 // avance en 1/64 de pixel
};

struct GlifoColocado {
    Rect destino;  // pixeles de pantalla
    Rect origen;   // region del atlas, en pixeles
};

class FuenteAtlas {
public:
    Result<bool> carga(const std::string& rutaJson);
    Result<bool> cargaTexto(const std::string& json);
    bool cargada() const { return m_cargada; }
    int anchoAtlas() const { return m_ancho; }
    int altoAtlas() const { return m_alto; }

    // Ascenso y descenso en pixeles (la voz pixel: 9 filas por encima de la
    // linea base, las 2 de arriba para las tildes).
    int ascenso(Letra letra, int escala = 1) const;
    int descenso(Letra letra, int escala = 1) const;

    // Ancho en pixeles de una linea de texto.
    int ancho(Letra letra, const std::string& texto, int escala = 1) const;
    // Parte el texto en lineas de como mucho anchoMax pixeles, por los
    // espacios; una palabra que no cabe sola se corta. Respeta los '\n'.
    std::vector<std::string> envuelve(Letra letra, const std::string& texto, float anchoMax,
                                      int escala = 1) const;
    // Una sola linea que quepa en anchoMax, con puntos suspensivos si no.
    std::string recorta(Letra letra, const std::string& texto, float anchoMax,
                        int escala = 1) const;
    // Coloca una linea con la pluma en (x, lineaBase). Devuelve el avance.
    float coloca(Letra letra, const std::string& texto, float x, float lineaBase, int escala,
                 std::vector<GlifoColocado>& salida) const;

private:
    struct Fuente {
        bool pixel = false;
        int ascenso = 0, descenso = 0;
        std::unordered_map<std::uint32_t, Glifo> glifos;
        std::unordered_map<std::uint64_t, int> kerning;
    };
    const Fuente& fuente(Letra letra) const { return m_fuentes[static_cast<std::size_t>(letra)]; }

    std::array<Fuente, static_cast<std::size_t>(Letra::Total)> m_fuentes;
    int m_ancho = 0;
    int m_alto = 0;
    bool m_cargada = false;
};

// UTF-8 -> codepoints (los bytes mal formados salen como '?').
std::vector<std::uint32_t> decodificaUtf8(const std::string& texto);
// Mayusculas para la voz pixel: a-z y las minusculas de Latin-1.
std::uint32_t mayuscula(std::uint32_t c);

}  // namespace Pergamino
