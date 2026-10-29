// UI-BG3-01 — Las hojas sin rejilla del cuaderno de creacion: Aspecto (las
// filas del editor con sus flechas), Historia (C02c: las siete preguntas en
// una matriz de 7 x 5 tarjetas, con la emergente de la senalada) y Nombre
// (C02d: el campo, las sugerencias, quien eres, tu historia y «Empezar la
// aventura»).
#include <algorithm>
#include <cmath>

#include "PaginasCreacionInterno.h"

namespace Pergamino::creacion {

namespace {

using Fila = Game::EditorApariencia::Fila;

void aviso(const Contexto& k, const std::string& texto, Hoja ir, const std::string& boton) {
    centrado(k, Letra::Lora18i, texto, kIzqX, 300.0f, 560.0f, 28.0f, 3, Color::kTinta3);
    const float w = static_cast<float>(k.p.letras().ancho(Letra::Lora18, boton)) + 40.0f;
    const Rect r{std::round(kIzqX + (560.0f - w) * 0.5f), 372.0f, w, 40.0f};
    k.l.color(r, Color::kPapelLuz);
    k.p.contorno(k.l, r, 2.0f, Color::kLinea);
    k.p.texto(k.l, Letra::Lora18, boton, r.x + 20.0f, r.y + 6.0f, 28.0f, Color::kTinta);
    k.l.zona(r, tipo(ZonaC::Editar), static_cast<int>(ir));
}

// Un enlace «editar» en carmin subrayado; devuelve su ancho.
float enlace(const Contexto& k, const std::string& texto, float x, float y, Hoja h) {
    const float w = k.p.texto(k.l, Letra::Lora16, texto, x, y, 22.0f, Color::kCarmin);
    k.l.color(Rect{x, y + 19.0f, w, 1.0f}, Color::kCarmin);
    k.l.zona(Rect{x - 2.0f, y, w + 4.0f, 22.0f}, tipo(ZonaC::Editar), static_cast<int>(h));
    return w;
}

}  // namespace

// --- Aspecto ----------------------------------------------------------------

void pintaAspecto(const Contexto& k) {
    const Game::EditorApariencia* ed = k.c.editor();
    if (ed == nullptr || k.c.eleccion().raceId.empty()) {
        aviso(k, "El aspecto sale de tu pueblo: antes elige tu raza.", Hoja::Raza, "Ir a Raza");
        pintaCentro(k, k.c.vista(), 360.0f, 500.0f);
        return;
    }
    k.p.pixel(k.l, "TU ASPECTO", kIzqX, 122.0f, 2, Color::kTinta3);
    k.p.texto(k.l, Letra::Lora16, "Pasa el ratón por una fila y cambia con las flechas", 260.0f, 118.0f, 24.0f,
              Color::kTinta3);
    // EDITOR-PERSONALIZACION-01: con las filas nuevas (piel, marcas, alas, cola, cuernos) pueden ser veinte:
    // las filas se aprietan para caber en la pagina (de 32 px a 22 como poco).
    const float ancho = 560.0f;
    const float n = static_cast<float>(std::max<std::size_t>(1, ed->filas().size()));
    const float alto = std::clamp(std::floor((588.0f - 152.0f) / n), 22.0f, 32.0f);
    const float hTexto = alto - 4.0f, dy = std::round((alto - 32.0f) * 0.5f);
    float y = 152.0f;
    for (std::size_t i = 0; i < ed->filas().size(); ++i, y += alto) {
        const Fila fila = ed->filas()[i];
        const bool foco = static_cast<int>(i) == k.c.foco();
        const Rect r{kIzqX, y, ancho, alto - 2.0f};
        if (foco) k.p.filaFoco(k.l, r);
        k.l.zona(r, tipo(ZonaC::FilaAspecto), static_cast<int>(i));
        k.p.icono(k.l, Game::EditorApariencia::icono(fila), r.x + 22.0f, y + 7.0f + dy, 1);
        k.p.texto(k.l, foco ? Letra::Lora18b : Letra::Lora18, frase(Game::EditorApariencia::etiqueta(fila)), r.x + 46.0f,
                  y + 1.0f, hTexto, Color::kTinta);
        const std::string valor = ed->valor(fila);
        const bool accion = fila == Fila::AlAzar || fila == Fila::ComoSuRaza;
        const float xi = r.x + 300.0f, xd = r.derecha() - 26.0f;
        if (accion) {
            k.p.texto(k.l, Letra::Lora18i, valor, xi + 24.0f, y + 1.0f, hTexto, Color::kTinta2);
            k.l.zona(Rect{xi, y, xd + 26.0f - xi, alto}, tipo(ZonaC::Mas), static_cast<int>(i));
            continue;
        }
        const float wv = static_cast<float>(k.p.letras().ancho(Letra::Lora18, valor));
        k.p.texto(k.l, Letra::Lora18, valor, std::round(xi + 26.0f + (xd - xi - 26.0f - wv) * 0.5f), y + 1.0f, hTexto,
                  Color::kTinta);
        const Vector4 flecha = foco ? Color::kCarmin : Color::kTinta3;
        k.p.pixel(k.l, "◀", xi + 4.0f, y + 6.0f + dy, 2, flecha);
        k.p.pixel(k.l, "▶", xd + 4.0f, y + 6.0f + dy, 2, flecha);
        k.l.zona(Rect{xi - 4.0f, y, 30.0f, alto}, tipo(ZonaC::Menos), static_cast<int>(i));
        k.l.zona(Rect{xd - 4.0f, y, 30.0f, alto}, tipo(ZonaC::Mas), static_cast<int>(i));
    }
    pintaCentro(k, k.c.vista(), 360.0f, 500.0f);
    const Fila marcada = ed->filaMarcada();
    float yf = fichaTitulo(k, frase(Game::EditorApariencia::etiqueta(marcada)), 116.0f);
    yf += k.p.parrafo(k.l, Letra::Lora18, Game::EditorApariencia::ayuda(marcada), kFichaX, yf, kFichaW, 24.0f,
                      Color::kTinta2, 5) + 16.0f;
    fichaBloque(k, "TU ROPA", "", "Es la de diario. Lo que te equipes en la partida se pone encima.", yf, 3);
}

// --- Historia (C02c) ----------------------------------------------------------

void pintaHistoria(const Contexto& k) {
    const RPG::BackgroundDefinition* b = k.c.datos().reglas->backgrounds().find(k.c.eleccion().backgroundId);
    if (b == nullptr || k.c.opciones(0) == nullptr) {
        aviso(k, "Tu historia sale de tu origen: antes elige en qué mes naciste.", Hoja::Origen, "Ir a Origen");
        return;
    }
    k.p.icono(k.l, "lugar", kIzqX + 12.0f, 120.0f, 2);
    k.p.texto(k.l, Letra::Lora24b, b->name, kIzqX + 50.0f, 116.0f, 34.0f, Color::kTinta);
    int hechas = 0;
    for (int f = 0; f < Cuaderno::kPreguntas; ++f) hechas += k.c.elegidaHistoria(f).empty() ? 0 : 1;
    const std::string cuenta = std::to_string(hechas) + " DE 7";
    const float wn = static_cast<float>(k.p.letras().ancho(Letra::Lora16, "una tarjeta por fila"));
    k.p.texto(k.l, Letra::Lora16, "una tarjeta por fila", 1214.0f - wn, 122.0f, 24.0f, Color::kTinta3);
    k.p.pixel(k.l, cuenta, 1214.0f - wn - 14.0f - k.p.anchoPixel(cuenta, 2), 124.0f, 2, Color::kTinta2);

    const float x0 = 200.0f, y0 = 160.0f, w = 196.0f, h = 54.0f, dx = 204.0f, dy = 62.0f;
    Rect emergente{};
    std::string titulo, texto;
    for (int f = 0; f < Cuaderno::kPreguntas; ++f) {
        const float y = y0 + static_cast<float>(f) * dy;
        const bool contestada = !k.c.elegidaHistoria(f).empty();
        const float wl = k.p.pixel(k.l, Cuaderno::nombrePregunta(f), kIzqX, y + 16.0f, 2,
                                   contestada ? Color::kTinta : Color::kTinta2);
        if (contestada) k.p.pixel(k.l, "✓", kIzqX + wl + 6.0f, y + 18.0f, 2, Color::kExito);
        const auto* ops = k.c.opciones(f);
        if (ops == nullptr) continue;
        for (int c = 0; c < Cuaderno::kColumnas && c < static_cast<int>(ops->size()); ++c) {
            const auto& o = (*ops)[static_cast<std::size_t>(c)];
            const Rect r{x0 + static_cast<float>(c) * dx, y, w, h};
            const int indice = f * Cuaderno::kColumnas + c;
            const bool foco = indice == k.c.foco();
            const bool elegida = o.id == k.c.elegidaHistoria(f);
            k.l.color(r, foco ? rgb(222, 196, 150, 0.85f) : elegida ? Color::kPapelLuz : rgb(242, 228, 193, 0.5f));
            k.p.contorno(k.l, r, foco || elegida ? 2.0f : 1.0f,
                         foco ? Color::kCarmin : elegida ? Color::kTinta2 : rgb(107, 80, 56, 0.45f));
            // Sin nombre en el canon (algunas listas solo traen la frase):
            // la frase hace de titulo.
            const std::string nombre = !o.name.empty() ? o.name : !o.description.empty() ? o.description : o.id;
            k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, nombre, w - 16.0f), r.x + 8.0f,
                      r.y + 3.0f, 24.0f, Color::kTinta);
            if (!o.name.empty())
                k.p.texto(k.l, Letra::Lora16, k.p.letras().recorta(Letra::Lora16, o.description, w - 16.0f),
                          r.x + 8.0f, r.y + 27.0f, 22.0f, Color::kTinta2);
            if (elegida) k.l.material(pieza(Material::Sello), Vector2{r.derecha() - 16.0f, r.y - 10.0f});
            if (foco) {
                k.l.material(pieza(Material::Cinta), Vector2{r.x + 2.0f, r.y - 6.0f});
                emergente = r;
                titulo = nombre;
                texto = o.name.empty() ? std::string() : o.description;
            }
            k.l.zona(r, tipo(ZonaC::Tarjeta), indice);
        }
    }
    // La emergente de la senalada: el nombre y la frase enteros.
    if (emergente.w > 0.0f) {
        const float we = 300.0f;
        const auto lineas = k.p.letras().envuelve(Letra::Lora18, texto, we - 28.0f);
        // Sin frase aparte (el titulo ya es la frase), el titulo entero.
        const auto lineasTitulo = k.p.letras().envuelve(Letra::Lora18b, titulo, we - 28.0f);
        const bool tituloLargo = texto.empty();
        const float he = (tituloLargo ? 14.0f + 26.0f * static_cast<float>(std::min<std::size_t>(lineasTitulo.size(), 3))
                                      : 30.0f + 24.0f * static_cast<float>(std::min<std::size_t>(lineas.size(), 3))) +
                         34.0f;
        float xe = std::min(emergente.x + 60.0f, 1222.0f - we);
        float ye = emergente.abajo() + 6.0f;
        if (ye + he > 620.0f) ye = emergente.y - he - 6.0f;
        const int alto = static_cast<int>(std::ceil(he / 2.0f) * 2.0f);
        k.l.materialConSombra(hoja(static_cast<int>(we), alto, 71, 1, 0, 1), Vector2{xe, ye}, Color::kSombra);
        if (tituloLargo) {
            k.p.parrafo(k.l, Letra::Lora18b, titulo, xe + 14.0f, ye + 10.0f, we - 28.0f, 26.0f, Color::kTinta, 3);
        } else {
            k.p.texto(k.l, Letra::Lora18b, k.p.letras().recorta(Letra::Lora18b, titulo, we - 28.0f), xe + 14.0f,
                      ye + 8.0f, 26.0f, Color::kTinta);
            k.p.parrafo(k.l, Letra::Lora18, texto, xe + 14.0f, ye + 36.0f, we - 28.0f, 24.0f, Color::kTinta2, 3);
        }
        k.p.texto(k.l, Letra::Lora16, "Clic o ENTER para elegirla", xe + 14.0f, ye + he - 28.0f, 22.0f,
                  Color::kTinta3);
    }
}

// --- Nombre (C02d) --------------------------------------------------------------

void pintaNombre(const Contexto& k) {
    const RPG::CreationChoice& e = k.c.eleccion();
    const RPG::CharacterCreation& reglas = *k.c.datos().reglas;
    // Izquierda: tu personaje con el kit (se vera asi al ponertelo) y los
    // atributos con los que empiezas (todo a 0).
    Cuaderno::Vista v = k.c.vista();
    pintaVista(k, v, 200.0f, 404.0f, 3);
    if (v.conKit)
        centrado(k, Letra::Lora18, "Con el kit del cofre", 62.0f, 432.0f, 276.0f, 26.0f, 1, Color::kTinta2);
    // AV16-02: todo empieza a 0; ni la raza ni el kit suman (se destacan los atributos principales de la clase).
    k.p.tituloBloque(k.l, "AL EMPEZAR", 80.0f, 474.0f, 256.0f);
    pintaAtributos(k, v, 80.0f, 500.0f);

    // Centro: el nombre.
    const float xm = 368.0f, wm = 422.0f;
    k.p.pixel(k.l, "CÓMO TE LLAMAS", xm, 122.0f, 2, Color::kTinta3);
    const Rect campo{xm, 152.0f, wm, 52.0f};
    k.l.color(campo, Color::kPapelLuz);
    k.p.contorno(k.l, campo, 2.0f, Color::kLinea);
    const float wn = k.p.texto(k.l, Letra::Lora24, e.displayName, campo.x + 14.0f, campo.y + 9.0f, 34.0f,
                               Color::kTinta);
    if (std::fmod(k.c.segundos(), 1.0) < 0.6)
        k.l.color(Rect{campo.x + 18.0f + wn, campo.y + 12.0f, 2.0f, 28.0f}, Color::kCarmin);
    float x = xm;
    const auto& sugerencias = Cuaderno::sugerencias();
    for (std::size_t i = 0; i < sugerencias.size(); ++i) {
        const float w = chip(k, sugerencias[i], x, 214.0f, sugerencias[i] == e.displayName);
        k.l.zona(Rect{x, 214.0f, w, 30.0f}, tipo(ZonaC::Sugerencia), static_cast<int>(i));
        x += w + 8.0f;
    }
    k.p.texto(k.l, Letra::Lora16, "Escribe el tuyo o elige uno con el ratón o las flechas.", xm, 252.0f, 22.0f,
              Color::kTinta3);
    k.p.tituloBloque(k.l, "QUIÉN ERES", xm, 290.0f, wm);
    auto nombreDe = [](const auto* def, const char* nada) { return def != nullptr ? def->name : std::string(nada); };
    struct Linea {
        const char* etiqueta;
        std::string valor;
        Hoja hoja;
    };
    const Linea lineas[] = {
        {"Raza", nombreDe(reglas.races().find(e.raceId), "Sin elegir"), Hoja::Raza},
        {"Clase", nombreDe(reglas.classes().find(e.classId), "Sin elegir"), Hoja::Clase},
        {"Origen", nombreDe(reglas.backgrounds().find(e.backgroundId), "Sin elegir"), Hoja::Origen},
        {"Deidad", nombreDe(reglas.deities().find(e.deityId), "Ninguna"), Hoja::Deidad}};
    float y = 316.0f;
    for (const Linea& l : lineas) {
        k.p.texto(k.l, Letra::Lora16, l.etiqueta, xm, y + 2.0f, 22.0f, Color::kTinta3);
        const std::string valor = k.p.letras().recorta(Letra::Lora18, l.valor, wm - 68.0f - 60.0f);
        const float wv = k.p.texto(k.l, Letra::Lora18, valor, xm + 68.0f, y, 26.0f, Color::kTinta);
        enlace(k, "editar", xm + 68.0f + wv + 8.0f, y + 3.0f, l.hoja);
        y += 28.0f;
    }
    k.p.icono(k.l, "objeto_mision", xm, y + 16.0f, 1);
    k.p.parrafo(k.l, Letra::Lora18, "Empiezas sin equipo. En El Vacío te espera un cofre con el kit de tu clase: va a "
                                    "la mochila y te lo pones desde el cuaderno.",
                xm + 26.0f, y + 12.0f, wm - 26.0f, 23.0f, Color::kTinta2, 4);

    // Derecha: tu historia y empezar.
    const float xh = 824.0f, wh = 394.0f;
    k.p.tituloBloque(k.l, "TU HISTORIA", xh, 122.0f, wh);
    y = 148.0f;
    for (int f = 0; f < Cuaderno::kPreguntas; ++f, y += 23.0f) {
        k.p.texto(k.l, Letra::Lora16, Cuaderno::nombrePregunta(f), xh, y, 22.0f, Color::kTinta3);
        std::string valor = "—";
        if (const auto* ops = k.c.opciones(f))
            for (const auto& o : *ops)
                if (o.id == k.c.elegidaHistoria(f))
                    valor = !o.name.empty() ? o.name : !o.description.empty() ? o.description : o.id;
        k.p.texto(k.l, Letra::Lora16, k.p.letras().recorta(Letra::Lora16, valor, wh - 80.0f), xh + 80.0f, y, 22.0f,
                  Color::kTinta);
    }
    enlace(k, "editar la historia", xh, y + 6.0f, Hoja::Historia);
    const std::vector<std::string> problemas = k.c.problemas();
    const Rect empezar{xh, 372.0f, wh, 52.0f};
    if (problemas.empty()) {
        k.l.color(Rect{empezar.x - 4.0f, empezar.y - 4.0f, empezar.w + 8.0f, empezar.h + 8.0f}, Color::kCarmin);
        k.l.color(Rect{empezar.x - 2.0f, empezar.y - 2.0f, empezar.w + 4.0f, empezar.h + 4.0f}, Color::kPapelLuz);
        boton(k, empezar, "EMPEZAR LA AVENTURA", true);
    } else {
        boton(k, empezar, "EMPEZAR LA AVENTURA", false);
        float yp = empezar.abajo() + 16.0f;
        for (const std::string& pr : problemas) {
            if (yp > 600.0f) break;
            k.p.pixel(k.l, "!", xh, yp + 4.0f, 2, Color::kError);
            yp += k.p.parrafo(k.l, Letra::Lora18, pr, xh + 18.0f, yp, wh - 18.0f, 24.0f, Color::kError, 2) + 4.0f;
        }
    }
    k.l.zona(empezar, tipo(ZonaC::Continuar));
}

}  // namespace Pergamino::creacion
