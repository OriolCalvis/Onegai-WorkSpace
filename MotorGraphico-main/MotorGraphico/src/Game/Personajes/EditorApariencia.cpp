// APARIENCIA-01 — ver include/Game/Personajes/EditorApariencia.h.
#include "Game/Personajes/RasgosApariencia.h"
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

// EDITOR-PERSONALIZACION-01: los colores de pelo que admite un estilo. Si el atlas trae su fila castana, el
// montaje la tine a cualquier material «pelo_*» con paleta (MontajePersonajes): todos, los del atlas primero
// y el resto por orden. Si no (el pelo propio de una raza), solo los que traiga el atlas.
// NET-E1a: el orden de los de siempre es fijo (con el atlas recortado a la fila castana tambien salen igual):
// Castano, Rubio, Negro, Pelirrojo, Oscuro, Blanco, Gris, Plata lila, Verde musgo, Azul hielo y, despues,
// el resto de materiales «pelo_*» (por nombre).
std::vector<std::string> coloresDePelo(const ManifiestoPersonajes& m, const GrupoDeCapas& grupo,
                                       const std::string& estilo) {
    std::vector<std::string> out = colores(grupo, estilo);
    if (grupo.buscar(estilo + "/pelo_castano") == nullptr || m.paletas.count("pelo_castano") == 0)
        return out;
    static const char* const kOrden[] = {"pelo_castano",  "pelo_rubio",       "pelo_negro",
                                         "pelo_pelirrojo", "pelo_oscuro",      "pelo_blanco",
                                         "pelo_gris",      "pelo_plata_lila",  "pelo_verde_musgo",
                                         "pelo_azul_hielo"};
    for (const char* material : kOrden) {
        if (m.paletas.count(material) != 0 && !contiene(out, material))
            out.push_back(material);
    }
    for (const auto& [material, tonos] : m.paletas) {
        (void)tonos;
        if (material.rfind("pelo_", 0) == 0 && !contiene(out, material))
            out.push_back(material);
    }
    return out;
}

std::vector<std::string> idsDeMarcas(const GrupoDeCapas& grupo) {
    std::vector<std::string> out;
    for (const FilaDeCapa& f : grupo.filas)
        out.push_back(f.id);
    return out;
}

// La posicion de una fila MARCA (0, 1, 2) o -1.
int indiceDeMarca(Fila fila) {
    return fila == Fila::Marca1 ? 0 : fila == Fila::Marca2 ? 1 : fila == Fila::Marca3 ? 2 : -1;
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
    {Fila::Marca1, "MARCA", "rasgo", nullptr, "Pecas, manchas, un tatuaje, una cicatriz... Hasta tres."},
    {Fila::Marca2, "MARCA", "rasgo", nullptr, "Otra marca de la piel."},
    {Fila::Marca3, "MARCA", "rasgo", nullptr, "Otra marca de la piel."},
    {Fila::Alas, "ALAS", "rasgo", "alas", "Alas en la espalda: de murcielago, de plumas, de libelula..."},
    {Fila::Cola, "COLA", "rasgo", "cola", "Una cola: de diablo, de gato, de lagarto, de zorro, de lobo."},
    {Fila::Cuernos, "CUERNOS", "rasgo", "cuernos", "Cuernos en la cabeza: rectos, de carnero, astas..."},
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
        {"pelo_azul_hielo", "Azul hielo"}, {"pelo_llama", "Llama"}, {"BARBA", "Larga"}, {"BARBA_CORTA", "Corta"},
        // EDITOR-PERSONALIZACION-01: los peinados y barbas de PL-ROPA-01
        {"rapado", "Rapado"}, {"flequillo", "Flequillo"}, {"lado", "Mechón ladeado"}, {"tupe", "Tupé"},
        {"puntas", "De punta"}, {"entradas", "Con entradas"}, {"afro", "Afro"},
        {"mohicano", "Mohicano"}, {"mono_alto", "Moño alto"}, {"coletas", "Coletas"}, {"trenza", "Trenza"},
        {"melena_larga", "Melena larga"}, {"media_melena", "Media melena"}, {"ondulado", "Ondulado"},
        {"trenzas", "Trenzas"}, {"coleta_alta", "Coleta alta"}, {"rastas", "Rastas"}, {"tazon", "A tazón"},
        {"undercut", "Rapado a los lados"}, {"PERILLA", "Perilla"}, {"BIGOTE", "Bigote"}, {"PATILLAS", "Patillas"},
        {"BARBA_LARGA", "Hasta el pecho"}, {"BARBA_PARTIDA", "Partida"},
        {"CANDADO", "Candado"}, {"CHIVO", "De chivo"}, {"BIGOTE_MANILLAR", "Bigote de manillar"},
        {"BIGOTE_GRANDE", "Bigote de morsa"}};
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
    // EDITOR-PERSONALIZACION-01: la piel, para cualquier raza con «pieles» en su talla (el tinte del cuerpo)
    const std::string talla = m_apariencia.talla.empty() && raza != nullptr ? raza->talla : m_apariencia.talla;
    const bool conPieles = raza != nullptr && m_manifiesto.pieles.count(talla) != 0;
    if (raza != nullptr && (raza->cuerpoLiso || conPieles))
        m_filas.push_back(Fila::Piel);
    if (raza != nullptr && raza->cuerpoLiso)
        m_filas.push_back(Fila::Talla);
    // CAR-1: ojos, pelo y barba genericos solo en las razas que los admiten (no en cabezas de toro, ave, cobra...)
    if (admite("ojos"))
        m_filas.push_back(Fila::Ojos);
    if (admite("pelo")) {
        m_filas.push_back(Fila::Peinado);
        if (!m_apariencia.peloEstilo.empty())
            m_filas.push_back(Fila::ColorPelo);
    }
    if (admite("barba")) {
        m_filas.push_back(Fila::Barba);
        if (!m_apariencia.barbaEstilo.empty())
            m_filas.push_back(Fila::ColorBarba);
    }
    // EDITOR-PERSONALIZACION-01: las marcas (tres filas; la segunda y la tercera salen cuando hay una antes)
    if (!m_manifiesto.marcas.filas.empty() && m_manifiesto.marcas.atlas.count(talla) != 0) {
        m_filas.push_back(Fila::Marca1);
        if (m_apariencia.marcas.size() >= 1)
            m_filas.push_back(Fila::Marca2);
        if (m_apariencia.marcas.size() >= 2)
            m_filas.push_back(Fila::Marca3);
    }
    const auto rasgos = RasgosApariencia::filas(m_manifiesto, raza, talla);
    m_filas.insert(m_filas.end(), rasgos.begin(), rasgos.end());
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
    const RazaVisual* raza = m.raza(m_apariencia.raza);
    const std::string talla = m_apariencia.talla.empty() && raza != nullptr ? raza->talla : m_apariencia.talla;
    const auto pieles = m.pieles.find(talla);
    switch (fila) {
        case Fila::Piel: {
            // EDITOR-PERSONALIZACION-01: con «pieles», la de tu pueblo ("") y las demas; si no, el atlas de cuerpos
            if (pieles == m.pieles.end())
                return ids(m.cuerpos);
            std::vector<std::string> out{""};
            for (const std::string& id : pieles->second.orden) {
                if (raza == nullptr || id != raza->id)
                    out.push_back(id);
            }
            return out;
        }
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
            return coloresDePelo(m, m.pelo, m_apariencia.peloEstilo);
        case Fila::Barba: {
            std::vector<std::string> out{""};
            for (const std::string& e : estilos(m.barba))
                out.push_back(e);
            return out;
        }
        case Fila::ColorBarba:
            return coloresDePelo(m, m.barba, m_apariencia.barbaEstilo);
        case Fila::Marca1:
        case Fila::Marca2:
        case Fila::Marca3: {
            std::vector<std::string> out{""};
            for (const std::string& id : idsDeMarcas(m.marcas))
                out.push_back(id);
            return out;
        }
        case Fila::AlAzar:
        case Fila::ComoSuRaza:
            return {};
        default:
            break;
    }
    const std::string ranura = ranuraDe(fila);
    std::vector<std::string> out = RasgosApariencia::esRasgo(ranura) ? RasgosApariencia::opciones(raza, ranura) : std::vector<std::string>{""};
    const auto grupo = m.equipo.find(ranura);
    if (grupo != m.equipo.end()) {
        for (const std::string& id : idsDe(grupo->second, ranuraDe(fila)))
            out.push_back(id);
    }
    return out;
}

std::string EditorApariencia::actual(Fila fila) const {
    const Apariencia& a = m_apariencia;
    switch (fila) {
        case Fila::Piel: {
            // EDITOR-PERSONALIZACION-01: con «pieles», la de la raza es ""
            const RazaVisual* raza = m_manifiesto.raza(a.raza);
            const std::string talla = a.talla.empty() && raza != nullptr ? raza->talla : a.talla;
            if (raza != nullptr && m_manifiesto.pieles.count(talla) != 0 && (a.piel == raza->piel || a.piel == raza->id))
                return "";
            return a.piel;
        }
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
        case Fila::Marca1:
        case Fila::Marca2:
        case Fila::Marca3: {
            const int i = indiceDeMarca(fila);
            return static_cast<std::size_t>(i) < a.marcas.size() ? a.marcas[static_cast<std::size_t>(i)] : std::string();
        }
        case Fila::AlAzar:
        case Fila::ComoSuRaza:
            return "";
        default:
            break;
    }
    if ((ranuraDe(fila) != nullptr && RasgosApariencia::esRasgo(ranuraDe(fila))))
        return RasgosApariencia::actual(a, m_manifiesto.raza(a.raza), ranuraDe(fila));
    const auto it = a.ropa.find(ranuraDe(fila));
    return it == a.ropa.end() ? std::string() : it->second;
}

void EditorApariencia::poner(Fila fila, const std::string& valor) {
    Apariencia& a = m_apariencia;
    if ((ranuraDe(fila) != nullptr && RasgosApariencia::esRasgo(ranuraDe(fila)))) { RasgosApariencia::poner(a, m_manifiesto.raza(a.raza), ranuraDe(fila), valor); return; }
    const RazaVisual* raza = m_manifiesto.raza(a.raza);
    // Al elegir un estilo se conserva el color si ese estilo lo tiene; si no,
    // el de la raza, el del pelo (para la barba) o el primero que haya.
    const auto colorPara = [&](const GrupoDeCapas& grupo, const std::string& estilo,
                               std::initializer_list<std::string> preferidos) {
        const std::vector<std::string> hay = coloresDePelo(m_manifiesto, grupo, estilo);
        for (const std::string& c : preferidos) {
            if (!c.empty() && contiene(hay, c))
                return c;
        }
        return hay.empty() ? std::string() : hay.front();
    };
    switch (fila) {
        case Fila::Piel: {
            // EDITOR-PERSONALIZACION-01: "" = la de la raza
            const std::string talla = a.talla.empty() && raza != nullptr ? raza->talla : a.talla;
            if (valor.empty() && raza != nullptr && m_manifiesto.pieles.count(talla) != 0)
                a.piel = raza->piel;
            else
                a.piel = valor;
            break;
        }
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
            if (aJuego && contiene(coloresDePelo(m_manifiesto, m_manifiesto.barba, a.barbaEstilo), valor))
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
        case Fila::Marca1:
        case Fila::Marca2:
        case Fila::Marca3: {
            const auto i = static_cast<std::size_t>(indiceDeMarca(fila));
            if (valor.empty()) {
                if (i < a.marcas.size())
                    a.marcas.erase(a.marcas.begin() + static_cast<std::ptrdiff_t>(i));
            } else {
                if (i >= a.marcas.size())
                    a.marcas.push_back(valor);
                else
                    a.marcas[i] = valor;
            }
            break;
        }
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
    const auto pieles = m.pieles.find(raza->talla);
    if (pieles != m.pieles.end()) {
        // EDITOR-PERSONALIZACION-01: una de cada cuatro veces, otra piel (de otra raza o un material)
        if (tirada(100) < 25)
            a.piel = uno(pieles->second.orden);
    } else if (raza->cuerpoLiso) {
        a.piel = uno(ids(m.cuerpos));
    }
    // CAR-1: «Al azar» no toca los ojos (la capa de ojos aun no es fiable: CAR-2): se quedan los de la raza
    if (raza->admite("pelo")) {
        a.peloEstilo = tirada(100) < 85 ? uno(estilos(m.pelo)) : "";
        if (!a.peloEstilo.empty())
            a.peloColor = uno(coloresDePelo(m, m.pelo, a.peloEstilo));
    }
    if (raza->admite("barba")) {
        a.barbaEstilo = tirada(100) < 30 ? uno(estilos(m.barba)) : "";
        if (!a.barbaEstilo.empty()) {
            const std::vector<std::string> hay = coloresDePelo(m, m.barba, a.barbaEstilo);
            a.barbaColor = contiene(hay, a.peloColor) ? a.peloColor : uno(hay);
        }
    }
    // EDITOR-PERSONALIZACION-01: marcas (una de cada tres veces, una; una de cada diez, dos), alas, cola y
    // cuernos (uno de cada diez)
    a.marcas.clear();
    if (!m.marcas.filas.empty() && m.marcas.atlas.count(raza->talla) != 0) {
        const int cuantas = tirada(100) < 10 ? 2 : tirada(100) < 33 ? 1 : 0;
        for (int i = 0; i < cuantas; ++i) {
            const std::string id = uno(idsDeMarcas(m.marcas));
            if (!contiene(a.marcas, id))
                a.marcas.push_back(id);
        }
    }
    for (const char* ranura : kRanurasDeRasgos) {
        const auto grupo = m.equipo.find(ranura);
        if (grupo == m.equipo.end() || contiene(raza->capas, ranura) || grupo->second.atlas.count(raza->talla) == 0 ||
            tirada(100) >= 10)
            continue;
        a.ropa[ranura] = uno(idsDe(grupo->second, ranura));
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
    if (v == "__raza__") return "De tu pueblo";
    switch (fila) {
        case Fila::AlAzar:
            return "Otra combinacion";
        case Fila::ComoSuRaza:
            return "Como viste tu pueblo";
        case Fila::Peinado:
            return v.empty() ? "Sin pelo" : nombreDe(v);
        case Fila::Barba:
            return v.empty() ? "Sin barba" : nombreDe(v);
        case Fila::Piel: {
            // EDITOR-PERSONALIZACION-01: el nombre de la opcion de «pieles» (una raza o un material)
            const RazaVisual* raza = m_manifiesto.raza(m_apariencia.raza);
            const std::string talla = m_apariencia.talla.empty() && raza != nullptr ? raza->talla : m_apariencia.talla;
            const auto pieles = m_manifiesto.pieles.find(talla);
            if (pieles != m_manifiesto.pieles.end()) {
                if (v.empty())
                    return "La de tu pueblo";
                const auto op = pieles->second.opciones.find(v);
                if (op != pieles->second.opciones.end())
                    return op->second.raza ? "Como " + op->second.nombre : nombreDe(v);
            }
            return nombreDe(v);
        }
        case Fila::Marca1:
        case Fila::Marca2:
        case Fila::Marca3: {
            if (v.empty())
                return "Ninguna";
            const FilaDeCapa* marca = m_manifiesto.marcas.buscar(v);
            return marca != nullptr && !marca->nombre.empty() ? marca->nombre : nombreDe(v);
        }
        case Fila::Alas:
        case Fila::Cola:
        case Fila::Cuernos:
            if (v.empty())
                return fila == Fila::Cuernos ? "Ningunos" : fila == Fila::Alas ? "Ningunas" : "Ninguna";
            break;
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
