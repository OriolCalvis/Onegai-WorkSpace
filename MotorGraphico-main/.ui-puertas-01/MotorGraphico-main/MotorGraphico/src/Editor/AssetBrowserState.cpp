// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-13  (fase E, 4 j) - Navegador visual i importació de recursos
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/AssetBrowserState.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "Editor/ProblemsState.h"    // NormalizeForSearch (DEC-15)
#include "Project/ProjectIndex.h"    // suggestId (E-11)

// E-13 — Navegador de recursos e importacion. MITAD SIN INTERFAZ.
//
// El contrato y el porqué de cada decisión están en la cabecera; aquí sólo
// se añade lo que la implementación obliga a decir:
//
// SIN <filesystem> Y SIN mkdir. La existencia de un fichero se comprueba
// abriéndolo y la copia se hace con flujos, igual que ProjectIndex. Crear
// la carpeta de destino no se hace a propósito: ProjectHub.h documenta que
// este motor no tiene ni un mkdir en src/, y meter uno aquí abriría la
// primera excepción. Si la carpeta no existe, la copia falla y el error lo
// dice con palabras —el autor la crea (o elige otra) y vuelve a intentarlo.
//
// LA COPIA PASA POR UN TEMPORAL. El contenido se lee entero antes de
// escribir un solo byte, y se escribe en "<destino>.tmp" para moverlo
// después (mismo patrón que ProjectSerializer::atomicWrite). Un fallo a
// medias deja el temporal, nunca un PNG partido en la ruta que el proyecto
// referencia.

namespace Editor {

namespace {

// Firma PNG (8 bytes) + longitud del chunk (4) + "IHDR" (4) + ancho (4) +
// alto (4). Es lo mínimo que hay que tener leído para saber las dimensiones.
constexpr std::size_t kPngFirma = 8;
constexpr std::size_t kPngHastaAlto = 24;
constexpr unsigned char kPngSignature[8] = {0x89, 0x50, 0x4E, 0x47,
                                            0x0D, 0x0A, 0x1A, 0x0A};
constexpr std::size_t kLongitudPrefijoIhdr = 8;

int bigEndian32(const unsigned char* bytes) {
    return static_cast<int>((static_cast<unsigned int>(bytes[0]) << 24) |
                            (static_cast<unsigned int>(bytes[1]) << 16) |
                            (static_cast<unsigned int>(bytes[2]) << 8) |
                            static_cast<unsigned int>(bytes[3]));
}

bool existeFichero(const std::string& ruta) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) {
        return false;
    }
    std::fclose(f);
    return true;
}

bool leeFichero(const std::string& ruta, std::string& contenido) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada.is_open()) {
        return false;
    }
    std::ostringstream buffer;
    buffer << entrada.rdbuf();
    contenido = buffer.str();
    return static_cast<bool>(entrada) || entrada.eof();
}

// "assets/proyecto" + "textures/cofre.png" -> "assets/proyecto/textures/cofre.png".
std::string juntaRutas(const std::string& raiz, const std::string& relativa) {
    if (raiz.empty()) return relativa;
    if (relativa.empty()) return raiz;
    const bool separador = raiz.back() == '/' || raiz.back() == '\\';
    return separador ? (raiz + relativa) : (raiz + "/" + relativa);
}

// "textures/" -> "textures"; "./textures" -> "textures"; "" -> "".
// La referencia que se guarda es relativa y estable: ni barras al principio
// (sería absoluta) ni al final (duplicarían el separador al componerla).
std::string normalizaCarpeta(const std::string& carpeta) {
    std::size_t inicio = 0;
    std::size_t fin = carpeta.size();
    while (inicio < fin && (carpeta[inicio] == '/' || carpeta[inicio] == '\\')) ++inicio;
    while (fin > inicio && (carpeta[fin - 1] == '/' || carpeta[fin - 1] == '\\')) --fin;
    std::string salida = carpeta.substr(inicio, fin - inicio);
    while (salida == "." ) salida.clear();
    if (salida.rfind("./", 0) == 0) {
        return normalizaCarpeta(salida.substr(2));
    }
    return salida;
}

std::string carpetaDe(const std::string& ruta) {
    const std::size_t corte = ruta.find_last_of("/\\");
    return (corte == std::string::npos) ? std::string() : ruta.substr(0, corte);
}

}  // namespace

// ---------------------------------------------------------------------
// Cabecera PNG (sin GL)
// ---------------------------------------------------------------------

Result<PngInfo> ParsePngInfo(const unsigned char* bytes, std::size_t n) {
    using R = Result<PngInfo>;
    if (bytes == nullptr) {
        return R::Error("no hay bytes que leer");
    }
    if (n < kPngFirma) {
        return R::Error("solo hay " + std::to_string(n) +
                        " bytes: demasiado corto para ser un PNG");
    }
    if (std::memcmp(bytes, kPngSignature, kPngFirma) != 0) {
        return R::Error("no es un PNG: la firma no coincide (un recurso importado "
                        "tiene que ser un PNG valido)");
    }
    if (n < kPngHastaAlto) {
        return R::Error("PNG truncado: faltan los bytes del IHDR");
    }
    if (std::memcmp(bytes + 12, "IHDR", 4) != 0) {
        return R::Error("el primer chunk no es IHDR: no es un PNG utilizable");
    }
    if (bigEndian32(bytes + 8) < static_cast<int>(kLongitudPrefijoIhdr)) {
        return R::Error("IHDR declara una longitud imposible");
    }
    PngInfo info;
    info.width = bigEndian32(bytes + 16);
    info.height = bigEndian32(bytes + 20);
    if (info.width <= 0 || info.height <= 0) {
        return R::Error("dimensiones no positivas (" + std::to_string(info.width) +
                        "x" + std::to_string(info.height) + ")");
    }
    return R::Ok(info);
}

Result<PngInfo> ReadPngInfo(const std::string& path) {
    using R = Result<PngInfo>;
    std::ifstream entrada(path, std::ios::binary);
    if (!entrada.is_open()) {
        return R::Error("no se puede abrir '" + path +
                        "': el fichero no existe o no se puede leer");
    }
    unsigned char buffer[kPngHastaAlto] = {0};
    entrada.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(kPngHastaAlto));
    const std::size_t leidos = static_cast<std::size_t>(entrada.gcount());
    const Result<PngInfo> info = ParsePngInfo(buffer, leidos);
    if (!info.isOk()) {
        return R::Error("'" + path + "': " + info.errorMessage());
    }
    return info;
}

// ---------------------------------------------------------------------
// Tipo y frames
// ---------------------------------------------------------------------

const char* AssetKindName(AssetKind kind) {
    switch (kind) {
        case AssetKind::Tile:   return "baldosa";
        case AssetKind::Sprite: return "sprite";
        case AssetKind::UI:     return "interfaz";
        case AssetKind::Unknown: break;
    }
    return "desconocido";
}

Result<FrameGrid> ComputeFrameGrid(const FrameLayout& layout, int imageWidth, int imageHeight) {
    using R = Result<FrameGrid>;
    if (imageWidth <= 0 || imageHeight <= 0) {
        return R::Error("la imagen tiene dimensiones no positivas (" +
                        std::to_string(imageWidth) + "x" + std::to_string(imageHeight) + ")");
    }
    FrameGrid grid;
    if (!layout.isSheet()) {
        return R::Ok(grid);  // imagen suelta: una celda, un frame
    }
    if (layout.frameWidth > imageWidth || layout.frameHeight > imageHeight) {
        return R::Error("la celda de " + std::to_string(layout.frameWidth) + "x" +
                        std::to_string(layout.frameHeight) + " no cabe en la imagen de " +
                        std::to_string(imageWidth) + "x" + std::to_string(imageHeight));
    }
    const int sobranteX = imageWidth % layout.frameWidth;
    const int sobranteY = imageHeight % layout.frameHeight;
    if (sobranteX != 0 || sobranteY != 0) {
        return R::Error("la hoja no divide la imagen en entero: sobran " +
                        std::to_string(sobranteX) + " px en ancho y " +
                        std::to_string(sobranteY) +
                        " px en alto. Una celda partida a medias es un sprite roto; "
                        "se ve mejor aqui que dentro del juego");
    }
    grid.columns = imageWidth / layout.frameWidth;
    grid.rows = imageHeight / layout.frameHeight;
    grid.frameCount = grid.columns * grid.rows;
    return R::Ok(grid);
}

// ---------------------------------------------------------------------
// Catalogo, busqueda y seleccion
// ---------------------------------------------------------------------

void AssetBrowserState::setAssets(std::vector<AssetDescriptor> assets) {
    m_assets = std::move(assets);
    resolverSeleccion();
}

void AssetBrowserState::addAsset(AssetDescriptor asset) {
    m_assets.push_back(std::move(asset));
}

const AssetDescriptor* AssetBrowserState::findById(const std::string& id) const {
    for (const AssetDescriptor& asset : m_assets) {
        if (asset.id == id) {
            return &asset;
        }
    }
    return nullptr;
}

void AssetBrowserState::setQuery(const std::string& query) {
    m_query = query;
}

void AssetBrowserState::setKindFilter(AssetKind kind) {
    m_kindFilter = kind;
}

std::vector<const AssetDescriptor*> AssetBrowserState::visible() const {
    // Consulta vacia = todo, como la busqueda de E-15: el panel abre con la
    // caja vacia y la lista llena.
    const std::string aguja = NormalizeForSearch(m_query);
    std::vector<const AssetDescriptor*> salida;
    for (const AssetDescriptor& asset : m_assets) {
        if (m_kindFilter != AssetKind::Unknown && asset.kind != m_kindFilter) {
            continue;
        }
        if (!aguja.empty()) {
            const std::string nombre = NormalizeForSearch(asset.name);
            const std::string id = NormalizeForSearch(asset.id);
            if (nombre.find(aguja) == std::string::npos &&
                id.find(aguja) == std::string::npos) {
                continue;
            }
        }
        salida.push_back(&asset);
    }
    return salida;
}

bool AssetBrowserState::select(const std::string& id) {
    if (findById(id) == nullptr) {
        // No se selecciona al vecino ni se deja una seleccion colgando.
        m_hasSelection = false;
        m_selectedId.clear();
        m_lastSelectionChange = SelectionChange::Lost;
        return false;
    }
    m_selectedId = id;
    m_hasSelection = true;
    // Una eleccion explicita no tiene nada que avisar: Lost/Kept son para el
    // refresco del catalogo (resolverSeleccion).
    m_lastSelectionChange = SelectionChange::None;
    return true;
}

const AssetDescriptor* AssetBrowserState::selected() const {
    if (!m_hasSelection) {
        return nullptr;
    }
    return findById(m_selectedId);
}

void AssetBrowserState::clearSelection() {
    m_hasSelection = false;
    m_selectedId.clear();
    m_lastSelectionChange = SelectionChange::None;
}

void AssetBrowserState::resolverSeleccion() {
    if (!m_hasSelection) {
        m_lastSelectionChange = SelectionChange::None;
        return;
    }
    if (findById(m_selectedId) != nullptr) {
        m_lastSelectionChange = SelectionChange::Kept;
        return;
    }
    m_hasSelection = false;
    m_selectedId.clear();
    m_lastSelectionChange = SelectionChange::Lost;
}

// ---------------------------------------------------------------------
// Importacion
// ---------------------------------------------------------------------

Result<ImportPlan> AssetBrowserState::planImport(const ImportRequest& request) const {
    using R = Result<ImportPlan>;
    if (request.sourcePath.empty()) {
        return R::Error("falta el fichero de origen: el dialogo no devolvio ninguna ruta");
    }
    if (request.assetsRoot.empty()) {
        return R::Error("falta la raiz del proyecto: no hay donde guardar el recurso");
    }
    if (request.visibleName.empty()) {
        return R::Error("falta el nombre visible: de ahi salen el id y la referencia");
    }

    // La identidad sale del nombre con la misma regla que los proyectos
    // (E-11): el autor escribe «Cofre del templo» y la referencia es
    // "textures/cofre_del_templo.png", que es lo que no cambia nunca.
    const Result<std::string> base = ProjectIndex::suggestId(request.visibleName);
    if (!base.isOk()) {
        return R::Error(base.errorMessage());
    }

    ImportPlan plan;
    plan.id = base.value();
    const std::string carpeta = normalizaCarpeta(request.directory);
    plan.reference = carpeta.empty() ? (plan.id + ".png")
                                     : (carpeta + "/" + plan.id + ".png");
    plan.sourcePath = request.sourcePath;
    plan.absoluteDest = juntaRutas(request.assetsRoot, plan.reference);

    const Result<PngInfo> info = ReadPngInfo(request.sourcePath);
    if (!info.isOk()) {
        return R::Error(info.errorMessage());
    }
    plan.source = info.value();

    const Result<FrameGrid> grid = ComputeFrameGrid(request.frames,
                                                    plan.source.width,
                                                    plan.source.height);
    if (!grid.isOk()) {
        return R::Error(grid.errorMessage());
    }
    plan.grid = grid.value();

    plan.destFileExists = existeFichero(plan.absoluteDest);
    plan.idInCatalog = findById(plan.id) != nullptr;
    plan.collision = plan.destFileExists || plan.idInCatalog;
    return R::Ok(plan);
}

Result<std::string> AssetBrowserState::commitImport(const ImportPlan& plan,
                                                    ImportCollision onCollision) const {
    using R = Result<std::string>;
    if (plan.reference.empty() || plan.absoluteDest.empty() || plan.sourcePath.empty()) {
        return R::Error("el plan de importacion esta incompleto: vuelve a planearlo");
    }

    if (plan.collision) {
        if (onCollision == ImportCollision::Cancel) {
            return R::Error("importacion cancelada: ya existe " +
                            (plan.idInCatalog ? std::string("el recurso '") + plan.id + "'"
                                              : "el fichero " + plan.absoluteDest) +
                            "; no se ha escrito nada");
        }
        // Reutilizar no escribe nada: devuelve la referencia que ya existe,
        // que puede no ser la del plan si el recurso vive en otra carpeta.
        const AssetDescriptor* existente = findById(plan.id);
        if (existente != nullptr && !existente->reference.empty()) {
            return R::Ok(existente->reference);
        }
        return R::Ok(plan.reference);
    }

    // Entre planear y escribir puede haber aparecido el fichero o el id:
    // varios agentes trabajan a la vez sobre los mismos assets. Se vuelve a
    // comprobar aqui y se pierde la importacion antes que el original.
    if (existeFichero(plan.absoluteDest)) {
        return R::Error("entre planear y escribir ha aparecido " + plan.absoluteDest +
                        ": no se sobreescribe. Vuelve a planear la importacion");
    }
    if (findById(plan.id) != nullptr) {
        return R::Error("entre planear y escribir el catalogo ha registrado '" + plan.id +
                        "': no se sobreescribe. Vuelve a planear la importacion");
    }

    std::string contenido;
    if (!leeFichero(plan.sourcePath, contenido)) {
        return R::Error("no se ha podido leer el origen '" + plan.sourcePath +
                        "'; no se ha escrito nada");
    }

    const std::string temporal = plan.absoluteDest + ".tmp";
    {
        std::ofstream salida(temporal, std::ios::binary | std::ios::trunc);
        if (!salida) {
            return R::Error("no se puede escribir en " + temporal +
                            ". Si la carpeta '" + carpetaDe(plan.absoluteDest) +
                            "' no existe, creala o elige otra: este motor no crea carpetas");
        }
        salida.write(contenido.data(), static_cast<std::streamsize>(contenido.size()));
        salida.flush();
        if (!salida) {
            std::remove(temporal.c_str());
            return R::Error("no se ha podido escribir el temporal " + temporal +
                            "; no se ha importado nada");
        }
    }
    // Sin MOVEFILE_REPLACE_EXISTING a proposito: si el destino ha aparecido
    // entre la comprobacion y este punto, el movimiento falla y el original
    // sigue intacto, que es lo que el criterio de cierre pide.
    if (std::rename(temporal.c_str(), plan.absoluteDest.c_str()) != 0) {
        std::remove(temporal.c_str());
        return R::Error("no se ha podido mover el temporal a " + plan.absoluteDest +
                        "; no se ha importado nada");
    }
    return R::Ok(plan.reference);
}

}  // namespace Editor
