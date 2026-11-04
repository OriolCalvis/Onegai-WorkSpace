// E-DISPARADORES-1 (D0026) — Lo de las cinematicas que empiezan por una casilla: el punto de entrada y,
// mas adelante, el disparador (lleva, accion, radio y vista). Fichero propio: Cinematica.cpp ya pasa del techo.
#include <algorithm>
#include <cmath>

#include "Core/Json/JsonValue.h"
#include "Game/Cinematica.h"

namespace Game {

namespace {

bool celda(const JsonValue& v, GridCoord& c) {
    if (!v.isArray() || v.size() != 2 || !v[0].isNumber() || !v[1].isNumber()) return false;
    c = GridCoord{v[0].asInt(), v[1].asInt()};
    return true;
}

bool soloClaves(const JsonValue& o, std::initializer_list<const char*> validas, std::string& rara) {
    for (const auto& [clave, valor] : o.objectValues()) {
        bool vale = false;
        for (const char* v : validas) vale = vale || clave == v;
        if (!vale) {
            rara = clave;
            return false;
        }
    }
    return true;
}

}  // namespace

Result<Disparador> leeDisparador(const JsonValue& o, const std::string& id) {
    const std::string donde = "cinematicas: «" + id + "», disparador: ";
    auto error = [&](const std::string& que) { return Result<Disparador>::Error(donde + que); };
    if (!o.isObject()) return error("tiene que ser un objeto");
    std::string rara;
    if (!soloClaves(o, {"lleva", "accion", "once", "radio", "vision"}, rara))
        return error("«" + rara + "» no existe (lleva, accion, once, radio, vision)");
    Disparador d;
    d.hay = true;
    if (o.has("lleva")) {
        const JsonValue& l = o["lleva"];
        if (!l.isObject() || !soloClaves(l, {"a", "mira", "como"}, rara))
            return error("«lleva» es {a: [x, y], mira, como}" + (rara.empty() ? "" : " (sobra «" + rara + "»)"));
        if (!celda(l["a"], d.lleva)) return error("«lleva» necesita «a»: [x, y]");
        d.hayLleva = true;
        d.mira = l["mira"].asString();
        if (!d.mira.empty() && d.mira != "arriba" && d.mira != "abajo" && d.mira != "izquierda" && d.mira != "derecha")
            return error("«mira» es arriba, abajo, izquierda o derecha");
        const std::string como = l["como"].asString("andando");
        if (como != "andando" && como != "fundido") return error("«como» es andando o fundido");
        d.andando = como == "andando";
    }
    if (o.has("accion")) {
        const JsonValue& a = o["accion"];
        if (!a.isObject() || a.objectValues().size() != 1)
            return error("«accion» lleva exactamente una clave (cinematica, beat o marca)");
        const auto& [clave, valor] = *a.objectValues().begin();
        if (clave == "combate") d.accion = Disparador::Accion::Combate;
        else if (clave == "cinematica") d.accion = Disparador::Accion::Cinematica;
        else if (clave == "beat") d.accion = Disparador::Accion::Beat;
        else if (clave == "marca") d.accion = Disparador::Accion::Marca;
        else return error("la accion «" + clave + "» no existe (cinematica, beat, marca o combate)");
        if (!valor.isString() || valor.asString().empty()) return error("«" + clave + "» necesita un id");
        d.accionId = valor.asString();
    }
    if (o.has("once")) {
        if (!o["once"].isBool()) return error("«once» es true o false");
        d.once = o["once"].asBool(true);
    }
    if (o.has("radio")) {
        const JsonValue& r = o["radio"];
        if (r.isString() && r.asString() == "nivel") d.radio = -1;
        else if (r.isNumber() && r.asInt() >= 0) d.radio = r.asInt();
        else return error("«radio» es un entero >= 0 o \"nivel\"");
    }
    if (o.has("vision")) {
        if (!o["vision"].isBool()) return error("«vision» es true o false");
        d.vision = o["vision"].asBool(true) ? 1 : 0;
    }
    return Result<Disparador>::Ok(d);
}

GridCoord Cinematica::puntoDeEntrada(const std::function<bool(const GridCoord&)>& transitable) const {
    if (disparador.hayLleva) return disparador.lleva;
    if (!hayZona) return GridCoord{-1, -1};
    const GridCoord centro{(zonaDesde.x + zonaHasta.x) / 2, (zonaDesde.y + zonaHasta.y) / 2};
    if (!transitable || transitable(centro)) return centro;
    const int radioMax = std::max(zonaHasta.x - zonaDesde.x, zonaHasta.y - zonaDesde.y);
    for (int r = 1; r <= radioMax; ++r)
        for (int y = centro.y - r; y <= centro.y + r; ++y)
            for (int x = centro.x - r; x <= centro.x + r; ++x) {
                if (std::max(std::abs(x - centro.x), std::abs(y - centro.y)) != r) continue;  // solo el anillo
                if (x < zonaDesde.x || x > zonaHasta.x || y < zonaDesde.y || y > zonaHasta.y) continue;
                if (transitable(GridCoord{x, y})) return GridCoord{x, y};
            }
    return centro;  // ninguna transitable: el centro (el validador lo avisa)
}

// ---------------------------------------------------------------- cuando empieza

namespace {

bool acabaEn(const std::string& s, const std::string& fin) {
    return s.size() >= fin.size() && s.compare(s.size() - fin.size(), fin.size(), fin) == 0;
}

bool enLaZona(const Cinematica& c, const GridCoord& j) {
    return c.hayZona && j.x >= c.zonaDesde.x && j.x <= c.zonaHasta.x && j.y >= c.zonaDesde.y && j.y <= c.zonaHasta.y;
}

// El radio de la cinematica con disparador alrededor de su foco (con `vista`; sin ella no hay radio).
bool enElRadio(const Cinematica& c, const GridCoord& j, const VistaDelNivel& vista) {
    const int radio = c.disparador.radio >= 0 ? c.disparador.radio : radioDelAmbito(vista.ambito);
    return j.x >= 0 && distanciaChebyshev(j, c.puntoDeEntrada()) <= radio;
}

}  // namespace

const Cinematica* CatalogoCinematicas::laQueEmpieza(const std::string& nivel, const GridCoord& jugador,
                                                    const TieneFlag& flag, const VistaDelNivel* vista) const {
    for (const Cinematica& c : lista) {
        // 1. Las condiciones.
        if (flag(c.vista())) continue;
        if (!c.nivel.empty() && !acabaEn(nivel, c.nivel)) continue;
        if (std::any_of(c.requiere.begin(), c.requiere.end(), [&](const std::string& f) { return !flag(f); })) continue;
        if (std::any_of(c.sin.begin(), c.sin.end(), flag)) continue;
        const bool enZona = enLaZona(c, jugador);
        if (!c.disparador.hay) {  // la de siempre: su zona, o en cualquier sitio si no la tiene
            if (c.hayZona && !enZona) continue;
            return &c;
        }
        // 2. La zona pisada O dentro del radio del foco.
        bool porRadio = false;
        if (!enZona && c.hayFoco()) {
            const GridCoord foco = c.puntoDeEntrada();
            if (vista == nullptr) {
                if (jugador.x != foco.x || jugador.y != foco.y) continue;  // sin vista: EN el foco (el arnes)
            } else {
                if (!enElRadio(c, jugador, *vista)) continue;
                porRadio = true;
            }
        }
        // 3. Si vino por el radio, la linea de vision al foco (no en el interior pequeno, que dispara al entrar).
        if (porRadio) {
            const bool conVision = c.disparador.vision >= 0 ? c.disparador.vision == 1
                                                              : vista->ambito != AmbitoNivel::InteriorPequeno;
            if (conVision && !lineaDeVision(jugador, c.puntoDeEntrada(), vista->tapa)) continue;
        }
        return &c;  // 4. lleva + accion: pasosEfectivos
    }
    return nullptr;
}

// ---------------------------------------------------------------- rearmar (E5, D0026.4)

// Una zona repetible (`once: false`) se rearma cuando el jugador sale de ella, como los Touch del mapa
// (MapEventRunner): la flag «vista» que dejo su ultima escena es lo unico que la desarma, y se apaga al salir.
// «Dentro» es lo mismo que hace empezar: en otro nivel, no; en su zona o, con `vista`, en su radio, si; sin `vista`
// (arnes, explorador), la zona o EN el foco.
std::vector<std::string> CatalogoCinematicas::rearmadas(const std::string& nivel, const GridCoord& jugador, const TieneFlag& flag,
                                                        const VistaDelNivel* vista) const {
    std::vector<std::string> out;
    for (const Cinematica& c : lista) {
        if (!c.disparador.hay || c.disparador.once || !flag(c.vista())) continue;
        bool dentro = c.nivel.empty() || acabaEn(nivel, c.nivel);
        if (dentro) {
            const GridCoord foco = c.puntoDeEntrada();
            dentro = enLaZona(c, jugador) || (c.hayFoco() && (vista != nullptr ? enElRadio(c, jugador, *vista)
                                                                               : jugador.x == foco.x && jugador.y == foco.y));
        }
        if (!dentro) out.push_back(c.vista());
    }
    return out;
}

// ---------------------------------------------------------------- que se juega

std::vector<PasoCinematica> CatalogoCinematicas::pasosEfectivos(const Cinematica& c, const LlevaDelJugador* lleva) const {
    using Tipo = PasoCinematica::Tipo;
    if (!c.disparador.hay) return c.pasos;
    const Disparador& d = c.disparador;
    std::vector<PasoCinematica> out;
    if (lleva != nullptr && d.hayLleva) {
        const bool alli = lleva->jugador.x == d.lleva.x && lleva->jugador.y == d.lleva.y;
        std::vector<GridCoord> ruta;
        if (!alli && d.andando && lleva->ruta) ruta = lleva->ruta(d.lleva);
        if (!alli && !ruta.empty()) {  // andando: los pasos de siempre, con colisiones
            PasoCinematica anda;
            anda.tipo = Tipo::Anda;
            anda.actor = "jugador";
            anda.camino = ruta;
            out.push_back(anda);
        } else if (!alli) {  // fundido, o no hay camino (D0026.2)
            PasoCinematica f;
            f.tipo = Tipo::Funde;
            f.texto = "negro";
            f.valor = 1.0f;
            f.segundos = 0.4f;
            out.push_back(f);
            PasoCinematica coloca;
            coloca.tipo = Tipo::Coloca;
            coloca.actor = "jugador";
            coloca.celda = d.lleva;
            coloca.lleva = true;  // el jugador SI cambia de casilla
            out.push_back(coloca);
            f.valor = 0.0f;
            out.push_back(f);
        }
        if (!d.mira.empty()) {
            PasoCinematica mira;
            mira.tipo = Tipo::Mira;
            mira.actor = "jugador";
            mira.texto = d.mira;
            out.push_back(mira);
        }
    }
    if (d.accion == Disparador::Accion::Cinematica) {
        const Cinematica* otra = d.accionId.empty() || d.accionId == c.id ? &c : busca(d.accionId);
        if (otra != nullptr) out.insert(out.end(), otra->pasos.begin(), otra->pasos.end());
    } else {
        PasoCinematica p;
        p.tipo = d.accion == Disparador::Accion::Beat ? Tipo::Beat : d.accion == Disparador::Accion::Combate ? Tipo::Combate : Tipo::Flag;
        p.texto = d.accionId;
        if (p.tipo == Tipo::Combate) {  // E4: las flags del desenlace las pone el motor, con el id de la cinematica
            p.flagGana = "cine_" + c.id + "_ganado";
            p.flagPierde = "cine_" + c.id + "_perdido";
        }
        out.push_back(p);
    }
    return out;
}

Cinematica CatalogoCinematicas::efectiva(const Cinematica& c, const LlevaDelJugador* lleva) const {
    Cinematica e = c;
    e.pasos = pasosEfectivos(c, lleva);
    return e;
}

}  // namespace Game
