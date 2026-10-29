// MISIONES-01 + H3-02 (AV14-22, D0016) — ver include/Game/DiarioMisiones.h.
#include "Game/DiarioMisiones.h"

#include <algorithm>

#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"

namespace Game {

namespace {

bool encendida(const RPG::NarrativeState& estado, const std::string& flag) {
    return !flag.empty() && estado.hasFlag(flag);
}

bool cumple(const RPG::NarrativeState& estado, const RPG::QuestCondition& c) {
    if (c.flags.empty()) return false;
    for (const std::string& f : c.flags)
        if (!estado.hasFlag(f)) return false;
    for (const std::string& f : c.notFlags)
        if (estado.hasFlag(f)) return false;
    return true;
}

// Un objetivo venga de donde venga (la aventura o la propia ficha).
struct Fuente {
    std::string id, texto, flag, variable, visible;
    std::vector<std::string> donde;
    int requerido = 1;
    bool opcional = false;
};

Fuente deAventura(const RPG::ObjectiveDefinition& o) {
    Fuente f;
    f.id = o.id;
    f.texto = o.text;
    f.flag = o.doneFlag;
    f.donde = o.donde;  // MAPA-01
    return f;
}

Fuente deFicha(const RPG::QuestObjective& o) {
    Fuente f;
    f.id = o.id;
    f.texto = o.description;
    f.flag = o.doneFlag;
    f.variable = o.variable;
    f.visible = o.visibleFlag;
    f.donde = o.donde;
    f.requerido = std::max(1, o.requiredCount);
    f.opcional = o.optional;
    return f;
}

DiarioObjetivo resolverObjetivo(const Fuente& f, const RPG::NarrativeState& estado) {
    DiarioObjetivo o;
    o.id = f.id;
    o.texto = f.texto;
    o.donde = f.donde;
    o.opcional = f.opcional;
    o.requerido = f.requerido;
    if (!f.variable.empty()) o.actual = std::clamp(estado.variable(f.variable), 0, o.requerido);
    o.hecho = encendida(estado, f.flag) || (!f.variable.empty() && o.actual >= o.requerido);
    if (o.hecho) o.actual = o.requerido;
    return o;
}

const RPG::QuestDefinition* fichaDe(const FichasDeMisiones& fichas, const std::string& aventura) {
    for (const RPG::QuestDefinition& q : fichas)
        if (!q.adventureId.empty() && q.adventureId == aventura) return &q;
    return nullptr;
}

// Los objetivos de la ficha por apartados: cada id se busca primero en los
// objetivos propios de la ficha y despues en la aventura. Lo que la aventura
// tenga y ningun apartado nombre va al final, en «Otros»: no se pierde nada.
void porApartados(DiarioMision& m, const RPG::QuestDefinition& ficha,
                  const RPG::AdventureScript& aventura, const RPG::NarrativeState& estado) {
    std::vector<std::string> usados;
    auto busca = [&](const std::string& id, Fuente& f) {
        for (const RPG::QuestObjective& o : ficha.objectives) {
            if (o.id != id) continue;
            f = deFicha(o);
            return true;
        }
        for (const RPG::ObjectiveDefinition& o : aventura.objectives) {
            if (o.id != id) continue;
            f = deAventura(o);
            return true;
        }
        return false;
    };
    std::vector<RPG::QuestSection> secciones = ficha.sections;
    RPG::QuestSection otros{"Otros", "", {}};
    for (const RPG::ObjectiveDefinition& o : aventura.objectives) {
        bool nombrado = false;
        for (const RPG::QuestSection& s : secciones)
            nombrado = nombrado || std::find(s.objectiveIds.begin(), s.objectiveIds.end(), o.id) != s.objectiveIds.end();
        if (!nombrado) otros.objectiveIds.push_back(o.id);
    }
    if (!otros.objectiveIds.empty()) secciones.push_back(otros);

    // Dos pasadas: obligatorios y luego opcionales (ver DiarioMision::objetivos).
    std::vector<DiarioApartado> apartados(secciones.size());
    for (int pasada = 0; pasada < 2; ++pasada) {
        for (std::size_t i = 0; i < secciones.size(); ++i) {
            DiarioApartado& a = apartados[i];
            a.titulo = secciones[i].title;
            a.resumen = secciones[i].summary;
            for (const std::string& id : secciones[i].objectiveIds) {
                Fuente f;
                if (!busca(id, f) || f.opcional != (pasada == 1)) continue;
                if (!f.visible.empty() && !estado.hasFlag(f.visible)) continue;
                if (std::find(usados.begin(), usados.end(), id) != usados.end()) continue;
                usados.push_back(id);
                a.objetivos.push_back(m.objetivos.size());
                m.objetivos.push_back(resolverObjetivo(f, estado));
                const DiarioObjetivo& o = m.objetivos.back();
                if (o.opcional) continue;
                ++a.total;
                ++m.total;
                if (o.hecho) ++a.hechos, ++m.hechos;
            }
        }
    }
    // Un apartado se queda en su sitio aunque solo tenga opcionales; si no
    // le queda nada visible, no se ensena.
    for (DiarioApartado& a : apartados)
        if (!a.objetivos.empty()) m.apartados.push_back(std::move(a));
}

void decisiones(DiarioMision& m, const RPG::QuestDefinition& ficha, const RPG::NarrativeState& estado) {
    for (const RPG::QuestDecision& d : ficha.decisions) {
        for (const RPG::QuestChoice& c : d.choices) {
            if (!cumple(estado, c.when)) continue;
            DiarioDecision dd;
            dd.id = d.id;
            dd.titulo = d.title;
            dd.apartado = d.section;
            dd.eleccion = c.choice;
            dd.consecuencia = c.consequence;
            for (const RPG::QuestAftermath& a : c.aftermath)
                if (cumple(estado, a.when)) dd.despues.push_back(a.text);
            m.decisiones.push_back(std::move(dd));
            break;  // una salida por decision: la primera tomada
        }
    }
}

DiarioMision resolver(const RPG::AdventureScript& aventura, bool activa, const RPG::NarrativeState& estado,
                      const FichasDeMisiones& fichas) {
    DiarioMision m;
    m.id = aventura.id;
    m.nombre = aventura.name.empty() ? aventura.id : aventura.name;
    m.descripcion = aventura.description;
    m.activa = activa;
    const RPG::QuestDefinition* ficha = fichaDe(fichas, aventura.id);
    if (ficha == nullptr) {
        for (const RPG::ObjectiveDefinition& o : aventura.objectives) {
            m.objetivos.push_back(resolverObjetivo(deAventura(o), estado));
            ++m.total;
            if (m.objetivos.back().hecho) ++m.hechos;
        }
        return m;
    }
    if (!ficha->name.empty()) m.nombre = ficha->name;
    if (!ficha->summary.empty()) m.descripcion = ficha->summary;
    porApartados(m, *ficha, aventura, estado);
    decisiones(m, *ficha, estado);
    m.cerrada = encendida(estado, ficha->completionFlag);
    for (const RPG::QuestFailure& f : ficha->failures) {
        if (!encendida(estado, f.flag)) continue;
        m.fallida = true;
        m.motivoFallo = f.reason;
        break;
    }
    return m;
}

}  // namespace

const DiarioObjetivo* DiarioMision::siguiente() const {
    for (const DiarioObjetivo& o : objetivos)
        if (!o.hecho && !o.opcional) return &o;
    for (const DiarioObjetivo& o : objetivos)
        if (!o.hecho) return &o;
    return nullptr;
}

bool DiarioMision::empezada() const { return activa || completa() || fallida || hechos > 0 || !decisiones.empty(); }

bool sePuedeSeguir(const DiarioMision& m) { return m.empezada() && !m.completa() && !m.fallida; }

const DiarioMision* misionSeguida(const DiarioDatos& diario) {
    if (!diario.seguida.empty())
        for (const DiarioMision& m : diario.misiones)
            if (m.id == diario.seguida && sePuedeSeguir(m)) return &m;
    for (const DiarioMision& m : diario.misiones)
        if (m.activa && !m.completa() && !m.fallida) return &m;
    return nullptr;
}

int DiarioDatos::activas() const {
    return static_cast<int>(std::count_if(misiones.begin(), misiones.end(), [](const DiarioMision& m) {
        return m.activa && !m.completa() && !m.fallida;
    }));
}

int DiarioDatos::completas() const {
    return static_cast<int>(std::count_if(misiones.begin(), misiones.end(),
                                          [](const DiarioMision& m) { return m.completa(); }));
}

DiarioDatos construirDiario(const std::vector<const RPG::AdventureScript*>& campana,
                            const RPG::AdventureScript* activa, const RPG::NarrativeState& estado) {
    return construirDiario(campana, activa, estado, fichasPublicadas());
}

DiarioDatos construirDiario(const std::vector<const RPG::AdventureScript*>& campana,
                            const RPG::AdventureScript* activa, const RPG::NarrativeState& estado,
                            const FichasDeMisiones& fichas) {
    DiarioDatos d;
    if (activa != nullptr) d.misiones.push_back(resolver(*activa, true, estado, fichas));
    for (const RPG::AdventureScript* aventura : campana) {
        if (aventura == nullptr || aventura == activa) continue;
        if (activa != nullptr && aventura->id == activa->id) continue;
        d.misiones.push_back(resolver(*aventura, false, estado, fichas));
    }
    return d;
}

std::vector<std::string> lineasDiario(const DiarioDatos& diario) {
    std::vector<std::string> out;
    if (diario.misiones.empty()) {
        out.push_back("(sin misiones)");
        return out;
    }
    auto objetivo = [&out](const DiarioObjetivo& o) {
        std::string linea = std::string("  [") + (o.hecho ? "x" : " ") + "] " + (o.opcional ? "(opcional) " : "") + o.texto;
        if (o.requerido > 1) linea += " (" + std::to_string(o.actual) + "/" + std::to_string(o.requerido) + ")";
        out.push_back(linea);
    };
    for (const DiarioMision& m : diario.misiones) {
        const char* marca = m.fallida ? "! " : m.completa() ? "* " : (m.activa ? "> " : "  ");
        out.push_back(marca + m.nombre + " " + std::to_string(m.hechos) + "/" + std::to_string(m.total));
        if (m.fallida && !m.motivoFallo.empty()) out.push_back("  (fallida) " + m.motivoFallo);
        if (m.apartados.empty()) {
            for (const DiarioObjetivo& o : m.objetivos) objetivo(o);
        }
        for (const DiarioApartado& a : m.apartados) {
            out.push_back("  # " + a.titulo + " " + std::to_string(a.hechos) + "/" + std::to_string(a.total));
            for (std::size_t i : a.objetivos) objetivo(m.objetivos[i]);
        }
        for (const DiarioDecision& dd : m.decisiones) {
            out.push_back("  ~ " + dd.eleccion + " -> " + dd.consecuencia);
            for (const std::string& luego : dd.despues) out.push_back("    + " + luego);
        }
    }
    return out;
}

int capituloDeLaCampana(const RPG::NarrativeState& estado) {
    // GORRAN-DIA2: prologo 0, dia 1, dia de Gorran 2, dia 3 (boundington_segundo_dia) 3, Ocaso 4.
    if (estado.hasFlag("bnd_ocaso_empezado")) return 4;
    if (estado.hasFlag("bnd_dia2_empezado")) return 3;
    if (estado.hasFlag("bnd_diag_empezado")) return 2;
    if (estado.hasFlag("bnd_dia1_empezado")) return 1;
    return 0;
}

}  // namespace Game
