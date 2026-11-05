// UI-BG3-01 — ver include/HUD/Pergamino/MesaCombateHud.h. Aqui: la bandeja
// de cartas, la carta grande, lo que queda del turno, «F Pasar», el aviso de
// la reaccion y el rotulo del turno del rival. El orden de turno, la placa
// del enemigo, el registro y el emergente van en MesaCombateRetales.cpp.
//
// Medidas de la maqueta C03a en 1280x720: bandeja de cuero de 984x158 en
// (148, 548) con las cartas de 72x100 cada 80 px (basicas desde x = 166, la
// mano desde x = 426; el nombre en Lora de 13 px y hasta tres lineas, como
// «Bendicion / de las / Estrellas»); carta grande de 254x336 en (28, 150);
// el turno en una tira de 630x36 en (170, 505); «F Pasar» de 120x58 en
// (1144, 640).
#include "HUD/Pergamino/MesaCombateHud.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <vector>

#include "HUD/Pergamino/DadosPergamino.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"
#include "MesaCombateInterno.h"

namespace Pergamino {

namespace combate {

using Game::CartaBarra;
using Z = Game::BarraCombate::Zona;

int centrado(const Ctx& k, Letra letra, const std::string& s, float x, float y, float ancho, float alto,
             int maxLineas, const Vector4& color) {
    std::vector<std::string> lineas;
    std::string linea, palabra;
    auto cierra = [&] {
        if (palabra.empty()) return;
        const std::string prueba = linea.empty() ? palabra : linea + " " + palabra;
        if (!linea.empty() && k.p.letras().ancho(letra, prueba) > ancho) {
            lineas.push_back(linea);
            linea = palabra;
        } else {
            linea = prueba;
        }
        palabra.clear();
    };
    for (char ch : s) {
        if (ch == ' ') cierra();
        else palabra += ch;
    }
    cierra();
    if (!linea.empty()) lineas.push_back(linea);
    if (static_cast<int>(lineas.size()) > maxLineas) {
        std::string resto = lineas[static_cast<std::size_t>(maxLineas - 1)];
        for (std::size_t i = static_cast<std::size_t>(maxLineas); i < lineas.size(); ++i) resto += " " + lineas[i];
        lineas.resize(static_cast<std::size_t>(maxLineas));
        lineas.back() = k.p.letras().recorta(letra, resto, ancho);
    }
    for (std::size_t i = 0; i < lineas.size(); ++i) {
        const float w = static_cast<float>(k.p.letras().ancho(letra, lineas[i]));
        k.p.texto(k.l, letra, lineas[i], std::round(x + (ancho - w) * 0.5f), y + alto * static_cast<float>(i), alto,
                  color);
    }
    return static_cast<int>(lineas.size());
}

std::string conMayuscula(std::string s) {
    if (!s.empty()) s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
    return s;
}

void tipoDeCarta(RPG::ActionType t, const char*& glifo, Vector4& color, const char*& rotulo) {
    switch (t) {
        case RPG::ActionType::FREE_ACTION: glifo = "▶"; color = Color::kExito; rotulo = "ACCIÓN LIBRE"; break;
        case RPG::ActionType::REACTION: glifo = "◆"; color = Color::kCarmin; rotulo = "REACCIÓN"; break;
        case RPG::ActionType::CHANNEL: glifo = "●"; color = Color::kInfo; rotulo = "CANALIZA"; break;
        case RPG::ActionType::MOVEMENT: glifo = "▲"; color = Color::kAviso; rotulo = "MOVIMIENTO"; break;
        default: glifo = "■"; color = Color::kTinta; rotulo = "ACCIÓN PRINCIPAL"; break;
    }
}

const char* iconoDeCarta(const Pincel& p, const CartaBarra& c) {
    if (c.id == "#atacar") return "ranura_mano_principal";
    if (c.id == "#pocion") return "consumible";
    if (c.id == "#huir") return "cerrar";
    if (!c.condicion.empty() && p.iconos().frame(c.condicion) > 0) return c.condicion.c_str();
    if (!c.icono.empty() && p.iconos().frame(c.icono) > 0) return c.icono.c_str();  // EH-13: su familia
    switch (c.tipo) {
        case RPG::ActionType::FREE_ACTION: return "inspiracion";
        case RPG::ActionType::REACTION: return "mejora";
        case RPG::ActionType::CHANNEL: return "pm";
        case RPG::ActionType::MOVEMENT: return "ralentizado";
        default: return "dote";
    }
}

namespace {

constexpr float kCartaW = 72.0f, kCartaH = 100.0f, kPaso = 80.0f;
constexpr float kManoW = 84.0f, kManoPaso = 88.0f;  // EH-13: las de la mano, mas anchas (el nombre a 16 px)
constexpr float kBasicasX = 166.0f, kManoX = 426.0f, kCartaY = 586.0f;

std::string tecla(int n) { return n == 10 ? std::string("0") : std::to_string(n); }

// Una carta de la bandeja.
// EH-13 — La franja del elemento, abajo: Fuego rojo, Agua azul...
Vector4 colorDeElemento(const std::string& e) {
    if (e == "Fuego") return rgb(200, 72, 40);
    if (e == "Agua") return rgb(52, 110, 190);
    if (e == "Rayo") return rgb(222, 184, 40);
    if (e == "Hielo") return rgb(120, 190, 220);
    if (e == "Naturaleza") return rgb(80, 150, 60);
    if (e == "Veneno") return rgb(120, 160, 40);
    if (e == "Luz" || e == "Sagrado") return rgb(236, 206, 110);
    if (e == "Oscuro" || e == "Tinieblas") return rgb(96, 60, 130);
    if (e == "Arcano") return rgb(170, 90, 190);
    return Color::kPapelMancha;
}

void pintaCarta(const Ctx& k, const CartaBarra& c, float x, bool foco, int posicion, float ancho = kCartaW) {
    const float y = kCartaY - (foco ? 6.0f : 0.0f);
    const Rect r{x, y, ancho, kCartaH};
    const bool apagada = c.estado == CartaBarra::Estado::Gastada || c.estado == CartaBarra::Estado::Bloqueada;
    const bool preparada = c.estado == CartaBarra::Estado::Preparada;
    // La reaccion preparada brilla en oro (C03c): salta sola cuando toca.
    if (preparada) k.l.color(Rect{r.x - 6.0f, r.y - 6.0f, r.w + 12.0f, r.h + 12.0f}, rgb(240, 196, 72, 0.35f));
    k.l.color(Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 4.0f},
              foco ? Color::kCarmin : preparada ? rgb(212, 160, 40) : Color::kLinea);
    if (foco || preparada) k.l.color(Rect{r.x - 1.0f, r.y - 1.0f, r.w + 2.0f, r.h + 2.0f}, foco ? Color::kCarmin : rgb(212, 160, 40));
    k.l.color(r, apagada ? rgb(196, 178, 140) : Color::kPapelLuz);
    k.l.color(Rect{r.x, r.abajo() - 4.0f, r.w, 4.0f}, colorDeElemento(c.elemento));
    const char* glifo = "■";
    Vector4 color = Color::kTinta;
    const char* rotulo = "";
    tipoDeCarta(c.tipo, glifo, color, rotulo);
    if (c.id[0] == '#') glifo = "■", color = Color::kTinta;
    k.p.pixel(k.l, glifo, r.x + 6.0f, r.y + 4.0f, 2, apagada ? Color::kTinta3 : color);
    if (c.tecla > 0) {
        const std::string t = tecla(c.tecla);
        k.p.pixel(k.l, t, r.derecha() - 6.0f - k.p.anchoPixel(t, 2), r.y + 4.0f, 2, Color::kTinta2);
    }
    k.p.icono(k.l, iconoDeCarta(k.p, c), std::round(r.x + (r.w - 32.0f) * 0.5f), r.y + 18.0f, 2,
              apagada ? rgb(255, 255, 255, 0.45f) : Color::kBlanco);
    centrado(k, Letra::Lora16, c.nombre, r.x + 1.0f, r.y + 51.0f, r.w - 2.0f, 15.0f, 3,  // EH-13: a 16 px
             apagada ? Color::kTinta3 : Color::kTinta);
    if (c.estado == CartaBarra::Estado::Gastada) {
        // Gastada: velada y tachada en diagonal (se recupera al descansar).
        k.l.color(r, rgb(60, 40, 24, 0.22f));
        for (int i = 0; i < 9; ++i)
            k.l.color(Rect{r.x + 6.0f + static_cast<float>(i) * 7.0f, r.y + 86.0f - static_cast<float>(i) * 9.0f, 6.0f, 2.0f},
                      rgb(85, 62, 43, 0.6f));
    } else if (c.estado == CartaBarra::Estado::Bloqueada) {
        k.l.color(r, rgb(60, 40, 24, 0.18f));
    }
    k.l.zona(Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 8.0f}, static_cast<int>(Z::Carta), posicion);
}

// El hueco de las cartas de creador: tier 2, apartadas (C03a: «+3 de creador»).
void pintaCreador(const Ctx& k, const Game::VistaBarra& v, float x, bool foco) {
    const Rect r{x, kCartaY, kCartaW, kCartaH};
    const Vector4 claro = foco ? Color::kCarmin : Color::kClaroCuero2;
    for (float xx = r.x; xx < r.derecha(); xx += 8.0f) {
        k.l.color(Rect{xx, r.y, 4.0f, 2.0f}, claro);
        k.l.color(Rect{xx, r.abajo() - 2.0f, 4.0f, 2.0f}, claro);
    }
    for (float yy = r.y; yy < r.abajo(); yy += 8.0f) {
        k.l.color(Rect{r.x, yy, 2.0f, 4.0f}, claro);
        k.l.color(Rect{r.derecha() - 2.0f, yy, 2.0f, 4.0f}, claro);
    }
    k.p.pixel(k.l, "T2", r.derecha() - 8.0f - k.p.anchoPixel("T2", 2), r.y + 6.0f, 2, Color::kClaroCuero);
    k.p.icono(k.l, "tier", std::round(r.x + (r.w - 32.0f) * 0.5f), r.y + 22.0f, 2);
    centrado(k, Letra::Lora13, "+" + std::to_string(v.creador.size()) + " de creador", r.x + 1.0f, r.y + 58.0f,
             r.w - 2.0f, 15.0f, 2, Color::kClaroCuero);
    k.l.zona(r, static_cast<int>(Z::Creador));
}

// La carta senalada en grande, a la izquierda.
void pintaCartaGrande(const Ctx& k, const Game::VistaBarra& v, const Game::BarraCombate& b, const std::string& rival) {
    const Rect r{28.0f, 150.0f, 254.0f, 336.0f};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 81, 2, 0, 1), Vector2{r.x, r.y},
                          Color::kSombra);
    const float x = r.x + 16.0f, w = r.w - 32.0f;
    if (b.verCreador()) {
        k.l.color(Rect{r.x + 12.0f, r.y + 14.0f, r.w - 24.0f, 28.0f}, Color::kTinta2);
        k.p.pixel(k.l, "CARTAS DE CREADOR", x + 4.0f, r.y + 20.0f, 2, Color::kClaroCuero);
        float y = r.y + 56.0f;
        for (const std::string& n : v.creador) {
            k.p.pixel(k.l, "T2", x, y + 4.0f, 2, Color::kTinta3);
            k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, n, w - 30.0f), x + 30.0f, y, 26.0f,
                      Color::kTinta);
            y += 30.0f;
        }
        k.p.parrafo(k.l, Letra::Lora18, "Las cartas de creador se desbloquean al llegar a tier 2. Son las que podrás "
                                        "cambiar durante el juego.",
                    x, y + 8.0f, w, 23.0f, Color::kTinta2, 6);
        return;
    }
    const CartaBarra* c = Game::BarraCombate::carta(v, b.foco());
    if (c == nullptr) return;
    const char* glifo = "■";
    Vector4 color = Color::kTinta;
    const char* rotulo = "";
    tipoDeCarta(c->tipo, glifo, color, rotulo);
    if (c->id[0] == '#') rotulo = c->id == "#atacar" ? "ACCIÓN PRINCIPAL" : c->id == "#pocion" ? "OBJETO" : "HUIDA";
    k.l.color(Rect{r.x + 12.0f, r.y + 14.0f, r.w - 24.0f, 28.0f}, Color::kTinta);
    k.p.pixel(k.l, rotulo, x + 4.0f, r.y + 20.0f, 2, Color::kClaroCuero);
    if (c->tecla > 0) {
        const std::string t = tecla(c->tecla);
        k.p.pixel(k.l, t, r.derecha() - 20.0f - k.p.anchoPixel(t, 2), r.y + 20.0f, 2, Color::kClaroCuero);
    }
    k.p.icono(k.l, iconoDeCarta(k.p, *c), x, r.y + 56.0f, 2);
    float y = r.y + 50.0f;
    const auto lineasNombre = k.p.letras().envuelve(Letra::Lora24b, c->nombre, w - 42.0f);
    for (std::size_t i = 0; i < lineasNombre.size() && i < 2; ++i, y += 28.0f)
        k.p.texto(k.l, Letra::Lora24b, lineasNombre[i], x + 42.0f, y, 28.0f, Color::kTinta);
    y = std::max(y, r.y + 94.0f) + 4.0f;
    // Abajo, a quien, como se defiende y si cierra el turno; encima del pie.
    // Se miden antes: la descripcion cede lineas para que quepan enteras.
    struct Linea {
        const char* icono;
        std::string texto;
        int lineas;
    };
    std::vector<Linea> lineas;
    auto linea = [&](const char* icono, const std::string& texto) {
        if (texto.empty()) return;
        const int n = static_cast<int>(k.p.letras().envuelve(Letra::Lora16, texto, w - 22.0f).size());
        lineas.push_back(Linea{icono, texto, std::clamp(n, 1, 2)});
    };
    std::string objetivo;
    switch (c->objetivo) {
        case RPG::SkillTarget::SELF: objetivo = "Tú"; break;
        case RPG::SkillTarget::SINGLE_ENEMY: objetivo = rival.empty() ? std::string("Un enemigo") : rival; break;
        case RPG::SkillTarget::ALL_ENEMIES: objetivo = "Todos los enemigos"; break;
        case RPG::SkillTarget::SINGLE_ALLY: objetivo = "Un aliado (o tú)"; break;
        case RPG::SkillTarget::ALL_ALLIES: objetivo = "Tus aliados"; break;
        default: objetivo = rival.empty() ? std::string("Los que estén cerca") : rival + " y los que estén cerca"; break;
    }
    // H2-02: si hay a quien elegir, como se cambia.
    bool aliado = false;
    if (Game::BarraCombate::eligeObjetivo(*c, aliado)) {
        const std::vector<bool>& vivos = aliado ? v.aliadosVivos : v.enemigosVivos;
        if (std::count(vivos.begin(), vivos.end(), true) > 1) objetivo += " (W/S: otro)";
    }
    linea("objetivo", objetivo);
    // AV13-13 — Hasta donde llega, con tablero (Game/AlcanceCartas.h).
    if (c->alcance == 1) linea("objetivo", "Alcance: al lado (1 casilla)");
    else if (c->alcance > 1) linea("objetivo", "Alcance: " + std::to_string(c->alcance) + " casillas, viéndole");
    if (c->defensa == "defensa_mental") linea("perjuicio", "Se defiende con su defensa mental (CAR)");
    else if (c->defensa == "resistencia_fisica") linea("perjuicio", "Se defiende con su resistencia (CON)");
    else if (c->id[0] != '#' && c->objetivo != RPG::SkillTarget::SELF && c->objetivo != RPG::SkillTarget::SINGLE_ALLY &&
             c->objetivo != RPG::SkillTarget::ALL_ALLIES)
        linea("perjuicio", "Si gana su tirada, la resiste");
    std::string gasto;
    switch (c->tipo) {
        case RPG::ActionType::FREE_ACTION: gasto = "Se gasta y tu turno sigue"; break;
        case RPG::ActionType::REACTION: gasto = "Se prepara y salta sola"; break;
        case RPG::ActionType::CHANNEL: gasto = "Canaliza: dura varios turnos"; break;
        case RPG::ActionType::MOVEMENT: gasto = "Tu movimiento del turno"; break;
        default: gasto = c->id[0] == '#' ? "Cierra tu turno" : "Se gasta y cierra tu turno"; break;
    }
    linea("cerrar", gasto);
    const float yp = r.abajo() - 46.0f;  // el pie
    float altoLineas = 0.0f;
    for (const Linea& li : lineas) altoLineas += 20.0f * static_cast<float>(li.lineas) + 6.0f;
    const int maxDescripcion = std::clamp(static_cast<int>((yp - 6.0f - altoLineas - 10.0f - y) / 21.0f), 1, 6);
    y += k.p.parrafo(k.l, Letra::Lora16, c->descripcion, x, y, w, 21.0f, Color::kTinta2, maxDescripcion) + 10.0f;
    for (const Linea& li : lineas) {
        if (y + 20.0f > yp - 6.0f) break;
        const int n = y + 20.0f * static_cast<float>(li.lineas) > yp - 6.0f ? 1 : li.lineas;
        k.p.icono(k.l, li.icono, x, y + 3.0f, 1);
        y += k.p.parrafo(k.l, Letra::Lora16, li.texto, x + 22.0f, y, w - 22.0f, 20.0f, Color::kTinta2, n) + 6.0f;
    }
    // Pie: jugarla, o por que no se puede.
    if (c->estado == CartaBarra::Estado::Lista) {
        const float wt = k.p.tecla(k.l, "ENTER", x, yp);
        k.p.texto(k.l, Letra::Lora18, c->filas.size() > 1 ? "Elegir" : "Jugar", x + wt + 8.0f, yp, 28.0f,
                  Color::kTinta2);
        k.p.texto(k.l, Letra::Lora16, "o clic", x + wt + 70.0f, yp + 3.0f, 24.0f, Color::kTinta3);
    } else if (c->estado == CartaBarra::Estado::Gastada && c->filaDesgarro != 0) {
        const float wt = k.p.tecla(k.l, "ENTER", x, yp);
        k.p.texto(k.l, Letra::Lora16, "Recuperar con Desgarro", x + wt + 8.0f, yp + 3.0f, 24.0f, Color::kCarmin);
    } else {
        k.p.pixel(k.l, "!", x, yp + 6.0f, 2, Color::kError);
        k.p.parrafo(k.l, Letra::Lora16, conMayuscula(c->motivo), x + 16.0f, yp - 2.0f, w - 16.0f, 20.0f, Color::kError, 2);
    }
}

// Lo que queda del turno: accion, movimiento, canaliza y reaccion. AV13-15: con tablero, delante, las
// casillas que te quedan por andar, un rombo por casilla (en oro, la que queda; en gris, la gastada).
void pintaTurno(const Ctx& k, const Game::VistaBarra& v) {
    const bool conPasos = v.pasos >= 0 && v.pasosMax > 0;
    const Rect r{170.0f, 505.0f, conPasos ? 740.0f : 630.0f, 36.0f};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 82, 1, 0, 0), Vector2{r.x, r.y},
                          Color::kSombra);
    struct Item {
        const char* glifo;
        const char* texto;
        bool gastado;
        Vector4 color;
    };
    const bool lista = v.turno.preparada >= 0;
    const Item items[] = {{"●", "ACCIÓN", v.turno.principal, Color::kTinta},
                          {"▲", "MOVIMIENTO", v.turno.movimiento, Color::kTinta},
                          {"●", "CANALIZA", v.turno.canalizacion, Color::kTinta},
                          {"◆", lista ? "REACCIÓN LISTA" : "REACCIÓN", v.turno.reaccionUsada && !lista,
                           lista ? Color::kCarmin : Color::kTinta}};
    float x = r.x + 18.0f;
    if (conPasos) {
        const Vector4 c = v.pasos > 0 ? Color::kTinta : Color::kTinta3;
        x += k.p.pixel(k.l, "PASOS", x, r.y + 9.0f, 2, c) + 10.0f;
        for (int i = 0; i < v.pasosMax; ++i, x += 16.0f)
            k.p.pixel(k.l, "◆", x, r.y + 9.0f, 2, i < v.pasos ? Color::kAviso : Color::kTinta3);
        x += 18.0f;
    }
    for (const Item& it : items) {
        const Vector4 c = it.gastado ? Color::kTinta3 : it.color;
        x += k.p.pixel(k.l, it.glifo, x, r.y + 9.0f, 2, c) + 6.0f;
        const float w = k.p.pixel(k.l, it.texto, x, r.y + 9.0f, 2, c);
        if (it.gastado) k.l.color(Rect{x, r.y + 18.0f, w, 2.0f}, Color::kTinta3);
        x += w + 22.0f;
    }
}

}  // namespace

void pintaBandeja(const Ctx& k, const Game::VistaBarra& v, const Game::BarraCombate& b, const std::string& rival) {
    // La bandeja de cuero con sus dos grupos.
    k.l.materialConSombra(cuero(984, 158, 9, true, false, true), Vector2{148.0f, 548.0f}, Color::kSombraCuero);
    k.p.pixel(k.l, "BÁSICAS", kBasicasX, 556.0f, 2, Color::kClaroCuero);
    k.p.pixel(k.l, "TU MANO", kManoX, 556.0f, 2, Color::kClaroCuero);
    for (float y = 566.0f; y < 690.0f; y += 8.0f) k.l.color(Rect{410.0f, y, 2.0f, 4.0f}, Color::kClaroCuero2);
    int posicion = 0;
    for (std::size_t i = 0; i < v.basicas.size(); ++i, ++posicion)
        pintaCarta(k, v.basicas[i], kBasicasX + static_cast<float>(i) * kPaso, posicion == b.foco() && !b.verCreador(),
                   posicion);
    float x = kManoX;
    for (std::size_t i = 0; i < v.mano.size() && x + kManoW < 1128.0f; ++i, ++posicion, x += kManoPaso)
        pintaCarta(k, v.mano[i], x, posicion == b.foco() && !b.verCreador(), posicion, kManoW);
    if (!v.creador.empty() && x + kCartaW < 1132.0f) pintaCreador(k, v, x, b.verCreador());
    pintaCartaGrande(k, v, b, rival);
    pintaTurno(k, v);
    // F Terminar (AV13-28: el turno ya no lo cierra la accion principal, sino esto)
    const Rect pasar{1136.0f, 640.0f, 140.0f, 58.0f};
    k.l.materialConSombra(hoja(static_cast<int>(pasar.w), static_cast<int>(pasar.h), 86, 2, 0, 0),
                          Vector2{pasar.x, pasar.y}, Color::kSombra);
    const float wf = k.p.tecla(k.l, "F", pasar.x + 18.0f, pasar.y + 15.0f);
    k.p.texto(k.l, Letra::Lora18, "Terminar", pasar.x + 18.0f + wf + 8.0f, pasar.y + 15.0f, 28.0f, Color::kTinta);
    k.l.zona(pasar, static_cast<int>(Z::Pasar));
}

void pintaAvisoReaccion(const Ctx& k, const Game::BarraCombate& b) {
    if (b.avisoRestante() <= 0.0f || b.aviso().empty()) return;
    const std::size_t desde = k.l.cuantos();
    const Rect r{330.0f, 176.0f, 330.0f, 82.0f};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 88, 2, 0, 1), Vector2{r.x, r.y},
                          Color::kSombra);
    k.p.icono(k.l, "inspiracion", r.x + 18.0f, r.y + 25.0f, 2);
    k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, "¡" + b.aviso() + "!", r.w - 80.0f),
              r.x + 62.0f, r.y + 14.0f, 26.0f, Color::kTinta);
    k.p.texto(k.l, Letra::Lora16, "Ha saltado sola. Queda gastada.", r.x + 62.0f, r.y + 42.0f, 24.0f,
              Color::kTinta2);
    k.l.atenua(desde, std::min(1.0f, b.avisoRestante() / 0.5f));
}

// C03c / AV16-11: «Turno de Perdido Saqueador» con su retrato, mientras el rival actua.
// Su mano y su barra se esconden (pintaCombate): el turno es suyo, no tuyo.
void pintaTurnoRival(const Ctx& k, const DatosCombate& d) {
    const Game::BarraCombate& b = *d.barra;
    if (!b.rivalPendiente()) return;
    const std::string texto = "Turno de " + b.rival();
    const float wt = static_cast<float>(k.p.letras().ancho(Letra::Lora24b, texto));
    const float w = std::round(std::min(900.0f, wt + 160.0f)), h = 112.0f;
    const Rect r{std::round(640.0f - w * 0.5f), 520.0f, w, h};
    k.l.materialConSombra(hoja(static_cast<int>(r.w), static_cast<int>(r.h), 89, 2, 0, 1), Vector2{r.x, r.y},
                          Color::kSombra);
    // Su retrato: el del enemigo (o el aliado) al que le toca; sin iniciativa, el primero en pie (D0037).
    Rect uv{};
    if (d.sprites != nullptr) {
        const auto i = static_cast<std::size_t>(std::max(0, d.turnoEnemigo));
        if (d.turnoAliado > 0 && d.turnoEnemigo < 0 && static_cast<std::size_t>(d.turnoAliado) <= d.aliados.size()) {
            const AliadoEnMesa& a = d.aliados[static_cast<std::size_t>(d.turnoAliado) - 1];
            if (!a.objectId.empty()) uv = k.r.enemigo(*d.sprites, a.objectId);
        } else if (d.turnoEnemigo >= 0 && i < d.enemigos.size()) {
            if (!d.enemigos[i].objectId.empty()) uv = k.r.enemigo(*d.sprites, d.enemigos[i].objectId);
        } else {
            for (const EnemigoEnMesa& e : d.enemigos)
                if (uv.w <= 0.0f && e.vivo && !e.objectId.empty()) uv = k.r.enemigo(*d.sprites, e.objectId);
        }
    }
    const Rect ret{r.x + 20.0f, r.y + 14.0f, 84.0f, 84.0f};
    k.l.color(Rect{ret.x - 3.0f, ret.y - 3.0f, ret.w + 6.0f, ret.h + 6.0f}, Color::kCarmin);
    k.l.color(ret, rgb(222, 196, 150));
    if (uv.w > 0.0f) k.l.rect(Fuente::Retratos, ret, uv, Color::kBlanco);
    else k.p.icono(k.l, "perjuicio", ret.x + 26.0f, ret.y + 26.0f, 2);
    k.p.texto(k.l, Letra::Lora24b, k.p.letras().recorta(Letra::Lora24b, texto, r.w - 140.0f), ret.derecha() + 22.0f,
              r.y + 22.0f, 32.0f, Color::kTinta);
    k.p.texto(k.l, Letra::Lora16, "Espera: está actuando.", ret.derecha() + 22.0f, r.y + 66.0f, 24.0f, Color::kTinta2);
}

}  // namespace combate

void pintaCombate(Lienzo& l, const Pincel& p, const DatosCombate& d, Retratista& retratos, int anchoVista,
                  int altoVista) {
    if (d.vista == nullptr || d.barra == nullptr) return;
    const std::size_t desdeTrazo = l.cuantos(), desdeZona = l.cuantasZonas();
    const combate::Ctx k{l, p, retratos};
    // A quien apunta la carta senalada (H2-02), o el primero en pie.
    std::string rival;
    for (const Game::CartasEnCombate::Alcanzado& a : d.barra->afectados(*d.vista))
        if (!a.aliado && a.i < d.enemigos.size() && rival.empty()) rival = d.enemigos[a.i].nombre;
    for (const EnemigoEnMesa& e : d.enemigos)
        if (e.vivo && rival.empty()) rival = e.nombre;
    combate::pintaOrden(k, d);
    combate::pintaPlaca(k, d);
    combate::pintaRegistro(k, d);
    // H2-01: la ultima tirada, entre el registro (hasta y = 418) y la bandeja
    // (desde y = 548), con sitio para su halo.
    // La que tiene en pantalla la barra (todas salen, una detras de otra); si la
    // barra aun no ha mirado ninguna, la ultima de la vista.
    const RPG::TiradaVisible& tirada =
        d.barra->tiradaEnPantalla().numero > 0 ? d.barra->tiradaEnPantalla() : d.vista->tirada;
    if (tirada.numero > 0) {
        // Oriol (22/10): los dados ruedan los primeros kRodar segundos del brillo antes de dar el resultado.
        static const bool reducidas = std::getenv("MOTOR_MOVIMIENTO_REDUCIDO") != nullptr;
        const float desde = (1.0f - d.barra->brilloTirada()) * Game::BarraCombate::kBrilloTirada;
        const float rodar = !reducidas && d.barra->brilloTirada() > 0.0f && desde < dados::kRodar ? desde : -1.0f;
        pintaTiradaCombate(l, p, tirada, 972.0f, 428.0f, d.barra->brilloTirada(), d.barra->tiradaDe(),
                           d.barra->tiradaEn(), rodar);
    }
    // AV16-11: en el turno del rival no hay mano ni barra tuyas, solo su cartel.
    if (!d.barra->rivalPendiente()) combate::pintaBandeja(k, *d.vista, *d.barra, rival);
    combate::pintaAvisoReaccion(k, *d.barra);
    combate::pintaTurnoRival(k, d);
    combate::pintaEmergenteEstado(k, d, desdeZona);  // H2-03
    // Maquetado a 1280x720: en una vista mayor, abajo y centrado.
    const float dx = std::max(0.0f, std::round((static_cast<float>(anchoVista) - 1280.0f) * 0.5f));
    const float dy = std::max(0.0f, static_cast<float>(altoVista) - 720.0f);
    if (dx > 0.0f || dy > 0.0f) l.desplaza(desdeTrazo, desdeZona, dx, dy);
    combate::pintaEmergente(k, *d.barra, dx, dy, anchoVista, altoVista);
}

}  // namespace Pergamino
