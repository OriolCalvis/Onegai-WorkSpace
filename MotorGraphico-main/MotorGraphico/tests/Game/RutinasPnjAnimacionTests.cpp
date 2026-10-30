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
#include <initializer_list>
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

const char* const kClips[8] = {"idle_up", "idle_right", "idle_down", "idle_left",
                               "walk_up", "walk_right", "walk_down", "walk_left"};

// UN paso desde el estado vivo dado, con las entradas del PNJ; ademas exige punto fijo: un
// segundo paso con las mismas entradas no cambia estado ni clip ni pide cambio de clip.
EstadoAnimacionVivo unPaso(const MaquinaAnimacion& m, EstadoAnimacionVivo vivo,
                           const EntradasAnimacion& en) {
    const PasoAnimacion p = pasoAnimacion(m, vivo, en);
    const PasoAnimacion q = pasoAnimacion(m, p.vivo, en);
    MOTOR_CHECK(q.vivo.estado == p.vivo.estado);
    MOTOR_CHECK(q.vivo.clip == p.vivo.clip);
    MOTOR_CHECK(!q.cambioDeClip);
    return p.vivo;
}

// Compara contra el oraculo UN paso desde {reposo, andar} x los 8 clips (los 4 de cada estado
// y los 4 del otro, incoherentes). Devuelve el numero de discrepancias.
int discrepanciasDesdeTodoEstado(const MaquinaAnimacion& m, const EstadoPnj& e, bool reducidas) {
    const EntradasAnimacion en = entradasDePnj(e, reducidas);
    compruebaEntradas(m, en);
    const std::string esperado = RutinasPnj::animacion(e, reducidas);
    int malos = 0;
    for (const char* est : {"reposo", "andar"}) {
        for (const char* c : kClips) {
            const EstadoAnimacionVivo vivo{IdEstado::parse(est).value(), IdClip::parse(c).value()};
            if (unPaso(m, vivo, en).clip.value() != esperado) {
                ++malos;
            }
        }
    }
    return malos;
}

std::string clipMaquina(const MaquinaAnimacion& m, const EstadoPnj& e, bool reducidas) {
    const EntradasAnimacion en = entradasDePnj(e, reducidas);
    compruebaEntradas(m, en);
    return unPaso(m, {m.inicial, IdClip::parse("idle_down").value()}, en).clip.value();
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
                            discrepancias += discrepanciasDesdeTodoEstado(m, e, red == 1);
                        }
    MOTOR_CHECK(total == 3 * 2 * 5 * 4 * 5 * 2);  // x16 estados vivos de partida
    MOTOR_CHECK(total >= 1200);
    MOTOR_CHECK(discrepancias == 0);
}

// Fronteras de float (multiplos de `paso` +- unos ulps): la aritmetica float del productor debe
// coincidir bit a bit con la del oraculo (un double o `* 1.25f` desempataria distinto).
int fronterasFloat(const MaquinaAnimacion& m, MovimientoPnj mv, float paso, int multiplos,
                   std::initializer_list<float> extra) {
    int casos = 0;
    auto prueba = [&](float rl) {
        for (int rumbo : {0, 3}) {
            for (int red = 0; red < 2; ++red) {
                const EstadoPnj e = pnj(1.0f, 1.0f, mv, rumbo, rl);
                MOTOR_CHECK(discrepanciasDesdeTodoEstado(m, e, red == 1) == 0);
                ++casos;
            }
        }
    };
    for (int k = 1; k <= multiplos; ++k) {
        float arriba = paso * static_cast<float>(k);
        float abajo = arriba;
        for (int ulp = 0; ulp <= 4; ++ulp) {
            prueba(arriba);
            prueba(abajo);
            arriba = std::nextafter(arriba, 1.0e9f);
            abajo = std::nextafter(abajo, 0.0f);
        }
    }
    for (float rl : extra) {
        prueba(rl);
    }
    return casos;
}

MOTOR_TEST_CASE(vaiven_en_las_fronteras_de_float) {
    const MaquinaAnimacion m = cargaPnj();
    MOTOR_CHECK(fronterasFloat(m, MovimientoPnj::Vaiven, 0.6f, 12, {}) == 12 * 5 * 2 * 4);
}

MOTOR_TEST_CASE(giro_en_las_fronteras_de_float) {
    const MaquinaAnimacion m = cargaPnj();
    // 5.5999999f: reloj / 0.8f da 7 en float y 6 en double. 10.3999996f, 13.5999994f y
    // 16.7999992f: x / 0.8f != x * 1.25f (12 vs 13, 16 vs 17, 20 vs 21).
    MOTOR_CHECK(fronterasFloat(m, MovimientoPnj::Giro, 0.8f, 24,
                               {5.5999999f, 5.6f, 10.3999996f, 13.5999994f, 16.7999992f}) ==
                (24 * 5 * 2 + 5) * 4);
}

MOTOR_TEST_CASE(rumbo_fuera_de_rango_se_emite_en_0_a_3_y_coincide_con_el_oraculo) {
    const MaquinaAnimacion m = cargaPnj();
    for (int rumbo : {-5, -1, 4, 7}) {
        for (MovimientoPnj mv : {MovimientoPnj::Respirar, MovimientoPnj::Vaiven,
                                 MovimientoPnj::Giro, MovimientoPnj::AndarSitio}) {
            for (float pr : {0.5f, 1.0f}) {
                for (int red = 0; red < 2; ++red) {
                    const EstadoPnj e = pnj(pr, 1.0f, mv, rumbo, 0.7f);
                    MOTOR_CHECK(discrepanciasDesdeTodoEstado(m, e, red == 1) == 0);
                }
            }
        }
    }
}

// Secuencia viva: el estado vivo se arrastra de paso en paso sin reiniciar; el PNJ avanza en el
// tiempo (reloj += 1/60, progreso que sube a 1, actividad que se enciende y se apaga).
MOTOR_TEST_CASE(secuencia_viva_sin_reiniciar_el_estado) {
    const MaquinaAnimacion m = cargaPnj();
    int pasos = 0;
    int cambiosDeEstado = 0;
    for (MovimientoPnj mv : {MovimientoPnj::Respirar, MovimientoPnj::Vaiven, MovimientoPnj::Golpe,
                             MovimientoPnj::Giro, MovimientoPnj::AndarSitio}) {
        for (int rumbo = 0; rumbo < 4; ++rumbo) {
            for (int red = 0; red < 2; ++red) {
                EstadoPnj e = pnj(1.0f, 0.0f, mv, rumbo, 0.0f);
                EstadoAnimacionVivo vivo{m.inicial, IdClip::parse("idle_down").value()};
                for (int t = 0; t < 900; ++t) {
                    e.reloj += 1.0f / 60.0f;
                    // 0..119 parado; 120..179 andando (progreso 0 -> 1); luego parado.
                    e.progreso = (t >= 120 && t < 180) ? static_cast<float>(t - 120) / 60.0f : 1.0f;
                    if (t % 300 == 150) {
                        e.rumbo = (e.rumbo + 1) % 4;  // gira a mitad de paso
                    }
                    e.actividad = (t % 300 >= 200) ? 1.0f : 0.0f;  // se enciende y se apaga
                    const EntradasAnimacion en = entradasDePnj(e, red == 1);
                    compruebaEntradas(m, en);
                    const PasoAnimacion p = pasoAnimacion(m, vivo, en);
                    cambiosDeEstado += p.vivo.estado == vivo.estado ? 0 : 1;
                    vivo = p.vivo;
                    ++pasos;
                    MOTOR_CHECK(vivo.clip.value() == RutinasPnj::animacion(e, red == 1));
                }
            }
        }
    }
    MOTOR_CHECK(pasos == 5 * 4 * 2 * 900);
    MOTOR_CHECK(cambiosDeEstado > 0);  // ejercita reposo->andar y andar->reposo
}

// Politica fijada (contrato 2): ambas transiciones de pnj_basico son reiniciarSiempre. Con este
// asset es indistinguible de reiniciarSiDistinto para el nombre del clip; la vigila E06a.
MOTOR_TEST_CASE(politica_de_las_transiciones_es_reiniciar_siempre) {
    const MaquinaAnimacion m = cargaPnj();
    MOTOR_CHECK(m.transiciones.size() == 2);
    MOTOR_CHECK(m.transiciones[0].politica == PoliticaCambio::ReiniciarSiempre);
    MOTOR_CHECK(m.transiciones[1].politica == PoliticaCambio::ReiniciarSiempre);
}

int main() { return motor_test::runAllTests(); }
