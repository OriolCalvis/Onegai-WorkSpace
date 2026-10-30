// APARIENCIA-01 — ver include/Game/Personajes/MontajePersonajes.h.
#include "Game/Personajes/MontajePersonajes.h"

#include <algorithm>
#include <utility>

namespace Game {

namespace {

using Capas = Result<std::vector<CapaDeMontaje>>;

bool contiene(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

// PL-JUEGO-01: el orden propio de una fila para una talla. Las claves «<talla>/f<n>» (un arma de PixelLab
// que en esa talla va delante o detras del cuerpo en otro frame que en la media) mandan sobre «f<n>» en
// esa talla y no se usan en las demas.
std::map<std::string, int> ordenDeTalla(const std::map<std::string, int>& orden, const std::string& talla) {
    std::map<std::string, int> out;
    const std::string prefijo = talla + "/";
    for (const auto& [clave, z] : orden)
        if (clave.find('/') == std::string::npos) out[clave] = z;
    for (const auto& [clave, z] : orden)
        if (clave.rfind(prefijo, 0) == 0) out[clave.substr(prefijo.size())] = z;
    return out;
}

// La ruta del atlas de `grupo` para `talla`, o vacia si no lo tiene.
std::string atlasDe(const GrupoDeCapas& grupo, const std::string& talla) {
    const auto it = grupo.atlas.find(talla);
    return it == grupo.atlas.end() ? std::string() : it->second;
}

// ESTETICA-01: el tinte de una pieza en colores. Tono a tono (luz con luz,
// base con base...); si dos origenes comparten un color, manda el primero,
// igual que en el probador. Error si falta la paleta de algun material.
Result<std::vector<std::pair<CapaDeMontaje::Color, CapaDeMontaje::Color>>> cambiosDe(
    const ManifiestoPersonajes& m, const PiezaTenida& pieza) {
    using Cambios = std::vector<std::pair<CapaDeMontaje::Color, CapaDeMontaje::Color>>;
    Cambios out;
    for (const auto& [origen, destino] : pieza.tinte) {
        const auto o = m.paletas.find(origen);
        const auto d = m.paletas.find(destino);
        if (o == m.paletas.end() || d == m.paletas.end()) {
            return Result<Cambios>::Error("la pieza '" + pieza.pieza + "' pide la paleta '" +
                                          (o == m.paletas.end() ? origen : destino) +
                                          "', que no esta en personajes.json");
        }
        for (std::size_t t = 0; t < 5; ++t) {
            const CapaDeMontaje::Color& de = o->second[t];
            const bool yaEsta = std::any_of(out.begin(), out.end(),
                                            [&](const auto& c) { return c.first == de; });
            if (!yaEsta && de != d->second[t])
                out.emplace_back(de, d->second[t]);
        }
    }
    return Result<Cambios>::Ok(std::move(out));
}

}  // namespace

MontajePersonajes::MontajePersonajes(ManifiestoPersonajes manifiesto, std::string carpetaTexturas,
                                     std::size_t presupuestoBytes)
    : m_manifiesto(std::move(manifiesto))
    , m_carpeta(std::move(carpetaTexturas))
    , m_presupuesto(presupuestoBytes) {}

Result<std::unique_ptr<MontajePersonajes>> MontajePersonajes::cargar(
    const std::string& carpetaTexturas) {
    auto m = ManifiestoPersonajes::cargar(carpetaTexturas);
    if (!m.isOk())
        return Result<std::unique_ptr<MontajePersonajes>>::Error(m.errorMessage());
    return Result<std::unique_ptr<MontajePersonajes>>::Ok(
        std::make_unique<MontajePersonajes>(std::move(m.value()), carpetaTexturas));
}

Capas MontajePersonajes::capas(const Apariencia& a,
                               const std::map<std::string, std::string>& equipo) const {
    const ManifiestoPersonajes& m = m_manifiesto;
    const RazaVisual* r = m.raza(a.raza);
    if (r == nullptr)
        return Capas::Error("no hay raza '" + a.raza + "'");

    // Otra piel u otra talla: el cuerpo sale del atlas de cuerpos lisos,
    // que trae los ojos azules; si no, de las capas de la raza.
    const bool otra =
        (!a.talla.empty() && a.talla != r->talla) || (!a.piel.empty() && a.piel != r->piel);
    const std::string talla = a.talla.empty() ? r->talla : a.talla;
    const std::string tallaRopa = talla == r->talla ? r->tallaRopa : talla;
    const std::string ojos = a.ojos.empty() ? r->ojos : a.ojos;

    std::vector<CapaDeMontaje> out;
    std::string ojosDelCuerpo;
    if (otra) {
        if (!r->cuerpoLiso) {
            return Capas::Error("la raza " + r->id +
                                " tiene rasgos propios: no admite otra piel ni talla");
        }
        const std::string piel = a.piel.empty() ? r->piel : a.piel;
        const FilaDeCapa* fila = m.cuerpos.buscar(piel);
        const std::string ruta = atlasDe(m.cuerpos, talla);
        if (fila == nullptr || ruta.empty())
            return Capas::Error("no hay cuerpo '" + piel + "' en talla " + talla);
        out.push_back({"cuerpo", ruta, fila->fila});
        ojosDelCuerpo = "azul";
    } else {
        for (const std::string& capa : r->capas) {
            if (contiene(a.quitarCapas, capa))
                continue;
            const auto it = m.atlasDeRaza.find(capa);
            if (it == m.atlasDeRaza.end())
                return Capas::Error("la capa de raza '" + capa + "' no tiene atlas");
            out.push_back({capa, it->second, r->fila});
        }
        ojosDelCuerpo = r->ojos;
    }
    if (ojos != ojosDelCuerpo) {
        const FilaDeCapa* fila = m.ojos.buscar(ojos);
        const std::string ruta = atlasDe(m.ojos, talla);
        if (fila == nullptr || ruta.empty())
            return Capas::Error("no hay ojos '" + ojos + "' en talla " + talla);
        out.insert(out.begin() + static_cast<std::ptrdiff_t>(std::min<std::size_t>(1, out.size())),
                   CapaDeMontaje{"ojos", ruta, fila->fila});
    }
    if (!a.barbaEstilo.empty()) {
        const std::string id = a.barbaEstilo + "/" + a.barbaColor;
        const FilaDeCapa* fila = m.barba.buscar(id);
        const std::string ruta = atlasDe(m.barba, talla);
        if (fila == nullptr || ruta.empty())
            return Capas::Error("no hay barba '" + id + "'");
        out.push_back({"barba", ruta, fila->fila});
    }

    // Lo puesto manda sobre la ropa de la apariencia, ranura a ranura.
    std::map<std::string, std::string> puesto = a.ropa;
    for (const auto& [ranura, pieza] : equipo)
        puesto[ranura] = pieza;
    const auto cabeza = puesto.find("cabeza");
    const auto grupoCabeza = m.equipo.find("cabeza");
    const bool tapaElPelo = cabeza != puesto.end() && grupoCabeza != m.equipo.end() &&
                            contiene(grupoCabeza->second.ocultaPelo, separarTinte(cabeza->second).pieza);
    if (!a.peloEstilo.empty() && !tapaElPelo) {
        const std::string id = a.peloEstilo + "/" + a.peloColor;
        const FilaDeCapa* fila = m.pelo.buscar(id);
        const std::string ruta = atlasDe(m.pelo, talla);
        if (fila == nullptr || ruta.empty())
            return Capas::Error("no hay peinado '" + id + "'");
        out.push_back({"pelo", ruta, fila->fila});
    }
    for (const char* nombre : kRanurasDeEquipo) {
        const std::string ranura = nombre;
        const auto it = puesto.find(ranura);
        if (it == puesto.end() || it->second.empty() || !r->admite(ranura))
            continue;
        const auto grupo = m.equipo.find(ranura);
        const FilaDeCapa* fila =
            grupo == m.equipo.end() ? nullptr : grupo->second.buscar(it->second);
        if (fila == nullptr)
            return Capas::Error("no hay pieza de " + ranura + " '" + it->second + "'");
        const std::string ruta = atlasDe(grupo->second, esRanuraDeRopa(ranura) ? tallaRopa : talla);
        if (ruta.empty())
            return Capas::Error("la pieza '" + it->second + "' no tiene talla " + tallaRopa);
        auto cambios = cambiosDe(m, separarTinte(it->second));
        if (!cambios.isOk())
            return Capas::Error(cambios.errorMessage());
        out.push_back({ranura, ruta, fila->fila, std::move(cambios.value())});
        // SP-02: el orden propio de la fila (los guanteletes). PL-JUEGO-01: el de un arma de PixelLab cambia
        // de una talla a otra en algun frame («<talla>/f<n>»): el de la talla de su atlas.
        out.back().orden = ordenDeTalla(fila->orden, esRanuraDeRopa(ranura) ? tallaRopa : talla);
    }
    return Capas::Ok(std::move(out));
}

Result<const imagen::ImagenRGBA*> MontajePersonajes::atlas(const std::string& ruta) {
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
        if (it->first == ruta) {
            m_cache.splice(m_cache.begin(), m_cache, it);
            return Result<const imagen::ImagenRGBA*>::Ok(&m_cache.front().second);
        }
    }
    auto leida = imagen::cargarPNG(m_carpeta + "/" + ruta);
    if (!leida.isOk())
        return Result<const imagen::ImagenRGBA*>::Error(leida.errorMessage());
    m_bytes += leida.value().bytes();
    m_cache.emplace_front(ruta, std::move(leida.value()));
    return Result<const imagen::ImagenRGBA*>::Ok(&m_cache.front().second);
}

void MontajePersonajes::recortarCache() {
    // Nunca durante un montaje (los punteros a los atlas siguen en uso):
    // solo al acabarlo. Siempre queda al menos el ultimo atlas usado.
    while (m_bytes > m_presupuesto && m_cache.size() > 1) {
        m_bytes -= m_cache.back().second.bytes();
        m_cache.pop_back();
    }
}

void MontajePersonajes::vaciarCache() {
    m_cache.clear();
    m_bytes = 0;
}

std::string MontajePersonajes::tallaDe(const Apariencia& a) const {
    const RazaVisual* r = m_manifiesto.raza(a.raza);
    if (r == nullptr)
        return "";
    return a.talla.empty() ? r->talla : a.talla;
}

Result<imagen::ImagenRGBA> MontajePersonajes::montar(
    const Apariencia& apariencia, const std::map<std::string, std::string>& equipo, MapaDeCapas* mapa) {
    using Hoja = Result<imagen::ImagenRGBA>;
    auto lista = capas(apariencia, equipo);
    if (!lista.isOk())
        return Hoja::Error(lista.errorMessage());
    const std::vector<CapaDeMontaje>& capasDelPersonaje = lista.value();

    std::vector<const imagen::ImagenRGBA*> atlas;
    for (const CapaDeMontaje& capa : capasDelPersonaje) {
        auto a = this->atlas(capa.atlas);
        if (!a.isOk())
            return Hoja::Error(a.errorMessage());
        const imagen::ImagenRGBA* img = a.value();
        if (img->ancho < kLado * frames() || img->alto < capa.fila * kLado || capa.fila < 1) {
            return Hoja::Error(capa.atlas + " no tiene la fila " + std::to_string(capa.fila) + " de " +
                               std::to_string(frames()) + " frames de 64x64");
        }
        if (m_manifiesto.orden.find(capa.ranura) == m_manifiesto.orden.end()) {
            return Hoja::Error("la ranura '" + capa.ranura + "' no tiene orden de dibujo");
        }
        atlas.push_back(img);
    }

    imagen::ImagenRGBA hoja;
    const int nFrames = frames();
    const Reparto reparto = repartoDeColumnas(nFrames);
    hoja.ancho = kLado * nFrames;
    hoja.alto = kLado;
    hoja.pixeles.assign(static_cast<std::size_t>(hoja.ancho) * hoja.alto * 4, 0);
    if (mapa != nullptr) {
        mapa->ranuras.clear();
        for (const CapaDeMontaje& capa : capasDelPersonaje)
            mapa->ranuras.push_back(capa.ranura);
        mapa->arriba.assign(static_cast<std::size_t>(hoja.ancho) * hoja.alto, 0);
        const auto anclas = m_manifiesto.anclas.find(tallaDe(apariencia));
        mapa->anclas = anclas != m_manifiesto.anclas.end() ? anclas->second : AnclasDeTalla{};
    }
    std::vector<std::pair<int, std::size_t>> pila;  // (z, indice): a igual z, el orden de capas()
    for (int frame = 1; frame <= nFrames; ++frame) {
        const char* direccion = vistaDeFrame(frame, reparto);
        pila.clear();
        for (std::size_t i = 0; i < capasDelPersonaje.size(); ++i) {
            // SP-02: el de su fila, si lo trae (los guanteletes), y si no el de su ranura. Oriol (23/10):
            // la Forja de Onegai pone un arma delante o detras en un frame suelto: «f<n>» manda sobre la direccion.
            const auto delFrame = capasDelPersonaje[i].orden.find("f" + std::to_string(frame));
            if (delFrame != capasDelPersonaje[i].orden.end()) {
                pila.emplace_back(delFrame->second, i);
                continue;
            }
            const auto propio = capasDelPersonaje[i].orden.find(direccion);
            if (propio != capasDelPersonaje[i].orden.end()) {
                pila.emplace_back(propio->second, i);
                continue;
            }
            const auto& porDireccion = m_manifiesto.orden.at(capasDelPersonaje[i].ranura);
            const auto z = porDireccion.find(direccion);
            pila.emplace_back(z == porDireccion.end() ? 0 : z->second, i);
        }
        std::sort(pila.begin(), pila.end());
        const int x0 = (frame - 1) * kLado;
        for (const auto& [z, i] : pila) {
            const imagen::ImagenRGBA& origen = *atlas[i];
            const int y0 = (capasDelPersonaje[i].fila - 1) * kLado;
            const auto& cambios = capasDelPersonaje[i].cambios;
            for (int y = 0; y < kLado; ++y) {
                for (int x = 0; x < kLado; ++x) {
                    std::uint8_t* destino =
                        hoja.pixeles.data() + (static_cast<std::size_t>(y) * hoja.ancho +
                                               static_cast<std::size_t>(x0 + x)) *
                                                  4;
                    const std::uint8_t* px = origen.pixel(x0 + x, y0 + y);
                    if (mapa != nullptr && px[3] != 0)
                        mapa->arriba[static_cast<std::size_t>(y) * hoja.ancho + static_cast<std::size_t>(x0 + x)] =
                            static_cast<std::uint8_t>(i + 1);
                    if (cambios.empty() || px[3] == 0) {
                        mezclarEncima(destino, px);
                        continue;
                    }
                    // ESTETICA-01: el tinte, antes de mezclar.
                    std::uint8_t tenido[4] = {px[0], px[1], px[2], px[3]};
                    for (const auto& [de, a] : cambios) {
                        if (de[0] == px[0] && de[1] == px[1] && de[2] == px[2]) {
                            tenido[0] = a[0];
                            tenido[1] = a[1];
                            tenido[2] = a[2];
                            break;
                        }
                    }
                    mezclarEncima(destino, tenido);
                }
            }
        }
    }
    recortarCache();
    return Hoja::Ok(std::move(hoja));
}

void mezclarEncima(std::uint8_t* debajo, const std::uint8_t* encima) {
    // Pillow, libImaging/AlphaComposite.c: 7 bits de precision y divisiones
    // entre 255 redondeadas con ((a >> 8) + a) >> 8. Copiada tal cual para
    // que la hoja sea la misma que la del probador, no «casi» la misma.
    if (encima[3] == 0)
        return;
    constexpr std::uint32_t kPrecision = 7;
    const auto entre255 = [](std::uint32_t a) { return ((a >> 8) + a) >> 8; };
    const std::uint32_t alfaEncima = encima[3];
    const std::uint32_t mezcla = static_cast<std::uint32_t>(debajo[3]) * (255u - alfaEncima);
    const std::uint32_t alfa255 = alfaEncima * 255u + mezcla;
    const std::uint32_t coef1 = alfaEncima * 255u * 255u * (1u << kPrecision) / alfa255;
    const std::uint32_t coef2 = 255u * (1u << kPrecision) - coef1;
    for (int c = 0; c < 3; ++c) {
        const std::uint32_t t = static_cast<std::uint32_t>(encima[c]) * coef1 +
                                static_cast<std::uint32_t>(debajo[c]) * coef2;
        debajo[c] = static_cast<std::uint8_t>(entre255(t + (0x80u << kPrecision)) >> kPrecision);
    }
    debajo[3] = static_cast<std::uint8_t>(entre255(alfa255 + 0x80u));
}

Reparto repartoDeColumnas(int columnas) {
    return columnas >= MontajePersonajes::kFramesDiagonal ? Reparto::Diagonal : Reparto::Clasico;
}

int framesDeReparto(Reparto reparto) {
    return reparto == Reparto::Diagonal ? MontajePersonajes::kFramesDiagonal : MontajePersonajes::kFrames;
}

namespace {
constexpr int kAndarDiagonal = 6;     // walking-6-frames de PixelLab
constexpr int kRespirarDiagonal = 4;  // breathing-idle
}  // namespace

Rumbo rumboDeFrame(int frame, Reparto reparto) {
    const int f = std::max(1, std::min(frame, framesDeReparto(reparto)));
    if (reparto == Reparto::Diagonal) {
        if (f > MontajePersonajes::kFramesDeAndarDiagonal)
            return static_cast<Rumbo>((f - MontajePersonajes::kFramesDeAndarDiagonal - 1) / kRespirarDiagonal);
        return static_cast<Rumbo>((f - 1) / kAndarDiagonal);
    }
    if (f > MontajePersonajes::kFramesDeAndar)  // el reposo: dos por rumbo
        return static_cast<Rumbo>((f - MontajePersonajes::kFramesDeAndar - 1) / 2);
    return static_cast<Rumbo>((f - 1) / 3);
}

const char* nombreDeRumbo(Rumbo rumbo) {
    switch (rumbo) {
        case Rumbo::Arriba:
            return "arriba";
        case Rumbo::Derecha:
            return "derecha";
        case Rumbo::Abajo:
            return "abajo";
        case Rumbo::Izquierda:
            return "izquierda";
    }
    return "abajo";
}

const char* vistaDeFrame(int frame, Reparto reparto) {
    const Rumbo r = rumboDeFrame(frame, reparto);
    if (reparto == Reparto::Clasico)
        return nombreDeRumbo(r);
    // sureste (Derecha) y suroeste (Abajo) miran a camara; noreste y noroeste dan la espalda
    return r == Rumbo::Derecha || r == Rumbo::Abajo ? "abajo" : "arriba";
}

int frameQuieto(Rumbo rumbo, Reparto reparto) {
    if (reparto == Reparto::Diagonal)
        return MontajePersonajes::kFramesDeAndarDiagonal + 1 + static_cast<int>(rumbo) * kRespirarDiagonal;
    return static_cast<int>(rumbo) * 3 + 2;
}

std::vector<int> cicloDeAndar(Rumbo rumbo, Reparto reparto) {
    if (reparto == Reparto::Diagonal) {
        std::vector<int> ciclo;
        const int primero = 1 + static_cast<int>(rumbo) * kAndarDiagonal;
        for (int i = 0; i < kAndarDiagonal; ++i)
            ciclo.push_back(primero + i);
        return ciclo;
    }
    const int q = frameQuieto(rumbo);
    return {q, q - 1, q, q + 1};
}

std::vector<int> framesDeReposo(Rumbo rumbo, Reparto reparto) {
    if (reparto == Reparto::Diagonal) {
        const int primero = frameQuieto(rumbo, reparto);
        return {primero, primero + 1, primero + 2, primero + 3};
    }
    const int primero = MontajePersonajes::kFramesDeAndar + 1 + static_cast<int>(rumbo) * 2;
    return {primero, primero + 1};
}

std::vector<int> cicloDeReposo(Rumbo rumbo, Reparto reparto) {
    if (reparto == Reparto::Diagonal) {
        // La respiracion de PixelLab, cada frame cinco veces: ~1,4 s por respiracion a 0,07 s.
        std::vector<int> ciclo;
        for (int f : framesDeReposo(rumbo, reparto))
            ciclo.insert(ciclo.end(), 5, f);
        return ciclo;
    }
    // Como en NosTale: casi quieto, pero vivo. 12 frames quieto y 9 respirando.
    const std::vector<int> reposo = framesDeReposo(rumbo);
    std::vector<int> ciclo(12, reposo[0]);
    ciclo.insert(ciclo.end(), 9, reposo[1]);
    return ciclo;
}

int frameDeVistaPrevia(double segundos, Reparto reparto) {
    static const Rumbo kGiro[4] = {Rumbo::Abajo, Rumbo::Derecha, Rumbo::Arriba, Rumbo::Izquierda};
    if (reparto == Reparto::Diagonal) {
        // 0,12 s por frame: dos pasos de seis y dos respiraciones (cada frame dos veces) por rumbo.
        constexpr double kPorFrame = 0.12;
        constexpr int kAndando = 2 * kAndarDiagonal;
        constexpr int kFramesPorRumbo = kAndando + 4 * kRespirarDiagonal;
        const long n = segundos > 0.0 ? static_cast<long>(segundos / kPorFrame) : 0;
        const Rumbo rumbo = kGiro[(n / kFramesPorRumbo) % 4];
        const long k = n % kFramesPorRumbo;
        if (k < kAndando)
            return cicloDeAndar(rumbo, reparto)[static_cast<std::size_t>(k % kAndarDiagonal)];
        return framesDeReposo(rumbo, reparto)[static_cast<std::size_t>(((k - kAndando) / 2) % kRespirarDiagonal)];
    }
    // 0,17 s por frame (el GIF del probador): dos ciclos de andar de cuatro
    // frames por rumbo y, ESTETICA-01, su reposo (quieto y respirando, cuatro
    // de cada) antes de girar: abajo, derecha, arriba, izquierda.
    constexpr double kPorFrame = 0.17;
    constexpr int kAndando = 8;
    constexpr int kFramesPorRumbo = kAndando + 8;
    static const int kPaso[4] = {-1, 0, 1, 0};  // paso, quieto, paso, quieto
    const long n = segundos > 0.0 ? static_cast<long>(segundos / kPorFrame) : 0;
    const Rumbo rumbo = kGiro[(n / kFramesPorRumbo) % 4];
    const long k = n % kFramesPorRumbo;
    if (k < kAndando)
        return frameQuieto(rumbo) + kPaso[k % 4];
    return framesDeReposo(rumbo)[k - kAndando < 4 ? 0 : 1];
}

}  // namespace Game
