// ESTETICA-01 — El brillo del equipo, en el motor de verdad (con ventana).
//
// Oriol (06/10/2026), con NosTale de referencia: el brillo sube con lo fuerte
// que es la pieza (Tier 1 = +1/+2, Tier 2 = +3/+4 ... Tier 5 = +9/+10). Este
// demo pone en fila a seis humanos con un arma de cada escalon y a uno con un
// juego de reliquias, cada uno con su HojaPersonaje (montada con el brillo) y
// su AuraJugador (que pinta el halo, los destellos y las particulas), sobre el
// IsometricRenderer, y vuelca algunos fotogramas a PPM para mirarlos.
//
//     demo_brillo_equipo [fotogramas]   (por defecto 96; vuelca uno de cada 3
//                                        de los ultimos 48 en
//                                        demo_brillo_equipo_NN.ppm)
//     demo_brillo_equipo 96 aguja_del_firmamento hoz_del_cuarto_menguante ...
//                                       (hasta siete objetos, uno por figura:
//                                        las unicas, por ejemplo)
//     demo_brillo_equipo 96 yelmo_del_guerrero+coraza_del_guerrero+...
//                                       (SP-02: varios objetos en una figura,
//                                        un set entero)
//
// DEMO_BRILLO_SIN_CAPA=1 quita la capa de la raza (de espaldas tapa el set).
//
// PART-01 (las particulas de los elementos y de las clases, tools/particulas.py):
//     demo_brillo_equipo 96 item_short_sword@dragones   (la pieza, con las particulas de
//                                       esa familia al nivel DEMO_BRILLO_NIVEL, 9 por defecto:
//                                       como un +9)
//     demo_brillo_equipo 96 @llamas+@escarcha:8ad8ff  (dos abalorios, uno de fuego y otro de
//                                       hielo; «:rrggbb», el color, si la familia no trae)
// DEMO_BRILLO_FONDO=0.45,0.5,0.38 cambia el fondo (lo oscuro que tapa se ve mejor en claro).
//     demo_brillo_equipo 96 torso=sp_abrigo_cazador@sombra:7a4ab4#6+arma=f_revolver_raro@animas#6
//                                      («ranura=pieza»: una pieza del atlas sin objeto del catalogo,
//                                       para probar piezas nuevas; «#nivel», el nivel de su brillo)
// PART-02b (los abalorios echan desde su sitio):
//     demo_brillo_equipo 96 item_short_sword+mano@llamas,escarcha:8ad8ff#6
//                                      («sitio@familia»: un abalorio en la mano del arma, «mano»; en la
//                                       otra, «otraMano»; en el pecho, «pecho», o en la cintura,
//                                       «cintura»; «familia,otra»: dos efectos, a medias: el anillo de
//                                       fuego y hielo de Oriol)
//     demo_brillo_equipo 96 item_short_sword+anillo_de_vitalidad+amuleto_de_foco
//                                      (los del catalogo echan desde su sitio: el anillo, de la mano;
//                                       el amuleto, del pecho)
//
// PART-02d (las cartas al jugarse en el combate): DEMO_BRILLO_CARTA=dragones,llamas_puras,petalos lanza
// cada 1,6 s una carta sobre cada figura (sus particulas brotan del cuerpo y se apagan, con
// Render::EfectosDeCartas), una familia por figura que va rotando.
//
// Para grabar de cerca (un GIF): DEMO_BRILLO_ZOOM=4 (zoom de la camara; con 4
// caben cuatro figuras) y DEMO_BRILLO_VOLCADO=2,96 (uno de cada 2 de los
// ultimos 96 fotogramas; por defecto 3,48).
//
// MORAL-02: DEMO_BRILLO_MORAL=78,84,92,100 pone a cada figura su moral (el aura: halo, rayos, alas y
// sus chispas), de izquierda a derecha; con ella, las sombras blob (el halo y las chispas las usan).
//
// Quietas, las figuras estan en reposo (el arma como se lleva parado, y
// respirando). Con DEMO_BRILLO_PASEO=1 cada una anda sin moverse del sitio y
// descansa, girando: abajo, derecha, arriba e izquierda (el ciclo de andar y
// el reposo de cada rumbo, ESTETICA-01).
#include "Core/Check.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Engine/InputState.h"
#include "Engine/Window.h"
#include "Game/Personajes/BrilloEquipo.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Game/Personajes/Vestuario.h"
#include "Render/AuraJugador.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/CapturaGL.h"
#include "Render/EfectosDeCartas.h"
#include "Render/HojaPersonaje.h"
#include "Render/IsometricRenderer.h"
#include "Render/Player.h"
#include "Render/TileMap.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <glad/glad.h>

namespace {

Texture texturaBlanca() {
    unsigned char blanco[4] = {255, 255, 255, 255};
    unsigned int id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blanco);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture(id, 1, 1);
}

struct Figura {
    std::unique_ptr<Player> jugador;
    std::unique_ptr<HojaPersonaje> hoja;
    std::unique_ptr<AuraJugador> aura;
};

// PART-01: «pieza@familia[:rrggbb]» (o «@familia» para un abalorio). PART-02b: «sitio@familia[,otra]»,
// un abalorio en su sitio, con uno o dos efectos.
struct ConParticulas {
    std::string pieza, familia, otraFamilia;
    std::array<std::uint8_t, 3> color{255, 240, 200}, otroColor{255, 240, 200};
    int nivel = 0;  // 0: el de DEMO_BRILLO_NIVEL
};

// «familia[:rrggbb]»: la familia y su color (el de la familia si trae y no se dice otro).
void leerFamilia(std::string texto, std::string& familia, std::array<std::uint8_t, 3>& color) {
    const auto dos = texto.find(':');
    const Game::TipoDeParticula tipo = Game::tipoDeParticula(texto.substr(0, dos));
    if (!tipo.colores.empty())
        color = tipo.colores.front();
    if (dos != std::string::npos) {
        const unsigned long rgb = std::strtoul(texto.c_str() + dos + 1, nullptr, 16);
        color = {static_cast<std::uint8_t>(rgb >> 16), static_cast<std::uint8_t>(rgb >> 8),
                 static_cast<std::uint8_t>(rgb)};
        texto.resize(dos);
    }
    familia = texto;
}

ConParticulas leerParticulas(const std::string& token) {
    ConParticulas p;
    const auto arroba = token.find('@');
    p.pieza = token.substr(0, arroba);
    std::string familias = token.substr(arroba + 1);
    if (const auto almohadilla = familias.find('#'); almohadilla != std::string::npos) {
        p.nivel = std::atoi(familias.c_str() + almohadilla + 1);
        familias.resize(almohadilla);
    }
    const auto coma = familias.find(',');
    leerFamilia(familias.substr(0, coma), p.familia, p.color);
    if (coma != std::string::npos)
        leerFamilia(familias.substr(coma + 1), p.otraFamilia, p.otroColor);
    return p;
}

bool esSitio(const std::string& s) { return s == "mano" || s == "otraMano" || s == "pecho" || s == "cintura"; }

}  // namespace

int main(int argc, char** argv) {
    int ancho = 1152, alto = 360;
    // MORAL-02: DEMO_BRILLO_TAM=1600x640 agranda la ventana (las alas abiertas no caben en la fila de siempre)
    if (const char* t = std::getenv("DEMO_BRILLO_TAM")) std::sscanf(t, "%dx%d", &ancho, &alto);
    const int fotogramas = argc > 1 ? std::atoi(argv[1]) : 96;
    try {
        Window ventana(ancho, alto, "Motor Grafico - brillo del equipo (ESTETICA-01)");
        ShaderManager shaders;
        auto sprite = shaders.load("sprite", "assets/shaders/sprite");
        require(sprite.isOk());
        TextureManager texturas;
        auto checker = texturas.load("checker", "assets/textures/test_checker.png");
        require(checker.isOk());
        TextureAtlas atlasMapa(checker.value(), 8, 8);
        atlasMapa.defineRegion(1, 0, 0);
        atlasMapa.defineRegion(2, 0, 0);
        TileMap mapa;
        require(mapa.loadFromFile("assets/maps/test_map.tmx").isOk());
        Camera camara(ancho, alto);
        IsometricRenderer renderer(&camara, &mapa, &atlasMapa, sprite.value());
        Texture blanco = texturaBlanca();

        auto montaje = Game::MontajePersonajes::cargar("assets/textures");
        if (!montaje.isOk()) {
            std::cerr << montaje.errorMessage() << "\n";
            return 1;
        }
        const Game::ManifiestoPersonajes& man = montaje.value()->manifiesto();
        Game::Apariencia humano = Game::Apariencia::deRaza(*man.raza("race_human_marches"));
        // SP-02: DEMO_BRILLO_SIN_CAPA=1 quita la capa de la raza (de espaldas tapa el set).
        if (std::getenv("DEMO_BRILLO_SIN_CAPA") != nullptr) humano.ropa.erase("capa");

        // De izquierda a derecha: +1, +2, +4, +5, +7, +8 y un juego de reliquias (Tier 3).
        std::vector<std::vector<std::string>> filas = {
            {"daga_de_milicia"},        {"daga_de_plata"},           {"daga_de_plaga"},
            {"sable_ligero_de_estrella"}, {"sable_ligero_de_vacio"}, {"aguja_del_firmamento"},
            {"yelmo_del_amanecer", "melena_dorada", "botas_del_primero_en_entrar", "espada_del_primer_rayo"},
        };
        // O los objetos que se digan, uno por figura (caben siete en la ventana);
        // SP-02: «a+b+c» viste a una figura con varios (un set entero).
        if (argc > 2) {
            filas.clear();
            for (int i = 2; i < argc && filas.size() < 7; ++i) {
                std::vector<std::string> piezas;
                std::string pieza;
                for (const char* c = argv[i]; *c != '\0'; ++c) {
                    if (*c == '+') {
                        if (!pieza.empty()) piezas.push_back(pieza);
                        pieza.clear();
                    } else {
                        pieza += *c;
                    }
                }
                if (!pieza.empty()) piezas.push_back(pieza);
                if (piezas.size() > 7) piezas.resize(7);
                filas.push_back(piezas);
            }
        }
        std::vector<Figura> figuras;
        // MORAL-02: la moral de cada figura
        std::vector<int> morales;
        if (const char* m = std::getenv("DEMO_BRILLO_MORAL")) {
            std::string tok;
            for (const char c : std::string(m) + ",") {
                if (c == ',') {
                    if (!tok.empty()) morales.push_back(std::atoi(tok.c_str()));
                    tok.clear();
                } else {
                    tok += c;
                }
            }
        }
        std::unique_ptr<Texture> sombras;
        if (!morales.empty()) {
            sombras.reset(CreateBlobShadowTexture(64));
            renderer.setBlobShadows(sombras.get());
            renderer.setBlobShadowsEnabled(true);
        }
        // En fila en la pantalla (x - y crece, x + y fijo), lejos de las baldosas del mapa.
        const int n = static_cast<int>(filas.size());
        // MORAL-02: DEMO_BRILLO_HUECO=2 separa las figuras el doble (con alas grandes se tocan)
        const char* huecoEnv = std::getenv("DEMO_BRILLO_HUECO");
        const int hueco = huecoEnv != nullptr ? std::max(1, std::atoi(huecoEnv)) : 1;
        const int k = 2 * (n - 1) * hueco + 8;
        const char* nivelEnv = std::getenv("DEMO_BRILLO_NIVEL");
        const int nivel = nivelEnv != nullptr ? std::atoi(nivelEnv) : 9;
        for (int i = 0; i < n; ++i) {
            RPG::EquipoPuesto puestos{};
            std::vector<ConParticulas> conParticulas;
            std::map<std::string, std::string> piezasSueltas;  // «ranura=pieza»
            std::size_t j = 0;
            for (const std::string& token : filas[static_cast<std::size_t>(i)]) {
                if (token.find('@') != std::string::npos)
                    conParticulas.push_back(leerParticulas(token));
                const std::string id = token.substr(0, token.find('@'));
                if (const auto igual = id.find('='); igual != std::string::npos) {
                    piezasSueltas[id.substr(0, igual)] = id.substr(igual + 1);
                    continue;
                }
                if (esSitio(id))
                    continue;  // PART-02b: un abalorio en su sitio, sin objeto
                // PART-02b: los abalorios del catalogo, en su ranura (el anillo en la mano del arma, el
                // amuleto en el cuello); un segundo anillo, en la otra mano
                if (const auto ab = man.brilloDeAbalorio.find(id); ab != man.brilloDeAbalorio.end()) {
                    const auto ranura = ab->second.sitio == "pecho"     ? RPG::EquipSlot::Neck
                                        : ab->second.sitio == "cintura" ? RPG::EquipSlot::Belt
                                                                        : RPG::EquipSlot::Accessory;
                    auto& sitio = puestos[static_cast<std::size_t>(ranura)];
                    if (sitio.empty())
                        sitio = id;
                    else if (ranura == RPG::EquipSlot::Accessory)
                        puestos[static_cast<std::size_t>(RPG::EquipSlot::Ring2)] = id;
                    continue;
                }
                if (!id.empty() && j < puestos.size())
                    puestos[j++] = id;
            }
            auto brillos = Game::Vestuario::brillosVisibles(puestos, man);
            auto equipo = Game::Vestuario::piezasVisibles(puestos, man);
            for (const auto& [ranura, pieza] : piezasSueltas)
                equipo[ranura] = pieza;
            for (std::size_t q = 0; q < conParticulas.size(); ++q) {
                const ConParticulas& p = conParticulas[q];
                Game::BrilloDePieza b;
                b.nivel = p.nivel > 0 ? p.nivel : nivel;
                b.color = p.color;
                b.particulas = p.familia;
                const auto pieza = man.piezaDeObjeto.find(p.pieza);
                if (const auto igual = p.pieza.find('='); igual != std::string::npos) {
                    brillos[p.pieza.substr(0, igual)] = b;  // la ranura de la pieza suelta
                } else if (pieza != man.piezaDeObjeto.end()) {
                    brillos[pieza->second.ranura] = b;
                } else {
                    b.abalorio = true;
                    // PART-02b: en su sitio, con su otro efecto
                    if (esSitio(p.pieza))
                        b.sitio = p.pieza;
                    b.otrasParticulas = p.otraFamilia;
                    b.otroColor = p.otroColor;
                    brillos["abalorio:" + (b.sitio.empty() ? std::string() : b.sitio + ":") + p.familia +
                            std::to_string(q)] = b;
                }
            }
            Figura f;
            f.jugador = std::make_unique<Player>(GridCoord{4 + i * hueco, k - 4 - i * hueco}, &atlasMapa);
            f.hoja = std::make_unique<HojaPersonaje>();
            auto vestido = f.hoja->vestir(*montaje.value(), humano, equipo, brillos);
            if (!vestido.isOk()) {
                std::cerr << vestido.errorMessage() << "\n";
                return 1;
            }
            f.hoja->ponerA(*f.jugador, 64, 64, Vector2{0.0f, 32.0f - 64.0f});
            f.aura = std::make_unique<AuraJugador>(f.jugador.get(), 64, 32);
            f.aura->ponBrillo(f.hoja.get(), &blanco);
            if (static_cast<std::size_t>(i) < morales.size()) {
                f.aura->ponMoral(morales[static_cast<std::size_t>(i)]);
                f.jugador->setTint(f.aura->aspecto().tinteSprite);
            }
            renderer.addToQueue(f.jugador.get());
            f.aura->encolar(renderer);  // MORAL-02: con su pieza de detras (las alas)
            std::cout << "[brillo] " << filas[static_cast<std::size_t>(i)].front() << ": "
                      << f.hoja->brillo().fuentes.size() << " piezas que brillan\n";
            figuras.push_back(std::move(f));
        }
        // La camara en el centro de la fila.
        const char* zoomEnv = std::getenv("DEMO_BRILLO_ZOOM");
        const float zoom = zoomEnv != nullptr ? static_cast<float>(std::atof(zoomEnv)) : 2.2f;
        const float centroX = static_cast<float>((4 + (n - 1) * hueco / 2.0f) * 2 - k) * 32.0f + 32.0f;
        camara.transitionTo(Vector2{centroX, static_cast<float>(k) * 16.0f + 4.0f}, zoom > 0.0f ? zoom : 2.2f,
                            0.0f);
        int cada = 3, ultimos = 48;
        if (const char* volcado = std::getenv("DEMO_BRILLO_VOLCADO")) {
            std::sscanf(volcado, "%d,%d", &cada, &ultimos);
            if (cada < 1) cada = 1;
            if (ultimos < 1) ultimos = 1;
        }

        float fondo[3] = {0.20f, 0.23f, 0.19f};
        if (const char* f = std::getenv("DEMO_BRILLO_FONDO"))
            std::sscanf(f, "%f,%f,%f", &fondo[0], &fondo[1], &fondo[2]);
        const bool paseo = std::getenv("DEMO_BRILLO_PASEO") != nullptr;
        // PART-02d: las cartas jugadas sobre las figuras
        std::vector<std::string> cartas;
        if (const char* c = std::getenv("DEMO_BRILLO_CARTA")) {
            std::string f;
            for (const char* q = c;; ++q) {
                if (*q == ',' || *q == '\0') {
                    if (!f.empty()) cartas.push_back(f);
                    f.clear();
                    if (*q == '\0') break;
                } else {
                    f += *q;
                }
            }
        }
        Render::EfectosDeCartas efectos;
        if (!cartas.empty()) {
            Game::ParticulasDeCartas mapa;
            for (const std::string& f : cartas) mapa.pon("demo_" + f, f);
            efectos.ponMapa(mapa);
            efectos.ponBlanco(&blanco);
            renderer.addToQueue(&efectos);
        }
        static const char* const kAndar[4] = {"walk_down", "walk_right", "walk_up", "walk_left"};
        static const char* const kReposo[4] = {"idle_down", "idle_right", "idle_up", "idle_left"};
        constexpr int kPorRumbo = 160;  // 70 fotogramas andando y 90 en reposo
        InputState nada;
        for (int frame = 0; frame < fotogramas && !ventana.shouldClose(); ++frame) {
            ventana.pollEvents();
            camara.update(1.0f / 60.0f);
            const int rumbo = (frame / kPorRumbo) % 4;
            const bool andando = frame % kPorRumbo < 70;
            for (Figura& f : figuras) {
                if (paseo) {
                    // Sin moverse de la baldosa: el ciclo se anima igual que andando.
                    f.jugador->play(andando ? kAndar[rumbo] : kReposo[rumbo]);
                    f.jugador->AnimatedEntity::update(1.0f / 60.0f);
                    continue;
                }
                f.jugador->handleInput(nada);
                f.jugador->update(1.0f / 60.0f);
            }
            if (!cartas.empty() && frame % 96 == 0) {
                for (std::size_t i = 0; i < figuras.size(); ++i) {
                    Vector2 esquina, tam;
                    int region = 0;
                    if (figuras[i].jugador->dibujoDePersonaje(esquina, tam, region))
                        efectos.lanza("demo_" + cartas[(i + static_cast<std::size_t>(frame / 96)) % cartas.size()],
                                      {}, esquina, tam);
                }
            }
            renderer.sortQueue();
            glClearColor(fondo[0], fondo[1], fondo[2], 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            require(renderer.renderFrame().isOk());
            const int desdeElFinal = fotogramas - 1 - frame;
            if (desdeElFinal < ultimos && desdeElFinal % cada == 0) {
                glFinish();
                char nombre[64];
                std::snprintf(nombre, sizeof nombre, "demo_brillo_equipo_%02d.ppm",
                              (ultimos - 1 - desdeElFinal) / cada);
                const auto volcado = Render::VuelcaFramebufferPPM(nombre, ancho, alto);
                if (!volcado.isOk()) std::cerr << volcado.errorMessage() << "\n";
            }
            ventana.swapBuffers();
            // El brillo va con el reloj de verdad: ~60 fotogramas por segundo.
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        std::cout << "demo_brillo_equipo: " << fotogramas << " fotogramas.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
