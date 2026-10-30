// PROLOGO-02 — ver include/Game/Cinematica.h.
#include "Game/Cinematica.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

namespace {

std::vector<std::string> cadenas(const JsonValue& v) {
    std::vector<std::string> out;
    if (v.isString()) out.push_back(v.asString());
    for (std::size_t i = 0; v.isArray() && i < v.size(); ++i) out.push_back(v[i].asString());
    return out;
}

bool leeCelda(const JsonValue& v, GridCoord& c) {
    if (!v.isArray() || v.size() != 2 || !v[0].isNumber() || !v[1].isNumber()) return false;
    c = GridCoord{v[0].asInt(), v[1].asInt()};
    return true;
}

const char* kRumbos[][2] = {{"arriba", "up"}, {"abajo", "down"}, {"izquierda", "left"}, {"derecha", "right"}};

Result<PasoCinematica> leePaso(const JsonValue& o, const std::string& donde) {
    using Tipo = PasoCinematica::Tipo;
    PasoCinematica p;
    auto error = [&](const std::string& que) { return Result<PasoCinematica>::Error(donde + ": " + que); };
    if (o.has("anda")) {
        p.tipo = Tipo::Anda;
        p.actor = o["anda"].asString();
        GridCoord c;
        if (leeCelda(o["a"], c)) p.camino.push_back(c);
        const JsonValue& por = o["por"];
        for (std::size_t i = 0; por.isArray() && i < por.size(); ++i) {
            if (!leeCelda(por[i], c)) return error("«por» es una lista de celdas [x, y]");
            p.camino.push_back(c);
        }
        if (p.camino.empty()) return error("«anda» necesita «a» o «por»");
        p.velocidad = static_cast<float>(o["velocidad"].asNumber(2.5));
        if (p.velocidad <= 0.0f) return error("«velocidad» tiene que ser positiva");
        p.espera = !o["sinEsperar"].asBool(false);
    } else if (o.has("coloca")) {
        p.tipo = Tipo::Coloca;
        p.actor = o["coloca"].asString();
        if (!leeCelda(o["en"], p.celda)) return error("«coloca» necesita «en»: [x, y]");
    } else if (o.has("mira")) {
        p.tipo = Tipo::Mira;
        p.actor = o["mira"].asString();
        p.texto = o["hacia"].asString();
        bool vale = false;
        for (const auto& r : kRumbos) vale = vale || p.texto == r[0];
        if (!vale) return error("«hacia» es arriba, abajo, izquierda o derecha");
    } else if (o.has("espera")) {
        p.tipo = Tipo::Espera;
        p.segundos = static_cast<float>(o["espera"].asNumber(0.0));
    } else if (o.has("beat")) {
        p.tipo = Tipo::Beat;
        p.texto = o["beat"].asString();
    } else if (o.has("flag")) {
        p.tipo = Tipo::Flag;
        p.texto = o["flag"].asString();
    } else if (o.has("nivel")) {
        p.tipo = Tipo::Nivel;
        p.texto = o["nivel"].asString();
        if (!leeCelda(o["en"], p.celda)) return error("«nivel» necesita «en»: [x, y]");
    } else if (o.has("patrulla")) {  // AV17-10: ida y vuelta por el camino
        p.tipo = Tipo::Anda;
        p.patrulla = true;
        p.actor = o["patrulla"].asString();
        GridCoord c;
        const JsonValue& por = o["por"];
        for (std::size_t i = 0; por.isArray() && i < por.size(); ++i) {
            if (!leeCelda(por[i], c)) return error("«por» es una lista de celdas [x, y]");
            p.camino.push_back(c);
        }
        if (p.camino.empty()) return error("«patrulla» necesita «por»");
        p.velocidad = static_cast<float>(o["velocidad"].asNumber(1.5));
        if (p.velocidad <= 0.0f) return error("«velocidad» tiene que ser positiva");
        p.veces = std::max(1, o["veces"].asInt(1));
        p.espera = !o["sinEsperar"].asBool(false);
    } else if (o.has("trepa") || o.has("baja")) {  // AV17-10
        p.tipo = o.has("trepa") ? Tipo::Trepa : Tipo::Baja;
        p.actor = o[o.has("trepa") ? "trepa" : "baja"].asString();
        p.segundos = static_cast<float>(o["segundos"].asNumber(1.5));
        p.altura = static_cast<float>(o["altura"].asNumber(1.5));
        if (p.segundos <= 0.0f || p.altura <= 0.0f) return error("«segundos» y «altura» tienen que ser positivos");
    } else if (o.has("arde") || o.has("apaga")) {  // AV17-10: el fuego de una casilla
        p.tipo = Tipo::Fuego;
        p.valor = o.has("arde") ? 1.0f : 0.0f;
        if (!leeCelda(o[o.has("arde") ? "arde" : "apaga"], p.celda)) return error("«arde» y «apaga» llevan una celda [x, y]");
    } else if (o.has("tumba") || o.has("levanta")) {  // AV17-10: herido o muerto, o de pie otra vez
        p.tipo = Tipo::Postura;
        p.actor = o[o.has("tumba") ? "tumba" : "levanta"].asString();
        p.texto = o.has("tumba") ? o["como"].asString("herido") : "de_pie";
        if (p.texto != "herido" && p.texto != "muerto") {
            if (o.has("tumba")) return error("«como» es herido o muerto");
        }
    } else if (o.has("emocion")) {  // AV17-10: un bocadillo con un icono
        p.tipo = Tipo::Emocion;
        p.actor = o["emocion"].asString();
        p.texto = o["icono"].asString();
        p.segundos = static_cast<float>(o["segundos"].asNumber(1.5));
        p.espera = o["aguarda"].asBool(false);
        if (p.texto.empty()) return error("«emocion» necesita «icono»");
    } else if (o.has("funde")) {
        p.tipo = Tipo::Funde;
        p.valor = std::clamp(static_cast<float>(o["funde"].asNumber(0.0)), 0.0f, 1.0f);
        p.segundos = static_cast<float>(o["segundos"].asNumber(0.8));
        p.texto = o["color"].asString("blanco");
        if (p.texto != "blanco" && p.texto != "negro") return error("«color» es blanco o negro");
    } else {
        return error("orden desconocida (anda, patrulla, coloca, mira, espera, beat, flag, nivel, funde, trepa, baja, arde, apaga, tumba, levanta o emocion)");
    }
    if (o.has("desfase")) {  // PROLOGO-02: anda y coloca
        const JsonValue& d = o["desfase"];
        if ((p.tipo != Tipo::Anda && p.tipo != Tipo::Coloca) || !d.isArray() || d.size() != 2)
            return error("«desfase» va en «anda» o «coloca», como [dx, dy] en celdas");
        p.hayDesfase = true;
        p.desfaseX = static_cast<float>(d[0].asNumber(0.0));
        p.desfaseY = static_cast<float>(d[1].asNumber(0.0));
    }
    if ((p.tipo == Tipo::Beat || p.tipo == Tipo::Flag || p.tipo == Tipo::Nivel) && p.texto.empty())
        return error("le falta el valor");
    if ((p.tipo == Tipo::Anda || p.tipo == Tipo::Coloca || p.tipo == Tipo::Mira || p.tipo == Tipo::Trepa ||
         p.tipo == Tipo::Baja || p.tipo == Tipo::Postura || p.tipo == Tipo::Emocion) && p.actor.empty())
        return error("le falta a quién");
    return Result<PasoCinematica>::Ok(p);
}

bool acabaEn(const std::string& s, const std::string& fin) {
    return s.size() >= fin.size() && s.compare(s.size() - fin.size(), fin.size(), fin) == 0;
}

std::string rumboIngles(const std::string& rumbo) {
    for (const auto& r : kRumbos)
        if (rumbo == r[0]) return r[1];
    return "down";
}

}  // namespace

std::vector<GridCoord> celdasDelPaseo(const GridCoord& desde, const std::vector<GridCoord>& camino) {
    std::vector<GridCoord> out;
    GridCoord c = desde;
    for (const GridCoord& punto : camino) {
        while (c.x != punto.x) {
            c.x += punto.x > c.x ? 1 : -1;
            out.push_back(c);
        }
        while (c.y != punto.y) {
            c.y += punto.y > c.y ? 1 : -1;
            out.push_back(c);
        }
    }
    return out;
}

std::string animacionDePaso(int dx, int dy) {
    return dy < 0 ? "walk_up" : dy > 0 ? "walk_down" : dx < 0 ? "walk_left" : "walk_right";
}

Result<CatalogoCinematicas> CatalogoCinematicas::lee(const std::string& textoJson) {
    auto leido = JsonValue::parse(textoJson);
    if (!leido.isOk()) return Result<CatalogoCinematicas>::Error("cinematicas: " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    CatalogoCinematicas cat;
    const JsonValue& esp = raiz["espectro"];
    cat.espectro.flag = esp["flag"].asString();
    cat.espectro.jugador = esp["jugador"].asBool(true);
    cat.espectro.objetos = cadenas(esp["objetos"]);
    const JsonValue& tinte = esp["tinte"];
    if (tinte.isArray() && tinte.size() == 4)
        cat.espectro.tinte = Vector4{static_cast<float>(tinte[0].asNumber()), static_cast<float>(tinte[1].asNumber()),
                                     static_cast<float>(tinte[2].asNumber()), static_cast<float>(tinte[3].asNumber())};
    const JsonValue& aps = raiz["apariciones"];
    auto condicion = [](const JsonValue& v) {
        CondicionFlags c;
        c.todas = cadenas(v["todas"]);
        c.alguna = cadenas(v["alguna"]);
        c.ninguna = cadenas(v["ninguna"]);
        return c;
    };
    for (std::size_t i = 0; aps.isArray() && i < aps.size(); ++i) {
        Aparicion a{aps[i]["objeto"].asString(), aps[i]["si"].asString(), aps[i]["sin"].asString(), {}, {}};
        a.visibleSi = condicion(aps[i]["visibleSi"]);
        a.ocultoSi = condicion(aps[i]["ocultoSi"]);
        if (a.objeto.empty()) return Result<CatalogoCinematicas>::Error("cinematicas: una aparicion sin «objeto»");
        cat.apariciones.push_back(std::move(a));
    }
    for (const auto& [nivel, color] : raiz["fondos"].objectValues()) {
        if (!color.isArray() || color.size() < 3)
            return Result<CatalogoCinematicas>::Error("cinematicas: el fondo de «" + nivel + "» es [r, g, b]");
        cat.fondos.push_back({nivel, Vector4{static_cast<float>(color[0].asNumber()), static_cast<float>(color[1].asNumber()),
                                             static_cast<float>(color[2].asNumber()), 1.0f}});
    }
    const JsonValue& titulo = raiz["titulo"];
    cat.titulo.nivel = titulo["nivel"].asString();
    cat.titulo.conNivelInicial = titulo["conNivelInicial"].asString();
    cat.titulo.velocidad = static_cast<float>(titulo["velocidad"].asNumber(0.6));
    cat.titulo.zoom = static_cast<float>(titulo["zoom"].asNumber(1.0));
    for (std::size_t i = 0; titulo["camino"].isArray() && i < titulo["camino"].size(); ++i) {
        GridCoord p;
        if (!leeCelda(titulo["camino"][i], p))
            return Result<CatalogoCinematicas>::Error("cinematicas: el camino del titulo es una lista de celdas [x, y]");
        cat.titulo.camino.push_back(p);
    }
    const JsonValue& lista = raiz["cinematicas"];
    std::set<std::string> ids;
    for (std::size_t i = 0; lista.isArray() && i < lista.size(); ++i) {
        const JsonValue& o = lista[i];
        Cinematica c;
        c.id = o["id"].asString();
        if (c.id.empty()) return Result<CatalogoCinematicas>::Error("cinematicas: la " + std::to_string(i) + " no tiene id");
        if (!ids.insert(c.id).second) return Result<CatalogoCinematicas>::Error("cinematicas: «" + c.id + "» repetida");
        c.nivel = o["nivel"].asString();
        c.requiere = cadenas(o["requiere"]);
        c.sin = cadenas(o["sin"]);
        c.alTerminar = cadenas(o["alTerminar"]);
        const JsonValue& zona = o["zona"];
        if (!zona.isNull()) {
            if (!zona.isArray() || zona.size() != 4)
                return Result<CatalogoCinematicas>::Error("cinematicas: «" + c.id + "»: «zona» es [x0, y0, x1, y1]");
            c.hayZona = true;
            c.zonaDesde = GridCoord{std::min(zona[0].asInt(), zona[2].asInt()), std::min(zona[1].asInt(), zona[3].asInt())};
            c.zonaHasta = GridCoord{std::max(zona[0].asInt(), zona[2].asInt()), std::max(zona[1].asInt(), zona[3].asInt())};
        }
        const JsonValue& pasos = o["pasos"];
        for (std::size_t k = 0; pasos.isArray() && k < pasos.size(); ++k) {
            auto paso = leePaso(pasos[k], "cinematicas: «" + c.id + "», paso " + std::to_string(k));
            if (!paso.isOk()) return Result<CatalogoCinematicas>::Error(paso.errorMessage());
            c.pasos.push_back(paso.value());
        }
        cat.lista.push_back(std::move(c));
    }
    return Result<CatalogoCinematicas>::Ok(std::move(cat));
}

Result<CatalogoCinematicas> CatalogoCinematicas::carga(const std::string& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) return Result<CatalogoCinematicas>::Ok(CatalogoCinematicas{});
    std::stringstream texto;
    texto << entrada.rdbuf();
    return lee(texto.str());
}

const Cinematica* CatalogoCinematicas::busca(const std::string& id) const {
    for (const Cinematica& c : lista)
        if (c.id == id) return &c;
    return nullptr;
}

const Cinematica* CatalogoCinematicas::laQueEmpieza(const std::string& nivel, const GridCoord& jugador,
                                                    const TieneFlag& flag) const {
    for (const Cinematica& c : lista) {
        if (flag(c.vista())) continue;
        if (!c.nivel.empty() && !acabaEn(nivel, c.nivel)) continue;
        if (std::any_of(c.requiere.begin(), c.requiere.end(), [&](const std::string& f) { return !flag(f); })) continue;
        if (std::any_of(c.sin.begin(), c.sin.end(), flag)) continue;
        if (c.hayZona && (jugador.x < c.zonaDesde.x || jugador.x > c.zonaHasta.x || jugador.y < c.zonaDesde.y ||
                          jugador.y > c.zonaHasta.y))
            continue;
        return &c;
    }
    return nullptr;
}

bool CondicionFlags::cumple(const TieneFlag& flag) const {
    if (std::any_of(todas.begin(), todas.end(), [&](const std::string& f) { return !flag(f); })) return false;
    if (!alguna.empty() && std::none_of(alguna.begin(), alguna.end(), flag)) return false;
    return std::none_of(ninguna.begin(), ninguna.end(), flag);
}

bool CatalogoCinematicas::visible(const std::string& objeto, const TieneFlag& flag) const {
    for (const Aparicion& a : apariciones) {
        if (a.objeto != objeto) continue;
        if (!a.siFlag.empty() && !flag(a.siFlag)) return false;
        if (!a.sinFlag.empty() && flag(a.sinFlag)) return false;
        if (!a.visibleSi.vacia() && !a.visibleSi.cumple(flag)) return false;
        if (!a.ocultoSi.vacia() && a.ocultoSi.cumple(flag)) return false;
    }
    return true;
}

bool CatalogoCinematicas::esEspectro(const std::string& objeto, const TieneFlag& flag) const {
    if (espectro.flag.empty() || !flag(espectro.flag)) return false;
    return std::find(espectro.objetos.begin(), espectro.objetos.end(), objeto) != espectro.objetos.end();
}

bool CatalogoCinematicas::jugadorEspectro(const TieneFlag& flag) const {
    return espectro.jugador && !espectro.flag.empty() && flag(espectro.flag);
}

void FondoTitulo::punto(float celdas, float& x, float& y) const {
    x = y = 0.0f;
    if (camino.empty()) return;
    float total = 0.0f;
    for (std::size_t i = 0; i < camino.size(); ++i) {
        const GridCoord& a = camino[i];
        const GridCoord& b = camino[(i + 1) % camino.size()];
        total += static_cast<float>(std::abs(b.x - a.x) + std::abs(b.y - a.y));
    }
    x = static_cast<float>(camino[0].x);
    y = static_cast<float>(camino[0].y);
    if (total <= 0.0f) return;
    float resto = std::fmod(std::max(0.0f, celdas), total);
    for (std::size_t i = 0; i < camino.size(); ++i) {
        const GridCoord& a = camino[i];
        const GridCoord& b = camino[(i + 1) % camino.size()];
        const float tramo = static_cast<float>(std::abs(b.x - a.x) + std::abs(b.y - a.y));
        if (resto <= tramo && tramo > 0.0f) {
            const float k = resto / tramo;
            x = static_cast<float>(a.x) + static_cast<float>(b.x - a.x) * k;
            y = static_cast<float>(a.y) + static_cast<float>(b.y - a.y) * k;
            return;
        }
        resto -= tramo;
    }
}

bool CatalogoCinematicas::afecta(const std::string& objeto) const {
    return std::any_of(apariciones.begin(), apariciones.end(), [&](const Aparicion& a) { return a.objeto == objeto; }) ||
           std::find(espectro.objetos.begin(), espectro.objetos.end(), objeto) != espectro.objetos.end();
}

// ---------------------------------------------------------------- el reproductor

void ReproductorCinematica::empieza(const Cinematica& c) {
    m_c = c;
    m_activa = true;
    m_paso = 0;
    m_enPaso = false;
    m_reloj = 0.0f;
    m_paseos.clear();
    m_desfases.clear();
    m_trepa.clear();
}

void ReproductorCinematica::avanza(float dt, EscenarioCinematica& e) {
    if (!m_activa) {
        // El velo se va solo: una cinemática que acaba en blanco no deja la pantalla tapada.
        m_velo = std::max(0.0f, m_velo - dt * 1.5f);
        return;
    }
    avanzaPaseos(dt, e);
    while (m_paso < m_c.pasos.size()) {
        const PasoCinematica& p = m_c.pasos[m_paso];
        bool hecho;
        if (!m_enPaso) {
            m_enPaso = true;
            m_reloj = 0.0f;
            hecho = empiezaPaso(p, e);
        } else {
            hecho = sigueEnPaso(p, dt, e);
        }
        if (!hecho) return;
        ++m_paso;
        m_enPaso = false;
    }
    if (m_paseos.empty()) termina(e);
}

bool ReproductorCinematica::empiezaPaso(const PasoCinematica& p, EscenarioCinematica& e) {
    using Tipo = PasoCinematica::Tipo;
    auto suelta = [&](const std::string& actor) {
        m_paseos.erase(std::remove_if(m_paseos.begin(), m_paseos.end(),
                                      [&](const Paseo& q) { return q.actor == actor; }),
                       m_paseos.end());
    };
    switch (p.tipo) {
        case Tipo::Anda: {
            GridCoord desde;
            if (!e.posicion(p.actor, desde)) return true;  // no está en este nivel: nada que andar
            suelta(p.actor);
            if (p.hayDesfase) m_desfases[p.actor] = {p.desfaseX, p.desfaseY};
            Paseo q;
            q.actor = p.actor;
            std::vector<GridCoord> camino = p.camino;
            if (p.patrulla) {  // ida, vuelta por el mismo camino hasta donde empezo, y asi `veces`
                std::vector<GridCoord> vuelta(p.camino.rbegin() + 1, p.camino.rend());
                vuelta.push_back(desde);
                camino.clear();
                for (int v = 0; v < p.veces; ++v) {
                    camino.insert(camino.end(), p.camino.begin(), p.camino.end());
                    camino.insert(camino.end(), vuelta.begin(), vuelta.end());
                }
            }
            q.celdas = celdasDelPaseo(desde, camino);
            q.desde = desde;
            q.velocidad = p.velocidad;
            q.t = 1.0f;  // el jugador da el primer paso ya
            m_paseos.push_back(q);
            avanzaPaseos(0.0f, e);
            return !p.espera;
        }
        case Tipo::Coloca:
            suelta(p.actor);
            if (p.hayDesfase) m_desfases[p.actor] = {p.desfaseX, p.desfaseY};
            pon(e, p.actor, p.celda, 0.0f, 0.0f, "idle");
            return true;
        case Tipo::Mira: {
            GridCoord c;
            if (e.posicion(p.actor, c)) pon(e, p.actor, c, 0.0f, 0.0f, "idle_" + rumboIngles(p.texto));
            return true;
        }
        case Tipo::Espera:
            return p.segundos <= 0.0f;
        case Tipo::Beat:
            e.enciende(p.texto);
            e.avanzaHistoria();
            return !e.hablando();
        case Tipo::Flag:
            e.enciende(p.texto);
            return true;
        case Tipo::Nivel:
            m_paseos.clear();  // los de este nivel se quedan donde estén
            quitaDesfaseDelJugador(e);
            m_desfases.clear();
            m_trepa.clear();
            e.cargaNivel(p.texto, p.celda);
            return false;
        case Tipo::Trepa:
        case Tipo::Baja: {
            GridCoord c;
            if (!e.posicion(p.actor, c)) return true;
            suelta(p.actor);
            m_trepaDesde = m_trepa[p.actor];
            m_trepaHasta = p.tipo == Tipo::Trepa ? m_trepaDesde + p.altura : std::max(0.0f, m_trepaDesde - p.altura);
            pon(e, p.actor, c, 0.0f, 0.0f, p.tipo == Tipo::Trepa ? "climb" : "climb_down");
            return false;
        }
        case Tipo::Fuego:
            e.fuego(p.celda, p.valor > 0.5f);
            return true;
        case Tipo::Postura:
            e.postura(p.actor, p.texto);
            return true;
        case Tipo::Emocion:
            e.emocion(p.actor, p.texto, p.segundos);
            return !p.espera || p.segundos <= 0.0f;
        case Tipo::Funde:
            m_veloDesde = m_velo;
            m_veloBlanco = p.texto != "negro";
            if (p.segundos <= 0.0f) {
                m_velo = p.valor;
                return true;
            }
            return false;
    }
    return true;
}

bool ReproductorCinematica::sigueEnPaso(const PasoCinematica& p, float dt, EscenarioCinematica& e) {
    using Tipo = PasoCinematica::Tipo;
    m_reloj += dt;
    switch (p.tipo) {
        case Tipo::Anda:
            return std::none_of(m_paseos.begin(), m_paseos.end(), [&](const Paseo& q) { return q.actor == p.actor; });
        case Tipo::Espera:
            return m_reloj >= p.segundos;
        case Tipo::Trepa:
        case Tipo::Baja: {
            const float k = std::min(1.0f, m_reloj / p.segundos);
            m_trepa[p.actor] = m_trepaDesde + (m_trepaHasta - m_trepaDesde) * k;
            GridCoord c;
            if (e.posicion(p.actor, c)) {
                const bool sube = p.tipo == Tipo::Trepa;
                pon(e, p.actor, c, 0.0f, 0.0f, k >= 1.0f ? (sube ? "idle_up" : "idle_down") : (sube ? "climb" : "climb_down"));
            }
            if (k >= 1.0f && m_trepa[p.actor] <= 0.0f) m_trepa.erase(p.actor);
            return k >= 1.0f;
        }
        case Tipo::Emocion:
            return m_reloj >= p.segundos;
        case Tipo::Beat:
            return !e.hablando();
        case Tipo::Nivel:
            return !e.cargandoNivel();
        case Tipo::Funde: {
            const float k = std::min(1.0f, m_reloj / p.segundos);
            m_velo = m_veloDesde + (p.valor - m_veloDesde) * k;
            return k >= 1.0f;
        }
        default:
            return true;
    }
}

void ReproductorCinematica::avanzaPaseos(float dt, EscenarioCinematica& e) {
    for (auto it = m_paseos.begin(); it != m_paseos.end();) {
        Paseo& q = *it;
        GridCoord aqui;
        if (!e.posicion(q.actor, aqui)) {
            it = m_paseos.erase(it);
            continue;
        }
        if (q.actor == "jugador") {
            // El jugador anda con sus reglas (colisiones) pero a la manera de un PNJ:
            // cada celda dura 1/velocidad y su dibujo la recorre entera, de corrido,
            // sin parar entre una y otra (AV16-05: antes daba el paso en 160 ms y
            // esperaba el resto, y se veía a tirones). El resto de tiempo de un paso
            // pasa al siguiente. Si algo le cierra el paso, se queda donde está.
            q.t += dt * q.velocidad;
            bool fuera = false;
            while (!fuera && q.t >= 1.0f) {
                if (q.celdas.empty()) {
                    fuera = true;
                } else {
                    const GridCoord sig = q.celdas.front();
                    q.celdas.erase(q.celdas.begin());
                    q.t -= 1.0f;
                    e.ritmoDelJugador(1.0f / q.velocidad);
                    fuera = !e.pasoDelJugador(sig.x - aqui.x, sig.y - aqui.y);
                    aqui = sig;
                }
            }
            it = fuera ? m_paseos.erase(it) : it + 1;
            continue;
        }
        // Un PNJ: en el mundo ya está en la celda a la que va; el dibujo se
        // queda atrás (el desfase) y se acerca hasta llegar.
        float resto = dt * q.velocidad;
        bool fuera = false;
        while (true) {
            if (!q.andando) {
                if (q.celdas.empty()) {
                    fuera = true;
                    break;
                }
                q.desde = aqui;
                q.andando = true;
                q.t = 0.0f;
            }
            const GridCoord hacia = q.celdas.front();
            const std::string anim = animacionDePaso(hacia.x - q.desde.x, hacia.y - q.desde.y);
            q.t += resto;
            if (q.t < 1.0f) {
                const float falta = 1.0f - q.t;
                pon(e, q.actor, hacia, static_cast<float>(q.desde.x - hacia.x) * falta,
                      static_cast<float>(q.desde.y - hacia.y) * falta, anim);
                break;
            }
            resto = q.t - 1.0f;
            aqui = hacia;
            q.celdas.erase(q.celdas.begin());
            q.andando = false;
            if (q.celdas.empty()) {
                std::string quieto = anim;
                quieto.replace(0, 4, "idle");  // walk_down -> idle_down
                pon(e, q.actor, hacia, 0.0f, 0.0f, quieto);
                fuera = true;
                break;
            }
        }
        it = fuera ? m_paseos.erase(it) : it + 1;
    }
}

void ReproductorCinematica::pon(EscenarioCinematica& e, const std::string& actor, const GridCoord& c, float dx,
                                float dy, const std::string& animacion) {
    const auto it = m_desfases.find(actor);
    const bool hay = it != m_desfases.end();
    // AV17-10: subido a `altura` celdas = el dibujo sube altura * alto de casilla (dx = dy = -altura).
    const auto tr = m_trepa.find(actor);
    const float alto = tr != m_trepa.end() ? -tr->second : 0.0f;
    e.pon(actor, c, dx + alto + (hay ? it->second.first : 0.0f), dy + alto + (hay ? it->second.second : 0.0f), animacion);
}

void ReproductorCinematica::quitaDesfaseDelJugador(EscenarioCinematica& e) {
    const bool subido = m_trepa.erase("jugador") > 0;
    if (m_desfases.erase("jugador") == 0 && !subido) return;
    if (GridCoord c; e.posicion("jugador", c)) e.pon("jugador", c, 0.0f, 0.0f, "");
}

void ReproductorCinematica::termina(EscenarioCinematica& e) {
    quitaDesfaseDelJugador(e);
    m_desfases.clear();
    m_trepa.clear();
    m_activa = false;
    e.enciende(m_c.vista());
    for (const std::string& f : m_c.alTerminar) e.enciende(f);
    e.avanzaHistoria();
}

}  // namespace Game
