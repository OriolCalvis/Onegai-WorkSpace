// PROLOGO-02 — Las cinemáticas, en el juego (la lógica: Game/Cinematica.h).
//
// Application no crece por esto más que en tres líneas (está en su techo):
// el estado vive en EstadoCinematicas y lo que hace, en ApplicationCinematica,
// amiga de Application. Cada fotograma, en exploración:
//   - si no hay ninguna en curso, mira si empieza una (nivel, zona y flags);
//   - la que está en curso avanza (y el jugador no se mueve por su cuenta);
//   - los espectros se tiñen y las apariciones se funden, poco a poco;
//   - el velo (el fundido a blanco o a negro) va en un panel del HUD que se
//     dibuja debajo del diálogo, así lo que se dice se lee encima.
#pragma once

#include <functional>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Core/Errors/Result.h"

#include "Core/Math/Vector2.h"
#include "Engine/ApplicationCamara.h"
#include "Core/Math/Vector4.h"
#include "Game/Cinematica.h"
#include "Game/EfectosDeGolpe.h"
#include "Game/EscenaViva.h"
#include "Render/FuegoNivel.h"
#include "Game/EscenaNoqueo.h"
#include "Game/PasosEnRuta.h"
#include "HUD/Pergamino/EmocionVista.h"
#include "World/EntityHandle.h"
#include "RPG/RandomEngine.h"

class Application;
struct SaveGameData;
class BattleState;
class Entity;
class HudPanel;
class ICombatant;
namespace Engine::Input { class InputSnapshot; }

struct EstadoCinematicas {
    Game::CatalogoCinematicas catalogo;
    Game::ReproductorCinematica reproductor;
    bool cargando = false;                        // el nivel que pidió la cinemática
    Vector4 tinteJugador{1.0f, 1.0f, 1.0f, 1.0f};  // el del espectro, ya suavizado
    const void* sesionVista = nullptr;            // otra: nivel nuevo, tintes de golpe
    float tituloRecorrido = 0.0f;                 // el paseo de la cámara del título (celdas)
    bool tituloCargando = false, tituloFallido = false;
    std::unique_ptr<HudPanel> velo;
    // E-DISPARADORES-1 — El ambito del nivel en curso (se resuelve al cambiar de nivel: JSON + tamano del mapa).
    std::string ambitoDeNivel;
    AmbitoNivel ambito = AmbitoNivel::Exterior;
    // COMBATE-01 — El combate a la vista (ApplicationCinematica::combate).
    const void* combateVisto = nullptr;   // el BattleState que ya tiene camara y semilla
    float zoomAntes = 1.0f;               // el de explorar, para volver
    Vector2 centroCombate;                // entre el heroe y sus rivales
    EstadoCamaraJuego camara;             // U4-01: seguimiento, destino y sacudida de la camara virtual
    std::unique_ptr<RPG::Xoroshiro128p> dados;  // los del combate en curso, con semilla nueva
    // AV13-12 — Andar en el combate: la ruta del heroe (un paso cada 0,16 s, con
    // Player::moveTo) y la de cada enemigo que se ha acercado (su dibujo la anda).
    std::vector<GridCoord> rutaHeroe;
    float relojHeroe = 0.0f;
    std::vector<std::pair<Engine::EntityHandle, Game::PasosEnRuta>> rutasEnemigos;
    GridCoord sueloSenalado{-1, -1};  // la casilla bajo el raton (para no rehacer la ruta cada fotograma)
    bool rivalesPorPegar = false;     // han echado a andar: pegan al llegar (rivalesPegan)
    // AV14-01/02 — Los golpes de la batalla ya ensenados, lo que hace cada uno en
    // el cuerpo y en tu hoja, y la vida que se va (la barra fantasma de tu hoja).
    int golpeVisto = 0;
    Game::EfectosDeGolpe efectos;
    std::map<std::pair<bool, std::size_t>, Vector2> basesGolpe;  // el sitio de cada cuerpo antes del retroceso
    float pvFantasma = -1.0f, esperaFantasma = 0.0f;
    // AV14-08 — A quien mira la camara (el que juega) y con que zoom pelea.
    std::string actorVisto;
    float zoomCombate = 1.7f;
    bool cicatrizPuesta = false;  // AV13-33: la de esta caida ya se ha apuntado
    // AV13-33 — La escena de noqueo o rescate (mientras dura, no hay papiro de «Has caído») y quien
    // te saca de la pelea: el primero de los tuyos en pie cuando la batalla se perdió.
    std::unique_ptr<Game::EscenaNoqueo> noqueo;
    std::string rescatador;
    Vector2 baseNoqueo;  // el sitio del dibujo del heroe antes de caer
    // C2-04b — La escena viva (posturas, fuego, emociones): ApplicationCinematica::escena.
    Game::EscenaViva escena;
    Render::FuegoNivel fuego;                 // C2-04c: las llamas de arde/apaga (cola del renderer)
    const void* escenaSesion = nullptr;       // la sesion y el nivel con que se rehizo
    std::string escenaNivel;
    std::set<std::string> tumbados;           // marcadores con tinte de postura (para devolverlos a blanco)
    Vector4 tinteJugadorPostura{0.0f, 0.0f, 0.0f, 0.0f};  // el ultimo que puso la postura al heroe
    bool jugadorTumbado = false;
    std::vector<Pergamino::EmocionVista> emocionesVista;  // C2-04d: las de `escena`, resueltas a px de mundo
    EstadoCinematicas();
    ~EstadoCinematicas();
};

struct ApplicationCinematica {
    // En init, antes de componer el HUD: el catálogo y el panel del velo.
    static void carga(Application& app);
    // Partida nueva: nada a medias.
    static void reinicia(Application& app);
    static bool enCurso(const Application& app);
    // En update, después del cambio de nivel pendiente.
    static void actualiza(Application& app, float dt);
    // E-DISPARADORES-1 (D0026; codigo en src/Engine/ApplicationCinematicaDisparador.h) — La que empieza ahora: con el
    // ambito del nivel y lo que tapa la vista (tiles con colision y objetos `tapaVista`). Y la misma con los
    // pasos de llevar al jugador (GameSession::rutaAndando, o un fundido si no hay camino) y la accion.
    static const Game::Cinematica* laQueEmpieza(Application& app, const Game::TieneFlag& tiene);
    static Game::Cinematica efectiva(Application& app, const Game::Cinematica& c);
    // E-DISPARADORES-2 E4 (D0026.3) — Lanza el encuentro `enemigo` de un disparador `combate` por la puerta del efecto
    // `startBattle` de los beats (GameSession::startNarrativeBattle).
    static void lanzaCombate(Application& app, const std::string& enemigo, const std::string& gana, const std::string& pierde);
    // En update, después de refreshHud (que pone el tinte de la moral).
    static void tintes(Application& app, float dt);
    // C2-04b — La escena viva, cada fotograma (tras tintes): avanza las emociones, la rehace si
    // cambia el nivel o la sesion, y pone las posturas (tumba/levanta) a los dibujos.
    // Codigo en ApplicationCinematicaEscena.cpp.
    static void escena(Application& app, float dt);
    // C2-04d — Las emociones en curso con su sitio (pies del actor, px de mundo) y su alfa; nullptr
    // si no hay. Para EstadoJuego::emociones en refreshHud.
    static const std::vector<Pergamino::EmocionVista>* emocionesVista(Application& app);
    // El color de fondo del nivel (los recuerdos, en una niebla clara).
    static Vector4 fondo(const Application& app);
    // Al empezar a pintar: limpia el fotograma con ese color (glClear).
    static void limpiaPantalla(const Application& app);
    // El título, cada fotograma (la escena del título no corre update): carga
    // la ciudad de fondo y la pasea; la gente, los pájaros y las mascotas, vivos.
    static void titulo(Application& app, float dt);
    // Al dejar el título por una partida nueva: si detrás estaba la ciudad del
    // fondo, la partida empieza de cero en el nivel inicial (vacío: no hace falta).
    static std::function<Result<bool>()> alSalirDelTitulo(Application& app);
    // COMBATE-01 (Oriol, 14/10: «no he visto partículas», «algún feedback visual
    // de que me peguen»): en update, antes de mover la cámara. Al empezar un
    // combate, sus dados con semilla nueva (antes todos empezaban igual) y la
    // cámara se acerca a los que pelean; cada golpe salta en números sobre quien
    // lo recibe, lo que no entra dice «Fallo» o «Empate», y si te pegan a ti la
    // pantalla tiembla y se enrojece un instante. Al acabar, la cámara vuelve.
    static void combate(Application& app, float dt);
    // U4-01: tras Camera::update y ApplicationCamara::compone, los numeros de golpe leen la vista ya sacudida.
    static void vistaGolpes(Application& app);
    // AV13-12 (Oriol, 14/10: «moverse por casillas en combate, como en un tactics»): en tu turno,
    // con el raton sobre el suelo, la ruta hasta esa casilla; con clic, el heroe la anda.
    static void sueloCombate(Application& app, const Engine::Input::InputSnapshot& input);
    // Tras el turno del rival: los enemigos del mundo, donde dice el tablero, andando su ruta.
    static void rivalesAndan(Application& app);
    // AV13-29 (ApplicationNivel.cpp) — El dibujo de cada acompanante en el tablero, y el de
    // los enemigos que aparecen a mitad de partida (emboscadas de la narrativa, oleadas).
    static void acompanantesALaVista(Application& app);
    // La segunda mitad del turno del rival, cuando han llegado: pegan, se cierra la ronda y, si
    // toca, el desenlace. false si la batalla ya no esta (no se puede seguir usando).
    static bool rivalesPegan(Application& app);
    // AV13-11 — Al cerrar un turno, el siguiente de la iniciativa: si es un enemigo, su rotulo
    // «TURNO DE ...» (y tras el, turnoRival); si eres tu, juegas. Sin iniciativa, todos los enemigos.
    static void siguienteTurno(Application& app);
    // Mientras los rivales andan o estan por pegar, tu no juegas.
    static bool turnoRivalEnMarcha(const Application& app);
    // AV14-02 — El critico para el mundo 70 ms: en update, antes de mover a nadie.
    // true mientras dura (el mundo no avanza ese fotograma).
    static bool pausaDeGolpe(Application& app, float segundos);
    // AV14-01/02 — En combate(): cada golpe nuevo de la batalla, una vez (su numero, su grado,
    // el parpadeo, el retroceso, tu hoja), y lo que dura de cada uno.
    static void golpesALaVista(Application& app, const BattleState& b, float dt);
    // AV14-08 — Quien juega ahora (a quien mira la camara): el enemigo en su turno, o tu.
    static std::string actorDe(const Application& app, const BattleState& b);
    // AV13-33 — Has caido: una cicatriz mas para tu personaje (saves/<proyecto>/cicatrices.txt, fuera de
    // las partidas, para que cargar no la borre) y la pantalla de «Has caído» lo dice.
    static void ponCicatriz(Application& app);
    // AV13-33 (ApplicationHud.cpp) — Al caer: la cicatriz y la escena de noqueo o rescate. Cada
    // fotograma de la derrota, la escena avanza y el heroe se hunde; noqueoTecla se come el ENTER
    // mientras dura (pasa la frase) y devuelve false cuando ya toca el menu de «Has caído».
    static void empiezaNoqueo(Application& app);
    static void noqueo(Application& app, float dt);
    static bool noqueoTecla(Application& app, bool confirma);
    // TITULO-01 (Oriol, 14/10: «el menú del inicio no da feedback visual de
    // nada»): antes de algo que tarda (abrir la creación, cargar la partida),
    // el aviso en la línea del papiro y un fotograma pintado con él: que se vea
    // que la elección ha llegado aunque lo siguiente tarde.
    static void avisoYa(Application& app, const std::string& texto);
    // H0-05 / E-GUION-02: cargar una partida guardada (ApplicationCinematica.cpp); la usan loadSlot y `juego --guardado`.
    static bool cargaPartida(Application& app, const SaveGameData& datos);
    // Lo alto de la cabeza de un combatiente, en el mundo (false: no tiene dibujo).
    static bool cabezaDe(const Application& app, const ICombatant* cuerpo, Vector2& cabeza);
    // Su dibujo en el mundo (el heroe, el marcador de un enemigo o el de un acompanante); nullptr si no tiene.
    static Entity* dibujoDe(const Application& app, const ICombatant* cuerpo);
};
