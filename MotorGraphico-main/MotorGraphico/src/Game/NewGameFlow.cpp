#include "Game/Personajes/RasgosApariencia.h"
#include "Game/NewGameFlow.h"

#include "Game/Personajes/Vestuario.h"
#include "Game/SaveGame.h"
#include "RPG/InventoryEngine.h"

namespace Game {

namespace {
using Paso = RPG::CharacterCreationScreen::Paso;
}  // namespace

NewGameFlow::NewGameFlow(const RPG::CharacterCreation& reglas)
    : m_reglas(reglas), m_pantalla(reglas) {}

NewGameFlow::NewGameFlow(const RPG::CharacterCreation& reglas, const ManifiestoPersonajes* personajes)
    : m_reglas(reglas), m_pantalla(reglas) {
    if (personajes != nullptr) m_apariencia = std::make_unique<EditorApariencia>(*personajes);
}

void NewGameFlow::moveUp() {
    if (m_estado != Estado::Creando) return;
    if (m_enApariencia) {
        m_apariencia->arriba();
    } else {
        m_pantalla.arriba();
    }
}

void NewGameFlow::moveDown() {
    if (m_estado != Estado::Creando) return;
    if (m_enApariencia) {
        m_apariencia->abajo();
    } else {
        m_pantalla.abajo();
    }
}

// Izquierda/derecha solo cambian algo en Apariencia: el reparto de stats
// se retiro (H2-06, Oriol 05/10: la base es 0 y la suben raza y equipo).
void NewGameFlow::adjustLeft() {
    if (m_estado == Estado::Creando && m_enApariencia) m_apariencia->izquierda();
}

void NewGameFlow::adjustRight() {
    if (m_estado == Estado::Creando && m_enApariencia) m_apariencia->derecha();
}

void NewGameFlow::confirm() {
    if (m_estado != Estado::Creando) return;
    if (m_enApariencia) {
        // La pantalla ya esta en Clase: salir de Apariencia es avanzar.
        m_enApariencia = false;
        return;
    }
    if (m_pantalla.paso() == Paso::Resumen) {
        // Del resumen solo se sale terminando (o con ESC, que retrocede).
        if (m_pantalla.problemas().empty()) {
            m_estado = Estado::Terminado;
        }
        return;
    }
    const Paso antes = m_pantalla.paso();
    m_pantalla.siguiente();
    if (m_apariencia && antes == Paso::Raza && m_pantalla.paso() == Paso::Clase) {
        m_apariencia->empezar(m_pantalla.eleccion().raceId);
        m_enApariencia = true;
    }
}

void NewGameFlow::cancel() {
    if (m_estado != Estado::Creando) return;
    if (m_enApariencia) {
        m_enApariencia = false;
        m_pantalla.anterior();   // de Clase a Raza
        return;
    }
    if (m_apariencia && m_pantalla.paso() == Paso::Clase) {
        m_enApariencia = true;   // de Clase se vuelve a Apariencia
        return;
    }
    // ESC en Raza cancela; en los demás pasos la propia pantalla lo
    // traduce a retroceder un paso.
    if (m_pantalla.tecla(27) == RPG::CharacterCreationScreen::Accion::Cancelado) {
        m_estado = Estado::Cancelado;
    }
}

void NewGameFlow::textChar(char c) {
    if (m_estado == Estado::Creando && !m_enApariencia) m_pantalla.tecla(c);
}

void NewGameFlow::backspace() {
    if (m_estado == Estado::Creando && !m_enApariencia) m_pantalla.tecla('\b');
}

int NewGameFlow::numeroDePaso() const {
    const int n = m_pantalla.numeroDePaso();
    if (!m_apariencia) return n;
    if (m_enApariencia) return 2;
    return m_pantalla.paso() == Paso::Raza ? n : n + 1;
}

int NewGameFlow::totalDePasos() const {
    return RPG::CharacterCreationScreen::totalDePasos() + (m_apariencia ? 1 : 0);
}

const char* NewGameFlow::tituloPaso() const {
    return m_enApariencia ? "QUÉ ASPECTO TIENES" : m_pantalla.tituloPaso();
}

NewGameFlow::VistaPrevia NewGameFlow::vistaPrevia() const {
    VistaPrevia vista;
    if (!m_apariencia) return vista;
    vista.eleccion = m_pantalla.eleccion();
    vista.apariencia = m_apariencia->apariencia();
    const auto marcada = [&]() -> std::string {
        const auto& ops = m_pantalla.opciones();
        return m_pantalla.marcado() < ops.size() ? ops[m_pantalla.marcado()] : std::string();
    };
    const Paso paso = m_pantalla.paso();
    if (paso == Paso::Raza) {
        // La raza marcada, como viste su pueblo; la ya editada, como quedo.
        const std::string raza = marcada();
        if (vista.apariencia.raza != raza) {
            if (const RazaVisual* visual = m_apariencia->manifiesto().raza(raza)) {
                vista.apariencia = Apariencia::deRaza(*visual);
            }
        }
        return vista;
    }
    if (m_enApariencia) return vista;
    if (paso == Paso::Clase) vista.eleccion.classId = marcada();
    if (paso == Paso::Trasfondo) vista.eleccion.backgroundId = marcada();
    vista.conEquipo = !vista.eleccion.classId.empty();
    return vista;
}

Result<RPG::CharacterSheet> NewGameFlow::finalizar(const RPG::TierRules& reglas,
                                                  const RPG::Catalogs::EquipmentCatalog* equipo) const {
    if (m_estado == Estado::Cancelado) {
        return Result<RPG::CharacterSheet>::Error("la creacion se cancelo: no hay ficha");
    }
    if (m_estado == Estado::Creando) {
        return Result<RPG::CharacterSheet>::Error("la creacion no ha terminado");
    }
    auto ficha = m_pantalla.construir();
    if (!ficha.isOk()) {
        return ficha;
    }
    // Sin este paso la ficha sale con healthCap()==0 y defensas a cero:
    // ver el comentario de la clase. recalcular() delega en
    // CharacterSheet::recompute_derived(), que es el único dueño de las
    // fórmulas del GDD.
    m_reglas.recalcular(ficha.value(), reglas);
    // Decision de Oriol (29/09): se empieza con el equipo inicial puesto.
    // equip() recalcula la ficha con cada pieza.
    if (equipo != nullptr) {
        if (const RPG::ClassDefinition* clase = m_reglas.classes().find(ficha.value().classId)) {
            Vestuario::equiparInicial(ficha.value(), *clase, reglas, *equipo);
        }
    }
    return ficha;
}

Result<RPG::CharacterSheet> NewGameFlow::fichaSinEquipo(const RPG::CharacterCreation& reglas,
                                                       const RPG::CreationChoice& eleccion,
                                                       const RPG::TierRules& tiers) {
    auto ficha = reglas.construir(eleccion);
    if (!ficha.isOk()) return ficha;
    // construir() deja el equipo de arranque de la clase y del trasfondo en
    // la mochila: aqui se empieza sin nada (el cofre trae el kit).
    ficha.value().inventoryEquipment.clear();
    reglas.recalcular(ficha.value(), tiers);
    return ficha;
}

Result<RPG::CharacterSheet> NewGameFlow::fichaGuardada(const RPG::CharacterCreation& reglas,
                                                      const RPG::TierRules& tiers,
                                                      const RPG::Catalogs::EquipmentCatalog* equipo,
                                                      const SavedHero& heroe, const std::string& nombre) {
    RPG::CreationChoice e;
    e.displayName = nombre;
    e.raceId = heroe.raceId;
    e.classId = heroe.classId;
    e.backgroundId = heroe.backgroundId;
    e.deityId = heroe.deityId;
    e.bondId = heroe.bondId;
    e.fearId = heroe.fearId;
    e.flawId = heroe.flawId;
    e.goalId = heroe.goalId;
    e.idealId = heroe.idealId;
    e.personalityId = heroe.personalityId;
    e.virtueId = heroe.virtueId;
    auto ficha = fichaSinEquipo(reglas, e, tiers);
    if (ficha.isOk()) ficha.value().factionRep = heroe.reputacion;  // AV14-23 (D-MOR-09)
    if (!ficha.isOk() || equipo == nullptr) return ficha;
    RPG::CharacterSheet& f = ficha.value();
    for (std::size_t i = 0; i < heroe.equipped.size() && i < f.equippedIds.size(); ++i) {
        if (equipo->find(heroe.equipped[i]) != nullptr) f.equippedIds[i] = heroe.equipped[i];
    }
    RPG::InventoryEngine::recalc_equipment_bonuses(f, *equipo);
    reglas.recalcular(f, tiers);
    return ficha;
}

Apariencia NewGameFlow::aparienciaGuardada(const SavedHero& heroe) {
    Apariencia a;
    a.raza = heroe.raza;
    a.piel = heroe.piel;
    a.talla = heroe.talla;
    a.ojos = heroe.ojos;
    a.peloEstilo = heroe.peloEstilo;
    a.peloColor = heroe.peloColor;
    a.barbaEstilo = heroe.barbaEstilo;
    a.barbaColor = heroe.barbaColor;
    a.ropa = heroe.ropa;
    a.quitarCapas = heroe.quitarCapas;
    a.marcas = heroe.marcas;
    RasgosApariencia::migrarTallaAnterior(a);
    RasgosApariencia::migrarEstilosRetirados(a);
    return a;
}

void NewGameFlow::guardaApariencia(const Apariencia& a, SavedHero& heroe) {
    heroe.raza = a.raza;
    heroe.piel = a.piel;
    heroe.talla = a.talla;
    heroe.ojos = a.ojos;
    heroe.peloEstilo = a.peloEstilo;
    heroe.peloColor = a.peloColor;
    heroe.barbaEstilo = a.barbaEstilo;
    heroe.barbaColor = a.barbaColor;
    heroe.ropa = a.ropa;
    heroe.quitarCapas = a.quitarCapas;
    heroe.marcas = a.marcas;
}

}  // namespace Game
