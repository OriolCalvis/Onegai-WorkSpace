// PERF-1 / PERF-1b — La cache de filas de atlas de MontajePersonajes (aparte de MontajePersonajes.cpp
// por tamano). Un montaje solo usa una fila de 64 px de cada atlas: se guardan filas, no atlas.
#include "Game/Personajes/MontajePersonajes.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace Game {

std::size_t MontajePersonajes::presupuestoDeTexto(const char* texto) {
    char* fin = nullptr;
    const long long mb = texto != nullptr ? std::strtoll(texto, &fin, 10) : -1;
    return texto != nullptr && fin != texto && mb >= 0 ? static_cast<std::size_t>(mb) * 1024u * 1024u
                                                       : kPresupuestoPorDefecto;
}

std::size_t MontajePersonajes::presupuestoDeEntorno() {
    static const std::size_t valor = presupuestoDeTexto(std::getenv("MOTOR_CACHE_ATLAS_MB"));
    return valor;
}

std::string MontajePersonajes::claveDe(const std::string& ruta, int fila) {
    return ruta + "#" + std::to_string(fila);
}

std::size_t MontajePersonajes::atlasEnCache() const {
    std::set<std::string> rutas;
    for (const Entrada& e : m_cache)
        rutas.insert(e.ruta);
    for (const Entrada& e : m_especulativas)
        rutas.insert(e.ruta);
    return rutas.size();
}

std::vector<std::string> MontajePersonajes::clavesEnOrden() const {
    std::vector<std::string> out;
    for (const Entrada& e : m_cache)
        out.push_back(e.clave);
    for (const Entrada& e : m_especulativas)
        out.push_back(e.clave);
    return out;
}

void MontajePersonajes::meterFila(const std::string& ruta, int fila, int ancho,
                                  std::vector<std::uint8_t> pixeles, bool alFrente) {
    Entrada e;
    e.clave = claveDe(ruta, fila);
    e.ruta = ruta;
    e.ancho = ancho;
    e.montaje = alFrente ? m_montaje : 0;
    e.especulativa = !alFrente;
    e.pixeles = std::move(pixeles);
    const std::size_t bytesFila = e.pixeles.size();
    Lista& donde = alFrente ? m_cache : m_especulativas;
    donde.push_back(std::move(e));
    const auto it = std::prev(donde.end());
    if (alFrente)
        m_cache.splice(m_cache.begin(), m_cache, it);
    m_indice[it->clave] = it;
    m_bytes += bytesFila;
    m_pico = std::max(m_pico, m_bytes);
}

Result<bool> MontajePersonajes::leerAtlas(const std::string& ruta, const std::set<int>& necesarias) {
    if (leerDeFilas(ruta, necesarias))
        return Result<bool>::Ok(true);  // PERF-3: solo las filas necesarias, sin tocar el PNG
    auto leida = imagen::cargarPNG(m_carpeta + "/" + ruta);
    if (!leida.isOk())
        return Result<bool>::Error(leida.errorMessage());
    ++m_lecturas;
    const imagen::ImagenRGBA& img = leida.value();
    const int filasTotales = img.alto / kLado;
    const std::size_t bytesFila = static_cast<std::size_t>(img.ancho) * kLado * 4;
    const bool anchoBueno = img.ancho >= kLado * frames();
    const auto guardar = [&](int fila, bool alFrente) {
        const std::uint8_t* origen = img.pixeles.data() + static_cast<std::size_t>(fila - 1) * bytesFila;
        meterFila(ruta, fila, img.ancho, std::vector<std::uint8_t>(origen, origen + bytesFila), alFrente);
    };
    // Las del montaje en curso: se hace sitio ANTES de guardar cada una, y no se sueltan.
    for (int fila : necesarias) {
        if (!anchoBueno || fila < 1 || fila > filasTotales)
            return Result<bool>::Error(ruta + " no tiene la fila " + std::to_string(fila) + " de " +
                                       std::to_string(frames()) + " frames de 64x64");
        if (m_indice.count(claveDe(ruta, fila)) != 0)
            continue;
        recortarCache(bytesFila);
        guardar(fila, true);
    }
    // Las demas del atlas (la pagina Raza pide 20 razas del mismo): solo si caben sin soltar ninguna
    // fila usada (sueltan, a lo mas, otras «por si acaso» de otros atlas, las mas antiguas).
    for (int fila = 1; fila <= filasTotales; ++fila) {
        while (m_bytes + bytesFila > m_presupuesto && !m_especulativas.empty() &&
               m_especulativas.front().ruta != ruta)
            soltar(m_especulativas, m_especulativas.begin());
        if (m_bytes + bytesFila > m_presupuesto)
            break;
        if (m_indice.count(claveDe(ruta, fila)) != 0)
            continue;
        guardar(fila, false);
    }
    return Result<bool>::Ok(true);  // el atlas entero se suelta al salir
}

MontajePersonajes::Lista::iterator MontajePersonajes::soltar(Lista& lista, Lista::iterator it) {
    m_bytes -= it->pixeles.size();
    m_indice.erase(it->clave);
    return lista.erase(it);
}

void MontajePersonajes::recortarCache(std::size_t reservar) {
    // Primero las «por si acaso», la mas antigua primero.
    while (m_bytes + reservar > m_presupuesto && !m_especulativas.empty())
        soltar(m_especulativas, m_especulativas.begin());
    // Las filas del montaje en curso siguen apuntadas desde `montar`: no se sueltan nunca (el
    // presupuesto puede pasarse mientras dura, pero no se acumula nada que el montaje no use).
    auto it = m_cache.end();
    while (m_bytes + reservar > m_presupuesto && it != m_cache.begin()) {
        --it;
        if (m_montaje != 0 && it->montaje == m_montaje)
            continue;
        it = soltar(m_cache, it);
    }
}

void MontajePersonajes::reservarParaElMontaje(const std::vector<CapaDeMontaje>& capas) {
    for (const CapaDeMontaje& capa : capas) {
        const auto ix = m_indice.find(claveDe(capa.atlas, capa.fila));
        if (ix == m_indice.end())
            continue;
        // LRU: la usada, al frente de m_cache, sacandola de la lista donde este (splice exige
        // la lista de origen correcta: si no, los tamaños de las dos listas se corrompen).
        Lista& origen = ix->second->especulativa ? m_especulativas : m_cache;
        ix->second->montaje = m_montaje;
        ix->second->especulativa = false;
        m_cache.splice(m_cache.begin(), origen, ix->second);
    }
}

void MontajePersonajes::vaciarCache() {
    m_cache.clear();
    m_especulativas.clear();
    m_indice.clear();
    m_bytes = 0;
}

}  // namespace Game
