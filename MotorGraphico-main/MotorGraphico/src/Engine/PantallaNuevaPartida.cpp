// NUEVA-PARTIDA-01 — ver include/Engine/PantallaNuevaPartida.h.
#include "Engine/PantallaNuevaPartida.h"
#include "Game/SaveSlotState.h"

#include <iostream>
#include <utility>

#include "Engine/ApplicationInternal.h"
#include "Game/NewGameFlow.h"
#include "Game/Personajes/Vestuario.h"
#include "Game/SaveGame.h"
#include "HUD/Pergamino/InterfazPergamino.h"
#include "Render/Player.h"

namespace Engine {

namespace {
using Engine::Input::InputAction;
using Zona = Game::CuadernoCreacion::Zona;

// Las zonas del cuaderno son las de su enum; el resto del lienzo no le toca.
bool esZonaDeCreacion(int tipo) {
    return tipo >= static_cast<int>(Zona::Pestana) && tipo <= static_cast<int>(Zona::Continuar);
}
}  // namespace

PantallaNuevaPartida::PantallaNuevaPartida(const RPG::Catalogs::EquipmentCatalog* equipo)
    : m_jugador(std::make_unique<HojaPersonaje>()) {
    m_datos.equipo = equipo;
}

Result<bool> PantallaNuevaPartida::cargaDatos() {
    if (!m_datos.reglas) {
        auto reglas = RPG::CharacterCreation::load("assets/catalogs");
        if (!reglas.isOk()) {
            m_error = reglas.errorMessage();
            return Result<bool>::Error("Creacion de personaje: " + m_error);
        }
        m_datos.reglas = std::make_unique<RPG::CharacterCreation>(std::move(reglas.value()));
        // Sin fichero de tiers valen los valores por defecto de TierRules.
        (void)m_datos.tiers.loadFromFile("assets/rules/tier_rules.json");
        // Sin personajes.json la creacion sigue, sin aspecto ni retratos.
        auto montaje = Game::MontajePersonajes::cargar("assets/textures");
        if (montaje.isOk()) {
            m_datos.montaje = std::move(montaje.value());
        } else {
            std::cerr << "APARIENCIA-01: " << montaje.errorMessage() << "\n";
        }
        // UI-BG3-01 — Sin las reglas del kit valen las de por defecto.
        auto kit = Game::KitBasico::cargarReglas("assets/rules/kit_basico.json");
        if (kit.isOk()) m_datos.kit = kit.value();
        else std::cerr << "UI-BG3-01: " << kit.errorMessage() << " (reglas del kit por defecto)\n";
        m_datos.rasgos = Game::CuadernoCreacion::leeRasgos("assets/catalogs/races.json");
    }
    return Result<bool>::Ok(true);
}

const Game::KitBasico::Kit& PantallaNuevaPartida::kitDe(const std::string& claseId) {
    static const Game::KitBasico::Kit kVacio;
    const auto it = m_datos.kits.find(claseId);
    if (it != m_datos.kits.end()) return it->second;
    if (!cargaDatos().isOk() || m_datos.equipo == nullptr) return kVacio;
    const RPG::ClassDefinition* clase = m_datos.reglas->classes().find(claseId);
    if (clase == nullptr) return kVacio;
    return m_datos.kits
        .emplace(claseId, Game::KitBasico::deClase(*clase, *m_datos.equipo, m_datos.tiers, m_datos.kit,
                                                   m_datos.montaje ? &m_datos.montaje->manifiesto() : nullptr))
        .first->second;
}

InventoryRules PantallaNuevaPartida::reglasDeInventario(const RPG::CharacterSheet* ficha) {
    InventoryRules r;
    r.equipment = m_datos.equipo;
    if (ficha == nullptr || !cargaDatos().isOk()) return r;
    r.playerClass = m_datos.reglas->classes().find(ficha->classId);
    r.tiers = &m_datos.tiers;
    return r;
}

std::string PantallaNuevaPartida::nombreDeClase(const std::string& claseId) {
    if (!cargaDatos().isOk()) return claseId;
    const RPG::ClassDefinition* clase = m_datos.reglas->classes().find(claseId);
    return clase != nullptr ? clase->name : claseId;
}

Result<bool> PantallaNuevaPartida::abrir(const Game::CatalogoCartas* cartas) {
    const auto cargados = cargaDatos();
    if (!cargados.isOk()) return cargados;
    Game::CuadernoCreacion::Datos d;
    d.reglas = m_datos.reglas.get();
    d.tiers = &m_datos.tiers;
    d.personajes = m_datos.montaje ? &m_datos.montaje->manifiesto() : nullptr;
    d.equipo = m_datos.equipo;
    d.kit = &m_datos.kit;
    d.cartas = cartas;
    d.rasgos = &m_datos.rasgos;
    m_cuaderno = std::make_unique<Game::CuadernoCreacion>(d);
    m_estado = Estado::Creando;
    m_error.clear();
    return Result<bool>::Ok(true);
}

RPG::CharacterSheet* PantallaNuevaPartida::tomarFicha() {
    if (m_estado != Estado::Terminada) return nullptr;
    m_estado = Estado::Cerrada;
    m_cuaderno.reset();
    // Los atlas de la creacion (decenas de MB) no hacen falta en partida.
    if (m_datos.montaje) m_datos.montaje->vaciarCache();
    return &m_ficha;
}

bool PantallaNuevaPartida::consumirCancelacion() {
    if (m_estado != Estado::Cancelada) return false;
    m_estado = Estado::Cerrada;
    m_cuaderno.reset();
    if (m_datos.montaje) m_datos.montaje->vaciarCache();
    return true;
}

void PantallaNuevaPartida::cerrar() {
    if (m_estado == Estado::Creando) m_estado = Estado::Cancelada;
    m_cuaderno.reset();
}

void PantallaNuevaPartida::vestir(Player& jugador) const {
    m_jugador->ponerA(jugador, Interno::kCharacterDrawW, Interno::kCharacterDrawH, Interno::characterAnchor());
}

bool PantallaNuevaPartida::vestirPorDefecto(Player& jugador) {
    if (!cargaDatos().isOk() || !m_datos.montaje) {
        std::cerr << "GUION-01: sin personajes.json; el jugador conserva el marcador personaje.png\n";
        return false;
    }
    const Game::ManifiestoPersonajes& manifiesto = m_datos.montaje->manifiesto();
    // El humano de las Marcas es la raza por defecto; si el manifiesto ya no la trae, la primera que haya.
    const Game::RazaVisual* raza = manifiesto.raza("race_human_marches");
    if (raza == nullptr && !manifiesto.razas.empty()) raza = &manifiesto.razas.front();
    if (raza == nullptr) return false;
    const auto vestido = m_jugador->vestir(*m_datos.montaje, Game::Apariencia::deRaza(*raza), {});
    if (!vestido.isOk()) {
        std::cerr << "APARIENCIA-01: " << vestido.errorMessage() << "\n";
        return false;
    }
    m_datos.montaje->vaciarCache();
    vestir(jugador);
    return m_jugador->lista();
}

void PantallaNuevaPartida::actualizarAspecto() {
    if (!m_jugador->lista() || !m_datos.montaje || activa()) return;
    const auto& manifiesto = m_datos.montaje->manifiesto();
    const auto rehecha = m_jugador->vestir(*m_datos.montaje, m_jugador->apariencia(),
                                           Game::Vestuario::piezasVisibles(m_ficha.equippedIds, manifiesto),
                                           Game::Vestuario::brillosVisibles(m_ficha.equippedIds, manifiesto));
    if (!rehecha.isOk()) {
        std::cerr << rehecha.errorMessage() << "\n";
    } else if (rehecha.value()) {
        m_datos.montaje->vaciarCache();   // equipar es raro: mejor releer que guardar
    }
}

Game::CuadernoCreacion::Vista PantallaNuevaPartida::vistaDelJugador() const {
    Game::CuadernoCreacion::Vista v;
    if (!m_jugador->lista() || !m_datos.montaje || activa()) return v;
    v.apariencia = m_jugador->apariencia();
    v.equipo = Game::Vestuario::piezasVisibles(m_ficha.equippedIds, m_datos.montaje->manifiesto());
    return v;
}

void PantallaNuevaPartida::apuntaAspecto(SaveGameData& data) const {
    if (!m_jugador->lista() || data.hero.empty()) return;
    Game::NewGameFlow::guardaApariencia(m_jugador->apariencia(), data.hero);
}

RPG::CharacterSheet* PantallaNuevaPartida::restaurarHeroe(const SaveGameData& data) {
    if (data.hero.empty() || activa() || !cargaDatos().isOk()) return nullptr;
    auto ficha = Game::NewGameFlow::fichaGuardada(*m_datos.reglas, m_datos.tiers, m_datos.equipo, data.hero,
                                                  data.characterName);
    if (!ficha.isOk()) {
        std::cerr << "GUARDAR-FICHA: no se puede rehacer el heroe: " << ficha.errorMessage() << "\n";
        return nullptr;
    }
    m_ficha = std::move(ficha.value());
    m_ficha.id = data.hero.uuid.empty() ? Game::nuevoUuid() : data.hero.uuid;  // D0037: SaveSlots::load ya se lo ha dado
    // Su aspecto, con lo que lleva puesto y su brillo.
    if (m_datos.montaje && !data.hero.raza.empty()) {
        const auto& manifiesto = m_datos.montaje->manifiesto();
        Game::Apariencia guardada = Game::NewGameFlow::aparienciaGuardada(data.hero);
        if (const Game::RazaVisual* raza = manifiesto.raza(guardada.raza))
            guardada.limpiarSegunRaza(*raza);  // CAR-1: pelo, barba y ojos que su pueblo no admite
        const auto vestido = m_jugador->vestir(*m_datos.montaje, guardada,
                                               Game::Vestuario::piezasVisibles(m_ficha.equippedIds, manifiesto),
                                               Game::Vestuario::brillosVisibles(m_ficha.equippedIds, manifiesto));
        if (!vestido.isOk()) std::cerr << vestido.errorMessage() << "\n";
        m_datos.montaje->vaciarCache();
    }
    m_estado = Estado::Cerrada;
    m_cuaderno.reset();
    return &m_ficha;
}

void PantallaNuevaPartida::procesar(const Engine::Input::InputSnapshot& input, const Pergamino::InterfazPergamino* ui) {
    if (!activa() || !m_cuaderno) return;
    Game::CuadernoCreacion& c = *m_cuaderno;
    // El raton: senalar al pasar por encima, elegir con un clic, la rueda
    // mueve la rejilla.
    const auto& raton = input.mouse();
    if (ui != nullptr && raton.inside && (raton.moved || raton.leftPressed)) {
        const Pergamino::Zona* z = ui->zonaEn(raton.x, raton.y);
        if (z != nullptr && esZonaDeCreacion(z->tipo)) {
            const auto zona = static_cast<Zona>(z->tipo);
            if (raton.moved) c.senala(zona, z->indice);
            if (raton.leftPressed) c.pulsa(zona, z->indice);
        }
    }
    if (raton.wheel != 0) c.rueda(raton.wheel);
    // El teclado. Con un campo de texto abierto (la busqueda, el nombre) las
    // letras escriben: WASD, Q y E no mueven ni pasan de hoja.
    const bool izquierda = input.pressed(InputAction::MenuLeft);
    const bool derecha = input.pressed(InputAction::MenuRight);
    if (c.escribiendo()) {
        c.escribe(input.text());
        if (input.pressed(InputAction::Erase)) c.borra();
        if (c.hoja() == Game::CuadernoCreacion::Hoja::Nombre && (izquierda || derecha)) c.mueve(derecha ? 1 : -1, 0);
        // La barra espaciadora tambien es «Confirmar»: si este fotograma ha
        // escrito algo, el espacio era una letra.
        if (input.pressed(InputAction::Confirm) && input.text().empty()) c.acepta();
        if (input.pressed(InputAction::Quit)) c.atras();
    } else {
        if (input.pressed(InputAction::PrevTab)) c.anterior();
        if (input.pressed(InputAction::NextTab)) c.siguiente();
        // La E pasa de hoja aqui (Q/E) aunque en el resto del juego confirme: no hace las dos cosas.
        const bool pasaHoja = input.pressed(InputAction::PrevTab) || input.pressed(InputAction::NextTab);
        const int dx = (input.pressed(InputAction::SideRight) || derecha ? 1 : 0) -
                       (input.pressed(InputAction::SideLeft) || izquierda ? 1 : 0);
        const int dy = (input.pressed(InputAction::MenuDown) ? 1 : 0) - (input.pressed(InputAction::MenuUp) ? 1 : 0);
        if (dx != 0 || dy != 0) c.mueve(dx, dy);
        if (input.pressed(InputAction::Confirm) && !pasaHoja) c.acepta();
        if (input.pressed(InputAction::Quit)) c.atras();
    }
    if (c.estado() == Game::CuadernoCreacion::Estado::Terminado) {
        terminar();
    } else if (c.estado() == Game::CuadernoCreacion::Estado::Cancelado) {
        m_estado = Estado::Cancelada;
    }
}

void PantallaNuevaPartida::terminar() {
    auto ficha = m_cuaderno->finalizar();
    if (!ficha.isOk()) {
        m_error = ficha.errorMessage();
        return;
    }
    m_ficha = std::move(ficha.value());
    m_ficha.id = Game::nuevoUuid();  // D0035
    m_estado = Estado::Terminada;
    // El aspecto vivo del jugador: lo elegido, sin nada puesto (el kit esta
    // en el cofre de El Vacio).
    if (m_datos.montaje && m_cuaderno->editor() != nullptr) {
        const auto vestido = m_jugador->vestir(*m_datos.montaje, m_cuaderno->editor()->apariencia(), {});
        if (!vestido.isOk()) std::cerr << vestido.errorMessage() << "\n";
    }
}

}  // namespace Engine
