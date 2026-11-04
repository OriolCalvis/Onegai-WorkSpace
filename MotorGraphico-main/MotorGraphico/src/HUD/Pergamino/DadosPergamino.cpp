// H2-01 — ver include/HUD/Pergamino/DadosPergamino.h.
#include "HUD/Pergamino/DadosPergamino.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace dados {

namespace {

// Con qué regla se contó la tirada (el PoolResult no lo guarda): la que da
// los éxitos que trae. Ventaja: el 4 también vale medio; desventaja: el 5 no.
enum class Modo { Normal, Ventaja, Desventaja };

Modo modoDe(const RPG::PoolResult& r) {
    int seis = 0, cincos = 0, cuatros = 0;
    for (int v : r.rolls) {
        seis += v == 6;
        cincos += v == 5;
        cuatros += v == 4;
    }
    const float normal = static_cast<float>(seis) + 0.5f * static_cast<float>(cincos);
    const float ventaja = normal + 0.5f * static_cast<float>(cuatros);
    const float desventaja = static_cast<float>(seis);
    auto igual = [&](float x) { return std::fabs(r.successes - x) < 0.01f; };
    if (igual(normal)) return Modo::Normal;
    if (igual(ventaja)) return Modo::Ventaja;
    if (igual(desventaja)) return Modo::Desventaja;
    return Modo::Normal;
}

struct Estilo {
    Vector4 fondo, borde, puntos;
};

Estilo estiloDe(Cuenta c) {
    switch (c) {
        case kExito: return {rgb(0xD5, 0xE8, 0xC8), Color::kExito, Color::kExito};
        case kMedio: return {rgb(0xF3, 0xDC, 0xA4), Color::kAviso, Color::kAviso};
        case kPifia: return {rgb(0xF4, 0xCC, 0xC2), Color::kError, Color::kError};
        default: return {Color::kPapelLuz, Color::kLinea, Color::kTinta};
    }
}

}  // namespace

Cuenta cuentaDe(const RPG::PoolResult& r, int valor) {
    if (r.botch) return valor == 1 ? kPifia : kNada;
    const Modo m = modoDe(r);
    if (valor == 6) return kExito;
    if (valor == 5) return m == Modo::Desventaja ? kNada : kMedio;
    if (valor == 4) return m == Modo::Ventaja ? kMedio : kNada;
    return kNada;
}

void cara(Lienzo& l, int valor, float x, float y, float lado, Cuenta cuenta) {
    const Estilo e = estiloDe(cuenta);
    l.color(Rect{x, y, lado, lado}, e.borde);
    l.color(Rect{x + 1.0f, y + 1.0f, lado - 2.0f, lado - 2.0f}, e.fondo);
    // Los puntos en una rejilla de 3 x 3 con el mismo paso entero: en una
    // cara de 21 px, puntos de 3 en 4, 9 y 14 (con 2 px entre ellos; si se
    // tocan, el 6 se lee como dos rayas).
    const float punto = std::max(2.0f, std::round(lado * 0.15f));
    const float margen = std::max(2.0f, std::round(lado * 0.2f));
    const float paso = std::floor((lado - 2.0f * margen - punto) * 0.5f);
    const float pos[3] = {x + margen, x + margen + paso, x + margen + 2.0f * paso};
    const float posY[3] = {y + margen, y + margen + paso, y + margen + 2.0f * paso};
    auto pon = [&](int col, int fila) { l.color(Rect{pos[col], posY[fila], punto, punto}, e.puntos); };
    switch (valor) {
        case 1: pon(1, 1); break;
        case 2: pon(0, 0); pon(2, 2); break;
        case 3: pon(0, 0); pon(1, 1); pon(2, 2); break;
        case 4: pon(0, 0); pon(2, 0); pon(0, 2); pon(2, 2); break;
        case 5: pon(0, 0); pon(2, 0); pon(1, 1); pon(0, 2); pon(2, 2); break;
        case 6: pon(0, 0); pon(0, 1); pon(0, 2); pon(2, 0); pon(2, 1); pon(2, 2); break;
        default: break;
    }
}

namespace {

constexpr float kHueco = 3.0f;

float ladoDe(std::size_t n, float anchoMax, float ladoMax) {
    return std::clamp(std::floor((anchoMax + kHueco) / static_cast<float>(n)) - kHueco, 10.0f, ladoMax);
}

}  // namespace

float anchoFila(std::size_t n, float anchoMax, float ladoMax) {
    if (n == 0) return 0.0f;
    return static_cast<float>(n) * (ladoDe(n, anchoMax, ladoMax) + kHueco) - kHueco;
}

float fila(Lienzo& l, const RPG::PoolResult& r, float x, float y, float anchoMax, float ladoMax) {
    if (r.rolls.empty()) return 0.0f;
    const float lado = ladoDe(r.rolls.size(), anchoMax, ladoMax);
    const float dy = std::round((ladoMax - lado) * 0.5f);  // centrados en la fila
    float cx = x;
    for (int v : r.rolls) {
        cara(l, v, cx, y + dy, lado, cuentaDe(r, v));
        cx += lado + kHueco;
    }
    return cx - kHueco - x;
}

const char* rotulo(const RPG::TiradaVisible& t) {
    if (t.pifia()) return "¡PIFIA!";
    switch (t.grado) {
        case RPG::Degree::CRITICAL: return "CRÍTICO";
        case RPG::Degree::SUCCESS: return "ÉXITO";
        case RPG::Degree::PARTIAL: return "PARCIAL";
        case RPG::Degree::BOTCH: break;
    }
    return "FALLO";
}

Vector4 color(const RPG::TiradaVisible& t) {
    if (t.pifia()) return Color::kError;
    switch (t.grado) {
        case RPG::Degree::CRITICAL: return rgb(0x8A, 0x5A, 0x00);
        case RPG::Degree::SUCCESS: return Color::kExito;
        case RPG::Degree::PARTIAL: return Color::kAviso;
        case RPG::Degree::BOTCH: break;
    }
    return Color::kTinta3;
}

std::string exitos(float n) {
    char buf[16];
    std::snprintf(buf, sizeof buf, n == std::floor(n) ? "%.0f" : "%.1f", static_cast<double>(n));
    std::string s = buf;
    std::replace(s.begin(), s.end(), '.', ',');
    return s;
}

RPG::PoolResult rodando(const RPG::PoolResult& r, float t) {
    RPG::PoolResult v = r;
    v.botch = false;
    const unsigned tramo = static_cast<unsigned>(std::max(0.0f, t) * 14.0f);
    for (std::size_t i = 0; i < v.rolls.size(); ++i) {
        unsigned h = (tramo + 1u) * 2654435761u ^ (static_cast<unsigned>(i) + 7u) * 40503u;
        h ^= h >> 13;
        v.rolls[i] = 1 + static_cast<int>(h % 6u);
    }
    return v;
}

}  // namespace dados

namespace {

// DIALOGOS-01 (Oriol, 13/10: «no se entiende del todo si superamos la tirada
// o no») — El cuadro de la tirada lleva un marco del color del resultado:
// verde si se supera (éxito o crítico), rojo si se falla (fallo o pifia) y
// ámbar si sale a medias (parcial).
void marcoDelResultado(Lienzo& l, const RPG::TiradaVisible& t, float x, float y, float w, float h) {
    const bool superada = !t.pifia() && (t.grado == RPG::Degree::SUCCESS || t.grado == RPG::Degree::CRITICAL);
    const bool parcial = !t.pifia() && t.grado == RPG::Degree::PARTIAL;
    const Vector4 c = superada ? rgb(0x3F, 0x9B, 0x4A) : parcial ? Color::kAviso : Color::kError;
    const float g = 3.0f;
    l.color(Rect{x, y, w, g}, c);
    l.color(Rect{x, y + h - g, w, g}, c);
    l.color(Rect{x, y, g, h}, c);
    l.color(Rect{x + w - g, y, g, h}, c);
}

// El rótulo del grado en una caja; la pifia, en blanco sobre rojo.
float rotuloGrado(Lienzo& l, const Pincel& p, const RPG::TiradaVisible& t, float derecha, float y) {
    const std::string r = dados::rotulo(t);
    const float w = p.anchoPixel(r, 2);
    const float x = derecha - w;
    if (t.pifia()) {
        l.color(Rect{x - 8.0f, y - 4.0f, w + 16.0f, 22.0f}, Color::kError);
        p.pixel(l, r, x, y, 2, Color::kClaroCuero);
    } else {
        p.pixel(l, r, x, y, 2, dados::color(t));
    }
    return w + (t.pifia() ? 16.0f : 0.0f);
}

}  // namespace

float pintaTiradaCombate(Lienzo& l, const Pincel& p, const RPG::TiradaVisible& quieta, float x, float y,
                         float brillo, int de, int en, float rodar) {
    // Rodando: las caras cambian y el resultado (marco, grado y exitos) espera a que paren.
    const bool rueda = rodar >= 0.0f;
    RPG::TiradaVisible t = quieta;
    if (rueda) {
        t.tirada = dados::rodando(quieta.tirada, rodar);
        t.rival = dados::rodando(quieta.rival, rodar + 0.37f);
    }
    const float w = 280.0f;
    const float h = t.enfrentada ? 112.0f : 82.0f;
    if (brillo > 0.0f) {
        // Recien salida: un halo dorado de tres anillos, mas fuerte junto al papel.
        const float b = std::min(1.0f, brillo);
        const float anillos[3][2] = {{8.0f, 0.18f}, {5.0f, 0.30f}, {2.0f, 0.50f}};
        for (const auto& a : anillos)
            l.color(Rect{x - a[0], y - a[0], w + 2.0f * a[0], h + 2.0f * a[0]}, rgb(255, 214, 102, a[1] * b));
    }
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 90, 2, 0, 1), Vector2{x, y}, Color::kSombra);
    if (!rueda) marcoDelResultado(l, t, x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f);  // verde, rojo o ámbar
    const float xc = x + 16.0f, derecha = x + w - 16.0f;
    // Cabecera: qué se tira, cuál de las de su acción («2/3») y el grado.
    float wGrado = rueda ? 0.0f : rotuloGrado(l, p, t, derecha, y + 15.0f);
    if (en > 1) {
        const std::string cuenta = std::to_string(de) + "/" + std::to_string(en);
        const float wc = p.anchoPixel(cuenta, 2);
        p.pixel(l, cuenta, derecha - wGrado - 10.0f - wc, y + 15.0f, 2, Color::kTinta3);
        wGrado += 10.0f + wc;
    }
    p.texto(l, Letra::Lora18b, p.letras().recorta(Letra::Lora18b, t.que, w - 40.0f - wGrado), xc, y + 10.0f, 24.0f,
            Color::kTinta);
    // Una fila por lado. A la derecha los éxitos (los de quien tira, con su
    // bono: «(+0,5) 2,5» si el rival sangra) y pegados a ellos los dados; el
    // nombre se queda con lo que sobra.
    auto lado = [&](const std::string& nombre, const RPG::PoolResult& r, float bono, float yy, bool rival) {
        const std::string e = rueda ? std::string("?") : dados::exitos(r.successes + bono);
        float xd = derecha - p.anchoPixel(e, 2);
        p.pixel(l, e, xd, yy + 5.0f, 2, Color::kTinta);
        if (bono != 0.0f) {
            const std::string b = std::string("(") + (bono > 0.0f ? "+" : "-") + dados::exitos(std::fabs(bono)) + ")";
            xd -= 6.0f + static_cast<float>(p.letras().ancho(Letra::Lora16, b));
            p.texto(l, Letra::Lora16, b, xd, yy + 1.0f, 22.0f, Color::kTinta3);
        }
        xd -= 10.0f;
        float wd = 0.0f;
        if (r.rolls.empty()) {
            wd = static_cast<float>(p.letras().ancho(Letra::Lora16, "sin dados"));
            p.texto(l, Letra::Lora16, "sin dados", xd - wd, yy + 1.0f, 22.0f, Color::kTinta3);
        } else {
            wd = dados::anchoFila(r.rolls.size(), xd - (xc + 78.0f));  // el nombre, 70 px como poco
            dados::fila(l, r, xd - wd, yy + 2.0f, wd);
        }
        const float wNombre = xd - wd - 8.0f - xc;
        float xn = xc;
        if (rival) {  // «contra Nerea»; si no cabe entero, sin «contra»
            const float wContra = static_cast<float>(p.letras().ancho(Letra::Lora16, "contra "));
            if (wContra + static_cast<float>(p.letras().ancho(Letra::Lora16, nombre)) <= wNombre)
                xn += p.texto(l, Letra::Lora16, "contra ", xn, yy + 1.0f, 22.0f, Color::kTinta3);
        }
        p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, nombre, wNombre - (xn - xc)), xn, yy + 1.0f,
                22.0f, Color::kTinta2);
    };
    lado(t.quien, t.tirada, t.bono, y + 44.0f, false);
    if (t.enfrentada) lado(t.contra, t.rival, 0.0f, y + 74.0f, true);
    return h;
}

void pintaTiradaDialogo(Lienzo& l, const Pincel& p, const RPG::TiradaVisible& quieta, float derecha, float abajo,
                        float rodar) {
    // [QUÉ] dados éxitos · contra dados éxitos · GRADO, en una sola tira. Rodando: las caras cambian
    // y saltan un poco; el grado, el marco y los éxitos salen al parar.
    const bool rueda = rodar >= 0.0f;
    RPG::TiradaVisible t = quieta;
    if (rueda) {
        t.tirada = dados::rodando(quieta.tirada, rodar);
        t.rival = dados::rodando(quieta.rival, rodar + 0.37f);
    }
    const float salto = rueda ? -std::round(std::fabs(std::sin(rodar * 18.0f)) * 4.0f) : 0.0f;
    const float h = 44.0f;
    const std::string que = t.que;
    const float wQue = static_cast<float>(p.letras().ancho(Letra::Lora18b, que));
    const auto anchoFila = [](const RPG::PoolResult& r) { return dados::anchoFila(r.rolls.size(), 1e6f); };
    const std::string e1 = rueda ? std::string("?") : dados::exitos(t.exitos());
    const std::string e2 = rueda ? std::string("?") : dados::exitos(t.rival.successes);
    const bool conRival = t.enfrentada && !t.rival.rolls.empty();
    const std::string grado = dados::rotulo(quieta);
    // El ancho, con los exitos de verdad: la tira no cambia de tamano al parar los dados.
    float w = 20.0f + wQue + 14.0f + anchoFila(t.tirada) + 10.0f + p.anchoPixel(dados::exitos(quieta.exitos()), 2) + 20.0f +
              p.anchoPixel(grado, 2) + (t.pifia() ? 16.0f : 0.0f) + 20.0f;
    const float wContra = static_cast<float>(p.letras().ancho(Letra::Lora16, "contra"));
    if (conRival) w += wContra + 10.0f + anchoFila(t.rival) + 10.0f + p.anchoPixel(dados::exitos(quieta.rival.successes), 2) + 20.0f;
    w = std::ceil(w / 2.0f) * 2.0f;
    const float x = std::round(derecha - w), y = std::round(abajo - h);
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 91, 1, 0, 0), Vector2{x, y}, Color::kSombra);
    if (!rueda) marcoDelResultado(l, t, x, y, w, h);
    float cx = x + 20.0f;
    cx += p.texto(l, Letra::Lora18b, que, cx, y + 9.0f, 26.0f, Color::kTinta) + 14.0f;
    cx += dados::fila(l, t.tirada, cx, y + 12.0f + salto, anchoFila(t.tirada)) + 10.0f;
    cx += p.pixel(l, e1, cx, y + 15.0f, 2, Color::kTinta) + 20.0f;
    if (conRival) {
        cx += p.texto(l, Letra::Lora16, "contra", cx, y + 10.0f, 24.0f, Color::kTinta3) + 10.0f;
        cx += dados::fila(l, t.rival, cx, y + 12.0f + salto, anchoFila(t.rival)) + 10.0f;
        cx += p.pixel(l, e2, cx, y + 15.0f, 2, Color::kTinta) + 20.0f;
    }
    if (!rueda) rotuloGrado(l, p, t, x + w - 20.0f, y + 15.0f);
}

}  // namespace Pergamino
