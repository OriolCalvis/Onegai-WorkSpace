// ESTETICA-01 — ver include/Game/Personajes/BrilloEquipo.h.
#include "Game/Personajes/BrilloEquipo.h"

#include <algorithm>
#include <cmath>

namespace Game {

namespace {

constexpr int kLado = MontajePersonajes::kLado;
constexpr int kFrames = MontajePersonajes::kFrames;
constexpr double kPi = 3.14159265358979323846;

bool esRopa(const std::string& ranura) { return ranura != "arma" && ranura != "mano_izq"; }

// Azar determinista: el mismo destello en el mismo instante, siempre.
std::uint32_t mezcla(std::uint32_t a, std::uint32_t b, std::uint32_t c) {
    std::uint32_t h = a * 0x9E3779B1u ^ (b + 0x7F4A7C15u) * 0x85EBCA77u ^ (c + 0x165667B1u) * 0xC2B2AE3Du;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    h *= 0x297A2D39u;
    h ^= h >> 15;
    return h;
}
float azar01(std::uint32_t a, std::uint32_t b, std::uint32_t c) {
    return static_cast<float>(mezcla(a, b, c) & 0xFFFFFFu) / static_cast<float>(0x1000000);
}
double fraccion(double x) { return x - std::floor(x); }

}  // namespace

namespace {
// PART-01: lo intensas que son las particulas de cada nivel (las mismas para armas, ropa y abalorios).
void intensidadDeParticulas(EscalonDeBrillo& e, int nivel) {
    // nivel -> opacidad: Tier 3 (+5/+6) un pelo, Tier 4 (+7/+8) mas, Tier 5 (+9/+10) mas aun
    static const float kOpacidad[11] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.08f, 1.15f, 1.35f, 1.45f, 1.7f, 1.85f};
    const int n = std::max(0, std::min(10, nivel));
    e.opacidad = kOpacidad[n];
    e.contraste = n >= 9 ? 0.35f : 0.0f;  // el Tier 5: otras mas oscuras, de contraste
}
}  // namespace

EscalonDeBrillo escalonDeBrillo(int nivel, bool ropa) {
    // nivel -> radio, alfa, latido, destellos/s, particulas/s (las armas)
    static const EscalonDeBrillo kEscalera[11] = {
        {0, 0.00f, 0.00f, 0.0f, 0.0f, false},   // 0: no brilla
        {0, 0.00f, 0.00f, 0.5f, 0.0f, false},   // +1  Tier 1
        {0, 0.00f, 0.00f, 1.0f, 0.0f, false},   // +2
        {1, 0.30f, 0.15f, 1.2f, 0.0f, false},   // +3  Tier 2
        {1, 0.45f, 0.20f, 1.6f, 0.0f, false},   // +4
        {2, 0.45f, 0.25f, 2.0f, 2.0f, false},   // +5  Tier 3
        {2, 0.55f, 0.25f, 2.4f, 4.0f, false},   // +6
        {2, 0.65f, 0.30f, 3.0f, 6.0f, false},   // +7  Tier 4
        {3, 0.75f, 0.30f, 3.5f, 9.0f, false},   // +8
        {3, 0.85f, 0.35f, 4.0f, 13.0f, false},  // +9  Tier 5
        {4, 1.00f, 0.35f, 5.0f, 18.0f, false},  // +10
    };
    EscalonDeBrillo e = kEscalera[std::max(0, std::min(10, nivel))];
    intensidadDeParticulas(e, nivel);
    if (ropa) {
        e.radio = std::max(0, e.radio - 1);
        e.alfa *= 0.5f;
        e.destellos *= 0.6f;
        e.particulas *= 0.4f;
        e.barrido = nivel >= 3;
    }
    return e;
}

EscalonDeBrillo escalonDeAbalorio(int nivel) {
    EscalonDeBrillo e;
    if (nivel <= 0)
        return e;
    intensidadDeParticulas(e, nivel);
    e.destellos = 0.4f + 0.1f * static_cast<float>(nivel);
    e.particulas = 0.8f + 0.5f * static_cast<float>(nivel);
    return e;
}

namespace {
EscalonDeBrillo escalonDe(const FuenteDeBrillo& f) {
    return f.esAbalorio ? escalonDeAbalorio(f.brillo.nivel) : escalonDeBrillo(f.brillo.nivel, f.esRopa);
}
}  // namespace

HojaDeBrillo montarBrillo(const imagen::ImagenRGBA& hoja, const MapaDeCapas& mapa,
                          const std::map<std::string, BrilloDePieza>& brillos) {
    HojaDeBrillo out;
    const int ancho = kLado * kFrames;
    out.halo.ancho = ancho;
    out.halo.alto = kLado;
    out.halo.pixeles.assign(static_cast<std::size_t>(ancho) * kLado * 4, 0);
    if (hoja.ancho != ancho || hoja.alto != kLado ||
        mapa.arriba.size() != static_cast<std::size_t>(ancho) * kLado)
        return out;

    // Por pixel: cuanta luz llega de cada pieza (peso) y de que color (suma ponderada).
    std::vector<float> peso(static_cast<std::size_t>(ancho) * kLado, 0.0f);
    std::vector<std::array<float, 3>> color(peso.size(), {0.0f, 0.0f, 0.0f});
    for (const auto& [ranura, brillo] : brillos) {
        if (brillo.nivel <= 0)
            continue;
        // Un abalorio brilla en todo el cuerpo; una pieza, en lo que se ve de ella.
        const bool abalorio = brillo.abalorio || ranura.rfind("abalorio:", 0) == 0;
        const auto capa = std::find(mapa.ranuras.begin(), mapa.ranuras.end(), ranura);
        if (!abalorio && capa == mapa.ranuras.end())
            continue;
        const std::uint8_t id = abalorio ? 0 : static_cast<std::uint8_t>(capa - mapa.ranuras.begin() + 1);
        FuenteDeBrillo fuente;
        fuente.ranura = ranura;
        fuente.brillo = brillo;
        fuente.esAbalorio = abalorio;
        fuente.esRopa = !abalorio && esRopa(ranura);
        const EscalonDeBrillo escalon = escalonDe(fuente);
        const int r = escalon.radio;
        // PART-02b: un abalorio con sitio echa desde su ancla en cada frame (el anillo, del puno; el collar,
        // de debajo del cuello; el cinturon, de la linea del cinturon, a lo ancho)
        const std::vector<AnclasDeTalla::Punto>* anclas = abalorio ? mapa.anclas.de(brillo.sitio) : nullptr;
        for (int f = 0; f < kFrames; ++f) {
            if (anclas != nullptr && static_cast<std::size_t>(f) < anclas->size()) {
                const auto& [ax, ay] = (*anclas)[static_cast<std::size_t>(f)];
                const bool cintura = brillo.sitio == "cintura";
                for (int dy = cintura ? 0 : -1; dy <= 1; ++dy) {
                    for (int dx = cintura ? -3 : -1; dx <= (cintura ? 3 : 1); ++dx) {
                        const int x = ax + dx, y = ay + dy;
                        if (x >= 0 && x < kLado && y >= 0 && y < kLado)
                            fuente.pixeles[static_cast<std::size_t>(f)].push_back(
                                {static_cast<std::uint8_t>(x), static_cast<std::uint8_t>(y)});
                    }
                }
                continue;
            }
            const int x0 = f * kLado;
            for (int y = 0; y < kLado; ++y) {
                for (int x = 0; x < kLado; ++x) {
                    const std::uint8_t arriba =
                        mapa.arriba[static_cast<std::size_t>(y) * ancho + static_cast<std::size_t>(x0 + x)];
                    if (abalorio ? arriba == 0 : arriba != id)
                        continue;
                    fuente.pixeles[static_cast<std::size_t>(f)].push_back(
                        {static_cast<std::uint8_t>(x), static_cast<std::uint8_t>(y)});
                    if (r <= 0)
                        continue;
                    // El halo: la luz cae con la distancia; encima del cuerpo, poco
                    // (las armas) o nada (la ropa: solo el borde de fuera).
                    for (int dy = -r - 1; dy <= r + 1; ++dy) {
                        for (int dx = -r - 1; dx <= r + 1; ++dx) {
                            const int xx = x + dx, yy = y + dy;
                            if (xx < 0 || xx >= kLado || yy < 0 || yy >= kLado)
                                continue;
                            const float d = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                            if (d > static_cast<float>(r) + 0.75f)
                                continue;
                            float v = std::max(0.0f, 1.0f - d / (static_cast<float>(r) + 1.0f)) * escalon.alfa;
                            const std::size_t i = static_cast<std::size_t>(yy) * ancho + static_cast<std::size_t>(x0 + xx);
                            if (hoja.pixeles[i * 4 + 3] != 0)
                                v *= fuente.esRopa ? 0.0f : 0.35f;
                            if (v <= peso[i] * 0.999f)  // gana la luz mas fuerte de esa pieza...
                                continue;
                            const float antes = peso[i];
                            peso[i] = v;
                            for (int c = 0; c < 3; ++c)  // ...y el color se mezcla con el de otras
                                color[i][c] = antes > 0.0f ? (color[i][c] * antes + brillo.color[c] * (v - antes)) / v
                                                           : static_cast<float>(brillo.color[c]);
                        }
                    }
                }
            }
        }
        // PART-02b: dos efectos (fuego y hielo), dos fuentes en el mismo sitio, a medias
        if (abalorio && !brillo.otrasParticulas.empty()) {
            fuente.parte = 0.5f;
            FuenteDeBrillo otra = fuente;
            otra.ranura = ranura + "#2";
            otra.brillo.particulas = brillo.otrasParticulas;
            otra.brillo.color = brillo.otroColor;
            otra.conDestellos = false;
            out.fuentes.push_back(std::move(fuente));
            out.fuentes.push_back(std::move(otra));
            continue;
        }
        out.fuentes.push_back(std::move(fuente));
    }
    for (std::size_t i = 0; i < peso.size(); ++i) {
        if (peso[i] <= 0.0f)
            continue;
        for (int c = 0; c < 3; ++c)
            out.halo.pixeles[i * 4 + c] = static_cast<std::uint8_t>(std::lround(std::min(255.0f, color[i][c])));
        out.halo.pixeles[i * 4 + 3] = static_cast<std::uint8_t>(std::lround(std::min(1.0f, peso[i]) * 255.0f));
    }
    return out;
}

float latidoDeBrillo(const HojaDeBrillo& brillo, double segundos) {
    float latido = 0.0f;
    for (const FuenteDeBrillo& f : brillo.fuentes)
        latido = std::max(latido, escalonDe(f).latido);
    const double onda = 0.5 + 0.5 * std::sin(segundos * 2.0 * kPi / 1.6);
    return static_cast<float>(1.0 - latido + latido * onda);
}

void destellosDeBrillo(const HojaDeBrillo& brillo, int frame, double segundos, std::vector<Destello>& out) {
    const std::size_t f = static_cast<std::size_t>(std::max(1, std::min(kFrames, frame)) - 1);
    for (std::size_t k = 0; k < brillo.fuentes.size(); ++k) {
        const FuenteDeBrillo& fuente = brillo.fuentes[k];
        const auto& pix = fuente.pixeles[f];
        if (pix.empty())
            continue;
        const std::uint32_t n = static_cast<std::uint32_t>(pix.size());
        const std::uint32_t kk = static_cast<std::uint32_t>(k) + 1;
        EscalonDeBrillo e = escalonDe(fuente);
        e.particulas *= fuente.parte;  // PART-02b: un abalorio de dos efectos, la mitad cada uno

        // Destellos: un pixel de la pieza se enciende un instante.
        if (e.destellos > 0.0f && fuente.conDestellos) {
            const int cuantos = static_cast<int>(e.destellos * 3.0f) + 1;
            for (int i = 0; i < cuantos; ++i) {
                const double t = segundos * e.destellos / cuantos + static_cast<double>(i) / cuantos +
                                 azar01(kk, static_cast<std::uint32_t>(i), 7u);
                const double fase = fraccion(t);
                if (fase >= 0.18)
                    continue;
                const auto ciclo = static_cast<std::uint32_t>(static_cast<std::int64_t>(std::floor(t)));
                const auto& p = pix[mezcla(kk, static_cast<std::uint32_t>(i), ciclo) % n];
                out.push_back({static_cast<float>(p[0]), static_cast<float>(p[1]),
                               static_cast<float>(1.0 - fase / 0.18), FormaDeParticula::Cruz, true,
                               {255, 255, 240}});
            }
        }

        // La ropa: un barrido de luz en diagonal cada 2,2 s.
        if (e.barrido) {
            const double centro = fraccion(segundos / 2.2 + static_cast<double>(k) * 0.3) * 140.0 - 20.0;
            const float fuerza = 0.25f + 0.05f * static_cast<float>(fuente.brillo.nivel);
            std::array<std::uint8_t, 3> claro{};
            for (int c = 0; c < 3; ++c)
                claro[static_cast<std::size_t>(c)] =
                    static_cast<std::uint8_t>(fuente.brillo.color[static_cast<std::size_t>(c)] / 2 + 127);
            for (const auto& p : pix) {
                const double banda = (p[0] + p[1]) - centro;
                const float v = static_cast<float>(std::max(0.0, 1.0 - std::abs(banda) / 3.0)) * fuerza;
                if (v > 0.0f)
                    out.push_back({static_cast<float>(p[0]), static_cast<float>(p[1]), v, FormaDeParticula::Punto,
                                   true, claro});
            }
        }

        // Particulas de su esencia: nacen en un pixel de la pieza y se van (PART-01: como se mueve cada familia).
        if (e.particulas > 0.0f) {
            const TipoDeParticula tipo = tipoDeParticula(fuente.brillo.particulas);
            const int cuantas = static_cast<int>(e.particulas * tipo.vida * tipo.densidad) + 1;
            // el centro de lo que se ve de la pieza: alrededor de el giran las que orbitan
            float cx = 0.0f, cy = 0.0f;
            for (const auto& q : pix) {
                cx += q[0];
                cy += q[1];
            }
            cx /= static_cast<float>(n);
            cy /= static_cast<float>(n);
            for (int i = 0; i < cuantas; ++i) {
                const std::uint32_t ii = static_cast<std::uint32_t>(i);
                const double desfase = azar01(kk, ii, 11u) * tipo.vida;
                const double vivida = segundos + desfase;
                const double edad = std::fmod(vivida, static_cast<double>(tipo.vida));
                const auto ciclo = static_cast<std::uint32_t>(static_cast<std::int64_t>(std::floor(vivida / tipo.vida)));
                if ((tipo.forma == FormaDeParticula::Rayo || tipo.movimiento == MovimientoDeParticula::Chispazo) &&
                    edad > 0.12)
                    continue;
                const auto& p = pix[mezcla(kk, ii, ciclo + 1000u) % n];
                const float lado = azar01(kk, ii, ciclo + 2000u) * 2.0f - 1.0f;
                const float t = static_cast<float>(edad);
                const float vivo = 1.0f - static_cast<float>(edad / tipo.vida);
                // lo vivas que salen, por tier: con mas opacidad tardan mas en apagarse (alfa hasta 1)
                const float alfa = std::min(1.0f, vivo * e.opacidad);
                std::array<std::uint8_t, 3> color = fuente.brillo.color;
                if (!tipo.colores.empty())
                    color = tipo.colores[mezcla(kk, ii, ciclo + 3000u) % tipo.colores.size()];
                // el Tier 5: algunas de un tono mas oscuro, que tapan, para darles contraste
                bool suma = tipo.suma;
                if (e.contraste > 0.0f && azar01(kk, ii, ciclo + 7000u) < e.contraste) {
                    for (auto& canal : color)
                        canal = static_cast<std::uint8_t>(canal * 42 / 100);
                    suma = false;
                }
                float x = static_cast<float>(p[0]), y = static_cast<float>(p[1]);
                // su forma: la de la familia, la otra para algunas (las cruces doradas de las monedas) y, si gira,
                // su otra cara a ratos
                FormaDeParticula forma = tipo.forma;
                if (tipo.conOtraForma > 0.0f && azar01(kk, ii, ciclo + 6000u) < tipo.conOtraForma)
                    forma = tipo.otraForma;
                if (tipo.gira && static_cast<int>(std::floor((edad + desfase) / 0.12)) % 2 == 1) {
                    if (forma == FormaDeParticula::Engranaje)
                        forma = FormaDeParticula::EngranajeGirado;
                    else if (forma == FormaDeParticula::Moneda)
                        forma = FormaDeParticula::MonedaDeCanto;
                }
                switch (tipo.movimiento) {
                    case MovimientoDeParticula::Salpica: {
                        // sale disparada hacia arriba y a un lado y cae: vuelve a su altura al final de su vida
                        const float g = 2.0f * std::abs(tipo.vy) / tipo.vida;
                        const float vxs = lado * tipo.vx;
                        x += vxs * t;
                        y += tipo.vy * t + 0.5f * g * t * t;
                        if (forma == FormaDeParticula::TrazoNE) {  // el trazo, hacia donde vuela ahora
                            const bool sube = tipo.vy + g * t < 0.0f;
                            forma = vxs >= 0.0f ? (sube ? FormaDeParticula::TrazoNE : FormaDeParticula::TrazoSE)
                                                : (sube ? FormaDeParticula::TrazoNO : FormaDeParticula::TrazoSO);
                        }
                        break;
                    }
                    case MovimientoDeParticula::Deriva: {
                        // de lado, siempre hacia el mismo sitio en su vida, meciendose un poco
                        const float sentido = lado < 0.0f ? -1.0f : 1.0f;
                        x += sentido * tipo.vx * t + static_cast<float>(std::sin(edad * 5.0 + i)) * 0.6f;
                        y += tipo.vy * t;
                        break;
                    }
                    case MovimientoDeParticula::Converge: {
                        // nace lejos (6-10 px) y cae hacia su pixel de la pieza
                        const double ang = azar01(kk, ii, ciclo + 4000u) * 2.0 * kPi;
                        const float r = (6.0f + 4.0f * azar01(kk, ii, ciclo + 5000u)) * vivo;
                        x += r * static_cast<float>(std::cos(ang));
                        y += r * static_cast<float>(std::sin(ang));
                        break;
                    }
                    case MovimientoDeParticula::Orbita: {
                        // gira alrededor del centro de la pieza, en un anillo aplastado (perspectiva)
                        const double ang = azar01(kk, ii, ciclo + 4000u) * 2.0 * kPi + edad * tipo.vx * 0.5;
                        const float r = 5.0f + 3.0f * azar01(kk, ii, ciclo + 5000u);
                        x = cx + r * static_cast<float>(std::cos(ang));
                        y = cy + r * 0.5f * static_cast<float>(std::sin(ang)) + tipo.vy * t;
                        break;
                    }
                    case MovimientoDeParticula::Expande: {
                        // un aro que se abre desde su pixel: unos puntos en circulo
                        const float r = 1.0f + 6.0f * (1.0f - vivo);
                        for (int k2 = 0; k2 < 10; ++k2) {
                            const double ang = k2 * 2.0 * kPi / 10.0;
                            out.push_back({x + r * static_cast<float>(std::cos(ang)),
                                           y + r * 0.6f * static_cast<float>(std::sin(ang)), alfa * 0.8f,
                                           FormaDeParticula::Punto, suma, color});
                        }
                        continue;
                    }
                    default:  // sube, cae o salta: lo de siempre
                        x += tipo.vx * static_cast<float>(std::sin(edad * 3.0 + i)) * 0.4f +
                             lado * tipo.vx * t * 0.5f;
                        y += tipo.vy * t;
                        break;
                }
                out.push_back({x, y, alfa, forma, suma, color});
            }
        }
    }
}

}  // namespace Game
