// U3-06.E06b -- ver CatalogoAnimacionJuego.h
#include "Game/CatalogoAnimacionJuego.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace Game {

namespace {

std::string rotulo(const std::vector<std::string>& nombres, std::size_t i, const char* tipo) {
    return i < nombres.size() ? nombres[i] : std::string(tipo) + "[" + std::to_string(i) + "]";
}

// Lee y parsea un fichero; ante fallo avisa y devuelve false.
bool leeDocumento(const std::string& ruta, JsonValue& salida, std::ostream& avisos) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) {
        avisos << "animacion: no se puede abrir " << ruta << "\n";
        return false;
    }
    std::ostringstream texto;
    texto << f.rdbuf();
    auto json = JsonValue::parse(texto.str());
    if (!json.isOk()) {
        avisos << "animacion: " << ruta << ": " << json.errorMessage() << "\n";
        return false;
    }
    salida = json.value();
    return true;
}

CatalogoAnimacion cargaDelJuego() {
    std::vector<JsonValue> clips, estados;
    std::vector<std::string> nClips, nEstados;
    for (const char* ruta : kRutasClipsDelJuego) {
        JsonValue doc;
        if (leeDocumento(ruta, doc, std::cerr)) {
            clips.push_back(std::move(doc));
            nClips.emplace_back(ruta);
        }
    }
    for (const char* ruta : kRutasEstadosDelJuego) {
        JsonValue doc;
        if (leeDocumento(ruta, doc, std::cerr)) {
            estados.push_back(std::move(doc));
            nEstados.emplace_back(ruta);
        }
    }
    return construyeCatalogoTolerante(clips, estados, std::cerr, nClips, nEstados);
}

}  // namespace

CatalogoAnimacion construyeCatalogoTolerante(const std::vector<JsonValue>& docsClips,
                                             const std::vector<JsonValue>& docsEstados,
                                             std::ostream& avisos,
                                             const std::vector<std::string>& nombresClips,
                                             const std::vector<std::string>& nombresEstados) {
    // Primero los clips, documento a documento: el que rompe se descarta con un aviso.
    std::vector<JsonValue> clipsBuenos;
    for (std::size_t i = 0; i < docsClips.size(); ++i) {
        std::vector<JsonValue> prueba = clipsBuenos;
        prueba.push_back(docsClips[i]);
        auto r = construyeCatalogoAnimacion(prueba, {});
        if (r.isOk()) {
            clipsBuenos = std::move(prueba);
        } else {
            avisos << "animacion: clips descartados (" << rotulo(nombresClips, i, "clips")
                   << "): " << r.errorMessage() << "\n";
        }
    }
    // Despues las maquinas, una a una, sobre los clips buenos.
    std::vector<JsonValue> estadosBuenos;
    for (std::size_t i = 0; i < docsEstados.size(); ++i) {
        std::vector<JsonValue> prueba = estadosBuenos;
        prueba.push_back(docsEstados[i]);
        auto r = construyeCatalogoAnimacion(clipsBuenos, prueba);
        if (r.isOk()) {
            estadosBuenos = std::move(prueba);
        } else {
            avisos << "animacion: maquinas descartadas (" << rotulo(nombresEstados, i, "estados")
                   << "): " << r.errorMessage() << "\n";
        }
    }
    auto fin = construyeCatalogoAnimacion(clipsBuenos, estadosBuenos);
    if (fin.isOk()) return std::move(fin.value());
    return CatalogoAnimacion{};  // inalcanzable: cada documento ya se probo en ese mismo orden
}

const CatalogoAnimacion& catalogoAnimacionDelJuego() {
    static const CatalogoAnimacion catalogo = cargaDelJuego();
    return catalogo;
}

}  // namespace Game
