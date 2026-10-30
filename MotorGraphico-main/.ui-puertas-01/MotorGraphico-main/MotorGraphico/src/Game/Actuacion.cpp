// ACTUAR-01 — Actuar en la calle. Ver include/Game/Actuacion.h.
#include "Game/Actuacion.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "Game/SessionData.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/DefinicionesObjetos.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/Narrative/NarrativeState.h"
#include "RPG/RandomEngine.h"
#include "RPG/TiradaVisible.h"
#include "Render/ICombatant.h"

namespace Game {
namespace {

// El reloj de pasos: nace con la primera actuacion (quien no actua no lo
// lleva en el guardado) y cada sitio apunta en que paso se actuo (+1: el 0
// es «nunca»).
const char* const kReloj = "actuacion.pasos";

std::string claveDelSitio(const std::string& id) {
    return "actuacion." + id;
}

// PO-49 — Con la cuenta de dias de la historia en marcha (dia >= 1), en cada
// sitio se actua UNA vez por dia: aqui se apunta en que dia. Sin dias (dia 0,
// un nivel suelto), vale el descanso de pasos de siempre.
std::string claveDelDia(const std::string& id) {
    return "actuacion.dia." + id;
}

int diaDe(const RPG::NarrativeState& estado) {
    return RPG::franjas::momentoDe(estado).dia;
}

std::string enMinusculas(std::string s) {
    for (char& c : s)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    return s;
}

std::string conMayuscula(std::string s) {
    if (!s.empty() && s[0] >= 'a' && s[0] <= 'z') s[0] = static_cast<char>(s[0] - 'a' + 'A');
    return s;
}

// «1» o «1,5»: los exitos van de medio en medio.
std::string exitos(float n) {
    const int medios = static_cast<int>(n * 2.0f + 0.5f);
    return std::to_string(medios / 2) + (medios % 2 != 0 ? ",5" : "");
}

bool llevaEtiqueta(const RPG::EquipmentDefinition& e, const char* etiqueta) {
    return std::find(e.grantedTags.begin(), e.grantedTags.end(), etiqueta) != e.grantedTags.end();
}

// Lo que se dice de la gente, segun el grado y cuantos se paran.
std::string loQueHaceLaGente(const Actuacion& a) {
    const std::string n = std::to_string(a.publico);
    switch (a.grado) {
        case RPG::Degree::BOTCH:
            return a.tirada.botch ? "A la primera se te tuerce todo y alguien se ríe. La gente sigue su camino."
                                  : "La gente pasa de largo: hoy nadie tiene tiempo para ti.";
        case RPG::Degree::PARTIAL:
            return a.publico == 1 ? "Se para 1 persona un momento."
                                  : "Se paran " + n + " personas un momento.";
        case RPG::Degree::SUCCESS:
            return a.publico == 1 ? "Se queda 1 persona hasta el final y te aplaude."
                                  : "Se forma un corro de " + n + " personas que te aplauden al final.";
        case RPG::Degree::CRITICAL:
            return a.publico == 1 ? "¡Quien pasa se queda embobado y te pide otra!"
                                  : "¡Se para todo el que pasa! " + n + " personas te aplauden y te piden otra.";
    }
    return "";
}

}  // namespace

ConQueActuas conQueActuas(const RPG::EquipmentDefinition* mano) {
    ConQueActuas con;
    if (mano == nullptr || !(llevaEtiqueta(*mano, "Instrumento") || llevaEtiqueta(*mano, "Malabares"))) return con;
    const std::string nombre = enMinusculas(mano->name);
    const std::string primera = nombre.substr(0, nombre.find(' '));
    con.nombre = (primera.size() > 1 && primera.back() == 's' ? "tus " : "tu ") + nombre;
    con.dadosExtra = mano->rarity >= RPG::ItemRarity::Epic ? 3 : mano->rarity >= RPG::ItemRarity::Rare ? 2 : 1;
    return con;
}

Actuacion actua(const PerformanceData& sitio, int carisma, const ConQueActuas& con, RPG::RandomEngine& rng) {
    Actuacion a;
    a.dados = std::max(0, carisma) + con.dadosExtra;
    a.rivalDice = std::max(0, sitio.rivalDice);
    a.tirada = RPG::DicePoolEngine::roll_pool(a.dados, RPG::DiceMod::NORMAL, rng);
    a.rival = RPG::DicePoolEngine::roll_pool(a.rivalDice, RPG::DiceMod::NORMAL, rng);
    a.grado = RPG::DicePoolEngine::opposed_degree(a.tirada, a.rival);

    // Cuantos se paran (y unos pocos mas, al azar) y cuanto echa cada uno.
    const int pasan = std::max(0, sitio.afluencia);
    const int propina = static_cast<int>(a.grado);  // 0, 1, 2 y 3 de oro por cabeza
    const int divisor = a.grado == RPG::Degree::PARTIAL ? 4 : a.grado == RPG::Degree::SUCCESS ? 2 : 1;
    const int base = propina > 0 ? std::max(1, pasan / divisor) : 0;
    if (base > 0) a.publico = base + rng.roll_in_range(0, base / 4);
    a.oro = a.publico * propina;

    const std::string lugar = conMayuscula(sitio.lugar.empty() ? "aquí" : sitio.lugar);
    a.lineas.push_back(con.nombre.empty() ? lugar + ", sin nada en las manos, cantas y bailas lo que te sabes."
                                          : lugar + ", con " + con.nombre + ", empiezas tu número.");
    a.lineas.push_back(loQueHaceLaGente(a));
    a.lineas.push_back(a.oro > 0 ? "Ganas " + std::to_string(a.oro) + " de oro." : "No cae ni una moneda.");
    return a;
}

void cuentaUnPaso(RPG::NarrativeState& estado) {
    if (estado.hasVariable(kReloj)) estado.addToVariable(kReloj, 1);
}

bool yaActuasteHoy(const RPG::NarrativeState& estado, const std::string& sitioId) {
    const int dia = diaDe(estado);
    const std::string clave = claveDelDia(sitioId);
    return dia > 0 && estado.hasVariable(clave) && estado.variable(clave) == dia;
}

int pasosQueFaltan(const RPG::NarrativeState& estado, const std::string& sitioId) {
    if (diaDe(estado) > 0) return 0;  // con dias manda el dia, no los pasos
    const std::string clave = claveDelSitio(sitioId);
    if (!estado.hasVariable(clave)) return 0;
    const int andados = estado.variable(kReloj) - (estado.variable(clave) - 1);
    return std::max(0, kPasosDeDescanso - andados);
}

Actuacion actuaEn(SessionData& s, const ObjectDefinition& sitio) {
    RPG::NarrativeState* estado = s.m_narrativeState;
    Actuacion a;
    const int faltan = estado != nullptr ? pasosQueFaltan(*estado, sitio.id) : 0;
    const bool hoyYa = estado != nullptr && yaActuasteHoy(*estado, sitio.id);
    if (hoyYa || faltan > 0) {
        a.descansando = true;
        a.pasosQueFaltan = faltan;
        a.lineas = {hoyYa ? "La gente que pasa por aquí ya te ha visto hoy. Vuelve mañana."
                          : "La gente que pasa por aquí ya te ha visto. Date una vuelta y vuelve luego."};
    } else {
        // La carta de Interpretacion dice con que se tira (CAR) y como se llama.
        const RPG::SkillDefinition* carta = s.m_nd6Skills != nullptr ? s.m_nd6Skills->find("interpretacion") : nullptr;
        const RPG::Stat stat = carta != nullptr ? carta->casting_stat : RPG::Stat::CAR;
        const RPG::CharacterSheet* ficha = s.m_playerBody != nullptr ? s.m_playerBody->character_sheet() : nullptr;
        const RPG::EquipmentDefinition* mano = nullptr;
        if (ficha != nullptr && s.m_inventoryRules.equipment != nullptr) {
            const std::string& id = ficha->equippedIds[static_cast<std::size_t>(RPG::EquipSlot::MainHand)];
            if (!id.empty()) mano = s.m_inventoryRules.equipment->find(id);
        }
        a = actua(sitio.actuacion, s.m_playerBody != nullptr ? s.m_playerBody->stat(stat) : 0, conQueActuas(mano),
                  *s.m_rngInUse);
        if (estado != nullptr) {
            if (!estado->hasVariable(kReloj)) estado->setVariable(kReloj, 0);
            estado->setVariable(claveDelSitio(sitio.id), estado->variable(kReloj) + 1);
            if (diaDe(*estado) > 0) estado->setVariable(claveDelDia(sitio.id), diaDe(*estado));
            // H2-01 — La foto de la tirada, para la hoja del dialogo.
            RPG::TiradaVisible vista;
            vista.que = carta != nullptr && !carta->name.empty() ? carta->name : "Interpretación";
            vista.quien = ficha != nullptr && !ficha->displayName.empty() ? ficha->displayName : "Tú";
            vista.contra = "El público";
            vista.tirada = a.tirada;
            vista.rival = a.rival;
            vista.enfrentada = a.rivalDice > 0;
            vista.grado = a.grado;
            estado->ponTirada(std::move(vista));
        }
        s.m_log.push_back("(actuacion en " + sitio.id + ": " + std::to_string(a.dados) + "d6, " +
                          exitos(a.tirada.successes) + " exitos vs " + std::to_string(a.rivalDice) + "d6, " +
                          exitos(a.rival.successes) + " exitos -> grado " +
                          std::to_string(static_cast<int>(a.grado)) + ")");
    }
    // La voz del narrador: sin quien hable.
    s.m_dialogueSpeaker.clear();
    s.m_dialogueLines = a.lineas;
    s.m_shopOffers.clear();
    s.m_mode = GameMode::Dialogue;
    for (const std::string& linea : a.lineas) s.m_log.push_back(linea);
    return a;
}

}  // namespace Game
