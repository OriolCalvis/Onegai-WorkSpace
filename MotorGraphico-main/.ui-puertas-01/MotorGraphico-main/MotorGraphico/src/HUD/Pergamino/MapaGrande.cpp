// MAPA-02 — El mapa grande (tecla M): el nivel entero en una hoja, con los
// sitios por su nombre, tu, el objetivo seguido, las chinchetas y la leyenda.
// Ver include/HUD/Pergamino/GuiaMapa.h.
//
// Medidas para la vista de 1280x720: hoja a 24 px del borde (area segura); el
// terreno a la izquierda (la imagen del minimapa, ampliada a celdas enteras) y
// una columna de 264 a la derecha con la leyenda, el objetivo y lo que hay bajo
// el cursor; el pie con las teclas, como las demas hojas.
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Game/DiarioMisiones.h"
#include "Game/GameSession.h"
#include "Game/Lugares.h"
#include "HUD/Pergamino/GuiaMapa.h"
#include "HUD/Pergamino/HudRetales.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"
#include "Level/ObjectCatalog.h"
#include "World/World.h"

namespace Pergamino {

namespace {

constexpr float kBorde = 24.0f;
constexpr float kColumna = 264.0f;
constexpr float kCeldaMaxima = 16.0f;

struct Clave {
    const char* glifo;
    Vector4 color;
    std::string texto;
};

Rect crece2(const Rect& r) { return Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 4.0f}; }

bool pisa(const Rect& a, const std::vector<Rect>& otros) {
    for (const Rect& b : otros)
        if (a.x < b.derecha() && b.x < a.derecha() && a.y < b.abajo() && b.y < a.abajo()) return true;
    return false;
}

// El nombre de un sitio en una etiqueta de papel claro, si no pisa a otra.
void etiqueta(Lienzo& l, const Pincel& p, const std::string& nombre, float x, float y, const Rect& limite,
              std::vector<Rect>& puestas) {
    const float w = p.anchoTexto(Letra::Lora16, nombre) + 12.0f, h = 22.0f;
    Rect r{std::round(x), std::round(y - h * 0.5f), w, h};
    if (r.derecha() > limite.derecha()) r.x = std::round(x - 24.0f - w);  // a la izquierda de la marca
    if (r.x < limite.x || r.y < limite.y || r.abajo() > limite.abajo() || pisa(r, puestas)) return;
    puestas.push_back(r);
    l.color(r, Color::kPapelLuz);
    p.contorno(l, r, 1.0f, Color::kAdorno);
    p.texto(l, Letra::Lora16, nombre, r.x + 6.0f, r.y, h, Color::kTinta);
}

}  // namespace

void GuiaMapa::pintaMapa(Lienzo& l, const Pincel& p, const GameSession& sesion, const Game::DiarioDatos* diario,
                         const std::string& zona, int anchoVista, int altoVista) {
    const float W = static_cast<float>(anchoVista), H = static_cast<float>(altoVista);
    l.color(Rect{0.0f, 0.0f, W, H}, Color::kVelo);
    const float x0 = kBorde, y0 = kBorde, w = W - 2.0f * kBorde, h = H - 2.0f * kBorde;
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 23, 2, 0, 3), Vector2{x0, y0}, Color::kSombra);
    p.tituloConFiletes(l, "MAPA", x0, y0 + 16.0f, w, 160);
    const float wz = p.anchoTexto(Letra::Lora20b, zona);
    p.texto(l, Letra::Lora20b, zona, std::round(x0 + (w - wz) * 0.5f), y0 + 48.0f, 28.0f, Color::kTinta2);

    // --- El terreno, a celdas enteras ---
    const int mw = std::max(1, m_minimapa.ancho()), mh = std::max(1, m_minimapa.alto());
    const Rect area{x0 + 24.0f, y0 + 88.0f, w - 48.0f - kColumna - 16.0f, h - 88.0f - 64.0f};
    const float celda = std::clamp(std::floor(std::min(area.w / static_cast<float>(mw), area.h / static_cast<float>(mh))),
                                   1.0f, kCeldaMaxima);
    const Rect terreno{std::round(area.x + (area.w - celda * static_cast<float>(mw)) * 0.5f),
                       std::round(area.y + (area.h - celda * static_cast<float>(mh)) * 0.5f),
                       celda * static_cast<float>(mw), celda * static_cast<float>(mh)};
    m_mapa.terreno = terreno;
    p.contorno(l, crece2(terreno), 2.0f, Color::kLinea);
    l.rect(Fuente::Minimapa, terreno, Rect{0.0f, 0.0f, 1.0f, 1.0f}, Color::kBlanco);
    l.zona(terreno, kZonaMapaGrande);

    // --- Las marcas: entradas (con su nombre), gente, objetivo, chinchetas y tu ---
    std::vector<MarcaMinimapa> marcas;
    std::vector<std::pair<GridCoord, std::string>> nombres;
    bool hayEnemigo = false;
    const ObjectCatalog* catalogo = sesion.objectCatalog();
    if (const Engine::World* mundo = sesion.world()) {
        mundo->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& e) {
            MarcaMinimapa m;
            m.x = static_cast<float>(e.position().x), m.y = static_cast<float>(e.position().y);
            if (e.kind == Engine::EntityKind::Enemy) {
                m.tipo = MarcaMinimapa::Tipo::Enemigo;
                hayEnemigo = true;
            } else if (!e.object.targetLevel.empty()) {
                m.tipo = MarcaMinimapa::Tipo::Entrada;
                if (const Game::Lugar* lugar = Game::Lugares::delJuego().de(e.object.targetLevel)) {
                    m.emblema = lugar->icono();
                    nombres.emplace_back(e.position(), lugar->nombre);
                } else if (const Game::Calle* c = Game::Lugares::delJuego().calle(e.object.targetLevel)) {
                    m.emblema = "lugar";  // AV16-06: la salida, con el nombre de lo que hay fuera
                    nombres.emplace_back(e.position(), "Hacia " + c->nombre);
                }
            } else {
                const ObjectDefinition* def = catalogo ? catalogo->find(e.object.objectId) : nullptr;
                if (def == nullptr || def->category != ObjectCategory::Npc) return;
                m.tipo = MarcaMinimapa::Tipo::Personaje;
            }
            marcas.push_back(m);
        });
    }
    const std::vector<MinimapaTinta::Sitio> sitios = m_minimapa.sitiosObjetivo(sesion, diario);
    for (const MinimapaTinta::Sitio& s : sitios) {
        MarcaMinimapa m;
        m.tipo = MarcaMinimapa::Tipo::Objetivo;
        m.x = static_cast<float>(s.celda.x), m.y = static_cast<float>(s.celda.y);
        marcas.push_back(m);
    }
    for (const Game::Chincheta& c : chinchetasAqui()) {
        MarcaMinimapa m;
        m.tipo = MarcaMinimapa::Tipo::Chincheta;
        m.x = static_cast<float>(c.x), m.y = static_cast<float>(c.y);
        m.color = c.color, m.numero = c.numero;
        marcas.push_back(m);
    }
    MarcaMinimapa tu;
    tu.tipo = MarcaMinimapa::Tipo::Jugador;
    tu.x = static_cast<float>(sesion.playerPosition().x), tu.y = static_cast<float>(sesion.playerPosition().y);
    marcas.push_back(tu);
    // De abajo arriba: gente y entradas, objetivo, chinchetas y tu encima.
    for (MarcaMinimapa::Tipo capa : {MarcaMinimapa::Tipo::Personaje, MarcaMinimapa::Tipo::Enemigo, MarcaMinimapa::Tipo::Entrada,
                                     MarcaMinimapa::Tipo::Objetivo, MarcaMinimapa::Tipo::Chincheta, MarcaMinimapa::Tipo::Jugador})
        for (const MarcaMinimapa& m : marcas) {
            if (m.tipo != capa) continue;
            pintaMarcaMinimapa(l, p, m, terreno.x, terreno.y, celda);
            if (m.tipo == MarcaMinimapa::Tipo::Chincheta)  // su numero, al lado (no solo el color)
                p.pixel(l, std::to_string(m.numero), terreno.x + (m.x + 0.5f) * celda + 7.0f,
                        terreno.y + (m.y + 0.5f) * celda - 16.0f, 2, Color::kTinta);
        }
    std::vector<Rect> puestas;
    for (const auto& [donde, nombre] : nombres)
        etiqueta(l, p, nombre, terreno.x + (static_cast<float>(donde.x) + 0.5f) * celda + 12.0f,
                 terreno.y + (static_cast<float>(donde.y) + 0.5f) * celda, area, puestas);

    // --- El cursor: el contorno del foco (carmin) ---
    const float lado = std::max(celda, 12.0f);
    const Rect foco{std::round(terreno.x + (static_cast<float>(m_mapa.cursor.x) + 0.5f) * celda - lado * 0.5f) - 2.0f,
                    std::round(terreno.y + (static_cast<float>(m_mapa.cursor.y) + 0.5f) * celda - lado * 0.5f) - 2.0f,
                    lado + 4.0f, lado + 4.0f};
    p.contorno(l, foco, 2.0f, Color::kCarmin);

    // --- La columna: leyenda, objetivo y lo que hay bajo el cursor ---
    const float xc = x0 + w - 24.0f - kColumna;
    float y = y0 + 88.0f;
    p.tituloBloque(l, "LEYENDA", xc, y, kColumna);
    y += 30.0f;
    std::vector<Clave> claves = {{"▲", Color::kTinta, "Tú"}, {"●", Color::kTinta2, "Personaje"}, {"■", Color::kInfo, "Entrada"}};
    if (hayEnemigo) claves.push_back({"●", Color::kCarmin, "Enemigo"});
    if (!sitios.empty()) claves.push_back({"◆", Color::kCarmin, "Objetivo"});
    claves.push_back({"▼", Color::kInfo,
                      "Chinchetas " + std::to_string(chinchetasAqui().size()) + "/" + std::to_string(Game::MarcasMapa::kMaximo)});
    for (const Clave& c : claves) {
        p.pixel(l, c.glifo, xc, y + 3.0f, 2, c.color);
        p.texto(l, Letra::Lora18, c.texto, xc + 20.0f, y, 26.0f, Color::kTinta);
        y += 28.0f;
    }
    y += 12.0f;
    p.tituloBloque(l, "OBJETIVO", xc, y, kColumna);
    y += 30.0f;
    const Game::DiarioMision* mision = diario != nullptr ? Game::misionSeguida(*diario) : nullptr;
    const Game::DiarioObjetivo* paso = mision != nullptr ? mision->siguiente() : nullptr;
    if (mision == nullptr || paso == nullptr) {
        y += p.parrafo(l, Letra::Lora18, "Ninguna misión seguida. Elige una con TAB, en Misiones.", xc, y, kColumna,
                       24.0f, Color::kTinta2, 3);
    } else {
        p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, mision->nombre, kColumna), xc, y, 22.0f, Color::kTinta2);
        y += 24.0f;
        y += p.parrafo(l, Letra::Lora18, paso->texto, xc, y, kColumna, 24.0f, Color::kTinta, 3);
        if (sitios.empty())
            y += p.parrafo(l, Letra::Lora16, "No se cumple en este sitio.", xc, y + 4.0f, kColumna, 22.0f, Color::kTinta2, 2) + 4.0f;
    }
    y += 12.0f;
    p.tituloBloque(l, "CURSOR", xc, y, kColumna);
    y += 30.0f;
    std::string aqui = "Nada marcado aquí.";
    for (const auto& [donde, nombre] : nombres)
        if (std::abs(donde.x - m_mapa.cursor.x) + std::abs(donde.y - m_mapa.cursor.y) <= 1) aqui = "Entrada: " + nombre;
    for (const Game::Chincheta& c : chinchetasAqui())
        if (c.x == m_mapa.cursor.x && c.y == m_mapa.cursor.y) aqui = "Chincheta " + std::to_string(c.numero) + ". ENTER la quita.";
    y += p.parrafo(l, Letra::Lora18, aqui, xc, y, kColumna, 24.0f, Color::kTinta, 2);
    if (!m_mapa.aviso.empty()) {
        p.pixel(l, "!", xc, y + 8.0f, 2, Color::kAviso);
        p.parrafo(l, Letra::Lora16, m_mapa.aviso, xc + 16.0f, y + 4.0f, kColumna - 16.0f, 22.0f, Color::kTinta, 3);
    }

    // --- El pie: las teclas ---
    struct Ayuda {
        std::vector<std::string> teclas;
        const char* verbo;
    };
    const Ayuda ayudas[] = {{{"←", "↑", "→", "↓"}, "Mover"}, {{"ENTER"}, "Chincheta"}, {{"M", "ESC"}, "Cerrar"}};
    float total = 0.0f;
    for (const Ayuda& a : ayudas) total += p.anchoAyuda(a.teclas, a.verbo) + 24.0f;
    const std::string nota = "o haz clic con el ratón";
    total += p.anchoTexto(Letra::Lora16, nota);
    float cx = std::round(x0 + (w - total) * 0.5f);
    const float yp = y0 + h - 48.0f;
    for (const Ayuda& a : ayudas) cx += p.ayuda(l, a.teclas, a.verbo, cx, yp, Color::kTinta2) + 24.0f;
    p.texto(l, Letra::Lora16, nota, cx, yp + 2.0f, 26.0f, Color::kTinta2);
}

}  // namespace Pergamino
