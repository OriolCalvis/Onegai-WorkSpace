// UI-BG3-01 — ver include/HUD/Pergamino/PaginasCreacion.h. Aqui: el marco
// (cuero, pestanas, hoja, lomo y pie), los bloques que comparten las hojas
// (personaje, atributos, rejillas, ficha) y el reparto por hoja.
//
// Medidas de la maqueta C02a en 1280x720: cuero de 1248x696 en (16, 12);
// «NUEVA PARTIDA» en pixel x3 a (44, 26); pestanas desde x = 88 entre las
// teclas Q y E; hoja de 1204x530 en (38, 100) con su lomo abajo; pie a 660 con
// las teclas y CONTINUAR (146x46) a la derecha.
#include "HUD/Pergamino/PaginasCreacion.h"

#include <algorithm>
#include <cmath>

#include "Game/Personajes/MontajePersonajes.h"
#include "Game/Personajes/VisibilidadRasgos.h"
#include "PaginasCreacionInterno.h"

namespace Pergamino {

namespace creacion {

namespace {

struct Pestana {
    const char* etiqueta;
    const char* icono;
};
const Pestana kPestanas[Cuaderno::kHojas] = {
    {"RAZA", "personaje"}, {"CLASE", "tier"}, {"ASPECTO", "ranura_cabeza"}, {"ORIGEN", "lugar"},
    {"HISTORIA", "lore"},  {"DEIDAD", "deidad"},         {"NOMBRE", "mejora"}};

const char* const kStats[4] = {"CON", "DES", "INT", "CAR"};
const char* const kIconosStat[4] = {"con", "des", "int", "car"};

void pintaMarco(const Contexto& k, float ancho) {
    Lienzo& l = k.l;
    const Pincel& p = k.p;
    l.materialConSombra(cuero(1248, 696, 7, true, false, true), Vector2{16.0f, 12.0f}, Color::kSombraCuero);
    // Cabecera: «NUEVA PARTIDA · Paso 1 de 7 · Raza» y la campana a la derecha.
    const float wt = p.pixel(l, "NUEVA PARTIDA", 44.0f, 24.0f, 3, Color::kClaroCuero);
    const std::string paso = "Paso " + std::to_string(static_cast<int>(k.c.hoja()) + 1) + " de " +
                             std::to_string(Cuaderno::kHojas) + " · " + Cuaderno::nombreHoja(k.c.hoja());
    p.texto(l, Letra::Lora18, paso, 44.0f + wt + 22.0f, 24.0f, 28.0f, Color::kClaroCuero2);
    const std::string campana = "Los Perdidos de Boundington · Tier 1";
    const float wc = static_cast<float>(p.letras().ancho(Letra::Lora18, campana));
    p.texto(l, Letra::Lora18, campana, std::round(1236.0f - wc), 24.0f, 28.0f, Color::kClaroCuero2);

    // Pestanas entre Q y E: las de detras primero, la activa la ultima.
    p.tecla(l, "Q", 46.0f, 68.0f);
    float x = 88.0f;
    struct Caja {
        Rect r;
        int i;
    };
    std::vector<Caja> cajas;
    for (int i = 0; i < Cuaderno::kHojas; ++i) {
        const bool hecha = k.c.hecha(static_cast<Hoja>(i));
        float w = 16.0f + 16.0f + 8.0f + p.anchoPixel(kPestanas[i].etiqueta, 2) + (hecha ? 20.0f : 0.0f) + 16.0f;
        w = std::round(w / 2.0f) * 2.0f;
        const bool activa = i == static_cast<int>(k.c.hoja());
        const float y = activa ? 62.0f : 66.0f;
        cajas.push_back({Rect{x, y, w, kHojaY - y + (activa ? 4.0f : 0.0f)}, i});
        x += w + 8.0f;
    }
    p.tecla(l, "E", std::min(x + 4.0f, ancho - 60.0f), 68.0f);
    auto pinta = [&](const Caja& c) {
        const bool activa = c.i == static_cast<int>(k.c.hoja());
        const PedidoMaterial m = pestana(static_cast<int>(c.r.w), static_cast<int>(c.r.h), activa, 60 + c.i);
        if (activa) l.material(m, Vector2{c.r.x, c.r.y});
        else l.materialConSombra(m, Vector2{c.r.x, c.r.y}, rgb(26, 16, 8, 0.45f));
        const float xc = c.r.x + 16.0f;
        const float centro = 83.0f;
        p.icono(l, kPestanas[c.i].icono, xc, centro - 8.0f, 1);
        const float w = p.pixel(l, kPestanas[c.i].etiqueta, xc + 24.0f, centro - 9.0f, 2, Color::kTinta);
        if (k.c.hecha(static_cast<Hoja>(c.i))) p.pixel(l, "✓", xc + 24.0f + w + 6.0f, centro - 7.0f, 2, Color::kExito);
        l.zona(c.r, tipo(ZonaC::Pestana), c.i);
    };
    for (const Caja& c : cajas)
        if (c.i != static_cast<int>(k.c.hoja())) pinta(c);
    l.materialConSombra(hoja(static_cast<int>(kHojaW), static_cast<int>(kHojaH), 50, 1, 6, 2),
                        Vector2{kHojaX, kHojaY}, Color::kSombra);
    for (const Caja& c : cajas)
        if (c.i == static_cast<int>(k.c.hoja())) pinta(c);

    // Lomo con sus tres tachuelas.
    const float yLomo = kHojaY + kHojaH - 4.0f;
    l.material(lomo(static_cast<int>(kHojaW) + 16, 14), Vector2{kHojaX - 8.0f, yLomo});
    for (float xr : {24.0f, kHojaW * 0.5f - 4.0f, kHojaW - 32.0f})
        l.material(pieza(Material::Tachuela), Vector2{kHojaX - 8.0f + xr - 2.0f, yLomo + 1.0f});
}

void pintaPie(const Contexto& k) {
    Lienzo& l = k.l;
    const Pincel& p = k.p;
    const float y = 662.0f;
    const bool nombre = k.c.hoja() == Hoja::Nombre;
    float x = 46.0f;
    if (nombre) {
        x += p.ayuda(l, {"←", "→"}, "Sugerencia", x, y, Color::kClaroCuero2) + 24.0f;
        x += p.ayuda(l, {"ENTER"}, "Empezar", x, y, Color::kClaroCuero2) + 24.0f;
    } else {
        x += p.ayuda(l, {"Q", "E"}, "Paso", x, y, Color::kClaroCuero2) + 24.0f;
        x += p.ayuda(l, {"W", "A", "S", "D"}, "Moverte", x, y, Color::kClaroCuero2) + 24.0f;
        x += p.ayuda(l, {"ENTER"}, "Elegir", x, y, Color::kClaroCuero2) + 24.0f;
    }
    x += p.ayuda(l, {"ESC"}, k.c.hoja() == Hoja::Raza ? "Salir" : "Atrás", x, y, Color::kClaroCuero2) + 24.0f;
    const float limite = nombre ? 1236.0f : 1080.0f;
    if (!k.c.aviso().empty()) {
        p.pixel(l, "!", x, y + 5.0f, 2, Color::kClaroCuero);
        p.texto(l, Letra::Lora18, p.letras().recorta(Letra::Lora18, k.c.aviso(), limite - x - 20.0f), x + 18.0f,
                y + 1.0f, 26.0f, Color::kClaroCuero);
    } else {
        p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, "Ratón: encima para ver, clic para elegir",
                                                     limite - x),
                x, y + 2.0f, 24.0f, Color::kClaroCuero2);
    }
    if (!nombre) {
        const Rect r{1096.0f, 652.0f, 146.0f, 46.0f};
        boton(k, r, "CONTINUAR", true);
        l.zona(r, tipo(ZonaC::Continuar));
    }
}

}  // namespace

std::string frase(const std::string& s) {
    std::string out;
    bool primera = true;
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0xC3 && i + 1 < s.size()) {  // tildes mayusculas: À-Þ -> à-þ
            const unsigned char d = static_cast<unsigned char>(s[i + 1]);
            out += static_cast<char>(c);
            out += static_cast<char>(!primera && d >= 0x80 && d <= 0x9E ? d + 0x20 : d);
            ++i;
        } else {
            out += static_cast<char>(primera ? c : std::tolower(c));
        }
        primera = false;
    }
    return out;
}

int centrado(const Contexto& k, Letra letra, const std::string& s, float x, float y, float ancho, float alto,
             int maxLineas, const Vector4& color) {
    // Se parte solo por los espacios: una palabra larga («Cambiaformas») va
    // entera en su linea aunque roce el borde, en vez de partirse en dos.
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

void boton(const Contexto& k, const Rect& r, const std::string& texto, bool primario) {
    if (primario) {
        k.l.color(Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 4.0f}, Color::kCarmin2);
        k.l.color(r, Color::kCarmin);
        k.l.color(Rect{r.x, r.abajo() - 4.0f, r.w, 4.0f}, Color::kCarmin2);
    } else {
        k.l.color(Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 4.0f}, Color::kLinea);
        k.l.color(r, Color::kPapelLuz);
    }
    const float w = k.p.anchoPixel(texto, 2);
    k.p.pixel(k.l, texto, std::round(r.x + (r.w - w) * 0.5f), std::round(r.y + (r.h - 18.0f) * 0.5f) - 2.0f, 2,
              primario ? Color::kClaroCuero : Color::kTinta);
}

float chip(const Contexto& k, const std::string& texto, float x, float y, bool activo) {
    const float w = static_cast<float>(k.p.letras().ancho(Letra::Lora16, texto)) + 18.0f;
    const Rect r{x, y, w, 30.0f};
    k.l.color(r, activo ? Color::kTinta2 : rgb(242, 228, 193, 0.6f));
    k.p.contorno(k.l, r, 1.0f, activo ? Color::kTinta : rgb(107, 80, 56, 0.6f));
    k.p.texto(k.l, Letra::Lora16, texto, x + 9.0f, y + 3.0f, 24.0f, activo ? Color::kClaroCuero : Color::kTinta2);
    return w;
}

void sombraRombo(const Contexto& k, float cx, float cy, float a, float b) {
    k.l.color(Rect{cx - a * 0.7f, cy - b * 0.35f, a * 1.4f, b * 0.7f}, rgb(107, 80, 56, 0.10f));
    const Vector4 trazo = rgb(107, 80, 56, 0.75f);
    const int pasos = 18;
    for (int lado = 0; lado < 4; ++lado) {
        const float sx = lado == 0 || lado == 3 ? -1.0f : 1.0f;
        const float sy = lado < 2 ? -1.0f : 1.0f;
        for (int i = 0; i < pasos; i += 2) {
            const float t = static_cast<float>(i) / pasos;
            const float px = cx + sx * a * (1.0f - t), py = cy + sy * b * t;
            k.l.color(Rect{std::round(px) - 2.0f, std::round(py) - 1.0f, 4.0f, 2.0f}, trazo);
        }
    }
}

void pintaVista(const Contexto& k, const Cuaderno::Vista& v, float cx, float yPies, int escala, int orientacion) {
    if (v.vacia()) return;
    sombraRombo(k, cx, yPies, 74.0f, 22.0f);
    // de frente, quieto (1-based): el 8 de siempre; en diagonal, el suroeste respirando
    const Game::Reparto reparto = k.r.reparto();
    int frame = Game::frameQuieto(Game::Rumbo::Abajo, reparto);
    // DIALOGOS-01 (Oriol, 13/10): siempre de cara; solo gira con «Girar».
    if (k.c.rumbo() >= 0) frame = Game::frameQuieto(static_cast<Game::Rumbo>(k.c.rumbo()), reparto);
    if (orientacion >= 0) frame = Game::VisibilidadRasgos::frame(k.c.segundos(), static_cast<Game::Rumbo>(orientacion), reparto);
    const Rect uv = k.r.frame(v, frame - 1);
    if (uv.w <= 0.0f) return;
    const float lado = 64.0f * static_cast<float>(escala);
    // Los pies caen en el 90 % del alto del frame (la sombra de Entity).
    k.l.rect(Fuente::Retratos, Rect{std::round(cx - lado * 0.5f), std::round(yPies - lado * 0.9f), lado, lado}, uv,
             Color::kBlanco);
}

void pintaAtributos(const Contexto& k, const Cuaderno::Vista& v, float x, float y) {
    const float w = 58.0f, h = 86.0f, hueco = 8.0f;
    for (int i = 0; i < 4; ++i) {
        const Rect r{x + static_cast<float>(i) * (w + hueco), y, w, h};
        // AV16-02 (Oriol, 15/10): todo empieza a 0. Ni la raza ni el kit suman; como mucho se destacan
        // (marco carmin) los atributos principales de la clase.
        const bool principal = v.principales[static_cast<std::size_t>(i)];
        k.l.color(r, rgb(242, 228, 193, 0.55f));
        k.p.contorno(k.l, r, principal ? 2.0f : 1.0f, principal ? Color::kCarmin : rgb(107, 80, 56, 0.6f));
        k.p.icono(k.l, kIconosStat[i], r.x + 4.0f, r.y + 6.0f, 1);
        k.p.pixel(k.l, kStats[i], r.x + 22.0f, r.y + 5.0f, 2, Color::kTinta2);
        const std::string valor = principal ? "1" : "0";  // AV16-02b: +1 en los dos importantes de la clase
        const float wv = k.p.anchoPixel(valor, 3);
        k.p.pixel(k.l, valor, std::round(r.x + (w - wv) * 0.5f), r.y + 28.0f, 3, Color::kTinta);
        const std::string nota = principal ? "principal" : "";
        if (!nota.empty()) {
            const float wn = static_cast<float>(k.p.letras().ancho(Letra::Lora16, nota));
            k.p.texto(k.l, Letra::Lora16, nota, std::round(r.x + (w - wn) * 0.5f), r.y + 60.0f, 22.0f, Color::kCarmin);
        }
    }
}

void pintaCentro(const Contexto& k, const Cuaderno::Vista& v, float yPies, float yAtributos) {
    const float cx = kCentroX + kCentroW * 0.5f;
    if (v.vacia()) {
        centrado(k, Letra::Lora18i, "Elige tu raza y aquí aparecerás tú.", kCentroX, yPies - 120.0f, kCentroW, 26.0f,
                 3, Color::kTinta3);
    } else {
        if (k.c.hoja() == Hoja::Aspecto) {
            const int rumbo = k.c.rumbo() < 0 ? 2 : k.c.rumbo();
            const int opuesto = (rumbo + 2) % 4;
            pintaVista(k, v, cx - 64.0f, yPies, 2, rumbo);
            pintaVista(k, v, cx + 64.0f, yPies, 2, opuesto);
            centrado(k, Letra::Lora16, (rumbo == 1 || rumbo == 2) ? "Frente / Espalda" : "Espalda / Frente", kCentroX, yPies - 150.0f, kCentroW, 22.0f, 1, Color::kTinta2);
            const int f = Game::VisibilidadRasgos::frame(k.c.segundos(), static_cast<Game::Rumbo>(rumbo), k.r.reparto());
            const auto aviso = k.r.aviso(v, f - 1);
            if (!aviso.empty()) centrado(k, Letra::Lora16, aviso, kCentroX, yPies + 75.0f, kCentroW, 20.0f, 2, Color::kCarmin);
        } else pintaVista(k, v, cx, yPies, 3);
        // ◀ Girar ▶
        const float yg = yPies + 34.0f;
        const float wg = static_cast<float>(k.p.letras().ancho(Letra::Lora18, "Girar"));
        const float x0 = std::round(cx - (32.0f + 12.0f + wg + 12.0f + 32.0f) * 0.5f);
        const float w1 = k.p.tecla(k.l, "◀", x0, yg);
        k.l.zona(Rect{x0 - 2.0f, yg - 2.0f, w1 + 4.0f, 32.0f}, tipo(ZonaC::Girar), -1);
        k.p.texto(k.l, Letra::Lora18, "Girar", x0 + w1 + 12.0f, yg, 28.0f, Color::kTinta2);
        const float x2 = x0 + w1 + 12.0f + wg + 12.0f;
        const float w2 = k.p.tecla(k.l, "▶", x2, yg);
        k.l.zona(Rect{x2 - 2.0f, yg - 2.0f, w2 + 4.0f, 32.0f}, tipo(ZonaC::Girar), 1);
    }
    k.p.tituloBloque(k.l, "AL EMPEZAR", kCentroX, yAtributos - 30.0f, kCentroW);
    pintaAtributos(k, v, kCentroX, yAtributos);
}

void pintaRejilla(const Contexto& k, float x, float y, bool retratos, const char* icono) {
    const std::vector<std::string> ids = k.c.ids();
    const Hoja h = k.c.hoja();
    const int filas = Cuaderno::filasVisibles(h);
    const int primera = k.c.primeraFila();
    const std::string elegido = k.c.elegido(h);
    for (int fila = primera; fila < primera + filas; ++fila) {
        for (int col = 0; col < Cuaderno::kColumnas; ++col) {
            const int i = fila * Cuaderno::kColumnas + col;
            if (i >= static_cast<int>(ids.size())) break;
            const std::string& id = ids[static_cast<std::size_t>(i)];
            const Rect r{x + static_cast<float>(col) * (kTeselaW + kHueco),
                         y + static_cast<float>(fila - primera) * (kTeselaH + kHueco), kTeselaW, kTeselaH};
            const bool foco = i == k.c.foco();
            const bool elegida = id == elegido && (h == Hoja::Deidad ? k.c.hecha(Hoja::Deidad) : !id.empty());
            k.l.color(r, foco ? rgb(222, 196, 150, 0.85f) : rgb(242, 228, 193, 0.55f));
            k.p.contorno(k.l, r, foco ? 2.0f : 1.0f, foco ? Color::kCarmin : rgb(107, 80, 56, 0.45f));
            std::string nombre = "Ninguna";
            const RPG::CharacterCreation& reglas = *k.c.datos().reglas;
            if (h == Hoja::Raza) {
                if (const auto* d = reglas.races().find(id)) nombre = d->name;
            } else if (h == Hoja::Clase) {
                if (const auto* d = reglas.classes().find(id)) {
                    nombre = d->name;
                    // El icono de la esquina es su atributo principal.
                    const int s = d->primaryStat == "CON" ? 0 : d->primaryStat == "DES" ? 1
                                                            : d->primaryStat == "INT" ? 2 : 3;
                    k.p.icono(k.l, kIconosStat[s], r.derecha() - 20.0f, r.y + 4.0f, 1);
                }
            } else if (h == Hoja::Origen) {
                if (const auto* d = reglas.backgrounds().find(id)) nombre = d->name;
            } else if (h == Hoja::Deidad && !id.empty()) {
                if (const auto* d = reglas.deities().find(id)) nombre = d->name;
            }
            bool conRetrato = false;
            if (retratos) {
                const Rect uv = k.r.frame(k.c.retrato(h, id), Game::frameQuieto(Game::Rumbo::Abajo, k.r.reparto()) - 1);
                if (uv.w > 0.0f) {
                    k.l.rect(Fuente::Retratos, Rect{std::round(r.x + (r.w - 64.0f) * 0.5f), r.y + 2.0f, 64.0f, 64.0f},
                             uv, Color::kBlanco);
                    conRetrato = true;
                }
            }
            if (!conRetrato) {
                const char* ic = (h == Hoja::Deidad && id.empty()) ? "cerrar" : icono;
                k.p.icono(k.l, ic, std::round(r.x + (r.w - 32.0f) * 0.5f), r.y + 16.0f, 2);
            }
            centrado(k, Letra::Lora16, nombre, r.x + 2.0f, r.y + 62.0f, r.w - 4.0f, 17.0f, 2,
                     foco || elegida ? Color::kTinta : Color::kTinta2);
            if (elegida) k.l.material(pieza(Material::Cinta), Vector2{r.x + 8.0f, r.y - 4.0f});
            k.l.zona(r, tipo(ZonaC::Tesela), i);
        }
    }
    // La barra de la rejilla: cuanto se ve de cuanto hay.
    const int total = static_cast<int>(ids.size());
    const int filasTotal = (total + Cuaderno::kColumnas - 1) / Cuaderno::kColumnas;
    if (filasTotal > filas) {
        const float alto = static_cast<float>(filas) * (kTeselaH + kHueco) - kHueco;
        const float xb = x + 5.0f * (kTeselaW + kHueco) + 2.0f;
        k.l.color(Rect{xb, y, 4.0f, alto}, rgb(107, 80, 56, 0.25f));
        const float trozo = alto * static_cast<float>(filas) / static_cast<float>(filasTotal);
        const float desde = alto * static_cast<float>(primera) / static_cast<float>(filasTotal);
        k.l.color(Rect{xb, y + desde, 4.0f, trozo}, Color::kTinta2);
    }
}

void pintaCuenta(const Contexto& k, float x, float y, const char* que) {
    const int total = static_cast<int>(k.c.ids().size());
    const int filas = Cuaderno::filasVisibles(k.c.hoja());
    const int desde = k.c.primeraFila() * Cuaderno::kColumnas;
    const int hasta = std::min(total, desde + filas * Cuaderno::kColumnas);
    std::string t = total == 0 ? std::string("Nada que enseñar")
                               : std::to_string(desde + 1) + "–" + std::to_string(hasta) + " de " + std::to_string(total);
    if (hasta < total || desde > 0) t += std::string(" · ") + que;
    k.p.texto(k.l, Letra::Lora16, t, x, y, 24.0f, Color::kTinta3);
}

float fichaTitulo(const Contexto& k, const std::string& nombre, float y) {
    const std::vector<std::string> lineas = k.p.letras().envuelve(Letra::Lora24b, nombre, kFichaW);
    float yy = y;
    for (std::size_t i = 0; i < lineas.size() && i < 2; ++i, yy += 30.0f)
        k.p.texto(k.l, Letra::Lora24b, lineas[i], kFichaX, yy, 30.0f, Color::kTinta);
    return yy + 6.0f;
}

float fichaBloque(const Contexto& k, const std::string& rotulo, const std::string& titulo, const std::string& texto,
                  float y, int maxLineas) {
    k.p.tituloBloque(k.l, rotulo, kFichaX, y, kFichaW);
    float yy = y + 24.0f;
    if (!titulo.empty()) {
        k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, titulo, kFichaW), kFichaX, yy, 26.0f,
                  Color::kTinta);
        yy += 28.0f;
    }
    if (!texto.empty()) yy += k.p.parrafo(k.l, Letra::Lora18, texto, kFichaX, yy, kFichaW, 23.0f, Color::kTinta2, maxLineas);
    return yy + 10.0f;
}

}  // namespace creacion

Game::Reparto Retratista::reparto() const {
    return m_montaje == nullptr ? Game::Reparto::Clasico : Game::repartoDeColumnas(m_montaje->frames());
}

std::string Retratista::aviso(const Game::CuadernoCreacion::Vista& v, int frame) const {
    std::string base = v.apariencia.clave();
    for (const auto& [ranura, pieza] : v.equipo) base += "|" + ranura + "=" + pieza;
    return m_atlas.aviso(base + "#" + std::to_string(frame));
}

Rect Retratista::frame(const Game::CuadernoCreacion::Vista& v, int frame) {
    if (m_montaje == nullptr || v.vacia() || frame < 0 || frame >= m_montaje->frames()) return Rect{};
    std::string base = v.apariencia.clave();
    for (const auto& [ranura, pieza] : v.equipo) base += "|" + ranura + "=" + pieza;
    const std::string clave = base + "#" + std::to_string(frame);
    if (m_atlas.tiene(clave)) return m_atlas.celda(clave, nullptr);
    if (m_atlas.tira(base) == nullptr) {
        Game::MapaDeCapas mapa;
        auto hoja = m_montaje->montar(v.apariencia, v.equipo, &mapa);
        if (!hoja.isOk()) return Rect{};
        const auto avisos = Game::VisibilidadRasgos::avisos(v.apariencia, mapa, m_montaje->frames());
        for (std::size_t f = 0; f < avisos.size(); ++f) m_atlas.guardaAviso(base + "#" + std::to_string(f), avisos[f]);
        Imagen tira;
        tira.ancho = hoja.value().ancho;
        tira.alto = hoja.value().alto;
        tira.rgba = std::move(hoja.value().pixeles);
        m_atlas.guardaTira(base, std::move(tira));
    }
    const Imagen* tira = m_atlas.tira(base);
    return m_atlas.celda(clave, [&](Imagen& out) {
        copiaFrame(tira->rgba.data(), tira->ancho, frame, out);
        return true;
    });
}

void pintaCreacion(Lienzo& l, const Pincel& p, const Game::CuadernoCreacion& c, Retratista& retratos,
                   int anchoVista, int altoVista) {
    using namespace creacion;
    l.color(Rect{0.0f, 0.0f, static_cast<float>(anchoVista), static_cast<float>(altoVista)}, Color::kVelo);
    const std::size_t desdeTrazo = l.cuantos(), desdeZona = l.cuantasZonas();
    const Contexto k{l, p, c, retratos};
    pintaMarco(k, 1280.0f);
    switch (c.hoja()) {
        case Hoja::Raza: pintaRaza(k); break;
        case Hoja::Aspecto: pintaAspecto(k); break;
        case Hoja::Clase: pintaClase(k); break;
        case Hoja::Origen: pintaOrigen(k); break;
        case Hoja::Historia: pintaHistoria(k); break;
        case Hoja::Deidad: pintaDeidad(k); break;
        case Hoja::Nombre: pintaNombre(k); break;
    }
    pintaPie(k);
    // Maquetado a 1280x720: en una vista mayor va al centro.
    const float dx = std::max(0.0f, std::round((static_cast<float>(anchoVista) - 1280.0f) * 0.5f));
    const float dy = std::max(0.0f, std::round((static_cast<float>(altoVista) - 720.0f) * 0.5f));
    if (dx > 0.0f || dy > 0.0f) l.desplaza(desdeTrazo, desdeZona, dx, dy);
}

}  // namespace Pergamino
