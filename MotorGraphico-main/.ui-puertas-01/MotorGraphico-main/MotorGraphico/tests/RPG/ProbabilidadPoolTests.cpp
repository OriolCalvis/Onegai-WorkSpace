// AV13-04 — La probabilidad exacta de una tirada enfrentada
// (include/RPG/ProbabilidadPool.h).
//
// Tres comprobaciones:
//   1. Exacta: para 0 a 6 dados y los tres modos, la cuenta coincide caso
//      a caso con TODAS las tiradas posibles pasadas por roll_pool.
//   2. Bordes: pool vacío, un dado, dos 6, el crítico de D0013 con 2 a 6
//      dados y la pifia.
//   3. Contra el azar: los 81 pares 0..8 x 0..8 con las nueve mezclas de
//      ventaja/desventaja, contra 200 000 tiradas de roll_pool cada uno.
//      Todo tiene que quedar a menos de 0,5 puntos.
//
// GL-free y sin assets.
#include "TestHarness.h"

#include "RPG/DicePoolEngine.h"
#include "RPG/ProbabilidadPool.h"
#include "RPG/RandomEngine.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using RPG::Degree;
using RPG::DiceMod;
using RPG::DicePoolEngine;
using RPG::DistribucionPool;
using RPG::PoolResult;
using RPG::ProbabilidadEnfrentada;
using RPG::ProbabilidadPool;
using RPG::TipoTirada;

namespace {

const std::array<DiceMod, 3> kModos = {DiceMod::NORMAL, DiceMod::ADVANTAGE, DiceMod::DISADVANTAGE};

const char* nombreModo(DiceMod m) {
    switch (m) {
        case DiceMod::ADVANTAGE: return "ventaja";
        case DiceMod::DISADVANTAGE: return "desventaja";
        default: return "normal";
    }
}

// Devuelve las caras que se le dan, en orden.
class Caras : public RPG::RandomEngine {
public:
    explicit Caras(std::vector<int> caras) : m_caras(std::move(caras)) {}
    int roll_d6() override { return m_caras[m_siguiente++]; }
    int roll_in_range(int a, int) override { return a; }
    uint64_t seed() const override { return 0; }

private:
    std::vector<int> m_caras;
    std::size_t m_siguiente = 0;
};

TipoTirada tipoDe(const PoolResult& r) {
    if (r.critical) return TipoTirada::Critico;
    if (r.botch) return TipoTirada::Pifia;
    return TipoTirada::Normal;
}

int medios(float exitos) { return static_cast<int>(std::lround(exitos * 2.0f)); }

bool cerca(double a, double b) { return std::fabs(a - b) < 1e-12; }

// Una semilla distinta y bien mezclada para cada pool (splitmix64).
uint64_t semilla(uint64_t n) {
    uint64_t z = n + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

}  // namespace

MOTOR_TEST_CASE(la_cuenta_es_la_de_todas_las_tiradas_posibles) {
    for (DiceMod mod : kModos) {
        for (int n = 0; n <= 6; ++n) {
            DistribucionPool porMano;
            porMano.dados = n;
            porMano.total = 1;
            for (int i = 0; i < n; ++i) porMano.total *= 6;
            porMano.casos.assign(static_cast<std::size_t>(2 * n + 1), {0, 0, 0});
            std::vector<int> caras(static_cast<std::size_t>(n), 1);
            for (uint64_t k = 0; k < porMano.total; ++k) {
                uint64_t resto = k;
                for (int i = 0; i < n; ++i) {
                    caras[static_cast<std::size_t>(i)] = static_cast<int>(resto % 6) + 1;
                    resto /= 6;
                }
                Caras rng(caras);
                const PoolResult r = DicePoolEngine::roll_pool(n, mod, rng);
                ++porMano.casos[static_cast<std::size_t>(medios(r.successes))]
                               [static_cast<std::size_t>(tipoDe(r))];
            }
            const DistribucionPool d = ProbabilidadPool::pool(n, mod);
            MOTOR_CHECK(d.total == porMano.total);
            MOTOR_CHECK(d.casos == porMano.casos);
        }
    }
}

MOTOR_TEST_CASE(bordes_pool_vacio_un_dado_y_dos_seises) {
    // Sin dados: 0 éxitos seguro, ni crítico ni pifia.
    const DistribucionPool cero = ProbabilidadPool::pool(0, DiceMod::NORMAL);
    MOTOR_CHECK(cero.total == 1);
    MOTOR_CHECK(cero.cuenta(0, TipoTirada::Normal) == 1);
    MOTOR_CHECK(cero.probCritico() == 0.0 && cero.probPifia() == 0.0);
    const ProbabilidadEnfrentada nada = ProbabilidadPool::enfrentada(0, DiceMod::NORMAL, 0, DiceMod::NORMAL);
    MOTOR_CHECK(nada.empata == 1.0 && nada.gana == 0.0 && nada.pierde == 0.0);
    MOTOR_CHECK(nada.grado[static_cast<std::size_t>(Degree::PARTIAL)] == 1.0);
    // Menos de cero es cero, y más de 24 es 24, como en roll_pool.
    MOTOR_CHECK(ProbabilidadPool::pool(-3, DiceMod::NORMAL).casos == cero.casos);
    MOTOR_CHECK(ProbabilidadPool::pool(30, DiceMod::NORMAL).dados == 24);

    // Un dado: nunca crítico ni pifia. Gana a nadie con un 5 o un 6.
    const ProbabilidadEnfrentada uno = ProbabilidadPool::enfrentada(1, DiceMod::NORMAL, 0, DiceMod::NORMAL);
    MOTOR_CHECK(cerca(uno.gana, 2.0 / 6.0) && cerca(uno.empata, 4.0 / 6.0) && uno.pierde == 0.0);
    MOTOR_CHECK(uno.criticoAtacante == 0.0 && uno.pifiaAtacante == 0.0);
    MOTOR_CHECK(cerca(uno.probDiferencia(1.0f), 1.0 / 6.0) && cerca(uno.probDiferencia(0.5f), 1.0 / 6.0));
    // Con ventaja también el 4; con desventaja solo el 6.
    MOTOR_CHECK(cerca(ProbabilidadPool::enfrentada(1, DiceMod::ADVANTAGE, 0, DiceMod::NORMAL).gana, 3.0 / 6.0));
    MOTOR_CHECK(cerca(ProbabilidadPool::enfrentada(1, DiceMod::DISADVANTAGE, 0, DiceMod::NORMAL).gana, 1.0 / 6.0));

    // Dos dados: crítico = los dos en 6 (1 de 36); pifia = 1 y 1 (1 de 36).
    const DistribucionPool dos = ProbabilidadPool::pool(2, DiceMod::NORMAL);
    MOTOR_CHECK(dos.cuentaTipo(TipoTirada::Critico) == 1 && dos.cuenta(4, TipoTirada::Critico) == 1);
    MOTOR_CHECK(dos.cuentaTipo(TipoTirada::Pifia) == 1 && dos.cuenta(0, TipoTirada::Pifia) == 1);
    // Dos 6 contra un defensor sin dados: gana con crítico 1 de 36.
    const ProbabilidadEnfrentada dosContraNada = ProbabilidadPool::enfrentada(dos, cero);
    MOTOR_CHECK(cerca(dosContraNada.ganaConCritico, 1.0 / 36.0));
    MOTOR_CHECK(cerca(dosContraNada.grado[static_cast<std::size_t>(Degree::CRITICAL)], 1.0 / 36.0));

    // Las sumas cuadran: ganar + empatar + perder = 1, y lo mismo los grados
    // y la diferencia; la diferencia positiva es «gana».
    const ProbabilidadEnfrentada e = ProbabilidadPool::enfrentada(5, DiceMod::ADVANTAGE, 3, DiceMod::DISADVANTAGE);
    double grados = 0.0, dif = 0.0, positiva = 0.0;
    for (double g : e.grado) grados += g;
    for (std::size_t i = 0; i < e.diferencia.size(); ++i) {
        dif += e.diferencia[i];
        if (static_cast<int>(i) + e.diferenciaMinima > 0) positiva += e.diferencia[i];
    }
    MOTOR_CHECK(std::fabs(e.gana + e.empata + e.pierde - 1.0) < 1e-12);
    MOTOR_CHECK(std::fabs(grados - 1.0) < 1e-12 && std::fabs(dif - 1.0) < 1e-12);
    MOTOR_CHECK(std::fabs(positiva - e.gana) < 1e-12);
}

// D0013: la tabla de la decisión, con la regla vieja al lado. Con 4 dados
// (el estudio de dificultad del Tier 1) el crítico pasa del 0,08 % al 13,2 %.
MOTOR_TEST_CASE(el_critico_de_dos_seises_sale_lo_que_dice_D0013) {
    // Casos con al menos dos 6 = 6^n - 5^n - n * 5^(n-1).
    const std::array<uint64_t, 7> esperado = {0, 0, 1, 16, 171, 1526, 12281};
    const std::array<double, 7> tablaD0013 = {0.0, 0.0, 2.8, 7.4, 13.2, 19.6, 26.3};
    for (int n = 0; n <= 6; ++n) {
        const DistribucionPool d = ProbabilidadPool::pool(n, DiceMod::NORMAL);
        MOTOR_CHECK(d.cuentaTipo(TipoTirada::Critico) == esperado[static_cast<std::size_t>(n)]);
        MOTOR_CHECK(std::fabs(100.0 * d.probCritico() - tablaD0013[static_cast<std::size_t>(n)]) < 0.05);
        double todosSeis = n >= 2 ? 1.0 : 0.0;
        for (int i = 0; i < n; ++i) todosSeis /= 6.0;
        std::printf("  %d dados: critico %6.3f %% (antes, todos 6: %6.3f %%)\n", n,
                    100.0 * d.probCritico(), 100.0 * todosSeis);
    }
    // El modo no cambia el crítico: cuenta los 6, no los éxitos.
    for (DiceMod mod : kModos) {
        MOTOR_CHECK(ProbabilidadPool::pool(4, mod).cuentaTipo(TipoTirada::Critico) == 171);
    }
    // Tres dados con un solo 6 nunca son crítico: los 16 casos críticos de
    // tres dados son los que tienen dos o tres 6.
    const DistribucionPool tres = ProbabilidadPool::pool(3, DiceMod::NORMAL);
    MOTOR_CHECK(tres.cuenta(2, TipoTirada::Critico) == 0);  // 1 éxito: un solo 6
    MOTOR_CHECK(tres.cuenta(4, TipoTirada::Critico) == 12);  // 6, 6 y un 1-4 (3 x 4)
    MOTOR_CHECK(tres.cuenta(5, TipoTirada::Critico) == 3);   // 6, 6 y un 5
    MOTOR_CHECK(tres.cuenta(6, TipoTirada::Critico) == 1);   // 6, 6, 6
}

MOTOR_TEST_CASE(coincide_con_200000_tiradas_de_roll_pool) {
    constexpr int kTiradas = 200000;
    constexpr int kMax = 8;
    // Cada tirada, en un byte: medios éxitos (0..16), crítico y pifia.
    auto tirar = [](int n, DiceMod mod, uint64_t s) {
        RPG::Xoroshiro128p rng(semilla(s));
        std::vector<uint8_t> t(kTiradas);
        for (int i = 0; i < kTiradas; ++i) {
            const PoolResult r = DicePoolEngine::roll_pool(n, mod, rng);
            t[static_cast<std::size_t>(i)] = static_cast<uint8_t>(
                medios(r.successes) | (r.critical ? 32 : 0) | (r.botch ? 64 : 0));
        }
        return t;
    };
    // Atacantes y defensores con semillas distintas: pares independientes.
    std::vector<std::vector<uint8_t>> atk, def;
    std::vector<DistribucionPool> exacta;
    for (int m = 0; m < 3; ++m) {
        for (int n = 0; n <= kMax; ++n) {
            atk.push_back(tirar(n, kModos[static_cast<std::size_t>(m)], 1000u + 16u * m + n));
            def.push_back(tirar(n, kModos[static_cast<std::size_t>(m)], 5000u + 16u * m + n));
            exacta.push_back(ProbabilidadPool::pool(n, kModos[static_cast<std::size_t>(m)]));
        }
    }

    double peor = 0.0;
    std::string dondePeor;
    auto mira = [&](double exacto, uint64_t veces, const std::string& que) {
        const double d = std::fabs(100.0 * exacto - 100.0 * static_cast<double>(veces) / kTiradas);
        if (d > peor) { peor = d; dondePeor = que; }
    };

    // Grado de cada par de bytes, por el reglamento.
    auto grado = [](uint8_t a, uint8_t d) {
        PoolResult pa, pd;
        pa.successes = 0.5f * static_cast<float>(a & 31);
        pa.critical = (a & 32) != 0;
        pa.botch = (a & 64) != 0;
        pd.successes = 0.5f * static_cast<float>(d & 31);
        pd.critical = (d & 32) != 0;
        pd.botch = (d & 64) != 0;
        return static_cast<int>(DicePoolEngine::opposed_degree(pa, pd));
    };

    int pares = 0;
    for (std::size_t ia = 0; ia < atk.size(); ++ia) {
        // El pool solo: crítico, pifia y cada número de éxitos.
        uint64_t crit = 0, pifia = 0;
        std::array<uint64_t, 17> porMedios{};
        for (uint8_t b : atk[ia]) {
            crit += (b & 32) ? 1 : 0;
            pifia += (b & 64) ? 1 : 0;
            ++porMedios[b & 31];
        }
        const DistribucionPool& p = exacta[ia];
        const std::string pool = std::to_string(p.dados) + " " + nombreModo(p.mod);
        mira(p.probCritico(), crit, "critico de " + pool);
        mira(p.probPifia(), pifia, "pifia de " + pool);
        for (int m = 0; m <= 16; ++m) mira(p.probMedios(m), porMedios[static_cast<std::size_t>(m)], "exitos de " + pool);

        for (std::size_t id = 0; id < def.size(); ++id) {
            std::array<std::array<uint32_t, 128>, 128> veces{};
            for (int i = 0; i < kTiradas; ++i) ++veces[atk[ia][static_cast<std::size_t>(i)]][def[id][static_cast<std::size_t>(i)]];
            uint64_t gana = 0, empata = 0, pierde = 0, ganaCrit = 0;
            std::array<uint64_t, 4> grados{};
            std::array<uint64_t, 33> dif{};
            for (int a = 0; a < 128; ++a) {
                for (int d = 0; d < 128; ++d) {
                    const uint32_t v = veces[static_cast<std::size_t>(a)][static_cast<std::size_t>(d)];
                    if (v == 0) continue;
                    const int diferencia = (a & 31) - (d & 31);
                    if (diferencia > 0) { gana += v; if (a & 32) ganaCrit += v; }
                    else if (diferencia == 0) empata += v;
                    else pierde += v;
                    grados[static_cast<std::size_t>(grado(static_cast<uint8_t>(a), static_cast<uint8_t>(d)))] += v;
                    dif[static_cast<std::size_t>(diferencia + 16)] += v;
                }
            }
            const ProbabilidadEnfrentada e = ProbabilidadPool::enfrentada(exacta[ia], exacta[id]);
            const std::string par = pool + " contra " + std::to_string(exacta[id].dados) + " " +
                                    nombreModo(exacta[id].mod);
            mira(e.gana, gana, "gana, " + par);
            mira(e.empata, empata, "empata, " + par);
            mira(e.pierde, pierde, "pierde, " + par);
            mira(e.ganaConCritico, ganaCrit, "gana con critico, " + par);
            for (std::size_t g = 0; g < 4; ++g) mira(e.grado[g], grados[g], "grado, " + par);
            for (int x = -16; x <= 16; ++x) mira(e.probDiferencia(0.5f * x), dif[static_cast<std::size_t>(x + 16)], "diferencia, " + par);
            ++pares;
        }
    }
    std::printf("  %d pares x %d tiradas: la mayor distancia es %.3f puntos (%s)\n", pares, kTiradas,
                peor, dondePeor.c_str());
    const DistribucionPool cuatro = ProbabilidadPool::pool(4, DiceMod::NORMAL);
    uint64_t crit4 = 0;
    for (uint8_t b : atk[4]) crit4 += (b & 32) ? 1 : 0;
    std::printf("  critico con 4 dados: exacto %.3f %%, en %d tiradas %.3f %%\n", 100.0 * cuatro.probCritico(),
                kTiradas, 100.0 * static_cast<double>(crit4) / kTiradas);
    MOTOR_CHECK(pares == 27 * 27);
    MOTOR_CHECK(peor < 0.5);
}

int main() { return motor_test::runAllTests(); }
