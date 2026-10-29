// PERGAMINO-01 — ver include/HUD/Pergamino/PaginaCuaderno.h.
//
// Medidas de la maqueta P04a (cuaderno 1120x600 en 80,60; hoja de 1056x446):
// contenido con margen de 26/40/30/40, titulo entre filetes, lista de 360 px
// con cabecera, filas de 44 + 2, divisor punteado y detalle con 32 px de
// sangria. Todo en coordenadas de la hoja.
#include "HUD/Pergamino/PaginaCuaderno.h"

#include <algorithm>
#include <cmath>

#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {

constexpr float kMargenX = 40.0f;
constexpr float kArriba = 26.0f;
constexpr float kAbajo = 30.0f;
constexpr float kInicioRejilla = 54.0f;  // titulo (18) + 10 de hueco
constexpr float kFila = 44.0f;
constexpr float kHuecoFila = 2.0f;

Vector4 conAlfa(Vector4 c, float a) {
    c.w = a;
    return c;
}

Vector4 colorInsignia(TonoInsignia t) {
    switch (t) {
        case TonoInsignia::Exito: return Color::kExito;
        case TonoInsignia::Aviso: return Color::kAviso;
        case TonoInsignia::Info: return Color::kInfo;
        case TonoInsignia::Neutra: break;
    }
    return Color::kTinta2;
}

void pintaFila(Lienzo& l, const Pincel& p, const FilaPagina& f, float x, float y, float ancho,
               bool foco) {
    if (foco) p.filaFoco(l, Rect{x, y, ancho, kFila});
    float cursor = x + 36.0f + 20.0f * static_cast<float>(f.sangria);
    if (!f.icono.empty()) {
        const float lado = 16.0f * static_cast<float>(f.escalaIcono);
        p.icono(l, f.icono, cursor, y + std::round((kFila - lado) * 0.5f), f.escalaIcono);
        cursor += lado + 12.0f;
    }
    const Vector4 color = f.apagada ? Color::kTinta3 : Color::kTinta;
    float derecha = x + ancho - 12.0f;
    if (!f.valor.empty()) {
        if (f.valorPixel) {
            const float w = p.anchoPixel(f.valor, 2);
            p.pixel(l, f.valor, derecha - w, y + 13.0f, 2, f.hecha ? Color::kExito : Color::kTinta2);
            derecha -= w + 12.0f;
        } else {
            const float w = static_cast<float>(p.letras().ancho(Letra::Lora18i, f.valor));
            p.texto(l, Letra::Lora18i, f.valor, derecha - w, y + 10.0f, 25.0f, Color::kTinta3);
            derecha -= w + 12.0f;
        }
    }
    const Letra letra = foco ? Letra::Lora20b : Letra::Lora20;
    const std::string nombre = p.letras().recorta(letra, f.nombre, derecha - cursor);
    p.texto(l, letra, nombre, cursor, y + 10.0f, 25.0f, color);
}

float pintaDatos(Lienzo& l, const Pincel& p, const std::vector<DatoPagina>& datos, float x,
                 float y, float ancho, Letra letra = Letra::Lora18) {
    float clave = 0.0f;
    for (const DatoPagina& d : datos)
        clave = std::max(clave, static_cast<float>(p.letras().ancho(letra, d.clave)));
    const float xValor = x + clave + 24.0f;
    float cursor = y;
    for (const DatoPagina& d : datos) {
        p.texto(l, letra, d.clave, x, cursor, 26.0f, Color::kTinta3);
        const float h = p.parrafo(l, letra, d.valor, xValor, cursor, x + ancho - xValor, 25.0f,
                                  Color::kTinta, 2);
        cursor += std::max(26.0f, h) + 6.0f;
    }
    return cursor - y;
}

void pintaBoton(Lienzo& l, const Pincel& p, const std::string& etiqueta, bool activo, float x,
                float y, float& ancho) {
    ancho = p.anchoPixel(etiqueta, 2) + 40.0f;
    if (activo) {
        // Principal: lacre (carmin con texto claro) y contorno de tinta.
        l.color(Rect{x - 2.0f, y - 2.0f, ancho + 4.0f, 48.0f}, Color::kLinea);
        l.color(Rect{x, y, ancho, 44.0f}, Color::kCarmin);
        l.color(Rect{x, y + 40.0f, ancho, 4.0f}, Color::kCarmin2);
        p.pixel(l, etiqueta, x + 20.0f, y + 13.0f, 2, Color::kPapelLuz);
        return;
    }
    // Deshabilitado: contorno discontinuo (forma, no color) y tinta-3.
    for (float xx = x; xx < x + ancho; xx += 8.0f) {
        l.color(Rect{xx, y, std::min(4.0f, x + ancho - xx), 2.0f}, Color::kTinta3);
        l.color(Rect{xx, y + 42.0f, std::min(4.0f, x + ancho - xx), 2.0f}, Color::kTinta3);
    }
    for (float yy = y; yy < y + 44.0f; yy += 8.0f) {
        l.color(Rect{x, yy, 2.0f, std::min(4.0f, y + 44.0f - yy)}, Color::kTinta3);
        l.color(Rect{x + ancho - 2.0f, yy, 2.0f, std::min(4.0f, y + 44.0f - yy)}, Color::kTinta3);
    }
    p.pixel(l, etiqueta, x + 20.0f, y + 13.0f, 2, Color::kTinta3);
}

void pintaDetalle(Lienzo& l, const Pincel& p, const DetallePagina& d, float x, float y,
                  float ancho, float abajo) {
    float cursor = y;
    // Cabecera: icono x2, nombre en Lora 28 y la insignia.
    float xTitulo = x;
    if (!d.icono.empty()) {
        p.icono(l, d.icono, x, cursor, 2);
        xTitulo += 48.0f;
    }
    const float anchoInsignia = d.insignia.empty() ? 0.0f : p.anchoInsignia(d.insignia) + 16.0f;
    const auto lineas = p.letras().envuelve(Letra::Lora28b, d.titulo, x + ancho - xTitulo - anchoInsignia);
    for (std::size_t i = 0; i < std::min<std::size_t>(lineas.size(), 2); ++i)
        p.texto(l, Letra::Lora28b, lineas[i], xTitulo, cursor + 32.0f * static_cast<float>(i), 32.0f,
                Color::kTinta);
    if (!d.insignia.empty()) {
        const float anchoTitulo = lineas.empty() ? 0.0f : static_cast<float>(p.letras().ancho(Letra::Lora28b, lineas[0]));
        const float xi = lineas.size() > 1 ? x + ancho - anchoInsignia + 16.0f : xTitulo + anchoTitulo + 16.0f;
        const Vector4 c = colorInsignia(d.tono);
        p.insignia(l, d.insignia, xi, cursor + 3.0f, c, d.tono == TonoInsignia::Neutra ? Vector4{} : conAlfa(c, 0.10f));
    }
    cursor += 32.0f * static_cast<float>(std::max<std::size_t>(1, std::min<std::size_t>(lineas.size(), 2))) + 14.0f;
    if (!d.efecto.empty()) {
        float xe = x;
        if (!d.efectoIcono.empty()) {
            p.icono(l, d.efectoIcono, x, cursor + 7.0f, 1);
            xe += 24.0f;
        }
        p.texto(l, Letra::Lora20, d.efecto, xe, cursor + 2.0f, 25.0f, Color::kTinta);
        cursor += 29.0f + 14.0f;
    }
    if (!d.datos.empty()) cursor += pintaDatos(l, p, d.datos, x, cursor, ancho) + 8.0f;
    if (!d.bloque.empty()) {
        p.tituloBloque(l, d.bloque, x, cursor, ancho);
        cursor += 18.0f + 10.0f;
    }
    for (const FilaPagina& f : d.lista) {
        const float xi = x + 20.0f * static_cast<float>(f.sangria);
        const std::string icono = !f.icono.empty() ? f.icono : (f.hecha ? "objetivo_hecho" : "objetivo");
        p.icono(l, icono, xi, cursor + 5.0f, 1);
        const float h = p.parrafo(l, Letra::Lora18, f.nombre, xi + 24.0f, cursor, x + ancho - xi - 24.0f - 40.0f,
                                  25.0f, f.apagada || (f.hecha && f.icono.empty()) ? Color::kTinta3 : Color::kTinta,
                                  std::max(1, f.lineas));
        if (!f.valor.empty()) {
            const float w = p.anchoPixel(f.valor, 2);
            p.pixel(l, f.valor, x + ancho - w, cursor + 4.0f, 2, f.hecha ? Color::kExito : Color::kTinta3);
        }
        cursor += std::max(26.0f, h) + 6.0f;
        if (cursor > abajo - 60.0f) break;
    }
    for (const std::string& texto : d.parrafos) {
        cursor += p.parrafo(l, Letra::Lora20, texto, x, cursor, ancho, 30.0f, Color::kTinta2, 4) + 8.0f;
        if (cursor > abajo - 60.0f) break;
    }
    if (!d.nota.empty()) p.parrafo(l, Letra::Lora16i, d.nota, x, cursor + 4.0f, ancho, 22.0f, Color::kTinta3, 2);
    // Accion al pie del detalle: el boton y, al lado, su motivo o su resultado.
    const float yAccion = abajo - 44.0f;
    if (!d.estado.empty())
        p.parrafo(l, Letra::Lora18, d.estado, x, yAccion - 34.0f, ancho, 24.0f,
                  d.estadoError ? Color::kError : Color::kTinta2, 1);
    if (d.boton.empty()) return;
    float anchoBoton = 0.0f;
    pintaBoton(l, p, d.boton, d.botonActivo, x, yAccion, anchoBoton);
    l.zona(Rect{x, yAccion, anchoBoton, 44.0f}, kZonaBotonPagina);
    if (!d.motivo.empty()) {
        const float xm = x + anchoBoton + 16.0f;
        p.pixel(l, "!", xm, yAccion + 13.0f, 2, Color::kAviso);
        p.parrafo(l, Letra::Lora18, d.motivo, xm + 18.0f, yAccion + (d.motivo.size() > 48 ? -3.0f : 9.0f),
                  x + ancho - xm - 18.0f, 24.0f, Color::kTinta2, 2);
    }
}

void pintaLista(Lienzo& l, const Pincel& p, const PaginaCuaderno& pg, float x, float y, float abajo) {
    float cursor = y;
    if (!pg.cabeceraIzq.empty() || !pg.cabeceraDer.empty()) {
        p.pixel(l, pg.cabeceraIzq, x + 36.0f, cursor + 6.0f, 2, Color::kTinta3);
        if (!pg.cabeceraDer.empty())
            p.pixel(l, pg.cabeceraDer, x + pg.anchoLista - 12.0f - p.anchoPixel(pg.cabeceraDer, 2),
                    cursor + 6.0f, 2, Color::kTinta3);
        cursor += 29.0f + 10.0f;
    }
    const int total = static_cast<int>(pg.filas.size());
    const int caben = std::max(1, static_cast<int>((abajo - cursor) / (kFila + kHuecoFila)));
    // Ventana que sigue al foco: la fila elegida nunca queda fuera.
    int primera = 0;
    if (pg.foco >= caben) primera = pg.foco - caben + 1;
    for (int i = primera; i < std::min(total, primera + caben); ++i) {
        pintaFila(l, p, pg.filas[static_cast<std::size_t>(i)], x, cursor, pg.anchoLista, i == pg.foco);
        l.zona(Rect{x, cursor, pg.anchoLista, kFila}, kZonaFilaPagina, i);
        cursor += kFila + kHuecoFila;
    }
    const Vector4 flecha = Color::kTinta3;
    if (primera > 0) p.pixel(l, "▲", x + pg.anchoLista - 22.0f, y + 6.0f, 1, flecha);
    if (primera + caben < total) p.pixel(l, "▼", x + pg.anchoLista - 22.0f, abajo - 12.0f, 1, flecha);
}

void pintaFicha(Lienzo& l, const Pincel& p, const FichaPagina& f, float x, float y, float abajo) {
    const Vector4 divisor = conAlfa(Color::kTinta3, 0.6f);
    // Columna 1: identidad y estado (datos apilados).
    p.tituloBloque(l, "IDENTIDAD", x, y, 280.0f);
    float c1 = y + 28.0f;
    for (const DatoPagina& d : f.identidad) {
        p.texto(l, Letra::Lora16, d.clave, x, c1, 21.0f, Color::kTinta3);
        c1 += p.parrafo(l, Letra::Lora20, d.valor, x, c1 + 21.0f, 280.0f, 26.0f, Color::kTinta, 2) + 25.0f;
    }
    p.tituloBloque(l, "ESTADO", x, c1 + 4.0f, 280.0f);
    pintaDatos(l, p, f.estado, x, c1 + 32.0f, 280.0f);
    p.punteadoV(l, x + 300.0f, y + 8.0f, abajo, divisor);
    // Columna 2: atributos con su origen y el rasgo.
    const float x2 = x + 322.0f;
    p.tituloBloque(l, "ATRIBUTOS", x2, y, 340.0f);
    float c2 = y + 30.0f;
    for (const StatPagina& s : f.stats) {
        p.icono(l, s.icono, x2, c2 + 10.0f, 1);
        p.texto(l, Letra::Lora20, s.nombre, x2 + 24.0f, c2 + 5.0f, 26.0f, Color::kTinta);
        p.pixel(l, s.abreviatura, x2 + 152.0f, c2 + 11.0f, 2, Color::kTinta3);
        p.pixel(l, std::to_string(s.total), x2 + 196.0f, c2 + 4.0f, 3, Color::kTinta);
        p.texto(l, Letra::Lora18, s.detalle, x2 + 228.0f, c2 + 5.0f, 26.0f, Color::kTinta3);
        c2 += 42.0f;
    }
    if (!f.rasgoNombre.empty()) {
        p.tituloBloque(l, "RASGO", x2, c2 + 4.0f, 340.0f);
        c2 += 32.0f;
        if (!f.rasgoIcono.empty()) p.icono(l, f.rasgoIcono, x2, c2 + 5.0f, 1);
        p.texto(l, Letra::Lora20b, f.rasgoNombre, x2 + 24.0f, c2, 26.0f, Color::kTinta);
        p.parrafo(l, Letra::Lora18, f.rasgoDescripcion, x2, c2 + 32.0f, 340.0f, 25.0f, Color::kTinta2,
                  std::max(1, static_cast<int>((abajo - c2 - 32.0f) / 25.0f)));
    }
    p.punteadoV(l, x2 + 362.0f, y + 8.0f, abajo, divisor);
    // Columna 3: la historia personal que se eligio al crear el personaje.
    const float x3 = x2 + 384.0f;
    const float ancho3 = 1016.0f - x3;
    p.tituloBloque(l, "HISTORIA PERSONAL", x3, y, ancho3);
    float c3 = y + 28.0f;
    float clave = 0.0f;
    for (const DatoPagina& d : f.historia)
        clave = std::max(clave, static_cast<float>(p.letras().ancho(Letra::Lora16, d.clave)));
    for (const DatoPagina& d : f.historia) {
        p.texto(l, Letra::Lora16, d.clave, x3, c3, 23.0f, Color::kTinta3);
        const float h = p.parrafo(l, Letra::Lora16, d.valor, x3 + clave + 16.0f, c3, ancho3 - clave - 16.0f,
                                  23.0f, Color::kTinta, 2);
        c3 += std::max(23.0f, h) + 5.0f;
        if (c3 > abajo - 20.0f) break;
    }
    // AV14-23: la reputación, debajo de la historia, mientras quepa. La
    // facción se recorta (la columna es estrecha) para que se lea cómo te ven.
    if (f.reputacion.empty() || c3 + 26.0f + 23.0f > abajo) return;
    c3 += 4.0f;
    p.tituloBloque(l, "REPUTACIÓN", x3, c3, ancho3);
    c3 += 26.0f;
    float claveRep = 0.0f;
    for (const DatoPagina& d : f.reputacion)
        claveRep = std::max(claveRep, static_cast<float>(p.letras().ancho(Letra::Lora16, d.clave)));
    claveRep = std::min(claveRep, 104.0f);
    for (const DatoPagina& d : f.reputacion) {
        if (c3 + 23.0f > abajo) break;
        p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, d.clave, claveRep), x3, c3, 23.0f, Color::kTinta3);
        p.texto(l, Letra::Lora16, p.letras().recorta(Letra::Lora16, d.valor, ancho3 - claveRep - 12.0f),
                x3 + claveRep + 12.0f, c3, 23.0f, Color::kTinta);
        c3 += 24.0f;
    }
}

void pintaVacia(Lienzo& l, const Pincel& p, const PaginaCuaderno& pg, float x, float y, float ancho,
                float abajo) {
    const float centro = x + ancho * 0.5f;
    float cursor = y + (abajo - y) * 0.30f;
    if (!pg.vacioIcono.empty()) {
        p.icono(l, pg.vacioIcono, centro - 16.0f, cursor, 2);
        cursor += 44.0f;
    }
    const float wt = static_cast<float>(p.letras().ancho(Letra::Lora20b, pg.vacioTitulo));
    p.texto(l, Letra::Lora20b, pg.vacioTitulo, centro - wt * 0.5f, cursor, 26.0f, Color::kTinta);
    cursor += 32.0f;
    for (const std::string& linea : p.letras().envuelve(Letra::Lora18, pg.vacioTexto, 380.0f)) {
        const float w = static_cast<float>(p.letras().ancho(Letra::Lora18, linea));
        p.texto(l, Letra::Lora18, linea, centro - w * 0.5f, cursor, 25.0f, Color::kTinta2);
        cursor += 25.0f;
    }
}

}  // namespace

void pintaPagina(Lienzo& l, const Pincel& p, const PaginaCuaderno& pg, float ancho, float alto,
                 int numero, int total) {
    const float anchoUtil = ancho - 2.0f * kMargenX;
    const float abajo = alto - kAbajo;
    p.tituloConFiletes(l, pg.titulo, kMargenX, kArriba, anchoUtil, 180);
    // El folio («2/6») solo en el cuaderno; total 0 = sin folio (la tienda: sus dos
    // pestañas no son páginas, y «1/2» no decía nada, TIENDA-04).
    const std::string folio = std::to_string(numero) + "/" + std::to_string(total);
    if (total > 0) p.pixel(l, folio, ancho - 34.0f - p.anchoPixel(folio, 2), alto - 35.0f, 2, Color::kTinta3);
    switch (pg.tipo) {
        case PaginaCuaderno::Tipo::Ficha:
            pintaFicha(l, p, pg.ficha, kMargenX, kInicioRejilla + 4.0f, abajo);
            return;
        case PaginaCuaderno::Tipo::Vacia:
            pintaVacia(l, p, pg, kMargenX, kInicioRejilla, anchoUtil, abajo);
            return;
        case PaginaCuaderno::Tipo::Lista:
            break;
    }
    pintaLista(l, p, pg, kMargenX, kInicioRejilla, abajo);
    const float xSep = kMargenX + pg.anchoLista + 18.0f;
    p.punteadoV(l, xSep, kInicioRejilla + 8.0f, abajo - 8.0f, conAlfa(Color::kTinta3, 0.6f));
    const float xDetalle = xSep + 2.0f + 18.0f + 32.0f;
    pintaDetalle(l, p, pg.detalle, xDetalle, kInicioRejilla, ancho - kMargenX - 8.0f - xDetalle, abajo);
}

}  // namespace Pergamino
