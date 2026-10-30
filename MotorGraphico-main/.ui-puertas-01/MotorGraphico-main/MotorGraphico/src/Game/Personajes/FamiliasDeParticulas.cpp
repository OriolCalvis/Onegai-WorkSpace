// PART-01 — ver include/Game/Personajes/FamiliasDeParticulas.h.
#include "Game/Personajes/FamiliasDeParticulas.h"

#include <algorithm>
#include <map>

namespace Game {

namespace {
// PART-01: las familias de tools/particulas.py (la lista la comprueba test_validar_sprites).
const std::map<std::string, TipoDeParticula>& familias() {
    using F = FormaDeParticula;
    using M = MovimientoDeParticula;
    static const std::map<std::string, TipoDeParticula> kTipos = {
        {"llamas", {-16.0f, 3.0f, 0.9f, F::Llama, true, M::Sube,
          {{0xff, 0xf2, 0xa8}, {0xff, 0xb0, 0x40}, {0xff, 0x6a, 0x2a}}}},
        {"agua", {6.0f, 1.0f, 1.0f, F::Gota, true, M::Cae, {}}},
        {"escarcha", {5.0f, 3.0f, 1.5f, F::Copo, true, M::Cae,
          {{0xff, 0xff, 0xff}, {0xbf, 0xf0, 0xff}, {0x8a, 0xd8, 0xff}}}},
        {"rayos", {0.0f, 0.0f, 0.25f, F::Rayo, true, M::Chispazo, {}}},
        {"piedras", {14.0f, 2.0f, 0.8f, F::Piedra, false, M::Cae,
          {{0xc8, 0x9a, 0x5a}, {0x8a, 0x6a, 0x40}, {0xe6, 0xc6, 0x90}}}},
        {"viento", {-2.0f, 14.0f, 1.1f, F::Estela, true, M::Deriva, {{0xf4, 0xff, 0xf8}, {0xd8, 0xf5, 0xe6}}}},
        {"luz", {-10.0f, 1.0f, 1.3f, F::Punto, true, M::Sube, {}}},
        {"sombra", {-7.0f, 3.0f, 1.5f, F::Voluta, false, M::Sube, {}}},
        {"hojas", {8.0f, 6.0f, 1.6f, F::Hoja, false, M::Cae, {}}},
        {"veneno", {-6.0f, 3.0f, 1.6f, F::Calavera, false, M::Sube, {{0xb4, 0xf0, 0x64}, {0x8c, 0xd2, 0x3c}, {0x64, 0xaa, 0x28}},
                    0.6f}},
        {"sangre", {9.0f, 0.5f, 0.9f, F::Gota, false, M::Cae, {{0xd2, 0x28, 0x28}, {0x8a, 0x10, 0x18}}}},
        {"arcano", {-2.0f, 5.0f, 1.6f, F::Runa, true, M::Orbita,
          {{0xe6, 0xb4, 0xff}, {0xb4, 0x8c, 0xff}, {0xff, 0xb4, 0xe6}}}},
        {"vacio", {0.0f, 0.0f, 1.2f, F::Cruz, false, M::Converge,
          {{0x3c, 0x1e, 0x5a}, {0x5a, 0x3c, 0x8c}, {0x8a, 0x5a, 0xc8}}}},
        {"brasas", {-14.0f, 4.0f, 1.1f, F::Punto, true, M::Sube, {}}},
        {"ceniza", {-5.0f, 3.0f, 1.6f, F::Punto, false, M::Sube, {}}},
        {"chispas", {-24.0f, 10.0f, 0.8f, F::TrazoNE, true, M::Salpica, {}}},
        {"estrellas", {-3.0f, 1.0f, 1.0f, F::Estrella, true, M::Sube, {}}},
        {"llamas_puras", {-18.0f, 2.0f, 0.8f, F::Llama, true, M::Sube,
          {{0xff, 0xff, 0xff}, {0xff, 0xf7, 0xc8}, {0xff, 0xe0, 0x70}}}},
        {"dragones", {-12.0f, 6.0f, 1.4f, F::Dragon, true, M::Sube,
          {{0xff, 0xd2, 0x5a}, {0xff, 0x7a, 0x3a}, {0xe8, 0x48, 0x3a}}}},
        {"petalos", {5.0f, 7.0f, 1.8f, F::Petalo, true, M::Cae,
          {{0xff, 0xd2, 0xe6}, {0xff, 0xff, 0xff}, {0xff, 0xa0, 0xc8}}}},
        {"furia", {-22.0f, 1.0f, 0.6f, F::EstelaVertical, true, M::Sube,
          {{0xff, 0x5a, 0x3a}, {0xff, 0x9a, 0x3a}, {0xc8, 0x28, 0x1e}}}},
        {"oro", {-4.0f, 1.0f, 1.2f, F::Estrella, true, M::Sube,
          {{0xff, 0xf6, 0xc8}, {0xff, 0xd2, 0x3c}, {0xf5, 0xa5, 0x1e}}, 2.0f}},
        {"notas", {-9.0f, 4.0f, 1.6f, F::Nota, true, M::Sube, {}}},
        {"ondas", {0.0f, 0.0f, 0.9f, F::Punto, true, M::Expande, {}}},
        {"naipes", {-4.0f, 8.0f, 1.4f, F::Naipe, false, M::Deriva,
          {{0xff, 0xff, 0xff}, {0xc8, 0x32, 0x32}, {0x28, 0x28, 0x3c}}}},
        {"monedas", {12.0f, 2.0f, 1.0f, F::Moneda, true, M::Cae, {{0xff, 0xe6, 0x8a}, {0xe6, 0xb4, 0x32}}, 1.0f,
                     F::Estrella, 0.4f, true}},
        {"engranajes", {7.0f, 2.0f, 1.2f, F::Engranaje, false, M::Cae, {{0xc8, 0xc8, 0xc8}, {0xb4, 0x8c, 0x3c}}, 1.0f,
                        F::Punto, 0.0f, true}},
        {"humo", {-5.0f, 4.0f, 1.8f, F::Voluta, false, M::Sube, {{0xe6, 0xe6, 0xe6}, {0xa0, 0xa0, 0xa8}}}},
        {"polvora", {-10.0f, 6.0f, 0.6f, F::Punto, true, M::Sube, {{0xff, 0xe6, 0x8a}, {0xff, 0x9a, 0x3a}}}},
        {"alquimia", {-7.0f, 3.0f, 1.4f, F::Burbuja, true, M::Sube,
          {{0x78, 0xf0, 0xa0}, {0xff, 0x7a, 0xb4}, {0x7a, 0xb4, 0xff}, {0xff, 0xe6, 0x6e}}}},
        {"plumas", {4.0f, 8.0f, 2.0f, F::Pluma, false, M::Cae, {{0xff, 0xff, 0xff}, {0xe6, 0xe0, 0xd2}}}},
        {"murcielagos", {-8.0f, 12.0f, 1.2f, F::Murcielago, false, M::Deriva,
          {{0x2a, 0x1e, 0x32}, {0x4a, 0x2a, 0x46}}}},
        {"animas", {-6.0f, 5.0f, 2.0f, F::Voluta, true, M::Sube, {{0xc8, 0xff, 0xc8}, {0x8a, 0xe6, 0xa0}}}},
        {"espiritus", {-3.0f, 6.0f, 1.8f, F::Voluta, true, M::Orbita, {{0xb4, 0xe6, 0xff}, {0x78, 0xc8, 0xff}}}},
        {"esporas", {-3.0f, 5.0f, 2.2f, F::Punto, true, M::Deriva, {{0xe6, 0xf0, 0xa0}, {0xc8, 0xdc, 0x78}}}},
        {"ki", {-24.0f, 0.5f, 0.5f, F::EstelaVertical, true, M::Sube, {{0xff, 0xff, 0xff}, {0xc8, 0xf0, 0xff}}}},
        {"grano", {10.0f, 3.0f, 1.0f, F::Punto, true, M::Cae, {{0xff, 0xf0, 0xa0}, {0xe6, 0xc8, 0x5a}}}},
        {"confeti", {8.0f, 9.0f, 1.6f, F::Naipe, false, M::Cae,
          {{0xff, 0x5a, 0x5a}, {0x5a, 0xb4, 0xff}, {0xff, 0xe6, 0x5a}, {0x78, 0xe6, 0x78}}}},
        {"polvo", {-2.0f, 9.0f, 1.2f, F::Voluta, false, M::Deriva, {{0xc8, 0xaa, 0x82}, {0xa0, 0x86, 0x64}}}},
        {"elementos", {-2.0f, 6.0f, 1.6f, F::Cruz, true, M::Orbita,
          {{0xff, 0x8c, 0x3c}, {0x5a, 0xb4, 0xff}, {0xc8, 0x9a, 0x5a}, {0xd8, 0xf5, 0xe6}}}},
    };
    return kTipos;
}
}  // namespace

TipoDeParticula tipoDeParticula(const std::string& nombre) {
    const auto& tipos = familias();
    const auto it = tipos.find(nombre);
    return it == tipos.end() ? tipos.at("chispas") : it->second;
}

std::vector<std::string> familiasDeParticulas() {
    std::vector<std::string> out;
    for (const auto& [nombre, tipo] : familias()) {
        out.push_back(nombre);
        (void)tipo;
    }
    return out;
}

const std::vector<PixelDeForma>& pixelesDeForma(FormaDeParticula forma) {
    static const std::vector<PixelDeForma> kPunto = {{0, 0, 1.0f}};
    static const std::vector<PixelDeForma> kCruz = {
        {0, 0, 1.0f}, {1, 0, 0.45f}, {-1, 0, 0.45f}, {0, 1, 0.45f}, {0, -1, 0.45f}};
    static const std::vector<PixelDeForma> kBurbuja = {
        {0, 0, 0.5f}, {1, 0, 0.8f}, {-1, 0, 0.8f}, {0, 1, 0.8f}, {0, -1, 0.8f}};
    static const std::vector<PixelDeForma> kVoluta = {{0, 0, 1.0f}, {1, 0, 0.6f}, {0, -1, 0.6f}, {1, -1, 0.4f}};
    static const std::vector<PixelDeForma> kHoja = {{0, 0, 1.0f}, {1, 0, 0.7f}};
    static const std::vector<PixelDeForma> kRayo = {
        {0, 0, 1.0f}, {-1, 1, 1.0f}, {0, 2, 1.0f}, {-1, 3, 1.0f}, {0, 4, 0.8f}, {1, -1, 0.8f}};
    static const std::vector<PixelDeForma> kGota = {{0, 0, 1.0f}, {0, -1, 0.5f}};
    // PART-01
    // una lengua de fuego: ancha abajo y la punta arriba
    static const std::vector<PixelDeForma> kLlama = {
        {0, 0, 1.0f}, {-1, 0, 0.5f}, {1, 0, 0.5f}, {0, -1, 0.9f}, {1, -1, 0.3f}, {0, -2, 0.55f}};
    static const std::vector<PixelDeForma> kCopo = {{0, 0, 1.0f},  {1, 0, 0.5f},   {-1, 0, 0.5f},
                                                    {0, 1, 0.5f},  {0, -1, 0.5f},  {1, 1, 0.25f},
                                                    {-1, -1, 0.25f}, {1, -1, 0.25f}, {-1, 1, 0.25f}};
    static const std::vector<PixelDeForma> kPiedra = {{0, 0, 1.0f}, {1, 0, 0.8f}, {0, 1, 0.7f}, {1, 1, 0.5f}};
    // el aire: una estela que se borra hacia atras
    static const std::vector<PixelDeForma> kEstela = {{0, 0, 1.0f}, {-1, 0, 0.7f}, {-2, 0, 0.45f}, {-3, 0, 0.2f}};
    static const std::vector<PixelDeForma> kEstelaVertical = {
        {0, 0, 1.0f}, {0, 1, 0.7f}, {0, 2, 0.45f}, {0, 3, 0.2f}};
    static const std::vector<PixelDeForma> kRuna = {
        {0, -1, 0.8f}, {1, 0, 0.8f}, {0, 1, 0.8f}, {-1, 0, 0.8f}, {0, 0, 0.3f}};
    // la corchea: la cabeza abajo a la izquierda, la plica y la bandera
    static const std::vector<PixelDeForma> kNota = {{0, 0, 1.0f},  {1, 0, 0.9f},  {1, -1, 0.8f}, {1, -2, 0.8f},
                                                    {1, -3, 0.8f}, {2, -3, 0.6f}, {2, -2, 0.4f}};
    static const std::vector<PixelDeForma> kNaipe = {
        {0, 0, 1.0f}, {1, 0, 1.0f}, {0, 1, 0.9f}, {1, 1, 0.9f}, {0, 2, 0.8f}, {1, 2, 0.8f}};
    static const std::vector<PixelDeForma> kMoneda = {{0, 0, 1.0f}, {1, 0, 0.85f}, {0, 1, 0.85f}, {1, 1, 0.6f}};
    static const std::vector<PixelDeForma> kEngranaje = {
        {-1, -1, 0.6f}, {0, -1, 0.9f}, {1, -1, 0.6f}, {-1, 0, 0.9f}, {1, 0, 0.9f}, {-1, 1, 0.6f},
        {0, 1, 0.9f},   {1, 1, 0.6f},  {0, -2, 0.5f}, {0, 2, 0.5f},  {-2, 0, 0.5f}, {2, 0, 0.5f}};
    static const std::vector<PixelDeForma> kPluma = {
        {0, 0, 1.0f}, {1, -1, 0.9f}, {2, -2, 0.7f}, {3, -3, 0.4f}, {1, 0, 0.4f}, {2, -1, 0.4f}};
    // el cuerpo y las dos alas
    static const std::vector<PixelDeForma> kMurcielago = {{0, 0, 1.0f},  {-1, -1, 0.9f}, {-2, -1, 0.7f},
                                                          {-2, 0, 0.4f}, {1, -1, 0.9f},  {2, -1, 0.7f},
                                                          {2, 0, 0.4f}};
    // un dragon diminuto: la cabeza con el cuerno arriba y el cuerpo en S que se borra hacia la cola
    static const std::vector<PixelDeForma> kDragon = {{0, 0, 1.0f},  {1, 0, 0.9f},  {1, -1, 0.7f},
                                                      {0, 1, 0.85f}, {-1, 2, 0.8f}, {-1, 3, 0.65f},
                                                      {0, 4, 0.5f},  {1, 5, 0.3f}};
    static const std::vector<PixelDeForma> kPetalo = {{0, 0, 1.0f}, {1, 0, 0.8f}, {1, 1, 0.4f}};
    // Oriol, revisando la lista. La calavera (5x5: los ojos y el hueco entre los dientes son su contorno, oscuros).
    static const std::vector<PixelDeForma> kCalavera = {
        {-1, -2, 0.9f}, {0, -2, 0.9f}, {1, -2, 0.9f}, {-2, -1, 1.0f}, {-1, -1, 1.0f}, {0, -1, 1.0f},
        {1, -1, 1.0f},  {2, -1, 1.0f}, {-2, 0, 1.0f}, {0, 0, 1.0f},   {2, 0, 1.0f},   {-1, 1, 0.9f},
        {0, 1, 0.9f},   {1, 1, 0.9f},  {-1, 2, 0.8f}, {1, 2, 0.8f}};
    // la estrella de cuatro puntas de siempre: las puntas se apagan
    static const std::vector<PixelDeForma> kEstrella = {{0, 0, 1.0f},   {1, 0, 0.75f},  {-1, 0, 0.75f},
                                                        {0, 1, 0.75f},  {0, -1, 0.75f}, {2, 0, 0.35f},
                                                        {-2, 0, 0.35f}, {0, 2, 0.35f},  {0, -2, 0.35f}};
    // el trazo de una chispa: la punta brilla y la cola, detras de hacia donde vuela, se apaga
    static const std::vector<PixelDeForma> kTrazoNE = {{0, 0, 1.0f}, {-1, 1, 0.6f}, {-2, 2, 0.3f}};
    static const std::vector<PixelDeForma> kTrazoNO = {{0, 0, 1.0f}, {1, 1, 0.6f}, {2, 2, 0.3f}};
    static const std::vector<PixelDeForma> kTrazoSE = {{0, 0, 1.0f}, {-1, -1, 0.6f}, {-2, -2, 0.3f}};
    static const std::vector<PixelDeForma> kTrazoSO = {{0, 0, 1.0f}, {1, -1, 0.6f}, {2, -2, 0.3f}};
    // el engranaje girado un octavo de vuelta: los dientes en las esquinas
    static const std::vector<PixelDeForma> kEngranajeGirado = {
        {-1, -1, 0.9f}, {0, -1, 0.6f}, {1, -1, 0.9f}, {-1, 0, 0.6f},  {1, 0, 0.6f},  {-1, 1, 0.9f},
        {0, 1, 0.6f},   {1, 1, 0.9f},  {-2, -2, 0.5f}, {2, -2, 0.5f}, {-2, 2, 0.5f}, {2, 2, 0.5f}};
    // la moneda de canto, al girar
    static const std::vector<PixelDeForma> kMonedaDeCanto = {{0, 0, 1.0f}, {0, 1, 0.85f}};
    switch (forma) {
        case FormaDeParticula::Punto: return kPunto;
        case FormaDeParticula::Cruz: return kCruz;
        case FormaDeParticula::Burbuja: return kBurbuja;
        case FormaDeParticula::Voluta: return kVoluta;
        case FormaDeParticula::Hoja: return kHoja;
        case FormaDeParticula::Rayo: return kRayo;
        case FormaDeParticula::Gota: return kGota;
        case FormaDeParticula::Llama: return kLlama;
        case FormaDeParticula::Copo: return kCopo;
        case FormaDeParticula::Piedra: return kPiedra;
        case FormaDeParticula::Estela: return kEstela;
        case FormaDeParticula::EstelaVertical: return kEstelaVertical;
        case FormaDeParticula::Runa: return kRuna;
        case FormaDeParticula::Nota: return kNota;
        case FormaDeParticula::Naipe: return kNaipe;
        case FormaDeParticula::Moneda: return kMoneda;
        case FormaDeParticula::Engranaje: return kEngranaje;
        case FormaDeParticula::Pluma: return kPluma;
        case FormaDeParticula::Murcielago: return kMurcielago;
        case FormaDeParticula::Dragon: return kDragon;
        case FormaDeParticula::Petalo: return kPetalo;
        case FormaDeParticula::Calavera: return kCalavera;
        case FormaDeParticula::Estrella: return kEstrella;
        case FormaDeParticula::TrazoNE: return kTrazoNE;
        case FormaDeParticula::TrazoNO: return kTrazoNO;
        case FormaDeParticula::TrazoSE: return kTrazoSE;
        case FormaDeParticula::TrazoSO: return kTrazoSO;
        case FormaDeParticula::EngranajeGirado: return kEngranajeGirado;
        case FormaDeParticula::MonedaDeCanto: return kMonedaDeCanto;
    }
    return kPunto;
}

const std::vector<PixelDeForma>& contornoDeForma(FormaDeParticula forma) {
    static constexpr std::size_t kFormas = static_cast<std::size_t>(FormaDeParticula::MonedaDeCanto) + 1;
    static const std::array<std::vector<PixelDeForma>, kFormas> kContornos = [] {
            std::array<std::vector<PixelDeForma>, kFormas> out{};
            for (const FormaDeParticula f :
                 {FormaDeParticula::Llama, FormaDeParticula::Copo, FormaDeParticula::Runa, FormaDeParticula::Nota,
                  FormaDeParticula::Naipe, FormaDeParticula::Moneda, FormaDeParticula::Engranaje,
                  FormaDeParticula::Pluma, FormaDeParticula::Murcielago, FormaDeParticula::Dragon,
                  FormaDeParticula::Petalo, FormaDeParticula::Calavera, FormaDeParticula::EngranajeGirado,
                  FormaDeParticula::MonedaDeCanto}) {
                const auto& forma = pixelesDeForma(f);
                auto dentro = [&](int x, int y) {
                    return std::any_of(forma.begin(), forma.end(),
                                       [&](const PixelDeForma& p) { return p.dx == x && p.dy == y; });
                };
                auto& c = out[static_cast<std::size_t>(f)];
                for (const PixelDeForma& p : forma) {
                    for (const auto& [ex, ey] : {std::pair{1, 0}, std::pair{-1, 0}, std::pair{0, 1}, std::pair{0, -1}}) {
                        const int x = p.dx + ex, y = p.dy + ey;
                        const bool yaEsta = std::any_of(c.begin(), c.end(), [&](const PixelDeForma& q) {
                            return q.dx == x && q.dy == y;
                        });
                        if (!dentro(x, y) && !yaEsta)
                            c.push_back({x, y, 1.0f});
                    }
                }
            }
            return out;
        }();
    return kContornos[static_cast<std::size_t>(forma)];
}

}  // namespace Game
