// E-GUION-01 — El guion de captura: parseo, errores con numero de linea,
// estado de las teclas por fotograma y lista de capturas. Sin GL ni ventana.
#include "Engine/GuionCaptura.h"

#include <string>

#include "TestHarness.h"

namespace {

using Engine::GuionCaptura;
using Engine::Input::InputKey;

GuionCaptura guionDe(const std::string& texto) {
    auto r = GuionCaptura::parsea(texto);
    if (!r.isOk()) throw motor_test::AssertionFailure(r.errorMessage().c_str(), __FILE__, __LINE__);
    return r.value();
}

bool abajo(const GuionCaptura& g, int fotograma, InputKey k) { return g.teclas(fotograma).isDown(k); }

// Un texto que debe fallar: devuelve el mensaje de error ("" si parseo bien).
std::string error(const std::string& texto) {
    const auto r = GuionCaptura::parsea(texto);
    return r.isOk() ? std::string() : r.errorMessage();
}

bool contiene(const std::string& texto, const std::string& trozo) { return texto.find(trozo) != std::string::npos; }

MOTOR_TEST_CASE(parsea_ordenes_comentarios_y_lineas_en_blanco) {
    const auto g = guionDe(
        "# cabecera\n"
        "\n"
        "0 pulsa W   # andar\n"
        "10 SUELTA w\n"
        "12 Toca Enter\n"
        "   15 captura  final_1\n"
        "20 fin\n");
    MOTOR_CHECK(g.ordenes().size() == 5);
    MOTOR_CHECK(g.ordenes()[0].tipo == GuionCaptura::Tipo::Pulsa);
    MOTOR_CHECK(g.ordenes()[0].tecla == InputKey::W);
    MOTOR_CHECK(g.ordenes()[0].linea == 3);
    MOTOR_CHECK(g.ordenes()[2].tipo == GuionCaptura::Tipo::Toca);
    MOTOR_CHECK(g.ordenes()[2].tecla == InputKey::Enter);
    MOTOR_CHECK(g.ordenes()[3].tipo == GuionCaptura::Tipo::Captura);
    MOTOR_CHECK(g.ordenes()[3].nombre == "final_1");
    MOTOR_CHECK(g.ordenes()[3].linea == 6);
    MOTOR_CHECK(g.ordenes()[4].tipo == GuionCaptura::Tipo::Fin);
    MOTOR_CHECK(g.ultimoFotograma() == 20);
}

MOTOR_TEST_CASE(los_nombres_de_tecla_son_legibles) {
    const std::pair<const char*, InputKey> casos[] = {
        {"W", InputKey::W},           {"a", InputKey::A},         {"S", InputKey::S},
        {"D", InputKey::D},           {"ENTER", InputKey::Enter}, {"esc", InputKey::Escape},
        {"ESPACIO", InputKey::Space}, {"E", InputKey::E},         {"TAB", InputKey::Tab},
        {"M", InputKey::M},           {"ARRIBA", InputKey::UpArrow},
        {"ABAJO", InputKey::DownArrow}, {"IZQUIERDA", InputKey::LeftArrow},
        {"DERECHA", InputKey::RightArrow}, {"1", InputKey::Digit1}, {"0", InputKey::Digit0},
        {"9", InputKey::Digit9},
    };
    for (const auto& [nombre, esperada] : casos) {
        InputKey k = InputKey::Escape;
        MOTOR_CHECK(GuionCaptura::teclaDe(nombre, k));
        MOTOR_CHECK(k == esperada);
    }
    InputKey k = InputKey::W;
    MOTOR_CHECK(!GuionCaptura::teclaDe("PATATA", k));
    MOTOR_CHECK(!GuionCaptura::teclaDe("", k));
    MOTOR_CHECK(k == InputKey::W);  // no se toca si no existe
}

MOTOR_TEST_CASE(los_errores_dicen_la_linea_y_el_motivo) {
    const std::string tecla = error("0 pulsa W\n5 pulsa PATATA\n");
    MOTOR_CHECK(contiene(tecla, "linea 2"));
    MOTOR_CHECK(contiene(tecla, "PATATA"));

    const std::string orden = error("# c\n\n3 vuela W\n");
    MOTOR_CHECK(contiene(orden, "linea 3"));
    MOTOR_CHECK(contiene(orden, "vuela"));

    const std::string numero = error("0 pulsa W\n0 suelta W\nx pulsa W\n");
    MOTOR_CHECK(contiene(numero, "linea 3"));
    MOTOR_CHECK(!error("-1 pulsa W\n").empty());
    MOTOR_CHECK(contiene(error("-1 pulsa W\n"), "linea 1"));
    MOTOR_CHECK(!error("1.5 pulsa W\n").empty());

    MOTOR_CHECK(contiene(error("4 pulsa\n"), "linea 1"));
    MOTOR_CHECK(contiene(error("4 pulsa W E\n"), "linea 1"));
    MOTOR_CHECK(contiene(error("4\n"), "linea 1"));
    MOTOR_CHECK(contiene(error("4 captura\n"), "linea 1"));
    MOTOR_CHECK(contiene(error("4 captura a/b\n"), "linea 1"));
    MOTOR_CHECK(contiene(error("4 fin ya\n"), "linea 1"));
}

MOTOR_TEST_CASE(fotogramas_que_retroceden_capturas_repetidas_y_ordenes_tras_fin_son_errores) {
    const std::string atras = error("10 pulsa W\n\n9 suelta W\n");
    MOTOR_CHECK(contiene(atras, "linea 3"));
    MOTOR_CHECK(contiene(atras, "retroceder"));

    const std::string repetida = error("1 captura uno\n2 captura uno\n");
    MOTOR_CHECK(contiene(repetida, "linea 2"));
    MOTOR_CHECK(contiene(repetida, "uno"));

    const std::string tras = error("5 fin\n6 pulsa W\n");
    MOTOR_CHECK(contiene(tras, "linea 2"));
    MOTOR_CHECK(contiene(tras, "fin"));

    MOTOR_CHECK(!error("").empty());
    MOTOR_CHECK(!error("# solo comentarios\n\n").empty());
}

MOTOR_TEST_CASE(pulsa_mantiene_y_suelta_libera) {
    const auto g = guionDe("2 pulsa W\n5 suelta W\n");
    MOTOR_CHECK(!abajo(g, 0, InputKey::W));
    MOTOR_CHECK(!abajo(g, 1, InputKey::W));
    MOTOR_CHECK(abajo(g, 2, InputKey::W));   // pulsada en el fotograma de la orden
    MOTOR_CHECK(abajo(g, 3, InputKey::W));
    MOTOR_CHECK(abajo(g, 4, InputKey::W));
    MOTOR_CHECK(!abajo(g, 5, InputKey::W));  // suelta ya en su fotograma
    MOTOR_CHECK(!abajo(g, 6, InputKey::W));
    MOTOR_CHECK(!abajo(g, 2, InputKey::S));  // las demas, sin tocar
}

MOTOR_TEST_CASE(toca_dura_exactamente_un_fotograma) {
    const auto g = guionDe("3 toca ENTER\n10 fin\n");
    MOTOR_CHECK(!abajo(g, 2, InputKey::Enter));
    MOTOR_CHECK(abajo(g, 3, InputKey::Enter));
    MOTOR_CHECK(!abajo(g, 4, InputKey::Enter));
    MOTOR_CHECK(!abajo(g, 5, InputKey::Enter));
    MOTOR_CHECK(!abajo(g, 10, InputKey::Enter));
}

MOTOR_TEST_CASE(dos_toca_seguidos_dan_dos_pulsaciones_con_hueco) {
    const auto g = guionDe("3 toca E\n5 toca E\n");
    MOTOR_CHECK(abajo(g, 3, InputKey::E));
    MOTOR_CHECK(!abajo(g, 4, InputKey::E));
    MOTOR_CHECK(abajo(g, 5, InputKey::E));
    MOTOR_CHECK(!abajo(g, 6, InputKey::E));
}

MOTOR_TEST_CASE(un_pulsa_justo_despues_de_un_toca_no_queda_deshecho) {
    // La suelta del toca (fotograma 4) no debe pisar al pulsa del fotograma 4.
    const auto g = guionDe("3 toca W\n4 pulsa W\n8 suelta W\n");
    MOTOR_CHECK(abajo(g, 3, InputKey::W));
    MOTOR_CHECK(abajo(g, 4, InputKey::W));
    MOTOR_CHECK(abajo(g, 7, InputKey::W));
    MOTOR_CHECK(!abajo(g, 8, InputKey::W));
}

MOTOR_TEST_CASE(varias_teclas_a_la_vez_y_misma_linea_de_tiempo) {
    const auto g = guionDe("0 pulsa W\n0 pulsa D\n2 suelta W\n");
    MOTOR_CHECK(abajo(g, 0, InputKey::W) && abajo(g, 0, InputKey::D));
    MOTOR_CHECK(!abajo(g, 2, InputKey::W) && abajo(g, 2, InputKey::D));
}

MOTOR_TEST_CASE(lista_de_capturas_por_fotograma_y_total) {
    const auto g = guionDe("5 captura a\n5 captura b\n9 captura c\n12 fin\n");
    const auto en5 = g.capturasEn(5);
    MOTOR_CHECK(en5.size() == 2);
    MOTOR_CHECK(en5[0] == "a" && en5[1] == "b");
    MOTOR_CHECK(g.capturasEn(6).empty());
    MOTOR_CHECK(g.capturasEn(9).size() == 1);
    const auto todas = g.capturas();
    MOTOR_CHECK(todas.size() == 3);
    MOTOR_CHECK(todas[2] == "c");
    MOTOR_CHECK(GuionCaptura::ficheroDe("a") == "captura_a.ppm");
}

MOTOR_TEST_CASE(sin_fin_acaba_en_la_ultima_orden) {
    MOTOR_CHECK(guionDe("0 pulsa W\n7 captura x\n").ultimoFotograma() == 7);
    MOTOR_CHECK(guionDe("0 pulsa W\n7 captura x\n30 fin\n").ultimoFotograma() == 30);
}

MOTOR_TEST_CASE(un_guion_que_no_existe_da_error_con_su_ruta) {
    const auto r = GuionCaptura::carga("no/existe/guion.txt");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(contiene(r.errorMessage(), "no/existe/guion.txt"));
}

MOTOR_TEST_CASE(sin_guion_la_ejecucion_en_curso_deja_pasar_lo_real) {
    Engine::CapturaEnCurso c;
    c.situa(7);
    MOTOR_CHECK(!c.guionado());
    MOTOR_CHECK(c.reloj(12.5) == 12.5);
    MOTOR_CHECK(c.dt(0.031f, 1.0f / 60.0f) == 0.031f);
    MOTOR_CHECK(c.tomaCapturas().empty());
}

MOTOR_TEST_CASE(con_guion_la_ejecucion_en_curso_manda_en_teclas_reloj_dt_y_capturas) {
    Engine::CapturaEnCurso c;
    c.empieza(guionDe("2 pulsa W\n4 captura x\n6 fin\n"));
    MOTOR_CHECK(c.guionado());
    MOTOR_CHECK(c.fotogramas() == 7);  // 0..6
    c.situa(1);
    MOTOR_CHECK(!c.teclas().isDown(InputKey::W));
    MOTOR_CHECK(c.dt(0.5f, 1.0f / 60.0f) == 1.0f / 60.0f);
    c.situa(3);
    MOTOR_CHECK(c.teclas().isDown(InputKey::W));
    MOTOR_CHECK(c.reloj(99.0) == 3.0 / 60.0);  // el reloj del guion, no el de pared
    c.situa(4);
    const auto una = c.tomaCapturas();
    MOTOR_CHECK(una.size() == 1 && una[0] == "x");
    MOTOR_CHECK(c.tomaCapturas().empty());  // se piden una sola vez
    c.situa(5);
    MOTOR_CHECK(c.tomaCapturas().empty());
}

#ifdef MOTOR_FUENTES_DIR
MOTOR_TEST_CASE(los_guiones_de_ejemplo_del_repositorio_son_validos) {
    const auto r = GuionCaptura::carga(std::string(MOTOR_FUENTES_DIR) + "/tools/guiones/titulo_nueva_partida.txt");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(!r.value().capturas().empty());
}
#endif

}  // namespace

int main() { return motor_test::runAllTests(); }
