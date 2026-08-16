// La pantalla de creacion de personaje, dibujada.
//
// Ejecutable APARTE y no un modo dentro de juego.cpp a proposito:
// Application.* y juego.cpp son zona de otra tarea en el
// PARALLEL_DEVELOPMENT_BOARD (catalogos compuestos, en curso). Meter aqui
// un paso previo al prologo seria tocar el terreno de otro justo mientras
// lo esta editando. Cuando esa tarea cierre, engancharlo es una llamada.
//
//   ./creacion_personaje            crea un personaje y lo escribe a JSON
//   ./creacion_personaje 3          modo humo: 3 frames y sale (CI)
//   ./creacion_personaje --salida f.json
//
// Controles:
//   W/S      moverse por la lista (o entre los cuatro stats)
//   + / -    subir y bajar el stat marcado
//   letras   filtrar la lista (43 razas, 61 clases: sin filtro no se anda)
//   BACKSPACE borrar del filtro o del nombre
//   ENTER    siguiente paso; en el resumen, confirmar
//   ESC      volver; en el primer paso, cancelar
//
// TODA la logica esta en RPG::CharacterCreationScreen, GL-free y probada
// en demo_creacion_personaje. Aqui solo se pinta lo que diga y se le
// pasan las teclas. Es el mismo reparto que la pantalla de proyectos del
// editor, y por el mismo motivo: lo que se mete en un fichero con GL
// delante no se puede ejecutar en CI, solo compilar.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "Engine/Window.h"
#include "RPG/CharacterCreationScreen.h"
#include "RPG/TierRules.h"
#include "Render/BitmapFont.h"
#include "Render/HudElement.h"
#include "Render/HudManager.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Core/Resources/ShaderManager.h"
#include "Render/SpriteBatch.h"
#include "Core/Resources/Texture.h"

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp).
#include <glad/glad.h>

#include <GLFW/glfw3.h>

using RPG::CharacterCreation;
using RPG::CharacterCreationScreen;

namespace {

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

// Deteccion de flanco, con prime() para que la tecla que te trajo aqui no
// cuente como pulsacion nueva (mismo problema y misma cura que en el
// editor: ESC seguia hundido al cambiar de fase).
class KeyEdge {
public:
    bool pressed(GLFWwindow* w, int key) {
        const bool down = glfwGetKey(w, key) == GLFW_PRESS;
        const bool edge = down && !m_was[key];
        m_was[key] = down;
        return edge;
    }
    void prime(GLFWwindow* w) {
        for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
            m_was[k] = glfwGetKey(w, k) == GLFW_PRESS;
        }
    }

private:
    bool m_was[GLFW_KEY_LAST + 1] = {};
};

std::string aMayus(std::string t) {
    for (char& c : t) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
    }
    return t;
}

// El JSON de la ficha. Se escribe a mano y no con una libreria porque el
// motor no tiene serializador: JsonValue solo lee (ver su header).
std::string fichaAJson(const RPG::CharacterSheet& f, const RPG::CreationChoice& e) {
    std::string s = "{\n";
    auto campo = [&](const char* k, const std::string& v, bool coma = true) {
        s += "  \"" + std::string(k) + "\": \"" + v + "\"" + (coma ? ",\n" : "\n");
    };
    campo("nombre", f.displayName);
    campo("razaId", f.raceId);
    campo("claseId", f.classId);
    campo("trasfondoId", f.backgroundId);
    campo("deidadId", f.deityId);
    s += "  \"tier\": " + std::to_string(f.tier) + ",\n";
    s += "  \"stats\": { \"CON\": " + std::to_string(f.con()) + ", \"DES\": " +
         std::to_string(f.des()) + ", \"INT\": " + std::to_string(f.int_()) +
         ", \"CAR\": " + std::to_string(f.car()) + " },\n";
    s += "  \"vida\": " + std::to_string(f.healthCap()) + ",\n";
    s += "  \"narrativa\": {\n";
    s += "    \"vinculo\": \"" + e.bondId + "\",\n";
    s += "    \"miedo\": \"" + e.fearId + "\",\n";
    s += "    \"defecto\": \"" + e.flawId + "\",\n";
    s += "    \"meta\": \"" + e.goalId + "\",\n";
    s += "    \"ideal\": \"" + e.idealId + "\",\n";
    s += "    \"personalidad\": \"" + e.personalityId + "\",\n";
    s += "    \"virtud\": \"" + e.virtueId + "\"\n";
    s += "  },\n";
    s += "  \"habilidades\": [";
    for (std::size_t i = 0; i < f.knownSkillIds.size(); ++i) {
        s += (i ? ", \"" : "\"") + f.knownSkillIds[i] + "\"";
    }
    s += "],\n  \"equipo\": [";
    for (std::size_t i = 0; i < f.inventoryEquipment.size(); ++i) {
        s += (i ? ", \"" : "\"") + f.inventoryEquipment[i].first + "\"";
    }
    s += "]\n}\n";
    return s;
}

}  // namespace

int main(int argc, char** argv) {
    const int width = 1280;
    const int height = 720;
    int maxFrames = -1;
    std::string salida = "personaje.json";
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--salida" && i + 1 < argc) {
            salida = argv[++i];
        } else if (!a.empty() && a[0] != '-') {
            maxFrames = std::atoi(a.c_str());
        }
    }

    auto reglasCarga = CharacterCreation::load("assets/catalogs");
    if (!reglasCarga.isOk()) {
        std::fprintf(stderr, "No se pudieron leer los catalogos: %s\n",
                     reglasCarga.errorMessage().c_str());
        return 1;
    }
    const CharacterCreation& reglas = reglasCarga.value();
    CharacterCreationScreen pantalla(reglas);

    try {
        Window window(width, height, "Motor Grafico - Crear personaje");
        ShaderManager shaders;
        auto shader = shaders.load("sprite", "assets/shaders/sprite");
        if (!shader.isOk()) {
            std::fprintf(stderr, "shader: %s\n", shader.errorMessage().c_str());
            return 1;
        }

        SpriteBatch batch;
        Texture blanco = makeWhiteTexture();
        BitmapFont fuente(/*scale=*/2);
        const float lh = static_cast<float>(fuente.lineHeight());
        const Vector4 kFondo{0.05f, 0.05f, 0.08f, 0.92f};
        const Vector4 kMarco{0.55f, 0.55f, 0.65f, 0.9f};
        const Vector4 kBlanco{1.0f, 1.0f, 1.0f, 1.0f};
        const Vector4 kApagado{0.70f, 0.70f, 0.75f, 1.0f};
        const Vector4 kAviso{1.0f, 0.60f, 0.25f, 1.0f};
        const Vector4 kBien{0.45f, 0.90f, 0.55f, 1.0f};

        KeyEdge teclas;
        GLFWwindow* win = window.handle();
        teclas.prime(win);
        int frame = 0;
        bool terminado = false;

        while (!window.shouldClose() && (maxFrames < 0 || frame < maxFrames)) {
            window.pollEvents();

            // --- Teclas -> la pantalla, que es quien sabe que hacer ---
            if (!terminado) {
                CharacterCreationScreen::Accion accion =
                    CharacterCreationScreen::Accion::Ninguna;
                if (teclas.pressed(win, GLFW_KEY_W) || teclas.pressed(win, GLFW_KEY_UP))
                    accion = pantalla.tecla('w');
                if (teclas.pressed(win, GLFW_KEY_S) || teclas.pressed(win, GLFW_KEY_DOWN))
                    accion = pantalla.tecla('s');
                if (teclas.pressed(win, GLFW_KEY_ENTER)) accion = pantalla.tecla('\n');
                if (teclas.pressed(win, GLFW_KEY_ESCAPE)) accion = pantalla.tecla(27);
                if (teclas.pressed(win, GLFW_KEY_BACKSPACE)) pantalla.tecla('\b');
                if (teclas.pressed(win, GLFW_KEY_EQUAL) || teclas.pressed(win, GLFW_KEY_KP_ADD))
                    pantalla.tecla('+');
                if (teclas.pressed(win, GLFW_KEY_MINUS) ||
                    teclas.pressed(win, GLFW_KEY_KP_SUBTRACT))
                    pantalla.tecla('-');
                // Letras y numeros: filtran, o escriben el nombre.
                const bool shift = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                   glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
                for (int k = GLFW_KEY_A; k <= GLFW_KEY_Z; ++k) {
                    if (teclas.pressed(win, k)) {
                        // W y S navegan; solo escriben si el paso es el del
                        // nombre. Es el precio de no usar el callback de
                        // texto de GLFW (Window ya gestiona los suyos).
                        const bool navega = (k == GLFW_KEY_W || k == GLFW_KEY_S);
                        if (!navega || pantalla.paso() == CharacterCreationScreen::Paso::Nombre) {
                            const char base = shift ? 'A' : 'a';
                            pantalla.tecla(static_cast<char>(base + (k - GLFW_KEY_A)));
                        }
                    }
                }
                for (int k = GLFW_KEY_0; k <= GLFW_KEY_9; ++k) {
                    if (teclas.pressed(win, k)) {
                        pantalla.tecla(static_cast<char>('0' + (k - GLFW_KEY_0)));
                    }
                }
                if (teclas.pressed(win, GLFW_KEY_SPACE)) pantalla.tecla(' ');

                if (accion == CharacterCreationScreen::Accion::Cancelado) {
                    std::printf("Cancelado: no se ha creado ningun personaje.\n");
                    return 0;
                }
                if (accion == CharacterCreationScreen::Accion::Terminado) {
                    auto hecha = pantalla.construir();
                    if (hecha.isOk()) {
                        RPG::CharacterSheet f = hecha.value();
                        RPG::TierRules tr;
                        reglas.recalcular(f, tr);
                        std::ofstream out(salida, std::ios::binary);
                        out << fichaAJson(f, pantalla.eleccion());
                        std::printf("Personaje escrito en %s\n", salida.c_str());
                        terminado = true;
                    }
                }
            }

            // --- Pintar ---
            glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            HudManager hud;
            std::vector<std::unique_ptr<IHudElement>> vivos;
            auto panel = [&](HudAnchor a, Vector2 off, Vector2 size) {
                auto* p = new HudPanel(HudTransform{a, off, size}, &blanco, kFondo);
                p->setBorder(kMarco, 2.0f);
                vivos.emplace_back(p);
                hud.addElement(p);
            };
            auto texto = [&](HudAnchor a, Vector2 off, const std::string& s, const Vector4& col) {
                auto* t = new HudText(HudTransform{a, off, {0.0f, 0.0f}}, &fuente);
                t->setText(s);
                t->setColor(col);
                vivos.emplace_back(t);
                hud.addElement(t);
            };

            // Cabecera con el paso
            panel(HudAnchor::TopLeft, {20.0f, 20.0f}, {static_cast<float>(width) - 40.0f, 46.0f});
            texto(HudAnchor::TopLeft, {34.0f, 34.0f},
                  "PASO " + std::to_string(pantalla.numeroDePaso()) + "/" +
                      std::to_string(CharacterCreationScreen::totalDePasos()) + "   " +
                      pantalla.tituloPaso(),
                  kBlanco);

            // Panel izquierdo: la lista, o los stats, o el nombre
            panel(HudAnchor::TopLeft, {20.0f, 80.0f}, {700.0f, lh * 14.0f + 30.0f});
            const auto paso = pantalla.paso();
            if (paso == CharacterCreationScreen::Paso::Reparto) {
                static const char* kN[4] = {"CON", "DES", "INT", "CAR"};
                for (int i = 0; i < 4; ++i) {
                    const bool m = i == pantalla.statMarcado();
                    texto(HudAnchor::TopLeft, {34.0f, 96.0f + lh * static_cast<float>(i) * 1.6f},
                          std::string(m ? "> " : "  ") + kN[i] + "   " +
                              std::string(static_cast<std::size_t>(
                                              pantalla.eleccion().baseStats[static_cast<std::size_t>(i)]),
                                          '#') +
                              "  " +
                              std::to_string(
                                  pantalla.eleccion().baseStats[static_cast<std::size_t>(i)]),
                          m ? kBlanco : kApagado);
                }
                texto(HudAnchor::TopLeft, {34.0f, 96.0f + lh * 7.0f},
                      "PUNTOS LIBRES: " + std::to_string(pantalla.puntosLibres()) +
                          "    + Y - PARA REPARTIR",
                      pantalla.puntosLibres() == 0 ? kBien : kAviso);
            } else if (paso == CharacterCreationScreen::Paso::Nombre) {
                texto(HudAnchor::TopLeft, {34.0f, 100.0f},
                      "> " + pantalla.eleccion().displayName + "_", kBien);
            } else if (paso == CharacterCreationScreen::Paso::Resumen) {
                const auto& e = pantalla.eleccion();
                const std::vector<std::string> filas = {
                    "NOMBRE     " + aMayus(e.displayName),
                    "RAZA       " + aMayus(e.raceId),
                    "CLASE      " + aMayus(e.classId),
                    "TRASFONDO  " + aMayus(e.backgroundId),
                    "DEIDAD     " + aMayus(e.deityId.empty() ? "(ninguna)" : e.deityId),
                    "",
                    "CON " + std::to_string(pantalla.eleccion().baseStats[0]) + "   DES " +
                        std::to_string(pantalla.eleccion().baseStats[1]) + "   INT " +
                        std::to_string(pantalla.eleccion().baseStats[2]) + "   CAR " +
                        std::to_string(pantalla.eleccion().baseStats[3]) + "   (sin bonos de raza)",
                    "",
                    "ENTER PARA CONFIRMAR",
                };
                for (std::size_t i = 0; i < filas.size(); ++i) {
                    texto(HudAnchor::TopLeft, {34.0f, 96.0f + lh * static_cast<float>(i)}, filas[i],
                          i == filas.size() - 1 ? kBien : kApagado);
                }
            } else {
                // Lista con ventana de 12: con 61 clases no caben todas, y
                // recortar por arriba deja lo marcado fuera de pantalla.
                constexpr std::size_t kVentana = 12;
                std::size_t primera = 0;
                const std::size_t n = pantalla.opciones().size();
                if (n > kVentana && pantalla.marcado() > kVentana / 2) {
                    primera = pantalla.marcado() - kVentana / 2;
                    if (primera + kVentana > n) {
                        primera = n - kVentana;
                    }
                }
                for (std::size_t i = 0; i < kVentana && primera + i < n; ++i) {
                    const std::size_t idx = primera + i;
                    const bool m = idx == pantalla.marcado();
                    texto(HudAnchor::TopLeft, {34.0f, 96.0f + lh * static_cast<float>(i)},
                          (m ? "> " : "  ") + pantalla.etiquetas()[idx], m ? kBlanco : kApagado);
                }
                if (n == 0) {
                    texto(HudAnchor::TopLeft, {34.0f, 96.0f}, "(nada que elegir con este filtro)",
                          kAviso);
                }
            }

            // Panel derecho: lo elegido hasta ahora
            panel(HudAnchor::TopRight, {20.0f, 80.0f}, {480.0f, lh * 14.0f + 30.0f});
            {
                const auto& e = pantalla.eleccion();
                const std::vector<std::string> resumen = {
                    "LO QUE LLEVAS",
                    "",
                    "raza       " + (e.raceId.empty() ? "-" : e.raceId),
                    "clase      " + (e.classId.empty() ? "-" : e.classId),
                    "trasfondo  " + (e.backgroundId.empty() ? "-" : e.backgroundId),
                    "vinculo    " + (e.bondId.empty() ? "-" : e.bondId),
                    "miedo      " + (e.fearId.empty() ? "-" : e.fearId),
                    "defecto    " + (e.flawId.empty() ? "-" : e.flawId),
                    "meta       " + (e.goalId.empty() ? "-" : e.goalId),
                    "ideal      " + (e.idealId.empty() ? "-" : e.idealId),
                    "caracter   " + (e.personalityId.empty() ? "-" : e.personalityId),
                    "virtud     " + (e.virtueId.empty() ? "-" : e.virtueId),
                };
                for (std::size_t i = 0; i < resumen.size(); ++i) {
                    texto(HudAnchor::TopRight, {466.0f, 96.0f + lh * static_cast<float>(i)},
                          resumen[i], i == 0 ? kBlanco : kApagado);
                }
            }

            // Barra inferior: filtro, aviso y teclas
            panel(HudAnchor::BottomLeft, {20.0f, 20.0f},
                  {static_cast<float>(width) - 40.0f, 62.0f});
            std::string abajo = "W/S MOVER   ENTER SIGUIENTE   ESC VOLVER";
            if (paso == CharacterCreationScreen::Paso::Reparto) {
                abajo = "W/S ELEGIR STAT   +/- REPARTIR   ENTER SIGUIENTE   ESC VOLVER";
            } else if (paso == CharacterCreationScreen::Paso::Nombre) {
                abajo = "ESCRIBE EL NOMBRE   BACKSPACE BORRA   ENTER SIGUIENTE";
            }
            texto(HudAnchor::BottomLeft, {34.0f, 52.0f}, abajo, kApagado);
            if (!pantalla.filtro().empty()) {
                texto(HudAnchor::BottomLeft, {34.0f, 30.0f},
                      "BUSCANDO: " + aMayus(pantalla.filtro()) + "   (" +
                          std::to_string(pantalla.opciones().size()) + " DE " +
                          std::to_string(pantalla.totalSinFiltrar()) + ")",
                      kBien);
            } else if (!pantalla.aviso().empty()) {
                texto(HudAnchor::BottomLeft, {34.0f, 30.0f}, aMayus(pantalla.aviso()), kAviso);
            }

            if (terminado) {
                panel(HudAnchor::Center, {0.0f, 0.0f}, {760.0f, 120.0f});
                texto(HudAnchor::Center, {-340.0f, -20.0f}, "PERSONAJE CREADO", kBien);
                texto(HudAnchor::Center, {-340.0f, 16.0f}, "escrito en " + salida, kApagado);
            }

            batch.begin();
            hud.render(batch, *shader.value(), width, height);
            batch.end();
            window.swapBuffers();
            ++frame;
        }

        const GLenum err = glGetError();
        std::printf("[GL] glGetError() al salir = %d (esperado: 0)\n", static_cast<int>(err));
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error fatal: %s\n", e.what());
        return 1;
    }
    return 0;
}
