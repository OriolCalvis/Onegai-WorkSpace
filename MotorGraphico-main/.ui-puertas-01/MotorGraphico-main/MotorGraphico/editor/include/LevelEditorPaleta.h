// EST-21 (relevo A) — EL VOCABULARIO DE LA PALETA DEL EDITOR.
//
// Todo lo que traduce DATOS a lo que una persona ve en la paleta: el
// nombre legible de un escenario, la semantica de un tile, el icono de
// un objeto, el filtro de una familia. Ni una linea de OpenGL: por eso
// esta aqui y no dentro de las 2777 lineas de level_editor.cpp, donde
// no se podia probar sin abrir una ventana.
//
// LA REGLA QUE ORDENA ESTE FICHERO: las rutas y los identificadores son
// necesarios para el motor, pero no son un lenguaje de autoria.
// "interior_taberna.json" se ensena como "Interior taberna", y un gid
// sin semantica declarada cae en un nombre de reserva en vez de ensenar
// el numero. Quien edita un nivel no deberia tener que leer rutas.

#pragma once

#include <functional>
#include <initializer_list>
#include <string>
#include <unordered_map>

#include "Core/Resources/TextureAtlas.h"
#include "Editor/EditorState.h"
#include "Level/ObjectCatalog.h"

namespace EditorNiveles {

struct TilePaletteInfo {
    std::string name;
    std::string affinity;
    std::string effect;
    std::string requirement;
    std::string tag;
    bool collision = false;
};

// Un sprite editorial por raza. El catálogo conserva raceId como fuente de
// verdad; esta tabla solo decide en qué atlas/celda se ve en el editor.
struct RaceNpcPreview {
    int atlas = 0;  // 1..4; 0 = usar icono PNJ genérico
    int frame = 0;
};

using ObjectFilter = std::function<bool(const ObjectDefinition&)>;

const char* toolName(EditorTool tool);

// Las rutas son necesarias para el motor, pero no son un lenguaje de
// autoría. La UI siempre muestra el nombre que una persona reconoce:
// "interior_taberna.json" pasa a ser "Interior taberna".
std::string nombreEscenario(const std::string& path);

std::string textoDeArchivo(const std::string& path);

TilePaletteInfo semanticasTile(int gid, const std::string& name, bool collision);

std::unordered_map<int, TilePaletteInfo> cargarSemanticaTiles(const std::string& tilesetPath);

bool contieneCualquiera(const std::string& value,
                        std::initializer_list<const char*> tokens);

ObjectFilter filtroAfinidadObjeto(const std::string& group);

const char* nombreCategoria(ObjectCategory category);

std::string nombreDePaleta(const ObjectCatalog& catalog, const std::string& id);

// Posiciones 1..16 del atlas editor_object_icons.png. No son sprites de
// combate: son pictogramas de autoría, consistentes aunque un objeto aún no
// tenga arte final. Así una puerta, un PNJ o una llave se reconocen antes
// de leer una sola letra.
int iconoEditor(const ObjectDefinition& definition);

// Frame de la biblioteca visual de enemigos (4 columnas x 3 filas), o 0
// cuando el objeto usa el pictograma editorial general. Mantener este
// mapeo por ID evita depender de nombres traducidos: el autor puede
// renombrar un enemigo sin cambiar cómo se reconoce sobre el mapa.
int iconoBibliotecaEnemigos(const ObjectDefinition& definition);

RaceNpcPreview previewNpcRacial(const ObjectDefinition& definition);

// TextureManager carga los PNG con Y invertida. Los previews se envían
// directamente al SpriteBatch (no pasan por Entity), por lo que necesitan
// invertir V aqui para que cabeza y pies no queden al revés.
UVRect uvPreviewPng(const TextureAtlas& atlas, int frame);

// La barra de estado va en mayusculas (BitmapFont solo tiene caja alta).
std::string aMayus(const std::string& t);

}  // namespace EditorNiveles
