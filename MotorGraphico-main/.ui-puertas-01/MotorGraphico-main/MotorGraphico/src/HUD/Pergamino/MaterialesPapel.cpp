// PERGAMINO-01 — El papel: hoja, papiro y pestana (ver Materiales.h).
//
// Traduccion directa de texturas.py (maquetas del 28/09). Todo se calcula en
// la rejilla logica (media resolucion) y se amplia x2 al final, igual que
// los sprites del juego.
#include <algorithm>
#include <cmath>

#include "MaterialesInterno.h"

namespace Pergamino {

void Imagen::crea(int w, int h) {
    ancho = std::max(0, w);
    alto = std::max(0, h);
    rgba.assign(static_cast<std::size_t>(ancho) * static_cast<std::size_t>(alto) * 4, 0);
}

namespace Interno {

const Color3 kPapel[6] = {{242, 228, 193}, {235, 219, 178}, {226, 205, 156},
                          {214, 188, 133}, {198, 165, 108}, {174, 138, 85}};
const Color3 kLineaTinta = {74, 52, 33};
const Color3 kTintaAdorno = {107, 80, 56};

namespace {
const int kBayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

float suave(float t) { return t * t * (3.0f - 2.0f * t); }

// Desplazamiento 0..amp a lo largo de un borde (rasgado suave).
std::vector<int> perfil(int n, int amp, Azar& azar, int celda = 5) {
    std::vector<int> salida(static_cast<std::size_t>(std::max(0, n)), 0);
    if (amp <= 0 || n <= 0) return salida;
    std::vector<double> puntos(static_cast<std::size_t>(n / celda + 3));
    for (double& p : puntos) p = azar.real();
    for (int x = 0; x < n; ++x) {
        const double fx = static_cast<double>(x) / celda;
        const int i = static_cast<int>(fx);
        const double t = fx - i;
        double v = puntos[static_cast<std::size_t>(i)] * (1.0 - t) +
                   puntos[static_cast<std::size_t>(i + 1)] * t;
        v += (azar.real() - 0.5) * 0.35;
        salida[static_cast<std::size_t>(x)] =
            std::clamp(static_cast<int>(std::lround(v * amp)), 0, amp);
    }
    return salida;
}
}  // namespace

float bayer(int x, int y) { return static_cast<float>(kBayer[y & 3][x & 3]) / 16.0f; }

void pon(Imagen& img, int x, int y, const Color3& c, std::uint8_t alfa) {
    if (x < 0 || y < 0 || x >= img.ancho || y >= img.alto) return;
    std::uint8_t* p = img.pixel(x, y);
    p[0] = c[0];
    p[1] = c[1];
    p[2] = c[2];
    p[3] = alfa;
}

Rejilla<float> ruido(int w, int h, int celda, Azar& azar) {
    const int gw = std::max(2, w / celda + 2);
    const int gh = std::max(2, h / celda + 2);
    Rejilla<float> g(gw, gh, 0.0f);
    for (float& v : g.v) v = static_cast<float>(azar.real());
    Rejilla<float> salida(w, h, 0.0f);
    for (int y = 0; y < h; ++y) {
        const float fy = static_cast<float>(y) / celda;
        const int y0 = static_cast<int>(fy);
        const float ty = suave(fy - y0);
        for (int x = 0; x < w; ++x) {
            const float fx = static_cast<float>(x) / celda;
            const int x0 = static_cast<int>(fx);
            const float tx = suave(fx - x0);
            const float a = g.at(x0, y0) * (1 - tx) + g.at(x0 + 1, y0) * tx;
            const float b = g.at(x0, y0 + 1) * (1 - tx) + g.at(x0 + 1, y0 + 1) * tx;
            salida.at(x, y) = a * (1 - ty) + b * ty;
        }
    }
    return salida;
}

Rejilla<std::uint8_t> mascara(int w, int h, int rasgado, Azar& azar, int lados) {
    Rejilla<std::uint8_t> m(w, h, 1);
    if (lados & 1) {
        const auto p = perfil(w, rasgado, azar);
        for (int x = 0; x < w; ++x)
            for (int y = 0; y < std::min(h, p[static_cast<std::size_t>(x)]); ++y) m.at(x, y) = 0;
    }
    if (lados & 2) {
        const auto p = perfil(w, rasgado, azar);
        for (int x = 0; x < w; ++x)
            for (int y = std::max(0, h - p[static_cast<std::size_t>(x)]); y < h; ++y) m.at(x, y) = 0;
    }
    if (lados & 4) {
        const auto p = perfil(h, rasgado, azar);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < std::min(w, p[static_cast<std::size_t>(y)]); ++x) m.at(x, y) = 0;
    }
    if (lados & 8) {
        const auto p = perfil(h, rasgado, azar);
        for (int y = 0; y < h; ++y)
            for (int x = std::max(0, w - p[static_cast<std::size_t>(y)]); x < w; ++x) m.at(x, y) = 0;
    }
    // Esquinas mordidas.
    const int esquinas[4][2] = {{0, 0}, {w - 1, 0}, {0, h - 1}, {w - 1, h - 1}};
    for (const auto& e : esquinas) {
        const int r = azar.entero(1, 3 + rasgado);
        for (int dy = 0; dy < r; ++dy) {
            for (int dx = 0; dx < r - dy; ++dx) {
                const int yy = e[1] == 0 ? e[1] + dy : e[1] - dy;
                const int xx = e[0] == 0 ? e[0] + dx : e[0] - dx;
                if (xx >= 0 && yy >= 0 && xx < w && yy < h) m.at(xx, yy) = 0;
            }
        }
    }
    return m;
}

Rejilla<int> distanciaBorde(const Rejilla<std::uint8_t>& m) {
    const int w = m.ancho, h = m.alto;
    Rejilla<int> d(w, h, 0);
    for (std::size_t i = 0; i < d.v.size(); ++i) d.v[i] = m.v[i] ? 999 : 0;
    for (int paso = 0; paso < 14; ++paso) {
        Rejilla<int> n = d;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (!m.at(x, y)) continue;
                int v = n.at(x, y);
                if (y > 0) v = std::min(v, d.at(x, y - 1) + 1);
                if (y + 1 < h) v = std::min(v, d.at(x, y + 1) + 1);
                if (x > 0) v = std::min(v, d.at(x - 1, y) + 1);
                if (x + 1 < w) v = std::min(v, d.at(x + 1, y) + 1);
                n.at(x, y) = v;
            }
        }
        d = std::move(n);
    }
    for (int x = 0; x < w; ++x) {
        d.at(x, 0) = std::min(d.at(x, 0), 1);
        d.at(x, h - 1) = std::min(d.at(x, h - 1), 1);
    }
    for (int y = 0; y < h; ++y) {
        d.at(0, y) = std::min(d.at(0, y), 1);
        d.at(w - 1, y) = std::min(d.at(w - 1, y), 1);
    }
    for (std::size_t i = 0; i < d.v.size(); ++i)
        if (!m.v[i]) d.v[i] = 0;
    return d;
}

Rejilla<float> papel(int w, int h, Azar& azar, float tono, int manchas, bool fibras,
                     float oscuro) {
    const Rejilla<float> grande = ruido(w, h, 26, azar);
    const Rejilla<float> fino = ruido(w, h, 7, azar);
    Rejilla<float> t(w, h, 0.0f);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            t.at(x, y) = 0.34f + tono + 0.20f * (grande.at(x, y) - 0.5f) +
                         0.10f * (fino.at(x, y) - 0.5f) +
                         0.05f * (static_cast<float>(azar.real()) - 0.5f);
    for (int i = 0; i < manchas; ++i) {
        const float cy = static_cast<float>(azar.real()) * h;
        const float cx = static_cast<float>(azar.real()) * w;
        const float r = 6.0f + static_cast<float>(azar.real()) * std::min(w, h) * 0.18f;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                const float d2 = (y - cy) * (y - cy) + (x - cx) * (x - cx);
                t.at(x, y) += 0.10f * std::exp(-d2 / (r * r));
            }
    }
    if (fibras) {
        const int n = static_cast<int>(w * h * 0.004);
        for (int i = 0; i < n; ++i) {
            const int y = azar.entero(0, h), x = azar.entero(0, w), largo = azar.entero(3, 8);
            for (int k = 0; k < largo && x + k < w; ++k) t.at(x + k, y) += 0.11f;
        }
    }
    for (float& v : t.v) v += oscuro;
    return t;
}

Imagen cuantiza(Rejilla<float>& t, const Rejilla<std::uint8_t>& m, const Rejilla<int>& d,
                int canto) {
    Imagen img;
    img.crea(t.ancho, t.alto);
    for (int y = 0; y < t.alto; ++y) {
        for (int x = 0; x < t.ancho; ++x) {
            const float rim = std::pow(std::clamp(1.0f - static_cast<float>(d.at(x, y)) / canto,
                                                  0.0f, 1.0f), 1.3f) * 0.72f;
            const float v = t.at(x, y) + rim;
            const int idx = std::clamp(static_cast<int>(std::floor(v * 5.0f + bayer(x, y))), 0, 5);
            if (!m.at(x, y)) continue;
            const int dist = d.at(x, y);
            pon(img, x, y, dist <= 1 ? kLineaTinta : (dist == 2 ? kPapel[5] : kPapel[idx]));
        }
    }
    return img;
}

void filete(Imagen& img, int inset) {
    const int a = inset, b = inset, c = img.ancho - 1 - inset, dd = img.alto - 1 - inset;
    if (c <= a + 6 || dd <= b + 6) return;
    for (int x = a + 3; x < c - 2; ++x) {
        pon(img, x, b, kTintaAdorno);
        pon(img, x, dd, kTintaAdorno);
    }
    for (int y = b + 3; y < dd - 2; ++y) {
        pon(img, a, y, kTintaAdorno);
        pon(img, c, y, kTintaAdorno);
    }
    const int esq[4][2] = {{a, b}, {c, b}, {a, dd}, {c, dd}};
    for (const auto& e : esq)
        for (int dy = -2; dy <= 2; ++dy)
            for (int dx = -2; dx <= 2; ++dx)
                if (std::abs(dy) + std::abs(dx) <= 2) pon(img, e[0] + dx, e[1] + dy, kTintaAdorno);
}

Imagen amplia(const Imagen& logica, int factor) {
    Imagen salida;
    salida.crea(logica.ancho * factor, logica.alto * factor);
    for (int y = 0; y < salida.alto; ++y)
        for (int x = 0; x < salida.ancho; ++x) {
            const std::uint8_t* s = logica.pixel(x / factor, y / factor);
            std::copy(s, s + 4, salida.pixel(x, y));
        }
    return salida;
}

Imagen fabricaHoja(const PedidoMaterial& p) {
    Azar azar(static_cast<std::uint64_t>(p.semilla));
    const int w = p.ancho / 2, h = p.alto / 2;
    const auto m = mascara(w, h, p.a, azar);
    const auto d = distanciaBorde(m);
    auto t = papel(w, h, azar, 0.0f, p.c, true, static_cast<float>(p.d) / 100.0f);
    Imagen img = cuantiza(t, m, d, 9);
    if (p.b > 0) filete(img, p.b);
    return amplia(img);
}

namespace {
Imagen rodillo(int w, int alto, int remate) {
    Imagen img;
    img.crea(w, alto + 2);
    for (int y = 0; y < alto; ++y)
        for (int x = remate; x < w - remate; ++x) pon(img, x, y + 1, kMadera[y]);
    for (int lado = 0; lado < 2; ++lado) {
        const int x0 = lado == 0 ? 0 : w - remate;
        for (int x = x0; x < x0 + remate; ++x)
            for (int y = 0; y < alto + 2; ++y) pon(img, x, y, kLaton[std::clamp(y - 1, 0, alto - 1)]);
        for (int x : {x0, x0 + remate - 1}) {  // redondeo
            img.pixel(x, 0)[3] = 0;
            img.pixel(x, alto + 1)[3] = 0;
        }
    }
    // Contorno: todo pixel opaco con algun vecino transparente (o fuera).
    Imagen copia = img;
    auto opaco = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < w && y < alto + 2 && copia.pixel(x, y)[3] > 0;
    };
    for (int y = 0; y < alto + 2; ++y)
        for (int x = 0; x < w; ++x)
            if (opaco(x, y) && !(opaco(x - 1, y) && opaco(x + 1, y) && opaco(x, y - 1) && opaco(x, y + 1)))
                pon(img, x, y, {45, 28, 14});
    return img;
}
}  // namespace

Imagen fabricaPapiro(const PedidoMaterial& p) {
    Azar azar(static_cast<std::uint64_t>(p.semilla));
    const int sobresale = 8, altoR = 12;
    const int W = p.ancho / 2, H = p.alto / 2;
    const int wh = W - 2 * sobresale, hh = H - 2 * (altoR / 2);
    if (wh <= 16 || hh <= 16) return {};
    const auto m = mascara(wh, hh, 1, azar, 4 | 8);
    auto t = papel(wh, hh, azar, 0.0f, 3, true, 0.0f);
    // Sombra de la curva junto a los rodillos.
    for (int y = 0; y < hh; ++y) {
        const float s = 0.40f * std::clamp(1.0f - y / 7.0f, 0.0f, 1.0f) +
                        0.40f * std::clamp(1.0f - (hh - 1 - y) / 7.0f, 0.0f, 1.0f);
        for (int x = 0; x < wh; ++x) t.at(x, y) += s;
    }
    // Solo los lados llevan canto envejecido.
    Rejilla<int> dd(wh, hh, 0);
    for (int y = 0; y < hh; ++y)
        for (int x = 0; x < wh; ++x)
            dd.at(x, y) = m.at(x, y) ? std::min(std::min(x, wh - 1 - x) + 1, 99) : 0;
    Imagen hojaImg = cuantiza(t, m, dd, 7);
    filete(hojaImg, 7);
    Imagen img;
    img.crea(W, H);
    const int y0 = altoR / 2;
    for (int y = 0; y < hh; ++y)
        for (int x = 0; x < wh; ++x) {
            const std::uint8_t* s = hojaImg.pixel(x, y);
            std::copy(s, s + 4, img.pixel(sobresale + x, y0 + y));
        }
    const Imagen rod = rodillo(W, altoR - 2, 7);
    for (int yy : {0, H - rod.alto})
        for (int y = 0; y < rod.alto; ++y)
            for (int x = 0; x < W; ++x) {
                const std::uint8_t* s = rod.pixel(x, y);
                if (s[3] > 0) std::copy(s, s + 4, img.pixel(x, yy + y));
            }
    return amplia(img);
}

Imagen fabricaPestana(const PedidoMaterial& p) {
    Azar azar(static_cast<std::uint64_t>(p.semilla));
    const bool activa = p.a != 0;
    const int w = p.ancho / 2, h = p.alto / 2;
    if (w < 4 || h < 4) return {};
    Rejilla<std::uint8_t> m(w, h, 1);
    for (const auto& e : {std::pair<int, int>{0, 0}, {1, 0}, {0, 1}, {w - 1, 0}, {w - 2, 0}, {w - 1, 1}})
        m.at(e.first, e.second) = 0;
    auto d = distanciaBorde(m);
    if (activa)
        for (int x = 0; x < w; ++x) d.at(x, h - 1) = 99;  // la activa se funde con la hoja
    auto t = papel(w, h, azar, activa ? 0.0f : 0.28f, 0, false, 0.0f);
    Imagen img = cuantiza(t, m, d, 4);
    if (activa)
        for (int x = 1; x < w - 1; ++x) pon(img, x, h - 1, kPapel[2]);
    return amplia(img);
}

}  // namespace Interno
}  // namespace Pergamino
