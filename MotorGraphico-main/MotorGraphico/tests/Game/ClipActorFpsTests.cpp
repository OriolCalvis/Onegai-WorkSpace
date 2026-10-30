// U3-05.E08 -- Bucle y clip no ciclico con distintos dt (1/30, 1/60, 1/144 y uno grande): mismo
// fotograma final, mismos eventos (id y orden), cada evento una vez por paso real. Incluye el
// asset real del idle. Sin GL; el asset se lee desde el directorio del motor (WORKING_DIRECTORY).
#include "TestHarness.h"

#include <cmath>
#include <string>
#include <vector>

#include "Game/ClipActor.h"
#include "Game/IdleCuerpoBase.h"

using Game::ClipActor;
using Game::ClipAnimacion;
using Game::IdClip;

namespace {

IdClip idDe(const char* s) { return IdClip::parse(s).value(); }

struct Puertos : Game::PuertosEventoClip {
    std::vector<std::string> eventos;  // todos los tipos juntos, en orden de llegada
    void aplicaLogico(const Game::EventoFotograma& e) override { eventos.push_back(e.id); }
    void muestraVisual(const Game::EventoFotograma& e) override { eventos.push_back(e.id); }
    void suena(const Game::EventoFotograma& e) override { eventos.push_back(e.id); }
};

struct Destino : Game::DestinoFotograma {
    std::vector<int> recibidos;
    void ponerFotograma(int s) override { recibidos.push_back(s); }
};

// 3 fotogramas (sprites 11,12,13; 120/80/100 ms = 300 ms) con eventos en el primero (ini), en el
// segundo (a, b: mismo fotograma, orden declarado) y en el ultimo (ult).
ClipAnimacion clipEventos(bool bucle) {
    ClipAnimacion c;
    c.bucle = bucle;
    c.fotogramas = {{11, 120}, {12, 80}, {13, 100}};
    c.eventos = {{"ini", 0, "logico", "", 0}, {"a", 1, "visual", "", 1},
                 {"b", 1, "sonido", "", 2}, {"ult", 2, "logico", "", 3}};
    return c;
}

const std::vector<double> kDts = {1.0 / 30.0, 1.0 / 60.0, 1.0 / 144.0, 0.5};

// Avanza `total` segundos en pasos de dt (numero entero de pasos).
void corre(ClipActor& a, double dt, double total, Puertos& p, Destino& d) {
    const long pasos = std::lround(total / dt);
    for (long i = 0; i < pasos; ++i) MOTOR_CHECK(a.avanza(dt, p, d) == 0);
}

using V = std::vector<std::string>;
// Referencia a mano para 1000 ms con 120/80/100 (frontera en 120, 200, 300, 420, 500, 600, ...).
const V kRefCiclico = {"ini", "a", "b", "ult", "ini", "a", "b", "ult",
                       "ini", "a", "b", "ult", "ini"};
const V kRefNoCiclico = {"ini", "a", "b", "ult"};

}  // namespace

MOTOR_TEST_CASE(clip_ciclico_igual_con_todos_los_dt) {
    for (double dt : kDts) {
        ClipActor a(idDe("ciclo"), clipEventos(true));
        Puertos p;
        Destino d;
        corre(a, dt, 1.0, p, d);  // 1000 ms = 3 vueltas + 100 ms: fotograma 0, vuelta 3
        MOTOR_CHECK(p.eventos == kRefCiclico);
        MOTOR_CHECK(a.cursor().fotograma == 0);
        MOTOR_CHECK(a.cursor().vueltas == 3);
        MOTOR_CHECK(!a.cursor().terminado);
        MOTOR_CHECK(!d.recibidos.empty() && d.recibidos.back() == 11);
        if (dt < 0.1) {  // dt pequeno: se ve cada fotograma en orden: 3 vueltas y el primero otra vez
            const std::vector<int> ref = {11, 12, 13, 11, 12, 13, 11, 12, 13, 11};
            MOTOR_CHECK(d.recibidos == ref);
        }
    }
}

MOTOR_TEST_CASE(clip_no_ciclico_termina_igual_con_todos_los_dt_y_calla) {
    for (double dt : kDts) {
        ClipActor a(idDe("una"), clipEventos(false));
        Puertos p;
        Destino d;
        corre(a, dt, 1.0, p, d);  // 1000 ms >> 300 ms: terminado en el ultimo fotograma
        MOTOR_CHECK(p.eventos == kRefNoCiclico);
        MOTOR_CHECK(a.cursor().terminado);
        MOTOR_CHECK(a.cursor().fotograma == 2);
        MOTOR_CHECK(a.cursor().vueltas == 0);
        MOTOR_CHECK(!d.recibidos.empty() && d.recibidos.back() == 13);
        const std::vector<int> entregados = d.recibidos;
        corre(a, dt, 2.0, p, d);  // ya terminado: ni eventos ni sprites nuevos
        MOTOR_CHECK(p.eventos == kRefNoCiclico);
        MOTOR_CHECK(d.recibidos == entregados);
        MOTOR_CHECK(a.cursor().fotograma == 2);
        MOTOR_CHECK(a.cursor().terminado);
    }
}

MOTOR_TEST_CASE(cada_evento_una_vez_por_paso_real_incluido_el_ultimo_fotograma) {
    // Paso a paso (1/144 s): "ini" suena vueltas+1 veces y "ult" vueltas (+1 mientras el cursor
    // esta en el ultimo fotograma); al cerrar la vuelta "ult" no se repite.
    ClipActor a(idDe("ciclo"), clipEventos(true));
    Puertos p;
    Destino d;
    for (int i = 0; i < 144; ++i) {
        MOTOR_CHECK(a.avanza(1.0 / 144.0, p, d) == 0);
        long ini = 0, ult = 0;
        for (const std::string& e : p.eventos) {
            if (e == "ini") ++ini;
            if (e == "ult") ++ult;
        }
        const long vueltas = a.cursor().vueltas;
        MOTOR_CHECK(ini == vueltas + 1);
        MOTOR_CHECK(ult == vueltas + (a.cursor().fotograma == 2 ? 1 : 0));
    }
    MOTOR_CHECK(p.eventos == kRefCiclico);
}

MOTOR_TEST_CASE(idle_real_recorrido_completo_de_12_fotogramas_con_todos_los_dt) {
    const ClipAnimacion* clip = Game::idleCuerpoBase();
    MOTOR_CHECK(clip != nullptr);
    if (clip == nullptr) return;
    MOTOR_CHECK(clip->bucle);
    MOTOR_CHECK(clip->fotogramas.size() == 12u);
    // 5000 ms = 1 vuelta de 2640 ms (x1) + 2360 ms: 2360 / 220 = 10,7 -> fotograma 10 (sprite 11).
    for (double dt : kDts) {
        ClipActor a(idDe("idle"), *clip);
        Puertos p;
        Destino d;
        corre(a, dt, 5.0, p, d);
        MOTOR_CHECK(p.eventos.empty());
        MOTOR_CHECK(a.cursor().vueltas == 1);
        MOTOR_CHECK(a.cursor().fotograma == 10);
        MOTOR_CHECK(!d.recibidos.empty() && d.recibidos.back() == 11);
        if (dt < 0.1) {
            std::vector<int> ref;
            for (int v = 0; v < 2; ++v)
                for (int s = 1; s <= 12 && !(v == 1 && s > 11); ++s) ref.push_back(s);
            MOTOR_CHECK(d.recibidos == ref);  // 1..12, 1..11 sin saltos ni repeticiones
        }
    }
}

int main() { return motor_test::runAllTests(); }
