// C2-04a — test_escena_viva: postura/fuego/emoción y su caducidad, la
// reconstrucción por cinemáticas vistas, tumba+levanta y los alias de iconos.
#include <set>
#include <string>

#include <cmath>

#include "Game/CuadroGirado.h"
#include "Game/EscenaViva.h"
#include "TestHarness.h"

using namespace Game;

namespace {

CatalogoCinematicas catalogo(const std::string& texto) {
    auto r = CatalogoCinematicas::lee(texto);
    if (!r.isOk()) {
        MOTOR_CHECK_FAIL(r.errorMessage());
        return {};
    }
    return r.value();
}

}  // namespace

MOTOR_TEST_CASE(guarda_postura_fuego_y_emocion_y_avanza_caduca) {
    EscenaViva s;
    s.postura("edul", "herido");
    s.fuego(GridCoord{3, 4}, true);
    s.emocion("xila", "asustado", 1.0f);
    MOTOR_CHECK(s.posturaDe("edul") == "herido" && s.posturaDe("otro") == "de_pie");
    MOTOR_CHECK(s.hayFuego(GridCoord{3, 4}) && !s.hayFuego(GridCoord{4, 3}));
    MOTOR_CHECK(s.emociones().size() == 1);
    s.avanza(0.6f);
    MOTOR_CHECK(s.emociones().size() == 1);
    s.avanza(0.6f);
    MOTOR_CHECK(s.emociones().empty());
    s.fuego(GridCoord{3, 4}, false);
    MOTOR_CHECK(s.fuegos().empty());
}

MOTOR_TEST_CASE(reconstruye_el_fuego_solo_si_llamas_vista_y_rota_no) {
    const auto cat = catalogo(R"({"cinematicas": [
        {"id": "barricada_llamas", "nivel": "n1", "pasos": [{"arde": [1, 1]}, {"arde": [2, 1]}]},
        {"id": "barricada_rota", "nivel": "n1", "pasos": [{"apaga": [1, 1]}, {"apaga": [2, 1]}]},
        {"id": "otra", "nivel": "n2", "pasos": [{"arde": [9, 9]}]}]})");
    EscenaViva s;
    std::set<std::string> flags{"cine_barricada_llamas_vista", "cine_otra_vista"};
    auto tiene = [&](const std::string& f) { return flags.count(f) > 0; };
    s.reconstruye(cat, "n1", tiene);
    MOTOR_CHECK(s.fuegos().size() == 2 && s.hayFuego(GridCoord{1, 1}) && s.hayFuego(GridCoord{2, 1}));
    MOTOR_CHECK(!s.hayFuego(GridCoord{9, 9}));  // de otro nivel
    flags.insert("cine_barricada_rota_vista");
    s.reconstruye(cat, "n1", tiene);
    MOTOR_CHECK(s.fuegos().empty());
    flags.clear();
    s.reconstruye(cat, "n1", tiene);
    MOTOR_CHECK(s.fuegos().empty());
}

MOTOR_TEST_CASE(reconstruye_filtra_el_nivel_por_sufijo) {
    const auto cat = catalogo(R"({"cinematicas": [
        {"id": "a", "nivel": "afueras", "pasos": [{"arde": [1, 1]}]},
        {"id": "b", "nivel": "otro", "pasos": [{"arde": [2, 2]}]}]})");
    EscenaViva s;
    s.reconstruye(cat, "assets/levels/afueras", [](const std::string&) { return true; });
    MOTOR_CHECK(s.hayFuego(GridCoord{1, 1}) && !s.hayFuego(GridCoord{2, 2}));
}

MOTOR_TEST_CASE(cuadro_girado_geometria) {
    const Vector2 e{10.0f, 20.0f}, t{64.0f, 48.0f};  // no cuadrado: distingue ancho de alto
    auto cerca = [](const Vector2& p, float x, float y) { return std::fabs(p.x - x) < 1e-3f && std::fabs(p.y - y) < 1e-3f; };
    // Sin postura: el rectángulo.
    auto r = cuadroGirado(e, t, 0.0f, 1.0f);
    MOTOR_CHECK(cerca(r[0], 10, 20) && cerca(r[1], 74, 20) && cerca(r[2], 74, 68) && cerca(r[3], 10, 68));
    // Aplastado a 0,5: los pies fijos, la mitad de alto (24).
    auto a = cuadroGirado(e, t, 0.0f, 0.5f);
    MOTOR_CHECK(cerca(a[0], 10, 44) && cerca(a[1], 74, 44) && cerca(a[2], 74, 68) && cerca(a[3], 10, 68));
    // 90 grados: los pies (42, 68) no se mueven y la cabeza cae a la derecha.
    const float pi2 = 1.57079633f;
    auto g = cuadroGirado(e, t, pi2, 1.0f);
    MOTOR_CHECK(cerca(g[0], 90, 36) && cerca(g[1], 90, 100));
    MOTOR_CHECK(cerca(g[2], 42, 100) && cerca(g[3], 42, 36));
    // -90 grados: cae a la izquierda.
    auto h = cuadroGirado(e, t, -pi2, 1.0f);
    MOTOR_CHECK(cerca(h[0], -6, 100) && cerca(h[2], 42, 36));
}

MOTOR_TEST_CASE(tumba_y_levanta_en_orden_dejan_de_pie) {
    const auto cat = catalogo(R"({"cinematicas": [
        {"id": "a", "pasos": [{"tumba": "edul", "como": "herido"}, {"tumba": "xila", "como": "muerto"}]},
        {"id": "b", "pasos": [{"levanta": "edul"}]}]})");
    EscenaViva s;
    s.reconstruye(cat, "n1", [](const std::string&) { return true; });
    MOTOR_CHECK(s.posturaDe("edul") == "de_pie" && s.posturas().count("edul") == 0);
    MOTOR_CHECK(s.posturaDe("xila") == "muerto");
}

MOTOR_TEST_CASE(icono_de_emocion_con_alias) {
    MOTOR_CHECK(EscenaViva::iconoDeEmocion("alegre") == "encantado");
    MOTOR_CHECK(EscenaViva::iconoDeEmocion("enfadado") == "enfurecido");
    MOTOR_CHECK(EscenaViva::iconoDeEmocion("asustado") == "asustado");
}

MOTOR_TEST_CASE(alfa_de_entrada_y_salida_de_la_emocion) {
    EscenaViva s;
    s.emocion("edul", "!", 2.0f);
    MOTOR_CHECK(s.emociones()[0].alfa() == 0.0f);  // recien nacida: aun no se ve
    s.avanza(0.075f);
    MOTOR_CHECK(std::fabs(s.emociones()[0].alfa() - 0.5f) < 0.01f);  // a medio fundido de entrada
    s.avanza(1.0f);
    MOTOR_CHECK(s.emociones()[0].alfa() == 1.0f);  // en pleno
    s.avanza(0.8f);  // quedan 0.125 s de los 0.25 s de salida
    MOTOR_CHECK(std::fabs(s.emociones()[0].alfa() - 0.5f) < 0.01f);
    // Una emocion muy corta nunca pasa de 1 ni baja de 0.
    EscenaViva c;
    c.emocion("xila", "...", 0.2f);
    c.avanza(0.1f);
    const float a = c.emociones()[0].alfa();
    MOTOR_CHECK(a >= 0.0f && a <= 1.0f);
}

int main() { return motor_test::runAllTests(); }
