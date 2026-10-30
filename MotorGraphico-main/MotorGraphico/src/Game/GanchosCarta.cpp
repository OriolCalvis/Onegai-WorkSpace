// H2-06 — ver include/Game/GanchosCarta.h.
#include "Game/GanchosCarta.h"

#include <cstdlib>

#include "RPG/RandomEngine.h"

namespace Game {
namespace {

using T = Gancho::Tipo;

std::vector<std::string> partir(const std::string& texto, char sep) {
    std::vector<std::string> trozos;
    std::string actual;
    for (char c : texto) {
        if (c == sep) {
            trozos.push_back(actual);
            actual.clear();
        } else {
            actual += c;
        }
    }
    trozos.push_back(actual);
    return trozos;
}

// Lo que queda a partir del trozo i, vuelto a unir con ':'.
std::string resto(const std::vector<std::string>& p, std::size_t i) {
    std::string r;
    for (std::size_t k = i; k < p.size(); ++k) {
        if (k > i) r += ':';
        r += p[k];
    }
    return r;
}

int entero(const std::string& s) {
    if (s.empty()) return 0;
    char* fin = nullptr;
    const long v = std::strtol(s.c_str(), &fin, 10);
    return fin == s.c_str() ? 0 : static_cast<int>(v);
}

// "2" | "combate" -> rondas (-1 = todo el combate).
int rondasDe(const std::string& s) { return s == "combate" ? -1 : entero(s); }

// Ganchos de las cartas que el combate de hoy no simula: no son un error,
// hablan de cosas fuera del combate o de posiciones (ver la cabecera).
bool ignoradoEnCombate(const std::string& verbo) {
    static const char* const kLista[] = {
        "bloquea_acercarse", "ignora_cobertura_ligera",
        "ventaja_social", "interes", "objetivo_de_cobro", "liberarse", "coste_levantarse",
        "escalable", "muerte", "bloquea_tag", "fin_si_inactivo", "pierde_reaccion",
        "bloquea_ataque", "bono_alcance"};
    for (const char* v : kLista) {
        if (verbo == v) return true;
    }
    return false;
}

Gancho rasgo(const std::vector<std::string>& p, Gancho g) {
    const std::string& v = p[0];
    const std::string a = p.size() > 1 ? p[1] : std::string();
    if (v == "desventaja") {
        if (a == "todo") g.tipo = T::DesventajaTodo;
        else if (a == "defensa") g.tipo = T::DesventajaDefensa;
        else if (a == "salvacion_des_con") g.tipo = T::DesventajaDefensaFisica;
        else g.tipo = T::DesventajaAtaque;  // ataque, ataque_y_habilidad, todo_con_fuente_visible
    } else if (v == "ventaja") {
        g.tipo = a == "defensa" ? T::VentajaDefensa : T::VentajaAtaque;
    } else if (v == "ventaja_contra") {
        g.tipo = a == "melee" ? T::VentajaContraMelee
                 : a == "origen" ? T::VentajaContraOrigen : T::VentajaContraTodo;
    } else if (v == "bono_ca" || v == "penal_ca") {
        g.tipo = T::BonoCa;
        g.numero = v == "bono_ca" ? entero(a) : -entero(a);
    } else if (v == "incapacitado" || v == "aturdido") {
        g.tipo = T::Incapacitado;  // aturdido pierde el turno igual; solo cambia el icono
        g.texto = v;
    } else if (v == "velocidad_mitad" || v == "congelado") {
        g.tipo = T::VelocidadMitad;  // AV16-14: pierde la mitad de los pasos del turno
        g.texto = v;
    } else if (v == "velocidad_0") {
        g.tipo = T::VelocidadCero;
    } else if (v == "silenciado") {
        g.tipo = T::Silenciado;  // solo ataques fisicos/marciales y objetos
    } else if (v == "critico_contra") {
        g.tipo = T::CriticoContra;
    } else if (v == "quemado") {
        g.tipo = T::DanoPorTurno;  // como dano_por_turno, con su icono
        g.texto = v;
        g.dados = leerDados(a);
        if (g.dados.vacio()) g.tipo = T::Desconocido;
    } else if (v == "dano_por_turno" || v == "absorbe_por_turno" || v == "bono_dano_arma" ||
               v == "bono_dano_contra" || v == "bono_dano_melee") {
        g.tipo = v == "dano_por_turno"      ? T::DanoPorTurno
                 : v == "absorbe_por_turno" ? T::AbsorbePorTurno
                 : v == "bono_dano_arma"    ? T::BonoDanoArma
                 : v == "bono_dano_contra"  ? T::BonoDanoContra
                                            : T::BonoDanoMelee;
        g.dados = leerDados(a);
        if (g.dados.vacio()) g.tipo = T::Desconocido;
    } else if (v == "refleja") {
        g.tipo = T::Refleja;
    } else if (v == "provocado") {
        g.tipo = T::Provocado;
    } else if (v == "desarmado") {
        g.tipo = T::Desarmado;
    } else if (v == "bono_cura") {
        g.tipo = T::BonoCura;
        g.numero = entero(a);
    } else if (v == "bloquea" && a == "vision") {
        // Mismo traductor que el cargador de condiciones (D-04): cegado ->
        // los ataques contra ti tienen ventaja.
        g.tipo = T::VentajaContraTodo;
    } else if (v == "bloquea" && a == "ataques") {
        g.tipo = T::BloqueaTodos;
    } else if (v == "penal_dados") {
        g.tipo = T::PenalDados;
        g.numero = entero(a) > 0 ? entero(a) : 1;
    } else if (ignoradoEnCombate(v)) {
        g.tipo = T::Ignorado;
    }
    return g;
}

}  // namespace

int Dados::tirar(RPG::RandomEngine& rng) const {
    int total = fijo;
    for (int i = 0; i < cantidad; ++i) {
        total += caras == 6 ? rng.roll_d6() : rng.roll_in_range(1, caras);
    }
    return total;
}

std::string Dados::texto() const {
    if (cantidad <= 0) return std::to_string(fijo);
    std::string t = std::to_string(cantidad) + "d" + std::to_string(caras);
    if (fijo > 0) t += "+" + std::to_string(fijo);
    return t;
}

Dados leerDados(const std::string& texto) {
    Dados d;
    const auto pos = texto.find('d');
    if (pos == std::string::npos) {
        d.fijo = entero(texto);
        return d;
    }
    const int cantidad = pos == 0 ? 1 : entero(texto.substr(0, pos));
    const int caras = entero(texto.substr(pos + 1));
    if (cantidad <= 0 || caras <= 1) return Dados{};  // "1d[pendiente]" no es una tirada
    d.cantidad = cantidad;
    d.caras = caras;
    return d;
}

bool Gancho::esRasgo() const { return tipo >= T::DesventajaTodo && tipo <= T::Ignorado; }

Gancho leerGancho(const std::string& crudo) {
    Gancho g;
    g.crudo = crudo;
    const std::vector<std::string> p = partir(crudo, ':');
    const std::string& v = p[0];
    const std::string a = p.size() > 1 ? p[1] : std::string();
    const std::string b = p.size() > 2 ? p[2] : std::string();
    if (v == "dano") {
        g.tipo = a == "escudo" ? T::DanoEscudo : T::Dano;
        g.dados = leerDados(a);
        if (g.tipo == T::Dano && g.dados.vacio()) g.tipo = T::Desconocido;
    } else if (v == "cura") {
        g.tipo = a == "dano_causado" ? T::CuraDanoCausado : T::Cura;
        g.dados = leerDados(a);
        if (g.tipo == T::Cura && g.dados.vacio()) g.tipo = T::Desconocido;
    } else if (v == "ataque_arma") {
        g.tipo = T::AtaqueArma;
        g.numero = 1;
        if (!a.empty() && a[0] == 'x') g.numero = entero(a.substr(1));
        else g.texto = a;  // "", "critico", "ventaja"
    } else if (v == "ataques_arma") {
        g.tipo = T::AtaquesArma;
        g.numero = entero(a);
    } else if (v == "estado" || v == "si" || v == "propio") {
        g.tipo = v == "estado" ? T::Estado : v == "propio" ? T::Propio : T::Si;
        if (v != "si") {
            g.rondas = rondasDe(a);
        } else {
            const auto arroba = a.find('@');
            g.texto = a.substr(0, arroba);
            g.rondas = arroba == std::string::npos ? -1 : entero(a.substr(arroba + 1));
        }
        const Gancho dentro = leerGancho(resto(p, 2));
        if (dentro.esRasgo() && dentro.tipo != T::Ignorado) g.rasgo.push_back(dentro);
        else g.tipo = T::Desconocido;
    } else if (v == "siempre") {
        // SP-02: la pasiva que no se elige; el rasgo pesa todo el combate.
        g.tipo = T::Siempre;
        g.rondas = -1;
        const Gancho dentro = leerGancho(resto(p, 1));
        if (dentro.esRasgo() && dentro.tipo != T::Ignorado) g.rasgo.push_back(dentro);
        else g.tipo = T::Desconocido;
    } else if (v == "limpia") {
        g.tipo = T::Limpia;
    } else if (v == "condicion") {
        g.tipo = T::Condicion;
        g.texto = a;
        g.rondas = rondasDe(b);
    } else if (v == "escudo") {
        g.tipo = T::Escudo;
        g.numero = entero(a);
        g.maximo = entero(b);
    } else if (v == "turno_extra") {
        g.tipo = T::TurnoExtra;
    } else if (v == "devuelve_carta") {
        g.tipo = T::DevuelveCarta;
        g.texto = a;
    } else if (v == "invoca") {
        g.tipo = T::Invoca;
        g.rondas = rondasDe(a);
        g.opciones = partir(b, ',');
    } else if (v == "bloquea" && a == "ataque") {
        g.tipo = T::Bloquea;
    } else if (v == "bloquea" && a == "ataques" && !b.empty()) {
        g.tipo = T::BloqueaTodos;
        g.rondas = rondasDe(b);
    } else if (v == "dados_extra") {
        g.tipo = T::DadosExtra;
        g.numero = entero(a);
    } else if (v == "revive") {
        g.tipo = T::Revive;
        g.numero = entero(a) > 0 ? entero(a) : 1;
    } else if (v == "defiende_aliado") {
        g.tipo = T::DefiendeAliado;
        g.rondas = rondasDe(a);
    } else if (v == "aliado_ataca") {
        g.tipo = T::AliadoAtaca;
        g.texto = a;
    } else if (v == "pendiente" || v == "pendiente_parcial") {
        g.tipo = v == "pendiente" ? T::Pendiente : T::PendienteParcial;
        g.texto = resto(p, 1);
    } else if (v == "requiere_posiciones") {
        g.tipo = T::RequierePosiciones;
    } else if (v == "coste_desgarro") {
        g.tipo = T::CosteDesgarro;
        g.numero = entero(a);
    } else if (v == "elige_una" || v == "azar_una") {
        g.tipo = v == "elige_una" ? T::EligeUna : T::AzarUna;  // CP-01: azar_una, la suerte elige
        g.opciones = partir(a, ',');
    } else if (v == "recupera_carta") {
        g.tipo = T::RecuperaCarta;
        g.numero = entero(a) > 0 ? entero(a) : 1;
    } else {
        g = rasgo(p, g);
    }
    return g;
}

std::vector<Gancho> leerGanchos(const std::vector<std::string>& crudos) {
    std::vector<Gancho> ganchos;
    ganchos.reserve(crudos.size());
    for (const std::string& c : crudos) ganchos.push_back(leerGancho(c));
    return ganchos;
}

bool esPerjuicio(const Gancho& rasgo) {
    switch (rasgo.tipo) {
        case T::VentajaAtaque:
        case T::VentajaDefensa:
        case T::BonoDanoArma:
        case T::BonoDanoMelee:
        case T::BonoCura:
        case T::Refleja:
        case T::BloqueaTodos:
            return false;
        case T::BonoCa:
            return rasgo.numero < 0;  // penal_ca
        default:
            return true;
    }
}

std::string motivoBloqueo(const std::vector<Gancho>& ganchos) {
    if (ganchos.empty()) return "sin efecto en combate todavia";
    for (const Gancho& g : ganchos) {
        if (g.tipo == T::Pendiente) return "[pendiente] " + g.texto;
        if (g.tipo == T::RequierePosiciones) return "necesita posiciones en el combate";
        if (g.tipo == T::Desconocido) return "efecto no reconocido: " + g.crudo;
    }
    return {};
}

}  // namespace Game
