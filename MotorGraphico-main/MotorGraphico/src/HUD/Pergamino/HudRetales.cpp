// PERGAMINO-01 — ver include/HUD/Pergamino/HudRetales.h.
//
// Medidas de la maqueta P03a (vista de 1280x720): retales a 24 px del borde,
// personaje de 290 de ancho con 22 de margen, objetivo debajo con su
// chincheta, minimapa de 256x266 arriba a la derecha (terreno de 224x136, 4
// px por celda), ayudas de 246x52 abajo a la derecha y el aviso de objetivo
// centrado arriba (460x94).
#include "HUD/Pergamino/HudRetales.h"

#include <algorithm>
#include <cmath>

#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {

constexpr float kMargen = 24.0f;
constexpr float kAnchoRetal = 290.0f;
constexpr float kAltoPersonaje = 122.0f;
constexpr float kDuracionAviso = 5.0f;
constexpr float kCeldaMinimapa = 4.0f;

int par(float v) { return static_cast<int>(std::ceil(v / 2.0f) * 2.0f); }

float pintaPersonaje(Lienzo& l, const Pincel& p, const DatosExploracion& d) {
    const float x = kMargen + d.temblor.x, y = kMargen + d.temblor.y;
    // AV14-02 — Te han pegado: detras de la hoja, otra mas grande tenida de carmin, que asoma por
    // el borde rasgado (el marco de la hoja, no un cuadrado encima), y se apaga con el golpe.
    if (d.golpe > 0.0f)
        l.material(hoja(static_cast<int>(kAnchoRetal) + 20, static_cast<int>(kAltoPersonaje) + 20, 12, 4, 0, 0),
                   Vector2{x - 10.0f, y - 10.0f}, Vector4{0.78f, 0.12f, 0.08f, std::min(1.0f, d.golpe * 1.2f)});
    l.materialConSombra(hoja(static_cast<int>(kAnchoRetal), static_cast<int>(kAltoPersonaje), 11, 3, 0, 2),
                        Vector2{x, y}, Color::kSombra);
    const float xc = x + 22.0f, fila = y + 18.0f;
    const Letra letraNombre = d.compacto ? Letra::Lora20b : Letra::Lora22b;
    const float anchoOro = d.compacto ? 0.0f : 16.0f + 6.0f + p.anchoPixel(std::to_string(d.oro), 2);
    const std::string tier = "T" + std::to_string(d.tier);
    float maxNombre = kAnchoRetal - 44.0f - anchoOro - p.anchoInsignia(tier) - 20.0f;
    // La insignia de la pasiva va detras del tier (la voz pixel ya sale en
    // mayusculas); si con su espacio no cabe el nombre, el rombo va pegado.
    std::string insignia;
    if (!d.insignia.empty()) {
        const float anchoNombre = static_cast<float>(p.letras().ancho(letraNombre, d.nombre));
        insignia = "◆ " + d.insignia;
        if (anchoNombre > maxNombre + 4.0f - p.anchoInsignia(insignia)) insignia = "◆" + d.insignia;
        maxNombre += 4.0f - p.anchoInsignia(insignia);
    }
    const float wNombre = p.texto(l, letraNombre, p.letras().recorta(letraNombre, d.nombre, maxNombre), xc,
                                  fila, 26.0f, Color::kTinta);
    const float xTier = xc + wNombre + 8.0f;
    const float wTier = p.insignia(l, tier, xTier, fila, Color::kTinta2);
    if (!insignia.empty()) {
        const float xi = xTier + wTier + 8.0f;
        p.insignia(l, insignia, xi, fila, Color::kTinta2);
        p.pixel(l, "◆", xi + 8.0f, fila + 4.0f, 2, Color::kInfo);
    }
    if (!d.compacto) {
        const std::string oro = std::to_string(d.oro);
        const float xo = x + kAnchoRetal - 22.0f - p.anchoPixel(oro, 2);
        p.pixel(l, oro, xo, fila + 4.0f, 2, Color::kTinta);
        p.icono(l, "oro", xo - 22.0f, fila + 5.0f, 1);
    }
    // Vida: icono, sigla y cifra ENCIMA; la barra debajo no las tapa.
    const float yv = fila + 26.0f + 12.0f;
    p.icono(l, "pv", xc, yv + 7.0f, 1);
    p.pixel(l, "PV", xc + 24.0f, yv + 6.0f, 2, Color::kTinta2);
    const bool poca = d.pvMax > 0 && d.pv * 4 <= d.pvMax;
    const std::string cifra = std::to_string(d.pv) + "/" + std::to_string(d.pvMax);
    const float xCifra = xc + 242.0f - p.anchoPixel(cifra, 2);
    p.pixel(l, cifra, xCifra, yv + 6.0f, 2, poca || d.golpe > 0.4f ? Color::kAviso : Color::kTinta);
    if (poca) p.pixel(l, "!", xCifra - 18.0f, yv + 6.0f, 2, Color::kAviso);
    const Rect barra{xc, yv + 34.0f, 242.0f, 10.0f};
    p.barra(l, barra, d.pvMax > 0 ? static_cast<float>(d.pv) / static_cast<float>(d.pvMax) : 0.0f, Color::kPv);
    // AV14-02 — La vida que se acaba de perder, en carmin claro, hasta que la barra la alcanza.
    if (d.pvFantasma > static_cast<float>(d.pv) && d.pvMax > 0) {
        const float a = barra.w * static_cast<float>(d.pv) / static_cast<float>(d.pvMax);
        const float b = barra.w * std::min(1.0f, d.pvFantasma / static_cast<float>(d.pvMax));
        l.color(Rect{barra.x + a, barra.y, std::max(0.0f, b - a), barra.h}, Vector4{0.93f, 0.55f, 0.45f, 0.9f});
    }
    return y + kAltoPersonaje;
}

// H3-06 — Lo que miden los temporizadores: una fila por reloj (28 px) y su
// barra si la lleva (12 px mas).
float altoRelojes(const DatosExploracion& d) {
    float h = 0.0f;
    for (const Game::RelojVisible& r : d.relojes) h += r.barra ? 40.0f : 28.0f;
    return h;
}

// El nombre del reloj («El incendio»; «TIEMPO» si no tiene) y lo que queda
// («1:30»; en rojo el ultimo cuarto, o el ultimo medio minuto si no hay
// total) y debajo la barra si la pide.
void pintaRelojes(Lienzo& l, const Pincel& p, const DatosExploracion& d, float xc, float y) {
    for (const Game::RelojVisible& r : d.relojes) {
        const bool poco = r.total > 0.0f ? r.restante <= r.total * 0.25f : r.restante <= 30.0f;
        const std::string cuenta = Game::formatoReloj(r.restante);
        const float anchoCuenta = p.anchoPixel(cuenta, 2);
        if (r.nombre.empty()) {
            p.pixel(l, "TIEMPO", xc, y + 6.0f, 2, Color::kTinta2);
        } else {
            // Un nombre propio va en Lora, que cabe mas que la voz pixel.
            p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, r.nombre, 246.0f - anchoCuenta - 12.0f), xc,
                    y + 1.0f, 22.0f, Color::kTinta2);
        }
        p.pixel(l, cuenta, xc + 246.0f - anchoCuenta, y + 6.0f, 2, poco ? Color::kError : Color::kTinta);
        y += 28.0f;
        if (r.barra) {
            p.barra(l, Rect{xc, y, 246.0f, 8.0f}, std::clamp(r.restante / r.total, 0.0f, 1.0f),
                    poco ? Color::kError : Color::kAviso);
            y += 12.0f;
        }
    }
}

void pintaObjetivo(Lienzo& l, const Pincel& p, const DatosExploracion& d, float y) {
    const float x = kMargen, xc = x + 22.0f;
    if (d.paso.empty()) {
        // Un temporizador sin objetivo seguido: su retal solo.
        const float alto = static_cast<float>(par(20.0f + altoRelojes(d) + 12.0f));
        l.materialConSombra(hoja(static_cast<int>(kAnchoRetal), static_cast<int>(alto), 12, 3, 0, 2), Vector2{x, y},
                            Color::kSombra);
        pintaRelojes(l, p, d, xc, y + 16.0f);
        return;
    }
    const auto lineas = p.letras().envuelve(Letra::Lora18, d.paso, 222.0f);
    const int n = std::clamp(static_cast<int>(lineas.size()), 1, 3);
    const float relojes = d.relojes.empty() ? 0.0f : altoRelojes(d) + 6.0f;
    const float alto = static_cast<float>(par(22.0f + 29.0f + 6.0f + 21.0f + 6.0f + 24.0f * n + 18.0f + relojes));
    l.materialConSombra(hoja(static_cast<int>(kAnchoRetal), static_cast<int>(alto), 12, 3, 0, 2), Vector2{x, y},
                        Color::kSombra);
    l.material(pieza(Material::Chincheta), Vector2{x + 139.0f, y + 12.0f});
    const float fila = y + 22.0f;
    p.icono(l, "mision_activa", xc, fila + 7.0f, 1);
    p.pixel(l, "OBJETIVO", xc + 24.0f, fila + 6.0f, 2, Color::kTinta2);
    if (d.total > 0) {
        const std::string cuenta = std::to_string(d.hechos) + "/" + std::to_string(d.total);
        p.pixel(l, cuenta, x + kAnchoRetal - 22.0f - p.anchoPixel(cuenta, 2), fila + 6.0f, 2, Color::kTinta3);
    }
    p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, d.mision, 246.0f), xc, fila + 35.0f, 21.0f,
            Color::kTinta2);
    const float yp = fila + 62.0f;
    p.icono(l, "objetivo", xc, yp + 4.0f, 1);
    p.parrafo(l, Letra::Lora18, d.paso, xc + 24.0f, yp, 222.0f, 24.0f, Color::kTinta, 3);
    if (!d.relojes.empty()) pintaRelojes(l, p, d, xc, yp + 24.0f * static_cast<float>(n) + 6.0f);
}

}  // namespace

Vector4 colorChincheta(int color) {
    const Vector4 colores[] = {Color::kInfo, Color::kExito, Color::kAviso};
    return colores[((color % 3) + 3) % 3];
}

void pintaMarcaMinimapa(Lienzo& l, const Pincel& p, const MarcaMinimapa& m, float x0, float y0, float celda) {
    const float cx = x0 + (m.x + 0.5f) * celda;
    const float cy = y0 + (m.y + 0.5f) * celda;
    const char* glifo = "●";
    int escala = 1;
    Vector4 color = Color::kTinta2;
    switch (m.tipo) {
        case MarcaMinimapa::Tipo::Jugador: glifo = "▲"; escala = 2; color = Color::kTinta; break;
        case MarcaMinimapa::Tipo::Entrada: glifo = "■"; escala = 2; color = Color::kInfo; break;
        case MarcaMinimapa::Tipo::Enemigo: color = Color::kCarmin; break;
        case MarcaMinimapa::Tipo::Personaje: break;
        case MarcaMinimapa::Tipo::Objetivo: glifo = "◆"; escala = 2; color = Color::kCarmin; break;  // MAPA-01
        case MarcaMinimapa::Tipo::Chincheta: glifo = "▼"; escala = 2; color = colorChincheta(m.color); break;  // MAPA-02
    }
    // La forma visible ocupa las filas 1-5 de la celda (mas 2 de tilde).
    if (!m.emblema.empty() && p.iconos().frame(m.emblema) > 0) {  // AV15-02: el emblema del sitio
        p.icono(l, m.emblema, std::round(cx - 8.0f), std::round(cy - 8.0f), 1);
        return;
    }
    const float s = static_cast<float>(escala);
    const float gx = std::round(cx - 2.5f * s), gy = std::round(cy - 5.5f * s);
    const Vector4 borde = rgb(42, 26, 14);
    for (const Vector2& o : {Vector2{-1, 0}, Vector2{1, 0}, Vector2{0, -1}, Vector2{0, 1}})
        p.pixel(l, glifo, gx + o.x, gy + o.y, escala, borde);
    p.pixel(l, glifo, gx, gy, escala, color);
}

namespace {

void pintaMinimapa(Lienzo& l, const Pincel& p, const DatosExploracion& d, int anchoVista) {
    const float w = 256.0f;
    const float x = static_cast<float>(anchoVista) - kMargen - w, y = kMargen;
    // Un nivel pequeno (una sala de 9x9) no queda en un sello de 36 px: se
    // amplia x2 o x3, sin pasarse de la ventana de 224x136 de la ciudad.
    const float ampliado = std::floor(std::min(224.0f / (kCeldaMinimapa * static_cast<float>(d.ventanaAncho)),
                                               136.0f / (kCeldaMinimapa * static_cast<float>(d.ventanaAlto))));
    const float celda = kCeldaMinimapa * std::clamp(ampliado, 1.0f, 3.0f);
    const float anchoMapa = celda * static_cast<float>(d.ventanaAncho);
    const float altoMapa = celda * static_cast<float>(d.ventanaAlto);
    bool hayEnemigo = false, hayObjetivo = false, hayChincheta = false;
    for (const MarcaMinimapa& m : d.marcas) hayEnemigo = hayEnemigo || m.tipo == MarcaMinimapa::Tipo::Enemigo;
    for (const MarcaMinimapa& m : d.marcas) hayObjetivo = hayObjetivo || m.tipo == MarcaMinimapa::Tipo::Objetivo;
    for (const MarcaMinimapa& m : d.marcas) hayChincheta = hayChincheta || m.tipo == MarcaMinimapa::Tipo::Chincheta;
    // FASE1-BND-01 — El momento de la historia («Día 1 · Mañana») va en su linea,
    // debajo del nombre de la zona: al lado se lo comia (la zona quedaba en «Boundingt...»).
    const float lineaMomento = d.momento.empty() ? 0.0f : 22.0f;
    // MAPA-01: la zona entera, en dos renglones si hace falta («Casco Antiguo y
    // Distrito Comercial» se cortaba en «Casco Antiguo y Distr...»).
    auto renglonesZona = p.letras().envuelve(Letra::Lora18b, d.zona, 224.0f);
    if (renglonesZona.size() > 2) {
        renglonesZona[1] = p.letras().recorta(Letra::Lora18b, renglonesZona[1] + " " + renglonesZona[2], 224.0f);
        renglonesZona.resize(2);
    }
    const float zonaExtra = renglonesZona.size() > 1 ? 24.0f : 0.0f;
    const float lineaCerca = d.lugarCerca.empty() ? 0.0f : 26.0f;  // AV15-02
    // Leyenda: solo lo que se dibuja (la interfaz no inventa). MAPA-02: el retal crece con sus renglones.
    struct Clave { const char* glifo; Vector4 color; const char* texto; };
    std::vector<Clave> claves = {{"▲", Color::kTinta, "Tú"}, {"●", Color::kTinta2, "Personaje"},
                                 {"■", Color::kInfo, "Entrada"}};
    if (hayEnemigo) claves.push_back({"●", Color::kCarmin, "Enemigo"});
    if (hayObjetivo) claves.push_back({"◆", Color::kCarmin, "Objetivo"});
    if (hayChincheta) claves.push_back({"▼", Color::kInfo, "Chincheta"});  // MAPA-02
    auto anchoClave = [&](const Clave& c) { return 10.0f + 6.0f + static_cast<float>(p.letras().ancho(Letra::Lora16, c.texto)); };
    int renglones = 1;
    float enRenglon = 0.0f;
    for (const Clave& c : claves) {
        if (enRenglon > 0.0f && enRenglon + anchoClave(c) > 224.0f) enRenglon = 0.0f, ++renglones;
        enRenglon += anchoClave(c) + 14.0f;
    }
    const float altoLeyenda = 23.0f + 27.0f * static_cast<float>(renglones - 1);
    const float h = static_cast<float>(par(16.0f + 26.0f + 8.0f + zonaExtra + lineaMomento + altoMapa + 8.0f + altoLeyenda + lineaCerca + 12.0f));
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 13, 3, 0, 2), Vector2{x, y},
                        Color::kSombra);
    const float xc = x + 16.0f;
    for (std::size_t i = 0; i < renglonesZona.size(); ++i)
        p.texto(l, Letra::Lora18b, renglonesZona[i], xc, y + 16.0f + 24.0f * static_cast<float>(i), 26.0f, Color::kTinta);
    if (!d.momento.empty()) p.texto(l, Letra::Lora16, d.momento, xc, y + 41.0f + zonaExtra, 22.0f, Color::kTinta2);
    const float ym = y + 50.0f + zonaExtra + lineaMomento;
    const float xm = xc + std::round((224.0f - anchoMapa) * 0.5f);
    l.rect(Fuente::Minimapa, Rect{xm, ym, anchoMapa, altoMapa}, d.ventanaUV, Color::kBlanco);
    for (const MarcaMinimapa& m : d.marcas)
        if (m.tipo != MarcaMinimapa::Tipo::Jugador && m.tipo != MarcaMinimapa::Tipo::Objetivo) pintaMarcaMinimapa(l, p, m, xm, ym, celda);
    for (const MarcaMinimapa& m : d.marcas)  // MAPA-01: el objetivo, encima de las entradas
        if (m.tipo == MarcaMinimapa::Tipo::Objetivo) pintaMarcaMinimapa(l, p, m, xm, ym, celda);
    for (const MarcaMinimapa& m : d.marcas)
        if (m.tipo == MarcaMinimapa::Tipo::Jugador) pintaMarcaMinimapa(l, p, m, xm, ym, celda);
    float lx = xc, ly = ym + altoMapa + 8.0f;
    for (const Clave& c : claves) {
        const float wItem = anchoClave(c);
        if (lx > xc && lx + wItem > xc + 224.0f) {
            lx = xc;
            ly += 27.0f;
        }
        p.pixel(l, c.glifo, lx, ly + 3.0f, 2, c.color);
        p.texto(l, Letra::Lora16, c.texto, lx + 16.0f, ly, 23.0f, Color::kTinta2);
        lx += wItem + 14.0f;
    }
    if (!d.lugarCerca.empty())  // AV15-02: el sitio con nombre mas cercano
        p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, "Cerca: " + d.lugarCerca, 224.0f), xc, ly + 27.0f, 23.0f, Color::kTinta);
}

void pintaAyudas(Lienzo& l, const Pincel& p, int anchoVista, int altoVista) {
    // MAPA-02: y «M Mapa» (el retal crece lo que haga falta, en pasos de 4).
    const float total = p.anchoAyuda({"TAB"}, "Menú") + 18.0f + p.anchoAyuda({"M"}, "Mapa") + 18.0f +
                        p.anchoAyuda({"ESC"}, "Pausa");
    const float w = std::max(246.0f, 4.0f * std::ceil((total + 40.0f) / 4.0f)), h = 52.0f;
    const float x = static_cast<float>(anchoVista) - kMargen - w;
    const float y = static_cast<float>(altoVista) - 20.0f - h;
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 14, 2, 0, 0), Vector2{x, y},
                        Color::kSombra);
    float cursor = std::round(x + (w - total) * 0.5f);
    cursor += p.ayuda(l, {"TAB"}, "Menú", cursor, y + 12.0f, Color::kTinta2) + 18.0f;
    cursor += p.ayuda(l, {"M"}, "Mapa", cursor, y + 12.0f, Color::kTinta2) + 18.0f;
    p.ayuda(l, {"ESC"}, "Pausa", cursor, y + 12.0f, Color::kTinta2);
}

void pintaAviso(Lienzo& l, const Pincel& p, const DatosExploracion& d) {
    const float wVerbo = static_cast<float>(p.letras().ancho(Letra::Lora18, d.avisoVerbo + " "));
    const float wDestino = static_cast<float>(p.letras().ancho(Letra::Lora18b, d.avisoDestino));
    const float w = static_cast<float>(par(23.0f + 28.0f + 10.0f + wVerbo + wDestino + 23.0f));
    const float h = 46.0f;
    const float x = std::round(d.avisoAncla.x - w * 0.5f);
    const float y = std::round(d.avisoAncla.y - h - 8.0f);
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 16, 1, 0, 0), Vector2{x, y},
                        Color::kSombra);
    l.material(pieza(Material::Flecha), Vector2{std::round(d.avisoAncla.x) - 6.0f, y + h - 1.0f});
    p.tecla(l, "E", x + 23.0f, y + 9.0f);
    const float xt = x + 23.0f + 28.0f + 10.0f;
    p.texto(l, Letra::Lora18, d.avisoVerbo, xt, y + 10.0f, 26.0f, Color::kTinta);
    p.texto(l, Letra::Lora18b, d.avisoDestino, xt + wVerbo, y + 10.0f, 26.0f, Color::kTinta);
}

}  // namespace

void HudRetales::observaObjetivo(const std::string& paso) {
    if (paso == m_ultimoPaso) return;
    m_ultimoPaso = paso;
    if (paso.empty()) return;
    m_avisoTexto = paso;
    m_avisoRestante = kDuracionAviso;
}

void HudRetales::avanza(float segundos) {
    m_avisoRestante = std::max(0.0f, m_avisoRestante - segundos);
    m_lugarRestante = std::max(0.0f, m_lugarRestante - segundos);
}

void HudRetales::anunciaLugar(const std::string& nombre, const std::string& funcion, const std::string& emblema) {
    m_lugarNombre = nombre, m_lugarFuncion = funcion, m_lugarEmblema = emblema;
    m_lugarRestante = nombre.empty() ? 0.0f : kDuracionAviso * 0.7f;
}

void HudRetales::pinta(Lienzo& l, const Pincel& p, const DatosExploracion& d, int anchoVista,
                       int altoVista) const {
    const float abajo = pintaPersonaje(l, p, d);
    if (d.compacto) return;
    if (!d.paso.empty() || !d.relojes.empty()) pintaObjetivo(l, p, d, abajo + 12.0f);
    if (d.minimapa) pintaMinimapa(l, p, d, anchoVista);
    pintaAyudas(l, p, anchoVista, altoVista);
    if (!d.avisoDestino.empty()) pintaAviso(l, p, d);
    if (m_avisoRestante > 0.0f) {
        const std::size_t desde = l.cuantos();
        const float w = 460.0f, h = 94.0f;
        const float x = std::round((static_cast<float>(anchoVista) - w) * 0.5f), y = kMargen;
        l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 15, 2, 0, 1), Vector2{x, y},
                            Color::kSombra);
        p.icono(l, "mision_activa", x + 20.0f, y + 31.0f, 2);
        p.texto(l, Letra::Lora18b, "Nuevo objetivo", x + 64.0f, y + 14.0f, 23.0f, Color::kTinta);
        p.parrafo(l, Letra::Lora18, m_avisoTexto, x + 64.0f, y + 37.0f, 376.0f, 23.0f, Color::kTinta2, 2);
        // Se funde el ultimo medio segundo.
        l.atenua(desde, std::min(1.0f, m_avisoRestante / 0.5f));
    }
    if (m_lugarRestante > 0.0f) {  // AV15-02: el rotulo del sitio al entrar (bajo el aviso si lo hay)
        const std::size_t desde = l.cuantos();
        const float wNombre = static_cast<float>(p.letras().ancho(Letra::Lora24b, m_lugarNombre));
        const float w = std::max(320.0f, wNombre + 120.0f), h = 84.0f;
        const float x = std::round((static_cast<float>(anchoVista) - w) * 0.5f);
        const float y = m_avisoRestante > 0.0f ? kMargen + 94.0f + 10.0f : kMargen;
        l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 16, 2, 0, 1), Vector2{x, y}, Color::kSombra);
        if (!m_lugarEmblema.empty() && p.iconos().frame(m_lugarEmblema) > 0) p.icono(l, m_lugarEmblema, x + 22.0f, y + 26.0f, 2);
        p.texto(l, Letra::Lora24b, m_lugarNombre, std::round(x + (w - wNombre) * 0.5f), y + 12.0f, 32.0f, Color::kTinta);
        const float wf = static_cast<float>(p.letras().ancho(Letra::Lora18, m_lugarFuncion));
        p.texto(l, Letra::Lora18, m_lugarFuncion, std::round(x + (w - wf) * 0.5f), y + 46.0f, 24.0f, Color::kTinta2);
        l.atenua(desde, std::min(1.0f, m_lugarRestante / 0.5f));
    }
}

}  // namespace Pergamino
