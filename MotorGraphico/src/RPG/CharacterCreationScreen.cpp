#include "RPG/CharacterCreationScreen.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace RPG {
namespace {

std::string aMinusculas(const std::string& s) {
    std::string r = s;
    for (char& c : r) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return r;
}

bool casaConFiltro(const std::string& texto, const std::string& aguja) {
    return aguja.empty() || aMinusculas(texto).find(aguja) != std::string::npos;
}

}  // namespace

CharacterCreationScreen::CharacterCreationScreen(const CharacterCreation& reglas)
    : m_reglas(reglas) {
    m_eleccion.tier = 1;
    m_eleccion.baseStats = {CharacterCreation::kStatMin, CharacterCreation::kStatMin,
                            CharacterCreation::kStatMin, CharacterCreation::kStatMin};
    rehacerLista();
}

const char* CharacterCreationScreen::tituloPaso() const {
    switch (m_paso) {
        case Paso::Raza:        return "DE QUE PUEBLO VIENES";
        case Paso::Clase:       return "A QUE TE DEDICAS";
        case Paso::Trasfondo:   return "EN QUE MES NACISTE";
        case Paso::Reparto:     return "COMO ERES";
        case Paso::Vinculo:     return "QUE TE ATA A ALGUIEN";
        case Paso::Miedo:       return "QUE TE DA MIEDO";
        case Paso::Defecto:     return "QUE TE ESTROPEA";
        case Paso::Meta:        return "QUE PERSIGUES";
        case Paso::Ideal:       return "EN QUE CREES";
        case Paso::Personalidad:return "COMO TE TRATAN";
        case Paso::Virtud:      return "QUE TE SALVA";
        case Paso::Deidad:      return "A QUIEN REZAS (SE PUEDE NO REZAR)";
        case Paso::Nombre:      return "COMO TE LLAMAS";
        case Paso::Resumen:     return "ASI QUEDAS";
        default:                return "";
    }
}

int CharacterCreationScreen::puntosLibres() const {
    return CharacterCreation::kPuntosRepartibles -
           CharacterCreation::puntosGastados(m_eleccion.baseStats);
}

void CharacterCreationScreen::rehacerLista() {
    m_opciones.clear();
    m_etiquetas.clear();
    m_totalSinFiltrar = 0;
    const std::string aguja = aMinusculas(m_filtro);

    // Se recogen aparte y se ORDENAN por nombre antes de pintarlas.
    // Catalog guarda en un unordered_map, asi que forEach devuelve las
    // entradas en un orden que cambia entre ejecuciones: una lista de 61
    // clases que se baraja sola cada vez que abres la pantalla es
    // imposible de usar y de probar.
    std::vector<std::pair<std::string, std::string>> recogidas;   // (etiqueta, id)
    auto mete = [&](const std::string& id, const std::string& etiqueta) {
        ++m_totalSinFiltrar;
        if (casaConFiltro(etiqueta, aguja) || casaConFiltro(id, aguja)) {
            recogidas.emplace_back(etiqueta, id);
        }
    };

    // Las siete listas narrativas salen del trasfondo ya elegido.
    const CharacterCreation::OpcionesTrasfondo* op =
        m_eleccion.backgroundId.empty() ? nullptr : m_reglas.opcionesDe(m_eleccion.backgroundId);
    auto metelistado = [&](const std::vector<std::string>* v) {
        if (v == nullptr) {
            return;
        }
        for (const std::string& id : *v) {
            mete(id, id);
        }
    };

    switch (m_paso) {
        case Paso::Raza:
            m_reglas.races().forEach([&](const RaceDefinition& r) { mete(r.id, r.name); });
            break;
        case Paso::Clase:
            m_reglas.classes().forEach([&](const ClassDefinition& c) {
                // Una clase de tier superior al del personaje no se ofrece:
                // mejor no verla que verla y que la rechacen al final.
                if (c.tier > m_eleccion.tier) {
                    ++m_totalSinFiltrar;
                    return;
                }
                mete(c.id, c.name);
            });
            break;
        case Paso::Trasfondo:
            m_reglas.backgrounds().forEach(
                [&](const BackgroundDefinition& b) { mete(b.id, b.name); });
            break;
        case Paso::Deidad:
            m_reglas.deities().forEach([&](const DeityDefinition& d) { mete(d.id, d.name); });
            break;
        case Paso::Vinculo:      metelistado(op ? &op->bonds : nullptr); break;
        case Paso::Miedo:        metelistado(op ? &op->fears : nullptr); break;
        case Paso::Defecto:      metelistado(op ? &op->flaws : nullptr); break;
        case Paso::Meta:         metelistado(op ? &op->goals : nullptr); break;
        case Paso::Ideal:        metelistado(op ? &op->ideals : nullptr); break;
        case Paso::Personalidad: metelistado(op ? &op->personalities : nullptr); break;
        case Paso::Virtud:       metelistado(op ? &op->virtues : nullptr); break;
        default: break;   // Reparto, Nombre y Resumen no son listas
    }

    std::sort(recogidas.begin(), recogidas.end());
    // "(ninguna)" va SIEMPRE la primera en el paso de deidad: no toda
    // clase adora a nadie, y que la salida este arriba evita que alguien
    // elija un dios por no encontrarla.
    if (m_paso == Paso::Deidad) {
        m_opciones.push_back("");
        m_etiquetas.push_back("(ninguna)");
    }
    for (auto& par : recogidas) {
        m_etiquetas.push_back(par.first);
        m_opciones.push_back(par.second);
    }

    if (m_marcado >= m_opciones.size()) {
        m_marcado = m_opciones.empty() ? 0 : m_opciones.size() - 1;
    }
}

void CharacterCreationScreen::guardarMarcado() {
    if (m_opciones.empty()) {
        return;
    }
    const std::string id = m_opciones[m_marcado];
    switch (m_paso) {
        case Paso::Raza: m_eleccion.raceId = id; break;
        case Paso::Clase: m_eleccion.classId = id; break;
        case Paso::Trasfondo:
            // Cambiar de trasfondo invalida las siete narrativas: sus ids
            // son de las listas del trasfondo VIEJO. Dejarlas puestas es
            // como se cuela un vinculo que no es de este mes.
            if (m_eleccion.backgroundId != id) {
                m_eleccion.bondId.clear();
                m_eleccion.fearId.clear();
                m_eleccion.flawId.clear();
                m_eleccion.goalId.clear();
                m_eleccion.idealId.clear();
                m_eleccion.personalityId.clear();
                m_eleccion.virtueId.clear();
            }
            m_eleccion.backgroundId = id;
            break;
        case Paso::Vinculo:      m_eleccion.bondId = id; break;
        case Paso::Miedo:        m_eleccion.fearId = id; break;
        case Paso::Defecto:      m_eleccion.flawId = id; break;
        case Paso::Meta:         m_eleccion.goalId = id; break;
        case Paso::Ideal:        m_eleccion.idealId = id; break;
        case Paso::Personalidad: m_eleccion.personalityId = id; break;
        case Paso::Virtud:       m_eleccion.virtueId = id; break;
        case Paso::Deidad:       m_eleccion.deityId = id; break;
        default: break;
    }
}

void CharacterCreationScreen::arriba() {
    if (m_paso == Paso::Reparto) {
        m_statMarcado = (m_statMarcado + 3) % 4;
        return;
    }
    if (!m_opciones.empty()) {
        m_marcado = (m_marcado == 0) ? m_opciones.size() - 1 : m_marcado - 1;
    }
}

void CharacterCreationScreen::abajo() {
    if (m_paso == Paso::Reparto) {
        m_statMarcado = (m_statMarcado + 1) % 4;
        return;
    }
    if (!m_opciones.empty()) {
        m_marcado = (m_marcado + 1) % m_opciones.size();
    }
}

void CharacterCreationScreen::subirStat() {
    if (puntosLibres() <= 0) {
        m_aviso = "no te quedan puntos";
        return;
    }
    int& v = m_eleccion.baseStats[static_cast<std::size_t>(m_statMarcado)];
    if (m_eleccion.tier <= 1 && v >= CharacterCreation::kStatMaxTier01) {
        m_aviso = "en tier 0-1 ningun stat pasa de " +
                  std::to_string(CharacterCreation::kStatMaxTier01);
        return;
    }
    ++v;
    m_aviso.clear();
}

void CharacterCreationScreen::bajarStat() {
    int& v = m_eleccion.baseStats[static_cast<std::size_t>(m_statMarcado)];
    if (v <= CharacterCreation::kStatMin) {
        m_aviso = "no puede bajar de " + std::to_string(CharacterCreation::kStatMin);
        return;
    }
    --v;
    m_aviso.clear();
}

void CharacterCreationScreen::escribirEnFiltro(char c) {
    m_filtro += c;
    m_marcado = 0;
    rehacerLista();
}

void CharacterCreationScreen::borrarDelFiltro() {
    if (!m_filtro.empty()) {
        m_filtro.pop_back();
        m_marcado = 0;
        rehacerLista();
    }
}

void CharacterCreationScreen::limpiarFiltro() {
    if (!m_filtro.empty()) {
        m_filtro.clear();
        m_marcado = 0;
        rehacerLista();
    }
}

void CharacterCreationScreen::escribirEnNombre(char c) { m_eleccion.displayName += c; }

void CharacterCreationScreen::borrarDelNombre() {
    if (!m_eleccion.displayName.empty()) {
        m_eleccion.displayName.pop_back();
    }
}

bool CharacterCreationScreen::siguiente() {
    m_aviso.clear();

    if (m_paso == Paso::Reparto) {
        if (puntosLibres() != 0) {
            m_aviso = "te quedan " + std::to_string(puntosLibres()) + " puntos por repartir";
            return false;
        }
    } else if (m_paso == Paso::Nombre) {
        if (m_eleccion.displayName.empty()) {
            m_aviso = "el personaje necesita un nombre";
            return false;
        }
    } else if (m_paso == Paso::Resumen) {
        return false;   // del resumen se sale con tecla(), no avanzando
    } else if (m_paso != Paso::Deidad) {
        // Los pasos de lista exigen algo marcado. Deidad no: "(ninguna)"
        // es una respuesta.
        if (m_opciones.empty()) {
            m_aviso = m_filtro.empty() ? "no hay nada que elegir aqui"
                                       : "el filtro '" + m_filtro + "' no encuentra nada";
            return false;
        }
        guardarMarcado();
    }
    if (m_paso == Paso::Deidad && !m_opciones.empty()) {
        guardarMarcado();
    }

    m_paso = static_cast<Paso>(static_cast<int>(m_paso) + 1);
    m_marcado = 0;
    m_filtro.clear();
    rehacerLista();
    return true;
}

void CharacterCreationScreen::anterior() {
    m_aviso.clear();
    if (m_paso == Paso::Raza) {
        return;
    }
    m_paso = static_cast<Paso>(static_cast<int>(m_paso) - 1);
    m_marcado = 0;
    m_filtro.clear();
    rehacerLista();
}

CharacterCreationScreen::Accion CharacterCreationScreen::tecla(char k) {
    if (m_paso == Paso::Resumen) {
        if (k == '\n') {
            return problemas().empty() ? Accion::Terminado : Accion::Ninguna;
        }
        if (k == 27) {
            anterior();
        }
        return Accion::Ninguna;
    }

    switch (k) {
        case 'w': arriba(); return Accion::Ninguna;
        case 's': abajo();  return Accion::Ninguna;
        case '\n': siguiente(); return Accion::Ninguna;
        case 27:
            // ESC en el primer paso es cancelar; en los demas, volver.
            if (m_paso == Paso::Raza) {
                return Accion::Cancelado;
            }
            anterior();
            return Accion::Ninguna;
        case '\b':
            if (m_paso == Paso::Nombre) {
                borrarDelNombre();
            } else {
                borrarDelFiltro();
            }
            return Accion::Ninguna;
        default: break;
    }

    if (m_paso == Paso::Reparto) {
        if (k == '+') subirStat();
        if (k == '-') bajarStat();
        return Accion::Ninguna;
    }
    if (m_paso == Paso::Nombre) {
        // El nombre lo lee una persona: se admite lo que se pueda teclear,
        // no solo [a-z0-9_] como en los ids.
        if (k >= ' ' && k < 127) {
            escribirEnNombre(k);
        }
        return Accion::Ninguna;
    }
    // En los pasos de lista, escribir filtra. 'w' y 's' no llegan aqui
    // porque ya se usaron para navegar: es el precio de no tener flechas
    // y esta asumido -- para buscar "wardar" se empieza por otra letra.
    if ((k >= 'a' && k <= 'z') || (k >= '0' && k <= '9') || k == ' ') {
        escribirEnFiltro(k);
    }
    return Accion::Ninguna;
}

}  // namespace RPG
