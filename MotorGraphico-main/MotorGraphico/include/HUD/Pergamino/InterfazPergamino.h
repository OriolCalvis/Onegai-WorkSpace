// PERGAMINO-01 — La interfaz de pergamino entera, sin OpenGL.
//
// Recibe cada fotograma el estado del juego que se ve (EstadoJuego: la
// sesion, el cuaderno, la pausa, la camara...) y lo convierte en tres
// lienzos: la capa baja (HUD de exploracion y dialogo), la capa alta
// (cuaderno y papiros, con su velo) y la hoja viva del cuaderno. Tambien
// guarda el poco estado propio de la interfaz: la pagina del dialogo, la
// vuelta de hoja, el aviso de nuevo objetivo y el resultado del ultimo
// guardado.
//
// Es la «presentacion ya resuelta» que pedia EST-20b para refreshHud: el
// orquestador (Application) solo rellena EstadoJuego; que se ve y donde lo
// decide esto, y se prueba sin ventana. El dibujo lo hace Render/PergaminoGL.
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "HUD/Iconos.h"
#include "HUD/Pergamino/AtlasRetratos.h"
#include "HUD/Pergamino/Cuaderno.h"
#include "HUD/Pergamino/DialogoHoja.h"
#include "HUD/Pergamino/EmocionVista.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/HudRetales.h"
#include "HUD/Pergamino/HojaNegocio.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/GuiaMapa.h"  // MAPA-02: y el minimapa
#include "Game/CofreKit.h"
#include "Game/CuadernoCreacion.h"
#include "HUD/Pergamino/PaginasJugador.h"

class GameSession;
class RuntimeTileMap;

namespace Game {
class BarraCombate;
struct VistaBarra;
class RegistroSprites;
class MontajePersonajes;
class InventoryScreen;
class TitleMenuState;
class PauseMenu;
class OptionsState;
class GameOverScreen;
struct DiarioDatos;
struct CodiceDatos;
class TiendaEstado;
class NegocioEstado;
}  // namespace Game

namespace Pergamino {

struct EstadoJuego {
    int anchoVista = 1280;
    int altoVista = 720;
    bool oculta = false;  // titulo o creacion de personaje: el tema no se ve
    bool sinAviso = false;  // PROLOGO-02: en una cinematica la E no hace nada (sin «[E] Hablar»)
    // AV14-02 — Te han pegado: tu hoja tenida y temblando (0..1, y su temblor en pixeles) y la
    // vida que se va (la barra fantasma; < 0, ninguna).
    float golpeHoja = 0.0f, pvFantasma = -1.0f;
    float temblorHojaX = 0.0f, temblorHojaY = 0.0f;
    // AV13-33 — La escena de noqueo antes de «Has caído»: lo oscuro (< 0, no hay escena) y la frase.
    float noqueoVelo = -1.0f, noqueoAlfa = 0.0f;
    std::string noqueoLinea;
    GameSession* sesion = nullptr;
    Game::InventoryScreen* cuaderno = nullptr;
    bool cuadernoAbierto = false;
    const Game::PauseMenu* pausa = nullptr;
    const Game::OptionsState* opciones = nullptr;
    const Game::GameOverScreen* derrota = nullptr;
    const Game::DiarioDatos* diario = nullptr;
    const Game::CodiceDatos* codice = nullptr;
    int cicatrices = 0;  // AV15-08: las del personaje, para la fila de la ficha (0 = ninguna)
    const RuntimeTileMap* mapa = nullptr;
    // Camara del mundo: para poner el aviso de interaccion sobre la cabeza
    // de quien se habla (pantalla = (mundo - camara) * zoom + vista / 2).
    float camaraX = 0.0f, camaraY = 0.0f, zoom = 1.0f;
    bool animacionesReducidas = false;
    // UI-BG3-01 — El titulo del juego: su nombre sobre la ciudad y el menu
    // en un papiro (maqueta C01). Con titulo no se pinta nada mas.
    const Game::TitleMenuState* titulo = nullptr;
    // UI-BG3-01 — La creacion de personaje (maquetas C02): el cuaderno de
    // siete hojas y quien monta los retratos. Con creacion no se pinta nada mas.
    const Game::CuadernoCreacion* creacion = nullptr;
    Game::MontajePersonajes* montaje = nullptr;
    // UI-BG3-01 — El papiro del cofre de El Vacio recien abierto (C04b).
    const Game::CofreKit::Papiro* cofre = nullptr;
    // UI-BG3-01 — El combate (maquetas C03): lo que hay en la mesa, el estado
    // de la barra (foco, emergente, registro, turno del rival), el aspecto
    // del heroe para su retrato y las hojas de sprites de los enemigos. Sin
    // vista o sin barra, el combate solo ensena el retal del heroe.
    const Game::VistaBarra* vistaBarra = nullptr;
    const Game::BarraCombate* barra = nullptr;
    const Game::CuadernoCreacion::Vista* retratoHeroe = nullptr;
    const Game::RegistroSprites* sprites = nullptr;
    // H3-06 — Los temporizadores declarados por la aventura activa (los que
    // corren estan en el NarrativeState de la sesion).
    const std::vector<RPG::TimerDefinition>* timers = nullptr;
    // TIENDA-02 — La tienda abierta: su pestana, su foco y su ultimo resultado.
    const Game::TiendaEstado* tienda = nullptr;
    // AV15-05a — El negocio abierto: sus filas, su foco y su pregunta (comprar, revender).
    const Game::NegocioEstado* negocio = nullptr;
    // C2-04d — Las emociones de las cinematicas sobre su actor (lo rellena refreshHud).
    const std::vector<EmocionVista>* emociones = nullptr;
};

// Lo que haria la tecla E ahora mismo (la misma busqueda que
// GameSession::interact: la puerta que se pisa y luego la propia celda y
// las cuatro de al lado). Vacio si no hay nada.
struct AvisoInteraccion {
    std::string verbo, destino;
    int celdaX = 0, celdaY = 0;
    bool enLaCelda = false;  // la puerta que se pisa: sin rombo
};
AvisoInteraccion avisoInteraccion(const GameSession& sesion);
// AV15-02 (CartelesPuertas.cpp) — Sobre cada puerta cercana que lleva a un sitio con nombre, su cartel:
// el emblema, el nombre y la funcion. No sobre la casilla `salvo` (la que ya dice el aviso «Entrar en …»; D0037).
void pintaCarteles(Lienzo& l, const Pincel& p, const EstadoJuego& e, std::optional<GridCoord> salvo);
// PNJ-02 (BocadillosPnj.cpp) — Sobre los PNJ: el icono de su oficio mientras lo hacen y sus frases (a 4 casillas o menos).
void pintaBocadillosPnj(Lienzo& l, const Pincel& p, const EstadoJuego& e, std::optional<GridCoord> salvo);
// C2-04d (BocadillosEmocion.cpp) — Las emociones de las cinematicas: la misma bafarada, con alfa de
// entrada/salida; el texto literal ("!", "...") si el icono no esta en el atlas.
void pintaEmociones(Lienzo& l, const Pincel& p, const EstadoJuego& e);

// La baldosa de quien atiende a la E (P03a): un rombo carmin en el suelo,
// bajo los sprites y centrado en sus pies (los sprites pisan en el 90 % del
// alto de su baldosa, no en su centro). Aqui solo se decide donde; lo dibuja
// Render/PergaminoGL en el pase de sombras del mundo. Sigue marcado mientras
// dura el dialogo con ese mismo personaje (P03b).
struct MarcaSuelo {
    bool visible = false;
    int celdaX = 0, celdaY = 0;
    float anchoBaldosa = 64.0f, altoBaldosa = 32.0f;
    float pies = 0.9f;    // centro del rombo, en fraccion del alto de la baldosa
    std::string destino;  // a quien senala (su nombre, como hablante)
};

// AV16-06 — La marca de suelo de una SALIDA: una flecha de pergamino sobre su casilla, hacia fuera
// (la dibuja PergaminoGL junto al rombo de la marca, bajo los sprites).
struct MarcaSalida {
    int celdaX = 0, celdaY = 0;
    float anchoBaldosa = 64.0f, altoBaldosa = 32.0f;
    float pies = 0.9f;
};
// CartelesPuertas.cpp — Las salidas a 12 casillas o menos del jugador.
std::vector<MarcaSalida> salidasCercanas(const EstadoJuego& e);

class InterfazPergamino {
public:
    // Letras (assets/textures/letras_pergamino.json), iconos y los nombres de
    // la historia del personaje. Sin letras no hay tema: el llamador sigue
    // con el HUD de siempre.
    Result<bool> carga(const std::string& carpetaAssets);
    bool cargada() const { return m_letras.cargada(); }

    // Nombre de la zona (el del nivel) y clave para rehacer el minimapa.
    void ponZona(const std::string& nombre, const std::string& claveNivel);
    const std::string& zona() const { return m_zona; }
    // Resultado del ultimo guardado o carga desde la pausa (E17). Se borra
    // al salir de la pausa o al entrar y salir de las opciones. UI-BG3-01:
    // en el titulo es la linea de estado de su papiro («Creacion cancelada»).
    void avisaPausa(bool ok, const std::string& texto, const std::string& detalle);

    void avanza(float segundos);
    void actualiza(const EstadoJuego& e);
    // ENTER en un dialogo: true si habia otra pagina (y se ha pasado).
    bool pasaPaginaDialogo() { return m_dialogo.avanza(); }
    // OPCIONES-01 — La hoja del dialogo, para elegir (Engine/TeclasDialogo.h).
    DialogoHoja& dialogo() { return m_dialogo; }

    const Lienzo& capaBaja() const { return m_capas.baja; }
    const Lienzo& capaAlta() const { return m_capas.alta; }
    const Lienzo& hojaViva() const { return m_capas.hoja; }
    bool hojaVisible() const { return m_capas.hojaVisible; }
    int vueltasDeHoja() const { return m_cuaderno.vueltas(); }
    Rect rectHoja() const { return m_capas.rectHoja; }
    const MinimapaTinta& minimapa() const { return m_guia.minimapa(); }
    // MAPA-02 — La mision seguida, la flecha, el mapa grande (M) y sus chinchetas.
    GuiaMapa& guia() { return m_guia; }
    const GuiaMapa& guia() const { return m_guia; }
    const MarcaSuelo& marcaSuelo() const { return m_marca; }
    const std::vector<MarcaSalida>& salidas() const { return m_salidas; }  // AV16-06
    const FuenteAtlas& letras() const { return m_letras; }
    const Hud::Iconos& iconos() const { return m_iconos; }
    // UI-BG3-01 — Los retratos (los sube PergaminoGL cuando cambian).
    const AtlasRetratos& retratos() const { return m_retratos; }
    // Lo que hay bajo el raton en lo que se ve ahora (la capa alta tapa la
    // baja). nullptr si nada responde ahi.
    const Zona* zonaEn(float x, float y) const;

private:
    struct Capas {
        Lienzo baja, alta, hoja;
        bool hojaVisible = false;
        Rect rectHoja;
    };
    struct AvisoPausa {
        std::string texto, detalle;
        bool error = false;
    };
    void pintaExploracion(const EstadoJuego& e, bool compacto);
    void pintaCuaderno(const EstadoJuego& e);
    void pintaPausa(const EstadoJuego& e);
    void pintaDerrota(const EstadoJuego& e);
    void pintaTitulo(const EstadoJuego& e);
    void pintaCreacion(const EstadoJuego& e);
    void pintaCofre(const EstadoJuego& e);
    void pintaCombate(const EstadoJuego& e);
    void pintaTiendaAbierta(const EstadoJuego& e);

    FuenteAtlas m_letras;
    Hud::Iconos m_iconos;
    NombresHistoria m_nombres;
    HudRetales m_retales;
    DialogoHoja m_dialogo;
    Cuaderno m_cuaderno;
    GuiaMapa m_guia;  // MAPA-02: el minimapa, la flecha y el mapa grande
    std::string m_zona;
    std::string m_claveNivel;
    AvisoPausa m_avisoPausa;
    MarcaSuelo m_marca;
    std::vector<MarcaSalida> m_salidas;  // AV16-06
    Capas m_capas;
    AtlasRetratos m_retratos;
    bool m_cuadernoAbierto = false;
    bool m_opcionesAbiertas = false;  // el aviso es de la pagina donde salio
};

}  // namespace Pergamino
