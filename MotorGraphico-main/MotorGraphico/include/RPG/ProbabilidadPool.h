#pragma once
// AV13-04 (COMBATE_BG3.md §6, D0014 §2) — La probabilidad EXACTA de una
// tirada enfrentada, para la ayuda de dados («ganar · roce · fallar» antes
// de tirar).
//
// No simula: cuenta. Recorre las 6^N tiradas de un pool por convolución,
// dado a dado, con las mismas reglas que DicePoolEngine::roll_pool:
//   - 6 = 1 éxito; 5 = medio; 1-4 = nada.
//   - Ventaja: el 4 también vale medio. Desventaja: solo cuenta el 6.
//   - Crítico (D0013): al menos dos 6. Con un dado, nunca.
//   - Pifia (Ed. 3): ningún 5 ni 6 y al menos dos 1.
// Los éxitos de cada cara los da roll_pool (una sola fuente de la regla).
// Las cuentas son enteras (uint64_t: 6^24 cabe), así que la probabilidad
// de un pool es exacta; la de la enfrentada se combina en double.
//
// Los éxitos van en MEDIOS (enteros): 3 medios = 1,5 éxitos.
//
// GL-free y sin estado. La ayuda en la barra de combate es AV13-05.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "RPG/DicePoolEngine.h"

namespace RPG {

// Crítico y pifia no pueden darse a la vez: el crítico pide dos 6 y la
// pifia no admite ninguno.
enum class TipoTirada : uint8_t { Normal = 0, Critico = 1, Pifia = 2 };

// Lo que importa de una tirada para compararla con otra.
struct ResumenTirada {
    float exitos = 0.0f;
    bool critico = false;
    bool pifia = false;
};

// Todas las tiradas de un pool, contadas.
struct DistribucionPool {
    int dados = 0;
    DiceMod mod = DiceMod::NORMAL;
    uint64_t total = 1;  // 6^dados
    // casos[m][t]: cuántas de las `total` tiradas suman m medios éxitos y
    // son del tipo t (TipoTirada).
    std::vector<std::array<uint64_t, 3>> casos;

    uint64_t cuenta(int medios, TipoTirada t) const;
    uint64_t cuentaTipo(TipoTirada t) const;
    double prob(int medios, TipoTirada t) const;
    double probMedios(int medios) const;  // de cualquier tipo
    double probCritico() const;
    double probPifia() const;
    double exitosMedios() const;  // la media, en éxitos
};

// El atacante contra el defensor.
struct ProbabilidadEnfrentada {
    // Por la diferencia de éxitos (atacante - defensor).
    double gana = 0.0;     // > 0
    double empata = 0.0;   // = 0: el roce de D0014
    double pierde = 0.0;   // < 0
    double ganaConCritico = 0.0;  // gana y el atacante saca dos 6
    double criticoAtacante = 0.0;
    double pifiaAtacante = 0.0;
    double criticoDefensor = 0.0;
    double pifiaDefensor = 0.0;
    // El grado de DicePoolEngine::opposed_degree, con sus pifias:
    // [BOTCH, PARTIAL, SUCCESS, CRITICAL].
    std::array<double, 4> grado{};
    // diferencia[i] = P(atacante - defensor == (diferenciaMinima + i) medios).
    int diferenciaMinima = 0;
    std::vector<double> diferencia;

    double probDiferencia(float exitos) const;
};

class ProbabilidadPool {
public:
    // El mismo techo que roll_pool.
    static constexpr int kMaxDados = 24;

    static DistribucionPool pool(int dados, DiceMod mod);
    static ProbabilidadEnfrentada enfrentada(const DistribucionPool& atacante,
                                             const DistribucionPool& defensor);
    static ProbabilidadEnfrentada enfrentada(int dadosAtacante, DiceMod modAtacante,
                                             int dadosDefensor, DiceMod modDefensor);

    // Todos los pares (atacante, defensor) con su probabilidad. Sirve para
    // el daño previsto: f(atacante, defensor, p) con RPG::dano::dadosFisicos.
    template <class F>
    static void paraCadaPar(const DistribucionPool& atacante,
                            const DistribucionPool& defensor, F&& f) {
        const double totalA = static_cast<double>(atacante.total);
        const double totalD = static_cast<double>(defensor.total);
        for (std::size_t ma = 0; ma < atacante.casos.size(); ++ma) {
            for (int ta = 0; ta < 3; ++ta) {
                const uint64_t ca = atacante.casos[ma][ta];
                if (ca == 0) continue;
                const ResumenTirada a = resumen(static_cast<int>(ma), ta);
                for (std::size_t md = 0; md < defensor.casos.size(); ++md) {
                    for (int td = 0; td < 3; ++td) {
                        const uint64_t cd = defensor.casos[md][td];
                        if (cd == 0) continue;
                        const double p = (static_cast<double>(ca) / totalA) *
                                         (static_cast<double>(cd) / totalD);
                        f(a, resumen(static_cast<int>(md), td), p);
                    }
                }
            }
        }
    }

private:
    static ResumenTirada resumen(int medios, int tipo) {
        ResumenTirada r;
        r.exitos = 0.5f * static_cast<float>(medios);
        r.critico = tipo == static_cast<int>(TipoTirada::Critico);
        r.pifia = tipo == static_cast<int>(TipoTirada::Pifia);
        return r;
    }
};

}  // namespace RPG
