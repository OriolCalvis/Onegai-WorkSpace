// PERGAMINO-01 — ver include/HUD/Pergamino/Papiro.h.
//
// Medidas de la maqueta P05a (papiro de 480 x 640 con siete entradas):
// titulo en pixel x3 a 42 px del borde, filete de 300, filas de 44 + 2 con
// el texto en Lora 22 a 48 px del margen, filete de 260 entre grupos, linea
// de estado con sello y pie de teclas centrado, los dos contados desde abajo.
#include "HUD/Pergamino/Papiro.h"

#include <algorithm>
#include <cmath>

#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {
constexpr float kMargen = 44.0f;       // del borde del papiro al contenido
constexpr float kFila = 44.0f;
constexpr float kHueco = 2.0f;
constexpr float kSeparador = 32.0f;    // 10 + filete de 10 + 12 (tras el hueco de la fila)
constexpr float kCabecera = 103.0f;    // titulo + filete + hueco
// Medidas de P05a contadas desde el borde de abajo del papiro (rodillo
// incluido): la linea de estado empieza a 131 y las teclas del pie a 68.
constexpr float kEstado = 131.0f;
constexpr float kPie = 68.0f;
constexpr float kHuecoEstado = 52.0f;  // de la ultima fila a la linea de estado
constexpr float kMargenVista = 16.0f;  // el papiro nunca toca el borde de la vista
// UI-BG3-01
constexpr float kMotivo = 46.0f;       // dos lineas de Lora 16 cursiva bajo una entrada apagada
constexpr float kSinTitulo = 40.0f;    // el titulo del juego va fuera del papiro
constexpr float kNota = 26.0f;         // «o haz clic con el raton»
// Las ayudas del pie repartidas en filas que caben en el ancho util.
struct FilasPie {
    std::vector<std::vector<std::size_t>> filas{1};
    std::vector<float> anchos{0.0f};
};
FilasPie repartePie(const Pincel& p, const DatosPapiro& d) {
    FilasPie r;
    const float anchoUtil = d.ancho - 2.0f * kMargen;
    for (std::size_t i = 0; i < d.pie.size(); ++i) {
        const float w = p.anchoAyuda(d.pie[i].teclas, d.pie[i].verbo);
        if (!r.filas.back().empty() && r.anchos.back() + 22.0f + w > anchoUtil) {
            r.filas.emplace_back();
            r.anchos.push_back(0.0f);
        }
        r.anchos.back() += (r.filas.back().empty() ? 0.0f : 22.0f) + w;
        r.filas.back().push_back(i);
    }
    return r;
}
constexpr float kFilaPie = 46.0f;
}  // namespace

float altoPapiro(const DatosPapiro& d, const Pincel* p) {
    float alto = kCabecera + (d.subtitulo.empty() ? 0.0f : 30.0f);
    if (d.titulo.empty()) alto = kSinTitulo;
    for (const EntradaPapiro& e : d.entradas)
        alto += kFila + kHueco + (e.separadorAntes ? kSeparador : 0.0f) + (e.motivo.empty() ? 0.0f : kMotivo);
    // La linea de estado se reserva aunque este vacia: el papiro no cambia de
    // alto al guardar.
    alto += kHuecoEstado + kEstado;
    if (!d.notaPie.empty()) alto += kNota;
    if (p != nullptr) alto += kFilaPie * static_cast<float>(repartePie(*p, d).filas.size() - 1);
    // Par, por el pixel logico x2 del material.
    const float par = std::ceil(alto / 2.0f) * 2.0f;
    return std::max(400.0f, par);
}

void pintaPapiro(Lienzo& l, const Pincel& p, const DatosPapiro& d, int anchoVista, int altoVista) {
    if (d.velo) l.color(Rect{0.0f, 0.0f, static_cast<float>(anchoVista), static_cast<float>(altoVista)}, Color::kVelo);
    // Si no cabe en la vista, se come el hueco de antes de la linea de estado.
    const float techo = d.arriba >= 0.0f ? static_cast<float>(altoVista) - d.arriba - kMargenVista
                                         : static_cast<float>(altoVista) - 2.0f * kMargenVista;
    const float alto = std::min(altoPapiro(d, &p), std::max(400.0f, techo));
    const FilasPie pie = repartePie(p, d);
    const float bajoPie = (d.notaPie.empty() ? 0.0f : kNota) + kFilaPie * static_cast<float>(pie.filas.size() - 1);
    const float x0 = std::round((static_cast<float>(anchoVista) - d.ancho) * 0.5f);
    const float y0 = d.arriba >= 0.0f ? d.arriba : std::round((static_cast<float>(altoVista) - alto) * 0.5f);
    l.materialConSombra(papiro(static_cast<int>(d.ancho), static_cast<int>(alto)), Vector2{x0, y0},
                        Color::kSombra);
    const float xc = x0 + kMargen;
    const float anchoUtil = d.ancho - 2.0f * kMargen;
    float cursor = y0 + kSinTitulo;
    if (!d.titulo.empty()) {
        // Titulo en pixel x3, centrado, y su filete.
        const float wt = p.anchoPixel(d.titulo, 3);
        p.pixel(l, d.titulo, std::round(x0 + (d.ancho - wt) * 0.5f), y0 + 42.0f, 3, Color::kTinta);
        l.material(adorno(300), Vector2{std::round(x0 + (d.ancho - 300.0f) * 0.5f), y0 + 79.0f});
        cursor = y0 + kCabecera;
    }
    if (!d.subtitulo.empty()) {
        const float ws = static_cast<float>(p.letras().ancho(Letra::Lora18, d.subtitulo));
        p.texto(l, Letra::Lora18, d.subtitulo, std::round(x0 + (d.ancho - ws) * 0.5f), cursor - 6.0f, 26.0f,
                Color::kTinta2);
        cursor += 30.0f;
    }
    for (std::size_t i = 0; i < d.entradas.size(); ++i) {
        const EntradaPapiro& e = d.entradas[i];
        if (e.separadorAntes) {
            l.material(adorno(260), Vector2{std::round(x0 + (d.ancho - 260.0f) * 0.5f), cursor + 10.0f});
            cursor += kSeparador;
        }
        const bool foco = static_cast<int>(i) == d.foco;
        const float altoFila = kFila + (e.motivo.empty() ? 0.0f : kMotivo);
        const Rect fila{xc, cursor, anchoUtil, altoFila};
        if (foco) p.filaFoco(l, fila);
        l.zona(fila, kZonaEntradaPapiro, static_cast<int>(i));
        p.texto(l, foco ? Letra::Lora22b : Letra::Lora22, e.texto, xc + 48.0f, cursor + 8.0f, 28.0f,
                e.apagada ? Color::kTinta3 : Color::kTinta);
        if (!e.motivo.empty())
            p.parrafo(l, Letra::Lora16i, e.motivo, xc + 48.0f, cursor + 38.0f, anchoUtil - 64.0f, 22.0f,
                      Color::kTinta3, 2);
        if (e.deslizador >= 0) {
            // Deslizador de 11 marcas (0..10); el relleno se ve siempre, el
            // carmin solo en la fila con foco (un solo foco).
            const float xs = xc + anchoUtil - 16.0f - 110.0f, ys = cursor + 21.0f;
            l.color(Rect{xs, ys, 110.0f, 2.0f}, Color::kTinta3);
            for (int m = 0; m <= 10; ++m)
                l.color(Rect{xs + 11.0f * static_cast<float>(m) - 1.0f, ys - 3.0f, 2.0f, 8.0f},
                        m <= e.deslizador ? (foco ? Color::kCarmin : Color::kTinta2) : Color::kTinta3);
            l.color(Rect{xs + 11.0f * static_cast<float>(e.deslizador) - 4.0f, ys - 7.0f, 8.0f, 16.0f},
                    Color::kLinea);
        } else if (!e.valor.empty()) {
            const float wv = static_cast<float>(p.letras().ancho(Letra::Lora18, e.valor));
            if (foco) {
                const float wf = p.anchoPixel("◀", 2);
                const float xv = xc + anchoUtil - 16.0f - wf - 8.0f - wv - 8.0f - wf;
                p.pixel(l, "◀", xv, cursor + 13.0f, 2, Color::kCarmin);
                p.texto(l, Letra::Lora18, e.valor, xv + wf + 8.0f, cursor + 9.0f, 26.0f, Color::kTinta);
                p.pixel(l, "▶", xv + wf + 8.0f + wv + 8.0f, cursor + 13.0f, 2, Color::kCarmin);
            } else {
                p.texto(l, Letra::Lora18, e.valor, xc + anchoUtil - 16.0f - wv, cursor + 9.0f, 26.0f,
                        Color::kTinta2);
            }
        }
        cursor += altoFila + kHueco;
    }
    // Linea de estado junto a la accion (E17): «Partida guardada» con su sello.
    const float yEstado = y0 + alto - kEstado - bajoPie;
    if (!d.estado.empty()) {
        if (!d.estadoError) l.material(pieza(Material::Sello), Vector2{xc + 12.0f, yEstado + 12.0f});
        else p.pixel(l, "!", xc + 18.0f, yEstado + 14.0f, 2, Color::kError);
        p.texto(l, Letra::Lora18, d.estado, xc + 46.0f, yEstado, 23.0f,
                d.estadoError ? Color::kError : Color::kExito);
        p.texto(l, Letra::Lora18, d.estadoDetalle, xc + 46.0f, yEstado + 24.0f, 23.0f, Color::kTinta2);
    }
    // Pie centrado: ENTER Aceptar · ESC Reanudar. UI-BG3-01: si no cabe en
    // una fila (el papiro del titulo es estrecho), sigue en la de abajo.
    float yp = y0 + alto - kPie - bajoPie;
    for (std::size_t f = 0; f < pie.filas.size(); ++f, yp += kFilaPie) {
        float xp = std::round(x0 + (d.ancho - pie.anchos[f]) * 0.5f);
        for (std::size_t k = 0; k < pie.filas[f].size(); ++k) {
            if (k > 0) xp += 22.0f;
            const AyudaPapiro& a = d.pie[pie.filas[f][k]];
            xp += p.ayuda(l, a.teclas, a.verbo, xp, yp, Color::kTinta2);
        }
    }
    if (!d.notaPie.empty()) {
        const float wn = static_cast<float>(p.letras().ancho(Letra::Lora16, d.notaPie));
        p.texto(l, Letra::Lora16, d.notaPie, std::round(x0 + (d.ancho - wn) * 0.5f), yp - 14.0f, 22.0f,
                Color::kTinta3);
    }
}

}  // namespace Pergamino
