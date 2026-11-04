// APARIENCIA-01 — ver include/Game/Personajes/ManifiestoPersonajes.h.
#include "Game/Personajes/RasgosApariencia.h"
#include "Game/Personajes/ManifiestoPersonajes.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

#include "Core/Json/JsonValue.h"

namespace Game {

const char* const kRanurasDeEquipo[7] = {"cabeza", "torso", "piernas", "pies",
                                         "capa",   "arma",  "mano_izq"};

bool esRanuraDeRopa(const std::string& ranura) {
    return ranura == "torso" || ranura == "piernas" || ranura == "pies" || ranura == "capa";
}

const char* const kRanurasDeRasgos[3] = {"alas", "cola", "cuernos"};

namespace {

bool contiene(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

Result<JsonValue> leerJson(const std::string& ruta) {
    std::ifstream in(ruta, std::ios::binary);
    if (!in)
        return Result<JsonValue>::Error("no se puede abrir " + ruta);
    std::stringstream ss;
    ss << in.rdbuf();
    auto parsed = JsonValue::parse(ss.str());
    if (!parsed.isOk())
        return Result<JsonValue>::Error(ruta + ": " + parsed.errorMessage());
    return parsed;
}

std::vector<std::string> textos(const JsonValue& v) {
    std::vector<std::string> out;
    if (!v.isArray())
        return out;
    for (std::size_t i = 0; i < v.size(); ++i)
        out.push_back(v[i].asString());
    return out;
}

std::map<std::string, std::string> mapaDeTextos(const JsonValue& v) {
    std::map<std::string, std::string> out;
    if (!v.isObject())
        return out;
    for (const auto& [k, x] : v.objectValues()) {
        if (x.isString())
            out[k] = x.asString();
    }
    return out;
}

// [estilo, color] o null. Devuelve false si no es un par.
bool leerPar(const JsonValue& v, std::string& estilo, std::string& color) {
    estilo.clear();
    color.clear();
    if (!v.isArray() || v.size() < 2)
        return false;
    estilo = v[0].asString();
    color = v[1].asString();
    return !estilo.empty();
}

GrupoDeCapas leerGrupo(const JsonValue& v, const char* claveFilas) {
    GrupoDeCapas g;
    g.atlas = mapaDeTextos(v["atlas"]);
    const JsonValue& filas = v[claveFilas];
    for (std::size_t i = 0; i < filas.size(); ++i) {
        const JsonValue& f = filas[i];
        FilaDeCapa fila;
        fila.id = f["id"].asString();
        fila.estilo = f["estilo"].asString();
        fila.color = f["color"].asString();
        fila.nombre = f["nombre"].asString();
        fila.fila = f["fila"].asInt(0);
        fila.soloPnj = f["soloPnj"].asBool(false);
        fila.razas = textos(f["razas"]);
        fila.tallas = textos(f["tallas"]);
        if (f["orden"].isObject()) {
            for (const auto& [rumbo, z] : f["orden"].objectValues())
                fila.orden[rumbo] = z.asInt(0);
        }
        if (!fila.id.empty() && fila.fila > 0)
            g.filas.push_back(std::move(fila));
    }
    g.ocultaPelo = textos(v["ocultaPelo"]);
    return g;
}

RazaVisual leerRaza(const JsonValue& r) {
    RazaVisual raza;
    raza.id = r["id"].asString();
    raza.nombre = r["nombre"].asString(raza.id);
    raza.talla = r["talla"].asString("medio");
    raza.tallaRopa = r["tallaRopa"].asString(raza.talla);
    raza.fila = r["fila"].asInt(0);
    raza.capas = textos(r["capas"]);
    raza.sin = textos(r["sin"]);
    raza.piel = r["piel"].asString();
    raza.ojos = r["ojos"].asString();
    leerPar(r["pelo"], raza.peloEstilo, raza.peloColor);
    leerPar(r["barba"], raza.barbaEstilo, raza.barbaColor);
    raza.equipo = mapaDeTextos(r["equipo"]);
    if (r["liso"].isBool())
        raza.lisoDeclarado = r["liso"].asBool() ? 1 : 0;
    return raza;
}

// "rrggbb" -> color; si no lo es, `color` no cambia.
void leerColor(const std::string& hex, std::array<std::uint8_t, 3>& color) {
    if (hex.size() == 6 && hex.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos) {
        for (std::size_t c = 0; c < 3; ++c)
            color[c] = static_cast<std::uint8_t>(std::stoi(hex.substr(c * 2, 2), nullptr, 16));
    }
}

// ESTETICA-01: {"nivel": 1..10, "color": "rrggbb", "particulas": "..."}; nivel 0 si no hay.
// PART-02b: y, en un abalorio, "sitio" y el otro efecto ("otrasParticulas", "otroColor").
BrilloDePieza leerBrillo(const JsonValue& v) {
    BrilloDePieza b;
    if (!v.isObject())
        return b;
    b.nivel = std::max(0, std::min(10, v["nivel"].asInt(0)));
    b.particulas = v["particulas"].asString("chispas");
    b.abalorio = v["abalorio"].asBool(false);
    leerColor(v["color"].asString("ffffff"), b.color);
    b.sitio = v["sitio"].asString();
    b.otrasParticulas = v["otrasParticulas"].asString();
    leerColor(v["otroColor"].asString("ffffff"), b.otroColor);
    return b;
}

// PART-02b: de donde echa un abalorio por la ranura del catalogo (el collar, del pecho; el cinturon, de la
// cintura; un anillo o lo que se lleva en la mano sin verse, la caracola o el cuerno, de la mano del arma:
// si va en la otra, lo dice Vestuario). Otra (los monoculos, las lentes): el cuerpo entero.
std::string sitioDeRanura(const std::string& slot) {
    if (slot == "Neck")
        return "pecho";
    if (slot == "Belt")
        return "cintura";
    if (slot == "Ring" || slot == "Accessory" || slot == "MainHand")
        return "mano";
    if (slot == "Ring2" || slot == "OffHand")
        return "otraMano";
    return "";
}

// PART-02b: "x,y x,y ..." -> puntos; vacio si alguno no se lee.
std::vector<AnclasDeTalla::Punto> leerPuntos(const std::string& texto) {
    std::vector<AnclasDeTalla::Punto> out;
    std::istringstream in(texto);
    std::string par;
    while (in >> par) {
        const auto coma = par.find(',');
        if (coma == std::string::npos || coma == 0 || coma + 1 >= par.size() ||
            par.find_first_not_of("0123456789,") != std::string::npos)
            return {};
        out.push_back({std::stoi(par.substr(0, coma)), std::stoi(par.substr(coma + 1))});
    }
    return out;
}

}  // namespace

const std::vector<AnclasDeTalla::Punto>* AnclasDeTalla::de(const std::string& sitio) const {
    const std::vector<Punto>* v = sitio == "mano"       ? &mano
                                  : sitio == "otraMano" ? &otraMano
                                  : sitio == "pecho"    ? &pecho
                                  : sitio == "cintura"  ? &cintura
                                                        : nullptr;
    return v != nullptr && !v->empty() ? v : nullptr;
}

PiezaTenida separarTinte(const std::string& visible) {
    PiezaTenida out;
    const std::size_t dos = visible.find(':');
    out.pieza = visible.substr(0, dos);
    if (dos == std::string::npos)
        return out;
    std::size_t i = dos + 1;
    while (i <= visible.size()) {
        std::size_t fin = visible.find(',', i);
        if (fin == std::string::npos)
            fin = visible.size();
        const std::string par = visible.substr(i, fin - i);
        const std::size_t flecha = par.find('>');
        if (flecha != std::string::npos && flecha > 0 && flecha + 1 < par.size())
            out.tinte.emplace_back(par.substr(0, flecha), par.substr(flecha + 1));
        i = fin + 1;
    }
    return out;
}

std::string juntarTinte(const std::string& pieza,
                        const std::vector<std::pair<std::string, std::string>>& tinte) {
    std::string out = pieza;
    char sep = ':';
    for (const auto& [origen, destino] : tinte) {
        if (origen.empty() || destino.empty() || origen == destino)
            continue;
        out += sep;
        out += origen + ">" + destino;
        sep = ',';
    }
    return out;
}

const FilaDeCapa* GrupoDeCapas::buscar(const std::string& id) const {
    const std::string base = id.substr(0, id.find(':'));
    for (const FilaDeCapa& f : filas) {
        if (f.id == base)
            return &f;
    }
    return nullptr;
}

bool RazaVisual::admite(const std::string& ranura) const { return !contiene(sin, ranura); }

Apariencia Apariencia::deRaza(const RazaVisual& raza, bool conArmas) {
    Apariencia a;
    a.raza = raza.id;
    a.piel = raza.piel;
    a.talla = raza.talla;
    a.ojos = raza.ojos;
    if (raza.admite("pelo")) {
        a.peloEstilo = raza.peloEstilo;
        a.peloColor = raza.peloColor;
    }
    if (raza.admite("barba")) {
        a.barbaEstilo = raza.barbaEstilo;
        a.barbaColor = raza.barbaColor;
    }
    for (const auto& [ranura, pieza] : raza.equipo) {
        if (!conArmas && (ranura == "arma" || ranura == "mano_izq"))
            continue;
        a.ropa[ranura] = pieza;
    }
    return a;
}

void Apariencia::limpiarSegunRaza(const RazaVisual& raza) {
    if (!raza.admite("pelo")) {
        peloEstilo.clear();
        peloColor.clear();
    }
    if (!raza.admite("barba")) {
        barbaEstilo.clear();
        barbaColor.clear();
    }
    if (!raza.admite("ojos"))
        ojos = raza.ojos;
}

std::string Apariencia::clave() const {
    std::string c = raza + "|" + piel + "|" + talla + "|" + ojos + "|" + peloEstilo + "/" +
                    peloColor + "|" + barbaEstilo + "/" + barbaColor + "|";
    for (const auto& [ranura, pieza] : ropa)
        c += ranura + "=" + pieza + ";";
    c += "|";
    for (const std::string& q : quitarCapas)
        c += q + ";";
    if (!marcas.empty()) {
        c += "|";
        for (const std::string& q : marcas)
            c += q + ";";
    }
    return c;
}

const RazaVisual* ManifiestoPersonajes::raza(const std::string& id) const {
    for (const RazaVisual& r : razas) {
        if (r.id == id)
            return &r;
    }
    return nullptr;
}

Result<ManifiestoPersonajes> ManifiestoPersonajes::cargar(const std::string& carpetaTexturas) {
    auto leido = leerJson(carpetaTexturas + "/personajes.json");
    if (!leido.isOk())
        return Result<ManifiestoPersonajes>::Error("Personajes: " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    ManifiestoPersonajes m;
    m.frames = raiz["frames"].asInt(20);

    for (const auto& [ranura, dirs] : raiz["orden"].objectValues()) {
        for (const auto& [dir, z] : dirs.objectValues())
            m.orden[ranura][dir] = z.asInt(0);
    }
    for (const auto& [talla, def] : raiz["tallas"].objectValues()) {
        m.nombreDeTalla[talla] = def["nombre"].asString(talla);
        // PART-02b: las anclas de la talla (de donde echan los abalorios), si las trae enteras
        const JsonValue& anclas = def["anclas"];
        if (anclas.isObject()) {
            AnclasDeTalla a;
            a.mano = leerPuntos(anclas["mano"].asString());
            a.otraMano = leerPuntos(anclas["otraMano"].asString());
            a.pecho = leerPuntos(anclas["pecho"].asString());
            a.cintura = leerPuntos(anclas["cintura"].asString());
            const std::size_t n = a.mano.size();
            if (n > 0 && a.otraMano.size() == n && a.pecho.size() == n && a.cintura.size() == n)
                m.anclas[talla] = std::move(a);
        }
    }
    m.atlasDeRaza = mapaDeTextos(raiz["razas"]["atlas"]);
    const JsonValue& lista = raiz["razas"]["lista"];
    for (std::size_t i = 0; i < lista.size(); ++i)
        m.razas.push_back(leerRaza(lista[i]));
    m.cuerpos = leerGrupo(raiz["cuerpos"], "filas");
    m.ojos = leerGrupo(raiz["ojos"], "filas");
    m.pelo = leerGrupo(raiz["pelo"], "filas");
    m.barba = leerGrupo(raiz["barba"], "filas");
    // EDITOR-PERSONALIZACION-01: las marcas de la piel (si el generador las trae)
    if (raiz["marcas"].isObject())
        m.marcas = leerGrupo(raiz["marcas"], "filas");
    // EDITOR-PERSONALIZACION-01: las pieles de cada talla
    for (const auto& [talla, def] : raiz["pieles"].objectValues()) {
        PielesDeTalla p;
        const auto leerColores = [](const JsonValue& v) {
            std::vector<std::array<std::uint8_t, 3>> out;
            for (std::size_t i = 0; v.isArray() && i < v.size(); ++i) {
                const JsonValue& c = v[i];
                if (c.isArray() && c.size() == 3)
                    out.push_back({static_cast<std::uint8_t>(c[0].asInt(0)), static_cast<std::uint8_t>(c[1].asInt(0)),
                                   static_cast<std::uint8_t>(c[2].asInt(0))});
            }
            return out;
        };
        p.familia = leerColores(def["familia"]);
        for (const auto& [id, op] : def["opciones"].objectValues()) {
            OpcionDePiel o;
            o.nombre = op["nombre"].asString(id);
            o.raza = op["raza"].asBool(false);
            o.colores = leerColores(op["colores"]);
            if (o.colores.size() == p.familia.size() && !p.familia.empty()) {
                p.orden.push_back(id);
                p.opciones[id] = std::move(o);
            }
        }
        if (!p.opciones.empty())
            m.pieles[talla] = std::move(p);
    }
    for (const auto& [ranura, def] : raiz["equipo"].objectValues()) {
        m.equipo[ranura] = leerGrupo(def, "piezas");
    }
    // AV16-03: la ropa que el creador no ofrece porque no encaja (lo escribe tools/auditar_ropa.py;
    // sin el fichero se ofrece todo lo basico, como antes).
    const auto malas = leerJson(carpetaTexturas + "/personajes/ropa_que_no_encaja.json");
    if (malas.isOk()) {
        for (const auto& [ranura, piezas] : malas.value()["noEncaja"].objectValues()) {
            const auto grupo = m.equipo.find(ranura);
            if (grupo == m.equipo.end())
                continue;
            for (const auto& [id, tallas] : piezas.objectValues()) {
                (void)tallas;
                for (FilaDeCapa& f : grupo->second.filas) {
                    if (f.id == id)
                        f.noEncaja = true;
                }
            }
        }
    }
    for (RazaVisual& r : m.razas) {
        const bool capasLisas = r.lisoDeclarado >= 0 ? r.lisoDeclarado == 1
                                                      : r.capas.size() == 1 && r.capas[0] == "cuerpo";
        r.cuerpoLiso = capasLisas && m.cuerpos.buscar(r.piel) != nullptr;
    }

    if (m.orden.empty() || m.razas.empty() || m.pelo.filas.empty() || m.equipo.empty()) {
        return Result<ManifiestoPersonajes>::Error(
            "Personajes: " + carpetaTexturas +
            "/personajes.json no trae orden, razas, pelo o equipo");
    }
    for (const auto& [grupo, specs] : raiz["personajes"].objectValues()) {
        for (const auto& [id, spec] : specs.objectValues()) {
            auto a = m.desdeComposicion(spec);
            if (!a.isOk()) {
                return Result<ManifiestoPersonajes>::Error("Personajes: " + grupo + "/" + id +
                                                           ": " + a.errorMessage());
            }
            m.personajes[id] = std::move(a.value());
        }
    }

    // ESTETICA-01: los cinco tonos de cada material, "eef3f6 b4c0c8 7b8893
    // 3a424a 24292e" (luz, base, sombra, contorno, contorno duro).
    for (const auto& [material, texto] : raiz["paletas"].objectValues()) {
        if (!texto.isString())
            continue;
        TonosDeMaterial tonos{};
        std::istringstream in(texto.asString());
        std::string hex;
        int n = 0;
        while (n < 5 && in >> hex) {
            if (hex.size() != 6 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
                break;
            for (int c = 0; c < 3; ++c)
                tonos[static_cast<std::size_t>(n)][static_cast<std::size_t>(c)] =
                    static_cast<std::uint8_t>(std::stoi(hex.substr(static_cast<std::size_t>(c) * 2, 2), nullptr, 16));
            ++n;
        }
        if (n != 5) {
            return Result<ManifiestoPersonajes>::Error("Personajes: la paleta '" + material +
                                                       "' no tiene cinco tonos");
        }
        m.paletas[material] = tonos;
    }

    // La pieza de cada objeto. Sin el catalogo el juego sigue: solo no se ve
    // el equipo puesto (el aspecto elegido si).
    auto catalogo = leerJson(carpetaTexturas + "/personajes/equipo_catalogo.json");
    if (catalogo.isOk()) {
        for (const auto& [objeto, def] : catalogo.value()["objetos"].objectValues()) {
            PiezaDeObjeto p{def["ranura"].asString(), def["pieza"].asString(), {}};
            // El tinte va en orden ([[origen, destino], ...]): si dos origenes
            // compartieran un tono, manda el primero, como en el probador.
            const JsonValue& tinte = def["tinte"];
            for (std::size_t i = 0; tinte.isArray() && i < tinte.size(); ++i) {
                if (tinte[i].isArray() && tinte[i].size() == 2)
                    p.tinte.emplace_back(tinte[i][0].asString(), tinte[i][1].asString());
            }
            p.brillo = leerBrillo(def["brillo"]);
            if (!p.ranura.empty() && !p.pieza.empty())
                m.piezaDeObjeto[objeto] = std::move(p);
        }
        // Lo que no se ve puesto (anillos, amuletos...) tambien brilla.
        const JsonValue& sin = catalogo.value()["sinPieza"];
        for (std::size_t i = 0; sin.isArray() && i < sin.size(); ++i) {
            BrilloDePieza b = leerBrillo(sin[i]["brillo"]);
            if (b.nivel > 0 && !sin[i]["id"].asString().empty()) {
                b.abalorio = true;
                if (b.sitio.empty())
                    b.sitio = sitioDeRanura(sin[i]["slot"].asString());
                m.brilloDeAbalorio[sin[i]["id"].asString()] = b;
            }
        }
    }
    return Result<ManifiestoPersonajes>::Ok(std::move(m));
}

Result<Apariencia> ManifiestoPersonajes::desdeComposicion(const JsonValue& spec) const {
    const RazaVisual* r = raza(spec["raza"].asString());
    if (r == nullptr)
        return Result<Apariencia>::Error("no hay raza '" + spec["raza"].asString() + "'");
    Apariencia a = Apariencia::deRaza(*r, /*conArmas=*/true);
    a.ropa.clear();
    if (spec.has("piel") && spec["piel"].isString())
        a.piel = spec["piel"].asString();
    if (spec.has("talla") && spec["talla"].isString())
        a.talla = spec["talla"].asString();
    if (spec.has("ojos") && spec["ojos"].isString())
        a.ojos = spec["ojos"].asString();
    // Presente = explicito (null o [] deja sin pelo; un par se pinta aunque
    // la raza no tenga pelo, como la naga del rio). Ausente = el de la raza.
    if (spec.has("pelo"))
        leerPar(spec["pelo"], a.peloEstilo, a.peloColor);
    if (spec.has("barba"))
        leerPar(spec["barba"], a.barbaEstilo, a.barbaColor);
    a.ropa = mapaDeTextos(spec["equipo"]);
    a.quitarCapas = textos(spec["quitar_capas"]);
    a.marcas = textos(spec["marcas"]);
    RasgosApariencia::migrarTallaAnterior(a);
    RasgosApariencia::migrarEstilosRetirados(a);
    return Result<Apariencia>::Ok(std::move(a));
}

}  // namespace Game
