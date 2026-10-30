// H2-03 — Los estados en el combate en pergamino (ver Game/EstadosCombate.h).
//
// Cada estado es una ficha cuadrada con su icono del atlas, el borde rojo si
// va en contra de quien lo lleva (verde si a favor) y, en la esquina, las
// rondas que le quedan: bajan cada ronda y la ficha se va cuando se acaba.
//   - En la placa del enemigo, con su nombre al lado (uno o dos); con tres o
//     mas, solo las fichas.
//   - Los del heroe, bajo su retrato en el orden de turno.
//   - Con el raton encima (o un clic), un emergente con el nombre, lo que le
//     queda y lo que hace.
#include <algorithm>
#include <cmath>

#include "MesaCombateInterno.h"

namespace Pergamino::combate {

namespace {

using Z = Game::BarraCombate::Zona;
using Signo = Game::EstadoVisible::Signo;

Vector4 colorDe(Signo s) {
    switch (s) {
        case Signo::AFavor: return Color::kExito;
        case Signo::Neutro: return Color::kLinea;
        case Signo::Contra: break;
    }
    return Color::kError;
}

std::string conPila(const Game::EstadoVisible& e) {
    return e.pila > 1 ? e.nombre + " ×" + std::to_string(e.pila) : e.nombre;
}

}  // namespace

const char* iconoDeEstado(const Pincel& p, const Game::EstadoVisible& e) {
    if (!e.icono.empty() && p.iconos().frame(e.icono) > 0) return e.icono.c_str();
    return e.signo == Signo::AFavor ? "mejora" : "perjuicio";
}

void pintaFicha(const Ctx& k, const Game::EstadoVisible& e, float x, float y, float lado, int zona) {
    const Rect r{x, y, lado, lado};
    k.l.color(r, colorDe(e.signo));
    k.l.color(Rect{x + 1.0f, y + 1.0f, lado - 2.0f, lado - 2.0f}, Color::kPapelLuz);
    const float m = std::floor((lado - 16.0f) * 0.5f);
    k.p.icono(k.l, iconoDeEstado(k.p, e), x + m, y + m, 1);
    if (e.rondas > 0) {
        // Las rondas, en tinta clara sobre una etiqueta oscura en la esquina.
        const std::string n = std::to_string(e.rondas);
        const float w = k.p.anchoPixel(n, 1) + 4.0f;
        const Rect b{x + lado - w + 2.0f, y + lado - 9.0f, w, 10.0f};
        k.l.color(b, Color::kTinta);
        k.p.pixel(k.l, n, b.x + 2.0f, b.y - 1.0f, 1, Color::kClaroCuero);
    }
    if (zona >= 0) k.l.zona(r, static_cast<int>(Z::Estado), zona);
}

float altoEstadosPlaca(const std::vector<Game::EstadoVisible>& estados, bool soloFichas) {
    if (estados.empty()) return 0.0f;
    return estados.size() <= 2 && !soloFichas ? 30.0f * static_cast<float>(estados.size()) + 2.0f : 32.0f;
}

void pintaEstadosPlaca(const Ctx& k, const std::vector<Game::EstadoVisible>& estados, int quien, float x,
                       float y, float ancho, bool soloFichas) {
    const int base = Game::BarraCombate::kEstadoPorQuien * quien;
    if (estados.size() <= 2 && !soloFichas) {
        for (std::size_t j = 0; j < estados.size(); ++j) {
            const float yy = y + 30.0f * static_cast<float>(j);
            pintaFicha(k, estados[j], x, yy, 24.0f, base + static_cast<int>(j));
            k.p.texto(k.l, Letra::Lora16,
                      k.p.letras().recorta(Letra::Lora16, conPila(estados[j]), ancho - 32.0f), x + 32.0f, yy,
                      24.0f, Color::kTinta2);
        }
        return;
    }
    // Tres o mas: solo las fichas; las que no caben, «+N».
    const int caben = std::max(1, static_cast<int>((ancho + 4.0f) / 28.0f));
    const int n = static_cast<int>(estados.size());
    const int fichas = n <= caben ? n : caben - 1;
    for (int j = 0; j < fichas; ++j)
        pintaFicha(k, estados[static_cast<std::size_t>(j)], x + 28.0f * static_cast<float>(j), y, 24.0f, base + j);
    if (fichas < n) k.p.pixel(k.l, "+" + std::to_string(n - fichas), x + 28.0f * fichas + 2.0f, y + 4.0f, 2, Color::kTinta2);
}

void pintaEstadosHeroe(const Ctx& k, const std::vector<Game::EstadoVisible>& estados, float x, float y) {
    // Bajo un retrato de 64: tres fichas de 20, o dos y «+N».
    const int n = static_cast<int>(estados.size());
    const int fichas = n <= 3 ? n : 2;
    for (int j = 0; j < fichas; ++j)
        pintaFicha(k, estados[static_cast<std::size_t>(j)], x + 22.0f * static_cast<float>(j), y, 20.0f, j);
    if (fichas < n) k.p.pixel(k.l, "+" + std::to_string(n - fichas), x + 46.0f, y + 2.0f, 2, Color::kTinta2);
}

void pintaEmergenteEstado(const Ctx& k, const DatosCombate& d, std::size_t desdeZona) {
    const int indice = d.barra->estadoSenalado();
    const Game::EstadoVisible* e = Game::BarraCombate::estado(*d.vista, indice);
    if (e == nullptr || d.barra->emergente().abierto) return;
    const Zona* ficha = nullptr;
    for (std::size_t i = desdeZona; i < k.l.zonas().size(); ++i) {
        const Zona& z = k.l.zonas()[i];
        if (z.tipo == static_cast<int>(Z::Estado) && z.indice == indice) ficha = &z;
    }
    if (ficha == nullptr) return;
    const float w = 280.0f, ancho = w - 44.0f;
    const auto lineas = k.p.letras().envuelve(Letra::Lora16, e->descripcion, ancho);
    const int n = std::min(4, static_cast<int>(lineas.size()));
    const float h = 66.0f + (n > 0 ? 6.0f + 21.0f * static_cast<float>(n) : 0.0f) + 12.0f;
    float x = std::clamp(ficha->rect.x - 16.0f, 8.0f, 1280.0f - 8.0f - w);
    float y = ficha->rect.abajo() + 8.0f;
    if (y + h > 712.0f) y = ficha->rect.y - 8.0f - h;
    x = std::round(x);
    y = std::round(y);
    k.l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(std::ceil(h / 2.0f) * 2.0f), 92, 2, 0, 1),
                          Vector2{x, y}, Color::kSombra);
    k.l.color(Rect{x + 14.0f, y + 12.0f, 3.0f, 44.0f}, colorDe(e->signo));
    k.p.icono(k.l, iconoDeEstado(k.p, *e), x + 26.0f, y + 16.0f, 1);
    k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, conPila(*e), w - 72.0f), x + 50.0f, y + 10.0f,
              26.0f, Color::kTinta);
    const std::string queda = e->rondas == 1  ? "Queda 1 ronda"
                              : e->rondas > 1 ? "Quedan " + Game::duracionDe(*e)
                                              : conMayuscula(Game::duracionDe(*e));
    k.p.texto(k.l, Letra::Lora16, queda, x + 26.0f, y + 36.0f, 22.0f, Color::kTinta2);
    if (n > 0) k.p.parrafo(k.l, Letra::Lora16, e->descripcion, x + 26.0f, y + 66.0f, ancho, 21.0f, Color::kTinta, 4);
}

}  // namespace Pergamino::combate
