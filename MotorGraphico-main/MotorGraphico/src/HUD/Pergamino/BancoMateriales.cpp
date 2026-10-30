// PIXELLAB-01 — ver include/HUD/Pergamino/BancoMateriales.h.
#include "HUD/Pergamino/BancoMateriales.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Core/Imagen/ImagenRGBA.h"
#include "Core/Json/JsonValue.h"

namespace Pergamino {

namespace {

struct NombreMaterial {
    const char* nombre;
    Material tipo;
};

const NombreMaterial kNombres[] = {
    {"hoja", Material::Hoja},         {"papiro", Material::Papiro},   {"cuero", Material::Cuero},
    {"pestana", Material::Pestana},   {"adorno", Material::Adorno},   {"sello", Material::Sello},
    {"chincheta", Material::Chincheta}, {"cinta", Material::Cinta},   {"lomo", Material::Lomo},
    {"tachuela", Material::Tachuela}, {"flecha", Material::Flecha},
};

bool materialPorNombre(const std::string& nombre, Material& tipo) {
    for (const NombreMaterial& n : kNombres)
        if (nombre == n.nombre) {
            tipo = n.tipo;
            return true;
        }
    return false;
}

bool esPiezaFija(Material tipo) {
    int w = 0, h = 0;
    medidaPieza(tipo, w, h);
    return w > 0;
}

// Copia un bloque de la piel repitiendolo hasta llenar el destino.
void repite(const Imagen& piel, int sx, int sy, int sw, int sh, Imagen& dst, int dx, int dy, int dw, int dh) {
    if (sw <= 0 || sh <= 0) return;
    for (int y = 0; y < dh; ++y)
        for (int x = 0; x < dw; ++x) {
            const std::uint8_t* s = piel.pixel(sx + x % sw, sy + y % sh);
            std::copy(s, s + 4, dst.pixel(dx + x, dy + y));
        }
}

void oscurece(Imagen& img, int oscuro100) {
    if (oscuro100 <= 0) return;
    // La hoja procedimental suma oscuro/100 al valor del papel (hacia la
    // sombra de la paleta); aqui, un multiplicador equivalente.
    const int factor = std::max(0, 256 - oscuro100 * 160 / 100);
    for (int y = 0; y < img.alto; ++y)
        for (int x = 0; x < img.ancho; ++x) {
            std::uint8_t* p = img.pixel(x, y);
            for (int k = 0; k < 3; ++k) p[k] = static_cast<std::uint8_t>(p[k] * factor / 256);
        }
}

}  // namespace

std::vector<std::string> BancoMateriales::nombresParametros(Material tipo) {
    switch (tipo) {
        case Material::Hoja: return {"rasgado", "filete", "manchas", "oscuro"};
        case Material::Cuero: return {"costura", "remaches", "esquinas"};
        case Material::Pestana: return {"activa"};
        default: return {};
    }
}

Result<bool> BancoMateriales::carga(const std::string& carpeta) {
    m_pieles.clear();
    m_avisos.clear();
    m_cargado = false;
    const std::string ruta = carpeta + "/materiales.json";
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) return Result<bool>::Ok(false);  // sin pieles: todo procedimental
    std::stringstream texto;
    texto << entrada.rdbuf();
    auto leido = JsonValue::parse(texto.str());
    if (!leido.isOk()) return Result<bool>::Error("BancoMateriales: " + ruta + ": " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    if (!raiz["pieles"].isArray()) return Result<bool>::Error("BancoMateriales: " + ruta + " sin «pieles»");
    for (std::size_t i = 0; i < raiz["pieles"].size(); ++i) {
        auto r = cargaPiel(carpeta, raiz["pieles"][i]);
        if (!r.isOk()) m_avisos.push_back(r.errorMessage());
    }
    m_cargado = true;
    return Result<bool>::Ok(true);
}

Result<bool> BancoMateriales::cargaPiel(const std::string& carpeta, const JsonValue& datos) {
    Piel piel;
    const std::string nombre = datos["material"].asString();
    if (!materialPorNombre(nombre, piel.tipo)) return Result<bool>::Error("piel con material desconocido «" + nombre + "»");
    for (const std::string& p : nombresParametros(piel.tipo))
        if (datos.has(p)) piel.parametros[p] = datos[p].isBool() ? (datos[p].asBool() ? 1 : 0) : datos[p].asInt(0);
    std::vector<std::string> rutas;
    if (datos["imagenes"].isArray())
        for (std::size_t i = 0; i < datos["imagenes"].size(); ++i) rutas.push_back(datos["imagenes"][i].asString());
    else if (!datos["imagen"].asString().empty())
        rutas.push_back(datos["imagen"].asString());
    if (rutas.empty()) return Result<bool>::Error("piel de " + nombre + " sin «imagen»");
    int wFijo = 0, hFijo = 0;
    medidaPieza(piel.tipo, wFijo, hFijo);
    for (const std::string& rel : rutas) {
        auto png = imagen::cargarPNG(carpeta + "/" + rel);
        if (!png.isOk()) return Result<bool>::Error("piel de " + nombre + ": " + png.errorMessage());
        const imagen::ImagenRGBA& src = png.value();
        if (wFijo > 0 && (src.ancho != wFijo || src.alto != hFijo))
            return Result<bool>::Error("piel de " + nombre + ": " + rel + " mide " + std::to_string(src.ancho) + "x" +
                                       std::to_string(src.alto) + " y la pieza es de " + std::to_string(wFijo) + "x" +
                                       std::to_string(hFijo));
        Imagen img;
        img.crea(src.ancho, src.alto);
        img.rgba = src.pixeles;
        piel.imagenes.push_back(std::move(img));
    }
    if (datos["cortes"].isArray() && datos["cortes"].size() == 4)
        for (std::size_t i = 0; i < 4; ++i) piel.cortes[i] = std::max(0, datos["cortes"][i].asInt(0));
    m_pieles.push_back(std::move(piel));
    return Result<bool>::Ok(true);
}

const Piel* BancoMateriales::piel(const PedidoMaterial& pedido) const {
    const std::vector<std::string> nombres = nombresParametros(pedido.tipo);
    const int valores[4] = {pedido.a, pedido.b, pedido.c, pedido.d};
    const Piel* mejor = nullptr;
    std::size_t mejorFijados = 0;
    for (const Piel& p : m_pieles) {
        if (p.tipo != pedido.tipo) continue;
        bool encaja = true;
        for (std::size_t i = 0; i < nombres.size() && encaja; ++i) {
            const auto it = p.parametros.find(nombres[i]);
            if (it != p.parametros.end() && it->second != valores[i]) encaja = false;
        }
        if (!encaja) continue;
        if (mejor == nullptr || p.parametros.size() > mejorFijados) {
            mejor = &p;
            mejorFijados = p.parametros.size();
        }
    }
    return mejor;
}

Imagen BancoMateriales::recorta(const Imagen& piel, const int cortes[4], int ancho, int alto) {
    Imagen out;
    if (piel.vacia() || ancho <= 0 || alto <= 0) return out;
    out.crea(ancho, alto);
    int arriba = cortes[0], derecha = cortes[1], abajo = cortes[2], izquierda = cortes[3];
    // Si no cabe, los cortes se encogen a partes iguales.
    if (izquierda + derecha > ancho) {
        izquierda = ancho / 2;
        derecha = ancho - izquierda;
    }
    if (arriba + abajo > alto) {
        arriba = alto / 2;
        abajo = alto - arriba;
    }
    const int cw = piel.ancho - cortes[1] - cortes[3];  // cuerpo de la piel
    const int ch = piel.alto - cortes[0] - cortes[2];
    const int dw = ancho - izquierda - derecha;  // cuerpo del destino
    const int dh = alto - arriba - abajo;
    const int px1 = piel.ancho - cortes[1], py1 = piel.alto - cortes[2];  // inicio de los bordes derecho/inferior
    const int dx1 = ancho - derecha, dy1 = alto - abajo;
    // Esquinas (recortadas si se encogieron).
    repite(piel, 0, 0, cortes[3], cortes[0], out, 0, 0, izquierda, arriba);
    repite(piel, px1, 0, cortes[1], cortes[0], out, dx1, 0, derecha, arriba);
    repite(piel, 0, py1, cortes[3], cortes[2], out, 0, dy1, izquierda, abajo);
    repite(piel, px1, py1, cortes[1], cortes[2], out, dx1, dy1, derecha, abajo);
    // Bordes y cuerpo, repetidos.
    repite(piel, cortes[3], 0, cw, cortes[0], out, izquierda, 0, dw, arriba);
    repite(piel, cortes[3], py1, cw, cortes[2], out, izquierda, dy1, dw, abajo);
    repite(piel, 0, cortes[0], cortes[3], ch, out, 0, arriba, izquierda, dh);
    repite(piel, px1, cortes[0], cortes[1], ch, out, dx1, arriba, derecha, dh);
    repite(piel, cortes[3], cortes[0], cw, ch, out, izquierda, arriba, dw, dh);
    return out;
}

Imagen BancoMateriales::fabrica(const PedidoMaterial& pedido) const {
    if (pedido.ancho <= 0 || pedido.alto <= 0) return {};
    const Piel* p = piel(pedido);
    if (p == nullptr || p->imagenes.empty()) return Pergamino::fabrica(pedido);
    const Imagen& fuente = p->imagenes[static_cast<std::size_t>(std::max(0, pedido.semilla)) % p->imagenes.size()];
    Imagen img = esPiezaFija(pedido.tipo) ? fuente : recorta(fuente, p->cortes, pedido.ancho, pedido.alto);
    if (pedido.tipo == Material::Hoja && p->parametros.count("oscuro") == 0) oscurece(img, pedido.d);
    return img;
}

}  // namespace Pergamino
