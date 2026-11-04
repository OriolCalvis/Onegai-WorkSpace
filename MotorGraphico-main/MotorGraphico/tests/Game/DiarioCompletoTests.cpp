// AV14-22 (D0016) — El diario de misiones completo: la ficha (H3-02) y la
// vista de las flags (MISIONES-01) juntas, probadas con Boundington de verdad.
//
//   1  las fichas publicadas casan con las cuatro aventuras: cada objetivo de
//      la aventura esta en un apartado (y solo en uno), cada id de un
//      apartado existe, y cada flag que nombran decisiones, consecuencias y
//      objetivos la puede encender la aventura (una flag mal escrita es una
//      decision que nunca saldria en el cuaderno);
//   2  una decision tomada sale con su consecuencia, y lo que paso despues
//      solo cuando ha pasado; las no tomadas no se anuncian;
//   3  «ahora» es el primer obligatorio sin hacer; los opcionales esperan a
//      su flag de visible; los contadores se leen de una variable;
//   4  una mision fallida no se esconde: sale con su motivo;
//   5  la hoja del cuaderno ensena la mision entera: la mision, sus
//      apartados con su cuenta y cada decision, y el detalle de cada uno.
//
// Los assets se leen del directorio de trabajo (copiar_assets).

#include "Game/DiarioMisiones.h"
#include "HUD/Pergamino/PaginaMisiones.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/EventosDelMundo.h"
#include "RPG/Narrative/NarrativeState.h"
#include "TestHarness.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* const kAventuras[] = {"boundington_prologo", "boundington_primer_dia", "boundington_dia_gorran", "boundington_segundo_dia",
                                  "boundington_ocaso"};

std::vector<RPG::AdventureScript> boundington() {
    std::vector<RPG::AdventureScript> out;
    for (const char* nombre : kAventuras) {
        auto r = RPG::AdventureScript::loadFromFile(std::string("assets/adventures/") + nombre + ".json");
        MOTOR_CHECK(r.isOk());
        if (r.isOk()) out.push_back(r.value());
    }
    return out;
}

Game::FichasDeMisiones fichas() {
    std::ifstream f("assets/catalogs/quests.json", std::ios::binary);
    std::ostringstream texto;
    texto << f.rdbuf();
    return Game::leeFichasDeMisiones(texto.str());
}

void apunta(const std::vector<RPG::NarrativeEffect>& efectos, std::set<std::string>& flags) {
    for (const RPG::NarrativeEffect& e : efectos) {
        if (e.type == RPG::EffectType::SetFlag) flags.insert(e.arg);
        const RPG::SkillCheckRequest& t = e.skillCheck;
        for (const std::string* f : {&t.flagBotch, &t.flagPartial, &t.flagSuccess, &t.flagCritical}) flags.insert(*f);
        const RPG::BattleRequest& b = e.battle;
        for (const std::string* f : {&b.flagVictory, &b.flagDefeat, &b.flagHuida}) flags.insert(*f);
        for (const auto& r : b.refuerzos) flags.insert(r.flags.begin(), r.flags.end());
        flags.insert(e.tabla.flags.begin(), e.tabla.flags.end());
    }
}

// Todas las flags que las cuatro aventuras pueden encender, y las que
// enciende la Application al saltar de capitulo (capituloDeLaCampana).
std::set<std::string> flagsEncendibles(const std::vector<RPG::AdventureScript>& aventuras) {
    std::set<std::string> flags = {"bnd_dia1_empezado", "bndg_empezado", "bnd_dia2_empezado", "bnd_ocaso_empezado"};
    for (const RPG::AdventureScript& a : aventuras) {
        for (const RPG::NarrativeBeat& b : a.beats) {
            apunta(b.effects, flags);
            for (const RPG::OpcionDeDialogo& o : b.opciones) apunta(o.efectos, flags);
        }
    }
    // C8-04b: los eventos del mundo (assets/eventos/*.json) tambien encienden flags.
    std::error_code ec;
    for (const auto& f : std::filesystem::directory_iterator("assets/eventos", ec)) {
        if (f.path().extension() != ".json") continue;
        const auto cat = RPG::eventos::cargaCatalogo(f.path().generic_string());
        MOTOR_CHECK(cat.isOk());
        if (!cat.isOk()) continue;
        for (const RPG::eventos::Evento& e : cat.value().eventos) {
            apunta(e.efectos, flags);
            apunta(e.alCaducar, flags);
            for (const RPG::eventos::Fase& fase : e.fases) apunta(fase.efectos, flags);
        }
    }
    flags.erase("");
    return flags;
}

const RPG::AdventureScript* aventuraDe(const std::vector<RPG::AdventureScript>& aventuras, const std::string& id) {
    for (const RPG::AdventureScript& a : aventuras)
        if (a.id == id) return &a;
    return nullptr;
}

std::vector<const RPG::AdventureScript*> campana(const std::vector<RPG::AdventureScript>& aventuras) {
    std::vector<const RPG::AdventureScript*> c;
    for (const RPG::AdventureScript& a : aventuras) c.push_back(&a);
    return c;
}

const Game::DiarioMision* mision(const Game::DiarioDatos& d, const std::string& id) {
    for (const Game::DiarioMision& m : d.misiones)
        if (m.id == id) return &m;
    return nullptr;
}

const Game::DiarioDecision* decision(const Game::DiarioMision& m, const std::string& id) {
    for (const Game::DiarioDecision& dd : m.decisiones)
        if (dd.id == id) return &dd;
    return nullptr;
}

}  // namespace

MOTOR_TEST_CASE(las_fichas_de_boundington_casan_con_sus_aventuras) {
    const std::vector<RPG::AdventureScript> aventuras = boundington();
    const Game::FichasDeMisiones todas = fichas();
    const std::set<std::string> encendibles = flagsEncendibles(aventuras);
    MOTOR_CHECK(aventuras.size() == 5);  // GORRAN-DIA2: cuatro dias y el prologo
    int conAventura = 0;
    for (const RPG::QuestDefinition& q : todas) {
        if (q.adventureId.empty()) continue;
        ++conAventura;
        const RPG::AdventureScript* a = aventuraDe(aventuras, q.adventureId);
        MOTOR_CHECK(a != nullptr);
        if (a == nullptr) continue;
        // Cada objetivo de la aventura, en un apartado y solo en uno.
        for (const RPG::ObjectiveDefinition& o : a->objectives) {
            int veces = 0;
            for (const RPG::QuestSection& s : q.sections)
                veces += static_cast<int>(std::count(s.objectiveIds.begin(), s.objectiveIds.end(), o.id));
            if (veces != 1) std::fprintf(stderr, "  %s: el objetivo %s sale %d veces\n", q.id.c_str(), o.id.c_str(), veces);
            MOTOR_CHECK(veces == 1);
        }
        // Cada id de un apartado existe (en la ficha o en la aventura).
        std::set<std::string> titulos;
        for (const RPG::QuestSection& s : q.sections) {
            MOTOR_CHECK(!s.title.empty());
            titulos.insert(s.title);
            for (const std::string& id : s.objectiveIds) {
                bool existe = std::any_of(q.objectives.begin(), q.objectives.end(),
                                          [&](const RPG::QuestObjective& o) { return o.id == id; }) ||
                              std::any_of(a->objectives.begin(), a->objectives.end(),
                                          [&](const RPG::ObjectiveDefinition& o) { return o.id == id; });
                if (!existe) std::fprintf(stderr, "  %s: el apartado %s nombra %s, que no existe\n", q.id.c_str(), s.title.c_str(), id.c_str());
                MOTOR_CHECK(existe);
            }
        }
        // Cada flag que la ficha nombra la puede encender la aventura.
        auto existe = [&](const std::string& f, const std::string& donde) {
            const bool ok = encendibles.count(f) > 0;
            if (!ok) std::fprintf(stderr, "  %s: %s nombra la flag %s, que nadie enciende\n", q.id.c_str(), donde.c_str(), f.c_str());
            MOTOR_CHECK(ok);
        };
        for (const RPG::QuestObjective& o : q.objectives) {
            if (!o.doneFlag.empty()) existe(o.doneFlag, o.id);
            if (!o.visibleFlag.empty()) existe(o.visibleFlag, o.id);
        }
        for (const RPG::QuestDecision& d : q.decisions) {
            MOTOR_CHECK(!d.choices.empty());
            MOTOR_CHECK(d.section.empty() || titulos.count(d.section) > 0);
            for (const RPG::QuestChoice& c : d.choices) {
                MOTOR_CHECK(!c.when.flags.empty() && !c.choice.empty() && !c.consequence.empty());
                for (const std::string& f : c.when.flags) existe(f, d.id);
                for (const std::string& f : c.when.notFlags) existe(f, d.id);
                for (const RPG::QuestAftermath& luego : c.aftermath) {
                    MOTOR_CHECK(!luego.when.flags.empty() && !luego.text.empty());
                    for (const std::string& f : luego.when.flags) existe(f, d.id);
                    for (const std::string& f : luego.when.notFlags) existe(f, d.id);
                }
            }
        }
    }
    MOTOR_CHECK(conAventura == 4);
}

MOTOR_TEST_CASE(una_decision_tomada_sale_con_su_consecuencia) {
    const std::vector<RPG::AdventureScript> aventuras = boundington();
    const Game::FichasDeMisiones todas = fichas();
    RPG::NarrativeState estado;
    estado.setFlag("bnd_dia1_empezado");
    const RPG::AdventureScript* dia1 = aventuraDe(aventuras, "adv_boundington_primer_dia");
    MOTOR_CHECK(dia1 != nullptr);

    // Nada decidido: nada que anunciar.
    Game::DiarioDatos d = Game::construirDiario(campana(aventuras), dia1, estado, todas);
    const Game::DiarioMision* m = mision(d, "adv_boundington_primer_dia");
    MOTOR_CHECK(m != nullptr && m->activa && m->decisiones.empty());
    MOTOR_CHECK(m->nombre == "El Primer Día en Boundington");
    MOTOR_CHECK(m->apartados.size() == 3);  // «Encargos de la ciudad» aun no tiene nada visible

    // Mandas a Luisarda con Rosita: la decision, con lo que paso.
    estado.setFlag("bnd1_recado_luisarda");
    estado.setFlag("bnd1_luisarda_con_rosita");
    d = Game::construirDiario(campana(aventuras), dia1, estado, todas);
    m = mision(d, "adv_boundington_primer_dia");
    MOTOR_CHECK(m->decisiones.size() == 1);
    const Game::DiarioDecision* luisarda = decision(*m, "bnd1_luisarda_pide");
    MOTOR_CHECK(luisarda != nullptr);
    MOTOR_CHECK(luisarda->eleccion.find("cenar con Rosita") != std::string::npos);
    MOTOR_CHECK(luisarda->consecuencia.find("Rosita te debe una") != std::string::npos);
    MOTOR_CHECK(luisarda->apartado == "Encargos de la ciudad");
    MOTOR_CHECK(luisarda->despues.empty());  // el Dia 2 aun no ha llegado
    // Y el encargo opcional ya se ve, y cumplido.
    MOTOR_CHECK(m->apartados.size() == 4);
    MOTOR_CHECK(m->apartados.back().titulo == "Encargos de la ciudad");
    MOTOR_CHECK(m->apartados.back().total == 0 && m->apartados.back().objetivos.size() == 1);
    MOTOR_CHECK(m->objetivos[m->apartados.back().objetivos[0]].hecho);

    // Al dia siguiente Luisarda te lo agradece: la consecuencia de despues.
    estado.setFlag("bnd2_luisarda_visto");
    d = Game::construirDiario(campana(aventuras), dia1, estado, todas);
    luisarda = decision(*mision(d, "adv_boundington_primer_dia"), "bnd1_luisarda_pide");
    MOTOR_CHECK(luisarda->despues.size() == 1);
    MOTOR_CHECK(luisarda->despues[0].find("Te debo una, cielo") != std::string::npos);

    // Una decision con varias salidas: la tomada, no otra; y en el orden de
    // la ficha (Tomas va antes que Luisarda).
    estado.setFlag("bnd0_tomas_ofendido");
    d = Game::construirDiario(campana(aventuras), dia1, estado, todas);
    m = mision(d, "adv_boundington_primer_dia");
    MOTOR_CHECK(m->decisiones.size() == 2);
    MOTOR_CHECK(m->decisiones[0].id == "bnd1_tomas");
    MOTOR_CHECK(m->decisiones[0].eleccion == "Te reíste de Tomás.");

    // Una consecuencia con condicion negativa: los veinte hombres solo si
    // Venides no se quedo sin aviso.
    estado.setFlag("bnd_venides_20");
    estado.setFlag("bnd3_arenga");
    d = Game::construirDiario(campana(aventuras), dia1, estado, todas);
    const Game::DiarioDecision* informe = decision(*mision(d, "adv_boundington_primer_dia"), "bnd1_informe");
    MOTOR_CHECK(informe != nullptr && informe->despues.size() == 1);
    estado.setFlag("bnd3_sin_venides");
    d = Game::construirDiario(campana(aventuras), dia1, estado, todas);
    informe = decision(*mision(d, "adv_boundington_primer_dia"), "bnd1_informe");
    MOTOR_CHECK(informe != nullptr && informe->despues.empty());
}

MOTOR_TEST_CASE(ahora_es_el_primer_obligatorio_y_los_contadores_salen_de_una_variable) {
    const auto a = RPG::AdventureScript::loadFromString(
        R"({"id":"adv_x","name":"X","objectives":[
            {"id":"uno","text":"Uno","doneFlag":"f_uno"},{"id":"dos","text":"Dos","doneFlag":"f_dos"}],"beats":[]})");
    MOTOR_CHECK(a.isOk());
    Game::FichasDeMisiones fs = Game::leeFichasDeMisiones(R"({"entries":[{"id":"x","name":"La X","tier":1,"rarity":"common",
        "adventureId":"adv_x",
        "objectives":[{"id":"buzon","description":"Mirar el buzon","optional":true,"doneFlag":"f_buzon","visibleFlag":"f_rumor"},
                      {"id":"tres","description":"Tres jarras","requiredCount":3,"variable":"jarras"}],
        "sections":[{"title":"A","objectives":["buzon","dos","tres"]},{"title":"B","objectives":["uno"]}]}]})");
    MOTOR_CHECK(fs.size() == 1);
    RPG::NarrativeState e;
    Game::DiarioDatos d = Game::construirDiario({&a.value()}, &a.value(), e, fs);
    const Game::DiarioMision& m = d.misiones[0];
    MOTOR_CHECK(m.nombre == "La X");
    MOTOR_CHECK(m.total == 3);  // dos, tres, uno; el buzon es opcional y aun no se ve
    MOTOR_CHECK(m.objetivos.size() == 3);
    MOTOR_CHECK(m.siguiente() != nullptr && m.siguiente()->id == "dos");  // el orden de los apartados
    MOTOR_CHECK(m.apartados[0].titulo == "A" && m.apartados[0].total == 2);

    e.setFlag("f_dos");
    e.setVariable("jarras", 2);
    e.setFlag("f_rumor");
    d = Game::construirDiario({&a.value()}, &a.value(), e, fs);
    const Game::DiarioMision& m2 = d.misiones[0];
    MOTOR_CHECK(m2.objetivos.size() == 4);  // el buzon ya se ve
    MOTOR_CHECK(m2.total == 3 && m2.hechos == 1);
    MOTOR_CHECK(m2.siguiente()->id == "tres");
    MOTOR_CHECK(m2.siguiente()->actual == 2 && m2.siguiente()->requerido == 3);
    // El opcional va al final de la lista (el retal del HUD coge el primero sin
    // hacer), pero dentro de su apartado.
    MOTOR_CHECK(m2.objetivos.back().id == "buzon" && m2.objetivos.back().opcional);
    MOTOR_CHECK(m2.apartados[0].objetivos.size() == 3);

    e.setVariable("jarras", 9);  // se corta en lo requerido
    e.setFlag("f_uno");
    d = Game::construirDiario({&a.value()}, &a.value(), e, fs);
    const Game::DiarioMision& m3 = d.misiones[0];
    MOTOR_CHECK(m3.completa() && m3.hechos == 3);
    MOTOR_CHECK(m3.siguiente() != nullptr && m3.siguiente()->id == "buzon");  // solo queda lo opcional
    const auto lineas = Game::lineasDiario(d);
    MOTOR_CHECK(lineas[0] == "* La X 3/3");
    MOTOR_CHECK(lineas[1] == "  # A 2/2");
    MOTOR_CHECK(std::find(lineas.begin(), lineas.end(), "  [x] Tres jarras (3/3)") != lineas.end());
    MOTOR_CHECK(std::find(lineas.begin(), lineas.end(), "  [ ] (opcional) Mirar el buzon") != lineas.end());
}

MOTOR_TEST_CASE(una_mision_fallida_no_se_esconde_y_lo_que_falta_de_la_aventura_no_se_pierde) {
    const auto a = RPG::AdventureScript::loadFromString(
        R"({"id":"adv_f","name":"F","objectives":[
            {"id":"uno","text":"Uno","doneFlag":"f_uno"},{"id":"suelto","text":"Suelto","doneFlag":"f_suelto"}],"beats":[]})");
    Game::FichasDeMisiones fs = Game::leeFichasDeMisiones(R"({"entries":[{"id":"f","name":"F","tier":1,"rarity":"common",
        "adventureId":"adv_f","sections":[{"title":"A","objectives":["uno"]}],
        "failures":[{"flag":"f_muerto","reason":"Llegaste tarde."}]}]})");
    RPG::NarrativeState e;
    Game::DiarioDatos d = Game::construirDiario({&a.value()}, &a.value(), e, fs);
    // «suelto» no esta en ningun apartado: sale en «Otros», no desaparece.
    MOTOR_CHECK(d.misiones[0].apartados.size() == 2);
    MOTOR_CHECK(d.misiones[0].apartados[1].titulo == "Otros");
    MOTOR_CHECK(d.misiones[0].total == 2);
    e.setFlag("f_muerto");
    d = Game::construirDiario({&a.value()}, &a.value(), e, fs);
    MOTOR_CHECK(d.misiones[0].fallida && !d.misiones[0].completa());
    MOTOR_CHECK(std::string(d.misiones[0].estado()) == "fallida");
    MOTOR_CHECK(d.misiones[0].motivoFallo == "Llegaste tarde.");
    MOTOR_CHECK(d.activas() == 0);
    MOTOR_CHECK(Game::lineasDiario(d)[1] == "  (fallida) Llegaste tarde.");
}

MOTOR_TEST_CASE(el_cuaderno_ensena_la_mision_entera_con_sus_decisiones) {
    const std::vector<RPG::AdventureScript> aventuras = boundington();
    const Game::FichasDeMisiones todas = fichas();
    RPG::NarrativeState estado;
    for (const char* f : {"bnd_dia1_empezado", "bnd_tut_tendero", "bnd_tut_joyero", "bnd0_tomas_amigo",
                          "bnd1_venides_sabe_edul", "bnd_prologo_grito", "kit_basico_recogido"})
        estado.setFlag(f);
    const RPG::AdventureScript* dia1 = aventuraDe(aventuras, "adv_boundington_primer_dia");
    const Game::DiarioDatos d = Game::construirDiario(campana(aventuras), dia1, estado, todas);

    // Las filas: el Dia 1 (activo) con sus 3 apartados y 2 decisiones; el
    // prologo empezado con sus 3 apartados y 1 decision; Gorran, el Dia 2 y el Ocaso,
    // sin empezar, una fila cada uno.
    const std::vector<Pergamino::FilaDiario> filas = Pergamino::filasDelDiario(d);
    MOTOR_CHECK(filas.size() == 1 + 3 + 2 + 1 + 3 + 1 + 1 + 1 + 1);  // + el dia de Gorran, sin empezar
    Pergamino::PaginaCuaderno pg = Pergamino::paginaMisiones(d, 0);
    MOTOR_CHECK(pg.filas.size() == filas.size());
    MOTOR_CHECK(pg.filas[0].nombre == "El Primer Día en Boundington");
    MOTOR_CHECK(pg.filas[0].valor == "2/8");
    MOTOR_CHECK(pg.filas[1].sangria == 1 && pg.filas[1].nombre == "Presentarte en la plaza");
    MOTOR_CHECK(pg.filas[1].valor == "2/4");
    MOTOR_CHECK(pg.filas[4].icono == "moral" && pg.filas[4].nombre == "Tomás, en el rincón de la taberna");
    MOTOR_CHECK(pg.filas.back().apagada && pg.filas.back().sangria == 0);

    // La mision: lo que toca ahora, primero (cinco segundos).
    MOTOR_CHECK(pg.detalle.insignia == "SEGUIDA");
    MOTOR_CHECK(!pg.detalle.datos.empty() && pg.detalle.datos[0].clave == "Ahora");
    MOTOR_CHECK(pg.detalle.datos[0].valor.find("Preséntate al sastre") != std::string::npos);
    MOTOR_CHECK(pg.detalle.bloque == "APARTADOS" && pg.detalle.lista.size() == 3);

    // Un apartado: sus objetivos.
    pg = Pergamino::paginaMisiones(d, 1);
    MOTOR_CHECK(pg.detalle.titulo == "Presentarte en la plaza");
    MOTOR_CHECK(pg.detalle.lista.size() == 4 && pg.detalle.lista[0].hecha && !pg.detalle.lista[2].hecha);

    // Una decision: lo que elegiste y lo que paso.
    pg = Pergamino::paginaMisiones(d, 5);
    MOTOR_CHECK(pg.detalle.titulo == "Lo que supo Venides de Edul");
    MOTOR_CHECK(pg.detalle.insignia == "DECIDIDO");
    MOTOR_CHECK(pg.detalle.datos[0].clave == "Elegiste");
    MOTOR_CHECK(pg.detalle.datos[0].valor.find("Edul está con los Perdidos") != std::string::npos);
    MOTOR_CHECK(pg.detalle.lista.size() == 1 && pg.detalle.lista[0].icono == "marcado");
    MOTOR_CHECK(pg.detalle.lista[0].nombre.find("vamos a por él") != std::string::npos);

    // Una mision sin empezar no cuenta sus apartados.
    pg = Pergamino::paginaMisiones(d, filas.size() - 1);
    MOTOR_CHECK(pg.detalle.insignia == "PENDIENTE" && pg.detalle.lista.empty());
    // Fuera de rango se queda en la ultima.
    MOTOR_CHECK(Pergamino::paginaMisiones(d, 999).foco == static_cast<int>(filas.size()) - 1);
}

int main() { return motor_test::runAllTests(); }
