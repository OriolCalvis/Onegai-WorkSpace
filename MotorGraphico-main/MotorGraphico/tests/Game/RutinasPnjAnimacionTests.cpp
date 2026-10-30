// U3-06.E05 -- Game::entradasDePnj + maquina pnj_basico contra el oraculo RutinasPnj::animacion.
// Sin GL. Lee assets/animaciones/pnj_basico.estados.json (WORKING_DIRECTORY = fuentes).
// Estado vivo de partida de cada combinacion: {inicial, idle_down} y se dan pasos con las MISMAS
// entradas hasta que no haya cambio de estado (D-T6: una transicion por paso; maximo 3 pasos);
// se compara vivo.clip con el oraculo.
#include "TestHarness.h"

#include "Game/PasoAnimacion.h"
#include "Game/RutinasPnjAnimacion.h"

#include <cmath>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

using namespace Game;

namespace {

bool existeLegacy(const IdClip& id) {
    static const std::set<std::string> legacy = {"idle",       "walk_up",    "walk_right",
                                                 "walk_down",  "walk_left",  "idle_up",
                                                 "idle_right", "idle_down",  "idle_left"};
    return legacy.count(id.value()) > 0;
}

MaquinaAnimacion cargaPnj() {
    std::ifstream f("assets/animaciones/pnj_basico.estados.json");
    MOTOR_CHECK(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    auto doc = JsonValue::parse(ss.str());
    MOTOR_CHECK(doc.isOk());
    auto r = leerCatalogoMaquinas(doc.value(), existeLegacy);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().size() == 1);
    return r.value()[0];
}

EstadoPnj pnj(float progreso, float actividad, MovimientoPnj mov, int rumbo, float reloj) {
    EstadoPnj e;
    e.progreso = progreso;
    e.actividad = actividad;
    e.def.movimiento = mov;
    e.rumbo = rumbo;
    e.reloj = reloj;
    return e;
}

// Claves emitidas: declaradas en la maquina o "rumbo"; valores finitos; rumbo en 0..3.
void compruebaEntradas(const MaquinaAnimacion& m, const EntradasAnimacion& en) {
    for (const auto& kv : en.valores) {
        bool declarada = kv.first.value() == kParametroRumbo;
        for (const ParametroAnimacion& p : m.parametros) {
            declarada = declarada || p.id == kv.first;
        }
        MOTOR_CHECK(declarada);
        MOTOR_CHECK(std::isfinite(kv.second));
    }
    const auto it = en.valores.find(IdParametro::parse(kParametroRumbo).value());
    MOTOR_CHECK(it != en.valores.end());
    const double r = it->second;
    MOTOR_CHECK(r == 0.0 || r == 1.0 || r == 2.0 || r == 3.0);
    MOTOR_CHECK(!en.finClip);
}

std::string clipMaquina(const MaquinaAnimacion& m, const EstadoPnj& e, bool reducidas) {
    const EntradasAnimacion en = entradasDePnj(e, reducidas);
    compruebaEntradas(m, en);
    EstadoAnimacionVivo vivo{m.inicial, IdClip::parse("idle_down").value()};
    for (int paso = 0; paso < 3; ++paso) {
        const PasoAnimacion p = pasoAnimacion(m, vivo, en);
        const bool mismoEstado = p.vivo.estado == vivo.estado;
        vivo = p.vivo;
        if (mismoEstado) {
            break;
        }
    }
    return vivo.clip.value();
}

}  // namespace

MOTOR_TEST_CASE(contraejemplo_vaiven_con_reducidas_cae_al_defecto) {
    const MaquinaAnimacion m = cargaPnj();
    const EstadoPnj e = pnj(1.0f, 1.0f, MovimientoPnj::Vaiven, 2, 0.0f);
    MOTOR_CHECK(RutinasPnj::animacion(e, true) == "idle_down");
    MOTOR_CHECK(clipMaquina(m, e, true) == "idle_down");
    MOTOR_CHECK(RutinasPnj::animacion(e, false) == "idle_left");
    MOTOR_CHECK(clipMaquina(m, e, false) == "idle_left");
}

MOTOR_TEST_CASE(entradas_concretas) {
    const EstadoPnj giro = pnj(1.0f, 1.0f, MovimientoPnj::Giro, 3, 2.5f);  // 3 + 3 = 6 -> 2
    const EntradasAnimacion g = entradasDePnj(giro, true);
    MOTOR_CHECK(g.valores.at(IdParametro::parse("rumbo").value()) == 2.0);
    MOTOR_CHECK(g.valores.at(IdParametro::parse("andando").value()) == 0.0);
    const EstadoPnj anda = pnj(0.5f, 1.0f, MovimientoPnj::Vaiven, 1, 0.0f);  // andando manda
    const EntradasAnimacion a = entradasDePnj(anda, false);
    MOTOR_CHECK(a.valores.at(IdParametro::parse("andando").value()) == 1.0);
    MOTOR_CHECK(a.valores.at(IdParametro::parse("rumbo").value()) == 1.0);
}

MOTOR_TEST_CASE(equivalencia_exhaustiva_con_el_oraculo) {
    const MaquinaAnimacion m = cargaPnj();
    const float progresos[] = {0.0f, 0.5f, 1.0f};
    const float actividades[] = {0.0f, 1.0f};
    const MovimientoPnj movs[] = {MovimientoPnj::Respirar, MovimientoPnj::Vaiven, MovimientoPnj::Golpe,
                                  MovimientoPnj::Giro, MovimientoPnj::AndarSitio};
    const float relojes[] = {0.0f, 0.3f, 0.7f, 1.25f, 2.5f};
    int total = 0;
    int discrepancias = 0;
    for (float pr : progresos)
        for (float ac : actividades)
            for (MovimientoPnj mv : movs)
                for (int rumbo = 0; rumbo < 4; ++rumbo)
                    for (float rl : relojes)
                        for (int red = 0; red < 2; ++red) {
                            const EstadoPnj e = pnj(pr, ac, mv, rumbo, rl);
                            ++total;
                            if (clipMaquina(m, e, red == 1) != RutinasPnj::animacion(e, red == 1)) {
                                ++discrepancias;
                            }
                        }
    MOTOR_CHECK(total == 3 * 2 * 5 * 4 * 5 * 2);
    MOTOR_CHECK(total >= 1200);
    MOTOR_CHECK(discrepancias == 0);
}

// Fronteras del vaiven (multiplos de 0.6 +- unos ulps de float): la aritmetica float del
// productor debe coincidir con la del oraculo bit a bit (un double podria desempatar distinto).
MOTOR_TEST_CASE(vaiven_en_las_fronteras_de_float) {
    const MaquinaAnimacion m = cargaPnj();
    int casos = 0;
    for (int k = 1; k <= 12; ++k) {
        float base = 0.6f * static_cast<float>(k);
        float arriba = base;
        float abajo = base;
        for (int ulp = 0; ulp <= 4; ++ulp) {
            for (float rl : {arriba, abajo}) {
                const EstadoPnj e = pnj(1.0f, 1.0f, MovimientoPnj::Vaiven, 2, rl);
                MOTOR_CHECK(clipMaquina(m, e, false) == RutinasPnj::animacion(e, false));
                ++casos;
            }
            arriba = std::nextafter(arriba, 1.0e9f);
            abajo = std::nextafter(abajo, 0.0f);
        }
    }
    MOTOR_CHECK(casos == 120);
}

int main() { return motor_test::runAllTests(); }
