// AV13-04 — ver include/RPG/ProbabilidadPool.h.
#include "RPG/ProbabilidadPool.h"

#include <algorithm>
#include <cmath>

namespace RPG {

namespace {

// Un RandomEngine que siempre saca la misma cara: para preguntarle a
// roll_pool cuánto vale cada cara con cada modo, en vez de copiar la regla.
class CaraFija : public RandomEngine {
public:
    explicit CaraFija(int cara) : m_cara(cara) {}
    int roll_d6() override { return m_cara; }
    int roll_in_range(int a, int) override { return a; }
    uint64_t seed() const override { return 0; }

private:
    int m_cara;
};

// Medios éxitos de cada cara (índice 1..6) con un modo.
std::array<int, 7> mediosPorCara(DiceMod mod) {
    std::array<int, 7> medios{};
    for (int cara = 1; cara <= 6; ++cara) {
        CaraFija rng(cara);
        const float exitos = DicePoolEngine::roll_pool(1, mod, rng).successes;
        medios[cara] = static_cast<int>(std::lround(exitos * 2.0f));
    }
    return medios;
}

// El estado de la cuenta tras unos cuantos dados: los medios éxitos, los 6
// (0, 1 o 2 y más), si ha salido algún 5 o 6, y los 1 (0, 1 o 2 y más).
// Basta para saber al final si es crítico (dos 6) o pifia (ningún 5/6 y
// dos 1).
constexpr int kEstadosPorMedio = 3 * 2 * 3;

int indice(int medios, int seises, int con5o6, int unos) {
    return ((medios * 3 + seises) * 2 + con5o6) * 3 + unos;
}

}  // namespace

uint64_t DistribucionPool::cuenta(int medios, TipoTirada t) const {
    if (medios < 0 || medios >= static_cast<int>(casos.size())) return 0;
    return casos[static_cast<std::size_t>(medios)][static_cast<std::size_t>(t)];
}

uint64_t DistribucionPool::cuentaTipo(TipoTirada t) const {
    uint64_t n = 0;
    for (const auto& c : casos) n += c[static_cast<std::size_t>(t)];
    return n;
}

double DistribucionPool::prob(int medios, TipoTirada t) const {
    return static_cast<double>(cuenta(medios, t)) / static_cast<double>(total);
}

double DistribucionPool::probMedios(int medios) const {
    return prob(medios, TipoTirada::Normal) + prob(medios, TipoTirada::Critico) +
           prob(medios, TipoTirada::Pifia);
}

double DistribucionPool::probCritico() const {
    return static_cast<double>(cuentaTipo(TipoTirada::Critico)) / static_cast<double>(total);
}

double DistribucionPool::probPifia() const {
    return static_cast<double>(cuentaTipo(TipoTirada::Pifia)) / static_cast<double>(total);
}

double DistribucionPool::exitosMedios() const {
    double media = 0.0;
    for (std::size_t m = 0; m < casos.size(); ++m) {
        media += 0.5 * static_cast<double>(m) * probMedios(static_cast<int>(m));
    }
    return media;
}

double ProbabilidadEnfrentada::probDiferencia(float exitos) const {
    const int medios = static_cast<int>(std::lround(exitos * 2.0f));
    const int i = medios - diferenciaMinima;
    if (i < 0 || i >= static_cast<int>(diferencia.size())) return 0.0;
    return diferencia[static_cast<std::size_t>(i)];
}

DistribucionPool ProbabilidadPool::pool(int dados, DiceMod mod) {
    dados = std::max(0, std::min(dados, kMaxDados));
    const std::array<int, 7> medios = mediosPorCara(mod);
    const int maxMedios = 2 * dados;

    // Convolución dado a dado: cada estado reparte su cuenta entre las seis
    // caras del dado siguiente.
    std::vector<uint64_t> cuenta(static_cast<std::size_t>((maxMedios + 1) * kEstadosPorMedio), 0);
    std::vector<uint64_t> siguiente(cuenta.size(), 0);
    cuenta[static_cast<std::size_t>(indice(0, 0, 0, 0))] = 1;
    for (int dado = 0; dado < dados; ++dado) {
        std::fill(siguiente.begin(), siguiente.end(), 0);
        for (int m = 0; m <= 2 * dado; ++m) {
            for (int s = 0; s < 3; ++s) {
                for (int c = 0; c < 2; ++c) {
                    for (int u = 0; u < 3; ++u) {
                        const uint64_t n = cuenta[static_cast<std::size_t>(indice(m, s, c, u))];
                        if (n == 0) continue;
                        for (int cara = 1; cara <= 6; ++cara) {
                            const int m2 = m + medios[static_cast<std::size_t>(cara)];
                            const int s2 = std::min(2, s + (cara == 6 ? 1 : 0));
                            const int c2 = (c == 1 || cara >= 5) ? 1 : 0;
                            const int u2 = std::min(2, u + (cara == 1 ? 1 : 0));
                            siguiente[static_cast<std::size_t>(indice(m2, s2, c2, u2))] += n;
                        }
                    }
                }
            }
        }
        cuenta.swap(siguiente);
    }

    DistribucionPool d;
    d.dados = dados;
    d.mod = mod;
    d.total = 1;
    for (int i = 0; i < dados; ++i) d.total *= 6;
    d.casos.assign(static_cast<std::size_t>(maxMedios + 1), {0, 0, 0});
    for (int m = 0; m <= maxMedios; ++m) {
        for (int s = 0; s < 3; ++s) {
            for (int c = 0; c < 2; ++c) {
                for (int u = 0; u < 3; ++u) {
                    const uint64_t n = cuenta[static_cast<std::size_t>(indice(m, s, c, u))];
                    if (n == 0) continue;
                    TipoTirada t = TipoTirada::Normal;
                    if (s >= DicePoolEngine::kSeisesParaCritico) t = TipoTirada::Critico;
                    else if (c == 0 && u >= 2) t = TipoTirada::Pifia;
                    d.casos[static_cast<std::size_t>(m)][static_cast<std::size_t>(t)] += n;
                }
            }
        }
    }
    return d;
}

ProbabilidadEnfrentada ProbabilidadPool::enfrentada(const DistribucionPool& atacante,
                                                    const DistribucionPool& defensor) {
    ProbabilidadEnfrentada r;
    const int maxA = static_cast<int>(atacante.casos.size()) - 1;
    const int maxD = static_cast<int>(defensor.casos.size()) - 1;
    r.diferenciaMinima = -maxD;
    r.diferencia.assign(static_cast<std::size_t>(maxA + maxD + 1), 0.0);
    r.criticoAtacante = atacante.probCritico();
    r.pifiaAtacante = atacante.probPifia();
    r.criticoDefensor = defensor.probCritico();
    r.pifiaDefensor = defensor.probPifia();

    paraCadaPar(atacante, defensor, [&r](const ResumenTirada& a, const ResumenTirada& d, double p) {
        const int dif = static_cast<int>(std::lround((a.exitos - d.exitos) * 2.0f));
        r.diferencia[static_cast<std::size_t>(dif - r.diferenciaMinima)] += p;
        if (dif > 0) {
            r.gana += p;
            if (a.critico) r.ganaConCritico += p;
        } else if (dif == 0) {
            r.empata += p;
        } else {
            r.pierde += p;
        }
        // El grado lo decide el reglamento, no una copia de él.
        PoolResult pa;
        pa.successes = a.exitos;
        pa.critical = a.critico;
        pa.botch = a.pifia;
        PoolResult pd;
        pd.successes = d.exitos;
        pd.critical = d.critico;
        pd.botch = d.pifia;
        r.grado[static_cast<std::size_t>(DicePoolEngine::opposed_degree(pa, pd))] += p;
    });
    return r;
}

ProbabilidadEnfrentada ProbabilidadPool::enfrentada(int dadosAtacante, DiceMod modAtacante,
                                                    int dadosDefensor, DiceMod modDefensor) {
    return enfrentada(pool(dadosAtacante, modAtacante), pool(dadosDefensor, modDefensor));
}

}  // namespace RPG
