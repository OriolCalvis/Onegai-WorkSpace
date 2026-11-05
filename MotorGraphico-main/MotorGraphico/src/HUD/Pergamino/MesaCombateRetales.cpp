// UI-BG3-01 — ver include/HUD/Pergamino/MesaCombateHud.h. Aqui: el orden de
// turno (con la ronda y los retratos), la placa del enemigo, el registro, el
// emergente para elegir (C03b) y los retratos de los enemigos.
//
// Medidas de la maqueta C03a en 1280x720: orden de turno en (490, 14), placa
// del enemigo de 250 de ancho en (688, 186), registro de 280x270 en
// (972, 148); el emergente de 980x440 en (150, 128) con tarjetas de 214x236.
#include <algorithm>
#include <cmath>
#include <cstring>

#include "Core/Imagen/ImagenRGBA.h"
#include "Game/Acompanante.h"
#include "Game/BattleState.h"
#include "Game/RegistroSprites.h"
#include "MesaCombateInterno.h"
#include "Render/ICombatant.h"

namespace Pergamino {

Rect Retratista::enemigo(const Game::RegistroSprites& registro, const std::string& objectId) {
    const std::string clave = "enemigo|" + objectId;
    if (m_atlas.tiene(clave)) return m_atlas.celda(clave, nullptr);
    return m_atlas.celda(clave, [&](Imagen& out) {
        const Game::SpriteActor* actor = registro.actor(objectId, "");
        const Game::SpriteFijo* fijo = actor == nullptr ? registro.fijo(objectId) : nullptr;
        const Game::HojaSprites* hoja = registro.hoja(actor != nullptr ? actor->hoja : fijo != nullptr ? fijo->hoja : "");
        if (hoja == nullptr) return false;
        auto png = imagen::cargarPNG("assets/" + hoja->imagen);
        if (!png.isOk()) return false;
        const imagen::ImagenRGBA& im = png.value();
        int x0 = 0, y0 = 0;
        if (actor != nullptr) {
            x0 = columnaDeCaraQuieta(hoja->columnas) * hoja->celdaAncho;  // de frente y quieto (J-3: segun las columnas)
            y0 = (actor->fila - 1) * hoja->celdaAlto;
        } else {
            const int f = std::max(0, fijo->frame - 1);
            x0 = (f % std::max(1, hoja->columnas)) * hoja->celdaAncho;
            y0 = (f / std::max(1, hoja->columnas)) * hoja->celdaAlto;
        }
        const int lado = AtlasRetratos::kLado;
        if (x0 + lado > im.ancho || y0 + lado > im.alto) return false;
        for (int y = 0; y < lado; ++y)
            std::memcpy(out.pixel(0, y), im.pixel(x0, y0 + y), static_cast<std::size_t>(lado) * 4);
        return true;
    });
}

std::vector<AliadoEnMesa> aliadosEnMesa(const BattleState& batalla) {
    std::vector<AliadoEnMesa> out;
    for (std::size_t i = 1; i < batalla.allies().size(); ++i) {
        const BattleParticipant& b = batalla.allies()[i];
        AliadoEnMesa a;
        a.id = b.id;
        a.nombre = b.name;
        if (b.combatant != nullptr) {
            a.objectId = b.combatant->combatant_id();
            a.pv = b.combatant->health();
            a.pvMax = b.combatant->maxHealth();
            a.vivo = b.combatant->isAlive();
            const auto* acompanante = dynamic_cast<const Game::Acompanante*>(b.combatant);
            if (acompanante != nullptr && acompanante->cura()) a.curas = acompanante->curasQueQuedan();
        }
        out.push_back(std::move(a));
    }
    return out;
}

namespace combate {

namespace {

using Z = Game::BarraCombate::Zona;

// H2-02 — Si la carta senalada alcanza a ese aliado o enemigo.
bool alcanzado(const std::vector<Game::CartasEnCombate::Alcanzado>& afectados, bool aliado, std::size_t i) {
    for (const auto& a : afectados)
        if (a.aliado == aliado && a.i == i) return true;
    return false;
}

// La marca de los afectados: una reticula carmin, cuatro esquinas de 12 px
// a 4 px por fuera de lo que se senala (el marco entero es de quien juega).
void reticula(const Ctx& k, const Rect& r) {
    const float g = 3.0f, largo = 12.0f, fuera = 4.0f;
    const float x0 = r.x - fuera, y0 = r.y - fuera, x1 = r.derecha() + fuera, y1 = r.abajo() + fuera;
    for (const float x : {x0, x1 - largo}) {
        k.l.color(Rect{x, y0, largo, g}, Color::kCarmin);
        k.l.color(Rect{x, y1 - g, largo, g}, Color::kCarmin);
    }
    for (const float y : {y0, y1 - largo}) {
        k.l.color(Rect{x0, y, g, largo}, Color::kCarmin);
        k.l.color(Rect{x1 - g, y, g, largo}, Color::kCarmin);
    }
}

const char* iconoDeOpcion(const std::string& opcion) {
    std::string o;
    for (char c : opcion) o += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (o.find("primavera") != std::string::npos) return "pv";
    if (o.find("verano") != std::string::npos) return "enfurecido";
    if (o.find("oto") != std::string::npos) return "perjuicio";
    if (o.find("invierno") != std::string::npos) return "ralentizado";
    return "pasiva";
}

}  // namespace

void pintaOrden(const Ctx& k, const DatosCombate& d) {
    const int n = static_cast<int>(d.enemigos.size() + d.aliados.size());
    // AV16-11: retratos mas grandes (80) y el que actua, marcado con una flecha encima.
    constexpr float kRet = 80.0f, kPaso = 92.0f;
    const float w = 104.0f + kPaso * static_cast<float>(1 + n);
    // H2-03: con estados, el heroe los lleva bajo su retrato y el retal crece.
    const bool conEstados = !d.vista->estadosHeroe.empty();
    const Rect r{std::round(640.0f - w * 0.5f), 14.0f, w, conEstados ? 148.0f : 124.0f};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 83, 2, 0, 1), Vector2{r.x, r.y},
                          Color::kSombra);
    k.p.pixel(k.l, "RONDA", r.x + 18.0f, r.y + 20.0f, 2, Color::kTinta2);
    k.p.pixel(k.l, std::to_string(d.vista->turno.ronda), r.x + 22.0f, r.y + 46.0f, 4, Color::kTinta);
    float x = r.x + 100.0f;
    auto caja = [&](bool activo, bool vivo) {
        const Rect c{x, r.y + 24.0f, kRet, kRet + 6.0f};
        const float borde = activo ? 4.0f : 2.0f;
        k.l.color(Rect{c.x - borde, c.y - borde, c.w + 2.0f * borde, c.h + 2.0f * borde},
                  activo ? Color::kCarmin : Color::kLinea);
        if (activo) {  // la flecha del que actua: un triangulo apuntando a su retrato
            const float cx = c.x + c.w * 0.5f;
            k.l.color(Rect{cx - 7.0f, c.y - 16.0f, 14.0f, 3.0f}, Color::kCarmin);
            k.l.color(Rect{cx - 5.0f, c.y - 13.0f, 10.0f, 3.0f}, Color::kCarmin);
            k.l.color(Rect{cx - 3.0f, c.y - 10.0f, 6.0f, 3.0f}, Color::kCarmin);
        }
        k.l.color(c, vivo ? rgb(222, 196, 150) : rgb(160, 140, 110));
        return c;
    };
    // AV13-11 — La tirada de iniciativa de cada uno, en una ficha en su esquina.
    auto iniciativa = [&](const Rect& c, const Game::Puesto* p) {
        if (p == nullptr) return;
        const std::string t = std::to_string(p->tirada());
        const float wt = k.p.anchoPixel(t, 2) + 8.0f;
        const Rect ficha{c.derecha() - wt + 2.0f, c.y - 4.0f, wt, 16.0f};
        k.l.color(Rect{ficha.x - 1.0f, ficha.y - 1.0f, ficha.w + 2.0f, ficha.h + 2.0f}, Color::kLinea);
        k.l.color(ficha, rgb(240, 222, 186));
        k.p.pixel(k.l, t, ficha.x + 4.0f, ficha.y + 3.0f, 2, Color::kTinta);
    };
    // El marco carmin va en quien juega: tu, o el rival mientras actua. H2-02:
    // a quien alcanza la carta senalada, la reticula; su retrato tambien se
    // puede senalar con el raton.
    const bool rival = d.barra->rivalPendiente();
    const auto afectados = d.barra->afectados(*d.vista);
    // ACOMPANANTES-01: mientras se ve la tirada de un acompanante, el marco es suyo.
    const RPG::TiradaVisible& enPantalla = d.barra->tiradaEnPantalla();
    const std::string tira = enPantalla.numero > 0 && enPantalla.delJugador && !d.barra->tiradasVistas() ? enPantalla.quienId : "";
    const bool tiraAliado = std::any_of(d.aliados.begin(), d.aliados.end(),
                                        [&](const AliadoEnMesa& a) { return a.vivo && a.id == tira; });
    // El heroe, con sus estados debajo.
    auto heroe = [&](bool activo, const Game::Puesto* p) {
        const Rect cajaHeroe = caja(activo, true);
        k.l.zona(cajaHeroe, static_cast<int>(Z::Objetivo), 0);
        if (alcanzado(afectados, true, 0)) reticula(k, cajaHeroe);
        Rect uv{};
        if (d.retratoHeroe != nullptr) uv = k.r.frame(*d.retratoHeroe, Game::frameQuieto(Game::Rumbo::Abajo, k.r.reparto()) - 1);  // de frente, quieto
        if (uv.w > 0.0f) k.l.rect(Fuente::Retratos, Rect{cajaHeroe.x, cajaHeroe.y + 2.0f, kRet, kRet}, uv, Color::kBlanco);
        else k.p.icono(k.l, "personaje", cajaHeroe.x + 24.0f, cajaHeroe.y + 26.0f, 2);
        if (conEstados) pintaEstadosHeroe(k, d.vista->estadosHeroe, cajaHeroe.x, cajaHeroe.abajo() + 6.0f);
        iniciativa(cajaHeroe, p);
        x += kPaso;
    };
    // ACOMPANANTES-01: uno de los que van contigo, con su vida debajo o, si cura,
    // una marca verde por cura que le queda. AV13-29: en su puesto de la iniciativa.
    auto aliado = [&](std::size_t i, bool activo, const Game::Puesto* p) {
        if (i >= d.aliados.size()) return;
        const AliadoEnMesa& a = d.aliados[i];
        const Rect c = caja(a.vivo && (activo || a.id == tira), a.vivo);
        if (a.vivo) k.l.zona(c, static_cast<int>(Z::Objetivo), static_cast<int>(i) + 1);
        if (a.vivo && alcanzado(afectados, true, i + 1)) reticula(k, c);
        Rect uvA{};
        if (d.sprites != nullptr && !a.objectId.empty()) uvA = k.r.enemigo(*d.sprites, a.objectId);
        if (uvA.w > 0.0f)
            k.l.rect(Fuente::Retratos, Rect{c.x, c.y + 2.0f, kRet, kRet}, uvA,
                     a.vivo ? Color::kBlanco : rgb(255, 255, 255, 0.4f));
        else
            k.p.icono(k.l, "personaje", c.x + 24.0f, c.y + 26.0f, 2);
        if (!a.vivo) k.p.pixel(k.l, "×", c.x + 30.0f, c.y + 28.0f, 4, Color::kError);
        if (a.curas >= 0) {
            for (int cura = 0; cura < a.curas; ++cura)
                k.l.color(Rect{c.x + static_cast<float>(cura) * 12.0f, c.abajo() + 4.0f, 8.0f, 8.0f}, Color::kExito);
        } else if (a.vivo) {
            k.p.barra(k.l, Rect{c.x, c.abajo() + 5.0f, c.w, 6.0f},
                      a.pvMax > 0 ? static_cast<float>(a.pv) / static_cast<float>(a.pvMax) : 0.0f, Color::kPv);
        }
        if (a.vivo) iniciativa(c, p);
        x += kPaso;
    };
    auto enemigo = [&](std::size_t i, bool activo, const Game::Puesto* p) {
        if (i >= d.enemigos.size()) return;
        const EnemigoEnMesa& e = d.enemigos[i];
        const Rect c = caja(activo, e.vivo);
        if (e.vivo) k.l.zona(c, static_cast<int>(Z::Objetivo), Game::BarraCombate::kObjetivoEnemigo + static_cast<int>(i));
        if (e.vivo && alcanzado(afectados, false, i)) reticula(k, c);
        Rect uvE{};
        if (d.sprites != nullptr && !e.objectId.empty()) uvE = k.r.enemigo(*d.sprites, e.objectId);
        if (uvE.w > 0.0f)
            k.l.rect(Fuente::Retratos, Rect{c.x, c.y + 2.0f, kRet, kRet}, uvE,
                     e.vivo ? Color::kBlanco : rgb(255, 255, 255, 0.4f));
        else
            k.p.icono(k.l, "perjuicio", c.x + 24.0f, c.y + 26.0f, 2);
        if (!e.vivo) k.p.pixel(k.l, "×", c.x + 30.0f, c.y + 28.0f, 4, Color::kError);
        if (e.vivo) iniciativa(c, p);
        x += kPaso;
    };
    if (d.orden.empty()) {  // sin iniciativa: tu, los tuyos y los enemigos; el marco, en el primero en pie
        heroe(!rival && !tiraAliado, nullptr);
        for (std::size_t i = 0; i < d.aliados.size(); ++i) aliado(i, false, nullptr);
        bool marcado = false;
        for (std::size_t i = 0; i < d.enemigos.size(); ++i) {
            const bool vivo = d.enemigos[i].vivo;
            enemigo(i, rival && vivo && !marcado, nullptr);
            marcado = marcado || (rival && vivo);
        }
        return;
    }
    // AV13-11 — En el orden de la iniciativa; el marco carmin, en el enemigo al que le toca.
    // AV13-29: cada acompanante, en el suyo; el que no tenga puesto, detras del heroe.
    std::vector<bool> conPuesto(d.aliados.size(), false);
    for (const Game::Puesto& p : d.orden)
        if (!p.enemigo && p.i > 0 && p.i - 1 < conPuesto.size()) conPuesto[p.i - 1] = true;
    for (std::size_t k2 = 0; k2 < d.orden.size(); ++k2) {
        const Game::Puesto& p = d.orden[k2];
        const bool toca = !rival && k2 == d.enTurno;
        if (p.enemigo) {
            enemigo(p.i, rival && k2 == d.enTurno, &p);
        } else if (p.i > 0) {
            aliado(p.i - 1, toca, &p);
        } else {
            heroe(toca && !tiraAliado, &p);
            for (std::size_t i = 0; i < d.aliados.size(); ++i)
                if (!conPuesto[i]) aliado(i, false, nullptr);
        }
    }
}

// La placa compacta (50 de alto): el nombre a la izquierda y «12/22» a la
// derecha en la misma fila; debajo, la barra y, a su lado, sus estados en
// fichas.
static void pintaPlacaCompacta(const Ctx& k, const EnemigoEnMesa& e,
                               const std::vector<Game::EstadoVisible>& estados, int i, const Rect& r, bool marcada) {
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 84, 2, 0, 1), Vector2{r.x, r.y},
                          Color::kSombra);
    k.l.zona(r, static_cast<int>(Z::Objetivo), Game::BarraCombate::kObjetivoEnemigo + i);
    if (marcada) reticula(k, r);
    const float x = r.x + 16.0f;
    const std::string cifra = std::to_string(e.pv) + "/" + std::to_string(e.pvMax);
    const float wCifra = k.p.anchoPixel(cifra, 2);
    k.p.pixel(k.l, cifra, r.derecha() - 16.0f - wCifra, r.y + 12.0f, 2, Color::kTinta);
    // El numero del final («Devoto de los Perdidos 3») no se recorta nunca:
    // es lo que distingue a uno de otro.
    std::string base = e.nombre, numero;
    const std::size_t corte = base.find_last_not_of("0123456789");
    if (corte != std::string::npos && corte + 1 < base.size() && base[corte] == ' ') {
        numero = base.substr(corte);
        base.erase(corte);
    }
    const float anchoNombre = r.w - 44.0f - wCifra - static_cast<float>(k.p.letras().ancho(Letra::Lora18b, numero));
    k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, base, anchoNombre) + numero, x, r.y + 6.0f,
              26.0f, Color::kTinta);
    const float anchoBarra = estados.empty() ? r.w - 32.0f : 132.0f;
    k.p.barra(k.l, Rect{x, r.y + 40.0f, anchoBarra, 8.0f},
              e.pvMax > 0 ? static_cast<float>(e.pv) / static_cast<float>(e.pvMax) : 0.0f, Color::kPv);
    pintaEstadosPlaca(k, estados, i + 1, x + anchoBarra + 8.0f, r.y + 26.0f, r.w - 32.0f - anchoBarra - 8.0f, true);
}

void pintaPlaca(const Ctx& k, const DatosCombate& d) {
    static const std::vector<Game::EstadoVisible> kNinguno;
    const auto afectados = d.barra->afectados(*d.vista);
    auto estadosDe = [&](std::size_t i) -> const std::vector<Game::EstadoVisible>& {
        return i < d.vista->estadosEnemigos.size() ? d.vista->estadosEnemigos[i] : kNinguno;
    };
    // Un grupo (Oriol, 05/10): si las placas enteras no caben encima del
    // rotulo «TURNO DE ...» (y = 452), van compactas y empiezan mas arriba,
    // bajo el orden de turno: el nombre y la vida en una fila, y la barra con
    // sus estados en fichas en otra.
    const float hasta = 446.0f;
    float enteras = 0.0f;
    for (std::size_t i = 0; i < d.enemigos.size(); ++i)
        if (d.enemigos[i].vivo) enteras += 92.0f + altoEstadosPlaca(estadosDe(i)) + 10.0f;
    const bool compacta = 186.0f + enteras - 10.0f > hasta;  // AV16-11: la tira es mas alta; la compacta empieza en 170
    float y = compacta ? 170.0f : 186.0f;
    for (std::size_t i = 0; i < d.enemigos.size(); ++i) {
        const EnemigoEnMesa& e = d.enemigos[i];
        if (!e.vivo) continue;
        const std::vector<Game::EstadoVisible>& estados = estadosDe(i);
        if (compacta) {
            const float alto = 50.0f;  // 170 + 5*(50+6) - 6 = 444 <= 446: caben cinco
            if (y + alto > hasta) break;
            pintaPlacaCompacta(k, e, estados, static_cast<int>(i), Rect{688.0f, y, 250.0f, alto},
                               alcanzado(afectados, false, i));
            y += alto + 6.0f;
            continue;
        }
        const float alto = 92.0f + altoEstadosPlaca(estados);
        const Rect r{688.0f, y, 250.0f, alto};
        k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 84, 2, 0, 1), Vector2{r.x, r.y},
                              Color::kSombra);
        // H2-02: la placa se senala con el raton, y la carta que la alcanza
        // la marca con la reticula.
        k.l.zona(r, static_cast<int>(Z::Objetivo), Game::BarraCombate::kObjetivoEnemigo + static_cast<int>(i));
        if (alcanzado(afectados, false, i)) reticula(k, r);
        const float x = r.x + 16.0f;
        k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, e.nombre, r.w - 32.0f), x, r.y + 12.0f,
                  26.0f, Color::kTinta);
        k.p.icono(k.l, "pv", x, r.y + 47.0f, 1);
        k.p.pixel(k.l, "PV", x + 22.0f, r.y + 46.0f, 2, Color::kTinta2);
        const std::string cifra = std::to_string(e.pv) + "/" + std::to_string(e.pvMax);
        k.p.pixel(k.l, cifra, r.derecha() - 16.0f - k.p.anchoPixel(cifra, 2), r.y + 46.0f, 2, Color::kTinta);
        k.p.barra(k.l, Rect{x, r.y + 68.0f, r.w - 32.0f, 8.0f},
                  e.pvMax > 0 ? static_cast<float>(e.pv) / static_cast<float>(e.pvMax) : 0.0f, Color::kPv);
        // H2-03: sus estados, con icono y rondas.
        pintaEstadosPlaca(k, estados, static_cast<int>(i) + 1, x, r.y + 86.0f, r.w - 32.0f);
        y += alto + 10.0f;
        if (y > 420.0f) break;
    }
}

void pintaRegistro(const Ctx& k, const DatosCombate& d) {
    if (!d.barra->registro()) {
        const Rect r{1110.0f, 148.0f, 150.0f, 46.0f};
        k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 85, 2, 0, 0), Vector2{r.x, r.y},
                              Color::kSombra);
        const float wl = k.p.tecla(k.l, "L", r.x + 14.0f, r.y + 9.0f);
        k.p.texto(k.l, Letra::Lora18, "Registro", r.x + 14.0f + wl + 8.0f, r.y + 9.0f, 28.0f, Color::kTinta);
        k.l.zona(r, static_cast<int>(Z::Registro));
        return;
    }
    const Rect r{972.0f, 148.0f, 280.0f, 270.0f};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 85, 2, 0, 1), Vector2{r.x, r.y},
                          Color::kSombra);
    k.p.pixel(k.l, "REGISTRO", r.x + 16.0f, r.y + 18.0f, 2, Color::kTinta2);
    const float wo = k.p.anchoAyuda({"L"}, "Ocultar");
    k.p.ayuda(k.l, {"L"}, "Ocultar", r.derecha() - 14.0f - wo, r.y + 10.0f, Color::kTinta2);
    k.l.zona(Rect{r.derecha() - 20.0f - wo, r.y + 6.0f, wo + 12.0f, 36.0f}, static_cast<int>(Z::Registro));
    // Las ultimas lineas que caben, la mas reciente abajo.
    const float ancho = r.w - 32.0f, alto = 20.0f;
    const int maxLineas = 10;
    std::vector<std::string> lineas;
    for (auto it = d.registro.rbegin(); it != d.registro.rend() && static_cast<int>(lineas.size()) < maxLineas; ++it) {
        std::vector<std::string> partes = k.p.letras().envuelve(Letra::Lora16, *it, ancho);
        for (auto pt = partes.rbegin(); pt != partes.rend() && static_cast<int>(lineas.size()) < maxLineas; ++pt)
            lineas.insert(lineas.begin(), *pt);
    }
    float y = r.y + 50.0f;
    for (const std::string& l : lineas) {
        k.p.texto(k.l, Letra::Lora16, l, r.x + 16.0f, y, alto, Color::kTinta);
        y += alto;
    }
}

void pintaEmergente(const Ctx& k, const Game::BarraCombate& b, float dx, float dy, int anchoVista, int altoVista) {
    const Game::BarraCombate::Emergente& e = b.emergente();
    if (!e.abierto || e.opciones.empty()) return;
    k.l.color(Rect{0.0f, 0.0f, static_cast<float>(anchoVista), static_cast<float>(altoVista)}, Color::kVelo);
    const std::size_t desdeTrazo = k.l.cuantos(), desdeZona = k.l.cuantasZonas();
    const Rect r{150.0f, 128.0f, 980.0f, 440.0f};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 87, 2, 6, 2), Vector2{r.x, r.y},
                          Color::kSombra);
    const std::string& titulo = e.titulo;  // la voz pixel ya sale en mayusculas
    const float wt = k.p.anchoPixel(titulo, 3);
    k.p.pixel(k.l, titulo, std::round(r.x + (r.w - wt) * 0.5f), r.y + 28.0f, 3, Color::kTinta);
    k.l.material(adorno(300), Vector2{std::round(r.x + (r.w - 300.0f) * 0.5f), r.y + 64.0f});
    centrado(k, Letra::Lora18, e.texto, r.x + 40.0f, r.y + 82.0f, r.w - 80.0f, 26.0f, 1, Color::kTinta2);
    const int n = static_cast<int>(e.opciones.size());
    const float hueco = 16.0f;
    const float w = std::min(214.0f, (r.w - 80.0f - hueco * static_cast<float>(n - 1)) / static_cast<float>(n));
    float x = std::round(r.x + (r.w - (w * static_cast<float>(n) + hueco * static_cast<float>(n - 1))) * 0.5f);
    const float y = r.y + 122.0f, h = 210.0f;
    for (int i = 0; i < n; ++i, x += w + hueco) {
        const Rect c{x, y, w, h};
        const bool foco = i == e.foco;
        k.l.color(Rect{c.x - 2.0f, c.y - 2.0f, c.w + 4.0f, c.h + 4.0f}, foco ? Color::kCarmin : Color::kLinea);
        k.l.color(c, foco ? rgb(236, 214, 170) : Color::kPapelLuz);
        k.l.material(pieza(Material::Cinta), Vector2{std::round(c.x + (c.w - 14.0f) * 0.5f), c.y - 6.0f});
        const std::string& nombre = e.opciones[static_cast<std::size_t>(i)];
        const int escala = k.p.anchoPixel(nombre, 3) <= c.w - 16.0f ? 3 : 2;
        const float wn = k.p.anchoPixel(nombre, escala);
        k.p.pixel(k.l, nombre, std::round(c.x + (c.w - wn) * 0.5f), c.y + 30.0f, escala, Color::kTinta);
        k.p.icono(k.l, iconoDeOpcion(e.opciones[static_cast<std::size_t>(i)]), std::round(c.x + (c.w - 32.0f) * 0.5f),
                  c.y + 72.0f, 2);
        const std::string& texto = e.textos[static_cast<std::size_t>(i)];
        if (!texto.empty()) centrado(k, Letra::Lora16, texto, c.x + 10.0f, c.y + 118.0f, c.w - 20.0f, 20.0f, 4, Color::kTinta2);
        k.p.texto(k.l, Letra::Lora16, std::to_string(i + 1), c.x + 8.0f, c.y + h - 26.0f, 22.0f, Color::kTinta3);
        k.l.zona(c, static_cast<int>(Z::Opcion), i);
    }
    // Pie: teclas y botones.
    const float yb = r.abajo() - 76.0f;
    float xa = r.x + 40.0f;
    xa += k.p.ayuda(k.l, {"←", "→"}, "Opción", xa, yb + 10.0f, Color::kTinta2) + 20.0f;
    xa += k.p.ayuda(k.l, {"ENTER"}, "Elegir", xa, yb + 10.0f, Color::kTinta2) + 20.0f;
    k.p.ayuda(k.l, {"ESC"}, e.dePasiva ? "Ahora no" : "Cancelar", xa, yb + 10.0f, Color::kTinta2);
    const std::string elegir = "ELEGIR " + e.opciones[static_cast<std::size_t>(std::clamp(e.foco, 0, n - 1))];
    const float we = std::max(180.0f, k.p.anchoPixel(elegir, 2) + 40.0f);
    const Rect boton{r.derecha() - 40.0f - we, yb, we, 46.0f};
    k.l.color(Rect{boton.x - 2.0f, boton.y - 2.0f, boton.w + 4.0f, boton.h + 4.0f}, Color::kCarmin2);
    k.l.color(boton, Color::kCarmin);
    const float wb = k.p.anchoPixel(elegir, 2);
    k.p.pixel(k.l, elegir, std::round(boton.x + (boton.w - wb) * 0.5f), boton.y + 12.0f, 2, Color::kClaroCuero);
    k.l.zona(boton, static_cast<int>(Z::Opcion), e.foco);
    const float wn = 150.0f;
    const Rect ahoraNo{boton.x - 20.0f - wn, yb, wn, 46.0f};
    k.l.color(Rect{ahoraNo.x - 2.0f, ahoraNo.y - 2.0f, ahoraNo.w + 4.0f, ahoraNo.h + 4.0f}, Color::kLinea);
    k.l.color(ahoraNo, Color::kPapelLuz);
    const std::string txt = e.dePasiva ? "AHORA NO" : "CANCELAR";
    k.p.pixel(k.l, txt, std::round(ahoraNo.x + (ahoraNo.w - k.p.anchoPixel(txt, 2)) * 0.5f), ahoraNo.y + 12.0f, 2,
              Color::kTinta);
    k.l.zona(ahoraNo, static_cast<int>(Z::AhoraNo));
    if (dx > 0.0f || dy > 0.0f) k.l.desplaza(desdeTrazo, desdeZona, dx, dy);
}

}  // namespace combate
}  // namespace Pergamino
