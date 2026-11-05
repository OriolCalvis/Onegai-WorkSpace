// AV14-22 (D0016) — ver include/HUD/Pergamino/PaginaMisiones.h.
#include "HUD/Pergamino/PaginaMisiones.h"

#include <algorithm>
#include <string>

#include "Game/DiarioMisiones.h"

namespace Pergamino {

namespace {

std::string cuenta(int hechos, int total) {
    return total > 0 ? std::to_string(hechos) + "/" + std::to_string(total) : std::string();
}

const char* iconoMision(const Game::DiarioMision& m) {
    return m.fallida ? "mision_fallida" : m.completa() ? "mision_hecha" : "mision_activa";
}

std::string textoObjetivo(const Game::DiarioObjetivo& o) {
    return o.opcional ? "Opcional: " + o.texto : o.texto;
}

FilaPagina filaObjetivo(const Game::DiarioObjetivo& o) {
    FilaPagina f;
    f.nombre = textoObjetivo(o);
    f.hecha = o.hecho;
    if (o.requerido > 1) f.valor = cuenta(o.actual, o.requerido);
    return f;
}

void detalleMision(DetallePagina& d, const Game::DiarioMision& m, bool seguida) {
    d.icono = iconoMision(m);
    d.titulo = m.nombre;
    if (m.fallida) {
        d.insignia = "FALLIDA";
        d.tono = TonoInsignia::Aviso;
    } else if (m.completa()) {
        d.insignia = "HECHA";
        d.tono = TonoInsignia::Exito;
    } else {
        // MAPA-02: SEGUIDA es la que eliges (ENTER); la empezada que no sigues va EN CURSO.
        d.insignia = seguida ? "SEGUIDA" : m.empezada() ? "EN CURSO" : "PENDIENTE";
        d.tono = seguida ? TonoInsignia::Info : TonoInsignia::Neutra;
    }
    if (!m.empezada()) {
        d.datos.push_back({"Aún no", "Llegará más adelante en la historia."});
        if (!m.descripcion.empty()) d.parrafos.push_back(m.descripcion);
        return;
    }
    if (m.fallida && !m.motivoFallo.empty()) d.datos.push_back({"Por qué", m.motivoFallo});
    const Game::DiarioObjetivo* ahora = m.completa() || m.fallida ? nullptr : m.siguiente();
    if (ahora != nullptr) {
        std::string texto = textoObjetivo(*ahora);
        if (ahora->requerido > 1) texto += " (" + cuenta(ahora->actual, ahora->requerido) + ")";
        d.datos.push_back({"Ahora", texto});
    } else if (m.completa()) {
        d.datos.push_back({"Hecho", "Has cumplido todo lo que se te pedía."});
    }
    if (!m.decisiones.empty()) {
        const std::size_t n = m.decisiones.size();
        d.datos.push_back({"Decidiste", std::to_string(n) + (n == 1 ? " cosa" : " cosas") + " (debajo, en la lista)"});
    }
    if (m.apartados.empty()) {
        d.bloque = "OBJETIVOS";
        for (const Game::DiarioObjetivo& o : m.objetivos) d.lista.push_back(filaObjetivo(o));
    } else {
        d.bloque = "APARTADOS";
        for (const Game::DiarioApartado& a : m.apartados) {
            FilaPagina f;
            f.nombre = a.titulo;
            f.valor = cuenta(a.hechos, a.total);
            f.hecha = a.completo();
            d.lista.push_back(f);
        }
    }
    // El resumen solo si cabe: con muchos apartados llegaria al folio de la
    // esquina (y el resumen ya se leyo al empezar la mision).
    if (!m.descripcion.empty() && d.lista.size() + d.datos.size() <= 5) d.nota = m.descripcion;
}

void detalleApartado(DetallePagina& d, const Game::DiarioMision& m, const Game::DiarioApartado& a) {
    d.icono = a.completo() ? "objetivo_hecho" : "objetivo";
    d.titulo = a.titulo;
    d.insignia = a.completo() ? "HECHO" : (a.total == 0 ? "OPCIONAL" : "EN CURSO");
    d.tono = a.completo() ? TonoInsignia::Exito : TonoInsignia::Neutra;
    d.datos.push_back({"Misión", m.nombre});
    if (!a.resumen.empty()) d.parrafos.push_back(a.resumen);
    d.bloque = "OBJETIVOS";
    for (std::size_t i : a.objetivos) d.lista.push_back(filaObjetivo(m.objetivos[i]));
    std::string aqui;
    for (const Game::DiarioDecision& dd : m.decisiones) {
        if (dd.apartado != a.id) continue;
        aqui += (aqui.empty() ? "" : ", ") + dd.titulo;
    }
    if (!aqui.empty()) d.nota = "Lo que decidiste aquí: " + aqui + ".";
}

void detalleDecision(DetallePagina& d, const Game::DiarioMision& m, const Game::DiarioDecision& dd) {
    d.icono = "moral";
    d.titulo = dd.titulo;
    d.insignia = "DECIDIDO";
    d.tono = TonoInsignia::Info;
    d.datos.push_back({"Elegiste", dd.eleccion});
    d.bloque = dd.despues.empty() ? "LO QUE PASÓ" : "LO QUE PASÓ, Y DESPUÉS";
    FilaPagina ahora;
    ahora.icono = "marcado";
    ahora.nombre = dd.consecuencia;
    ahora.lineas = 4;
    d.lista.push_back(ahora);
    for (const std::string& luego : dd.despues) {
        FilaPagina f;
        f.icono = "lore";
        f.nombre = luego;
        f.lineas = 3;
        d.lista.push_back(f);
    }
    d.nota = dd.apartado.empty() ? m.nombre : m.nombre + " · " + dd.apartado;
}

}  // namespace

std::vector<FilaDiario> filasDelDiario(const Game::DiarioDatos& diario) {
    std::vector<FilaDiario> filas;
    for (std::size_t i = 0; i < diario.misiones.size(); ++i) {
        const Game::DiarioMision& m = diario.misiones[i];
        filas.push_back({FilaDiario::Tipo::Mision, i, 0});
        if (!m.empezada()) continue;
        for (std::size_t a = 0; a < m.apartados.size(); ++a) filas.push_back({FilaDiario::Tipo::Apartado, i, a});
        for (std::size_t k = 0; k < m.decisiones.size(); ++k) filas.push_back({FilaDiario::Tipo::Decision, i, k});
    }
    return filas;
}

PaginaCuaderno paginaMisiones(const Game::DiarioDatos& diario, std::size_t elegida) {
    PaginaCuaderno pg;
    pg.titulo = "MISIONES";
    pg.cabeceraIzq = "DIARIO";
    pg.anchoLista = 420.0f;
    pg.elegir = "Misión";
    const std::vector<FilaDiario> filas = filasDelDiario(diario);
    for (const FilaDiario& f : filas) {
        const Game::DiarioMision& m = diario.misiones[f.mision];
        FilaPagina fila;
        fila.escalaIcono = 1;
        switch (f.tipo) {
            case FilaDiario::Tipo::Mision:
                fila.icono = iconoMision(m);
                fila.nombre = m.nombre;
                fila.valor = cuenta(m.hechos, m.total);
                fila.apagada = !m.empezada();
                fila.hecha = m.completa();
                break;
            case FilaDiario::Tipo::Apartado: {
                const Game::DiarioApartado& a = m.apartados[f.indice];
                fila.sangria = 1;
                fila.icono = a.completo() ? "objetivo_hecho" : "objetivo";
                fila.nombre = a.titulo;
                fila.valor = cuenta(a.hechos, a.total);
                fila.hecha = a.completo();
                break;
            }
            case FilaDiario::Tipo::Decision:
                fila.sangria = 1;
                fila.icono = "moral";
                fila.nombre = m.decisiones[f.indice].titulo;
                break;
        }
        pg.filas.push_back(fila);
    }
    if (filas.empty()) return pg;
    pg.foco = static_cast<int>(std::min(elegida, filas.size() - 1));
    const FilaDiario& f = filas[static_cast<std::size_t>(pg.foco)];
    const Game::DiarioMision& m = diario.misiones[f.mision];
    switch (f.tipo) {
        case FilaDiario::Tipo::Mision:
            detalleMision(pg.detalle, m, &m == Game::misionSeguida(diario));
            // MAPA-02: ENTER sigue la mision (el retal OBJETIVO, el minimapa y la flecha van con ella).
            if (Game::sePuedeSeguir(m) && &m != Game::misionSeguida(diario)) pg.verbo = "Seguir";
            break;
        case FilaDiario::Tipo::Apartado: detalleApartado(pg.detalle, m, m.apartados[f.indice]); break;
        case FilaDiario::Tipo::Decision: detalleDecision(pg.detalle, m, m.decisiones[f.indice]); break;
    }
    return pg;
}

}  // namespace Pergamino
