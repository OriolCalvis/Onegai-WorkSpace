// APARIENCIA-01 — ver include/Game/Personajes/EditorApariencia.h.
#include "Game/Personajes/EditorApariencia.h"

#include <algorithm>
#include <initializer_list>
#include <map>
#include <random>

namespace Game {

namespace {

using Fila = EditorApariencia::Fila;

bool contiene(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

// DIALOGOS-01 (Oriol, 13/10: «limitar [lo] que aparezca como ropa inicial,
// solo algunos y de los mas basicos») — La ropa de diario es ropa: fuera los
// trajes de clase (sp_, cp_, it3_, con sus brillos) y las armaduras, que se
// ganan con el equipo.
bool esRopaBasica(const std::string& id) {
    for (const char* prefijo : {"sp_", "cp_", "it3_"})
        if (id.rfind(prefijo, 0) == 0) return false;
    for (const char* pieza : {"yelmo", "bacinete", "casco", "corona", "diadema", "cota_malla", "escamas", "placas",
                              "quijotes", "grebas", "botas_acero", "escarpes", "mascara", "antifaz", "gorro_bufon",
                              "bufon", "kimono", "sobrevesta"})
        if (id.find(pieza) != std::string::npos) return false;
    return true;
}

// Lo que puede elegir el jugador: las prendas de oficio de los PNJ no (PNJ-01),
// y solo la ropa basica.
std::vector<std::string> ids(const GrupoDeCapas& grupo) {
    std::vector<std::string> out;
    for (const FilaDeCapa& f : grupo.filas) {
        if (!f.soloPnj && !f.noEncaja && esRopaBasica(f.id))  // AV16-03: ni la que no encaja en el cuerpo
            out.push_back(f.id);
    }
    return out;
}

// DROWS-01 (Oriol, 14/10: «quitaría en el creador complementos como sombreros u
// otros de la cabeza, más que alguna corona de flores, un pañuelo o cosas así,
// que vayan por debajo de un gorro o un casco») — En la cabeza, solo lo que va
// por debajo; sombreros, capuchas y cascos se ganan con el equipo (y, puestos,
// mandan sobre esto: MontajePersonajes, «lo puesto manda»).
bool vaPorDebajo(const std::string& id) {
    for (const char* pieza : {"panuelo", "corona_flores", "corona_ramas", "cofia", "cinta"})
        if (id.rfind(pieza, 0) == 0) return true;
    return false;
}

std::vector<std::string> idsDe(const GrupoDeCapas& grupo, const std::string& ranura) {
    std::vector<std::string> out = ids(grupo);
    if (ranura == "cabeza") out.erase(std::remove_if(out.begin(), out.end(), [](const std::string& id) { return !vaPorDebajo(id); }), out.end());
    return out;
}

// Los estilos de pelo o barba, en el orden del atlas y sin repetir.
std::vector<std::string> estilos(const GrupoDeCapas& grupo) {
    std::vector<std::string> out;
    for (const FilaDeCapa& f : grupo.filas) {
        if (!f.estilo.empty() && !contiene(out, f.estilo))
            out.push_back(f.estilo);
    }
    return out;
}

std::vector<std::string> colores(const GrupoDeCapas& grupo, const std::string& estilo) {
    std::vector<std::string> out;
    for (const FilaDeCapa& f : grupo.filas) {
        if (f.estilo == estilo && !contiene(out, f.color))
            out.push_back(f.color);
    }
    return out;
}

// Lo que se pinta de cada fila: etiqueta, icono de iconos.json, la ranura
// si es ropa y el detalle del pie. Una tabla y no tres switch: una fila
// nueva es una linea.
struct DatosDeFila {
    Fila fila;
    const char* etiqueta;
    const char* icono;
    const char* ranura;  // nullptr si no es ropa
    const char* ayuda;
};
const char* const kRopa = "Tu ropa de diario. Lo que te equipes en la partida se pone encima.";
const DatosDeFila kFilas[] = {
    {Fila::Piel, "PIEL", "personaje", nullptr,
     "El tono de la piel. Solo los pueblos de cuerpo liso pueden cambiarlo."},
    {Fila::Talla, "TALLA", "personaje", nullptr, "Tu talla. La ropa y el equipo se ajustan solos."},
    {Fila::Ojos, "OJOS", "rasgo", nullptr, "El color de los ojos."},
    {Fila::Peinado, "PEINADO", "rasgo", nullptr, "El corte de pelo. Un yelmo cerrado lo tapa."},
    {Fila::ColorPelo, "COLOR DEL PELO", "rasgo", nullptr,
     "El color del pelo. Si la barba iba a juego, lo sigue."},
    {Fila::Barba, "BARBA", "rasgo", nullptr, "Larga, corta o sin barba."},
    {Fila::ColorBarba, "COLOR DE LA BARBA", "rasgo", nullptr, "El color de la barba."},
    {Fila::Cabeza, "CABEZA", "ranura_cabeza", "cabeza", kRopa},
    {Fila::Torso, "TORSO", "ranura_torso", "torso", kRopa},
    {Fila::Piernas, "PIERNAS", "ranura_piernas", "piernas", kRopa},
    {Fila::Pies, "PIES", "ranura_pies", "pies", kRopa},
    {Fila::Capa, "CAPA", "ranura_accesorio", "capa", kRopa},
    {Fila::AlAzar, "AL AZAR", "inspiracion", nullptr,
     "Izquierda o derecha: otra combinacion al azar, dentro de lo que admite tu pueblo."},
    {Fila::ComoSuRaza, "COMO TU PUEBLO", "lore", nullptr,
     "Izquierda o derecha: vuelve a como viste tu pueblo."},
};

const DatosDeFila& datosDe(Fila fila) {
    for (const DatosDeFila& d : kFilas) {
        if (d.fila == fila)
            return d;
    }
    return kFilas[0];
}

// La ranura de ropa de una fila, o nullptr si no es de ropa.
const char* ranuraDe(Fila fila) { return datosDe(fila).ranura; }

// Los ids del manifiesto que no traen nombre (pieles, tallas, ojos,
// peinados, colores y barbas), como los lee el jugador.
const std::map<std::string, std::string>& nombres() {
    // clang-format off
    static const std::map<std::string, std::string> tabla = {
        {"piel_clara", "Clara"}, {"piel_morena", "Morena"}, {"piel_palida", "Pálida"},
        {"piel_oscura", "Oscura"}, {"piel_aceituna", "Aceituna"}, {"piel_rojiza", "Rojiza"},
        {"piel_gris", "Gris"}, {"piel_elfo", "Élfica"}, {"porcelana", "Porcelana"},
        {"medio", "Normal"}, {"bajo", "Baja y ancha"}, {"pequeno", "Pequeña"}, {"grande", "Grande"},
        {"azul", "Azules"}, {"negro", "Negros"}, {"amarillo", "Amarillos"}, {"rojo", "Rojos"},
        {"violeta", "Violetas"}, {"dorado", "Dorados"}, {"blanco", "Blancos"},
        {"marron", "Marrones"}, {"verde", "Verdes"}, {"ambar", "Ámbar"},
        {"corto", "Corto"}, {"largo", "Largo"}, {"rizado", "Rizado"}, {"cresta", "Cresta"},
        {"greñas", "Greñas"}, {"recogido", "Recogido"}, {"coleta", "Coleta"},
        {"pelo_castano", "Castaño"}, {"pelo_rubio", "Rubio"}, {"pelo_negro", "Negro"},
        {"pelo_pelirrojo", "Pelirrojo"}, {"pelo_oscuro", "Oscuro"}, {"pelo_blanco", "Blanco"},
        {"pelo_gris", "Gris"}, {"pelo_plata_lila", "Plata lila"}, {"pelo_verde_musgo", "Verde musgo"},
        {"pelo_azul_hielo", "Azul hielo"}, {"BARBA", "Larga"}, {"BARBA_CORTA", "Corta"}};
    // clang-format on
    return tabla;
}

}  // namespace

EditorApariencia::EditorApariencia(const ManifiestoPersonajes& manifiesto)
    : m_manifiesto(manifiesto) {}

void EditorApariencia::empezar(const std::string& razaId) {
    if (m_apariencia.raza != razaId || m_filas.empty()) {
        const RazaVisual* raza = m_manifiesto.raza(razaId);
        m_apariencia = raza != nullptr ? Apariencia::deRaza(*raza) : Apariencia{};
        m_apariencia.raza = razaId;
        m_filas.clear();
        m_marcada = 0;
    }
    rehacerFilas();
}

void EditorApariencia::rehacerFilas() {
    const Fila antes = filaMarcada();
    const bool habiaFilas = !m_filas.empty();
    const RazaVisual* raza = m_manifiesto.raza(m_apariencia.raza);
    const auto admite = [&](const char* ranura) { return raza == nullptr || raza->admite(ranura); };

    m_filas.clear();
    if (raza != nullptr && raza->cuerpoLiso) {
        m_filas.push_back(Fila::Piel);
        m_filas.push_back(Fila::Talla);
    }
    m_filas.push_back(Fila::Ojos);
    // Pelo y barba, solo si el pueblo tiene pelo (ni plumas ni escamas).
    if (admite("pelo")) {
        m_filas.push_back(Fila::Peinado);
        if (!m_apariencia.peloEstilo.empty())
            m_filas.push_back(Fila::ColorPelo);
        m_filas.push_back(Fila::Barba);
        if (!m_apariencia.barbaEstilo.empty())
            m_filas.push_back(Fila::ColorBarba);
    }
    for (Fila f : {Fila::Cabeza, Fila::Torso, Fila::Piernas, Fila::Pies, Fila::Capa}) {
        if (admite(ranuraDe(f)))
            m_filas.push_back(f);
    }
    m_filas.push_back(Fila::AlAzar);
    m_filas.push_back(Fila::ComoSuRaza);

    const auto it = std::find(m_filas.begin(), m_filas.end(), antes);
    if (habiaFilas && it != m_filas.end()) {
        m_marcada = static_cast<std::size_t>(it - m_filas.begin());
    } else {
        m_marcada = std::min(m_marcada, m_filas.size() - 1);
    }
}

void EditorApariencia::arriba() {
    if (!m_filas.empty())
        m_marcada = m_marcada == 0 ? m_filas.size() - 1 : m_marcada - 1;
}

void EditorApariencia::abajo() {
    if (!m_filas.empty())
        m_marcada = (m_marcada + 1) % m_filas.size();
}

void EditorApariencia::izquierda() { cambiar(filaMarcada(), -1); }
void EditorApariencia::derecha() { cambiar(filaMarcada(), +1); }

std::vector<std::string> EditorApariencia::opciones(Fila fila) const {
    const ManifiestoPersonajes& m = m_manifiesto;
    switch (fila) {
        case Fila::Piel:
            return ids(m.cuerpos);
        case Fila::Talla: {
            std::vector<std::string> out;
            for (const char* t : {"medio", "bajo", "pequeno", "grande"}) {
                if (m.cuerpos.atlas.count(t) != 0)
                    out.push_back(t);
            }
            return out;
        }
        case Fila::Ojos:
            return ids(m.ojos);
        case Fila::Peinado: {
            std::vector<std::string> out{""};
            for (const std::string& e : estilos(m.pelo))
                out.push_back(e);
            return out;
        }
        case Fila::ColorPelo:
            return colores(m.pelo, m_apariencia.peloEstilo);
        case Fila::Barba: {
            std::vector<std::string> out{""};
            for (const std::string& e : estilos(m.barba))
                out.push_back(e);
            return out;
        }
        case Fila::ColorBarba:
            return colores(m.barba, m_apariencia.barbaEstilo);
        case Fila::AlAzar:
        case Fila::ComoSuRaza:
            return {};
        default:
            break;
    }
    std::vector<std::string> out{""};
    const auto grupo = m.equipo.find(ranuraDe(fila));
    if (grupo != m.equipo.end()) {
        for (const std::string& id : idsDe(grupo->second, ranuraDe(fila)))
            out.push_back(id);
    }
    return out;
}

std::string EditorApariencia::actual(Fila fila) const {
    const Apariencia& a = m_apariencia;
    switch (fila) {
        case Fila::Piel:
            return a.piel;
        case Fila::Talla:
            return a.talla;
        case Fila::Ojos: {
            const RazaVisual* raza = m_manifiesto.raza(a.raza);
            return a.ojos.empty() && raza != nullptr ? raza->ojos : a.ojos;
        }
        case Fila::Peinado:
            return a.peloEstilo;
        case Fila::ColorPelo:
            return a.peloColor;
        case Fila::Barba:
            return a.barbaEstilo;
        case Fila::ColorBarba:
            return a.barbaColor;
        case Fila::AlAzar:
        case Fila::ComoSuRaza:
            return "";
        default:
            break;
    }
    const auto it = a.ropa.find(ranuraDe(fila));
    return it == a.ropa.end() ? std::string() : it->second;
}

void EditorApariencia::poner(Fila fila, const std::string& valor) {
    Apariencia& a = m_apariencia;
    const RazaVisual* raza = m_manifiesto.raza(a.raza);
    // Al elegir un estilo se conserva el color si ese estilo lo tiene; si no,
    // el de la raza, el del pelo (para la barba) o el primero que haya.
    const auto colorPara = [&](const GrupoDeCapas& grupo, const std::string& estilo,
                               std::initializer_list<std::string> preferidos) {
        const std::vector<std::string> hay = colores(grupo, estilo);
        for (const std::string& c : preferidos) {
            if (!c.empty() && contiene(hay, c))
                return c;
        }
        return hay.empty() ? std::string() : hay.front();
    };
    switch (fila) {
        case Fila::Piel:
            a.piel = valor;
            break;
        case Fila::Talla:
            a.talla = valor;
            break;
        case Fila::Ojos:
            a.ojos = valor;
            break;
        case Fila::Peinado:
            a.peloEstilo = valor;
            if (!valor.empty()) {
                a.peloColor = colorPara(m_manifiesto.pelo, valor,
                                        {a.peloColor, raza != nullptr ? raza->peloColor : ""});
            }
            break;
        case Fila::ColorPelo: {
            // La barba sigue al pelo si iban a juego.
            const bool aJuego = !a.barbaEstilo.empty() && a.barbaColor == a.peloColor;
            a.peloColor = valor;
            if (aJuego && contiene(colores(m_manifiesto.barba, a.barbaEstilo), valor))
                a.barbaColor = valor;
            break;
        }
        case Fila::Barba:
            a.barbaEstilo = valor;
            if (!valor.empty()) {
                a.barbaColor = colorPara(m_manifiesto.barba, valor, {a.barbaColor, a.peloColor});
            }
            break;
        case Fila::ColorBarba:
            a.barbaColor = valor;
            break;
        case Fila::AlAzar:
        case Fila::ComoSuRaza:
            break;
        default:
            if (valor.empty()) {
                a.ropa.erase(ranuraDe(fila));
            } else {
                a.ropa[ranuraDe(fila)] = valor;
            }
            break;
    }
    rehacerFilas();
}

void EditorApariencia::cambiar(Fila fila, int paso) {
    if (fila == Fila::AlAzar) {
        alAzar();
        return;
    }
    if (fila == Fila::ComoSuRaza) {
        const RazaVisual* raza = m_manifiesto.raza(m_apariencia.raza);
        if (raza != nullptr)
            m_apariencia = Apariencia::deRaza(*raza);
        rehacerFilas();
        return;
    }
    const std::vector<std::string> ops = opciones(fila);
    if (ops.empty())
        return;
    const int n = static_cast<int>(ops.size());
    const auto it = std::find(ops.begin(), ops.end(), actual(fila));
    // Un valor que no esta en la lista (dato raro) cuenta como el anterior al primero.
    const int i = it == ops.end() ? (paso > 0 ? -1 : 0) : static_cast<int>(it - ops.begin());
    poner(fila, ops[static_cast<std::size_t>(((i + paso) % n + n) % n)]);
}

void EditorApariencia::alAzar() {
    const RazaVisual* raza = m_manifiesto.raza(m_apariencia.raza);
    if (raza == nullptr)
        return;
    std::mt19937 dado(0x9E3779B9u ^ (++m_tirada * 2654435761u));
    const auto tirada = [&](int caras) {
        return static_cast<int>(dado() % static_cast<std::uint32_t>(caras));
    };
    const auto uno = [&](const std::vector<std::string>& v) {
        return v.empty() ? std::string()
                         : v[static_cast<std::size_t>(tirada(static_cast<int>(v.size())))];
    };
    const ManifiestoPersonajes& m = m_manifiesto;
    Apariencia a = Apariencia::deRaza(*raza);  // la talla de la raza se queda
    a.ropa.clear();
    if (raza->cuerpoLiso)
        a.piel = uno(ids(m.cuerpos));
    a.ojos = uno(ids(m.ojos));
    if (raza->admite("pelo")) {
        a.peloEstilo = tirada(100) < 85 ? uno(estilos(m.pelo)) : "";
        if (!a.peloEstilo.empty())
            a.peloColor = uno(colores(m.pelo, a.peloEstilo));
        a.barbaEstilo = tirada(100) < 30 ? uno(estilos(m.barba)) : "";
        if (!a.barbaEstilo.empty()) {
            const std::vector<std::string> hay = colores(m.barba, a.barbaEstilo);
            a.barbaColor = contiene(hay, a.peloColor) ? a.peloColor : uno(hay);
        }
    }
    const std::pair<const char*, int> ropa[] = {
        {"cabeza", 35}, {"torso", 100}, {"piernas", 80}, {"pies", 85}, {"capa", 35}};
    for (const auto& [ranura, probabilidad] : ropa) {
        const auto grupo = m.equipo.find(ranura);
        if (grupo == m.equipo.end() || !raza->admite(ranura) || tirada(100) >= probabilidad)
            continue;
        a.ropa[ranura] = uno(idsDe(grupo->second, ranura));
    }
    m_apariencia = a;
    rehacerFilas();
}

const char* EditorApariencia::etiqueta(Fila fila) { return datosDe(fila).etiqueta; }
const char* EditorApariencia::ayuda(Fila fila) { return datosDe(fila).ayuda; }
const char* EditorApariencia::icono(Fila fila) { return datosDe(fila).icono; }

std::string EditorApariencia::valor(Fila fila) const {
    const std::string v = actual(fila);
    switch (fila) {
        case Fila::AlAzar:
            return "Otra combinacion";
        case Fila::ComoSuRaza:
            return "Como viste tu pueblo";
        case Fila::Peinado:
            return v.empty() ? "Sin pelo" : nombreDe(v);
        case Fila::Barba:
            return v.empty() ? "Sin barba" : nombreDe(v);
        case Fila::Piel:
        case Fila::Talla:
        case Fila::Ojos:
        case Fila::ColorPelo:
        case Fila::ColorBarba:
            return nombreDe(v);
        default:
            break;
    }
    if (v.empty())
        return "Nada";
    const auto grupo = m_manifiesto.equipo.find(ranuraDe(fila));
    const FilaDeCapa* pieza =
        grupo == m_manifiesto.equipo.end() ? nullptr : grupo->second.buscar(v);
    return pieza != nullptr && !pieza->nombre.empty() ? pieza->nombre : nombreDe(v);
}

std::string EditorApariencia::nombreDe(const std::string& id) {
    const auto it = nombres().find(id);
    if (it != nombres().end())
        return it->second;
    std::string out = id;
    for (char& c : out) {
        if (c == '_')
            c = ' ';
    }
    if (!out.empty() && out[0] >= 'a' && out[0] <= 'z')
        out[0] = static_cast<char>(out[0] - 'a' + 'A');
    return out;
}

}  // namespace Game
