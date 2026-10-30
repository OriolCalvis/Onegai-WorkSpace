// EST-26 (relevo A) — La linea de comandos del editor.
//
// Lo que vigila, en una frase: que un argumento mal escrito se conteste
// con un mensaje util y no con un volcado de la libreria estandar, y que
// las reglas que ya habia (fotogramas, escala) sigan siendo las mismas.
//
// EL FALLO QUE CIERRA. EST-11 lo dejo anotado el 21/09 y no se podia
// arreglar: `motor_editor --frames abc` respondia «stoi: no conversion»
// porque run() usaba std::stoi, y run() vivia en un fichero por encima
// del techo al que el trinquete prohibia anadir ni una linea. Al sacar la
// linea de comandos a su propia unidad, el arreglo cabe y ademas se puede
// demostrar.

#include "EditorOpciones.h"

#include "TestHarness.h"

#include <string>
#include <vector>

namespace {

using namespace Editor;

// argv de mentira. AnalizaArgumentos no toca disco ni GL.
struct Linea {
    std::vector<std::string> texto;
    std::vector<char*> punteros;
    explicit Linea(std::initializer_list<const char*> args) {
        texto.emplace_back("motor_editor");
        for (const char* a : args) texto.emplace_back(a);
        for (auto& s : texto) punteros.push_back(s.data());
    }
    Result<EditorOpciones> analiza() {
        return AnalizaArgumentos(static_cast<int>(punteros.size()), punteros.data());
    }
};

// Atajo: una Linea temporal dentro de MOTOR_CHECK partiria la macro por
// la coma de la lista de inicializacion.
Result<EditorOpciones> analiza(std::initializer_list<const char*> args) {
    Linea linea(args);
    return linea.analiza();
}

bool dice(const Result<EditorOpciones>& r, const std::string& trozo) {
    return !r.isOk() && r.errorMessage().find(trozo) != std::string::npos;
}

// ---------------------------------------------------------------- lo normal

MOTOR_TEST_CASE(sin_argumentos_arranca_una_sesion_normal) {
    Linea l{};
    const auto r = l.analiza();
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().frames == -1);   // -1 = sin limite
    MOTOR_CHECK(!r.value().smoke);
    MOTOR_CHECK(!r.value().ayuda);
    MOTOR_CHECK(r.value().uiScale == 1.0f);
    MOTOR_CHECK(r.value().initial.empty());
}

MOTOR_TEST_CASE(entiende_lo_que_dice_la_ayuda) {
    Linea l{"--proyecto", "boundington", "--frames", "120",
            "--capture", "salida.ppm", "--ui-scale", "1.5", "--smoke-check"};
    const auto r = l.analiza();
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().projectId == "boundington");
    MOTOR_CHECK(r.value().frames == 120);
    MOTOR_CHECK(r.value().screenshot == "salida.ppm");
    MOTOR_CHECK(r.value().uiScale > 1.49f && r.value().uiScale < 1.51f);
    MOTOR_CHECK(r.value().smoke);
}

MOTOR_TEST_CASE(la_ayuda_es_un_dato_no_un_efecto) {
    // --help no imprime aqui: lo dice, y quien llame decide. Asi se puede
    // comprobar sin capturar stdout.
    Linea l{"--help"};
    const auto r = l.analiza();
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().ayuda);
    MOTOR_CHECK(std::string(AyudaDeUso()).find("--ui-scale") != std::string::npos);
}

MOTOR_TEST_CASE(la_ayuda_gana_a_lo_que_venga_detras) {
    // "--help --frames abc" tiene que ensenar la ayuda, no un error sobre
    // algo que el usuario ya no va a ejecutar.
    Linea l{"--help", "--frames", "abc"};
    const auto r = l.analiza();
    MOTOR_CHECK(r.isOk() && r.value().ayuda);
}

// ------------------------------------------------- lo que antes era un volcado

MOTOR_TEST_CASE(frames_que_no_es_un_numero_se_contesta_con_palabras) {
    // EL CASO DE EST-11. Antes: «stoi: no conversion».
    Linea l{"--frames", "abc"};
    const auto r = l.analiza();
    MOTOR_CHECK(dice(r, "--frames"));
    MOTOR_CHECK(dice(r, "abc"));
    MOTOR_CHECK(r.errorMessage().find("stoi") == std::string::npos);
}

MOTOR_TEST_CASE(un_numero_a_medias_no_cuela) {
    // std::stoi("12x") devuelve 12 tan contento. Aqui no: el texto tiene
    // que consumirse entero, o el usuario cree que escribio otra cosa.
    Linea l{"--frames", "12x"};
    MOTOR_CHECK(dice(l.analiza(), "12x"));
}

MOTOR_TEST_CASE(un_numero_enorme_no_da_la_vuelta) {
    Linea l{"--frames", "99999999999999999999"};
    MOTOR_CHECK(!l.analiza().isOk());
}

MOTOR_TEST_CASE(una_escala_que_no_es_finita_se_para_aqui) {
    // strtof acepta "nan" e "inf" sin queja, y una escala no finita
    // revienta mucho mas tarde y lejos de donde se escribio.
    MOTOR_CHECK(!analiza({"--ui-scale", "nan"}).isOk());
    MOTOR_CHECK(!analiza({"--ui-scale", "inf"}).isOk());
    MOTOR_CHECK(!analiza({"--ui-scale", "uno"}).isOk());
}

MOTOR_TEST_CASE(una_opcion_sin_valor_no_lee_fuera_de_argv) {
    // "--frames" como ultimo argumento: antes se leia argv[argc].
    MOTOR_CHECK(dice(analiza({"--frames"}), "Falta valor"));
    MOTOR_CHECK(dice(analiza({"--load"}), "Falta valor"));
}

MOTOR_TEST_CASE(una_opcion_desconocida_se_nombra) {
    MOTOR_CHECK(dice(analiza({"--vuela"}), "--vuela"));
}

// ------------------------------------------------------- las reglas de siempre

MOTOR_TEST_CASE(cero_fotogramas_no_es_una_sesion) {
    // Un editor que se cierra antes de dibujar nada no es lo que nadie pide.
    MOTOR_CHECK(!analiza({"--frames", "0"}).isOk());
    MOTOR_CHECK(!analiza({"--frames", "-2"}).isOk());
    // -1 SI es valido: es "sin limite".
    MOTOR_CHECK(analiza({"--frames", "-1"}).isOk());
}

MOTOR_TEST_CASE(la_escala_vive_entre_uno_y_dos) {
    MOTOR_CHECK(analiza({"--ui-scale", "1"}).isOk());
    MOTOR_CHECK(analiza({"--ui-scale", "2"}).isOk());
    MOTOR_CHECK(!analiza({"--ui-scale", "0.9"}).isOk());
    MOTOR_CHECK(!analiza({"--ui-scale", "2.1"}).isOk());
    // Y el mensaje dice POR QUE, no solo que no.
    MOTOR_CHECK(dice(analiza({"--ui-scale", "3"}), "no cabe"));
}

MOTOR_TEST_CASE(un_valor_que_parece_una_opcion_se_toma_como_valor) {
    // "--load --frames" es un nivel que se llama "--frames". Raro, pero es
    // lo que se escribio, y fallar aqui escondería el error de verdad.
    Linea l{"--load", "--frames"};
    const auto r = l.analiza();
    MOTOR_CHECK(r.isOk() && r.value().initial == "--frames");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
