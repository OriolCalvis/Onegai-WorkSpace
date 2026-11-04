// PERF-3 — Lectura de filas sueltas de un atlas trozeado (`<atlas>.filas`, escrito por
// tools/trocejar_atlas_personajes.py), aparte de MontajeCache.cpp por tamano.
// Formato (little endian): cabecera de 32 bytes {"MGFL", version u32 = 1, ancho u32, alto de fila
// u32 = 64, filas u32, crc32 del PNG u32, bytes del PNG u64}, luego por fila {desplazamiento u64,
// bytes u32} y los PNG de cada fila (ancho x 64 RGBA).
#include "Game/Personajes/MontajePersonajes.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

namespace Game {

namespace {

using Tabla = std::array<std::array<std::uint32_t, 256>, 8>;

const Tabla& tablaCrc() {
    static const Tabla t = [] {
        Tabla r{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1u) ? (c >> 1) ^ 0xEDB88320u : c >> 1;
            r[0][i] = c;
        }
        for (std::uint32_t i = 0; i < 256; ++i)
            for (int s = 1; s < 8; ++s)
                r[s][i] = (r[s - 1][i] >> 8) ^ r[0][r[s - 1][i] & 0xFF];
        return r;
    }();
    return t;
}

std::uint32_t leer32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | static_cast<std::uint32_t>(p[1]) << 8 |
           static_cast<std::uint32_t>(p[2]) << 16 | static_cast<std::uint32_t>(p[3]) << 24;
}

std::uint64_t leer64(const std::uint8_t* p) {
    return static_cast<std::uint64_t>(leer32(p)) | static_cast<std::uint64_t>(leer32(p + 4)) << 32;
}

constexpr std::size_t kCabecera = 32;
constexpr std::size_t kEntrada = 12;
constexpr std::uint32_t kMaxBytesFila = 16u * 1024u * 1024u;  // una fila PNG de 2560x64 no llega a 1 MB
constexpr std::uint32_t kVersion = 1;

}  // namespace

std::uint32_t crc32Bytes(const std::uint8_t* datos, std::size_t bytes) {
    const Tabla& t = tablaCrc();
    std::uint32_t c = 0xFFFFFFFFu;
    while (bytes >= 8) {
        const std::uint32_t a = leer32(datos) ^ c;
        const std::uint32_t b = leer32(datos + 4);
        c = t[7][a & 0xFF] ^ t[6][(a >> 8) & 0xFF] ^ t[5][(a >> 16) & 0xFF] ^ t[4][a >> 24] ^
            t[3][b & 0xFF] ^ t[2][(b >> 8) & 0xFF] ^ t[1][(b >> 16) & 0xFF] ^ t[0][b >> 24];
        datos += 8;
        bytes -= 8;
    }
    while (bytes-- > 0)
        c = (c >> 8) ^ t[0][(c ^ *datos++) & 0xFF];
    return c ^ 0xFFFFFFFFu;
}

void MontajePersonajes::usarFilasEn(const std::string& carpeta) {
    m_carpetaFilas = carpeta;
    m_filas.clear();
}

bool MontajePersonajes::leerDeFilas(const std::string& ruta, const std::set<int>& necesarias) {
    auto it = m_filas.find(ruta);
    if (it == m_filas.end()) {
        // Se mira una vez por atlas y ejecucion (tambien cuando no hay .filas): cabecera, indice y
        // huella del PNG (crc32 + bytes; no la mtime, que iCloud cambia).
        InfoFilas info;
        std::string base = ruta;
        if (base.size() > 4 && base.compare(base.size() - 4, 4, ".png") == 0)
            base.resize(base.size() - 4);
        info.fichero = (m_carpetaFilas.empty() ? m_carpeta : m_carpetaFilas) + "/" + base + ".filas";
        std::ifstream f(info.fichero, std::ios::binary);
        std::uint8_t cab[kCabecera];
        if (f && f.read(reinterpret_cast<char*>(cab), kCabecera) && std::memcmp(cab, "MGFL", 4) == 0 &&
            leer32(cab + 4) == kVersion && leer32(cab + 12) == static_cast<std::uint32_t>(kLado)) {
            const std::uint32_t filas = leer32(cab + 16);
            // Un .filas roto no puede pedir memoria absurda: el numero de filas se mira ANTES de
            // reservar el indice, y cada entrada contra el tamano real del fichero (abajo).
            f.seekg(0, std::ios::end);
            const auto tamFichero = static_cast<std::uint64_t>(std::max<std::streamoff>(0, f.tellg()));
            f.seekg(kCabecera);
            const bool filasRazonables = filas > 0 && filas < 100000;
            std::vector<std::uint8_t> idx(filasRazonables ? static_cast<std::size_t>(filas) * kEntrada : 0);
            std::ifstream png(m_carpeta + "/" + ruta, std::ios::binary);
            std::vector<std::uint8_t> bytesPng((std::istreambuf_iterator<char>(png)),
                                               std::istreambuf_iterator<char>());
            if (filasRazonables && f.read(reinterpret_cast<char*>(idx.data()), static_cast<std::streamsize>(idx.size())) &&
                !bytesPng.empty() && bytesPng.size() == leer64(cab + 24) &&
                crc32Bytes(bytesPng.data(), bytesPng.size()) == leer32(cab + 20)) {
                info.valida = true;
                info.ancho = static_cast<int>(leer32(cab + 8));
                for (std::uint32_t i = 0; i < filas && info.valida; ++i) {
                    const std::uint64_t desde = leer64(idx.data() + i * kEntrada);
                    const std::uint32_t bytes = leer32(idx.data() + i * kEntrada + 8);
                    if (bytes == 0 || bytes > kMaxBytesFila || desde > tamFichero || bytes > tamFichero - desde)
                        info.valida = false;  // indice que apunta fuera del fichero: camino del atlas entero
                    else
                        info.indice.emplace_back(desde, bytes);
                }
                if (!info.valida)
                    info.indice.clear();
            }
        }
        it = m_filas.emplace(ruta, std::move(info)).first;
    }
    const InfoFilas& info = it->second;
    if (!info.valida || info.ancho < kLado * frames())
        return false;
    for (int fila : necesarias)
        if (fila < 1 || fila > static_cast<int>(info.indice.size()))
            return false;  // que el camino del atlas entero diga el error de siempre
    // Se decodifican todas antes de guardar ninguna: si una falla, no queda nada a medias.
    std::ifstream f(info.fichero, std::ios::binary);
    if (!f)
        return false;
    std::vector<std::pair<int, std::vector<std::uint8_t>>> leidas;
    std::vector<char> buf;
    for (int fila : necesarias) {
        if (m_indice.count(claveDe(ruta, fila)) != 0)
            continue;
        const auto [desde, bytes] = info.indice[static_cast<std::size_t>(fila - 1)];
        buf.resize(bytes);
        f.seekg(static_cast<std::streamoff>(desde));
        if (!f.read(buf.data(), static_cast<std::streamsize>(bytes)))
            return false;
        auto png = imagen::decodificarPNG(reinterpret_cast<const std::uint8_t*>(buf.data()), buf.size());
        if (!png.isOk() || png.value().ancho != info.ancho || png.value().alto != kLado)
            return false;
        leidas.emplace_back(fila, std::move(png.value().pixeles));
    }
    for (auto& [fila, pixeles] : leidas) {
        recortarCache(pixeles.size());
        meterFila(ruta, fila, info.ancho, std::move(pixeles), true);
        ++m_lecturasFilas;
    }
    return true;
}

}  // namespace Game
