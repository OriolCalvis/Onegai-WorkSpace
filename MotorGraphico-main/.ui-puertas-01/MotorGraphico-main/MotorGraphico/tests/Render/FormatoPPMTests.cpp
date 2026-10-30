// EST-55 — El formato PPM, probado sin ventana.
//
// Es la mitad que se podia probar y no se probaba: estaba pegada al
// glReadPixels dentro de once copias, asi que comprobarla habria exigido
// un contexto OpenGL. Separada, se comprueba con un vector de bytes.

#include "Render/FormatoPPM.h"

#include "TestHarness.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

fs::path temporal(const char* nombre) {
    static int contador = 0;
    return fs::temp_directory_path() / (std::string(nombre) + std::to_string(++contador) + ".ppm");
}

std::string leeTodo(const fs::path& ruta) {
    std::ifstream in(ruta, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

// Dos filas de un pixel: la de abajo roja, la de arriba verde.
std::vector<unsigned char> dosFilas() {
    return {255, 0, 0,   0, 255, 0};
}

MOTOR_TEST_CASE(escribe_la_cabecera_que_manda_el_formato) {
    const auto ruta = temporal("ppm_cabecera");
    MOTOR_CHECK(Render::EscribePPM(ruta.string(), 1, 2, dosFilas(), false).isOk());
    const auto texto = leeTodo(ruta);
    MOTOR_CHECK(texto.rfind("P6\n1 2\n255\n", 0) == 0);
    MOTOR_CHECK(texto.size() == 11 + 6);
    fs::remove(ruta);
}

MOTOR_TEST_CASE(voltear_las_filas_es_lo_que_cinco_copias_no_hacian) {
    // glReadPixels da el origen ABAJO; el PPM lo quiere ARRIBA. Con
    // origenAbajo=true, la primera fila del fichero tiene que ser la
    // ULTIMA del buffer.
    const auto derecha = temporal("ppm_derecha");
    MOTOR_CHECK(Render::EscribePPM(derecha.string(), 1, 2, dosFilas(), true).isOk());
    const auto conVolteo = leeTodo(derecha);
    MOTOR_CHECK(static_cast<unsigned char>(conVolteo[11]) == 0);    // R del verde
    MOTOR_CHECK(static_cast<unsigned char>(conVolteo[12]) == 255);  // G del verde

    const auto tal = temporal("ppm_tal_cual");
    MOTOR_CHECK(Render::EscribePPM(tal.string(), 1, 2, dosFilas(), false).isOk());
    const auto sinVolteo = leeTodo(tal);
    MOTOR_CHECK(static_cast<unsigned char>(sinVolteo[11]) == 255);  // R del rojo
    MOTOR_CHECK(static_cast<unsigned char>(sinVolteo[12]) == 0);

    MOTOR_CHECK(conVolteo != sinVolteo);
    fs::remove(derecha);
    fs::remove(tal);
}

MOTOR_TEST_CASE(un_buffer_corto_no_escribe_un_fichero_a_medias) {
    // Escribir menos bytes de los que anuncia la cabecera deja un fichero
    // que unos visores abren a medias y otros rechazan.
    const auto ruta = temporal("ppm_corto");
    const auto r = Render::EscribePPM(ruta.string(), 4, 4, dosFilas(), true);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("faltan pixeles") != std::string::npos);
    MOTOR_CHECK(!fs::exists(ruta));
}

MOTOR_TEST_CASE(un_tamano_absurdo_se_dice_en_vez_de_petar) {
    MOTOR_CHECK(!Render::EscribePPM("da_igual.ppm", 0, 4, dosFilas(), true).isOk());
    MOTOR_CHECK(!Render::EscribePPM("da_igual.ppm", 4, -1, dosFilas(), true).isOk());
    MOTOR_CHECK(!fs::exists("da_igual.ppm"));
}

MOTOR_TEST_CASE(una_ruta_que_no_se_puede_abrir_devuelve_error) {
    // Antes esto se tragaba en silencio en diez de las once copias: el
    // ofstream fallaba y la funcion volvia como si nada.
    const auto r = Render::EscribePPM("/no/existe/esta/carpeta/x.ppm", 1, 2, dosFilas(), true);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("no se pudo abrir") != std::string::npos);
}

MOTOR_TEST_CASE(sobran_bytes_no_es_un_problema) {
    // Un buffer mas grande de lo necesario (por ejemplo reutilizado entre
    // capturas de distinto tamano) se recorta, no se rechaza.
    std::vector<unsigned char> grande(3 * 100, 7);
    const auto ruta = temporal("ppm_grande");
    MOTOR_CHECK(Render::EscribePPM(ruta.string(), 1, 2, grande, true).isOk());
    MOTOR_CHECK(leeTodo(ruta).size() == 11 + 6);
    fs::remove(ruta);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
