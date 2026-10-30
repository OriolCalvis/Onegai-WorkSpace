// PIXELLAB-01 — test_pergamino (3/3): el banco de materiales con pieles.
//
// Sin pieles, el banco devuelve lo mismo que fabrica(); con pieles, el
// 9-slice mide lo pedido, conserva las esquinas y repite bordes y cuerpo sin
// estirar; la piel mas concreta gana; una piel rota avisa y no tumba.
// Las pieles de prueba se escriben aqui como PNG (sin comprimir) en una
// carpeta temporal: no hace falta ningun fichero del arbol.
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "HUD/Pergamino/BancoMateriales.h"
#include "HUD/Pergamino/Materiales.h"
#include "TestHarness.h"

using namespace Pergamino;

namespace {

std::uint32_t crc32(const std::uint8_t* d, std::size_t n, std::uint32_t c = 0xFFFFFFFFu) {
    for (std::size_t i = 0; i < n; ++i) {
        c ^= d[i];
        for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return c;
}

void be32(std::vector<std::uint8_t>& v, std::uint32_t x) {
    for (int i = 3; i >= 0; --i) v.push_back(static_cast<std::uint8_t>(x >> (8 * i)));
}

void chunk(std::vector<std::uint8_t>& png, const char* tipo, const std::vector<std::uint8_t>& datos) {
    be32(png, static_cast<std::uint32_t>(datos.size()));
    std::vector<std::uint8_t> cuerpo(tipo, tipo + 4);
    cuerpo.insert(cuerpo.end(), datos.begin(), datos.end());
    png.insert(png.end(), cuerpo.begin(), cuerpo.end());
    be32(png, crc32(cuerpo.data(), cuerpo.size()) ^ 0xFFFFFFFFu);
}

// PNG RGBA de 8 bits con bloques deflate «almacenados» (sin comprimir).
void escribePNG(const std::filesystem::path& ruta, const Imagen& img) {
    std::vector<std::uint8_t> png = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    std::vector<std::uint8_t> ihdr;
    be32(ihdr, static_cast<std::uint32_t>(img.ancho));
    be32(ihdr, static_cast<std::uint32_t>(img.alto));
    ihdr.insert(ihdr.end(), {8, 6, 0, 0, 0});
    chunk(png, "IHDR", ihdr);
    std::vector<std::uint8_t> crudo;
    for (int y = 0; y < img.alto; ++y) {
        crudo.push_back(0);  // filtro none
        const std::uint8_t* fila = img.pixel(0, y);
        crudo.insert(crudo.end(), fila, fila + static_cast<std::size_t>(img.ancho) * 4);
    }
    std::vector<std::uint8_t> z = {0x78, 0x01};
    std::size_t pos = 0;
    while (pos < crudo.size() || pos == 0) {
        const std::size_t n = std::min<std::size_t>(65535, crudo.size() - pos);
        z.push_back(pos + n >= crudo.size() ? 1 : 0);
        z.push_back(static_cast<std::uint8_t>(n & 0xFF));
        z.push_back(static_cast<std::uint8_t>(n >> 8));
        z.push_back(static_cast<std::uint8_t>(~n & 0xFF));
        z.push_back(static_cast<std::uint8_t>((~n >> 8) & 0xFF));
        z.insert(z.end(), crudo.begin() + static_cast<long>(pos), crudo.begin() + static_cast<long>(pos + n));
        pos += n;
        if (n == 0) break;
    }
    std::uint32_t a = 1, b = 0;
    for (std::uint8_t c : crudo) {
        a = (a + c) % 65521;
        b = (b + a) % 65521;
    }
    be32(z, (b << 16) | a);
    chunk(png, "IDAT", z);
    chunk(png, "IEND", {});
    std::ofstream(ruta, std::ios::binary).write(reinterpret_cast<const char*>(png.data()), static_cast<long>(png.size()));
}

// Una piel de 12x10 con esquinas de 3 px de colores distintos, bordes a rayas
// y cuerpo en damero: cualquier estiramiento o corte mal puesto se nota.
Imagen pielPrueba() {
    Imagen img;
    img.crea(12, 10);
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 12; ++x) {
            std::uint8_t* p = img.pixel(x, y);
            const bool ex = x < 3 || x >= 9, ey = y < 3 || y >= 7;
            if (ex && ey) {  // esquina: color propio
                p[0] = x < 3 ? 200 : 100;
                p[1] = y < 3 ? 200 : 100;
                p[2] = 50;
            } else if (ey) {  // borde horizontal: rayas verticales de periodo 3
                p[0] = p[1] = p[2] = static_cast<std::uint8_t>(((x - 3) % 3) * 60 + 40);
            } else if (ex) {  // borde vertical: rayas horizontales de periodo 2
                p[0] = p[1] = p[2] = static_cast<std::uint8_t>(((y - 3) % 2) * 100 + 80);
            } else {  // cuerpo: damero
                p[0] = p[1] = p[2] = static_cast<std::uint8_t>(((x + y) & 1) ? 220 : 120);
            }
            p[3] = 255;
        }
    return img;
}

bool mismoPixel(const Imagen& a, int ax, int ay, const Imagen& b, int bx, int by) {
    const std::uint8_t* p = a.pixel(ax, ay);
    const std::uint8_t* q = b.pixel(bx, by);
    return p[0] == q[0] && p[1] == q[1] && p[2] == q[2] && p[3] == q[3];
}

struct CarpetaTemporal {
    std::filesystem::path raiz;
    CarpetaTemporal() {
        raiz = std::filesystem::temp_directory_path() / "banco_materiales_test";
        std::filesystem::remove_all(raiz);
        std::filesystem::create_directories(raiz);
    }
    ~CarpetaTemporal() { std::filesystem::remove_all(raiz); }
    void escribe(const std::string& nombre, const std::string& texto) const { std::ofstream(raiz / nombre) << texto; }
    std::string ruta() const { return raiz.string(); }
};

}  // namespace

MOTOR_TEST_CASE(sin_pieles_es_el_procedimental) {
    BancoMateriales banco;
    const auto r = banco.carga("/no/existe");
    MOTOR_CHECK(r.isOk() && !r.value());
    MOTOR_CHECK(!banco.cargado() && banco.pieles() == 0);
    const PedidoMaterial pedido = hoja(120, 60, 7);
    const Imagen a = banco.fabrica(pedido);
    const Imagen b = Pergamino::fabrica(pedido);
    MOTOR_CHECK(a.ancho == 120 && a.alto == 60 && a.rgba == b.rgba);
    MOTOR_CHECK(banco.piel(pedido) == nullptr);
    MOTOR_CHECK(banco.fabrica(hoja(0, 10, 1)).vacia());
}

MOTOR_TEST_CASE(recorta_nueve_trozos_sin_estirar) {
    const Imagen piel = pielPrueba();
    const int cortes[4] = {3, 3, 3, 3};
    const Imagen out = BancoMateriales::recorta(piel, cortes, 40, 22);
    MOTOR_CHECK(out.ancho == 40 && out.alto == 22);
    // Esquinas intactas.
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x) {
            MOTOR_CHECK(mismoPixel(out, x, y, piel, x, y));
            MOTOR_CHECK(mismoPixel(out, 37 + x, y, piel, 9 + x, y));
            MOTOR_CHECK(mismoPixel(out, x, 19 + y, piel, x, 7 + y));
            MOTOR_CHECK(mismoPixel(out, 37 + x, 19 + y, piel, 9 + x, 7 + y));
        }
    // Borde superior: rayas de periodo 3 repetidas (no estiradas).
    for (int x = 3; x < 37; ++x) MOTOR_CHECK(mismoPixel(out, x, 1, piel, 3 + (x - 3) % 3, 1));
    // Borde izquierdo: periodo 2.
    for (int y = 3; y < 19; ++y) MOTOR_CHECK(mismoPixel(out, 1, y, piel, 1, 3 + (y - 3) % 2));
    // Cuerpo: damero continuo.
    for (int y = 3; y < 19; ++y)
        for (int x = 3; x < 37; ++x) MOTOR_CHECK(mismoPixel(out, x, y, piel, 3 + (x - 3) % 6, 3 + (y - 3) % 4));
    // Mas pequeno que los cortes: no revienta y las esquinas se encogen.
    const Imagen chica = BancoMateriales::recorta(piel, cortes, 4, 4);
    MOTOR_CHECK(chica.ancho == 4 && chica.alto == 4);
    MOTOR_CHECK(mismoPixel(chica, 0, 0, piel, 0, 0) && mismoPixel(chica, 3, 3, piel, 11, 9));
    MOTOR_CHECK(BancoMateriales::recorta(piel, cortes, 0, 5).vacia());
}

MOTOR_TEST_CASE(pieles_por_material_parametros_y_semilla) {
    CarpetaTemporal c;
    escribePNG(c.raiz / "hoja.png", pielPrueba());
    Imagen otra = pielPrueba();
    otra.pixel(5, 5)[0] = 7;  // distinta a la primera
    escribePNG(c.raiz / "hoja_b.png", otra);
    Imagen activa;
    activa.crea(8, 8);
    for (int i = 0; i < 8 * 8 * 4; ++i) activa.rgba[static_cast<std::size_t>(i)] = 255;
    escribePNG(c.raiz / "pestana_activa.png", activa);
    Imagen sello;
    sello.crea(24, 24);
    for (int i = 0; i < 24 * 24 * 4; ++i) sello.rgba[static_cast<std::size_t>(i)] = 9;
    escribePNG(c.raiz / "sello.png", sello);
    c.escribe("materiales.json", R"({"pieles": [
        {"material": "hoja", "imagenes": ["hoja.png", "hoja_b.png"], "cortes": [3, 3, 3, 3]},
        {"material": "hoja", "filete": 6, "imagen": "hoja.png", "cortes": [3, 3, 3, 3], "oscuro": 22},
        {"material": "pestana", "activa": true, "imagen": "pestana_activa.png", "cortes": [2, 2, 2, 2]},
        {"material": "sello", "imagen": "sello.png"}]})");
    BancoMateriales banco;
    const auto r = banco.carga(c.ruta());
    for (const std::string& a : banco.avisos()) std::printf("  aviso: %s\n", a.c_str());
    MOTOR_CHECK(r.isOk() && r.value() && banco.cargado() && banco.pieles() == 4 && banco.avisos().empty());
    // La hoja generica sirve a cualquier hoja; la semilla elige la variante.
    const Imagen h1 = banco.fabrica(hoja(60, 30, 2));
    const Imagen h2 = banco.fabrica(hoja(60, 30, 3));
    MOTOR_CHECK(h1.ancho == 60 && h1.alto == 30 && h2.ancho == 60);
    MOTOR_CHECK(mismoPixel(h1, 5, 5, pielPrueba(), 5, 5) && !mismoPixel(h2, 5, 5, pielPrueba(), 5, 5));
    MOTOR_CHECK(mismoPixel(h2, 5, 5, otra, 5, 5));
    // La hoja con filete=6 y oscuro=22 solo sirve a los pedidos que coinciden
    // en los dos, y entonces gana por concreta; si no, la generica.
    const Piel* concreta = banco.piel(hoja(60, 30, 2, 1, 6, 3, 22));
    MOTOR_CHECK(concreta != nullptr && concreta->parametros.size() == 2 && concreta->parametros.count("oscuro") == 1);
    MOTOR_CHECK(banco.piel(hoja(60, 30, 2, 1, 6))->parametros.empty());         // oscuro 0 != 22
    MOTOR_CHECK(banco.piel(hoja(60, 30, 2, 1, 0, 3, 22))->parametros.empty());  // filete 0 != 6
    MOTOR_CHECK(banco.piel(hoja(60, 30, 2, 1, 6, 3, 50))->parametros.empty());  // oscuro 50 != 22
    // Una hoja pedida mas oscura sobre la piel generica sale mas oscura.
    const Imagen clara = banco.fabrica(hoja(60, 30, 2, 1, 0, 3, 0));
    const Imagen oscura = banco.fabrica(hoja(60, 30, 2, 1, 0, 3, 22));
    MOTOR_CHECK(oscura.pixel(10, 10)[0] < clara.pixel(10, 10)[0]);
    MOTOR_CHECK(oscura.pixel(10, 10)[3] == clara.pixel(10, 10)[3]);
    // Pestana: solo la activa tiene piel; la inactiva es procedimental.
    MOTOR_CHECK(banco.piel(pestana(40, 20, true, 1)) != nullptr);
    MOTOR_CHECK(banco.piel(pestana(40, 20, false, 1)) == nullptr);
    MOTOR_CHECK(banco.fabrica(pestana(40, 20, true, 1)).pixel(20, 10)[0] == 255);
    // Pieza fija: la imagen tal cual.
    const Imagen s = banco.fabrica(pieza(Material::Sello));
    MOTOR_CHECK(s.ancho == 24 && s.alto == 24 && s.pixel(3, 3)[2] == 9);
    // Lo que no tiene piel sigue siendo procedimental.
    MOTOR_CHECK(banco.piel(papiro(200, 300)) == nullptr);
    MOTOR_CHECK(banco.fabrica(papiro(200, 300)).rgba == Pergamino::fabrica(papiro(200, 300)).rgba);
}

MOTOR_TEST_CASE(piel_rota_avisa_y_no_tumba) {
    CarpetaTemporal c;
    Imagen mal;
    mal.crea(20, 20);
    escribePNG(c.raiz / "sello_mal.png", mal);
    c.escribe("materiales.json", R"({"pieles": [
        {"material": "nada", "imagen": "x.png"},
        {"material": "hoja"},
        {"material": "cuero", "imagen": "no_existe.png"},
        {"material": "sello", "imagen": "sello_mal.png"}]})");
    BancoMateriales banco;
    MOTOR_CHECK(banco.carga(c.ruta()).isOk());
    MOTOR_CHECK(banco.cargado() && banco.pieles() == 0 && banco.avisos().size() == 4);
    MOTOR_CHECK(banco.fabrica(pieza(Material::Sello)).rgba == Pergamino::fabrica(pieza(Material::Sello)).rgba);
    c.escribe("materiales.json", "{esto no es json");
    MOTOR_CHECK(!banco.carga(c.ruta()).isOk());
    MOTOR_CHECK(!banco.cargado() && banco.pieles() == 0);
}
