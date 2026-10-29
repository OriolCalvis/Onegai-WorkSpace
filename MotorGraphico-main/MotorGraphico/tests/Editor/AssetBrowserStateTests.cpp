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

// E-13 — Navegador de recursos e importacion, mitad sin interfaz.
//
// LO QUE DEFIENDE. El criterio de cierre del paquete pide cuatro cosas y
// cada una tiene aqui su prueba:
//
//   1. Busqueda por nombre y tipo con acentos plegados (DEC-15): el autor
//      escribe «mision» y encuentra «Misión del templo».
//   2. Seleccion por identidad que sobrevive al refresco: reenumerar el
//      catalogo no cambia lo que el autor tenia seleccionado, y si el
//      recurso desaparece se avisa (Lost) en vez de seleccionar al vecino.
//   3. Dimensiones y frames leidos del PNG sin tocar GL: la firma y el
//      IHDR se leen a mano; un PNG invalido se rechaza con palabras.
//   4. Importacion con referencia relativa y estable que NUNCA
//      sobreescribe: colision por fichero o por id => reutilizar o
//      cancelar; mover la carpeta del proyecto conserva el recurso.
//
// Los PNG de estas pruebas son sinteticos: firma + IHDR + unos bytes de
// relleno distintos por fichero, para poder afirmar que el original no se
// ha tocado. stb_image no se carga ningun PNG aqui (no hay GL).

#include "Editor/AssetBrowserState.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "TestHarness.h"

namespace {

namespace fs = std::filesystem;
using namespace Editor;

// ---------------------------------------------------------------------
// Andamiaje
// ---------------------------------------------------------------------

struct Fixture {
    fs::path root;
    Fixture() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned suffix = 0;; ++suffix) {
            root = fs::temp_directory_path() /
                ("motor asset browser " + std::to_string(stamp) + "-" + std::to_string(suffix));
            if (fs::create_directory(root)) break;
        }
    }
    ~Fixture() { std::error_code ignored; fs::remove_all(root, ignored); }
    std::string path(const std::string& name) const { return (root / name).string(); }
    std::string carpeta(const std::string& name) {
        const fs::path ruta = root / name;
        std::error_code ignored;
        fs::create_directories(ruta, ignored);
        return ruta.string();
    }
};

// Bytes de un PNG minimo: firma + IHDR con las dimensiones + relleno.
std::vector<unsigned char> pngBytes(int width, int height, unsigned char relleno = 0) {
    std::vector<unsigned char> bytes = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    auto be32 = [&bytes](unsigned int valor) {
        bytes.push_back(static_cast<unsigned char>((valor >> 24) & 0xFF));
        bytes.push_back(static_cast<unsigned char>((valor >> 16) & 0xFF));
        bytes.push_back(static_cast<unsigned char>((valor >> 8) & 0xFF));
        bytes.push_back(static_cast<unsigned char>(valor & 0xFF));
    };
    be32(13);                       // longitud del IHDR
    bytes.insert(bytes.end(), {'I', 'H', 'D', 'R'});
    be32(static_cast<unsigned int>(width));
    be32(static_cast<unsigned int>(height));
    bytes.push_back(8);             // profundidad de bit
    bytes.push_back(6);             // color type: RGBA
    bytes.push_back(0);             // compresion
    bytes.push_back(0);             // filtro
    bytes.push_back(0);             // entrelazado
    be32(0);                        // CRC (no se valida)
    for (int i = 0; i < 16; ++i) bytes.push_back(relleno);
    return bytes;
}

void escribeBytes(const fs::path& ruta, const std::vector<unsigned char>& bytes) {
    std::ofstream salida(ruta, std::ios::binary);
    if (!salida) throw std::runtime_error("no se pudo escribir " + ruta.string());
    salida.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    salida.close();
    if (!salida) throw std::runtime_error("fallo al escribir " + ruta.string());
}

// Un PNG en disco, con un relleno propio para poder distinguirlo.
std::string escribePng(const Fixture& f, const std::string& nombre, int width, int height,
                       unsigned char relleno = 0) {
    const fs::path ruta = f.root / nombre;
    std::error_code ignored;
    fs::create_directories(ruta.parent_path(), ignored);
    escribeBytes(ruta, pngBytes(width, height, relleno));
    return ruta.string();
}

std::string leeTexto(const fs::path& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) throw std::runtime_error("no se pudo leer " + ruta.string());
    return std::string(std::istreambuf_iterator<char>(entrada), std::istreambuf_iterator<char>());
}

AssetDescriptor recurso(const std::string& id, const std::string& name, AssetKind kind,
                        const std::string& reference, int width = 32, int height = 32) {
    AssetDescriptor asset;
    asset.id = id;
    asset.name = name;
    asset.kind = kind;
    asset.reference = reference;
    asset.width = width;
    asset.height = height;
    return asset;
}

// Catalogo con acentos, mayusculas y los cuatro tipos.
std::vector<AssetDescriptor> catalogo() {
    return {
        recurso("cofre_del_templo", "Cofre del templo", AssetKind::Sprite, "textures/cofre_del_templo.png"),
        recurso("mision_llave", "Misión de la llave", AssetKind::UI, "ui/mision_llave.png"),
        recurso("eter", "Éter", AssetKind::Sprite, "textures/eter.png"),
        recurso("baldosa_adobe", "Baldosa de adobe", AssetKind::Tile, "tiles/baldosa_adobe.png", 64, 64),
        recurso("marcador", "marcador", AssetKind::Unknown, "textures/marcador.png"),
    };
}

ImportRequest peticion(const Fixture& f, const std::string& origen, const std::string& nombre,
                       const std::string& raiz) {
    ImportRequest request;
    request.sourcePath = origen;
    request.visibleName = nombre;
    request.kind = AssetKind::Sprite;
    request.assetsRoot = raiz;
    request.directory = "textures";
    return request;
}

std::vector<std::string> idsVisibles(const std::vector<const AssetDescriptor*>& lista) {
    std::vector<std::string> ids;
    for (const AssetDescriptor* asset : lista) ids.push_back(asset->id);
    return ids;
}

bool contiene(const std::vector<std::string>& lista, const std::string& buscado) {
    for (const std::string& valor : lista) {
        if (valor == buscado) return true;
    }
    return false;
}

// ---------------------------------------------------------------------
// 3. Dimensiones y frames sin GL
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(png_info_lee_las_dimensiones_sin_gl) {
    const std::vector<unsigned char> bytes = pngBytes(48, 24);
    const Result<PngInfo> info = ParsePngInfo(bytes.data(), bytes.size());
    MOTOR_CHECK(info.isOk());
    MOTOR_CHECK(info.value().width == 48);
    MOTOR_CHECK(info.value().height == 24);

    Fixture f;
    const std::string ruta = escribePng(f, "origen/hoja.png", 64, 32, 7);
    const Result<PngInfo> desdeFichero = ReadPngInfo(ruta);
    MOTOR_CHECK(desdeFichero.isOk());
    MOTOR_CHECK(desdeFichero.value().width == 64);
    MOTOR_CHECK(desdeFichero.value().height == 32);
}

MOTOR_TEST_CASE(png_info_rechaza_lo_que_no_es_un_png_utilizable) {
    const std::vector<unsigned char> bueno = pngBytes(8, 8);

    std::vector<unsigned char> firmaMala = bueno;
    firmaMala[1] = 'X';
    MOTOR_CHECK(!ParsePngInfo(firmaMala.data(), firmaMala.size()).isOk());

    // Demasiado corto para llevar la firma.
    MOTOR_CHECK(!ParsePngInfo(bueno.data(), 4).isOk());

    // Firma correcta pero sin IHDR: truncado antes de las dimensiones.
    MOTOR_CHECK(!ParsePngInfo(bueno.data(), 12).isOk());

    // Primer chunk que no es IHDR.
    std::vector<unsigned char> chunkRaro = bueno;
    chunkRaro[12] = 'g';
    MOTOR_CHECK(!ParsePngInfo(chunkRaro.data(), chunkRaro.size()).isOk());

    // Dimensiones a cero: un PNG de 0x0 no se puede previsualizar.
    std::vector<unsigned char> cero = bueno;
    cero[19] = 0;  // byte alto del ancho ya es 0; se pone el bajo a 0
    cero[16] = 0;
    cero[17] = 0;
    cero[18] = 0;
    MOTOR_CHECK(!ParsePngInfo(cero.data(), cero.size()).isOk());

    // Puntero nulo: error, no excepción.
    MOTOR_CHECK(!ParsePngInfo(nullptr, 0).isOk());
}

MOTOR_TEST_CASE(png_info_de_un_fichero_que_no_existe_da_error_legible) {
    Fixture f;
    const Result<PngInfo> info = ReadPngInfo(f.path("no/existe.png"));
    MOTOR_CHECK(!info.isOk());
    // El mensaje nombra la ruta: es lo que el panel enseña al autor.
    MOTOR_CHECK(info.errorMessage().find("no/existe.png") != std::string::npos);

    // Un fichero que existe pero no es un PNG tampoco cuela.
    const fs::path texto = f.root / "notas.txt";
    { std::ofstream salida(texto); salida << "esto no es una imagen"; }
    const Result<PngInfo> noPng = ReadPngInfo(texto.string());
    MOTOR_CHECK(!noPng.isOk());
    MOTOR_CHECK(noPng.errorMessage().find("notas.txt") != std::string::npos);
}

MOTOR_TEST_CASE(frame_grid_divide_la_hoja_y_rechaza_celdas_partidas) {
    // Imagen suelta: una celda, un frame.
    const Result<FrameGrid> suelta = ComputeFrameGrid(FrameLayout{}, 32, 32);
    MOTOR_CHECK(suelta.isOk());
    MOTOR_CHECK(suelta.value().columns == 1);
    MOTOR_CHECK(suelta.value().rows == 1);
    MOTOR_CHECK(suelta.value().frameCount == 1);

    // Hoja de 64x32 con celdas de 16x16: 4 columnas, 2 filas, 8 frames.
    FrameLayout hoja;
    hoja.frameWidth = 16;
    hoja.frameHeight = 16;
    const Result<FrameGrid> grid = ComputeFrameGrid(hoja, 64, 32);
    MOTOR_CHECK(grid.isOk());
    MOTOR_CHECK(grid.value().columns == 4);
    MOTOR_CHECK(grid.value().rows == 2);
    MOTOR_CHECK(grid.value().frameCount == 8);

    // 5x5 sobre 32x32 no divide: se ve mejor al importar que en el juego.
    FrameLayout mala;
    mala.frameWidth = 5;
    mala.frameHeight = 5;
    const Result<FrameGrid> partida = ComputeFrameGrid(mala, 32, 32);
    MOTOR_CHECK(!partida.isOk());
    MOTOR_CHECK(partida.errorMessage().find("no divide") != std::string::npos);

    // Celda mayor que la imagen.
    FrameLayout enorme;
    enorme.frameWidth = 128;
    enorme.frameHeight = 128;
    MOTOR_CHECK(!ComputeFrameGrid(enorme, 32, 32).isOk());

    // Dimensiones de imagen imposibles.
    MOTOR_CHECK(!ComputeFrameGrid(hoja, 0, 32).isOk());
}

MOTOR_TEST_CASE(asset_kind_name_nombra_los_cuatro_tipos) {
    MOTOR_CHECK(std::string(AssetKindName(AssetKind::Tile)) == "baldosa");
    MOTOR_CHECK(std::string(AssetKindName(AssetKind::Sprite)) == "sprite");
    MOTOR_CHECK(std::string(AssetKindName(AssetKind::UI)) == "interfaz");
    MOTOR_CHECK(std::string(AssetKindName(AssetKind::Unknown)) == "desconocido");
}

// ---------------------------------------------------------------------
// 1. Busqueda y filtro
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(la_busqueda_pliega_acentos_y_mayusculas) {
    AssetBrowserState state;
    state.setAssets(catalogo());

    // Consulta vacia: todo el catalogo, en su orden.
    MOTOR_CHECK(state.visible().size() == 5);

    // «mision» sin acento encuentra «Misión de la llave» (DEC-15).
    state.setQuery("mision");
    MOTOR_CHECK(state.visible().size() == 1);
    MOTOR_CHECK(state.visible()[0]->id == "mision_llave");

    // Mayusculas y acentos plegados a la vez.
    state.setQuery("ÉTER");
    MOTOR_CHECK(state.visible().size() == 1);
    MOTOR_CHECK(state.visible()[0]->id == "eter");

    // Tambien se busca por id, que es lo que el autor ve en el inspector.
    state.setQuery("baldosa_adobe");
    MOTOR_CHECK(state.visible().size() == 1);

    // Una consulta sin resultado no rompe nada.
    state.setQuery("inexistente");
    MOTOR_CHECK(state.visible().empty());
}

MOTOR_TEST_CASE(el_filtro_por_tipo_y_la_consulta_se_combinan) {
    AssetBrowserState state;
    state.setAssets(catalogo());

    state.setKindFilter(AssetKind::Sprite);
    const std::vector<std::string> sprites = idsVisibles(state.visible());
    MOTOR_CHECK(sprites.size() == 2);
    MOTOR_CHECK(contiene(sprites, "cofre_del_templo"));
    MOTOR_CHECK(contiene(sprites, "eter"));

    // Filtro + consulta: solo el sprite que ademas encaja.
    state.setQuery("cofre");
    MOTOR_CHECK(state.visible().size() == 1);
    MOTOR_CHECK(state.visible()[0]->id == "cofre_del_templo");

    // Unknown = sin filtro de tipo.
    state.setKindFilter(AssetKind::Unknown);
    state.setQuery("");
    MOTOR_CHECK(state.visible().size() == 5);

    // Un tipo sin recursos da lista vacia, no todo el catalogo.
    state.setKindFilter(AssetKind::Tile);
    state.setQuery("eter");
    MOTOR_CHECK(state.visible().empty());
}

// ---------------------------------------------------------------------
// 2. Seleccion por identidad
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(la_seleccion_sobrevive_al_refresco_por_identidad) {
    AssetBrowserState state;
    state.setAssets(catalogo());
    MOTOR_CHECK(state.select("eter"));
    MOTOR_CHECK(state.selected() != nullptr);
    MOTOR_CHECK(state.selected()->id == "eter");

    // Reenumerar en otro orden: la seleccion sigue apuntando al mismo
    // recurso, no a la fila que ahora ocupa su sitio.
    std::vector<AssetDescriptor> alReves = catalogo();
    std::reverse(alReves.begin(), alReves.end());
    state.setAssets(alReves);

    MOTOR_CHECK(state.selected() != nullptr);
    MOTOR_CHECK(state.selected()->id == "eter");
    MOTOR_CHECK(state.lastSelectionChange() == AssetBrowserState::SelectionChange::Kept);

    // Buscar no es reenumerar, pero tampoco mueve la seleccion.
    state.setQuery("cofre");
    MOTOR_CHECK(state.selected()->id == "eter");

    state.clearSelection();
    MOTOR_CHECK(state.selected() == nullptr);
    MOTOR_CHECK(!state.select("no_existe"));
    MOTOR_CHECK(state.lastSelectionChange() == AssetBrowserState::SelectionChange::Lost);
}

MOTOR_TEST_CASE(la_seleccion_perdida_no_salta_al_vecino) {
    AssetBrowserState state;
    state.setAssets(catalogo());
    MOTOR_CHECK(state.select("cofre_del_templo"));

    // El recurso desaparece del catalogo (otro autor lo ha borrado).
    std::vector<AssetDescriptor> sinCofre;
    for (const AssetDescriptor& asset : catalogo()) {
        if (asset.id != "cofre_del_templo") sinCofre.push_back(asset);
    }
    state.setAssets(sinCofre);

    MOTOR_CHECK(state.selected() == nullptr);
    MOTOR_CHECK(state.lastSelectionChange() == AssetBrowserState::SelectionChange::Lost);

    // Y se puede decir: sin seleccion previa no hay nada que avisar.
    state.setAssets(catalogo());
    MOTOR_CHECK(state.lastSelectionChange() == AssetBrowserState::SelectionChange::None);
    MOTOR_CHECK(state.selected() == nullptr);

    // addAsset alimenta el catalogo sin tocar la seleccion.
    MOTOR_CHECK(state.select("eter"));
    state.addAsset(recurso("nuevo", "Nuevo recurso", AssetKind::UI, "ui/nuevo.png"));
    MOTOR_CHECK(state.assets().size() == 6);
    MOTOR_CHECK(state.selected()->id == "eter");
    MOTOR_CHECK(state.findById("nuevo") != nullptr);
}

// ---------------------------------------------------------------------
// 4. Importacion: plan, colision y copia
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(plan_import_genera_una_referencia_relativa_y_estable) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    const std::string origen = escribePng(f, "descargas/COFRE Bonito.png", 32, 16, 1);

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(plan.isOk());

    // El id sale del nombre visible con la regla de los proyectos (E-11).
    MOTOR_CHECK(plan.value().id == "cofre_del_templo");
    MOTOR_CHECK(plan.value().reference == "textures/cofre_del_templo.png");
    MOTOR_CHECK(plan.value().absoluteDest == raiz + "/textures/cofre_del_templo.png");

    // Relativa de verdad: ni raíz absoluta ni barras sobrantes.
    MOTOR_CHECK(fs::path(plan.value().reference).is_relative());
    MOTOR_CHECK(plan.value().reference.find(raiz) == std::string::npos);
    MOTOR_CHECK(plan.value().reference.front() != '/');

    // Dimensiones leidas del origen y plan sin escribir nada aun.
    MOTOR_CHECK(plan.value().source.width == 32);
    MOTOR_CHECK(plan.value().source.height == 16);
    MOTOR_CHECK(plan.value().grid.frameCount == 1);
    MOTOR_CHECK(!plan.value().collision);
    MOTOR_CHECK(!fs::exists(plan.value().absoluteDest));

    // Sin nombre visible no hay identidad, y se dice.
    ImportRequest sinNombre = peticion(f, origen, "", raiz);
    MOTOR_CHECK(!state.planImport(sinNombre).isOk());
    // Sin origen ni raiz, igual.
    MOTOR_CHECK(!state.planImport(peticion(f, "", "Cofre", raiz)).isOk());
    MOTOR_CHECK(!state.planImport(peticion(f, origen, "Cofre", "")).isOk());
    // Un nombre del que no sale ningun identificador es un error, no un id raro.
    MOTOR_CHECK(!state.planImport(peticion(f, origen, "¿?", raiz)).isOk());
}

MOTOR_TEST_CASE(plan_import_valida_los_frames_de_la_hoja) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    const std::string origen = escribePng(f, "descargas/heroe.png", 24, 24, 2);

    ImportRequest request = peticion(f, origen, "Héroe andando", raiz);
    request.frames.frameWidth = 8;
    request.frames.frameHeight = 12;   // 24/12 = 2 filas; 24/8 = 3 columnas

    AssetBrowserState state;
    const Result<ImportPlan> plan = state.planImport(request);
    MOTOR_CHECK(plan.isOk());
    MOTOR_CHECK(plan.value().grid.columns == 3);
    MOTOR_CHECK(plan.value().grid.rows == 2);
    MOTOR_CHECK(plan.value().grid.frameCount == 6);

    // Una hoja que no divide la imagen se rechaza ANTES de escribir nada.
    request.frames.frameWidth = 5;
    const Result<ImportPlan> mala = state.planImport(request);
    MOTOR_CHECK(!mala.isOk());
    MOTOR_CHECK(mala.errorMessage().find("no divide") != std::string::npos);
    MOTOR_CHECK(!fs::exists(plan.value().absoluteDest));
}

MOTOR_TEST_CASE(plan_import_rechaza_un_origen_que_no_es_png) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    const fs::path falso = f.root / "descargas" / "sprite.bmp";
    std::error_code ignored;
    fs::create_directories(falso.parent_path(), ignored);
    { std::ofstream salida(falso); salida << "BM no soy un png"; }

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, falso.string(), "Sprite falso", raiz));
    MOTOR_CHECK(!plan.isOk());
    MOTOR_CHECK(plan.errorMessage().find("PNG") != std::string::npos);

    // Y un origen que no existe tampoco.
    MOTOR_CHECK(!state.planImport(peticion(f, f.path("nada.png"), "Nada", raiz)).isOk());
}

MOTOR_TEST_CASE(plan_import_detecta_colision_por_fichero_y_por_id) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    f.carpeta("proyecto/textures");
    const std::string origen = escribePng(f, "descargas/cofre.png", 32, 32, 3);

    AssetBrowserState state;

    // Colision por fichero: ya hay algo en el destino.
    escribeBytes(fs::path(raiz) / "textures" / "cofre_del_templo.png", pngBytes(4, 4, 9));
    const Result<ImportPlan> porFichero =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(porFichero.isOk());
    MOTOR_CHECK(porFichero.value().destFileExists);
    MOTOR_CHECK(!porFichero.value().idInCatalog);
    MOTOR_CHECK(porFichero.value().collision);

    // Colision por id: el catalogo ya conoce el recurso aunque el fichero
    // viva en otra carpeta.
    fs::remove(fs::path(raiz) / "textures" / "cofre_del_templo.png");
    state.addAsset(recurso("cofre_del_templo", "Cofre del templo", AssetKind::Sprite,
                           "importado_antiguo/cofre.png"));
    const Result<ImportPlan> porId =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(porId.isOk());
    MOTOR_CHECK(!porId.value().destFileExists);
    MOTOR_CHECK(porId.value().idInCatalog);
    MOTOR_CHECK(porId.value().collision);
}

MOTOR_TEST_CASE(commit_import_copia_el_fichero_y_devuelve_la_referencia) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    f.carpeta("proyecto/textures");
    const std::vector<unsigned char> original = pngBytes(32, 32, 5);
    const fs::path origen = f.root / "descargas" / "llave.png";
    std::error_code ignored;
    fs::create_directories(origen.parent_path(), ignored);
    escribeBytes(origen, original);

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen.string(), "Llave del templo", raiz));
    MOTOR_CHECK(plan.isOk());

    const Result<std::string> commit = state.commitImport(plan.value(), ImportCollision::Reuse);
    MOTOR_CHECK(commit.isOk());
    MOTOR_CHECK(commit.value() == "textures/llave_del_templo.png");

    // El destino tiene los mismos bytes que el origen y se lee sin GL.
    const fs::path destino = fs::path(raiz) / commit.value();
    MOTOR_CHECK(fs::exists(destino));
    MOTOR_CHECK(leeTexto(destino) == leeTexto(origen));
    const Result<PngInfo> info = ReadPngInfo(destino.string());
    MOTOR_CHECK(info.isOk());
    MOTOR_CHECK(info.value().width == 32);

    // La copia no deja temporales sueltos.
    MOTOR_CHECK(!fs::exists(destino.string() + ".tmp"));
    // Y el origen sigue en su sitio: importar copia, no mueve.
    MOTOR_CHECK(fs::exists(origen));
}

MOTOR_TEST_CASE(commit_import_con_colision_reutiliza_sin_escribir) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    f.carpeta("proyecto/textures");
    const fs::path existente = fs::path(raiz) / "textures" / "cofre_del_templo.png";
    escribeBytes(existente, pngBytes(4, 4, 9));
    const std::string antes = leeTexto(existente);

    const std::string origen = escribePng(f, "descargas/cofre.png", 32, 32, 3);

    AssetBrowserState state;
    state.addAsset(recurso("cofre_del_templo", "Cofre del templo", AssetKind::Sprite,
                           "textures/cofre_del_templo.png"));

    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(plan.isOk());
    MOTOR_CHECK(plan.value().collision);

    const Result<std::string> reutilizar =
        state.commitImport(plan.value(), ImportCollision::Reuse);
    MOTOR_CHECK(reutilizar.isOk());
    MOTOR_CHECK(reutilizar.value() == "textures/cofre_del_templo.png");
    // El original no se ha tocado: ni un byte.
    MOTOR_CHECK(leeTexto(existente) == antes);
}

MOTOR_TEST_CASE(commit_import_con_colision_cancela_sin_escribir) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    f.carpeta("proyecto/textures");
    const fs::path existente = fs::path(raiz) / "textures" / "cofre_del_templo.png";
    escribeBytes(existente, pngBytes(4, 4, 9));
    const std::string antes = leeTexto(existente);
    const std::string origen = escribePng(f, "descargas/cofre.png", 32, 32, 3);

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(plan.isOk());
    MOTOR_CHECK(plan.value().collision);

    const Result<std::string> cancelar =
        state.commitImport(plan.value(), ImportCollision::Cancel);
    MOTOR_CHECK(!cancelar.isOk());
    MOTOR_CHECK(cancelar.errorMessage().find("cancelada") != std::string::npos);
    MOTOR_CHECK(leeTexto(existente) == antes);
}

MOTOR_TEST_CASE(commit_import_vuelve_a_comprobar_el_destino_antes_de_escribir) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");
    f.carpeta("proyecto/textures");
    const std::string origen = escribePng(f, "descargas/cofre.png", 32, 32, 3);

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(plan.isOk());
    MOTOR_CHECK(!plan.value().collision);

    // Entre planear y escribir aparece el fichero: otro autor, otro agente.
    const fs::path destino = fs::path(plan.value().absoluteDest);
    escribeBytes(destino, pngBytes(2, 2, 8));
    const std::string aparecido = leeTexto(destino);

    const Result<std::string> commit = state.commitImport(plan.value(), ImportCollision::Reuse);
    MOTOR_CHECK(!commit.isOk());
    MOTOR_CHECK(commit.errorMessage().find("no se sobreescribe") != std::string::npos);
    MOTOR_CHECK(leeTexto(destino) == aparecido);

    // Lo mismo si lo que aparece entre medias es el id en el catalogo.
    AssetBrowserState state2;
    const Result<ImportPlan> plan2 =
        state2.planImport(peticion(f, escribePng(f, "descargas/eter.png", 8, 8, 4),
                                   "Éter grande", raiz));
    MOTOR_CHECK(plan2.isOk());
    MOTOR_CHECK(!plan2.value().collision);
    state2.addAsset(recurso(plan2.value().id, "Éter grande", AssetKind::Sprite,
                            "textures/eter_grande.png"));
    MOTOR_CHECK(!state2.commitImport(plan2.value(), ImportCollision::Reuse).isOk());
    MOTOR_CHECK(!fs::exists(plan2.value().absoluteDest));
}

MOTOR_TEST_CASE(commit_import_sin_carpeta_de_destino_da_un_error_legible) {
    Fixture f;
    const std::string raiz = f.carpeta("proyecto");   // sin textures/ dentro
    const std::string origen = escribePng(f, "descargas/cofre.png", 32, 32, 3);

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen, "Cofre del templo", raiz));
    MOTOR_CHECK(plan.isOk());

    const Result<std::string> commit = state.commitImport(plan.value(), ImportCollision::Reuse);
    MOTOR_CHECK(!commit.isOk());
    MOTOR_CHECK(commit.errorMessage().find("no crea carpetas") != std::string::npos);
    MOTOR_CHECK(!fs::exists(plan.value().absoluteDest));
}

MOTOR_TEST_CASE(commit_import_con_un_plan_incompleto_no_escribe) {
    AssetBrowserState state;
    ImportPlan vacio;
    MOTOR_CHECK(!state.commitImport(vacio, ImportCollision::Reuse).isOk());
    MOTOR_CHECK(!state.commitImport(vacio, ImportCollision::Cancel).isOk());
}

MOTOR_TEST_CASE(la_referencia_relativa_sobrevive_a_mover_el_proyecto) {
    Fixture f;
    const std::string raizA = f.carpeta("proyecto_viejo");
    f.carpeta("proyecto_viejo/textures");
    const std::string origen = escribePng(f, "descargas/cofre.png", 40, 20, 6);

    AssetBrowserState state;
    const Result<ImportPlan> plan =
        state.planImport(peticion(f, origen, "Cofre del templo", raizA));
    MOTOR_CHECK(plan.isOk());
    const Result<std::string> commit = state.commitImport(plan.value(), ImportCollision::Reuse);
    MOTOR_CHECK(commit.isOk());
    const std::string reference = commit.value();

    // El autor mueve (o renombra) la carpeta del proyecto entero.
    const fs::path raizB = f.root / "proyecto_nuevo";
    std::error_code ignored;
    fs::rename(raizA, raizB, ignored);
    MOTOR_CHECK(fs::exists(raizB));

    // La referencia guardada sigue valiendo contra la raiz de HOY.
    const fs::path movido = raizB / reference;
    MOTOR_CHECK(fs::exists(movido));
    const Result<PngInfo> info = ReadPngInfo(movido.string());
    MOTOR_CHECK(info.isOk());
    MOTOR_CHECK(info.value().width == 40);
    MOTOR_CHECK(info.value().height == 20);

    // Y al reabrir, el catalogo alimentado desde la raiz nueva resuelve igual.
    AssetBrowserState reabierto;
    AssetDescriptor asset = recurso(plan.value().id, "Cofre del templo", AssetKind::Sprite,
                                    reference);
    asset.width = info.value().width;
    asset.height = info.value().height;
    reabierto.addAsset(asset);
    MOTOR_CHECK(reabierto.findById("cofre_del_templo") != nullptr);
    MOTOR_CHECK(reabierto.findById("cofre_del_templo")->reference == reference);
    MOTOR_CHECK(fs::exists(raizB / reabierto.findById("cofre_del_templo")->reference));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
