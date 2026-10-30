// PNJ-02 — Los PNJ se mueven y hacen cosas (ver include/Game/RutinasPnj.h). Aqui, lo que
// cambia cada fotograma; el catalogo y la pose, en RutinasPnjCatalogo.cpp.
#include "Game/RutinasPnj.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>

#include "Game/Mascotes.h"
#include "Level/ObjectCatalog.h"
#include "World/World.h"

namespace Game {

namespace {

// --- El azar de cada PNJ (splitmix64): misma semilla, mismo paseo. ---
std::uint64_t siguiente(std::uint64_t& s) {
    s += 0x9E3779B97F4A7C15ull;
    std::uint64_t z = s;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
float azar01(std::uint64_t& s) { return static_cast<float>(siguiente(s) >> 40) / 16777216.0f; }
float entre(std::uint64_t& s, float a, float b) { return a + (b - a) * azar01(s); }
int entreEnteros(std::uint64_t& s, int a, int b) { return a + static_cast<int>(siguiente(s) % static_cast<std::uint64_t>(b - a + 1)); }

std::uint32_t fnv(const std::string& texto, std::uint32_t h = 2166136261u) {
    for (const char c : texto) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    return h;
}

int lejania(const GridCoord& a, const GridCoord& b) { return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)); }
bool igual(const GridCoord& a, const GridCoord& b) { return a.x == b.x && a.y == b.y; }

constexpr int kDx[4] = {0, 1, 0, -1};  // por rumbo: arriba, derecha, abajo, izquierda
constexpr int kDy[4] = {-1, 0, 1, 0};

}  // namespace

// =====================================================================
// Montar
// =====================================================================
void RutinasPnj::monta(Engine::World& mundo, const ObjectCatalog& objetos, const CatalogoRutinasPnj& catalogo,
                       const std::vector<GridCoord>& carriles, std::uint32_t semilla) {
    m_pnjs.clear();
    m_prohibidas.clear();
    m_carriles.clear();
    for (const GridCoord& c : carriles) m_carriles.insert({c.x, c.y});
    mundo.eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& ent) {
        if (ent.kind != Engine::EntityKind::Object || ent.object.targetLevel.empty()) return;
        const GridCoord p = ent.object.position;
        m_prohibidas.insert({p.x, p.y});  // la puerta y la casilla de al lado (la de delante)
        for (int k = 0; k < 4; ++k) m_prohibidas.insert({p.x + kDx[k], p.y + kDy[k]});
    });
    mundo.eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& ent) {
        if (ent.kind != Engine::EntityKind::Object) return;
        const std::string& id = ent.object.objectId;
        const ObjectDefinition* def = objetos.find(id);
        if (def == nullptr || def->category != ObjectCategory::Npc || id.rfind("acompanante_", 0) == 0) return;
        EstadoPnj e;
        e.handle = h;
        e.objectId = id;
        e.def = catalogo.para(id);
        e.origen = e.casilla = e.desde = ent.object.position;
        e.azar = (static_cast<std::uint64_t>(fnv(id, semilla)) << 32) ^
                 static_cast<std::uint64_t>((e.origen.x * 73856093) ^ (e.origen.y * 19349663));
        e.rumboCalle = -1;  // se calcula en el primer avanza (hace falta saber que esta libre)
        e.reloj = entre(e.azar, 0.0f, 2.4f);
        e.espera = entre(e.azar, 0.5f, e.def.pausaMax);
        e.hastaActividad = entre(e.azar, 0.5f, e.def.cadaMax);
        e.hastaFrase = entre(e.azar, 2.0f, 10.0f);
        m_pnjs.push_back(std::move(e));
    });
    m_activa = true;
}

void RutinasPnj::montaNivel(Engine::World& mundo, const ObjectCatalog& objetos, const std::string& rutaNivel) {
    monta(mundo, objetos, CatalogoRutinasPnj::delJuego(), leeCarriles(rutaNivel), fnv(rutaNivel));
}

// =====================================================================
// Consultas
// =====================================================================
EstadoPnj* RutinasPnj::de(Engine::EntityHandle quien) {
    for (EstadoPnj& e : m_pnjs)
        if (e.handle == quien) return &e;
    return nullptr;
}

const EstadoPnj* RutinasPnj::de(Engine::EntityHandle quien) const {
    for (const EstadoPnj& e : m_pnjs)
        if (e.handle == quien) return &e;
    return nullptr;
}

GridCoord RutinasPnj::origenDe(Engine::EntityHandle quien, const GridCoord& siNo) const {
    const EstadoPnj* e = de(quien);
    return e != nullptr ? e->origen : siNo;
}

// =====================================================================
// Por donde puede ir
// =====================================================================
bool RutinasPnj::puedePisar(const EstadoPnj& e, const GridCoord& c, const MundoPnj& m, const GridCoord* desde) const {
    if (igual(c, e.casilla)) return true;
    if (prohibida(c) || igual(c, m.jugador)) return false;
    if (m.libre && !m.libre(c)) return false;
    if (m.pasa && !m.pasa(desde != nullptr ? *desde : e.casilla, c)) return false;
    for (const EstadoPnj& o : m_pnjs)
        if (&o != &e && igual(o.casilla, c)) return false;
    if (m_mascotas != nullptr) {
        for (const Mascota& masc : m_mascotas->mascotes()) {
            if (igual(Mascotes::cella(masc), c)) return false;
            if (!masc.cami.empty() && igual(masc.cami.front(), c)) return false;
        }
    }
    return true;
}

bool RutinasPnj::puedePararse(const EstadoPnj& e, const GridCoord& c, const MundoPnj& m) const {
    if (igual(c, e.origen)) return puedePisar(e, c, m);  // su sitio vale siempre
    return !carril(c) && puedePisar(e, c, m);
}

// Ruta corta (anchura) hasta `meta` sin salir de la caja de su zona (mas `margen`).
bool RutinasPnj::vaA(EstadoPnj& e, const GridCoord& meta, int margen, const MundoPnj& m) {
    if (igual(meta, e.casilla) || !puedePararse(e, meta, m)) return false;
    const int r = e.def.rutina == RutinaPnj::IrYVolver ? 0 : e.def.radio;
    const int x0 = std::min({e.origen.x - r, meta.x, e.casilla.x}) - margen;
    const int x1 = std::max({e.origen.x + r, meta.x, e.casilla.x}) + margen;
    const int y0 = std::min({e.origen.y - r, meta.y, e.casilla.y}) - margen;
    const int y1 = std::max({e.origen.y + r, meta.y, e.casilla.y}) + margen;
    const int ancho = x1 - x0 + 1, alto = y1 - y0 + 1;
    if (ancho <= 0 || alto <= 0 || ancho * alto > 1600) return false;
    std::vector<int> padre(static_cast<std::size_t>(ancho * alto), -2);
    auto indice = [&](const GridCoord& c) { return (c.y - y0) * ancho + (c.x - x0); };
    std::deque<GridCoord> cola{e.casilla};
    padre[static_cast<std::size_t>(indice(e.casilla))] = -1;
    while (!cola.empty()) {
        const GridCoord c = cola.front();
        cola.pop_front();
        if (igual(c, meta)) break;
        for (int k = 0; k < 4; ++k) {
            const GridCoord n{c.x + kDx[k], c.y + kDy[k]};
            if (n.x < x0 || n.x > x1 || n.y < y0 || n.y > y1) continue;
            int& p = padre[static_cast<std::size_t>(indice(n))];
            if (p != -2 || !puedePisar(e, n, m, &c)) continue;
            p = indice(c);
            cola.push_back(n);
        }
    }
    if (padre[static_cast<std::size_t>(indice(meta))] == -2) return false;
    e.ruta.clear();
    for (int i = indice(meta); i != indice(e.casilla); i = padre[static_cast<std::size_t>(i)])
        e.ruta.push_back(GridCoord{x0 + i % ancho, y0 + i / ancho});
    std::reverse(e.ruta.begin(), e.ruta.end());
    return !e.ruta.empty();
}

// Deambular: a veces vuelve a su sitio; si no, a una casilla libre de su radio.
bool RutinasPnj::paseo(EstadoPnj& e, const MundoPnj& m) {
    const bool lejos = lejania(e.casilla, e.origen) > e.def.radio;
    if (!igual(e.casilla, e.origen) && (lejos || azar01(e.azar) < 0.3f) && vaA(e, e.origen, 1, m)) return true;
    if (e.def.radio <= 0 || prohibida(e.origen)) return false;
    for (int intento = 0; intento < 10; ++intento) {
        const GridCoord c{e.origen.x + entreEnteros(e.azar, -e.def.radio, e.def.radio),
                          e.origen.y + entreEnteros(e.azar, -e.def.radio, e.def.radio)};
        if (!igual(c, e.casilla) && vaA(e, c, 0, m)) return true;
    }
    return false;
}

const EstadoPnj* RutinasPnj::vecino(const EstadoPnj& e) const {
    const EstadoPnj* mejor = nullptr;
    for (const EstadoPnj& o : m_pnjs) {
        if (&o == &e || lejania(o.casilla, e.casilla) > kDistanciaCharla) continue;
        if (mejor == nullptr || lejania(o.casilla, e.casilla) < lejania(mejor->casilla, e.casilla)) mejor = &o;
    }
    return mejor;
}

// =====================================================================
// Decidir y mirar
// =====================================================================
void RutinasPnj::decide(EstadoPnj& e, const MundoPnj& m) {
    e.espera = entre(e.azar, e.def.pausaMin, e.def.pausaMax);
    const bool clavado = prohibida(e.origen);  // nacio junto a una puerta: no se mueve de ahi
    const bool fuera = !igual(e.casilla, e.origen);
    switch (e.def.rutina) {
        case RutinaPnj::Quieto:
            if (fuera) vaA(e, e.origen, 2, m);
            break;
        case RutinaPnj::Vigilar:
            if (fuera) vaA(e, e.origen, 2, m);
            else e.rumbo = (e.rumbo + 1) % 4;  // de cara en cara
            break;
        case RutinaPnj::Deambular:
            if (!clavado) paseo(e, m);
            break;
        case RutinaPnj::Actividad:
            if (!clavado && e.def.radio > 0 && azar01(e.azar) < 0.5f) paseo(e, m);
            else if (fuera) vaA(e, e.origen, 1, m);
            break;
        case RutinaPnj::IrYVolver: {
            if (clavado) break;
            const GridCoord meta = e.haciaDestino ? GridCoord{e.origen.x + e.def.destino.x, e.origen.y + e.def.destino.y}
                                                  : e.origen;
            if (vaA(e, meta, 2, m) || igual(e.casilla, meta)) e.haciaDestino = !e.haciaDestino;
            break;
        }
        case RutinaPnj::Charlar: {
            const EstadoPnj* otro = vecino(e);
            if (otro == nullptr) {
                if (!clavado) paseo(e, m);
                break;
            }
            e.rumbo = rumboHacia(e.casilla, otro->casilla);  // se ponen de cara
            EstadoPnj* el = de(otro->handle);
            if (el != nullptr && !el->andando() && el->ruta.empty() && el->hablando <= 0.0f) {
                el->rumbo = rumboHacia(el->casilla, e.casilla);
                el->espera = std::max(el->espera, e.espera);
            }
            break;
        }
    }
    if (e.def.mira == MiraPnj::Giro && e.ruta.empty() && e.def.rutina != RutinaPnj::Vigilar) e.rumbo = (e.rumbo + 1) % 4;
}

void RutinasPnj::mira(EstadoPnj& e, const MundoPnj& m) {
    switch (e.def.mira) {
        case MiraPnj::Jugador:
            if (lejania(e.casilla, m.jugador) <= kDistanciaMirar) e.rumbo = rumboHacia(e.casilla, m.jugador);
            break;
        case MiraPnj::Calle:
            if (e.rumboCalle >= 0) e.rumbo = e.rumboCalle;
            break;
        case MiraPnj::Vecino:
            if (const EstadoPnj* o = vecino(e)) e.rumbo = rumboHacia(e.casilla, o->casilla);
            break;
        case MiraPnj::Fijo:
            e.rumbo = e.def.rumboFijo;
            break;
        case MiraPnj::Giro:
            break;  // gira en decide()
    }
}

// Las frases: de vez en cuando, y solo si estas a 4 casillas o menos.
void RutinasPnj::dice(EstadoPnj& e, const MundoPnj& m, float dt) {
    if (e.def.frases.empty()) return;
    const int d = lejania(e.casilla, m.jugador);
    if (e.fraseQueda > 0.0f) {
        e.fraseQueda -= dt;
        if (e.fraseQueda <= 0.0f || d > kDistanciaFrase || m.parado) e.frase.clear(), e.fraseQueda = 0.0f;
        return;
    }
    if (m.parado || e.hablando > 0.0f) return;
    e.hastaFrase -= dt;
    if (e.hastaFrase > 0.0f) return;
    if (d > kDistanciaFrase) {
        e.hastaFrase = 1.0f;  // ahora no te oye: se prueba en un rato
        return;
    }
    e.frase = e.def.frases[siguiente(e.azar) % e.def.frases.size()];
    e.fraseQueda = kFrase;
    e.hastaFrase = entre(e.azar, 8.0f, 16.0f);
}

void RutinasPnj::habla(Engine::EntityHandle quien, const GridCoord& jugador) {
    EstadoPnj* e = de(quien);
    if (e == nullptr) return;
    e->ruta.clear();
    e->actividad = 0.0f;
    e->frase.clear();
    e->fraseQueda = 0.0f;
    e->hablando = 2.0f;  // y aun te mira un poco despues de cerrar el dialogo
    e->rumbo = rumboHacia(e->casilla, jugador);
}

// =====================================================================
// Cada fotograma
// =====================================================================
void RutinasPnj::avanza(float dt, Engine::World& mundo, const MundoPnj& m) {
    if (!m_activa || m_pausa || dt <= 0.0f) return;
    for (EstadoPnj& e : m_pnjs) {
        Engine::WorldEntity* ent = mundo.get(e.handle);
        if (ent == nullptr || !ent->enabled || ent->kind != Engine::EntityKind::Object) continue;
        if (!igual(ent->object.position, e.casilla)) {
            // Lo ha movido otro (una cinematica): su sitio es ahora ese.
            e.origen = e.casilla = e.desde = ent->object.position;
            e.ruta.clear();
            e.progreso = 1.0f;
        }
        e.reloj += dt;
        if (e.rumboCalle < 0) {
            int mejor = -1, largo = -1;
            for (const int k : {2, 1, 3, 0}) {
                int n = 0;
                while (n < 4) {
                    const GridCoord previa{e.casilla.x + kDx[k] * n, e.casilla.y + kDy[k] * n};
                    if (!puedePisar(e, GridCoord{e.casilla.x + kDx[k] * (n + 1), e.casilla.y + kDy[k] * (n + 1)}, m, &previa)) break;
                    ++n;
                }
                if (n > largo) mejor = k, largo = n;
            }
            e.rumboCalle = mejor;
        }
        dice(e, m, dt);
        // El paso en curso se acaba siempre (tambien con el mundo parado).
        float sobra = 0.0f;
        if (e.andando()) {
            e.progreso += dt / kPaso;
            if (e.progreso < 1.0f) continue;
            sobra = (e.progreso - 1.0f) * kPaso;
            e.progreso = 1.0f;
        }
        if (m.parado) {
            if (e.hablando > 0.0f) e.rumbo = rumboHacia(e.casilla, m.jugador);
            continue;
        }
        if (e.hablando > 0.0f) {
            e.hablando -= dt;
            e.rumbo = rumboHacia(e.casilla, m.jugador);
            continue;
        }
        if (!e.ruta.empty()) {
            const GridCoord siguienteCasilla = e.ruta.front();
            if (puedePisar(e, siguienteCasilla, m) && !igual(siguienteCasilla, e.casilla)) {
                e.desde = e.casilla;
                e.casilla = siguienteCasilla;
                ent->object.position = siguienteCasilla;
                e.rumbo = rumboHacia(e.desde, e.casilla);
                e.progreso = std::min(0.99f, sobra / kPaso);
                e.ruta.erase(e.ruta.begin());
            } else {
                e.ruta.clear();  // alguien se ha puesto delante: espera y lo piensa otra vez
                e.espera = carril(e.casilla) ? 0.3f : entre(e.azar, 0.8f, 1.6f);
            }
            continue;
        }
        if (e.actividad > 0.0f) {
            e.actividad = std::max(0.0f, e.actividad - dt);
            continue;
        }
        if (!e.def.actividad.empty()) {
            e.hastaActividad -= dt;
            if (e.hastaActividad <= 0.0f) {
                e.actividad = kActividad;
                e.hastaActividad = entre(e.azar, e.def.cadaMin, e.def.cadaMax);
                continue;
            }
        }
        mira(e, m);
        e.espera -= dt;
        if (e.espera <= 0.0f) decide(e, m);
    }
}

}  // namespace Game
