// Fase 12 (motor_grafico_gantt_rpg.puml): editor de niveles visual.
//
// UI con el HUD PROPIO del motor (Fase 9: HudPanel/HudText/BitmapFont),
// NO con Dear ImGui como planeaba el Gantt: sin red en el entorno de
// desarrollo no se puede vendorizar (ver la nota de alcance en
// EditorState.h -- el nucleo del editor esta separado de la UI
// justamente para poder migrar a ImGui algun dia sin tocarlo).
//
// Controles:
//   Raton:  click izq  = aplicar herramienta en la celda bajo el cursor
//           click der   = herramienta "contraria" rapida (borrar tile /
//                         quitar objeto, segun el modo actual)
//   1/2/3/4/5 = herramienta (pintar/borrar tile, colocar/quitar objeto,
//               fijar punto de inicio del jugador)
//   Q/E     = ciclar la paleta activa (tiles u objetos, segun herramienta)
//   Ctrl+Z / Ctrl+Y = deshacer / rehacer
//   WASD    = pan de camara (SHIFT = x4, para mapas grandes)
//   +/-     = zoom (0 = volver a 1x), HOME = centrar en el mapa
//   G       = guardar en los ficheros DEL PROYECTO abierto (rutas
//             relativas al cwd: ejecutar desde build/ o MotorGraphico/)
//   V       = validar el nivel sin salir del editor
//   P       = probar: guarda y lanza ./juego sobre este nivel
//   M       = volver a la pantalla de proyectos (ESC hace lo mismo)
//
//   F5/F6/F7 siguen valiendo como alias de G/V/P. Las teclas de funcion
//   en un portatil Mac son teclas de medios: F5 de verdad pide Fn+F5, que
//   con las dos manos ocupadas en el mapa no es una tecla, es una
//   maniobra. Por eso manda la letra y la tecla de funcion es el alias, y
//   no al reves.
//
// PENDIENTE, documentado aqui antes de existir (que es como llego a estar
// escrito el F7 de "modo jugador" durante un tiempo sin que hubiera
// ninguno): nivel anterior/siguiente del proyecto sin pasar por la
// pantalla. Hoy se cambia de nivel volviendo con M.
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
#include <cstdio>
#include <memory>

#include "Editor/ProjectHub.h"
#include "Editor/ProjectIndex.h"
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
#include "Editor/EditorValidation.h"
#include "Render/HudElement.h"
#include "Render/HudManager.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Level/LevelDefinition.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "Render/SpriteBatch.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp):
// glad.h define el guard de GL y glfw3.h se salta su #include <GL/gl.h>.
#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace {

constexpr int kTileW = 64;
constexpr int kTileH = 32;

// Textura 1x1 blanca: mismo patron que demo_hud.cpp (fondo de paneles,
// resaltado de la celda bajo el cursor).
Texture makeWhiteTexture() {
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture(glID, 1, 1);
}

// Volcado del framebuffer a PPM a RESOLUCION COMPLETA (a diferencia de
// demo_fog/demo_lighting, que bajan a 160x90 porque su post-FX rompe el
// glReadPixels masivo en macOS/Metal: el editor no usa post-FX, y hace
// falta la resolucion entera para poder LEER el texto del HUD).
// glReadPixels tiene el origen abajo-izquierda y el PPM lo espera
// arriba-izquierda: las filas se escriben en orden inverso.
void writeFramebufferPPM(const std::string& path, int width, int height) {
    glFinish();
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";
    const std::size_t rowBytes = static_cast<std::size_t>(width) * 3;
    for (int y = height - 1; y >= 0; --y) {
        out.write(
            reinterpret_cast<const char*>(pixels.data() + static_cast<std::size_t>(y) * rowBytes),
            static_cast<std::streamsize>(rowBytes));
    }
}

const char* toolName(EditorTool tool) {
    switch (tool) {
        case EditorTool::PaintTile:
            return "PINTAR TILE";
        case EditorTool::EraseTile:
            return "BORRAR TILE";
        case EditorTool::PlaceObject:
            return "COLOCAR OBJETO";
        case EditorTool::RemoveObject:
            return "QUITAR OBJETO";
        case EditorTool::SetPlayerStart:
            return "INICIO JUGADOR";
    }
    return "?";
}

// Deteccion de flanco de subida de teclas (GLFW da estado, no eventos, y
// sin esto una pulsacion de 100ms cicla la paleta 6 veces a 60fps).
class KeyEdge {
public:
    bool pressed(GLFWwindow* window, int key) {
        bool down = glfwGetKey(window, key) == GLFW_PRESS;
        bool edge = down && !m_wasDown[key];
        m_wasDown[key] = down;
        return edge;
    }

    // Da por vistas las teclas que ya estan pulsadas AHORA, sin reportar
    // flanco por ellas.
    //
    // Hace falta al cambiar de fase (edicion <-> pantalla de proyectos).
    // Un KeyEdge recien creado cree que no habia nada pulsado, asi que la
    // tecla que te trajo aqui -- ESC, que sigue fisicamente hundida --
    // cuenta como flanco nuevo en el primer frame de la fase siguiente:
    // ESC en el editor te devolvia a la pantalla y esa misma pulsacion
    // cerraba el editor. Se veia como que ESC cerraba y ya.
    void prime(GLFWwindow* window) {
        for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
            m_wasDown[k] = glfwGetKey(window, k) == GLFW_PRESS;
        }
    }

private:
    bool m_wasDown[GLFW_KEY_LAST + 1] = {};
};

// Opciones de linea de comandos. El argumento numerico suelto sigue
// siendo el modo humo (N frames y salir), como en todos los demos GL:
// las opciones nuevas se anaden sin romper esa convencion.
struct EditorOptions {
    int mapWidth = 8;
    int mapHeight = 8;
    std::string loadLevelPath;  // vacio = mapa nuevo en blanco
    int maxFrames = -1;

    // --- Proyectos (pantalla de arranque) ---
    std::string projectId;      // proyecto activo: manda donde se GUARDA
    std::string levelName;      // nivel suyo a abrir; vacio = el primero
    std::string newProjectId;   // --nuevo: crear y salir
    std::string newPrefix;
    bool listProjects = false;  // --proyectos: listar y salir
};

// "64x48" -> {64, 48}. Devuelve false si no tiene la forma AxB con
// ambos numeros > 0 (mejor rechazar que arrancar con un mapa absurdo).
bool parseSize(const std::string& text, int& outW, int& outH) {
    const std::size_t sep = text.find('x');
    if (sep == std::string::npos || sep == 0 || sep + 1 >= text.size()) {
        return false;
    }
    outW = std::atoi(text.substr(0, sep).c_str());
    outH = std::atoi(text.substr(sep + 1).c_str());
    return outW > 0 && outH > 0;
}

EditorOptions parseArgs(int argc, char** argv) {
    EditorOptions opts;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--size" || arg == "-s") && i + 1 < argc) {
            if (!parseSize(argv[++i], opts.mapWidth, opts.mapHeight)) {
                std::cerr << "Tamano invalido (se esperaba AnchoxAlto, ej. 64x64)\n";
            }
        } else if ((arg == "--load" || arg == "-l") && i + 1 < argc) {
            opts.loadLevelPath = argv[++i];
        } else if (!arg.empty() && (std::isdigit(static_cast<unsigned char>(arg[0])) != 0)) {
            opts.maxFrames = std::atoi(arg.c_str());
        } else if (arg == "--proyectos" || arg == "-p") {
            opts.listProjects = true;
        } else if (arg == "--proyecto" && i + 1 < argc) {
            opts.projectId = argv[++i];
        } else if (arg == "--nivel" && i + 1 < argc) {
            opts.levelName = argv[++i];
        } else if (arg == "--nuevo" && i + 1 < argc) {
            opts.newProjectId = argv[++i];
        } else if (arg == "--prefijo" && i + 1 < argc) {
            opts.newPrefix = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout <<
                "Uso: level_editor [opciones] [frames]\n"
                "\n  PROYECTOS (no abren ventana)\n"
                "  --proyectos              lista los proyectos vivos y su estado\n"
                "  --nuevo <id> --prefijo <p_>\n"
                "                           crea un proyecto y sale\n"
                "\n  EDICION\n"
                "  --proyecto <id>          abre ese proyecto; F5 guarda EN EL,\n"
                "                           no en editor_map.tmx\n"
                "  --nivel <fichero.json>   nivel suyo a abrir (por defecto, el primero)\n"
                "  --size 64x64             mapa nuevo en blanco de ese tamano\n"
                "  --load ruta.json         abre un nivel suelto (sin proyecto)\n"
                "  frames                   modo humo: N frames, vuelca PPM y sale\n"
                "\n  Dentro del editor: F7 modo jugador, F8/F9 cambia de escenario,\n"
                "  ESC vuelve a la pantalla.\n"
                "\n  La build de un proyecto se saca con:\n"
                "    python3 tools/build_proyecto.py <id>\n";
            std::exit(0);
        }
    }
    return opts;
}

}  // namespace

// Pantalla de arranque en modo texto. Va ANTES de abrir la ventana a
// proposito: listar proyectos o crear uno no necesita contexto GL, y asi
// tambien sirve desde un script o desde CI. La misma informacion es la que
// pinta el editor en su lista cuando arranca con ventana.
int pantallaDeArranque(const EditorOptions& opts) {
    auto r = Editor::ProjectIndex::scan("assets");
    if (!r.isOk()) {
        std::cerr << "No se pudo leer assets/proyectos: " << r.errorMessage() << "\n";
        return 1;
    }
    const Editor::ProjectIndex idx = r.value();

    if (!opts.newProjectId.empty()) {
        auto nuevo = Editor::ProjectIndex::create("assets", opts.newProjectId,
                                                  opts.newProjectId, opts.newPrefix,
                                                  "editor", "2000 b.f.");
        if (!nuevo.isOk()) {
            std::cerr << "No se pudo crear: " << nuevo.errorMessage() << "\n";
            return 1;
        }
        std::cout << "Proyecto '" << nuevo.value().id << "' creado con prefijo '"
                  << nuevo.value().prefix << "'.\n"
                  << "Abrelo con:  ./level_editor --proyecto " << nuevo.value().id << "\n";
        return 0;
    }

    std::cout << "PROYECTOS VIVOS\n\n";
    std::printf("  %-13s %-32s %4s %4s %4s  %s\n",
                "id", "nombre", "niv", "av", "cat", "estado");
    for (const Editor::Project& p : idx.projects()) {
        const Editor::ProjectCheck c = idx.check(p.id);
        std::printf("  %-13s %-32s %4d %4d %4d  %s\n",
                    p.id.c_str(), p.name.substr(0, 32).c_str(),
                    (int)p.levels.size(), (int)p.adventures.size(),
                    (int)p.catalogs.size(),
                    c.ok() ? "completo" : (std::to_string(c.problems.size()) + " problema(s)").c_str());
        if (!c.ok()) {
            for (const std::string& s : c.problems) {
                std::printf("       - %s\n", s.c_str());
            }
        }
    }
    for (const std::string& o : idx.orphans()) {
        std::printf("  [huerfano] %s\n", o.c_str());
    }
    std::cout << "\n  abrir:  ./level_editor --proyecto <id>\n"
                 "  nuevo:  ./level_editor --nuevo <id> --prefijo <p_>\n"
                 "  build:  python3 tools/build_proyecto.py <id>\n";
    return 0;
}

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
            auto hr = Editor::ProjectHub::load("assets");
            if (!hr.isOk()) {
                std::cerr << "No se pudo leer assets/proyectos: " << hr.errorMessage() << "\n";
                return 1;
            }
            Editor::ProjectHub hub = hr.value();

            SpriteBatch menuBatch;
            Texture menuWhite = makeWhiteTexture();
            BitmapFont menuFont(/*scale=*/2);
            const float lh = static_cast<float>(menuFont.lineHeight());
            const Vector4 kFondo{0.05f, 0.05f, 0.08f, 0.92f};
            const Vector4 kMarco{0.55f, 0.55f, 0.65f, 0.9f};

            KeyEdge menuKeys;
            GLFWwindow* win = window.handle();
            menuKeys.prime(win);   // el ESC que venia del editor no cuenta
            bool salir = false;

            // Teclas que producen un caracter, para el campo del id. Se
            // recorren A-Z y 0-9 en vez de usar el callback de texto de
            // GLFW porque Window ya gestiona sus propios callbacks y
            // enchufar otro desde aqui es meterse en su terreno.
            auto caracterPulsado = [&]() -> char {
                for (int k = GLFW_KEY_A; k <= GLFW_KEY_Z; ++k) {
                    if (menuKeys.pressed(win, k)) {
                        return static_cast<char>('a' + (k - GLFW_KEY_A));
                    }
                }
                for (int k = GLFW_KEY_0; k <= GLFW_KEY_9; ++k) {
                    if (menuKeys.pressed(win, k)) {
                        return static_cast<char>('0' + (k - GLFW_KEY_0));
                    }
                }
                if (menuKeys.pressed(win, GLFW_KEY_MINUS)) {
                    return '_';   // el guion bajo pide Shift; se acepta el guion a secas
                }
                return 0;
            };

            while (!window.shouldClose() && !salir) {
                window.pollEvents();

                if (hub.mode() == Editor::ProjectHub::Mode::NewProject) {
                    const char c = caracterPulsado();
                    if (c != 0) {
                        hub.key(c);
                    }
                    if (menuKeys.pressed(win, GLFW_KEY_BACKSPACE)) hub.key('\b');
                    if (menuKeys.pressed(win, GLFW_KEY_ENTER))     hub.key('\n');
                    if (menuKeys.pressed(win, GLFW_KEY_ESCAPE))    hub.key(27);
                } else {
                    // En modo Message cualquier tecla cierra; se mandan las
                    // mismas y ProjectHub ya sabe que hacer con ellas.
                    Editor::ProjectHub::Action act = Editor::ProjectHub::Action::None;
                    if (menuKeys.pressed(win, GLFW_KEY_W) || menuKeys.pressed(win, GLFW_KEY_UP))
                        act = hub.key('w');
                    if (menuKeys.pressed(win, GLFW_KEY_S) || menuKeys.pressed(win, GLFW_KEY_DOWN))
                        act = hub.key('s');
                    if (menuKeys.pressed(win, GLFW_KEY_N))      act = hub.key('n');
                    if (menuKeys.pressed(win, GLFW_KEY_B))      act = hub.key('b');
                    if (menuKeys.pressed(win, GLFW_KEY_ENTER))  act = hub.key('\n');
                    if (menuKeys.pressed(win, GLFW_KEY_ESCAPE)) act = hub.key(27);

                    if (act == Editor::ProjectHub::Action::Quit) {
                        cerrarDelTodo = true;
                        salir = true;
                    }
                    if (act == Editor::ProjectHub::Action::Open) {
                        opts.projectId = hub.openId();
                        salir = true;
                    }
                }

                glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                HudManager menu;
                std::vector<std::unique_ptr<IHudElement>> vivos;
                auto guardar = [&](IHudElement* e) {
                    vivos.emplace_back(e);
                    menu.addElement(e);
                };
                auto panel = [&](HudAnchor a, Vector2 off, Vector2 size) {
                    auto* p = new HudPanel(HudTransform{a, off, size}, &menuWhite, kFondo);
                    p->setBorder(kMarco, 2.0f);
                    guardar(p);
                };
                auto texto = [&](HudAnchor a, Vector2 off, const std::string& s,
                                 const Vector4& col) {
                    auto* t = new HudText(HudTransform{a, off, {0.0f, 0.0f}}, &menuFont);
                    t->setText(s);
                    t->setColor(col);
                    guardar(t);
                };

                const Vector4 kBlanco{1.0f, 1.0f, 1.0f, 1.0f};
                const Vector4 kApagado{0.70f, 0.70f, 0.75f, 1.0f};
                const Vector4 kAviso{1.0f, 0.60f, 0.25f, 1.0f};
                const Vector4 kBien{0.45f, 0.90f, 0.55f, 1.0f};

                panel(HudAnchor::TopLeft, {20.0f, 20.0f}, {static_cast<float>(width) - 40.0f, 46.0f});
                texto(HudAnchor::TopLeft, {34.0f, 34.0f}, "MOTOR GRAFICO - PROYECTOS", kBlanco);

                // Lista
                panel(HudAnchor::TopLeft, {20.0f, 80.0f}, {620.0f, lh * 12.0f + 24.0f});
                for (std::size_t i = 0; i < hub.lines().size() && i < 12; ++i) {
                    const bool marcado = (i == hub.selected());
                    texto(HudAnchor::TopLeft, {34.0f, 94.0f + lh * static_cast<float>(i)},
                          (marcado ? "> " : "  ") + hub.lines()[i], marcado ? kBlanco : kApagado);
                }

                // Detalle del marcado
                const std::vector<std::string> det = hub.detail();
                panel(HudAnchor::TopRight, {20.0f, 80.0f}, {580.0f, lh * 12.0f + 24.0f});
                for (std::size_t i = 0; i < det.size() && i < 12; ++i) {
                    const bool malo = !det[i].empty() &&
                                      (det[i][0] == '!' || det[i].find("NO SE ENCUENTRA") != std::string::npos);
                    texto(HudAnchor::TopRight, {566.0f, 94.0f + lh * static_cast<float>(i)},
                          det[i], malo ? kAviso : kApagado);
                }

                // Barra de teclas
                panel(HudAnchor::BottomLeft, {20.0f, 20.0f},
                      {static_cast<float>(width) - 40.0f, 40.0f});
                texto(HudAnchor::BottomLeft, {34.0f, 32.0f},
                      "W/S mover   ENTER abrir   N nuevo   B compilar (build)   ESC cerrar el editor",
                      kApagado);

                // Modales encima de todo
                if (hub.mode() == Editor::ProjectHub::Mode::NewProject) {
                    panel(HudAnchor::Center, {0.0f, 0.0f}, {720.0f, 130.0f});
                    texto(HudAnchor::Center, {-330.0f, -46.0f}, "ID DEL PROYECTO NUEVO", kBlanco);
                    // El cursor no parpadea: en una pantalla que se pinta
                    // entera cada frame, un '_' fijo se lee igual de bien y
                    // no hay que llevar el tiempo hasta aqui.
                    texto(HudAnchor::Center, {-330.0f, -8.0f}, "> " + hub.draftId() + "_", kBien);
                    texto(HudAnchor::Center, {-330.0f, 34.0f},
                          "solo a-z 0-9 _    ENTER crea    ESC cancela", kApagado);
                } else if (hub.mode() == Editor::ProjectHub::Mode::Message) {
                    const std::vector<std::string>& m = hub.message();
                    const std::size_t filas = m.size() < 16 ? m.size() : 16;
                    const float alto = lh * static_cast<float>(filas) + 60.0f;
                    panel(HudAnchor::Center, {0.0f, 0.0f}, {1100.0f, alto});
                    for (std::size_t i = 0; i < filas; ++i) {
                        texto(HudAnchor::Center,
                              {-520.0f, -alto * 0.5f + 20.0f + lh * static_cast<float>(i)},
                              m[i], i == 0 ? (hub.lastOk() ? kBien : kAviso) : kApagado);
                    }
                    texto(HudAnchor::Center, {-520.0f, alto * 0.5f - 24.0f},
                          "(una tecla para volver)", kApagado);
                }

                menuBatch.begin();
                menu.render(menuBatch, *spriteShader.value(), width, height);
                menuBatch.end();
                window.swapBuffers();
            }
            if (cerrarDelTodo || !salir) {
                break;   // ESC en la pantalla, o cerraron la ventana
            }
            if (!resolverProyecto()) {
                return 1;
            }
        }

        // El tileset se decide DESPUES de saber que mapa se edita (con
        // --load, el TMX declara el suyo): ver mas abajo, tras la carga.
        TextureManager textureManager;

        // Catalogo real del repo: la paleta de objetos del editor se
        // rellena desde aqui (solo ids que existen de verdad).
        ObjectCatalog catalog;
        auto catalogResult = catalog.loadFromFile("assets/objects/test_objects.json");
        if (!catalogResult.isOk()) {
            std::cerr << "Error cargando catalogo: " << catalogResult.errorMessage() << "\n";
            return 1;
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
            mapW = loadedMap.getWidth();
            mapH = loadedMap.getHeight();
            levelTitle = loadedLevel.name;
            loaded = true;
        }

        // Tileset: el que declare el TMX cargado, o el de pruebas para un
        // mapa nuevo (mismo criterio que Application::init). Las regiones
        // se declaran en bloque con la convencion de Tiled -- GID
        // firstGid+i = celda i en orden de lectura -- en vez de a mano,
        // asi cualquier tileset queda cubierto sin tocar codigo.
        const std::string tilesetPath =
            loaded && !loadedMap.tilesetImagePath().empty()
                ? loadedMap.tilesetImagePath()
                : std::string("assets/textures/test_checker.png");
        auto textureResult = textureManager.load("tileset", tilesetPath);
        if (!textureResult.isOk()) {
            std::cerr << "Error cargando tileset " << tilesetPath << ": "
                      << textureResult.errorMessage() << "\n";
            return 1;
        }
        const int cellW = loaded && loadedMap.getTilesetTileWidth() > 0
                              ? loadedMap.getTilesetTileWidth()
                              : 8;
        const int cellH = loaded && loadedMap.getTilesetTileHeight() > 0
                              ? loadedMap.getTilesetTileHeight()
                              : 8;
        TextureAtlas atlas(textureResult.value(), cellW, cellH);
        const int atlasCols = loaded && loadedMap.getTilesetColumns() > 0
                                  ? loadedMap.getTilesetColumns()
                                  : std::max(1, textureResult.value()->getWidth() / cellW);
        const int atlasRows = std::max(1, textureResult.value()->getHeight() / cellH);
        const int firstGid = loaded ? loadedMap.getTilesetFirstGid() : 1;
        for (int row = 0; row < atlasRows; ++row) {
            for (int col = 0; col < atlasCols; ++col) {
                atlas.defineRegion(firstGid + row * atlasCols + col, col, row);
            }
        }

        EditorState editor(mapW, mapH);
        // Paleta de tiles: TODOS los GIDs del tileset, no solo dos --
        // con un tileset de ciudad hay que poder pintar cada material.
        std::vector<int> tileGids;
        for (int i = 0; i < atlasCols * atlasRows; ++i) {
            tileGids.push_back(firstGid + i);
        }
        editor.setTilePalette(tileGids);
        editor.setObjectPalette({"arbusto", "llave_cueva", "pocion", "eter", "slime"});

        if (loaded) {
            // Volcado del TMX al editor. Solo la capa 0: EditorState
            // maneja UNA capa (ver su nota de alcance), asi que abrir un
            // mapa multicapa y volver a guardar PERDERIA las demas --
            // por eso se avisa en vez de hacerlo callando.
            if (loadedMap.getLayerCount() > 1) {
                std::cerr << "Aviso: el mapa tiene " << loadedMap.getLayerCount()
                          << " capas y el editor solo maneja la primera; si guardas, "
                             "las demas se pierden.\n";
            }
            for (int y = 0; y < mapH; ++y) {
                for (int x = 0; x < mapW; ++x) {
                    editor.paintTile(x, y, loadedMap.getTile(0, x, y).getTilesetID());
                }
            }
            for (const ObjectSpawn& spawn : loadedLevel.objects) {
                // placeSpawn y no placeObject: conserva el destino de las
                // puertas (ver EditorState::exportLevelJson).
                editor.placeSpawn(spawn);
            }
            editor.setPlayerStart(loadedLevel.playerStart);
            std::cout << "Cargado \"" << loadedLevel.name << "\" (" << mapW << "x" << mapH
                      << ", " << loadedLevel.objects.size() << " objetos) desde "
                      << opts.loadLevelPath << "\n";
        }
        // Abrir/importar un nivel establece el documento base: Undo debe
        // afectar solo a lo que el autor haga DESPUES de abrirlo.
        editor.clearHistory();

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
        headerText.setText("MOTORGRAFICO LEVEL EDITOR - " + levelTitle + "  " +
                           std::to_string(mapW) + "X" + std::to_string(mapH));

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
            "5 " + std::string(toolName(EditorTool::SetPlayerStart))};
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
        toolsTitle.setText("HERRAMIENTAS");
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
        auto tileLabel = [](int gid) {
            std::string name = (gid == 2) ? "COLISION" : "SUELO";
            return "GID " + std::to_string(gid) + " " + name;
        };
        std::vector<std::string> tileOptions;
        for (int gid : editor.tilePalette()) {
            tileOptions.push_back(tileLabel(gid));
        }
        std::vector<std::string> objectOptions = editor.objectPalette();

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
        const float paletteW = std::max({tileSizeC.x, objectSizeC.x,
                                         font.measureText("PALETA  Q/E").x});
        const float paletteH = std::max(tileSizeC.y, objectSizeC.y);
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
        paletteTitle.setText("PALETA  Q/E");
        paletteTitle.setColor(kDimText);

        HudTransform paletteMenuT;
        paletteMenuT.anchor = HudAnchor::TopLeft;
        paletteMenuT.offset = {8.0f + kPad, toolsBottom + kPad + lineH + 6.0f};
        paletteMenuT.size = {paletteW, paletteH};
        HudCommandMenu tileMenu(paletteMenuT, &font,
                                paletteWindow(tileOptions, 0, dummyFirst));
        HudCommandMenu objectMenu(paletteMenuT, &font,
                                  paletteWindow(objectOptions, 0, dummyFirst));

        // Inspector (derecha): ficha de la entrada seleccionada en la
        // paleta, con AVISOS de ids sin resolver (defecto 04 del
        // documento de diseno: el skill fantasma del slime paso
        // inadvertido porque nada lo mostraba; aqui el editor lo grita).
        // Los ids registrados replican los de Application::init hasta
        // que las habilidades se carguen desde JSON (mismo comentario
        // alli).
        const std::vector<std::string> knownSkillIds{"tajo", "cura", "golpe_gelatinoso"};

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

        // Barra de estado (abajo, ancho completo): controles, y el
        // resultado del ultimo guardado durante unos segundos.
        HudTransform statusPanelT;
        statusPanelT.anchor = HudAnchor::BottomCenter;
        statusPanelT.offset = {0.0f, 8.0f};
        statusPanelT.size = {static_cast<float>(width) - 16.0f, lineH + 2.0f * kPad};
        HudPanel statusPanel(statusPanelT, &whiteTexture, kPanelColor);
        statusPanel.setBorder(kBorderColor, 2.0f);

        const std::string kControlsLine =
            "1-5 HERRAM  Q/E PALETA  CTRL+Z/Y HIST  F5 GUARDAR  F6 VALIDAR  F7 JUGAR  F8/F9 ESCENARIO  ESC PROYECTOS";
        // Peor caso del texto de estado (controles + contadores) para que el
        // panel de fondo cubra siempre la linea, aunque se anada el
        // "...TILES 64  OBJETOS 5" del final.
        // Peor caso real con mapas grandes: un 64x64 son 4096 tiles.
        const std::string kControlsLineWorst = kControlsLine + "   |   TILES 4096  OBJETOS 999";
        HudTransform statusTextT;
        statusTextT.anchor = HudAnchor::BottomCenter;
        statusTextT.offset = {0.0f, 8.0f + kPad};
        statusTextT.size = {font.measureText(kControlsLineWorst).x, lineH};
        HudText statusText(statusTextT, &font);
        statusText.setText(kControlsLine);
        int savedMessageFrames = 0;  // frames restantes del aviso de guardado

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
        hud.addElement(&statusPanel);
        hud.addElement(&statusText);

        KeyEdge keys;
        GLFWwindow* glfwWin = window.handle();
        keys.prime(glfwWin);   // el ENTER con que abriste el proyecto no cuenta
        int frame = 0;
        bool volverAlHub = false;   // ESC en la edicion -> pantalla de proyectos
        bool cambiarEscenario = false;  // F8/F9: reconstruye el documento con otro nivel

        // Estado de animacion del editor. Los widgets HUD no tienen
        // update(), asi que el estado que cambia con el tiempo (cursor
        // pulsante, flash de guardado) vive aqui y se avanza cada frame.
        float animTime = 0.0f;        // segundos acumulados (para senos)
        float saveFlashAlpha = 0.0f;  // overlay verde al guardar; decae a 0

        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            const float dt = 1.0f / 60.0f;
            window.pollEvents();

            // --- Avance del tiempo de animacion ---
            animTime += dt;
            if (saveFlashAlpha > 0.0f) {
                saveFlashAlpha -= dt * 2.5f;  // ~0.4s hasta 0
                if (saveFlashAlpha < 0.0f) {
                    saveFlashAlpha = 0.0f;
                }
            }

            // --- Input de teclado ---
            // ESC vuelve a la pantalla de proyectos, no cierra el editor.
            // Para cerrar del todo hay que pulsarlo otra vez alli. Cambiar
            // de proyecto no puede costar relanzar el binario.
            if (keys.pressed(glfwWin, GLFW_KEY_ESCAPE) || keys.pressed(glfwWin, GLFW_KEY_M)) {
                volverAlHub = true;
                break;
            }
            if (keys.pressed(glfwWin, GLFW_KEY_1))
                editor.setTool(EditorTool::PaintTile);
            if (keys.pressed(glfwWin, GLFW_KEY_2))
                editor.setTool(EditorTool::EraseTile);
            if (keys.pressed(glfwWin, GLFW_KEY_3))
                editor.setTool(EditorTool::PlaceObject);
            if (keys.pressed(glfwWin, GLFW_KEY_4))
                editor.setTool(EditorTool::RemoveObject);
            if (keys.pressed(glfwWin, GLFW_KEY_5))
                editor.setTool(EditorTool::SetPlayerStart);
            const bool controlDown = glfwGetKey(glfwWin, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                                     glfwGetKey(glfwWin, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
                                     glfwGetKey(glfwWin, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                                     glfwGetKey(glfwWin, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
            if (controlDown && keys.pressed(glfwWin, GLFW_KEY_Z)) {
                statusText.setText(editor.undo() ? "DESHECHO" : "NO HAY CAMBIOS QUE DESHACER");
                savedMessageFrames = 120;
            }
            if (controlDown && keys.pressed(glfwWin, GLFW_KEY_Y)) {
                statusText.setText(editor.redo() ? "REHECHO" : "NO HAY CAMBIOS QUE REHACER");
                savedMessageFrames = 120;
            }
            if (keys.pressed(glfwWin, GLFW_KEY_Q))
                editor.prevPaletteEntry();
            if (keys.pressed(glfwWin, GLFW_KEY_E))
                editor.nextPaletteEntry();

            // Zoom (+/-, y 0 para volver a 1x): imprescindible en cuanto
            // el mapa no cabe en pantalla -- un 64x64 mide 4096x2048
            // pixeles de mundo, mas del triple del viewport. Rango
            // acotado: por debajo de 0.15 los tiles son ilegibles y por
            // encima de 3 no se ve contexto suficiente para editar.
            if (keys.pressed(glfwWin, GLFW_KEY_EQUAL) || keys.pressed(glfwWin, GLFW_KEY_KP_ADD)) {
                camera.setZoom(std::min(camera.zoom() * 1.25f, 3.0f));
            }
            if (keys.pressed(glfwWin, GLFW_KEY_MINUS) ||
                keys.pressed(glfwWin, GLFW_KEY_KP_SUBTRACT)) {
                camera.setZoom(std::max(camera.zoom() / 1.25f, 0.15f));
            }
            if (keys.pressed(glfwWin, GLFW_KEY_0)) {
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
            if (glfwGetKey(glfwWin, GLFW_KEY_W) == GLFW_PRESS)
                camera.move({0.0f, -panSpeed});
            if (glfwGetKey(glfwWin, GLFW_KEY_S) == GLFW_PRESS)
                camera.move({0.0f, panSpeed});
            if (glfwGetKey(glfwWin, GLFW_KEY_A) == GLFW_PRESS)
                camera.move({-panSpeed, 0.0f});
            if (glfwGetKey(glfwWin, GLFW_KEY_D) == GLFW_PRESS)
                camera.move({panSpeed, 0.0f});

            // HOME: volver al centro del mapa (perderse en un 64x64 es
            // facil, y no habia forma de reorientarse salvo a ojo).
            if (keys.pressed(glfwWin, GLFW_KEY_HOME)) {
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
                return keys.pressed(glfwWin, letra) || keys.pressed(glfwWin, funcion);
            };

            // Guardar es una funcion porque probar tiene que
            // guardar antes: probar lo que hay en disco mientras miras
            // otra cosa en pantalla es la peor forma de perder una hora.
            auto guardarNivel = [&]() {
                TmxTilesetSettings settings;  // defaults = tileset del checker
                settings.collisionGids = {2};
                std::ofstream tmxOut(saveTmx, std::ios::binary);
                tmxOut << editor.exportTmx(settings);
                std::ofstream jsonOut(saveJson, std::ios::binary);
                jsonOut << editor.exportLevelJson(saveName, saveTmx);
                std::cout << "Guardado: " << saveTmx << " + " << saveJson << "\n";
                // Feedback en la barra de estado (~4s a 60fps); el bucle
                // la devuelve a los controles al caducar.
                std::string aviso = "GUARDADO: " + saveJson;
                for (char& ch : aviso) {
                    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                }
                statusText.setText(aviso);
                savedMessageFrames = 240;
                // Disparar el flash de guardado (overlay verde breve).
                saveFlashAlpha = 1.0f;
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
            const int direccionEscenario =
                keys.pressed(glfwWin, GLFW_KEY_F8) ? -1 :
                (keys.pressed(glfwWin, GLFW_KEY_F9) ? 1 : 0);
            if (direccionEscenario != 0) {
                if (opts.projectId.empty()) {
                    statusText.setText("F8/F9 NECESITA UN PROYECTO: ABRELO DESDE LA PANTALLA");
                    savedMessageFrames = 240;
                } else {
                    auto indice = Editor::ProjectIndex::scan("assets");
                    const Editor::Project* proyecto =
                        indice.isOk() ? indice.value().find(opts.projectId) : nullptr;
                    if (proyecto == nullptr || proyecto->levels.empty()) {
                        statusText.setText("EL PROYECTO NO DECLARA OTROS ESCENARIOS");
                        savedMessageFrames = 240;
                    } else {
                        guardarNivel();
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
                        cambiarEscenario = true;
                        break;
                    }
                }
            }

            // F7 — MODO JUGADOR. Guarda y lanza ./juego sobre este nivel,
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
            if (pedida(GLFW_KEY_P, GLFW_KEY_F7)) {
                guardarNivel();
                // El catalogo del proyecto, si declara alguno; si no, el
                // de Boundington, que es el unico completo hoy.
                std::string catalogoDePrueba = "assets/objects/boundington_npcs.json";
                if (!opts.projectId.empty()) {
                    auto ri = Editor::ProjectIndex::scan("assets");
                    if (ri.isOk()) {
                        const Editor::Project* pp = ri.value().find(opts.projectId);
                        if (pp != nullptr && !pp->catalogs.empty()) {
                            catalogoDePrueba = "assets/objects/" + pp->catalogs.front();
                        }
                    }
                }
                // El binario esta junto al del editor cuando se compila con
                // CMake, pero el cwd es la raiz del repo (los assets son
                // rutas relativas): se prueban los dos sitios.
                std::string exe = "./juego";
                if (std::FILE* f = std::fopen("build/juego", "rb")) {
                    std::fclose(f);
                    exe = "./build/juego";
                }
                const std::string orden =
                    exe + " --nivel \"" + saveJson + "\" --catalogo \"" + catalogoDePrueba + "\"";
                std::cout << "Probando: " << orden << "\n";
                statusText.setText("MODO JUGADOR... (cierra el juego para volver al editor)");
                savedMessageFrames = 240;
                const int codigo = std::system(orden.c_str());
                if (codigo != 0) {
                    std::cout << "El juego termino con codigo " << codigo << "\n";
                    statusText.setText("NO SE PUDO PROBAR: COMPILA EL OBJETIVO 'juego' PRIMERO");
                    savedMessageFrames = 240;
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
                savedMessageFrames = 240;
            }

            // --- Raton -> celda bajo el cursor ---
            double mouseX = 0.0;
            double mouseY = 0.0;
            glfwGetCursorPos(glfwWin, &mouseX, &mouseY);
            Vector2 world = camera.screenToWorld(
                Vector2{static_cast<float>(mouseX), static_cast<float>(mouseY)});
            GridCoord hovered = IsoMath::screenToGrid(world, static_cast<float>(kTileW),
                                                      static_cast<float>(kTileH));

            bool leftDown = glfwGetMouseButton(glfwWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            bool rightDown = glfwGetMouseButton(glfwWin, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (leftDown) {
                // Mantener pulsado pinta en arrastre (deliberado: es lo
                // comodo para rellenar zonas; para objetos, placeObject
                // reemplaza en la misma celda asi que no duplica).
                editor.applyAt(hovered.x, hovered.y);
            } else if (rightDown) {
                // Boton derecho: el "contrario" rapido de la herramienta.
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
                    Vector2 pos = IsoMath::gridToScreen(GridCoord{x, y}, static_cast<float>(kTileW),
                                                        static_cast<float>(kTileH)) +
                                  tileDrawOffset;
                    int gid = editor.tileAt(x, y);
                    if (gid != 0) {
                        batch.submit(pos, tileSize, atlas.getUV(gid), atlas.texture(),
                                     Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    } else {
                        // Celda vacia: rejilla tenue para ver donde se
                        // puede pintar (sin esto un mapa en blanco es
                        // una pantalla vacia sin referencia).
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
                if (def != nullptr && def->spriteId >= 0) {
                    batch.submit(pos, tileSize, atlas.getUV(def->spriteId), atlas.texture(),
                                 Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                } else {
                    batch.submit(pos, tileSize, UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                                 Vector4{0.9f, 0.9f, 0.3f, 0.5f});
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
                float pulse = 0.35f + 0.15f * std::sin(animTime * 4.0f);
                batch.submit(pos, tileSize, UVRect{0.0f, 0.0f, 1.0f, 1.0f}, &whiteTexture,
                             Vector4{1.0f, 1.0f, 1.0f, pulse});
            }
            batch.end();

            // --- HUD: sincronizar widgets con el estado del editor ---
            const bool objectMode = editor.tool() == EditorTool::PlaceObject ||
                                    editor.tool() == EditorTool::RemoveObject;

            // El menu de herramientas refleja editor.tool() (el orden de
            // toolOptions es el del enum, ver su comentario).
            toolsMenu.setSelectedIndex(static_cast<std::size_t>(editor.tool()));

            // Paleta visible segun herramienta, con su cursor en la
            // entrada activa (busqueda lineal: las paletas tienen un
            // punado de entradas).
            tileMenu.setVisible(!objectMode);
            objectMenu.setVisible(objectMode);
            // Indice absoluto de la seleccion en la paleta completa...
            const std::vector<int>& paletteGids = editor.tilePalette();
            std::size_t tileSel = 0;
            for (std::size_t i = 0; i < paletteGids.size(); ++i) {
                if (paletteGids[i] == editor.selectedTileGid()) tileSel = i;
            }
            const std::vector<std::string>& paletteIds = editor.objectPalette();
            std::size_t objSel = 0;
            for (std::size_t i = 0; i < paletteIds.size(); ++i) {
                if (paletteIds[i] == editor.selectedObjectId()) objSel = i;
            }
            // ...y ventana visible + indice RELATIVO a ella (el menu solo
            // conoce lo que se esta mostrando).
            std::size_t tileFirst = 0;
            std::size_t objFirst = 0;
            tileMenu.setOptions(paletteWindow(tileOptions, tileSel, tileFirst));
            tileMenu.setSelectedIndex(tileSel - tileFirst);
            objectMenu.setOptions(paletteWindow(objectOptions, objSel, objFirst));
            objectMenu.setSelectedIndex(objSel - objFirst);

            // Celda bajo el cursor (cabecera, derecha): coordenada + gid
            // + objeto si lo hay. tileAt() ya devuelve 0 fuera de rango.
            std::string hoverInfo = "CELDA " + std::to_string(hovered.x) + "," +
                                    std::to_string(hovered.y) + "  GID " +
                                    std::to_string(editor.tileAt(hovered.x, hovered.y));
            for (const ObjectSpawn& spawn : editor.objects()) {
                if (spawn.position.x == hovered.x && spawn.position.y == hovered.y) {
                    hoverInfo += "  [" + spawn.objectId + "]";
                    break;
                }
            }
            hoverText.setText(hoverInfo);

            // Inspector: ficha de la seleccion de la paleta activa, con
            // avisos de ids sin resolver en su propia linea naranja.
            std::string inspector;
            std::string warnings;
            if (editor.tool() == EditorTool::SetPlayerStart) {
                const GridCoord start = editor.playerStart();
                inspector = "INSPECTOR\n\nINICIO JUGADOR\nX " + std::to_string(start.x) +
                            "  Y " + std::to_string(start.y) +
                            "\nCLICK EN UNA CELDA\nPARA REUBICARLO";
            } else if (objectMode) {
                const std::string selectedId = editor.selectedObjectId();
                const ObjectDefinition* def = catalog.find(selectedId);
                if (def == nullptr) {
                    inspector = "INSPECTOR\n\n" + selectedId + "\nSIN DEFINICION EN CATALOGO";
                    warnings = "! ID SIN RESOLVER";
                } else {
                    const char* category = def->category == ObjectCategory::Enemy    ? "ENEMY"
                                           : def->category == ObjectCategory::Pickup ? "PICKUP"
                                                                                     : "PROP";
                    inspector = "INSPECTOR\n\nID " + def->id + "\nNOMBRE " + def->name +
                                "\nCATEGORIA " + category + "\nSPRITE " +
                                std::to_string(def->spriteId) + "\nBLOQUEA " +
                                (def->blocksMovement ? "SI" : "NO");
                    if (def->category == ObjectCategory::Enemy) {
                        inspector += "\nPV " + std::to_string(def->combat.maxHealth) + "  PM " +
                                     std::to_string(def->combat.maxMana);
                        for (const std::string& skillId : def->combat.skillIds) {
                            inspector += "\nSKILL " + skillId;
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
                        inspector += std::string("\nEFECTO ") + effect;
                        if (def->pickup.effect != PickupEffect::None) {
                            inspector += " " + std::to_string(def->pickup.power);
                        }
                    }
                }
            } else {
                const int gid = editor.selectedTileGid();
                inspector = "INSPECTOR\n\nTILE " + tileLabel(gid) + "\nCOLISION " +
                            (gid == 2 ? "SI" : "NO") + "\nINICIO " +
                            std::to_string(editor.playerStart().x) + "," +
                            std::to_string(editor.playerStart().y);
            }
            inspectorText.setText(inspector);
            inspectorWarn.setText(warnings);

            // Barra de estado: vuelve a los controles cuando caduca el
            // aviso de guardado (ver el manejador de F5). Cuando muestra los
            // controles, anade contadores en tiempo real (tiles pintados /
            // objetos colocados) para dar feedback del progreso del nivel.
            if (savedMessageFrames > 0) {
                --savedMessageFrames;
                if (savedMessageFrames == 0) {
                    statusText.setText(kControlsLine);
                }
            } else {
                int tileCount = 0;
                for (int y = 0; y < editor.height(); ++y) {
                    for (int x = 0; x < editor.width(); ++x) {
                        if (editor.tileAt(x, y) != 0)
                            ++tileCount;
                    }
                }
                statusText.setText(kControlsLine + "   |   TILES " + std::to_string(tileCount) +
                                   "  OBJETOS " + std::to_string(editor.objects().size()));
            }
            // El HUD va en su propia tanda begin()/end() (ver
            // HudManager::render() y demo_hud.cpp: el llamador rodea la
            // llamada; render() solo hace submit()).
            batch.begin();
            hud.render(batch, *shader, width, height);
            // Overlay de flash de guardado: quad verde semitransparente
            // cubriendo toda la pantalla, alpha = saveFlashAlpha. Desciende
            // a 0 en ~0.4s (ver el avance de animacion arriba). Feedback
            // claro de que F5 funciono, ademas del texto de la barra de
            // estado.
            if (saveFlashAlpha > 0.0f) {
                Vector4 flashCol{0.20f, 0.70f, 0.30f, saveFlashAlpha * 0.30f};
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
            if (maxFrames >= 0 && frame == maxFrames - 1) {
                glFinish();
                constexpr int kGridW = 160;
                constexpr int kGridH = 90;
                std::vector<unsigned char> px(static_cast<std::size_t>(kGridW) * kGridH * 3);
                for (int gy = 0; gy < kGridH; ++gy) {
                    for (int gx = 0; gx < kGridW; ++gx) {
                        unsigned char rgb[3] = {0, 0, 0};
                        glReadPixels(gx * width / kGridW, gy * height / kGridH, 1, 1, GL_RGB,
                                     GL_UNSIGNED_BYTE, rgb);
                        std::size_t idx = (static_cast<std::size_t>(gy) * kGridW + gx) * 3;
                        px[idx + 0] = rgb[0];
                        px[idx + 1] = rgb[1];
                        px[idx + 2] = rgb[2];
                    }
                }
                std::ofstream out("level_editor_output.ppm", std::ios::binary);
                out << "P6\n" << kGridW << " " << kGridH << "\n255\n";
                out.write(reinterpret_cast<char*>(px.data()),
                          static_cast<std::streamsize>(px.size()));
                std::cout << "Framebuffer volcado a level_editor_output.ppm.\n";
            }

            glDisable(GL_BLEND);

            // Modo humo (argumento numerico): en el ULTIMO frame se
            // vuelca el framebuffer, igual que los demas demos GL. Asi
            // "./level_editor 3" deja un level_editor_output.ppm con el
            // HUD real dibujado, inspeccionable sin captura a mano.
            // Antes del swapBuffers(): despues, el buffer que se leeria
            // ya no es el recien dibujado.
            if (maxFrames >= 0 && frame == maxFrames - 1) {
                writeFramebufferPPM("level_editor_output.ppm", width, height);
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
        if (!volverAlHub && !cambiarEscenario) {
            break;
        }
        if (cambiarEscenario) {
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
