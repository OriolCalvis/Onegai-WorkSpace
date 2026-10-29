// PERGAMINO-01 — ver include/HUD/Pergamino/Cuaderno.h.
//
// Geometria de la maqueta (vista de 1280x720): tapas de 1120x600 en 80,60;
// hoja de arriba de 1056x446 en 112,156, con la bisagra en su borde de abajo
// (y = 602); dos hojas debajo asomando 7 y 14 px; pestanas desde x = 120,
// de 44 + 12·letras + 16 px, la activa 8 px mas alta; lomo de 14 px sobre la
// bisagra con tres remaches; cabecera y pie sobre el cuero.
#include "HUD/Pergamino/Cuaderno.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {

struct Seccion {
    const char* etiqueta;
    const char* icono;
};
const Seccion kSecciones[Cuaderno::kHojas] = {
    {"INVENTARIO", "equipo"}, {"EQUIPO", "ranura_torso"}, {"FICHA", "personaje"},
    {"ACCIONES", "dote"},     {"MISIONES", "mision_activa"}, {"CÓDICE", "lore"}};

constexpr int kTiras = 28;

PedidoMaterial papelHoja() { return hoja(1056, 446, 40, 1, 6, 2); }
PedidoMaterial dorsoHoja() { return hoja(1056, 446, 77, 1, 0, 5, 22); }

Vector4 gris(float brillo, float alfa = 1.0f) { return Vector4{brillo, brillo, brillo, alfa}; }

void quadHoja(Lienzo& l, Fuente fuente, const Rect& r, const Vector4& color) {
    l.rect(fuente, r, Rect{0.0f, 0.0f, 1.0f, 1.0f}, color);
}

void tirasHoja(Lienzo& l, const std::vector<Game::TiraHoja>& tiras, Fuente fuente, int material,
               const Vector4& color) {
    for (const Game::TiraHoja& t : tiras) {
        Trazo tr;
        tr.fuente = fuente;
        tr.material = material;
        for (int i = 0; i < 4; ++i) tr.p[i] = t.p[i];
        tr.uv[0] = {0.0f, t.v0};
        tr.uv[1] = {1.0f, t.v0};
        tr.uv[2] = {1.0f, t.v1};
        tr.uv[3] = {0.0f, t.v1};
        tr.color = color;
        l.trazo(tr);
    }
}

// Sombra que la hoja en movimiento echa sobre la de debajo: degradado de la
// bisagra hacia arriba (.62 -> .18 al 45% -> 0 al 80%), en bandas.
void sombraBisagra(Lienzo& l, const Rect& h, float opacidad) {
    if (opacidad <= 0.01f) return;
    const int bandas = 24;
    for (int i = 0; i < bandas; ++i) {
        const float f = (static_cast<float>(i) + 0.5f) / bandas;  // 0 en la bisagra
        float a = 0.0f;
        if (f < 0.45f) a = 0.62f + (0.18f - 0.62f) * (f / 0.45f);
        else if (f < 0.80f) a = 0.18f * (1.0f - (f - 0.45f) / 0.35f);
        const float y1 = h.abajo() - h.h * static_cast<float>(i) / bandas;
        const float y0 = h.abajo() - h.h * static_cast<float>(i + 1) / bandas;
        l.color(Rect{h.x, std::round(y0), h.w, std::round(y1) - std::round(y0)}, rgb(38, 22, 8, a * opacidad));
    }
}

}  // namespace

Rect Cuaderno::tapas(int anchoVista, int altoVista) {
    return Rect{std::round((static_cast<float>(anchoVista) - kAncho) * 0.5f),
                std::round((static_cast<float>(altoVista) - kAlto) * 0.5f), kAncho, kAlto};
}

Rect Cuaderno::hoja(int anchoVista, int altoVista) {
    const Rect t = tapas(anchoVista, altoVista);
    return Rect{t.x + 32.0f, t.y + 96.0f, kHojaAncho, kHojaAlto};
}

void Cuaderno::abre(int foco) {
    m_foco = foco;
    m_vuelta.completa();
}

void Cuaderno::mira(int foco, bool animacionesReducidas) {
    if (foco == m_foco) return;
    m_vuelta.empieza(m_foco, foco, animacionesReducidas);
    m_foco = foco;
    ++m_vueltas;
}

void Cuaderno::pintaHoja(Lienzo& l, const Pincel& p, const PaginaCuaderno& pagina, int numero) const {
    l.material(papelHoja(), Vector2{0.0f, 0.0f});
    pintaPagina(l, p, pagina, kHojaAncho, kHojaAlto, numero, kHojas);
}

void Cuaderno::pintaPila(Lienzo& l, const Rect& h, float recorteY) const {
    const bool enCurso = m_vuelta.enCurso();
    const bool delante = m_vuelta.haciaDelante();
    // Debajo queda la hoja de destino si la vuelta va hacia delante (la
    // actual cae y la descubre) y la de origen si va hacia atras (la
    // anterior sube y la tapa). En reposo, solo la viva.
    const Fuente debajo = (!enCurso || delante) ? Fuente::HojaViva : Fuente::HojaFoto;
    const Fuente encima = delante ? Fuente::HojaFoto : Fuente::HojaViva;
    quadHoja(l, debajo, h, Color::kBlanco);
    if (!enCurso) return;
    if (m_vuelta.reducido()) {
        quadHoja(l, encima, h, gris(1.0f, m_vuelta.opacidadReducida()));
        return;
    }
    sombraBisagra(l, h, m_vuelta.sombraDebajo());
    const float angulo = m_vuelta.angulo();
    const auto tiras = Game::PasarHoja::tiras(h.x, h.y, h.w, h.h, angulo, kTiras, recorteY);
    if (m_vuelta.muestraDorso())
        tirasHoja(l, tiras, Fuente::Material, l.pide(dorsoHoja()), gris(m_vuelta.brilloDorso()));
    else
        tirasHoja(l, tiras, encima, -1, gris(m_vuelta.brilloFrente()));
}

void Cuaderno::pintaMarco(Lienzo& l, const Pincel& p, const CabeceraCuaderno& cab,
                          const PaginaCuaderno& pagina, int anchoVista, int altoVista) const {
    l.color(Rect{0.0f, 0.0f, static_cast<float>(anchoVista), static_cast<float>(altoVista)}, Color::kVelo);
    const Rect t = tapas(anchoVista, altoVista);
    const Rect h = hoja(anchoVista, altoVista);
    l.materialConSombra(cuero(static_cast<int>(kAncho), static_cast<int>(kAlto), 5, true, false, true),
                        Vector2{t.x, t.y}, Color::kSombraCuero);

    // Cabecera sobre el cuero: nombre, tier, raza y clase; a la derecha la
    // vida y el oro. Sin mana (decision de Oriol del 29/09).
    const float yCab = t.y + 12.0f;
    float x = t.x + 32.0f;
    x += p.texto(l, Letra::Lora24b, cab.nombre, x, yCab + 5.0f, 24.0f, Color::kClaroCuero) + 14.0f;
    const std::string tier = "T" + std::to_string(cab.tier);
    const float wTier = p.anchoInsignia(tier);
    l.color(Rect{x - 2.0f, yCab + 2.0f, wTier + 4.0f, 30.0f}, Color::kLinea);
    l.color(Rect{x, yCab + 4.0f, wTier, 26.0f}, Color::kPapelLuz);
    p.pixel(l, tier, x + 8.0f, yCab + 8.0f, 2, Color::kTinta);
    x += wTier + 14.0f;
    float xd = t.derecha() - 32.0f;
    const std::string oro = std::to_string(cab.oro);
    xd -= p.anchoPixel(oro, 2);
    p.pixel(l, oro, xd, yCab + 8.0f, 2, Color::kClaroCuero);
    p.icono(l, "oro", xd - 22.0f, yCab + 9.0f, 1);
    xd -= 22.0f + 16.0f;
    const std::string pv = std::to_string(cab.pv) + "/" + std::to_string(cab.pvMax);
    xd -= p.anchoPixel(pv, 2);
    p.pixel(l, pv, xd, yCab + 8.0f, 2, Color::kClaroCuero);
    p.icono(l, "pv", xd - 24.0f, yCab + 9.0f, 1);
    xd -= 24.0f + 16.0f;
    p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, cab.subtitulo, xd - x), x, yCab + 5.0f,
            25.0f, Color::kClaroCuero2);

    // Pie sobre el cuero: los verbos son los de la hoja de destino.
    const float yPie = t.abajo() - 42.0f;
    float xp = t.x + 32.0f;
    xp += p.ayuda(l, {"←", "→"}, "Pasar hoja", xp, yPie, Color::kClaroCuero2) + 24.0f;
    if (!pagina.elegir.empty()) xp += p.ayuda(l, {"W", "S"}, pagina.elegir, xp, yPie, Color::kClaroCuero2) + 24.0f;
    if (!pagina.verbo.empty()) p.ayuda(l, {"ENTER"}, pagina.verbo, xp, yPie, Color::kClaroCuero2);
    const float wTab = p.anchoAyuda({"TAB"}, "Cerrar");
    p.ayuda(l, {"TAB"}, "Cerrar", t.derecha() - 32.0f - wTab, yPie, Color::kClaroCuero2);

    // Pestanas: las inactivas detras de la pila; la activa, delante.
    struct Pestana { Rect r; bool activa; int i; };
    std::vector<Pestana> pestanas;
    float xt = t.x + 40.0f;
    for (int i = 0; i < kHojas; ++i) {
        const std::string et = kSecciones[i].etiqueta;
        float w = 44.0f + 12.0f * static_cast<float>(decodificaUtf8(et).size()) + 16.0f;
        w += std::fmod(w, 2.0f);
        const bool activa = i == m_foco;
        const float y = activa ? t.y + 58.0f : t.y + 62.0f;
        float alto = h.y - y + (activa ? 2.0f : -2.0f);
        alto += std::fmod(alto, 2.0f);
        pestanas.push_back({Rect{xt, y, w, alto}, activa, i});
        xt += w + 6.0f;
    }
    auto pintaPestana = [&](const Pestana& pe) {
        const PedidoMaterial m = pestana(static_cast<int>(pe.r.w), static_cast<int>(pe.r.h), pe.activa, 30 + pe.i);
        if (pe.activa) l.material(m, Vector2{pe.r.x, pe.r.y});
        else l.materialConSombra(m, Vector2{pe.r.x, pe.r.y}, rgb(26, 16, 8, 0.45f));
        const std::string et = kSecciones[pe.i].etiqueta;
        const float contenido = 16.0f + 8.0f + p.anchoPixel(et, 2);
        const float xc = std::round(pe.r.x + (pe.r.w - contenido) * 0.5f);
        const float centro = t.y + 78.0f;  // el mismo para activas e inactivas
        const Vector4 c = pe.activa ? Color::kTinta : Color::kTinta2;
        p.icono(l, kSecciones[pe.i].icono, xc, centro - 8.0f, 1);
        p.pixel(l, et, xc + 24.0f, centro - 9.0f, 2, c);
    };
    for (const Pestana& pe : pestanas)
        if (!pe.activa) pintaPestana(pe);

    // La pila: dos hojas por debajo asoman a la derecha y abajo.
    l.materialConSombra(Pergamino::hoja(1056, 440, 41, 1, 0, 4), Vector2{h.x + 14.0f, h.y + 6.0f}, Color::kSombra);
    l.materialConSombra(Pergamino::hoja(1056, 443, 42, 1, 0, 4), Vector2{h.x + 7.0f, h.y + 3.0f}, Color::kSombra);
    // Sombra de la hoja de arriba (la textura viva ya lleva su canto).
    l.rect(Fuente::HojaViva, Rect{h.x, h.y + 4.0f, h.w, h.h}, Rect{0, 0, 1, 1}, Color::kSombra);
    pintaPila(l, h, t.abajo() - 4.0f);

    for (const Pestana& pe : pestanas)
        if (pe.activa) pintaPestana(pe);

    // Lomo: la bisagra, con sus tres remaches, por encima de todo.
    const float yLomo = h.abajo() - 4.0f;
    l.color(Rect{h.x - 8.0f, yLomo - 2.0f, h.w + 16.0f, 2.0f}, rgb(0, 0, 0, 0.25f));
    l.material(lomo(static_cast<int>(h.w) + 16, 14), Vector2{h.x - 8.0f, yLomo});
    for (float xr : {24.0f, h.w * 0.5f - 4.0f, h.w - 32.0f})
        l.material(pieza(Material::Tachuela), Vector2{h.x - 8.0f + xr - 2.0f, yLomo + 1.0f});
}

}  // namespace Pergamino
