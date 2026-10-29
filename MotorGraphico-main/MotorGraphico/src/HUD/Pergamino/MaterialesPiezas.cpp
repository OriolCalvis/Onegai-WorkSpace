// PERGAMINO-01 — Cuero, adornos y piezas sueltas (ver Materiales.h).
#include <algorithm>
#include <cmath>
#include <tuple>

#include "MaterialesInterno.h"

namespace Pergamino {

bool PedidoMaterial::operator==(const PedidoMaterial& o) const {
    return std::tie(tipo, ancho, alto, semilla, a, b, c, d) ==
           std::tie(o.tipo, o.ancho, o.alto, o.semilla, o.a, o.b, o.c, o.d);
}

bool PedidoMaterial::operator<(const PedidoMaterial& o) const {
    return std::tie(tipo, ancho, alto, semilla, a, b, c, d) <
           std::tie(o.tipo, o.ancho, o.alto, o.semilla, o.a, o.b, o.c, o.d);
}

PedidoMaterial hoja(int w, int h, int semilla, int rasgado, int filete, int manchas, int oscuro100) {
    return {Material::Hoja, w, h, semilla, rasgado, filete, manchas, oscuro100};
}
PedidoMaterial papiro(int w, int h, int semilla) { return {Material::Papiro, w, h, semilla}; }
PedidoMaterial cuero(int w, int h, int semilla, bool costura, bool remaches, bool esquinas) {
    return {Material::Cuero, w, h, semilla, costura ? 1 : 0, remaches ? 1 : 0, esquinas ? 1 : 0, 0};
}
PedidoMaterial pestana(int w, int h, bool activa, int semilla) {
    return {Material::Pestana, w, h, semilla, activa ? 1 : 0};
}
PedidoMaterial adorno(int w) { return {Material::Adorno, w, 10, 1}; }
PedidoMaterial lomo(int w, int h) { return {Material::Lomo, w, h, 1}; }

void medidaPieza(Material tipo, int& w, int& h) {
    switch (tipo) {
        case Material::Sello: w = h = 24; break;
        case Material::Chincheta: w = h = 12; break;
        case Material::Cinta: w = 14; h = 28; break;
        case Material::Tachuela: w = h = 12; break;
        case Material::Flecha: w = 12; h = 8; break;
        default: w = h = 0; break;
    }
}

PedidoMaterial pieza(Material tipo) {
    PedidoMaterial p{tipo, 0, 0, 1};
    medidaPieza(tipo, p.ancho, p.alto);
    return p;
}

namespace Interno {

const Color3 kMadera[10] = {{58, 36, 18},  {110, 68, 36}, {143, 93, 52}, {176, 122, 69},
                            {160, 108, 60}, {143, 93, 52}, {122, 78, 42}, {98, 61, 31},
                            {75, 46, 23},  {58, 36, 18}};
const Color3 kLaton[10] = {{74, 53, 20},   {156, 122, 46}, {201, 162, 74}, {230, 198, 106},
                           {214, 180, 90}, {201, 162, 74}, {176, 140, 60}, {156, 122, 46},
                           {122, 94, 34},  {74, 53, 20}};
const Color3 kCuero[4] = {{63, 40, 22}, {78, 50, 29}, {94, 61, 35}, {109, 72, 42}};

namespace {
void esquinasLaton(Imagen& img, int tam = 9) {
    for (int fy = 0; fy < 2; ++fy)
        for (int fx = 0; fx < 2; ++fx)
            for (int y = 0; y < tam; ++y)
                for (int x = 0; x < tam - y; ++x) {
                    Color3 col = kLaton[2];
                    if (x == 0 || y == 0) col = kLaton[3];
                    if (x + y == tam - 1) col = kLaton[0];
                    else if (x + y == tam - 2) col = kLaton[6];
                    if (x == 2 && y == 2) col = kLaton[0];  // clavo
                    if (x == 3 && y == 3) col = kLaton[4];
                    const int px = fx ? img.ancho - 1 - x : x;
                    const int py = fy ? img.alto - 1 - y : y;
                    pon(img, px, py, col);
                }
}
}  // namespace

Imagen fabricaCuero(const PedidoMaterial& p) {
    Azar azar(static_cast<std::uint64_t>(p.semilla));
    const int w = p.ancho / 2, h = p.alto / 2;
    if (w < 8 || h < 8) return {};
    const Rejilla<float> r = ruido(w, h, 9, azar);
    Imagen img;
    img.crea(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            const float t = 0.45f + 0.25f * (r.at(x, y) - 0.5f) +
                            0.20f * (static_cast<float>(azar.real()) - 0.5f);
            const int idx = std::clamp(static_cast<int>(std::floor(t * 3.0f + bayer(x, y) - 0.5f)), 0, 3);
            pon(img, x, y, kCuero[idx]);
        }
    const Color3 borde{42, 26, 14};
    for (int x = 0; x < w; ++x) {
        pon(img, x, 0, borde);
        pon(img, x, h - 1, borde);
        if (x > 0 && x < w - 1) pon(img, x, 1, {122, 84, 50});  // brillo superior
    }
    for (int y = 0; y < h; ++y) {
        pon(img, 0, y, borde);
        pon(img, w - 1, y, borde);
    }
    if (p.a)  // costura
        for (int y : {3, h - 4})
            for (int x = 4; x < w - 4; ++x)
                if ((x / 2) % 2 == 0) pon(img, x, y, {201, 165, 106});
    if (p.c) esquinasLaton(img);
    if (p.b)  // remaches
        for (int x : {3, w - 4})
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    pon(img, x + dx, h / 2 + dy,
                        kLaton[(dy == -1 && dx == -1) ? 3 : (dy + dx < 1 ? 2 : 5)]);
    return amplia(img);
}

Imagen fabricaAdorno(const PedidoMaterial& p) {
    const int w = p.ancho / 2, h = 5;
    Imagen img;
    img.crea(w, h);
    const int c = w / 2;
    for (int x = 2; x < w - 2; ++x)
        if (std::abs(x - c) > 4) pon(img, x, 2, kTintaAdorno);
    for (int dy = -2; dy <= 2; ++dy)
        for (int dx = -2; dx <= 2; ++dx)
            if (std::abs(dy) + std::abs(dx) <= 2) pon(img, c + dx, 2 + dy, kTintaAdorno);
    for (int x : {c - 6, c + 6}) {
        pon(img, x, 1, kTintaAdorno);
        pon(img, x, 3, kTintaAdorno);
    }
    return amplia(img);
}

namespace {
Imagen sello() {
    const int s = 12;
    Imagen img;
    img.crea(s, s);
    for (int y = 0; y < s; ++y)
        for (int x = 0; x < s; ++x) {
            const double r = std::hypot(x - 5.5, y - 5.5);
            if (r > 5.6) continue;
            Color3 col = r < 4.2 ? Color3{154, 42, 28} : Color3{110, 26, 16};
            if (r < 2.2) col = {184, 64, 44};
            pon(img, x, y, col);
        }
    for (const auto& g : {std::pair<int, int>{9, 1}, {2, 10}, {10, 9}}) pon(img, g.first, g.second, {110, 26, 16});
    for (int y = 4; y < 8; ++y)
        for (int x = 5; x < 7; ++x) pon(img, x, y, {122, 30, 20});
    for (int y = 5; y < 7; ++y)
        for (int x = 4; x < 8; ++x) pon(img, x, y, {122, 30, 20});
    return amplia(img);
}

Imagen chincheta() {
    const int s = 6;
    Imagen img;
    img.crea(s, s);
    for (int y = 0; y < s; ++y)
        for (int x = 0; x < s; ++x) {
            const double r = std::hypot(x - 2.5, y - 2.5);
            if (r > 2.9) continue;
            Color3 col = r > 2.0 ? kLaton[1] : kLaton[2];
            if ((x == 1 && y == 1) || (x == 2 && y == 1) || (x == 1 && y == 2)) col = kLaton[3];
            pon(img, x, y, col);
        }
    Imagen copia = img;
    auto opaco = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < s && y < s && copia.pixel(x, y)[3] > 0;
    };
    for (int y = 0; y < s; ++y)
        for (int x = 0; x < s; ++x)
            if (opaco(x, y) && !(opaco(x - 1, y) && opaco(x + 1, y) && opaco(x, y - 1) && opaco(x, y + 1)))
                pon(img, x, y, kLaton[0]);
    return amplia(img);
}

// Cinta de marcapaginas: la punta en cola de golondrina (50% 74%) y la
// sombra interior de 4 px a la derecha, como en tema_pergamino.css.
Imagen cinta() {
    Imagen img;
    img.crea(14, 28);
    const Color3 carmin{0x9A, 0x2A, 0x1C}, carmin2{0x6E, 0x1A, 0x10};
    for (int y = 0; y < 28; ++y)
        for (int x = 0; x < 14; ++x) {
            const float cx = x + 0.5f;
            const float fondo = 28.0f - (7.0f - std::fabs(cx - 7.0f)) * (28.0f - 20.72f) / 7.0f;
            if (y + 0.5f < fondo) pon(img, x, y, x >= 10 ? carmin2 : carmin);
        }
    return img;
}

Imagen tachuela() {
    Imagen img;
    img.crea(12, 12);
    for (int y = 0; y < 12; ++y)
        for (int x = 0; x < 12; ++x) {
            Color3 c{0x2A, 0x1A, 0x0E};  // contorno de 2 px
            if (x >= 2 && x < 10 && y >= 2 && y < 10) {
                c = {0xC9, 0xA2, 0x4A};
                if (x >= 8 || y >= 8) c = {0x7A, 0x5E, 0x22};
                if (x < 4 || y < 4) c = {0xE6, 0xC6, 0x6A};
            }
            pon(img, x, y, c);
        }
    return img;
}

Imagen flecha() {
    Imagen img;
    img.crea(12, 8);
    for (int y = 0; y < 6; ++y)
        for (int x = y; x < 12 - y; ++x) pon(img, x, y, kLineaTinta);
    return img;
}
}  // namespace

Imagen fabricaPieza(const PedidoMaterial& p) {
    switch (p.tipo) {
        case Material::Sello: return sello();
        case Material::Chincheta: return chincheta();
        case Material::Cinta: return cinta();
        case Material::Tachuela: return tachuela();
        case Material::Flecha: return flecha();
        default: return {};
    }
}

Imagen fabricaLomo(const PedidoMaterial& p) {
    Imagen img;
    img.crea(p.ancho, p.alto);
    // Degradado de la bisagra: oscuro, cuero, brillo, cuero, oscuro.
    const Color3 tramos[5] = {{0x2A, 0x1A, 0x0E}, {0x3F, 0x28, 0x16}, {0x5E, 0x3D, 0x23},
                              {0x3F, 0x28, 0x16}, {0x2A, 0x1A, 0x0E}};
    const float paradas[5] = {0.0f, 0.30f, 0.55f, 0.80f, 1.0f};
    for (int y = 0; y < p.alto; ++y) {
        const float t = p.alto > 1 ? static_cast<float>(y) / (p.alto - 1) : 0.0f;
        int i = 0;
        while (i < 3 && t > paradas[i + 1]) ++i;
        const float f = (t - paradas[i]) / (paradas[i + 1] - paradas[i]);
        Color3 c;
        for (int k = 0; k < 3; ++k)
            c[static_cast<std::size_t>(k)] = static_cast<std::uint8_t>(std::lround(
                tramos[i][static_cast<std::size_t>(k)] * (1.0f - f) +
                tramos[i + 1][static_cast<std::size_t>(k)] * f));
        for (int x = 0; x < p.ancho; ++x) pon(img, x, y, c);
    }
    return img;
}

}  // namespace Interno

Imagen fabrica(const PedidoMaterial& p) {
    if (p.ancho <= 0 || p.alto <= 0) return {};
    switch (p.tipo) {
        case Material::Hoja: return Interno::fabricaHoja(p);
        case Material::Papiro: return Interno::fabricaPapiro(p);
        case Material::Cuero: return Interno::fabricaCuero(p);
        case Material::Pestana: return Interno::fabricaPestana(p);
        case Material::Adorno: return Interno::fabricaAdorno(p);
        case Material::Lomo: return Interno::fabricaLomo(p);
        case Material::Sello:
        case Material::Chincheta:
        case Material::Cinta:
        case Material::Tachuela:
        case Material::Flecha: return Interno::fabricaPieza(p);
    }
    return {};
}

}  // namespace Pergamino
