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

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

// E-13 — Navegador de recursos e importación. MITAD SIN INTERFAZ.
//
// QUÉ RESUELVE. Las listas con búsqueda ya ayudan a encontrar IDs, pero
// no dejan incorporar un sprite propio desde la misma herramienta: hoy el
// autor tiene que salir del editor, copiar el PNG a mano a una carpeta y
// escribir la ruta. Esto es el modelo de ese navegador —buscar por nombre
// y tipo, previsualizar dimensiones y frames, e importar un PNG al
// proyecto—; el panel ImGui que lo dibuja (editor/src/AssetBrowserPanel.cpp)
// no está aquí a propósito, igual que E-15 deja su panel a E-06: el estado
// abajo y sin GL, el dibujo arriba. Ver la nota de ubicación al final.
//
// LA REFERENCIA ES RELATIVA Y ESTABLE. Es el criterio de cierre: «moure la
// carpeta del projecte i reobrir conserva el recurs». Un asset se guarda
// como una ruta RELATIVA a la raíz del proyecto ("textures/cofre.png"), no
// absoluta. Así, mover o renombrar la carpeta del proyecto no rompe la
// referencia: quien la usa la resuelve contra la raíz de HOY. El id y la
// referencia salen del nombre visible con la MISMA regla que los proyectos
// (ProjectIndex::suggestId, E-11): el autor escribe «Cofre del templo» y la
// identidad interna es "cofre_del_templo", que es lo que no cambia nunca.
//
// NUNCA SOBREESCRIBE. Importar con un nombre que ya existe —o a una ruta
// que ya tiene fichero— es una colisión, y una colisión se resuelve
// reutilizando lo que ya hay o cancelando; jamás pisando el original. Perder
// el PNG de otro autor porque alguien importó encima es exactamente la clase
// de daño que el editor no puede deshacer. La comprobación se repite en el
// commit, no sólo al planear: entre planear y escribir el fichero puede
// haber aparecido (varios agentes trabajan a la vez sobre los mismos assets).
//
// LA SELECCIÓN ES POR IDENTIDAD, NO POR POSICIÓN. Como el foco de E-15: la
// lista se recalcula al buscar, filtrar o reenumerar, y si la selección
// fuera un número de fila, el autor acabaría con otro asset seleccionado
// cada vez que la lista cambia. Se guarda el id y se vuelve a resolver; si
// ya no está, se dice (Lost) en vez de seleccionar al vecino.
//
// GL-FREE. Las dimensiones y los frames de la previsualización salen de la
// cabecera IHDR del PNG, leída a mano: no se carga la textura ni se toca
// stb_image (que vive en la ruta GL de TextureManager). Y no se usa
// <filesystem> en producción, igual que ProjectIndex: la existencia se
// comprueba abriendo el fichero y la copia se hace con flujos.

namespace Editor {

// ---------------------------------------------------------------------
// Cabecera PNG (sin GL)
// ---------------------------------------------------------------------

struct PngInfo {
    int width = 0;
    int height = 0;
};

// Lee las dimensiones de la firma + IHDR. Pura: no toca el disco. Error si
// no es un PNG, si falta la cabecera o si las dimensiones no son positivas.
// No valida el CRC ni recorre los chunks: sólo lee lo que la previsualización
// necesita. La validez completa del PNG la comprueba stb_image al cargarlo.
Result<PngInfo> ParsePngInfo(const unsigned char* bytes, std::size_t n);

// Igual, leyendo sólo los primeros bytes del fichero. Error legible si el
// fichero no existe o no se puede leer, nunca una excepción.
Result<PngInfo> ReadPngInfo(const std::string& path);

// ---------------------------------------------------------------------
// Tipo y frames
// ---------------------------------------------------------------------

// De qué es el recurso. El autor filtra por esto («tiles i sprites»).
enum class AssetKind { Tile, Sprite, UI, Unknown };
const char* AssetKindName(AssetKind kind);

// Recorte de un spritesheet. frameWidth/Height a 0 => imagen suelta (no hoja).
struct FrameLayout {
    int frameWidth = 0;
    int frameHeight = 0;
    bool isSheet() const { return frameWidth > 0 && frameHeight > 0; }
};

struct FrameGrid {
    int columns = 1;
    int rows = 1;
    int frameCount = 1;
};

// Cuántas celdas caben en la imagen. Error si la hoja no divide la imagen en
// entero (una celda partida a medias es un sprite roto y se ve mejor al
// importar que dentro del juego). Una imagen suelta da 1x1.
Result<FrameGrid> ComputeFrameGrid(const FrameLayout& layout, int imageWidth, int imageHeight);

// ---------------------------------------------------------------------
// Un recurso
// ---------------------------------------------------------------------

struct AssetDescriptor {
    std::string id;          // identidad estable (slug); no cambia nunca
    std::string name;        // nombre visible, con acentos si se quiere (DEC-15)
    AssetKind kind = AssetKind::Unknown;
    std::string reference;   // ruta RELATIVA a la raíz del proyecto
    int width = 0;
    int height = 0;
    FrameLayout frames;
};

// ---------------------------------------------------------------------
// Importación
// ---------------------------------------------------------------------

struct ImportRequest {
    std::string sourcePath;   // absoluta: la que eligió el autor en el diálogo
    std::string visibleName;  // -> id y referencia estables (suggestId)
    AssetKind kind = AssetKind::Unknown;
    FrameLayout frames;       // hoja opcional
    std::string assetsRoot;   // raíz del proyecto
    std::string directory = "textures";  // carpeta relativa dentro del proyecto
};

struct ImportPlan {
    std::string id;
    std::string reference;     // directory/id.png — relativa y estable
    std::string sourcePath;    // para copiar en el commit
    std::string absoluteDest;  // assetsRoot/reference
    PngInfo source;            // dimensiones leídas del PNG de origen
    FrameGrid grid;            // recorte validado
    bool destFileExists = false;  // ya hay un fichero en el destino
    bool idInCatalog = false;     // ya hay un asset con este id
    bool collision = false;       // destFileExists || idInCatalog
};

// Qué decide el autor cuando el plan avisa de colisión. Reutilizar devuelve
// la referencia existente sin escribir nada; cancelar no hace nada.
enum class ImportCollision { Reuse, Cancel };

class AssetBrowserState {
public:
    AssetBrowserState() = default;

    // --- Catálogo ---
    // Lo alimenta quien enumera (el panel o ProjectIndex); el estado NO
    // escanea el disco (sin <filesystem>, igual que ProjectIndex).
    void setAssets(std::vector<AssetDescriptor> assets);
    void addAsset(AssetDescriptor asset);
    const std::vector<AssetDescriptor>& assets() const { return m_assets; }
    const AssetDescriptor* findById(const std::string& id) const;

    // --- Búsqueda y filtro ---
    // La consulta pliega acentos y mayúsculas (DEC-15: el autor escribe
    // «mision» y encuentra «Misión»). Consulta vacía = todo. Tipo Unknown =
    // sin filtro de tipo.
    void setQuery(const std::string& query);
    const std::string& query() const { return m_query; }
    void setKindFilter(AssetKind kind);
    AssetKind kindFilter() const { return m_kindFilter; }
    // Visibles tras consulta+filtro, en el orden estable del catálogo.
    std::vector<const AssetDescriptor*> visible() const;

    // --- Selección por identidad ---
    enum class SelectionChange { None, Kept, Lost };
    bool select(const std::string& id);
    const AssetDescriptor* selected() const;
    void clearSelection();
    SelectionChange lastSelectionChange() const { return m_lastSelectionChange; }

    // --- Importación ---
    // Planea: lee dimensiones del origen, genera id y referencia estables,
    // valida los frames y detecta colisión. NO escribe nada.
    Result<ImportPlan> planImport(const ImportRequest& request) const;
    // Escribe: copia origen -> destino. Con colisión, Reuse devuelve la
    // referencia existente sin tocar el original y Cancel se niega. Sin
    // colisión, copia. Vuelve a comprobar el destino justo antes de escribir.
    Result<std::string> commitImport(const ImportPlan& plan,
                                     ImportCollision onCollision) const;

private:
    // Vuelve a resolver la seleccion contra el catalogo de hoy: si el id
    // sigue ahi, Kept; si ya no esta, Lost y sin seleccion (nunca al vecino
    // que ocupe su sitio). Lo llama quien cambia el catalogo entero.
    void resolverSeleccion();

    std::vector<AssetDescriptor> m_assets;
    std::string m_query;
    AssetKind m_kindFilter = AssetKind::Unknown;
    std::string m_selectedId;
    bool m_hasSelection = false;
    SelectionChange m_lastSelectionChange = SelectionChange::None;
};

}  // namespace Editor

// NOTA DE UBICACIÓN. Esto vive en include/Editor + src/Editor y no en
// editor/, que es la capa ImGui. Tal cual está no toca GL, así que se
// prueba sin ventana; el panel de E-13 (AssetBrowserPanel.cpp) lo consumirá
// desde arriba. Mismo reparto que E-15 (ProblemsState) y E-11 (ProjectIndex):
// el estado abajo y sin GL, el dibujo arriba.
//
// Su .cpp se compila dentro de cmake/AssetBrowserStateTests.cmake y todavía
// no dentro de ninguna librería. Es deliberado: meterlo en motor_level obliga
// a editar el bloque compartido del CMakeLists, que varios agentes tocan a la
// vez. Cuando el panel lo necesite enlazado, ese paquete lo mueve.
