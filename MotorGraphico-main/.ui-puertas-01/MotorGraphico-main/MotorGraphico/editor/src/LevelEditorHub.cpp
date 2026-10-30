// EST-21 (relevo B) — Implementacion. Ver la cabecera.

#include "LevelEditorHub.h"

#include "LevelEditorInterno.h"
#include "Editor/ProjectHub.h"
#include "Core/Math/Vector4.h"
#include "Core/Resources/Texture.h"
#include "Render/BitmapFont.h"
#include "Render/HudElement.h"
#include "Render/HudManager.h"
#include "Render/HudTextWidgets.h"
#include "Render/HudWidgets.h"
#include "Render/SpriteBatch.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace EditorNiveles {

SalidaDelHub pantallaDeProyectos(Window& window, EditorOptions& opts,
                                 bool& cerrarDelTodo, Shader& spriteShader,
                                 int width, int height) {
        auto hr = Editor::ProjectHub::load("assets");
        if (!hr.isOk()) {
            std::cerr << "No se pudo leer assets/proyectos: " << hr.errorMessage() << "\n";
            return SalidaDelHub::Error;
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

        // Teclas que producen un caracter, para los campos de texto. Se
        // recorren A-Z y 0-9 en vez de usar el callback de texto de
        // GLFW porque Window ya gestiona sus propios callbacks y
        // enchufar otro desde aqui es meterse en su terreno.
        //
        // Consecuencia, dicha claro: por aqui no se pueden teclear acentos
        // ni ñ. ProjectIndex::suggestId los pliega bien, pero para
        // escribirlos haria falta el callback de texto. Se deja registrado
        // como cabo suelto de H4-01 en vez de enchufar a medias un segundo
        // callback al lado del que ya gestiona Window.
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
            // El espacio lo necesita el campo de duplicar, que escribe un
            // nombre visible («Copia norte») y deja que suggestId lo pliegue
            // a id. El campo de crear lo descarta el solo: sigue escribiendo
            // un id [a-z0-9_].
            if (menuKeys.pressed(win, GLFW_KEY_SPACE)) {
                return ' ';
            }
            return 0;
        };

        while (!window.shouldClose() && !salir) {
            window.pollEvents();

            const Editor::ProjectHub::Mode modo = hub.mode();
            const bool tecleando =
                modo == Editor::ProjectHub::Mode::NewProject ||
                modo == Editor::ProjectHub::Mode::DuplicateProject;
            if (tecleando) {
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
                if (menuKeys.pressed(win, GLFW_KEY_D))      act = hub.key('d');
                if (menuKeys.pressed(win, GLFW_KEY_X))      act = hub.key('x');
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
                  "W/S mover   ENTER abrir   N nuevo   D duplicar   X borrar   B compilar   ESC salir",
                  kApagado);

            // Modales encima de todo
            if (modo == Editor::ProjectHub::Mode::NewProject) {
                panel(HudAnchor::Center, {0.0f, 0.0f}, {720.0f, 130.0f});
                texto(HudAnchor::Center, {-330.0f, -46.0f}, "ID DEL PROYECTO NUEVO", kBlanco);
                // El cursor no parpadea: en una pantalla que se pinta
                // entera cada frame, un '_' fijo se lee igual de bien y
                // no hay que llevar el tiempo hasta aqui.
                texto(HudAnchor::Center, {-330.0f, -8.0f}, "> " + hub.draftId() + "_", kBien);
                texto(HudAnchor::Center, {-330.0f, 34.0f},
                      "solo a-z 0-9 _    BACKSPACE borra    ENTER crea    ESC cancela", kApagado);
            } else if (modo == Editor::ProjectHub::Mode::DuplicateProject) {
                panel(HudAnchor::Center, {0.0f, 0.0f}, {760.0f, 130.0f});
                texto(HudAnchor::Center, {-350.0f, -46.0f}, "NOMBRE DE LA COPIA", kBlanco);
                texto(HudAnchor::Center, {-350.0f, -8.0f}, "> " + hub.draftId() + "_", kBien);
                texto(HudAnchor::Center, {-350.0f, 34.0f},
                      "el id sale del nombre    BACKSPACE borra    ENTER duplica    ESC cancela",
                      kApagado);
            } else if (modo == Editor::ProjectHub::Mode::ConfirmDelete) {
                const Editor::Project* marcado = hub.current();
                panel(HudAnchor::Center, {0.0f, 0.0f}, {760.0f, 150.0f});
                texto(HudAnchor::Center, {-350.0f, -56.0f}, "BORRAR PROYECTO", kAviso);
                texto(HudAnchor::Center, {-350.0f, -18.0f},
                      "> " + std::string(marcado != nullptr ? marcado->name : ""), kBlanco);
                texto(HudAnchor::Center, {-350.0f, 20.0f},
                      "ENTER borra el manifiesto    cualquier otra tecla cancela", kApagado);
                texto(HudAnchor::Center, {-350.0f, 44.0f},
                      "sus niveles, mapas y catalogos se quedan en disco", kApagado);
            } else if (modo == Editor::ProjectHub::Mode::Message) {
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
            menu.render(menuBatch, spriteShader, width, height);
            menuBatch.end();
            window.swapBuffers();
        }
        return (cerrarDelTodo || !salir) ? SalidaDelHub::Salir
                                         : SalidaDelHub::Abrir;

}

}  // namespace EditorNiveles
