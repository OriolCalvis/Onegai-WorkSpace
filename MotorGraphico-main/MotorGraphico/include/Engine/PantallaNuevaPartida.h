// NUEVA-PARTIDA-01 — La creacion del personaje al empezar una partida nueva,
// como pantalla del juego. Guarda la ficha resultante, que Application
// vincula al Player.
//
// Un solo colaborador en Application.h: esta clase posee las reglas
// (CharacterCreation + TierRules + el kit del cofre), el cuaderno de
// creacion y la ficha viva.
//
// UI-BG3-01 — Oriol (05/10): de 14 pasos en lista a 7 hojas de cuaderno con
// rejillas, con raton o teclado (maquetas C02). El estado vive en
// Game::CuadernoCreacion (GL-free) y lo pinta el pergamino
// (Pergamino::pintaCreacion); aqui solo se traduce la entrada del fotograma
// (teclas, raton sobre las zonas que dejo la interfaz, rueda y texto) y se
// cierra la creacion. El personaje sale SIN equipo: el kit de su clase lo da
// el cofre de El Vacio.
//
// APARIENCIA-01 — Tambien guarda el aspecto vivo del jugador: vestir() se lo
// pone y actualizarAspecto() lo rehace cuando cambia lo que lleva equipado.
#pragma once

#include <map>
#include <memory>
#include <string>

#include "Core/Errors/Result.h"
#include "Engine/Input/InputSystem.h"
#include "Game/CuadernoCreacion.h"
#include "Game/Personajes/KitBasico.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Game/SessionData.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/TierRules.h"
#include "Render/HojaPersonaje.h"

class Player;
struct SaveGameData;

namespace Game {
class CatalogoCartas;
}
namespace Pergamino {
class InterfazPergamino;
}

namespace Engine {

class PantallaNuevaPartida {
public:
    enum class Estado { Cerrada, Creando, Terminada, Cancelada };

    // `equipo` es el catalogo de Application (no propietario): sin el, la
    // creacion no ensena el kit del cofre.
    explicit PantallaNuevaPartida(const RPG::Catalogs::EquipmentCatalog* equipo = nullptr);

    // Carga catalogos y reglas (una vez) y abre un cuaderno nuevo. `cartas`
    // (opcional) deja ver la mano de cada clase en su ficha.
    Result<bool> abrir(const Game::CatalogoCartas* cartas = nullptr);
    bool activa() const { return m_estado == Estado::Creando; }
    Estado estado() const { return m_estado; }

    // Consume el input del frame mientras esta activa. `ui` da lo que hay
    // bajo el raton (las zonas del ultimo fotograma pintado).
    void procesar(const Engine::Input::InputSnapshot& input, const Pergamino::InterfazPergamino* ui);
    void cerrar();

    // Lo que pinta el pergamino mientras se crea (nullptr si no).
    const Game::CuadernoCreacion* cuaderno() const { return activa() ? m_cuaderno.get() : nullptr; }
    Game::MontajePersonajes* montaje() { return m_datos.montaje.get(); }

    // Tras Terminada: la ficha construida y recalculada, una sola vez (deja
    // la pantalla Cerrada). El puntero es estable mientras viva esta
    // pantalla: Application se lo da al Player.
    RPG::CharacterSheet* tomarFicha();
    // Tras Cancelada: true una sola vez (deja la pantalla Cerrada).
    bool consumirCancelacion();
    const std::string& error() const { return m_error; }

    // APARIENCIA-01 — Pone al jugador el aspecto del personaje creado (su
    // hoja y los ciclos de andar). Sin personaje creado no hace nada.
    void vestir(Player& jugador) const;
    // E-GUION-01 — Para el jugador que no ha pasado por el creador (`juego --nivel`,
    // `--guion`): le pone el aspecto por defecto de un humano (race_human_marches:
    // su pelo y su ropa basica, sin armas) por el mismo camino que el creador, en
    // lugar del marcador provisional personaje.png. false (y no toca nada) si no
    // hay personajes.json o no se pudo montar la hoja. No crea ficha.
    bool vestirPorDefecto(Player& jugador);
    // Cada fotograma de partida: si cambio lo que lleva puesto la ficha,
    // vuelve a montar la hoja del jugador (y el equipo nuevo se ve, con su
    // brillo: ESTETICA-01).
    void actualizarAspecto();
    // ESTETICA-01 — La hoja que lleva el jugador, con el brillo de su equipo
    // (la dibuja AuraJugador). nullptr si no hay personaje creado.
    const HojaPersonaje* hojaDelJugador() const { return m_jugador && m_jugador->lista() ? m_jugador.get() : nullptr; }
    // UI-BG3-01 — Su aspecto con lo que lleva puesto, para el retrato del
    // orden de turno en combate. Vacia si no hay personaje creado.
    Game::CuadernoCreacion::Vista vistaDelJugador() const;

    // UI-BG3-01 — El kit del cofre de El Vacio para una clase (las mismas
    // reglas que ensena la creacion) y el nombre de la clase. Cargan las
    // reglas si aun no estan (una partida cargada no ha pasado por aqui).
    const Game::KitBasico::Kit& kitDe(const std::string& claseId);
    std::string nombreDeClase(const std::string& claseId);
    // Las reglas de inventario de la partida para esta ficha: el equipo, su
    // clase y los tiers. Sin la clase, equipar el kit recalculaba la vida con
    // la base de 8 de una clase vacia (el bardo bajaba de 19 a 18).
    InventoryRules reglasDeInventario(const RPG::CharacterSheet* ficha);

    // GUARDAR-FICHA — El aspecto del heroe en el guardado que acaba de sacar
    // la sesion (que ya lleva lo elegido al crearlo y lo puesto, pero no su
    // aspecto: no lo conoce). Sin heroe vestido no hace nada.
    void apuntaAspecto(SaveGameData& data) const;
    // GUARDAR-FICHA — Rehace el heroe de un guardado: la ficha con lo elegido
    // al crearlo (el nombre, de characterName), lo puesto con sus bonos y su
    // aspecto. nullptr si el guardado no trae heroe o ya no se puede rehacer
    // (una clase que no existe): la partida carga como antes, sin ficha. El
    // puntero es el mismo que da tomarFicha().
    RPG::CharacterSheet* restaurarHeroe(const SaveGameData& data);

private:
    // Lo que hace falta para crear un personaje, cargado una vez.
    struct Datos {
        std::unique_ptr<RPG::CharacterCreation> reglas;
        RPG::TierRules tiers;
        const RPG::Catalogs::EquipmentCatalog* equipo = nullptr;
        std::unique_ptr<Game::MontajePersonajes> montaje;  // nullptr = sin personajes por capas
        Game::KitBasico::Reglas kit;
        std::map<std::string, Game::CuadernoCreacion::Rasgo> rasgos;
        std::map<std::string, Game::KitBasico::Kit> kits;  // por clase
    };

    Result<bool> cargaDatos();
    void terminar();

    Datos m_datos;
    std::unique_ptr<Game::CuadernoCreacion> m_cuaderno;
    Estado m_estado = Estado::Cerrada;
    RPG::CharacterSheet m_ficha;
    std::string m_error;
    std::unique_ptr<HojaPersonaje> m_jugador;  // la hoja que lleva el jugador
};

}  // namespace Engine
