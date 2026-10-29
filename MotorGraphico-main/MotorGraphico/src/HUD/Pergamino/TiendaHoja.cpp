// TIENDA-02 — ver include/HUD/Pergamino/TiendaHoja.h.
//
// Medidas (vista de 1280x720): hoja de 940x600 centrada; cabecera a 22 px
// del borde (TIENDA en pixel x3); pestanas a 64, de 36 px (la activa 4 mas
// alta) sobre el filete de 100; la pagina de la pestana (PaginaCuaderno)
// de 104 a 600 - 96; la linea de estado a 84 del borde de abajo y el pie de
// teclas a 50. La lista ocupa el 38 % del ancho util, como en M06.
#include "HUD/Pergamino/TiendaHoja.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Game/ComercioReputacion.h"
#include "Game/GameSession.h"
#include "Game/TiendaEstado.h"
#include "HUD/Pergamino/PaginasJugador.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {

constexpr float kAncho = 940.0f;
constexpr float kAlto = 600.0f;
constexpr float kMargen = 36.0f;
constexpr float kCabecera = 22.0f;
constexpr float kPestanas = 64.0f;
constexpr float kFilete = 100.0f;
constexpr float kPagina = 104.0f;
constexpr float kBajoPagina = 96.0f;  // estado y pie
constexpr float kEstado = 84.0f;      // desde abajo
constexpr float kPie = 50.0f;         // desde abajo

std::string deOro(int n) { return std::to_string(n) + " de oro"; }

void detalleCompra(const Game::FilaTienda& f, int oro, DetallePagina& d) {
    d.datos.push_back({"Precio", deOro(f.precio)});
    d.datos.push_back({"Tienes", std::to_string(f.tienes)});
    d.boton = "COMPRAR 1 · " + std::to_string(f.precio) + " ORO";
    d.botonActivo = oro >= f.precio;
    if (d.botonActivo) d.datos.push_back({"Te quedarán", deOro(oro - f.precio)});
    else d.motivo = "Te faltan " + deOro(f.precio - oro) + ".";
}

void detalleVenta(const Game::FilaTienda& f, int oro, DetallePagina& d) {
    d.datos.push_back({"Te paga", deOro(f.precio)});
    d.datos.push_back({"Tienes", std::to_string(f.tienes)});
    d.datos.push_back({"Tendrás", deOro(oro + f.precio)});
    d.boton = "VENDER 1 · RECIBES " + std::to_string(f.precio);
    d.botonActivo = true;
}

struct Pestana {
    const char* etiqueta;
    const char* icono;
};
const Pestana kPestanasTienda[2] = {{"COMPRAR", "comprar"}, {"VENDER", "vender"}};

void pintaPestanas(Lienzo& l, const Pincel& p, int activa, float x0, float y0) {
    struct Caja {
        Rect r;
        int i;
    };
    std::vector<Caja> cajas;
    float x = x0 + kMargen;
    for (int i = 0; i < 2; ++i) {
        float w = 16.0f + 16.0f + 8.0f + p.anchoPixel(kPestanasTienda[i].etiqueta, 2) + 16.0f;
        w = std::round(w / 2.0f) * 2.0f;
        const bool esta = i == activa;
        const float y = y0 + (esta ? kPestanas - 4.0f : kPestanas);
        cajas.push_back({Rect{x, y, w, y0 + kFilete - y + (esta ? 2.0f : 0.0f)}, i});
        x += w + 8.0f;
    }
    // El filete sobre el que se apoyan; la activa lo tapa (queda abierta a su pagina).
    l.color(Rect{x0 + 24.0f, y0 + kFilete, kAncho - 48.0f, 2.0f}, Color::kLinea);
    for (const Caja& c : cajas) {
        const bool esta = c.i == activa;
        const PedidoMaterial m = pestana(static_cast<int>(c.r.w), static_cast<int>(c.r.h), esta, 70 + c.i);
        if (esta) l.material(m, Vector2{c.r.x, c.r.y});
        else l.materialConSombra(m, Vector2{c.r.x, c.r.y}, rgb(26, 16, 8, 0.30f));
        const float xc = c.r.x + 16.0f;
        const float centro = c.r.y + (esta ? 20.0f : 18.0f);
        p.icono(l, kPestanasTienda[c.i].icono, xc, centro - 8.0f, 1);
        p.pixel(l, kPestanasTienda[c.i].etiqueta, xc + 24.0f, centro - 9.0f, 2, esta ? Color::kTinta : Color::kTinta2);
        l.zona(c.r, kZonaPestanaTienda, c.i);
    }
}

}  // namespace

DatosTienda datosTienda(const Game::TiendaEstado& tienda, const GameSession& s) {
    DatosTienda d;
    d.tendero = s.shopKeeper();
    d.oro = s.gold();
    const bool comprar = tienda.pestana() == Game::PestanaTienda::Comprar;
    d.pestana = comprar ? 0 : 1;
    d.estado = tienda.estado();
    d.estadoError = tienda.estadoError();
    d.reputacion = Game::lineaDeLaTienda(Game::facciones(), s.characterSheet(),
                                         Game::tenderoPorNombre(Game::facciones(), s.objectCatalog(), d.tendero));
    PaginaCuaderno& pg = d.pagina;
    pg.titulo = comprar ? "COMPRAR" : "VENDER";
    pg.cabeceraIzq = "OBJETO";
    pg.cabeceraDer = comprar ? "PRECIO" : "TE PAGA";
    pg.anchoLista = 330.0f;  // el 38 % del ancho util de la hoja (868 px)
    const std::vector<Game::FilaTienda> filas = Game::TiendaEstado::filas(s, tienda.pestana());
    if (filas.empty()) {
        pg.tipo = PaginaCuaderno::Tipo::Vacia;
        pg.vacioIcono = comprar ? "tienda" : "vender";
        pg.vacioTitulo = comprar ? "No le queda nada que venderte" : "No llevas nada que le interese";
        pg.vacioTexto = comprar ? "Vuelve otro día: lo que vende cambia con la historia."
                                : "Las llaves y los objetos de misión no se venden. Lo demás que compres o "
                                  "recojas por el camino, sí.";
        return d;
    }
    pg.elegir = "Elegir";
    pg.verbo = comprar ? "Comprar" : "Vender";
    for (const Game::FilaTienda& f : filas) {
        FilaPagina fila;
        fila.icono = iconoCategoria(f.categoria);
        fila.nombre = f.nombre;
        fila.valor = std::to_string(f.precio);
        fila.apagada = comprar && d.oro < f.precio;  // se puede mirar, no comprar
        pg.filas.push_back(fila);
    }
    pg.foco = static_cast<int>(tienda.foco(filas.size()));
    const Game::FilaTienda& f = filas[static_cast<std::size_t>(pg.foco)];
    DetallePagina& det = pg.detalle;
    det.icono = iconoCategoria(f.categoria);
    det.titulo = f.nombre;
    det.insignia = insigniaCategoria(f.categoria);
    if (f.curaPv > 0) {
        det.efectoIcono = "pv";
        det.efecto = "Recupera " + std::to_string(f.curaPv) + " PV.";
    } else if (f.categoria != Game::ItemCategory::Consumable && !f.descripcion.empty()) {
        det.parrafos.push_back(f.descripcion);  // el maná no se nombra: ya no sale en la interfaz
    }
    if (comprar) detalleCompra(f, d.oro, det);
    else detalleVenta(f, d.oro, det);
    return d;
}

Rect rectTienda(int anchoVista, int altoVista) {
    return Rect{std::round((static_cast<float>(anchoVista) - kAncho) * 0.5f),
                std::round((static_cast<float>(altoVista) - kAlto) * 0.5f), kAncho, kAlto};
}

void pintaTienda(Lienzo& l, const Pincel& p, const DatosTienda& d, int anchoVista, int altoVista) {
    l.color(Rect{0.0f, 0.0f, static_cast<float>(anchoVista), static_cast<float>(altoVista)}, Color::kVelo);
    const Rect r = rectTienda(anchoVista, altoVista);
    l.materialConSombra(hoja(static_cast<int>(kAncho), static_cast<int>(kAlto), 33, 2, 6, 2), Vector2{r.x, r.y},
                        Color::kSombra);
    // Cabecera: el tipo de ventana en pixel y el tendero en Lora (MODELO_VISUAL.md I3); a la derecha, tu oro.
    const float yCab = r.y + kCabecera;
    const float wt = p.pixel(l, "TIENDA", r.x + kMargen, yCab, 3, Color::kTinta);
    const std::string oro = std::to_string(d.oro);
    float xd = r.derecha() - kMargen - p.anchoPixel(oro, 2);
    p.pixel(l, oro, xd, yCab + 6.0f, 2, Color::kTinta);
    p.icono(l, "oro", xd - 22.0f, yCab + 7.0f, 1);
    xd -= 22.0f;
    const float wTu = static_cast<float>(p.letras().ancho(Letra::Lora18, "Tu oro"));
    p.texto(l, Letra::Lora18, "Tu oro", xd - 12.0f - wTu, yCab + 1.0f, 26.0f, Color::kTinta3);
    const float xTendero = r.x + kMargen + wt + 18.0f;
    p.texto(l, Letra::Lora24b, p.letras().recorta(Letra::Lora24b, d.tendero, xd - 40.0f - wTu - xTendero),
            xTendero, yCab - 2.0f, 30.0f, Color::kTinta);
    pintaPestanas(l, p, d.pestana, r.x, r.y);
    // AV14-23: la reputacion con la faccion del tendero, a la derecha de las
    // pestanas (de donde sale el precio que se ve).
    if (!d.reputacion.empty()) {
        const float ancho = r.derecha() - kMargen - (r.x + 360.0f);
        const std::string texto = p.letras().recorta(Letra::Lora16i, d.reputacion, ancho);
        const float w = static_cast<float>(p.letras().ancho(Letra::Lora16i, texto));
        p.texto(l, Letra::Lora16i, texto, r.derecha() - kMargen - w, r.y + kPestanas + 6.0f, 22.0f, Color::kTinta2);
    }
    // La pagina de la pestana, en coordenadas de la hoja y luego en su sitio.
    const std::size_t trazos = l.cuantos(), zonas = l.cuantasZonas();
    pintaPagina(l, p, d.pagina, kAncho, kAlto - kPagina - kBajoPagina, d.pestana + 1, 0);  // sin folio
    l.desplaza(trazos, zonas, r.x, r.y + kPagina);
    // La linea de estado, junto a la accion (E17/E18): lo que acaba de pasar.
    // Como en M06: lo que ha pasado en color («Comprado: Poción») y las cuentas en tinta.
    const float yEstado = r.abajo() - kEstado;
    if (!d.estado.empty()) {
        if (d.estadoError) p.pixel(l, "!", r.x + kMargen + 6.0f, yEstado + 5.0f, 2, Color::kError);
        else p.pixel(l, "✓", r.x + kMargen + 2.0f, yEstado + 5.0f, 2, Color::kExito);
        const std::size_t corte = d.estadoError ? std::string::npos : d.estado.find(" · ");
        const std::string que = d.estado.substr(0, corte);
        float x = r.x + kMargen + 26.0f;
        x += p.texto(l, Letra::Lora18b, que, x, yEstado, 26.0f, d.estadoError ? Color::kError : Color::kExito);
        if (corte != std::string::npos)
            p.texto(l, Letra::Lora18,
                    p.letras().recorta(Letra::Lora18, d.estado.substr(corte), r.derecha() - kMargen - x), x,
                    yEstado, 26.0f, Color::kTinta);
    }
    // El pie: los verbos de la pestana y cerrar a la derecha.
    const float yPie = r.abajo() - kPie;
    float x = r.x + kMargen;
    x += p.ayuda(l, {"←", "→"}, "Comprar / Vender", x, yPie, Color::kTinta2) + 24.0f;
    if (!d.pagina.elegir.empty()) x += p.ayuda(l, {"W", "S"}, d.pagina.elegir, x, yPie, Color::kTinta2) + 24.0f;
    if (!d.pagina.verbo.empty()) p.ayuda(l, {"ENTER"}, d.pagina.verbo, x, yPie, Color::kTinta2);
    const float wCerrar = p.anchoAyuda({"ESC"}, "Cerrar tienda");
    p.ayuda(l, {"ESC"}, "Cerrar tienda", r.derecha() - kMargen - wCerrar, yPie, Color::kTinta2);
}

}  // namespace Pergamino
