// U3-05.E06 -- Enrutar eventos de clip a puertos (Game::enrutaEvento). Puertos falsos, sin audio.
#include "TestHarness.h"

#include "Game/CursorClip.h"
#include "Game/EventosClip.h"

#include <string>
#include <vector>

using Game::ClipAnimacion;
using Game::EventoFotograma;

namespace {

// Anota "L:id", "V:id" o "S:id" en el orden de llamada.
class PuertosFalsos : public Game::PuertosEventoClip {
public:
    std::vector<std::string> llamadas;
    void aplicaLogico(const EventoFotograma& e) override { llamadas.push_back("L:" + e.id); }
    void muestraVisual(const EventoFotograma& e) override { llamadas.push_back("V:" + e.id); }
    void suena(const EventoFotograma& e) override { llamadas.push_back("S:" + e.id); }
};

EventoFotograma ev(const std::string& id, const std::string& tipo) { return {id, 0, tipo, "", 0}; }

}  // namespace

MOTOR_TEST_CASE(cada_tipo_llama_a_un_solo_puerto) {
    const char* tipos[3] = {"logico", "visual", "sonido"};
    const char* marca[3] = {"L:x", "V:x", "S:x"};
    for (int i = 0; i < 3; ++i) {
        PuertosFalsos p;
        const auto r = Game::enrutaEvento(ev("x", tipos[i]), p);
        MOTOR_CHECK(r.isOk() && r.value());
        MOTOR_CHECK(p.llamadas.size() == 1 && p.llamadas[0] == marca[i]);
    }
}

MOTOR_TEST_CASE(tipo_desconocido_es_error_y_no_llama_a_nada) {
    for (const char* t : {"", "Logico", "logico ", "particula"}) {
        PuertosFalsos p;
        const auto r = Game::enrutaEvento(ev("x", t), p);
        MOTOR_CHECK(!r.isOk());
        MOTOR_CHECK(r.errorMessage() == std::string("tipo de evento no reconocido: ") + t);
        MOTOR_CHECK(p.llamadas.empty());
    }
}

MOTOR_TEST_CASE(el_evento_llega_entero_al_puerto) {
    class Captura : public Game::PuertosEventoClip {
    public:
        EventoFotograma visto;
        void aplicaLogico(const EventoFotograma& e) override { visto = e; }
        void muestraVisual(const EventoFotograma&) override {}
        void suena(const EventoFotograma&) override {}
    } c;
    const EventoFotograma e{"impacto", 2, "logico", "danio", 7};
    MOTOR_CHECK(Game::enrutaEvento(e, c).isOk());
    MOTOR_CHECK(c.visto.id == "impacto" && c.visto.fotograma == 2 && c.visto.dato == "danio" && c.visto.orden == 7);
}

MOTOR_TEST_CASE(enruta_los_eventos_de_un_paso_real_en_orden) {
    ClipAnimacion c;
    c.bucle = false;
    for (int i = 0; i < 2; ++i) c.fotogramas.push_back({i + 1, 50});
    c.eventos.push_back({"golpe", 0, "logico", "danio", 0});
    c.eventos.push_back({"chispa", 0, "visual", "flash", 1});
    c.eventos.push_back({"zas", 1, "sonido", "espada", 0});
    const Game::PasoClip paso = Game::avanzaClip({}, 1.0, c);
    MOTOR_CHECK(paso.eventos.size() == 3);
    PuertosFalsos p;
    for (const EventoFotograma& e : paso.eventos) MOTOR_CHECK(Game::enrutaEvento(e, p).isOk());
    MOTOR_CHECK(p.llamadas.size() == 3 && p.llamadas[0] == "L:golpe" && p.llamadas[1] == "V:chispa" &&
                p.llamadas[2] == "S:zas");
}

MOTOR_TEST_CASE(repintar_no_reaplica_el_logico) {
    // Solo el evento visual se repite: reenrutarlo no toca aplicaLogico ni suena.
    PuertosFalsos p;
    for (int i = 0; i < 3; ++i) MOTOR_CHECK(Game::enrutaEvento(ev("chispa", "visual"), p).isOk());
    MOTOR_CHECK(p.llamadas.size() == 3);
    for (const std::string& l : p.llamadas) MOTOR_CHECK(l == "V:chispa");
}

int main() { return motor_test::runAllTests(); }
