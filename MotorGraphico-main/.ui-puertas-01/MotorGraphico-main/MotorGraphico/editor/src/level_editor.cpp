// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 10 paquetes del corte B:
//   C-14  (fase C, 6 j) - L'editor treballa sobre MapDocument
//   C-15  (fase C, 2 j) - Desar deixa el projecte coherent
//   D-05  (fase D, 3 j) - Objectes, equipament i consumibles
//   E-07  (fase E, 4 j) - Validació estàtica d'aventures
//   E-10  (fase E, 6 j) - Espai de treball visual i controls coherents
//   F-01  (fase F, 4 j) - Playtest des de l'editor
//   F-02  (fase F, 2 j) - Provar des de la casella seleccionada
//   G-01  (fase G, 2 j) - Autosave i recuperació
//   G-02  (fase G, 3 j) - Les eines d'edició que falten
//   G-05  (fase G, 4 j) - Tutorial de l'editor
//
// AVISO - FICHERO DE CRUCE. Con 10 paquetes encima, es de los
// sitios donde dos agentes se pisan sin enterarse. Antes de una
// refactorizacion grande aqui, mira si hay trabajo sin commitear
// (git status) y di en que paquete estas trabajando.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// Fase 12 (motor_grafico_gantt_rpg.puml): editor de niveles visual.
// INF-03: esta aplicación deja de ser un vertical slice de examples/.
//
// UI con el HUD PROPIO del motor (Fase 9: HudPanel/HudText/BitmapFont),
// NO con Dear ImGui como planeaba el Gantt: sin red en el entorno de
// desarrollo no se puede vendorizar (ver la nota de alcance en
// EditorState.h -- el nucleo del editor esta separado de la UI
// justamente para poder migrar a ImGui algun dia sin tocarlo).
//
// Controles:
//   Raton:  click izq  = aplicar herramienta en la celda bajo el cursor
//           click der  = sobre un OBJETO: abre el panel de PROPIEDADES
//                        de esa instancia (modal: W/S navegar, ENTER
//                        editar/elegir, X quitar botin/propiedad, ESC
//                        cerrar; SPACE inserta un espacio al editar texto.
//                        El boton derecho tambien lo cierra).
//                        Sobre una celda sin objeto: la herramienta
//                        "contraria" rapida (borrar tile / quitar objeto)
//   1..7    = herramienta, en el orden del enum EditorTool:
//             1 pintar tile     2 borrar tile     3 colocar objeto
//             4 quitar objeto   5 inicio jugador  6 rellenar
//             7 enlazar nivel (la puerta que lleva a otro escenario)
//   Q/E     = navegar la paleta activa; en objetos recorre el arbol y luego
//             los recursos concretos (tiles u objetos, segun herramienta)
//   3       = abrir el panel de familias de objetos
//   ENTER   = entrar en una familia; ESC/BACKSPACE = cerrar y volver
//   [ / ]   = pagina anterior / siguiente dentro del menu de objetos
//             (sube/baja familias enteras cuando la lista es larga)
//   B       = BUSCAR en la paleta. Abre un campo y filtra por texto:
//             "taber" deja tabernero, taberna... ENTER/ESC/B lo cierra.
//             Con miles de entradas, pasarlas de una en una no es una
//             opcion. BACKSPACE borra.
//   C       = capa siguiente (SHIFT+C anterior), CTRL+N capa nueva.
//             Las herramientas de tile actuan solo sobre la capa activa;
//             los objetos y el inicio del jugador son del nivel.
//   R       = marcar una esquina del rectangulo; R otra vez lo cierra.
//             Con teclado y no arrastrando porque el boton izquierdo ya
//             pinta: un modificador de arrastre seria pintar sin querer
//             cada vez que se falle.
//   CTRL+C / CTRL+V / CTRL+X = copiar la seleccion / pegarla en la celda
//             bajo el cursor / borrarla. Pegar y borrar son UN paso de
//             historial aunque toquen mil celdas.
//   T       = darle al objeto bajo el cursor la patrulla del rectangulo
//             marcado. Reutiliza la seleccion en vez de pedir cuatro
//             numeros a mano.
//   H       = ciclar la escala del objeto bajo el cursor (50/75/100/150/200%).
//   Ctrl+Z / Ctrl+Y = deshacer / rehacer
//   WASD    = pan de camara (SHIFT = x4, para mapas grandes)
//   +/-     = zoom (0 = volver a 1x), HOME = centrar en el mapa
//   G       = guardar en los ficheros DEL PROYECTO abierto (rutas
//             relativas al cwd: ejecutar desde build/ o MotorGraphico/)
//   V       = validar el nivel sin salir del editor
//   P       = probar: guarda y lanza ./juego sobre este nivel
//   , / .   = escenario anterior / siguiente del proyecto (guarda antes;
//             dan la vuelta al llegar al final de la lista)
//   M       = volver a la pantalla de proyectos (ESC hace lo mismo)
//
//   F5/F6/F7/F8/F9 siguen valiendo como alias de G/V/P/,/. Las teclas de
//   funcion en un portatil Mac son teclas de medios: F5 de verdad pide
//   Fn+F5, que con las dos manos ocupadas en el mapa no es una tecla, es
//   una maniobra. Por eso manda la letra y la tecla de funcion es el
//   alias, y no al reves.
//
// Y la pantalla de proyectos, por la que se arranca:
//   W/S = mover   ENTER = abrir   N = nuevo   B = compilar   ESC = cerrar
//
// Tamano del mapa (los niveles NO tienen por que ser todos iguales: un
// mundo puede repartirse en varios mapas pequenos, cada uno con su JSON
// y su TMX):
//   ./level_editor                      8x8 en blanco (por defecto)
//   ./level_editor --size 64x64         mapa nuevo del tamano que sea
//   ./level_editor --load assets/levels/ejemplo_nivel.json
//                                       abre uno existente para seguir
//                                       editandolo (sus dimensiones las
//                                       manda el TMX cargado)
//
// GL-dependiente: en este entorno de trabajo solo se puede syntax-check
// (-fsyntax-only); el nucleo (EditorState, exportadores) SI esta
// verificado de verdad en demo_editor_state.cpp, incluidos los
// round-trips contra TileMap/LevelLoader reales. En una maquina con GLFW
// (el Mac del usuario), CMake construye "level_editor" como cualquier
// otro target GL.
//
// Con un argumento numerico corre N frames y sale (modo humo para CI,
// igual que los demos GL): comprueba glGetError()==0 tras renderizar y
// vuelca el ultimo frame a level_editor_output.ppm, para poder revisar
// el HUD sin una captura de pantalla a mano.
#include "Core/Math/IsoMath.h"
#include "Render/CapturaGL.h"
#include "Core/Json/JsonValue.h"
#include <cstdio>
#include <memory>

#include "Editor/ProjectHub.h"
#include "Project/ProjectIndex.h"
#include "Core/Math/Vector4.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/Window.h"
#include "Render/BitmapFont.h"
#include "Render/Camera.h"
#include "Editor/EditorState.h"
#include "LevelEditorPaleta.h"
#include "LevelEditorOpciones.h"
#include "LevelEditorArranque.h"
#include "LevelEditorEditState.h"
#include "LevelEditorHub.h"
#include "LevelEditorInterno.h"
#include "Editor/EditorDocumentIO.h"
#include "Import/TmxMapImporter.h"
#include "Level/LevelDocumentBridge.h"
#include "Project/ProjectSerializer.h"
#include "Project/ProjectCatalog.h"
#include <filesystem>
#include "Editor/EditorValidation.h"
#include "Editor/ObjectInspector.h"
#include "Render/HudElement.h"
#include "Render/HudManager.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Level/LevelDefinition.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "Render/SpriteBatch.h"
#include "Render/TileMap.h"

#include "Core/Check.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp):
// glad.h define el guard de GL y glfw3.h se salta su #include <GL/gl.h>.
#include <glad/glad.h>

#include <GLFW/glfw3.h>

// EST-21 — el vocabulario de la paleta y las opciones de arranque
// viven fuera desde que este fichero se partio: son el nucleo del
// editor que NO necesita ventana, y ahora tienen prueba propia.
using namespace EditorNiveles;

namespace {

constexpr int kTileW = 64;
constexpr int kTileH = 32;


}  // namespace


int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    EditorOptions opts = parseArgs(argc, argv);
    const int maxFrames = opts.maxFrames;

    if (opts.listProjects || !opts.newProjectId.empty()) {
        return pantallaDeArranque(opts);
    }

    // Con --proyecto, el nivel a abrir y el destino de guardado salen del
    // manifiesto. Antes F5 escribia SIEMPRE en assets/maps/editor_map.tmx:
    // editabas Boundington y el trabajo acababa en un fichero que no era de
    // nadie, y a la siguiente sesion lo pisabas.
    std::string saveTmx = "assets/maps/editor_map.tmx";
    std::string saveJson = "assets/levels/editor_level.json";
    std::string saveName = "Nivel del editor";

    // Se resuelve en una lambda porque ahora hay DOS momentos en que se
    // sabe el proyecto: por --proyecto antes de abrir la ventana, o por la
    // pantalla de proyectos, que ya necesita contexto GL para pintarse.
    auto resolverProyecto = [&]() -> bool {
        auto r = Editor::ProjectIndex::scan("assets");
        if (!r.isOk()) {
            std::cerr << "No se pudo leer assets/proyectos: " << r.errorMessage() << "\n";
            return false;
        }
        const Editor::Project* p = r.value().find(opts.projectId);
        if (p == nullptr) {
            std::cerr << "No existe el proyecto '" << opts.projectId
                      << "'. Prueba: ./level_editor --proyectos\n";
            return false;
        }
        std::string nivel = opts.levelName;
        if (nivel.empty() && !p->levels.empty()) {
            nivel = p->levels.front();
        }
        if (!nivel.empty()) {
            opts.loadLevelPath = "assets/levels/" + nivel;
            saveJson = opts.loadLevelPath;
            saveName = p->name;
            const std::string base = nivel.substr(0, nivel.find_last_of('.'));
            saveTmx = "assets/maps/" + base + ".tmx";
        } else {
            // Proyecto recien creado: primer nivel, nombre con su prefijo.
            const std::string base = p->prefix + "nivel_1";
            saveTmx = "assets/maps/" + base + ".tmx";
            saveJson = "assets/levels/" + base + ".json";
            saveName = p->name;
        }
        std::cout << "Proyecto '" << p->id << "' (" << p->name << ", " << p->epoch << ")\n"
                  << "  niveles: " << p->levels.size() << "  ·  guardara en " << saveJson << "\n";
        return true;
    };

    if (!opts.projectId.empty() && !resolverProyecto()) {
        return 1;
    }

    try {
        Window window(width, height, "Motor Grafico - Editor de niveles (Fase 12)");
        std::cout << "Contexto OpenGL creado correctamente.\n";

        ShaderManager shaderManager;
        auto spriteShader = shaderManager.load("sprite", "assets/shaders/sprite");
        if (!spriteShader.isOk()) {
            std::cerr << "Error cargando shader sprite: " << spriteShader.errorMessage() << "\n";
            return 1;
        }

        // El editor tiene DOS fases y se puede ir y volver entre ellas:
        // la pantalla de proyectos y la edicion. ESC en la edicion vuelve
        // aqui en vez de cerrar el programa -- cerrar del todo es ESC otra
        // vez, ya en la pantalla. Cambiar de proyecto no deberia costar
        // relanzar el binario.
        //
        // Cada vuelta rehace tileset, mapa y HUD a proposito: dos
        // proyectos no comparten tileset, y reaprovecharlos era la via
        // rapida para acabar editando Boundington con la paleta de otro.
        bool cerrarDelTodo = false;
        while (!cerrarDelTodo && !window.shouldClose()) {

        // =============================================================
        // FASE 1 — LA PANTALLA DE PROYECTOS
        //
        // Va antes de cargar nada porque es la que decide QUE se carga.
        // Tiene sus propios batch/font/textura en un ambito cerrado: son
        // baratos y asi el editor no arranca con recursos que solo hacen
        // falta en el menu.
        //
        // Toda la logica (que hay marcado, que pasa con cada tecla, que
        // dijo el build) vive en Editor::ProjectHub, GL-free y probada en
        // demo_proyectos. Aqui solo se pinta lo que diga y se le pasan las
        // teclas traducidas a caracteres. Ese reparto es a proposito: la
        // primera version de esta pantalla era un printf a stdout, y lo
        // fue porque lo que se mete en este fichero no se puede probar.
        // =============================================================
        if (opts.projectId.empty() && opts.loadLevelPath.empty() && maxFrames < 0) {
            const SalidaDelHub salida = pantallaDeProyectos(
                window, opts, cerrarDelTodo, *spriteShader.value(), width, height);
            if (salida == SalidaDelHub::Error) {
                return 1;
            }
            if (salida == SalidaDelHub::Salir) {
                break;   // ESC en la pantalla, o cerraron la ventana
            }
            if (!resolverProyecto()) {
                return 1;
            }
        }

        // El tileset se decide DESPUES de saber que mapa se edita (con
        // --load, el TMX declara el suyo): ver mas abajo, tras la carga.
        TextureManager textureManager;

        // La paleta y el playtest comparten las rutas del proyecto activo.
        ObjectCatalog catalog;
        std::vector<std::string> catalogPaths;
        if (opts.projectId.empty()) {
            // Modo suelto de pruebas, fuera de cualquier proyecto publicado.
            catalogPaths = {"assets/objects/test_objects.json"};
            const auto loaded = catalog.loadFromFile(catalogPaths.front());
            if (!loaded.isOk()) {
                std::cerr << "Error cargando catalogo: " << loaded.errorMessage() << "\n";
                return 1;
            }
        } else {
            const auto index = Editor::ProjectIndex::scan("assets");
            if (!index.isOk()) {
                std::cerr << "Error cargando proyectos: " << index.errorMessage() << "\n";
                return 1;
            }
            const auto* project = index.value().find(opts.projectId);
            if (!project) {
                std::cerr << "Proyecto no encontrado: " << opts.projectId << "\n";
                return 1;
            }
            auto loaded = Editor::ProjectCatalog::load(*project);
            if (!loaded.isOk()) {
                std::cerr << loaded.errorMessage() << "\n";
                return 1;
            }
            catalog = std::move(loaded.value().objects);
            catalogPaths = std::move(loaded.value().paths);
        }

        // Mapa nuevo del tamano pedido, o el nivel que se haya indicado
        // con --load (su TMX manda las dimensiones: abrir un nivel no
        // puede recortarlo en silencio).
        std::string levelTitle = "NIVEL NUEVO";
        int mapW = opts.mapWidth;
        int mapH = opts.mapHeight;
        TileMap loadedMap;
        LevelDefinition loadedLevel;
        bool loaded = false;
        MapDocument loadedDocument;

        if (!opts.loadLevelPath.empty()) {
            auto levelRes = LevelLoader::loadFromFile(opts.loadLevelPath);
            if (!levelRes.isOk()) {
                std::cerr << "Error cargando nivel: " << levelRes.errorMessage() << "\n";
                return 1;
            }
            loadedLevel = levelRes.value();
            auto mapRes = loadedMap.loadFromFile(loadedLevel.mapPath);
            if (!mapRes.isOk()) {
                std::cerr << "Error cargando mapa " << loadedLevel.mapPath << ": "
                          << mapRes.errorMessage() << "\n";
                return 1;
            }
            if (!loadedLevel.mapDocumentPath.empty()) {
                auto result = ProjectSerializer::loadMapFile(loadedLevel.mapDocumentPath);
                if (!result.isOk()) {
                    std::cerr << "Error cargando documento de autoría: " << result.errorMessage() << "\n";
                    return 1;
                }
                loadedDocument = std::move(result.value());
            } else {
                auto mapId = ids::MapId::parse(std::filesystem::path(loadedLevel.mapPath).stem().string());
                if (!mapId.isOk()) {
                    std::cerr << "ID de mapa inválido: " << mapId.errorMessage() << "\n";
                    return 1;
                }
                auto imported = TmxMapImporter{}.importFile(loadedLevel.mapPath, mapId.value());
                for (const auto& issue : imported.issues) std::cerr << "Importación: " << issue.message << "\n";
                if (imported.hasErrors()) return 1;
                loadedDocument = std::move(imported.document);
                auto entities = level_document::ToEntityLayer(loadedLevel.objects);
                if (!entities.isOk()) { std::cerr << entities.errorMessage() << "\n"; return 1; }
                auto layerId = ids::LayerId::parse("level_objects").value();
                auto added = loadedDocument.addLayer(MapLayer::entities(
                    {layerId, "Objetos", true, false, 1.0f, static_cast<int>(loadedDocument.layers().size())},
                    std::move(entities.value())));
                if (!added.isOk()) { std::cerr << added.errorMessage() << "\n"; return 1; }
            }
            mapW = loadedDocument.width();
            mapH = loadedDocument.height();
            levelTitle = loadedLevel.name;
            loaded = true;
        }

        // Tileset: el que declare el TMX cargado, o el de pruebas para un
        // mapa nuevo (mismo criterio que Application::init). Las regiones
        // se declaran en bloque con la convencion de Tiled -- GID
        // firstGid+i = celda i en orden de lectura -- en vez de a mano,
        // asi cualquier tileset queda cubierto sin tocar codigo.
        const bool usingConstructionTiles = opts.useConstructionTiles && !loaded;
        const std::string tilesetPath =
            loaded && !loadedMap.tilesetImagePath().empty()
                ? loadedMap.tilesetImagePath()
                : (usingConstructionTiles ? std::string("assets/textures/editor_construction_tiles.png")
                                          : std::string("assets/textures/test_checker.png"));
        auto textureResult = textureManager.load("tileset", tilesetPath);
        if (!textureResult.isOk()) {
            std::cerr << "Error cargando tileset " << tilesetPath << ": "
                      << textureResult.errorMessage() << "\n";
            return 1;
        }
        const int cellW = usingConstructionTiles ? 313 : (loaded && loadedMap.getTilesetTileWidth() > 0
                              ? loadedMap.getTilesetTileWidth()
                              : 8);
        const int cellH = usingConstructionTiles ? 313 : (loaded && loadedMap.getTilesetTileHeight() > 0
                              ? loadedMap.getTilesetTileHeight()
                              : 8);
        TextureAtlas atlas(textureResult.value(), cellW, cellH);
        const int atlasCols = usingConstructionTiles ? 4 : (loaded && loadedMap.getTilesetColumns() > 0
                                  ? loadedMap.getTilesetColumns()
                                  : std::max(1, textureResult.value()->getWidth() / cellW));
        const int atlasRows = std::max(1, textureResult.value()->getHeight() / cellH);
        const int firstGid = loaded ? loadedMap.getTilesetFirstGid() : 1;
        for (int row = 0; row < atlasRows; ++row) {  // fila desde abajo: PNG invertido (PIXELLAB-L3)
            for (int col = 0; col < atlasCols; ++col) {
                atlas.defineRegion(firstGid + row * atlasCols + col, col, atlasRows - 1 - row);
            }
        }

        // Atlas exclusivamente editorial: iconos pixel-art para objetos.
        // Mantenerlo separado del tileset evita que un sprite provisional
        // cambie cómo se ve el juego final.
        auto iconTextureResult =
            textureManager.load("editor_object_icons", "assets/textures/editor_object_icons.png");
        if (!iconTextureResult.isOk()) {
            std::cerr << "Error cargando iconos del editor: "
                      << iconTextureResult.errorMessage() << "\n";
            return 1;
        }
        TextureAtlas objectIconAtlas(iconTextureResult.value(), 313, 313);
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                objectIconAtlas.defineRegion(1 + row * 4 + col, col, 3 - row);
            }
        }

        // Biblioteca visual específica de enemigos. Es un atlas de
        // referencia del editor, separado del atlas que usará el juego
        // final; por eso no toca spriteId ni el render de runtime.
        auto enemyIconTextureResult = textureManager.load(
            "editor_enemy_library_v1", "assets/textures/editor_enemy_library_v1.png");
        if (!enemyIconTextureResult.isOk()) {
            std::cerr << "Error cargando biblioteca visual de enemigos: "
                      << enemyIconTextureResult.errorMessage() << "\n";
            return 1;
        }
        TextureAtlas enemyIconAtlas(enemyIconTextureResult.value(), 362, 362);
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                enemyIconAtlas.defineRegion(1 + row * 4 + col, col, 2 - row);
            }
        }

        // Cuatro atlas editoriales, 43 siluetas: una por raceId del
        // catálogo RPG. No se usan como sprites animados de runtime.
        auto raceNpcTexture1 = textureManager.load("editor_race_npcs_01",
                                                    "assets/textures/editor_race_npcs_01.png");
        auto raceNpcTexture2 = textureManager.load("editor_race_npcs_02",
                                                    "assets/textures/editor_race_npcs_02.png");
        auto raceNpcTexture3 = textureManager.load("editor_race_npcs_03",
                                                    "assets/textures/editor_race_npcs_03.png");
        auto raceNpcTexture4 = textureManager.load("editor_race_npcs_04",
                                                    "assets/textures/editor_race_npcs_04.png");
        if (!raceNpcTexture1.isOk() || !raceNpcTexture2.isOk() || !raceNpcTexture3.isOk() ||
            !raceNpcTexture4.isOk()) {
            std::cerr << "Error cargando biblioteca visual de razas\n";
            return 1;
        }
        TextureAtlas raceNpcAtlas1(raceNpcTexture1.value(), 362, 362);
        TextureAtlas raceNpcAtlas2(raceNpcTexture2.value(), 362, 362);
        TextureAtlas raceNpcAtlas3(raceNpcTexture3.value(), 362, 362);
        TextureAtlas raceNpcAtlas4(raceNpcTexture4.value(), 443, 443);
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                const int frame = 1 + row * 4 + col;
                raceNpcAtlas1.defineRegion(frame, col, 2 - row);
                raceNpcAtlas2.defineRegion(frame, col, 2 - row);
                raceNpcAtlas3.defineRegion(frame, col, 2 - row);
            }
        }
        for (int col = 0; col < 4; ++col) {
            raceNpcAtlas4.defineRegion(1 + col, col, 1);
            raceNpcAtlas4.defineRegion(5 + col, col, 0);
        }

        EditorState editor(mapW, mapH);
        LevelEditorEditState editState;
        // Paleta de tiles: TODOS los GIDs del tileset, no solo dos --
        // con un tileset de ciudad hay que poder pintar cada material.
        std::vector<int> tileGids;
        for (int i = 0; i < atlasCols * atlasRows; ++i) {
            tileGids.push_back(firstGid + i);
        }
        editor.setTilePalette(tileGids);
        // Subgrupos de la paleta de objetos. Un nivel terminado contiene
        // muchos más props que PNJ; separar por intención de diseño evita
        // convertir Q/E en una lotería de memoria.
        const std::vector<std::string> objectGroupNames{
            "TODOS", "PNJ · HUMANOS", "PNJ · ELFOS / DROWS", "PNJ · ENANOS / GNOMOS",
            "PNJ · ORCOS / GOBLINS", "PNJ · LAGARTOS / NAGAS", "PNJ · OTRAS RAZAS",
            "DECORADO · INTERIOR", "DECORADO · BOSQUE", "DECORADO · MONTAÑA / MINA",
            "DECORADO · SELVA / PANTANO", "DECORADO · DESIERTO / VOLCAN",
            "DECORADO · OTROS", "MONSTRUOS · ANIMALES", "MONSTRUOS · BIPEDOS",
            "MONSTRUOS · NO MUERTOS", "MONSTRUOS · ELEMENTALES", "MONSTRUOS · DRAGONES",
            "MONSTRUOS · SAGRADOS / LEGENDARIOS", "MONSTRUOS · OTROS",
            "RECOGIBLES · CONSUMIBLES", "RECOGIBLES · EQUIPO", "EVENTOS / PUERTAS"};
        auto objectGroupEntries = [&](std::size_t group) {
            const ObjectFilter filter = filtroAfinidadObjeto(objectGroupNames[group]);
            std::vector<std::string> result;
            for (const std::string& id : catalog.ids()) {
                const ObjectDefinition* def = catalog.find(id);
                if (def != nullptr && filter(*def)) result.push_back(id);
            }
            return result;
        };
        std::unordered_map<std::string, std::string> objectPaletteLabels;
        for (const std::string& id : catalog.ids()) {
            const ObjectDefinition* def = catalog.find(id);
            if (def != nullptr) {
                objectPaletteLabels[id] = def->name + " " + id;
            }
        }
        editor.setObjectPaletteLabels(std::move(objectPaletteLabels));
        const std::vector<std::string> objectRootNames{
            "NATURALEZA / BIOMAS", "CIUDAD / INTERIORES", "SERES VIVIENTES",
            "RECOGIBLES / EQUIPO", "EVENTOS / SISTEMAS"};
        const std::vector<std::vector<std::size_t>> objectRootGroups{
            {8, 9, 10, 11, 12},       // exteriores y decoración natural
            {7, 12, 22},              // interiores, otros props y eventos
            {1, 2, 3, 4, 5, 6, 13, 14, 15, 16, 17, 18, 19},
            {20, 21},
            {22}};
        editor.setObjectPalette(objectGroupEntries(editState.objectGroup()));
        std::vector<std::string> destinosDeNivel;
        if (!opts.projectId.empty()) {
            auto indice = Editor::ProjectIndex::scan("assets");
            const Editor::Project* proyecto =
                indice.isOk() ? indice.value().find(opts.projectId) : nullptr;
            if (proyecto != nullptr) {
                for (const std::string& nivel : proyecto->levels) {
                    if (nivel != opts.levelName) {
                        destinosDeNivel.push_back("assets/levels/" + nivel);
                    }
                }
            }
        }
        // El panel de propiedades (click derecho sobre un objeto) ofrece
        // los MISMOS destinos que la herramienta de enlazar: las dos
        // hablan de las salidas del nivel y no pueden listar cosas
        // distintas. La copia va antes del move.
        const std::vector<std::string> destinosInspector = destinosDeNivel;
        editor.setLevelPalette(std::move(destinosDeNivel));

        // Estado GL-free del panel de propiedades (ver ObjectInspector.h):
        // aqui solo se le pasan teclas y se pinta lo que devuelve. Vive
        // toda la fase de edicion; se abre y se cierra con el raton.
        Editor::ObjectInspector inspector(editor, catalog, destinosInspector);

        if (loaded) {
            auto adopted = editor.setDocument(std::move(loadedDocument));
            if (!adopted.isOk()) {
                std::cerr << "No se pudo abrir el documento: " << adopted.errorMessage() << "\n";
                return 1;
            }
            editor.setPlayerStart(loadedLevel.playerStart);
            std::cout << "Cargado \"" << loadedLevel.name << "\" (" << mapW << "x" << mapH
                      << ", " << editor.objects().size() << " objetos)\n";
        }
        // Abrir/importar un nivel establece el documento base: Undo debe
        // afectar solo a lo que el autor haga DESPUES de abrirlo.
        editor.clearHistory();
        editor.markClean();

        Camera camera(width, height);
        // Camara centrada en el CENTRO del mapa, no en su esquina: con un
        // 64x64 el origen (0,0) queda fuera de pantalla y el editor
        // arrancaba mirando al vacio.
        camera.transitionTo(IsoMath::gridToScreen(GridCoord{mapW / 2, mapH / 2},
                                                  static_cast<float>(kTileW),
                                                  static_cast<float>(kTileH)),
                            camera.zoom(), 0.0f, Camera::Easing::Linear);
        camera.update(0.0f);
        SpriteBatch batch;
        Texture whiteTexture = makeWhiteTexture();
        BitmapFont font(/*scale=*/2);

        // --- HUD del editor (mismo lenguaje visual que Application:
        // paneles con marco, medidos contra su contenido con
        // measureText -- los defectos 01 y 06 del documento de diseno no
        // pueden reaparecer aqui) ---
        const Vector4 kPanelColor{0.05f, 0.05f, 0.08f, 0.82f};
        const Vector4 kBorderColor{0.55f, 0.55f, 0.65f, 0.9f};
        const Vector4 kDimText{0.7f, 0.7f, 0.75f, 1.0f};
        const Vector4 kWarnText{1.0f, 0.6f, 0.25f, 1.0f};
        constexpr float kPad = 10.0f;
        // lineHeight() y no glyphHeight(): es el avance real entre
        // renglones de drawText(), con interlineado (ver BitmapFont.h).
        const float lineH = static_cast<float>(font.lineHeight());

        // Cabecera: titulo a la izquierda, datos de la celda bajo el
        // cursor a la derecha (se actualizan por frame).
        HudTransform headerPanelT;
        headerPanelT.anchor = HudAnchor::TopLeft;
        headerPanelT.offset = {8.0f, 8.0f};
        headerPanelT.size = {static_cast<float>(width) - 16.0f, lineH + 2.0f * kPad};
        HudPanel headerPanel(headerPanelT, &whiteTexture, kPanelColor);
        headerPanel.setBorder(kBorderColor, 2.0f);

        HudTransform headerTextT;
        headerTextT.anchor = HudAnchor::TopLeft;
        headerTextT.offset = {8.0f + kPad, 8.0f + kPad};
        HudText headerText(headerTextT, &font);
        // El nombre del PROYECTO va delante del nivel: con cinco
        // proyectos abiertos a lo largo de una sesion y niveles que se
        // llaman parecido (ciudad_centro, ciudad_en_grytoz...), saber en
        // cual estas guardando no puede depender de recordarlo.
        {
            std::string cab = "EDITOR";
            if (!opts.projectId.empty()) {
                cab += " - " + saveName;
            }
            cab += " - " + levelTitle + "  " + std::to_string(mapW) + "X" + std::to_string(mapH);
            for (char& ch : cab) {
                ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            }
            headerText.setText(cab);
        }

        HudTransform hoverTextT;
        hoverTextT.anchor = HudAnchor::TopRight;
        hoverTextT.offset = {8.0f + kPad, 8.0f + kPad};
        hoverTextT.size = {260.0f, lineH};
        HudText hoverText(hoverTextT, &font);
        hoverText.setColor(kDimText);

        // Panel de herramientas: un HudCommandMenu NO navegado por
        // teclas propias (1-4 ya fijan la herramienta) -- cada frame se
        // sincroniza selectedIndex con editor.tool(), y el menu solo
        // dibuja la lista con el cursor "> ". El orden de las opciones
        // ES el del enum EditorTool (se castea el tool a indice).
        const std::vector<std::string> toolOptions{
            "1 " + std::string(toolName(EditorTool::PaintTile)),
            "2 " + std::string(toolName(EditorTool::EraseTile)),
            "3 " + std::string(toolName(EditorTool::PlaceObject)),
            "4 " + std::string(toolName(EditorTool::RemoveObject)),
            "5 " + std::string(toolName(EditorTool::SetPlayerStart)),
            "6 " + std::string(toolName(EditorTool::FillTiles)),
            "7 " + std::string(toolName(EditorTool::LinkLevel))};
        // Medida del propio widget (cuenta el prefijo del cursor y el
        // interlineado): nada de cuentas duplicadas que se desincronicen.
        const Vector2 toolsSize = HudCommandMenu::contentSize(font, toolOptions);
        const float toolsW = toolsSize.x;
        const float headerBottom = 8.0f + headerPanelT.size.y + 10.0f;

        HudTransform toolsPanelT;
        toolsPanelT.anchor = HudAnchor::TopLeft;
        toolsPanelT.offset = {8.0f, headerBottom};
        toolsPanelT.size = {toolsW + 2.0f * kPad, lineH + 6.0f + toolsSize.y + 2.0f * kPad};
        HudPanel toolsPanel(toolsPanelT, &whiteTexture, kPanelColor);
        toolsPanel.setBorder(kBorderColor, 2.0f);

        HudTransform toolsTitleT;
        toolsTitleT.anchor = HudAnchor::TopLeft;
        toolsTitleT.offset = {8.0f + kPad, headerBottom + kPad};
        HudText toolsTitle(toolsTitleT, &font);
        toolsTitle.setText("HERRAMIENTAS  1-7");
        toolsTitle.setColor(kDimText);

        HudTransform toolsMenuT;
        toolsMenuT.anchor = HudAnchor::TopLeft;
        toolsMenuT.offset = {8.0f + kPad, headerBottom + kPad + lineH + 6.0f};
        toolsMenuT.size = toolsSize;
        HudCommandMenu toolsMenu(toolsMenuT, &font, toolOptions);

        // Paleta activa (Q/E): dos menus -- tiles y objetos -- sobre un
        // mismo panel; la herramienta decide cual es visible. Nombres de
        // los GIDs fijos del tileset de prueba (cuando el tileset venga
        // de datos, esto saldra de alli).
        const std::unordered_map<int, TilePaletteInfo> tileSemantics =
            cargarSemanticaTiles(tilesetPath);
        std::unordered_map<int, std::string> tilePaletteLabels;
        for (const auto& entry : tileSemantics) {
            tilePaletteLabels[entry.first] = entry.second.name + " " + entry.second.tag;
        }
        editor.setTilePaletteLabels(std::move(tilePaletteLabels));
        auto tileLabel = [&](int gid) {
            auto it = tileSemantics.find(gid);
            const TilePaletteInfo info = it != tileSemantics.end()
                                             ? it->second
                                             : semanticasTile(gid, "", false);
            std::string label = "GID " + std::to_string(gid) + " · " + info.name +
                                " [" + info.tag + "]";
            if (info.collision) label += " [BLOQUEA]";
            return label;
        };
        std::vector<std::string> tileOptions;
        for (int gid : editor.tilePalette()) {
            tileOptions.push_back(tileLabel(gid));
        }
        auto etiquetasObjeto = [&]() {
            std::vector<std::string> labels;
            for (const std::string& id : editor.objectPalette()) {
                labels.push_back(nombreDePaleta(catalog, id));
            }
            return labels;
        };
        std::vector<std::string> objectOptions = etiquetasObjeto();
        std::vector<std::string> levelOptions;
        for (const std::string& path : editor.levelPalette()) {
            levelOptions.push_back("SALIDA -> " + nombreEscenario(path));
        }
        std::vector<std::string> objectRootOptions = objectRootNames;
        objectRootOptions.push_back("← CERRAR MENU");
        std::vector<std::string> objectSubgroupOptions;
        auto objectSubgroupIndexes = [&]() -> const std::vector<std::size_t>& {
            return objectRootGroups[editState.objectRootGroup()];
        };
        auto rebuildSubgroupOptions = [&]() {
            objectSubgroupOptions.clear();
            for (std::size_t group : objectSubgroupIndexes()) {
                objectSubgroupOptions.push_back(objectGroupNames[group]);
            }
            objectSubgroupOptions.push_back("← VOLVER A CATEGORIAS");
        };
        rebuildSubgroupOptions();

        // El panel de la paleta hospeda los DOS menus (solo uno visible
        // a la vez), asi que se dimensiona con el maximo de ambos: al
        // alternar tiles/objetos con 1-4 el marco no da saltos.
        // Ventana visible de la paleta: con un tileset de ciudad hay
        // decenas de tiles y la lista entera no cabe en pantalla, asi que
        // se muestran solo kPaletteWindow entradas alrededor de la
        // seleccionada (se recalculan cada frame con setOptions).
        constexpr std::size_t kPaletteWindow = 10;
        auto paletteWindow = [&](const std::vector<std::string>& all, std::size_t selected,
                                 std::size_t& outFirst) {
            outFirst = 0;
            if (all.size() > kPaletteWindow) {
                // Centrada en la seleccion y pegada a los extremos al
                // llegar al principio/final de la lista.
                const std::size_t half = kPaletteWindow / 2;
                outFirst = selected > half ? selected - half : 0;
                outFirst = std::min(outFirst, all.size() - kPaletteWindow);
            }
            const std::size_t count = std::min(kPaletteWindow, all.size());
            return std::vector<std::string>(all.begin() + static_cast<std::ptrdiff_t>(outFirst),
                                            all.begin() +
                                                static_cast<std::ptrdiff_t>(outFirst + count));
        };
        std::size_t dummyFirst = 0;
        const Vector2 tileSizeC =
            HudCommandMenu::contentSize(font, paletteWindow(tileOptions, 0, dummyFirst));
        const Vector2 objectSizeC =
            HudCommandMenu::contentSize(font, paletteWindow(objectOptions, 0, dummyFirst));
        const Vector2 objectGroupSizeC =
            HudCommandMenu::contentSize(font, paletteWindow(objectSubgroupOptions, 0, dummyFirst));
        const Vector2 levelSizeC =
            HudCommandMenu::contentSize(font, paletteWindow(levelOptions, 0, dummyFirst));
        const float paletteW = std::max({tileSizeC.x, objectSizeC.x, objectGroupSizeC.x, levelSizeC.x,
                                         font.measureText("PALETA  Q/E").x});
        const float paletteH = std::max({tileSizeC.y, objectSizeC.y, objectGroupSizeC.y, levelSizeC.y});
        const float toolsBottom = headerBottom + toolsPanelT.size.y + 10.0f;

        HudTransform palettePanelT;
        palettePanelT.anchor = HudAnchor::TopLeft;
        palettePanelT.offset = {8.0f, toolsBottom};
        palettePanelT.size = {paletteW + 2.0f * kPad, lineH + 6.0f + paletteH + 2.0f * kPad};
        HudPanel palettePanel(palettePanelT, &whiteTexture, kPanelColor);
        palettePanel.setBorder(kBorderColor, 2.0f);

        HudTransform paletteTitleT;
        paletteTitleT.anchor = HudAnchor::TopLeft;
        paletteTitleT.offset = {8.0f + kPad, toolsBottom + kPad};
        HudText paletteTitle(paletteTitleT, &font);
        paletteTitle.setText("PALETA  Q/E   3 FAMILIAS");
        paletteTitle.setColor(kDimText);

        HudTransform paletteMenuT;
        paletteMenuT.anchor = HudAnchor::TopLeft;
        paletteMenuT.offset = {8.0f + kPad, toolsBottom + kPad + lineH + 6.0f};
        paletteMenuT.size = {paletteW, paletteH};
        HudCommandMenu tileMenu(paletteMenuT, &font,
                                paletteWindow(tileOptions, 0, dummyFirst));
        HudCommandMenu objectMenu(paletteMenuT, &font,
                                  objectRootOptions);

        // Inspector (derecha): ficha de la entrada seleccionada en la
        // paleta, con AVISOS de ids sin resolver (defecto 04 del
        // documento de diseno: el skill fantasma del slime paso
        // inadvertido porque nada lo mostraba; aqui el editor lo grita).
        // Los ids registrados replican los de Application::init hasta
        // que las habilidades se carguen desde JSON (mismo comentario
        // alli).
        const std::vector<std::string> knownSkillIds{"tajo", "cura", "golpe_gelatinoso"};

        // La cola se calcula desde el catálogo cargado, así los objetos sin
        // arte no desaparecen de la paleta: se ven con icono editorial y el
        // autor sabe exactamente qué PNG falta generar.
        const std::vector<std::string> missingPreviewIds = catalog.missingPreviewIds();
        if (!missingPreviewIds.empty()) {
            std::cout << "  [PALETA] " << missingPreviewIds.size()
                      << " previews PNG pendientes (icono editorial activo)\n";
        }

        HudTransform inspectorPanelT;
        inspectorPanelT.anchor = HudAnchor::TopRight;
        inspectorPanelT.offset = {8.0f, headerBottom};
        inspectorPanelT.size = {300.0f, 220.0f};
        HudPanel inspectorPanel(inspectorPanelT, &whiteTexture, kPanelColor);
        inspectorPanel.setBorder(kBorderColor, 2.0f);

        // TopRight + size.x = 280: la caja del texto queda 10px dentro
        // del borde izquierdo del panel de 300 (280 + 2*kPad = 300).
        HudTransform inspectorTextT;
        inspectorTextT.anchor = HudAnchor::TopRight;
        inspectorTextT.offset = {8.0f + kPad, headerBottom + kPad};
        inspectorTextT.size = {280.0f, lineH};
        HudText inspectorText(inspectorTextT, &font);

        HudTransform inspectorWarnT = inspectorTextT;
        inspectorWarnT.offset.y = headerBottom + inspectorPanelT.size.y - kPad - 2.0f * lineH;
        HudText inspectorWarn(inspectorWarnT, &font);
        inspectorWarn.setColor(kWarnText);

        // Guía de creación: el panel de herramientas dice QUÉ tecla hace
        // cada cosa, pero no en qué orden se construye un escenario. Este
        // panel convierte el estado actual en los siguientes pasos y evita
        // que el autor tenga que retener la receta mentalmente.
        const float guideTop = headerBottom + inspectorPanelT.size.y + 10.0f;
        HudTransform guidePanelT;
        guidePanelT.anchor = HudAnchor::TopRight;
        guidePanelT.offset = {8.0f, guideTop};
        // Ocho renglones reales (título, separador, cinco estados y
        // siguiente paso) con el interlineado de BitmapFont: 200 deja
        // margen inferior y evita que "SIGUE" se salga del marco.
        guidePanelT.size = {300.0f, 200.0f};
        HudPanel guidePanel(guidePanelT, &whiteTexture, kPanelColor);
        guidePanel.setBorder(kBorderColor, 2.0f);

        HudTransform guideTextT;
        guideTextT.anchor = HudAnchor::TopRight;
        guideTextT.offset = {8.0f + kPad, guideTop + kPad};
        guideTextT.size = {280.0f, lineH};
        HudText guideText(guideTextT, &font);
        guideText.setColor(kDimText);

        // -------------------------------------------------------------
        // MODAL DE PROPIEDADES (click derecho sobre un objeto colocado).
        // Centro de pantalla; oculto hasta que ObjectInspector se abre.
        // El estado entero (filas, marca, buffer) vive en el inspector;
        // estos widgets solo lo pintan, igual que el resto del HUD.
        // -------------------------------------------------------------
        constexpr float kPropW = 480.0f;
        constexpr float kPropH = 420.0f;

        HudTransform propPanelT;
        propPanelT.anchor = HudAnchor::Center;
        propPanelT.offset = {0.0f, 0.0f};
        propPanelT.size = {kPropW, kPropH};
        HudPanel propPanel(propPanelT, &whiteTexture, kPanelColor);
        propPanel.setBorder(kBorderColor, 2.0f);
        propPanel.setVisible(false);

        HudTransform propTitleT;
        propTitleT.anchor = HudAnchor::Center;
        propTitleT.offset = {-kPropW * 0.5f + kPad, -kPropH * 0.5f + kPad};
        propTitleT.size = {kPropW - 2.0f * kPad, lineH};
        HudText propTitle(propTitleT, &font);
        propTitle.setColor(kWarnText);
        propTitle.setVisible(false);

        HudTransform propTextT;
        propTextT.anchor = HudAnchor::Center;
        propTextT.offset = {-kPropW * 0.5f + kPad, -kPropH * 0.5f + kPad + lineH + 8.0f};
        propTextT.size = {kPropW - 2.0f * kPad, kPropH - 2.0f * kPad - 4.0f * lineH};
        HudText propText(propTextT, &font);
        propText.setVisible(false);

        // Campo de edicion activo: lo que se esta tecleando ahora mismo
        // (buffer del inspector). Sin edicion abierta no se pinta nada.
        HudTransform propEditT;
        propEditT.anchor = HudAnchor::Center;
        propEditT.offset = {-kPropW * 0.5f + kPad, kPropH * 0.5f - kPad - 2.0f * lineH - 4.0f};
        propEditT.size = {kPropW - 2.0f * kPad, lineH};
        HudText propEdit(propEditT, &font);
        propEdit.setColor(kWarnText);
        propEdit.setVisible(false);

        HudTransform propAvisoT;
        propAvisoT.anchor = HudAnchor::Center;
        propAvisoT.offset = {-kPropW * 0.5f + kPad, kPropH * 0.5f - kPad - lineH};
        propAvisoT.size = {kPropW - 2.0f * kPad, lineH};
        HudText propAviso(propAvisoT, &font);
        propAviso.setColor(kDimText);
        propAviso.setVisible(false);

        // Barra de estado (abajo, ancho completo): controles, y el
        // resultado del ultimo guardado durante unos segundos.
        HudTransform statusPanelT;
        statusPanelT.anchor = HudAnchor::BottomCenter;
        statusPanelT.offset = {0.0f, 8.0f};
        statusPanelT.size = {static_cast<float>(width) - 16.0f, lineH + 2.0f * kPad};
        HudPanel statusPanel(statusPanelT, &whiteTexture, kPanelColor);
        statusPanel.setBorder(kBorderColor, 2.0f);

        const std::string kControlsLine =
            "1-7 HERRAM  Q/E PALETA  3 CATEG  ENTER ABRIR  ESC ATRAS  B BUSCAR  C CAPA  R SEL  Z/Y HIST  G GUARDAR  V VALIDAR  P JUGAR  M PROYECTOS";
        HudTransform statusTextT;
        statusTextT.anchor = HudAnchor::BottomCenter;
        statusTextT.offset = {0.0f, 8.0f + kPad};
        statusTextT.size = {static_cast<float>(width) - 32.0f, lineH};
        HudText statusText(statusTextT, &font);
        statusText.setText(kControlsLine);
        HudManager hud;
        hud.addElement(&headerPanel);
        hud.addElement(&headerText);
        hud.addElement(&hoverText);
        hud.addElement(&toolsPanel);
        hud.addElement(&toolsTitle);
        hud.addElement(&toolsMenu);
        hud.addElement(&palettePanel);
        hud.addElement(&paletteTitle);
        hud.addElement(&tileMenu);
        hud.addElement(&objectMenu);
        hud.addElement(&inspectorPanel);
        hud.addElement(&inspectorText);
        hud.addElement(&inspectorWarn);
        hud.addElement(&guidePanel);
        hud.addElement(&guideText);
        // El modal de propiedades se pinta el ultimo: cuando esta abierto
        // va POR ENCIMA de paneles y del mapa (es modal).
        hud.addElement(&propPanel);
        hud.addElement(&propTitle);
        hud.addElement(&propText);
        hud.addElement(&propEdit);
        hud.addElement(&propAviso);
        hud.addElement(&statusPanel);
        hud.addElement(&statusText);

        KeyEdge keys;
        GLFWwindow* glfwWin = window.handle();
        keys.prime(glfwWin);   // el ENTER con que abriste el proyecto no cuenta
        int frame = 0;
        bool rightWasDown = false;  // flanco del boton derecho (abrir/cerrar propiedades)

        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            const float dt = 1.0f / 60.0f;
            window.pollEvents();

            // C-15: resetear confirmación de salida si se pulsa cualquier tecla distinta de ESC
            if (editState.returnConfirmationPending()) {
                for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
                    // ESC/M deben llegar al manejador de salida de abajo:
                    // consumirlos aqui convertia el segundo ESC (que debe
                    // confirmar) en un no-op.
                    if (k == GLFW_KEY_ESCAPE || k == GLFW_KEY_M) {
                        continue;
                    }
                    if (keys.pressed(glfwWin, k)) {
                        editState.cancelReturnConfirmation();
                        break;
                    }
                }
            }

            // --- Avance del tiempo de animacion ---
            editState.tick(dt);

            // --- Input de teclado ---
            // Con el panel de propiedades abierto, el teclado es SUYO:
            // el estado del panel (ObjectInspector, GL-free) recibe las
            // teclas que usa, y el barrido final consume el resto de los
            // flancos para que NADA llegue al mapa mientras se edita.
            // Pintar detras del panel sin querer es la clase de susto
            // que convierte un inspector en una trampa.
            if (inspector.isOpen()) {
                if (keys.pressed(glfwWin, GLFW_KEY_ESCAPE)) {
                    inspector.key(27);
                }
                if (keys.pressed(glfwWin, GLFW_KEY_ENTER)) {
                    inspector.key('\n');
                }
                if (keys.pressed(glfwWin, GLFW_KEY_BACKSPACE)) {
                    inspector.key('\b');
                }
                for (int k = GLFW_KEY_A; k <= GLFW_KEY_Z; ++k) {
                    if (keys.pressed(glfwWin, k)) {
                        inspector.key(static_cast<char>('a' + (k - GLFW_KEY_A)));
                    }
                }
                for (int k = GLFW_KEY_0; k <= GLFW_KEY_9; ++k) {
                    if (keys.pressed(glfwWin, k)) {
                        inspector.key(static_cast<char>('0' + (k - GLFW_KEY_0)));
                    }
                }
                if (keys.pressed(glfwWin, GLFW_KEY_SPACE)) {
                    inspector.key(' ');
                }
                if (keys.pressed(glfwWin, GLFW_KEY_MINUS) ||
                    keys.pressed(glfwWin, GLFW_KEY_KP_SUBTRACT)) {
                    inspector.key('-');
                }
                if (keys.pressed(glfwWin, GLFW_KEY_PERIOD)) {
                    inspector.key('.');
                }
                if (keys.pressed(glfwWin, GLFW_KEY_COMMA)) {
                    inspector.key(',');
                }
                // El resto del teclado: consumido sin efecto.
                for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
                    (void)keys.pressed(glfwWin, k);
                }
            }

            // La búsqueda es un modal ligero: mientras está abierta solo
            // recibe texto, confirmación/cancelación y borrar. Sin este
            // corte, una tecla escrita para el filtro podía pintar, cambiar
            // de capa, hacer zoom o incluso guardar el nivel.
            if (keys.pressed(glfwWin, GLFW_KEY_B)) {
                editState.toggleSearch();
                statusText.setText(editState.searchOpen() ? "BUSCAR: ESCRIBE Y PULSA ENTER/ESC"
                                                          : "BUSQUEDA CERRADA - FILTRO CONSERVADO");
                editState.showStatusFor(120);
            }
            const bool searchModal = editState.searchOpen();
            if (searchModal) {
                if (keys.pressed(glfwWin, GLFW_KEY_ESCAPE) ||
                    keys.pressed(glfwWin, GLFW_KEY_ENTER)) {
                    editState.closeSearch();
                    statusText.setText("BUSQUEDA CERRADA - FILTRO CONSERVADO");
                    editState.showStatusFor(120);
                } else {
                    std::string filter = editor.paletteFilter();
                    bool cambio = false;
                    for (int k = GLFW_KEY_A; k <= GLFW_KEY_Z; ++k) {
                        if (keys.pressed(glfwWin, k)) {
                            filter += static_cast<char>('a' + (k - GLFW_KEY_A));
                            cambio = true;
                        }
                    }
                    for (int k = GLFW_KEY_0; k <= GLFW_KEY_9; ++k) {
                        if (keys.pressed(glfwWin, k)) {
                            filter += static_cast<char>('0' + (k - GLFW_KEY_0));
                            cambio = true;
                        }
                    }
                    if (keys.pressed(glfwWin, GLFW_KEY_MINUS) ||
                        keys.pressed(glfwWin, GLFW_KEY_KP_SUBTRACT)) {
                        filter += '_';
                        cambio = true;
                    }
                    if (keys.pressed(glfwWin, GLFW_KEY_BACKSPACE) && !filter.empty()) {
                        filter.pop_back();
                        cambio = true;
                    }
                    if (cambio) {
                        editor.setPaletteFilter(filter);
                        objectOptions = etiquetasObjeto();
                        tileOptions.clear();
                        for (int gid : editor.tilePalette()) {
                            tileOptions.push_back(tileLabel(gid));
                        }
                    }
                }
            }

            // ESC vuelve a la pantalla de proyectos, no cierra el editor.
            // Para cerrar del todo hay que pulsarlo otra vez alli. Cambiar
            // de proyecto no puede costar relanzar el binario.
            const bool colocandoObjetos = editor.tool() == EditorTool::PlaceObject;
            if (!searchModal && colocandoObjetos && keys.pressed(glfwWin, GLFW_KEY_ESCAPE)) {
                if (editState.objectPalettePage() != ObjectPalettePage::Root) {
                    editState.moveObjectPalettePageBack();
                    objectMenu.setSelectedIndex(0);
                    statusText.setText("VOLVER ATRAS EN MENU DE OBJETOS");
                } else {
                    editor.setTool(EditorTool::PaintTile);
                    statusText.setText("MENU DE OBJETOS CERRADO");
                }
                editState.showStatusFor(120);
            } else if (!searchModal && colocandoObjetos && keys.pressed(glfwWin, GLFW_KEY_BACKSPACE)) {
                if (editState.objectPalettePage() != ObjectPalettePage::Root) {
                    editState.moveObjectPalettePageBack();
                    objectMenu.setSelectedIndex(0);
                }
                editState.showStatusFor(120);
            } else if (!searchModal && !colocandoObjetos &&
                       (keys.pressed(glfwWin, GLFW_KEY_ESCAPE) || keys.pressed(glfwWin, GLFW_KEY_M))) {
                if (editState.requestReturnToHub(editor.isModified()) ==
                    ReturnToHubRequest::ConfirmDiscard) {
                    statusText.setText("HAY CAMBIOS SIN GUARDAR - PULSA ESC OTRA VEZ PARA SALIR SIN GUARDAR");
                    editState.showStatusFor(300);
                } else {
                    break;
                }
            }
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_1))
                editor.setTool(EditorTool::PaintTile);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_2))
                editor.setTool(EditorTool::EraseTile);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_3))
                editor.setTool(EditorTool::PlaceObject);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_4))
                editor.setTool(EditorTool::RemoveObject);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_5))
                editor.setTool(EditorTool::SetPlayerStart);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_6))
                editor.setTool(EditorTool::FillTiles);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_7))
                editor.setTool(EditorTool::LinkLevel);
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_3)) {
                editState.setObjectPalettePage(ObjectPalettePage::Root);
                objectMenu.setOptions(objectRootOptions);
                objectMenu.setSelectedIndex(0);
            }

            if (!searchModal && colocandoObjetos) {
                if (editState.objectPalettePage() != ObjectPalettePage::Entries) {
                    if (keys.pressed(glfwWin, GLFW_KEY_Q)) objectMenu.moveUp();
                    if (keys.pressed(glfwWin, GLFW_KEY_E)) objectMenu.moveDown();
                } else {
                    if (keys.pressed(glfwWin, GLFW_KEY_Q)) editor.prevPaletteEntry();
                    if (keys.pressed(glfwWin, GLFW_KEY_E)) editor.nextPaletteEntry();
                }
                if (keys.pressed(glfwWin, GLFW_KEY_ENTER)) {
                    const std::size_t selected = objectMenu.selectedIndex();
                    if (editState.objectPalettePage() == ObjectPalettePage::Root &&
                        selected < objectRootNames.size()) {
                        editState.setObjectRootGroup(selected);
                        rebuildSubgroupOptions();
                        editState.setObjectPalettePage(ObjectPalettePage::Subgroups);
                        objectMenu.setOptions(objectSubgroupOptions);
                        objectMenu.setSelectedIndex(0);
                    } else if (editState.objectPalettePage() == ObjectPalettePage::Root &&
                               selected == objectRootNames.size()) {
                        editor.setTool(EditorTool::PaintTile);
                        editState.setObjectPalettePage(ObjectPalettePage::Root);
                        statusText.setText("MENU DE OBJETOS CERRADO");
                    } else if (editState.objectPalettePage() == ObjectPalettePage::Subgroups &&
                               selected < objectSubgroupIndexes().size()) {
                        editState.setObjectGroup(objectSubgroupIndexes()[selected]);
                        editor.setObjectPalette(objectGroupEntries(editState.objectGroup()));
                        objectOptions = etiquetasObjeto();
                        editState.setObjectPalettePage(ObjectPalettePage::Entries);
                        objectMenu.setOptions(paletteWindow(objectOptions, 0, dummyFirst));
                        objectMenu.setSelectedIndex(0);
                    } else if (editState.objectPalettePage() == ObjectPalettePage::Subgroups &&
                               selected == objectSubgroupIndexes().size()) {
                        editState.setObjectPalettePage(ObjectPalettePage::Root);
                        objectMenu.setOptions(objectRootOptions);
                        objectMenu.setSelectedIndex(0);
                    }
                    editState.showStatusFor(120);
                }
                if (keys.pressed(glfwWin, GLFW_KEY_LEFT_BRACKET) ||
                    keys.pressed(glfwWin, GLFW_KEY_RIGHT_BRACKET)) {
                    const bool next = keys.pressed(glfwWin, GLFW_KEY_RIGHT_BRACKET);
                    if (editState.objectPalettePage() == ObjectPalettePage::Subgroups &&
                        objectSubgroupIndexes().size() > 1) {
                        const std::size_t n = objectSubgroupIndexes().size();
                        const std::size_t current = objectMenu.selectedIndex() < n ? objectMenu.selectedIndex() : 0;
                        objectMenu.setSelectedIndex(next ? (current + 1) % n : (current == 0 ? n - 1 : current - 1));
                    } else if (editState.objectPalettePage() == ObjectPalettePage::Entries &&
                               objectOptions.size() > 10) {
                        // [/ ] cambia de familia; nunca sustituye a Q/E.
                        const std::size_t group = editState.objectGroup();
                        editState.setObjectGroup(next ? (group + 1) % objectGroupNames.size()
                                                       : (group == 0 ? objectGroupNames.size() - 1 : group - 1));
                        editor.setObjectPalette(objectGroupEntries(editState.objectGroup()));
                        objectOptions = etiquetasObjeto();
                    }
                    editState.showStatusFor(120);
                }
            }
            const bool controlDown = glfwGetKey(glfwWin, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                                     glfwGetKey(glfwWin, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
                                     glfwGetKey(glfwWin, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                                     glfwGetKey(glfwWin, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
            const bool shiftDown = glfwGetKey(glfwWin, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                   glfwGetKey(glfwWin, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

            // La celda bajo el cursor se calcula AQUI, antes de las
            // teclas, porque varias de ellas actuan sobre ella (pegar,
            // marcar esquina, dar patrulla). Antes se calculaba mas
            // abajo, junto al raton, y no habia nada que lo necesitara
            // hasta entonces.
            double mouseX = 0.0;
            double mouseY = 0.0;
            glfwGetCursorPos(glfwWin, &mouseX, &mouseY);
            const Vector2 world = camera.screenToWorld(
                Vector2{static_cast<float>(mouseX), static_cast<float>(mouseY)});
            const GridCoord hovered = IsoMath::screenToGrid(world, static_cast<float>(kTileW),
                                                            static_cast<float>(kTileH));

            // =========================================================
            // CAPAS, SELECCION, PORTAPAPELES Y BUSQUEDA
            //
            // Todo esto vive en EditorState y esta probado en
            // demo_editor_state; aqui solo se enchufan las teclas.
            // =========================================================

            // --- Capas: C siguiente, SHIFT+C anterior, Ctrl+N nueva ---
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_C) && !controlDown) {
                const int n = editor.layerCount();
                const int siguiente = shiftDown ? (editor.activeLayer() + n - 1) % n
                                                : (editor.activeLayer() + 1) % n;
                editor.setActiveLayer(siguiente);
                statusText.setText("CAPA " + std::to_string(siguiente + 1) + "/" +
                                   std::to_string(n) + ": " + aMayus(editor.layerName(siguiente)));
                editState.showStatusFor(150);
            }
            if (!searchModal && controlDown && keys.pressed(glfwWin, GLFW_KEY_N)) {
                const int nueva = editor.addLayer("");
                statusText.setText("CAPA NUEVA: " + std::to_string(nueva + 1) + " DE " +
                                   std::to_string(editor.layerCount()));
                editState.showStatusFor(180);
            }

            // --- Seleccion: R marca esquina, R otra vez cierra el rect ---
            // Con teclado y no arrastrando con el raton porque el boton
            // izquierdo ya pinta: anadir un modificador de arrastre
            // significaria pintar sin querer cada vez que se falle.
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_R)) {
                if (!editState.selectionPending()) {
                    editState.beginSelection(hovered);
                    editor.clearSelection();
                    statusText.setText("ESQUINA MARCADA - R OTRA VEZ PARA CERRAR, ESC CANCELA");
                    editState.showStatusFor(240);
                } else {
                    const GridCoord corner = editState.selectionFirstCorner();
                    editor.setSelection(corner.x, corner.y, hovered.x, hovered.y);
                    editState.finishSelection();
                    statusText.setText("SELECCION " + std::to_string(editor.selection().width()) +
                                       "X" + std::to_string(editor.selection().height()) +
                                       "   CTRL+C COPIAR   CTRL+X BORRAR   T PATRULLA");
                    editState.showStatusFor(300);
                }
            }

            // --- Portapapeles ---
            if (!searchModal && controlDown && keys.pressed(glfwWin, GLFW_KEY_C)) {
                editor.copySelection();
                statusText.setText(editor.hasClipboard()
                                       ? "COPIADO " + std::to_string(editor.clipboardWidth()) + "X" +
                                             std::to_string(editor.clipboardHeight())
                                       : "NO HAY NADA SELECCIONADO (R MARCA UN RECTANGULO)");
                editState.showStatusFor(180);
            }
            if (!searchModal && controlDown && keys.pressed(glfwWin, GLFW_KEY_V)) {
                if (editor.hasClipboard()) {
                    editor.pasteAt(hovered.x, hovered.y);
                    statusText.setText("PEGADO EN " + std::to_string(hovered.x) + "," +
                                       std::to_string(hovered.y));
                } else {
                    statusText.setText("EL PORTAPAPELES ESTA VACIO");
                }
                editState.showStatusFor(180);
            }
            if (!searchModal && controlDown && keys.pressed(glfwWin, GLFW_KEY_X)) {
                if (editor.hasSelection()) {
                    editor.eraseSelection();
                    statusText.setText("SELECCION BORRADA (CTRL+Z LA DEVUELVE)");
                } else {
                    statusText.setText("NO HAY NADA SELECCIONADO");
                }
                editState.showStatusFor(180);
            }

            // --- T: darle al objeto bajo el cursor la patrulla del rect ---
            // Reutiliza la seleccion en vez de inventar un modo aparte:
            // marcar el recorrido y decir "este PNJ patrulla ahi" es el
            // gesto natural, y no hay que teclear cuatro numeros.
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_T)) {
                if (!editor.hasSelection()) {
                    statusText.setText("MARCA ANTES EL RECORRIDO CON R");
                } else if (editor.objectAt(hovered.x, hovered.y) == nullptr) {
                    statusText.setText("PON EL CURSOR SOBRE EL OBJETO QUE VA A PATRULLAR");
                } else {
                    const EditorState::Rect r = editor.selection();
                    editor.setObjectPatrol(hovered.x, hovered.y, GridCoord{r.x0, r.y0},
                                           GridCoord{r.x1, r.y1});
                    statusText.setText("PATRULLA " + std::to_string(r.x0) + "," +
                                       std::to_string(r.y0) + " A " + std::to_string(r.x1) + "," +
                                       std::to_string(r.y1));
                }
                editState.showStatusFor(240);
            }

            // H: escala de la instancia bajo el cursor. El gesto es
            // deliberadamente discreto: elegir entre cinco tamanos que
            // se ven al instante es mas fiable que teclear decimales sin
            // contexto. La escala se exporta en el spawn, no en el
            // catalogo, por lo que no cambia todos los objetos del mundo.
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_H)) {
                const ObjectSpawn* objeto = editor.objectAt(hovered.x, hovered.y);
                if (objeto == nullptr) {
                    statusText.setText("PON EL CURSOR SOBRE EL OBJETO QUE QUIERES ESCALAR");
                } else {
                    const float escalas[] = {0.5f, 0.75f, 1.0f, 1.5f, 2.0f};
                    std::size_t siguiente = 0;
                    for (std::size_t i = 0; i < 5; ++i) {
                        if (objeto->scale <= escalas[i] + 0.01f) {
                            siguiente = (i + 1) % 5;
                            break;
                        }
                    }
                    editor.setObjectScale(hovered.x, hovered.y, escalas[siguiente]);
                    statusText.setText("ESCALA " + std::to_string(static_cast<int>(escalas[siguiente] * 100)) +
                                       "% - CTRL+Z PARA DESHACER");
                }
                editState.showStatusFor(180);
            }

            if (!searchModal && controlDown && keys.pressed(glfwWin, GLFW_KEY_Z)) {
                statusText.setText(editor.undo() ? "DESHECHO" : "NO HAY CAMBIOS QUE DESHACER");
                editState.showStatusFor(120);
            }
            if (!searchModal && controlDown && keys.pressed(glfwWin, GLFW_KEY_Y)) {
                statusText.setText(editor.redo() ? "REHECHO" : "NO HAY CAMBIOS QUE REHACER");
                editState.showStatusFor(120);
            }
            if (!searchModal && editor.tool() != EditorTool::PlaceObject &&
                keys.pressed(glfwWin, GLFW_KEY_Q))
                editor.prevPaletteEntry();
            if (!searchModal && editor.tool() != EditorTool::PlaceObject &&
                keys.pressed(glfwWin, GLFW_KEY_E))
                editor.nextPaletteEntry();

            // Zoom (+/-, y 0 para volver a 1x): imprescindible en cuanto
            // el mapa no cabe en pantalla -- un 64x64 mide 4096x2048
            // pixeles de mundo, mas del triple del viewport. Rango
            // acotado: por debajo de 0.15 los tiles son ilegibles y por
            // encima de 3 no se ve contexto suficiente para editar.
            if (!searchModal &&
                (keys.pressed(glfwWin, GLFW_KEY_EQUAL) || keys.pressed(glfwWin, GLFW_KEY_KP_ADD))) {
                camera.setZoom(std::min(camera.zoom() * 1.25f, 3.0f));
            }
            if (!searchModal && (keys.pressed(glfwWin, GLFW_KEY_MINUS) ||
                                 keys.pressed(glfwWin, GLFW_KEY_KP_SUBTRACT))) {
                camera.setZoom(std::max(camera.zoom() / 1.25f, 0.15f));
            }
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_0)) {
                camera.setZoom(1.0f);
            }

            // Pan COMPENSADO POR EL ZOOM: camera.move() trabaja en
            // unidades de mundo, asi que a zoom 0.25 un paso fijo
            // recorreria en pantalla la cuarta parte y alejarse haria el
            // paneo insoportablemente lento -- justo al reves de lo que
            // hace falta. Dividir entre el zoom mantiene constante el
            // desplazamiento EN PANTALLA. SHIFT acelera x4, para cruzar
            // un mapa grande de punta a punta.
            const float zoomFactor = camera.zoom() > 0.0f ? camera.zoom() : 1.0f;
            const bool fast = glfwGetKey(glfwWin, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                              glfwGetKey(glfwWin, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            const float panSpeed = (fast ? 24.0f : 6.0f) / zoomFactor;
            // El paneo es por ESTADO (no flanco), asi que el barrido de
            // teclas del modal no lo consume: se bloquea aqui mientras
            // el panel de propiedades esta abierto.
            if (!inspector.isOpen() && !searchModal) {
                if (glfwGetKey(glfwWin, GLFW_KEY_W) == GLFW_PRESS)
                    camera.move({0.0f, -panSpeed});
                if (glfwGetKey(glfwWin, GLFW_KEY_S) == GLFW_PRESS)
                    camera.move({0.0f, panSpeed});
                if (glfwGetKey(glfwWin, GLFW_KEY_A) == GLFW_PRESS)
                    camera.move({-panSpeed, 0.0f});
                if (glfwGetKey(glfwWin, GLFW_KEY_D) == GLFW_PRESS)
                    camera.move({panSpeed, 0.0f});
            }

            // HOME: volver al centro del mapa (perderse en un 64x64 es
            // facil, y no habia forma de reorientarse salvo a ojo).
            if (!searchModal && keys.pressed(glfwWin, GLFW_KEY_HOME)) {
                camera.transitionTo(IsoMath::gridToScreen(GridCoord{mapW / 2, mapH / 2},
                                                          static_cast<float>(kTileW),
                                                          static_cast<float>(kTileH)),
                                    camera.zoom(), 0.25f, Camera::Easing::EaseOutCubic);
            }

            // Las teclas de funcion en un portatil Mac son teclas de
            // medios: F5 de verdad pide Fn+F5, que con las dos manos ya
            // ocupadas en el mapa no es una tecla, es una maniobra. Asi
            // que cada accion tiene una LETRA, y la tecla de funcion se
            // mantiene como alias porque no cuesta nada y hay quien la
            // tiene en la memoria de los dedos.
            //
            // Las letras son la inicial en espanol: Guardar, Validar,
            // Probar, Menu. Ninguna choca con lo que ya usa el editor
            // (WASD camara, Q/E paleta, Z/Y historial, 1-5 herramientas).
            auto pedida = [&](int letra, int funcion) {
                return !searchModal &&
                       (keys.pressed(glfwWin, letra) || keys.pressed(glfwWin, funcion));
            };

            // Guardar es una funcion porque probar tiene que
            // guardar antes: probar lo que hay en disco mientras miras
            // otra cosa en pantalla es la peor forma de perder una hora.
            auto guardarNivel = [&]() -> bool {
                TmxTilesetSettings settings;  // defaults = tileset del checker
                settings.collisionGids = {2};

                // Si el nivel se ABRIO de disco, se devuelve SU tileset,
                // no el de por defecto.
                //
                // Esto era un destructor de mapas. Guardar escribia
                // siempre test_checker (2 tiles), asi que abrir un mapa de
                // ciudad y pulsar guardar dejaba el TMX declarando un
                // atlas de dos casillas para un mapa que usa 34. No daba
                // error: daba un mapa que se dibuja mal. Le paso a
                // ciudad_centro.tmx, el mapa principal de Boundington, y
                // le habria pasado a cualquiera que se editara.
                if (loaded && !loadedMap.getTilesetImageSource().empty()) {
                    settings.tileWidth = loadedMap.getTileWidth();
                    settings.tileHeight = loadedMap.getTileHeight();
                    settings.tilesetName = loadedMap.getTilesetName();
                    settings.imageSource = loadedMap.getTilesetImageSource();
                    settings.imageWidth = loadedMap.getTilesetImageWidth();
                    settings.imageHeight = loadedMap.getTilesetImageHeight();
                    settings.tilesetTileWidth = loadedMap.getTilesetTileWidth();
                    settings.tilesetTileHeight = loadedMap.getTilesetTileHeight();
                    settings.tileCount = loadedMap.getTilesetTileCount();
                    settings.columns = loadedMap.getTilesetColumns();
                    // Las colisiones tambien: se declaran por tile en el
                    // tileset, y perderlas convierte los muros en aire.
                    if (!loadedMap.getCollisionGids().empty()) {
                        settings.collisionGids = loadedMap.getCollisionGids();
                    }
                }

                if (usingConstructionTiles) {
                    settings.tilesetName = "editor_construction_tiles";
                    settings.imageSource = "../textures/editor_construction_tiles.png";
                    settings.imageWidth = 1252;
                    settings.imageHeight = 1252;
                    settings.tilesetTileWidth = 313;
                    settings.tilesetTileHeight = 313;
                    settings.tileCount = 16;
                    settings.columns = 4;
                    // Muros, murallas, vallas, setos y decoración bloquean
                    // el paso; suelo, camino y agua siguen transitables en
                    // esta primera biblioteca editorial.
                    settings.collisionGids = {9, 10, 11, 12, 13, 14, 15, 16};
                }
const auto saved = EditorDocumentIO::save(editor, saveName, saveTmx, saveJson,
                                                           settings, loadedLevel.enemies);
                if (!saved.isOk()) {
                    std::cerr << "Error guardando: " << saved.errorMessage() << "\n";
                    statusText.setText("NO SE HA GUARDADO: " + saved.errorMessage());
                    editState.showStatusFor(240);
                    return false;
                }
                editor.markClean();
                editState.cancelReturnConfirmation();
                std::cout << "Guardado documento: " << EditorDocumentIO::documentPathFor(saveTmx)
                          << " (exportación " << saveTmx << " + " << saveJson << ")\n";

                // C-15 — Registrar el nivel en el manifiesto del proyecto activo.
                if (!opts.projectId.empty()) {
                    std::size_t slash = saveJson.find_last_of('/');
                    std::string levelFileName = (slash == std::string::npos) ? saveJson : saveJson.substr(slash + 1);
                    auto reg = Editor::ProjectIndex::addLevelToProject("assets", opts.projectId, levelFileName);
                    if (!reg.isOk()) {
                        std::cerr << "Aviso: no se pudo registrar el nivel en el proyecto: " << reg.errorMessage() << "\n";
                    }
                }

                // Feedback en la barra de estado (~4s a 60fps); el bucle
                // la devuelve a los controles al caducar.
                std::string aviso = "GUARDADO: " + saveJson;
                for (char& ch : aviso) {
                    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                }
                statusText.setText(aviso);
                editState.showStatusFor(240);
                // Disparar el flash de guardado (overlay verde breve).
                editState.triggerSaveFlash();
                return true;
            };

            if (pedida(GLFW_KEY_G, GLFW_KEY_F5)) {
                guardarNivel();
            }

            // F8/F9 — NAVEGAR EL PROYECTO. El manifiesto declara el orden
            // de los escenarios; no se adivina por el nombre de archivo.
            // Guardamos ANTES de salir del documento: cambiar de mapa no
            // puede convertirse en otra forma de perder una tarde de
            // pintura. Los niveles dan la vuelta, como la lista de
            // proyectos, para que un proyecto grande sea navegable.
            // Coma y punto son el anterior/siguiente de toda la vida y
            // estan pegadas; F8/F9 se mantienen como alias, pero en un
            // portatil Mac piden Fn igual que F5, que es de lo que
            // veniamos huyendo.
            const int direccionEscenario =
                pedida(GLFW_KEY_COMMA, GLFW_KEY_F8) ? -1 :
                (pedida(GLFW_KEY_PERIOD, GLFW_KEY_F9) ? 1 : 0);
            if (direccionEscenario != 0) {
                if (opts.projectId.empty()) {
                    statusText.setText("CAMBIAR DE ESCENARIO NECESITA UN PROYECTO: ABRELO DESDE LA PANTALLA");
                    editState.showStatusFor(240);
                } else {
                    auto indice = Editor::ProjectIndex::scan("assets");
                    const Editor::Project* proyecto =
                        indice.isOk() ? indice.value().find(opts.projectId) : nullptr;
                    if (proyecto == nullptr || proyecto->levels.empty()) {
                        statusText.setText("EL PROYECTO NO DECLARA OTROS ESCENARIOS");
                        editState.showStatusFor(240);
                    } else if (guardarNivel()) {
                        std::size_t actual = 0;
                        for (std::size_t i = 0; i < proyecto->levels.size(); ++i) {
                            if (proyecto->levels[i] == opts.levelName) {
                                actual = i;
                                break;
                            }
                        }
                        const std::size_t total = proyecto->levels.size();
                        const std::size_t siguiente =
                            direccionEscenario > 0 ? (actual + 1) % total
                                                   : (actual + total - 1) % total;
                        opts.levelName = proyecto->levels[siguiente];
                        opts.loadLevelPath.clear();
                        std::cout << "Cambiando a escenario " << (siguiente + 1) << "/" << total
                                  << ": " << opts.levelName << "\n";
                        editState.requestLevelChange();
                        break;
                    }
                }
            }

            // P (o F7) — MODO JUGADOR. Guarda y lanza ./juego sobre este nivel,
            // como proceso aparte. Al cerrar la ventana del juego se vuelve
            // al modo desarrollador exactamente donde se estaba editando.
            //
            // Aparte y no dentro: Application monta su propia ventana y su
            // propio contexto GL, asi que no cabe dentro del editor; y
            // ademas un cuelgue probando no se lleva por delante lo que
            // estas editando. Es lo que hace el boton de play de Godot.
            //
            // El editor se queda bloqueado mientras el juego corre. Es lo
            // que se quiere: volver al editor con el juego abierto detras
            // y no saber cual de las dos ventanas manda es peor.
            if (pedida(GLFW_KEY_P, GLFW_KEY_F7) && guardarNivel()) {
                // Un proyecto vacío también se puede probar: el catálogo
                // neutro evita introducir objetos de Boundington o del fixture.
                const std::string catalogoDePrueba = catalogPaths.empty()
                    ? "assets/objects/empty_objects.json" : catalogPaths.front();
                // El binario esta junto al del editor cuando se compila con
                // CMake, pero el cwd es la raiz del repo (los assets son
                // rutas relativas): se prueban los dos sitios.
                std::string exe = "./juego";
                if (std::FILE* f = std::fopen("build/juego", "rb")) {
                    std::fclose(f);
                    exe = "./build/juego";
                }
                std::string orden =
                    exe + " --nivel \"" + saveJson + "\" --catalogo \"" + catalogoDePrueba + "\"";
                for (std::size_t i = 1; i < catalogPaths.size(); ++i) {
                    orden += " --catalogo-extra \"" + catalogPaths[i] + "\"";
                }
                std::cout << "Probando: " << orden << "\n";
                statusText.setText("MODO JUGADOR... (cierra el juego para volver al editor)");
                editState.showStatusFor(240);
                const int codigo = std::system(orden.c_str());
                if (codigo != 0) {
                    std::cout << "El juego termino con codigo " << codigo << "\n";
                    statusText.setText("NO SE PUDO PROBAR: COMPILA EL OBJETIVO 'juego' PRIMERO");
                    editState.showStatusFor(240);
                }
                // Las teclas que se pulsaron mientras el juego tenia el
                // foco no son del editor. Se resincroniza con lo que este
                // pulsado AHORA: crear un KeyEdge limpio haria justo lo
                // contrario, dar por nueva cualquier tecla aun hundida.
                keys.prime(glfwWin);
            }

            if (pedida(GLFW_KEY_V, GLFW_KEY_F6)) {
                const EditorValidationResult validation =
                    EditorValidation::check(editor, catalog, {2});
                if (!validation.ok()) {
                    statusText.setText("VALIDACION: " + std::to_string(validation.errors.size()) +
                                       " ERROR(ES) - " + validation.errors.front());
                } else {
                    statusText.setText("VALIDACION OK: " +
                                       std::to_string(validation.reachableTiles) + "/" +
                                       std::to_string(validation.walkableTiles) +
                                       " TILES ALCANZABLES" +
                                       (validation.warnings.empty() ? "" : " - REVISA ZONAS AISLADAS"));
                }
                editState.showStatusFor(240);
            }

            // --- Raton (la celda 'hovered' se calculo con las teclas) ---
            bool leftDown = glfwGetMouseButton(glfwWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            bool rightDown = glfwGetMouseButton(glfwWin, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            const bool rightEdge = rightDown && !rightWasDown;
            rightWasDown = rightDown;
            if (inspector.isOpen()) {
                // El panel se cierra como se abre: boton derecho fuera de
                // la edicion. El izquierdo no hace nada mientras esta
                // abierto (un click accidental no deberia pintar el mapa
                // que hay detras del panel).
                if (rightEdge) {
                    inspector.close();
                }
            } else if (searchModal) {
                // El buscador bloquea el mapa: escribir un filtro no puede
                // pintar debajo del panel ni abrir propiedades por accidente.
            } else if (leftDown) {
                // Mantener pulsado pinta en arrastre (deliberado: es lo
                // comodo para rellenar zonas; para objetos, placeObject
                // reemplaza en la misma celda asi que no duplica).
                editor.applyAt(hovered.x, hovered.y);
            } else if (rightEdge && editor.objectAt(hovered.x, hovered.y) != nullptr) {
                // Boton derecho SOBRE UN OBJETO: sus propiedades. Es el
                // gesto de cualquier editor (RPG Maker, Tiled, Godot):
                // click en la cosa, panel de la cosa. El estado entero
                // del panel vive en ObjectInspector (GL-free); aqui solo
                // se abre y se pinta.
                inspector.open(hovered.x, hovered.y);
                statusText.setText("PROPIEDADES: W/S NAVEGAR  ENTER EDITAR  ESC O CLICK DER CERRAR");
                editState.showStatusFor(240);
            } else if (rightDown) {
                // Boton derecho en celda sin objeto: el "contrario" rapido
                // de la herramienta (como antes de existir el panel).
                if (editor.tool() == EditorTool::PlaceObject ||
                    editor.tool() == EditorTool::RemoveObject) {
                    editor.removeObjectAt(hovered.x, hovered.y);
                } else {
                    editor.eraseTile(hovered.x, hovered.y);
                }
            }

            camera.update(1.0f / 60.0f);

            // --- Render del mundo (tiles del editor + objetos + cursor) ---
            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            Shader* shader = spriteShader.value();
            shader->use();
            shader->setUniformMat4("uViewProjection", camera.getViewProjectionMatrix());
            shader->setUniformInt("uTexture", 0);

            batch.begin();
            Vector2 tileSize{static_cast<float>(kTileW), static_cast<float>(kTileH)};
            // gridToScreen(c) devuelve el VERTICE SUPERIOR del rombo
            // isometrico (la punta de arriba del tile), no la esquina sup-izq
            // del rectangulo. Para que el tile se dibuje CENTRADO sobre ese
            // vertice -- y asi el centro del rectangulo dibujado coincida con
            // donde screenToGrid() resuelve la celda correcta al clicar -- hay
            // que desplazar el dibujo medio tile a la izquierda. Sin este
            // offset, el raton apuntaba al centro visual del tile pero
            // screenToGrid resolvia la celda de la derecha (desfase de medio
            // tile): ver el round-trip verificado en el desarrollo del editor.
            const Vector2 tileDrawOffset{-static_cast<float>(kTileW) * 0.5f, 0.0f};
            for (int y = 0; y < editor.height(); ++y) {
                for (int x = 0; x < editor.width(); ++x) {
                    const Vector2 pos = IsoMath::gridToScreen(GridCoord{x, y}, static_cast<float>(kTileW),
                                                              static_cast<float>(kTileH)) + tileDrawOffset;
                    bool painted = false;
                    for (int layer = 0; layer < editor.layerCount(); ++layer) {
                        if (!editor.layerVisible(layer) || editor.layerOpacity(layer) <= 0) continue;
                        const int gid = editor.tileAt(layer, x, y);
                        if (gid == 0) continue;
                        batch.submit(pos, tileSize, atlas.getUV(gid), atlas.texture(),
                                     Vector4{1.0f, 1.0f, 1.0f, editor.layerOpacity(layer)});
                        painted = true;
                    }
                    if (!painted) {
                        batch.submit(pos, tileSize, UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                                     Vector4{1.0f, 1.0f, 1.0f, 0.06f});
                    }
                }
            }
            // Objetos colocados: su sprite del catalogo (o un marcador
            // blanco semitransparente si spriteId es -1). Mismo offset que
            // los tiles (tileDrawOffset) para que queden alineados con la
            // celda sobre la que se colocaron.
            for (const ObjectSpawn& spawn : editor.objects()) {
                Vector2 pos = IsoMath::gridToScreen(spawn.position, static_cast<float>(kTileW),
                                                    static_cast<float>(kTileH)) +
                              tileDrawOffset;
                const ObjectDefinition* def = catalog.find(spawn.objectId);
                if (def != nullptr) {
                    const int enemyIcon = iconoBibliotecaEnemigos(*def);
                    const RaceNpcPreview racePreview = previewNpcRacial(*def);
                    if (racePreview.atlas != 0) {
                        const TextureAtlas* raceAtlases[] = {
                            nullptr, &raceNpcAtlas1, &raceNpcAtlas2, &raceNpcAtlas3, &raceNpcAtlas4};
                        const TextureAtlas* previewAtlas = raceAtlases[racePreview.atlas];
                        batch.submit(pos, tileSize, uvPreviewPng(*previewAtlas, racePreview.frame),
                                     previewAtlas->texture(), Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    } else if (enemyIcon != 0) {
                        batch.submit(pos, tileSize, uvPreviewPng(enemyIconAtlas, enemyIcon),
                                     enemyIconAtlas.texture(), Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    } else {
                        batch.submit(pos, tileSize, uvPreviewPng(objectIconAtlas, iconoEditor(*def)),
                                     objectIconAtlas.texture(), Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    }
                } else {
                    batch.submit(pos, tileSize, UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                                 Vector4{0.9f, 0.9f, 0.3f, 0.5f});
                }
                // Todo objeto conectado recibe una baliza naranja. No hay
                // que recordar qué prop es una salida: se ve en el mapa.
                if (!spawn.targetLevel.empty()) {
                    batch.submit(pos + Vector2{static_cast<float>(kTileW) * 0.32f,
                                                static_cast<float>(kTileH) * 0.18f},
                                 Vector2{static_cast<float>(kTileW) * 0.36f,
                                         static_cast<float>(kTileH) * 0.36f},
                                 UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                                 Vector4{1.0f, 0.55f, 0.08f, 0.92f});
                }
            }
            // El punto de inicio forma parte del nivel, no es un dato
            // oculto en el JSON: se muestra como una ficha cian para poder
            // recolocarlo con la herramienta 5 y verificarlo de un vistazo.
            {
                Vector2 pos = IsoMath::gridToScreen(editor.playerStart(),
                                                    static_cast<float>(kTileW),
                                                    static_cast<float>(kTileH)) +
                              tileDrawOffset;
                batch.submit(pos + Vector2{static_cast<float>(kTileW) * 0.25f,
                                            static_cast<float>(kTileH) * 0.25f},
                             Vector2{static_cast<float>(kTileW) * 0.5f,
                                     static_cast<float>(kTileH) * 0.5f},
                             UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                             Vector4{0.20f, 0.85f, 1.0f, 0.82f});
            }
            // Celda bajo el cursor: resaltado PULSANTE (onda senoidal). Mucho
            // mas facil de localizar la celda activa que un blanco estatico:
            // el alpha oscila entre ~0.20 y ~0.50 a ~0.6Hz. Mismo offset que
            // los tiles para que el resaltado caiga EXACTO sobre la celda que
            // el raton apunta (sin el desfase de medio tile).
            {
                Vector2 pos = IsoMath::gridToScreen(hovered, static_cast<float>(kTileW),
                                                    static_cast<float>(kTileH)) +
                              tileDrawOffset;
                float pulse = 0.35f + 0.15f * std::sin(editState.animationTime() * 4.0f);
                batch.submit(pos, tileSize, UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                             Vector4{1.0f, 1.0f, 1.0f, pulse});
            }
            batch.end();

            // --- HUD: sincronizar widgets con el estado del editor ---
            const bool objectMode = editor.tool() == EditorTool::PlaceObject ||
                                    editor.tool() == EditorTool::RemoveObject;
            const bool objectPaletteMode = editor.tool() == EditorTool::PlaceObject;
            const bool linkMode = editor.tool() == EditorTool::LinkLevel;

            // El menu de herramientas refleja editor.tool() (el orden de
            // toolOptions es el del enum, ver su comentario).
            toolsMenu.setSelectedIndex(static_cast<std::size_t>(editor.tool()));

            // Paleta visible segun herramienta, con su cursor en la
            // entrada activa (busqueda lineal: las paletas tienen un
            // punado de entradas).
            tileMenu.setVisible(!objectMode && !linkMode);
            objectMenu.setVisible(objectPaletteMode || linkMode);
            if (linkMode) {
                paletteTitle.setText("DESTINOS  Q/E");
            } else if (objectPaletteMode &&
                       editState.objectPalettePage() == ObjectPalettePage::Root) {
                paletteTitle.setText("COLOCAR OBJETOS · CATEGORIAS · ENTER ABRIR");
            } else if (objectPaletteMode &&
                       editState.objectPalettePage() == ObjectPalettePage::Subgroups) {
                paletteTitle.setText("SUBCATEGORIAS · Q/E ELEGIR · ENTER ABRIR · [/ ] PAGINA");
            } else if (objectPaletteMode) {
                paletteTitle.setText("OBJETOS: " + objectGroupNames[editState.objectGroup()] +
                                     "  Q/E ELEGIR · ESC ATRAS · [/ ] PAGINA");
            } else {
                paletteTitle.setText("PALETA  Q/E");
            }
            if (searchModal) {
                paletteTitle.setText("BUSCAR: " +
                                     (editor.paletteFilter().empty() ? std::string("<TEXTO>")
                                                                     : aMayus(editor.paletteFilter())) +
                                     "  ENTER/ESC CERRAR");
            }
            // Indice absoluto de la seleccion en la paleta completa...
            const std::vector<int>& paletteGids = editor.tilePalette();
            std::size_t tileSel = 0;
            for (std::size_t i = 0; i < paletteGids.size(); ++i) {
                if (paletteGids[i] == editor.selectedTileGid()) tileSel = i;
            }
            const std::vector<std::string>& paletteIds =
                linkMode ? editor.levelPalette() : editor.objectPalette();
            std::size_t objSel = 0;
            for (std::size_t i = 0; i < paletteIds.size(); ++i) {
                const std::string selected =
                    linkMode ? editor.selectedLevelPath() : editor.selectedObjectId();
                if (paletteIds[i] == selected) objSel = i;
            }
            // ...y ventana visible + indice RELATIVO a ella (el menu solo
            // conoce lo que se esta mostrando).
            std::size_t tileFirst = 0;
            std::size_t objFirst = 0;
            tileMenu.setOptions(paletteWindow(tileOptions, tileSel, tileFirst));
            tileMenu.setSelectedIndex(tileSel - tileFirst);
            if (linkMode) {
                objectMenu.setOptions(paletteWindow(levelOptions, objSel, objFirst));
                objectMenu.setSelectedIndex(objSel - objFirst);
            } else if (objectPaletteMode &&
                       editState.objectPalettePage() == ObjectPalettePage::Root) {
                objectMenu.setOptions(objectRootOptions);
                objectMenu.setSelectedIndex(std::min(objectMenu.selectedIndex(), objectRootOptions.size() - 1));
            } else if (objectPaletteMode &&
                       editState.objectPalettePage() == ObjectPalettePage::Subgroups) {
                objectMenu.setOptions(objectSubgroupOptions);
                objectMenu.setSelectedIndex(std::min(objectMenu.selectedIndex(), objectSubgroupOptions.size() - 1));
            } else if (objectPaletteMode) {
                objectMenu.setOptions(paletteWindow(objectOptions, objSel, objFirst));
                objectMenu.setSelectedIndex(objSel - objFirst);
            }

            // Celda bajo el cursor (cabecera, derecha): coordenada + gid
            // + objeto si lo hay. tileAt() ya devuelve 0 fuera de rango.
            std::string hoverInfo = "CELDA " + std::to_string(hovered.x) + "," +
                                    std::to_string(hovered.y) + "  GID " +
                                    std::to_string(editor.tileAt(hovered.x, hovered.y));
            for (const ObjectSpawn& spawn : editor.objects()) {
                if (spawn.position.x == hovered.x && spawn.position.y == hovered.y) {
                    const ObjectDefinition* def = catalog.find(spawn.objectId);
                    hoverInfo += "  [" + (def != nullptr ? def->name : spawn.objectId) + "]";
                    if (!spawn.targetLevel.empty()) {
                        hoverInfo += "  SALIDA -> " + nombreEscenario(spawn.targetLevel);
                    }
                    if (spawn.scale != 1.0f) {
                        hoverInfo += "  ESCALA " +
                                     std::to_string(static_cast<int>(spawn.scale * 100)) + "%";
                    }
                    break;
                }
            }
            hoverText.setText(hoverInfo);

            // Inspector: ficha de la seleccion de la paleta activa, con
            // avisos de ids sin resolver en su propia linea naranja.
            std::string fichaInspector;
            std::string warnings;
            if (editor.tool() == EditorTool::SetPlayerStart) {
                const GridCoord start = editor.playerStart();
                fichaInspector = "INSPECTOR\n\nINICIO JUGADOR\nX " + std::to_string(start.x) +
                            "  Y " + std::to_string(start.y) +
                            "\nCLICK EN UNA CELDA\nPARA REUBICARLO";
            } else if (linkMode) {
                const std::string destino = editor.selectedLevelPath();
                fichaInspector = "INSPECTOR\n\nCREAR SALIDA A\n" +
                            (destino.empty() ? std::string("SIN DESTINOS EN EL PROYECTO")
                                             : nombreEscenario(destino)) +
                            "\n\nCLICK SOBRE LA PUERTA\nQUE DEBE LLEVAR ALLI";
            } else if (objectPaletteMode) {
                const std::string selectedId = editor.selectedObjectId();
                const ObjectDefinition* def = catalog.find(selectedId);
                if (def == nullptr) {
                    fichaInspector = "INSPECTOR\n\n" + selectedId + "\nSIN DEFINICION EN CATALOGO";
                    warnings = "! ID SIN RESOLVER";
                } else {
                    const char* category = def->category == ObjectCategory::Enemy    ? "ENEMY"
                                           : def->category == ObjectCategory::Pickup ? "PICKUP"
                                                                                     : "PROP";
                    fichaInspector = "INSPECTOR\n\nID " + def->id + "\nNOMBRE " + def->name +
                                "\nCATEGORIA " + category + "\nSPRITE " +
                                std::to_string(def->spriteId) + "\nBLOQUEA " +
                                (def->blocksMovement ? "SI" : "NO");
                    fichaInspector += "\nPREVIEW " +
                                 (def->previewPng.empty() ? std::string("PNG PENDIENTE")
                                                          : def->previewPng);
                    if (def->previewPng.empty()) {
                        warnings += "! FALTA PNG REPRESENTATIVO\n";
                    }
                    if (def->category == ObjectCategory::Enemy) {
                        fichaInspector += "\nPV " + std::to_string(def->combat.maxHealth) + "  PM " +
                                     std::to_string(def->combat.maxMana);
                        for (const std::string& skillId : def->combat.skillIds) {
                            fichaInspector += "\nSKILL " + skillId;
                            bool known = false;
                            for (const std::string& registered : knownSkillIds) {
                                known = known || registered == skillId;
                            }
                            if (!known) {
                                warnings += "! SKILL SIN REGISTRAR: " + skillId + "\n";
                            }
                        }
                    } else if (def->category == ObjectCategory::Pickup) {
                        const char* effect = def->pickup.effect == PickupEffect::Heal ? "HEAL"
                                             : def->pickup.effect == PickupEffect::RestoreMana
                                                 ? "RESTORE MANA"
                                                 : "NONE";
                        fichaInspector += std::string("\nEFECTO ") + effect;
                        if (def->pickup.effect != PickupEffect::None) {
                            fichaInspector += " " + std::to_string(def->pickup.power);
                        }
                    }
                }
            } else if (editor.tool() == EditorTool::RemoveObject) {
                const ObjectSpawn* objeto = editor.objectAt(hovered.x, hovered.y);
                fichaInspector = "INSPECTOR\n\nQUITAR OBJETO\nCLICK IZQ SOBRE UN OBJETO\n"
                                  "O CLICK DER PARA BORRADO RAPIDO";
                if (objeto != nullptr) {
                    const ObjectDefinition* def = catalog.find(objeto->objectId);
                    fichaInspector += "\n\nSELECCIONADO\n" +
                                      (def != nullptr ? def->name : objeto->objectId);
                }
            } else {
                const int gid = editor.selectedTileGid();
                fichaInspector = "INSPECTOR\n\nTILE " + tileLabel(gid) + "\nCOLISION " +
                            (gid == 2 ? "SI" : "NO") + "\nINICIO " +
                            std::to_string(editor.playerStart().x) + "," +
                            std::to_string(editor.playerStart().y);
            }
            inspectorText.setText(fichaInspector);
            inspectorWarn.setText(warnings);

            // Estado mínimo de creación. No exige objetos ni salida: un
            // interior de combate o una sala de prueba puede no llevarlos.
            // Sí los hace visibles para que se añadan conscientemente y no
            // por acordarse de una lista externa.
            int activeLayerTiles = 0;
            for (int y = 0; y < editor.height(); ++y) {
                for (int x = 0; x < editor.width(); ++x) {
                    if (editor.tileAt(x, y) != 0) {
                        ++activeLayerTiles;
                    }
                }
            }
            int exits = 0;
            for (const ObjectSpawn& spawn : editor.objects()) {
                if (!spawn.targetLevel.empty()) {
                    ++exits;
                }
            }
            std::string nextStep;
            if (activeLayerTiles == 0) {
                nextStep = "SIGUE: 1 PINTA";
            } else if (editor.objects().empty()) {
                nextStep = "SIGUE: 3 OBJETOS";
            } else if (opts.projectId.size() > 0 && exits == 0) {
                nextStep = "SIGUE: 7 SALIDA";
            } else {
                nextStep = "SIGUE: V Y P";
            }
            guideText.setText(
                "GUIA DEL NIVEL\n\n"
                "[" + std::string(activeLayerTiles > 0 ? "OK" : "--") + "] SUELO " +
                std::to_string(activeLayerTiles) + "  C" + std::to_string(editor.activeLayer() + 1) + "\n"
                "[OK] INICIO " + std::to_string(editor.playerStart().x) + "," +
                std::to_string(editor.playerStart().y) + " (5)\n"
                "[" + std::string(editor.objects().empty() ? "--" : "OK") + "] OBJETOS " +
                std::to_string(editor.objects().size()) + "  (3)\n"
                "[" + std::string(exits == 0 ? "--" : "OK") + "] SALIDAS " +
                std::to_string(exits) + "  (7)\n"
                "V VALIDA / P JUEGA\nG GUARDA\n\n" + nextStep);

            // --- Modal de propiedades: sincronizar con el inspector ---
            const bool modalAbierto = inspector.isOpen();
            propPanel.setVisible(modalAbierto);
            propTitle.setVisible(modalAbierto);
            propText.setVisible(modalAbierto);
            propEdit.setVisible(modalAbierto && inspector.editandoTexto());
            propAviso.setVisible(modalAbierto);
            if (modalAbierto) {
                propTitle.setText(inspector.titulo());
                const std::vector<std::string>& lineas = inspector.lines();
                std::string cuerpo;
                if (inspector.editando() && !inspector.editandoTexto()) {
                    // Picker abierto (destino/botin): las lineas ya traen
                    // su propia marca "> "; se pintan tal cual.
                    for (const std::string& linea : lineas) {
                        cuerpo += linea + "\n";
                    }
                } else {
                    // Navegacion: ventana de filas alrededor de la
                    // marcada, mismo patron que la paleta.
                    constexpr std::size_t kVentanaProp = 13;
                    std::size_t primera = 0;
                    if (lineas.size() > kVentanaProp &&
                        inspector.selected() > kVentanaProp / 2) {
                        primera = inspector.selected() - kVentanaProp / 2;
                        if (primera + kVentanaProp > lineas.size()) {
                            primera = lineas.size() - kVentanaProp;
                        }
                    }
                    for (std::size_t i = primera;
                         i < lineas.size() && i < primera + kVentanaProp; ++i) {
                        cuerpo += (i == inspector.selected() ? "> " : "  ") +
                                  lineas[i] + "\n";
                    }
                }
                propText.setText(cuerpo);
                if (inspector.editandoTexto()) {
                    propEdit.setText("ESCRIBIENDO: " + inspector.buffer() + "_");
                }
                propAviso.setText(inspector.aviso().empty()
                                      ? std::string("W/S NAVEGAR  ENTER EDITAR  ESC CERRAR")
                                      : inspector.aviso());
            }

            // Barra de estado: vuelve a los controles cuando caduca el
            // aviso de guardado (ver el manejador de F5). Cuando muestra los
            // controles, anade contadores en tiempo real (tiles pintados /
            // objetos colocados) para dar feedback del progreso del nivel.
            if (editState.statusMessageActive()) {
                if (editState.advanceStatusFrame()) {
                    statusText.setText(kControlsLine);
                }
            } else {
                statusText.setText(kControlsLine);
            }
            // El HUD va en su propia tanda begin()/end() (ver
            // HudManager::render() y demo_hud.cpp: el llamador rodea la
            // llamada; render() solo hace submit()).
            batch.begin();
            hud.render(batch, *shader, width, height);
            // Overlay de flash de guardado: quad verde semitransparente
            // cubriendo toda la pantalla, alpha = editState.saveFlashAlpha(). Desciende
            // a 0 en ~0.4s (ver el avance de animacion arriba). Feedback
            // claro de que F5 funciono, ademas del texto de la barra de
            // estado.
            if (editState.saveFlashAlpha() > 0.0f) {
                Vector4 flashCol{0.20f, 0.70f, 0.30f, editState.saveFlashAlpha() * 0.30f};
                batch.submit({0.0f, 0.0f}, {static_cast<float>(width), static_cast<float>(height)},
                             UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture, flashCol);
            }
            batch.end();
            // hud.render() dejo la proyeccion de pantalla en el shader;
            // el proximo frame la vuelve a fijar a la de camara, asi que
            // no hay nada que restaurar (ver HudManager::render()).

            // En modo N-frames, volcar el framebuffer en el ultimo frame
            // para inspeccion visual (mismo patron que demo_lighting/fog/
            // lut). Sin esto no hay forma de verificar el render del editor
            // sin abrir la ventana a mano.
            // EST-55 — OJO: este fichero tiene DOS volcados al MISMO
            // level_editor_output.ppm (este, muestreado, y el de la fase
            // de edicion, a resolucion completa). El segundo pisa al
            // primero, asi que esta captura no llega a verse nunca.
            // Se deja llamando a la funcion compartida y no se decide
            // cual sobra: eso es comportamiento, no mantenimiento.
            if (maxFrames >= 0 && frame == maxFrames - 1) {
                const auto volcado =
                    Render::VuelcaFramebufferPPMMuestreado("level_editor_output.ppm", width, height);
                if (!volcado.isOk()) std::cerr << volcado.errorMessage() << "\n";
                else std::cout << "Framebuffer volcado a level_editor_output.ppm.\n";
            }

            glDisable(GL_BLEND);

            // Modo humo (argumento numerico): en el ULTIMO frame se
            // vuelca el framebuffer, igual que los demas demos GL. Asi
            // "./level_editor 3" deja un level_editor_output.ppm con el
            // HUD real dibujado, inspeccionable sin captura a mano.
            // Antes del swapBuffers(): despues, el buffer que se leeria
            // ya no es el recien dibujado.
            if (maxFrames >= 0 && frame == maxFrames - 1) {
                const auto volcado = Render::VuelcaFramebufferPPM("level_editor_output.ppm", width, height);
                if (!volcado.isOk()) std::cerr << volcado.errorMessage() << "\n";
                std::cout << "Framebuffer volcado a level_editor_output.ppm (" << width << "x"
                          << height << ") tras " << (frame + 1) << " frames.\n";
            }

            window.swapBuffers();
            ++frame;
        }

        // Fin de la fase de edicion. Si se salio con ESC se vuelve a la
        // pantalla de proyectos, y para eso hay que olvidar cual estaba
        // abierto: si no, resolverProyecto() lo reabriria en la siguiente
        // vuelta y ESC no llevaria a ninguna parte.
        if (editState.exit() == EditPhaseExit::Continue) {
            break;
        }
        if (editState.exit() == EditPhaseExit::ChangeLevel) {
            if (!resolverProyecto()) {
                return 1;
            }
            continue;
        }
        opts.projectId.clear();
        opts.loadLevelPath.clear();
        opts.levelName.clear();
        std::cout << "\nVuelta a la pantalla de proyectos.\n";
        }  // while (!cerrarDelTodo)

        GLenum glError = glGetError();
        std::cout << "[GL] glGetError() al salir = " << glError << " (esperado: 0)\n";
        require(glError == GL_NO_ERROR);
        std::cout << "\nEditor cerrado sin errores GL.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error fatal al arrancar: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
