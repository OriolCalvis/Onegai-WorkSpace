// PERGAMINO-01 — ver include/HUD/Pergamino/DialogoHoja.h.
#include "HUD/Pergamino/DialogoHoja.h"

#include <algorithm>
#include <cmath>

#include "HUD/Pergamino/DadosPergamino.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

namespace {

// TUTORIAL-01 — Las frases de una linea: cortan tras . ! ? o «…» (con las comillas o el
// parentesis que las cierren) seguidos de un espacio, que se come el corte.
std::vector<std::string> frases(const std::string& linea) {
    std::vector<std::string> out;
    std::size_t ini = 0;
    for (std::size_t i = 0; i < linea.size(); ++i) {
        std::size_t fin = i;
        if (linea[i] == '.' || linea[i] == '!' || linea[i] == '?') fin = i + 1;
        else if (linea.compare(i, 3, "\xE2\x80\xA6") == 0) fin = i + 3;  // «…»
        else continue;
        for (;;) {
            if (linea.compare(fin, 2, "\xC2\xBB") == 0) fin += 2;  // «»»
            else if (fin < linea.size() && (linea[fin] == '"' || linea[fin] == '\'' || linea[fin] == ')')) ++fin;
            else break;
        }
        if (fin < linea.size() && linea[fin] == ' ') {
            out.push_back(linea.substr(ini, fin - ini));
            ini = fin + 1;
            i = fin;
        }
    }
    if (ini < linea.size()) out.push_back(linea.substr(ini));
    return out;
}

}  // namespace

std::pair<std::string, std::string> DialogoHoja::vozDe(const std::string& linea, const std::string& hablante) {
    const std::string abre = "\xC2\xAB", cierra = "\xC2\xBB";  // « »
    const std::size_t dos = linea.find(": " + abre);
    if (dos == std::string::npos || dos == 0 || dos > 90) return {hablante, linea};
    const std::string prefijo = linea.substr(0, dos);
    if (prefijo.find_first_of(".!?()") != std::string::npos || prefijo.find(abre) != std::string::npos)
        return {hablante, linea};
    std::string quien = prefijo, acotacion;
    const std::size_t coma = prefijo.find(", ");
    if (coma != std::string::npos) {
        quien = prefijo.substr(0, coma);
        acotacion = prefijo.substr(coma + 2);
    }
    // «En el límite de Pico Dragón, [...] el letrero: «...»» es un sitio, no alguien.
    for (const char* sitio : {"En ", "Al ", "A ", "Desde ", "Junto ", "Con ", "Sobre ", "Tras ", "Bajo ", "Entre ", "Por "})
        if (quien.rfind(sitio, 0) == 0) return {hablante, linea};
    std::string dicho = linea.substr(dos + 2);
    const bool entreComillas = dicho.size() >= abre.size() + cierra.size() && dicho.compare(0, abre.size(), abre) == 0 &&
                               dicho.compare(dicho.size() - cierra.size(), cierra.size(), cierra) == 0 &&
                               dicho.find(abre, abre.size()) == std::string::npos;
    if (entreComillas) dicho = dicho.substr(abre.size(), dicho.size() - abre.size() - cierra.size());
    return {quien, acotacion.empty() ? dicho : "(" + acotacion + ") " + dicho};
}

const std::string& DialogoHoja::quienHabla() const {
    static const std::string nadie;
    return m_pagina < static_cast<int>(m_quienes.size()) ? m_quienes[static_cast<std::size_t>(m_pagina)] : nadie;
}

void DialogoHoja::mira(const std::string& hablante, const std::vector<std::string>& lineas,
                       const FuenteAtlas& letras) {
    if (hablante == m_hablante && lineas == m_lineas) return;
    m_hablante = hablante;
    m_lineas = lineas;
    m_pagina = 0;
    m_paginas.clear();
    m_quienes.clear();
    m_tirada = RPG::TiradaVisible{};
    std::string quien;  // DIALOGO-02: el de la linea que se esta paginando
    const auto renglonesDe = [&](const std::string& texto) {
        return letras.envuelve(Letra::Lora22, texto, kAnchoTexto);
    };
    const auto enPaginas = [&](const std::vector<std::string>& renglones) {
        for (std::size_t i = 0; i < renglones.size(); i += kRenglones) {
            std::vector<std::string> pag(renglones.begin() + static_cast<long>(i),
                                         renglones.begin() + static_cast<long>(std::min(renglones.size(), i + kRenglones)));
            m_paginas.push_back(std::move(pag));
            m_quienes.push_back(quien);
        }
    };
    // TUTORIAL-01 — Una linea que no cabe en dos renglones se parte por frases, no por
    // renglones: cada pagina lleva frases enteras (el prologo cortaba «...al
    // siguiente de un cadaver. Se acerca» / «sobre una barca...»). Una frase
    // que sola no cabe sigue, como antes, en la pagina siguiente.
    for (const std::string& original : lineas) {
        // DIALOGO-02 — Cada linea con su voz: «Sho-Noco: «...»» va bajo SHO-NOCO.
        auto [deQuien, linea] = vozDe(original, hablante);
        quien = deQuien;
        const std::vector<std::string> partes = frases(linea);
        if (partes.size() < 2) {
            enPaginas(renglonesDe(linea));
            continue;
        }
        std::string trozo;
        for (const std::string& frase : partes) {
            const std::string junto = trozo.empty() ? frase : trozo + " " + frase;
            if (renglonesDe(junto).size() <= static_cast<std::size_t>(kRenglones)) {
                trozo = junto;
                continue;
            }
            if (!trozo.empty()) enPaginas(renglonesDe(trozo));
            trozo = frase;
        }
        if (!trozo.empty()) enPaginas(renglonesDe(trozo));
    }
}

void DialogoHoja::cierra() {
    m_hablante.clear();
    m_lineas.clear();
    m_paginas.clear();
    m_quienes.clear();
    m_pagina = 0;
    m_tirada = RPG::TiradaVisible{};
    m_opciones.clear();
    m_opcion = 0;
}

void DialogoHoja::miraOpciones(const std::vector<std::string>& opciones) {
    if (opciones == m_opciones) return;
    m_opciones = opciones;
    m_opcion = 0;
}

void DialogoHoja::mueveOpcion(int paso) {
    const int n = static_cast<int>(m_opciones.size());
    if (n == 0) return;
    m_opcion = ((m_opcion + paso) % n + n) % n;
}

void DialogoHoja::miraTirada(const RPG::TiradaVisible& t) {
    // Distinto y no mayor: una partida nueva o cargada vuelve a contar.
    if (t.numero == m_tiradaVista) return;
    m_tiradaVista = t.numero;
    if (t.numero > 0 && !m_paginas.empty()) m_tirada = t;
}

bool DialogoHoja::avanza() {
    if (ultima()) return false;
    ++m_pagina;
    return true;
}

void DialogoHoja::pintaOpciones(Lienzo& l, const Pincel& p, float x, float abajo) const {
    // La hoja de las opciones, apoyada sobre la del dialogo: una fila por
    // opcion, con su tecla (1-4) y la elegida subrayada (como en los menus).
    constexpr float kFila = 40.0f;
    const float tecla = p.anchoTecla("4");
    float anchoTexto = 0.0f;
    for (const std::string& o : m_opciones) anchoTexto = std::max(anchoTexto, p.anchoTexto(Letra::Lora20, o));
    const float w = std::round(std::clamp(anchoTexto + tecla + 96.0f, 320.0f, kAncho - 48.0f));
    const float h = 30.0f + kFila * static_cast<float>(m_opciones.size());
    const float y = abajo - h;
    l.materialConSombra(hoja(static_cast<int>(w), static_cast<int>(h), 23, 2, 0, 2), Vector2{x, y}, Color::kSombra);
    for (std::size_t i = 0; i < m_opciones.size(); ++i) {
        const Rect fila{x + 14.0f, y + 15.0f + kFila * static_cast<float>(i), w - 28.0f, kFila - 4.0f};
        if (static_cast<int>(i) == m_opcion) p.filaFoco(l, fila);
        l.zona(fila, kZonaOpcionDialogo, static_cast<int>(i));
        p.tecla(l, std::to_string(i + 1), fila.x + 32.0f, std::round(fila.y + (fila.h - Medida::kTecla) * 0.5f));
        p.texto(l, Letra::Lora20, m_opciones[i], fila.x + 32.0f + tecla + 14.0f, fila.y, fila.h,
                static_cast<int>(i) == m_opcion ? Color::kTinta : Color::kTinta2);
    }
}

void DialogoHoja::pinta(Lienzo& l, const Pincel& p, int anchoVista, int altoVista,
                        const std::string& verboFinal) const {
    if (m_paginas.empty()) return;
    const float x = std::round((static_cast<float>(anchoVista) - kAncho) * 0.5f);
    const float y = static_cast<float>(altoVista) - 28.0f - kAlto;
    l.materialConSombra(hoja(static_cast<int>(kAncho), static_cast<int>(kAlto), 21, 2, 0, 2), Vector2{x, y},
                        Color::kSombra);
    // Rotulo de cuero con el hablante (voz pixel, claro sobre cuero 9,3:1).
    const std::string quien = quienHabla().empty() ? "NARRACIÓN" : quienHabla();  // DIALOGO-02: la de la pagina
    const float wq = p.anchoPixel(quien, 2);
    const float wr = std::max(150.0f, std::ceil((wq + 68.0f) / 2.0f) * 2.0f);
    const Vector2 rotulo{x + 32.0f, y - 18.0f};
    l.materialConSombra(cuero(static_cast<int>(wr), 36, 5, false, false, false), rotulo, Color::kSombraCuero);
    p.pixel(l, quien, std::round(rotulo.x + (wr - wq) * 0.5f), rotulo.y + 9.0f, 2, Color::kClaroCuero);
    // El texto de la pagina: Lora 22 con interlineado de 1,55.
    const auto& renglones = m_paginas[static_cast<std::size_t>(m_pagina)];
    for (std::size_t i = 0; i < renglones.size(); ++i)
        p.texto(l, Letra::Lora22, renglones[i], x + 40.0f, y + 34.0f + 34.0f * static_cast<float>(i), 34.0f,
                Color::kTinta);
    // Pie: pagina y tecla. La ultima dice «Cerrar».
    const float yPie = y + 114.0f;
    if (paginas() > 1) {
        const std::string n = std::to_string(m_pagina + 1) + "/" + std::to_string(paginas());
        p.pixel(l, n, x + 40.0f, yPie + 6.0f, 2, Color::kTinta3);
    }
    // OPCIONES-01: en la ultima pagina, con opciones, se elige en vez de cerrar.
    const bool eligiendo = conOpciones() && ultima();
    const std::string verbo = eligiendo ? "Decidir" : ultima() ? verboFinal : "Continuar";
    const float w = p.anchoAyuda({"ENTER"}, verbo);
    p.ayuda(l, {"ENTER"}, verbo, x + kAncho - 40.0f - w, yPie, Color::kTinta2);
    if (eligiendo) {
        const float we = p.anchoAyuda({"W", "S"}, "Elegir");
        p.ayuda(l, {"W", "S"}, "Elegir", x + kAncho - 64.0f - w - we, yPie, Color::kTinta2);
        pintaOpciones(l, p, x + 24.0f, y - 26.0f);
    }
    // H2-01: la tirada, en la ultima pagina (la que dice «Tirada de...»),
    // apoyada encima de la hoja por la derecha.
    if (conTirada() && ultima()) pintaTiradaDialogo(l, p, m_tirada, x + kAncho - 24.0f, y - 8.0f);
}

}  // namespace Pergamino
