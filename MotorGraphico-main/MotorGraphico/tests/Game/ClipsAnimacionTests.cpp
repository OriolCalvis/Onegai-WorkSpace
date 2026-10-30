// U3-05.E02 -- Lectura de clips de animacion en JSON (Game::leerClip). Sin GL.
// Los JSON se construyen en memoria; no hay assets.
#include "TestHarness.h"

#include "Game/ClipAnimacion.h"

#include <string>

using Game::ClipAnimacion;

namespace {

JsonValue json(const std::string& texto) {
    auto r = JsonValue::parse(texto);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

std::string doc(const std::string& clip, const std::string& version = "\"schemaVersion\":1,") {
    return "{" + version + "\"clips\":[" + clip + "]}";
}

const char* kTres =
    R"({"id":"golpe","bucle":false,
        "fotogramas":[{"sprite":13,"duracionMs":80},{"sprite":14,"duracionMs":60},{"sprite":15,"duracionMs":120}],
        "eventos":[{"id":"impacto","fotograma":1,"tipo":"logico","dato":"danio"},
                   {"id":"paso","fotograma":0},{"id":"paso","fotograma":2,"tipo":"sonido"}]})";

// Comprueba que la lectura falla y que el mensaje contiene "ruta".
// Si "motivo" no esta vacio, tambien debe aparecer en el mensaje.
void falla(const std::string& texto, const std::string& ruta, const std::string& motivo = "") {
    const auto r = Game::leerClip(json(texto));
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find(ruta) != std::string::npos);
    MOTOR_CHECK(motivo.empty() || r.errorMessage().find(motivo) != std::string::npos);
}

}  // namespace

MOTOR_TEST_CASE(clip_de_tres_fotogramas_con_eventos) {
    const auto r = Game::leerClip(json(doc(kTres)));
    MOTOR_CHECK(r.isOk());
    const ClipAnimacion& c = r.value();
    MOTOR_CHECK(c.id.value() == "golpe");
    MOTOR_CHECK(!c.bucle);
    MOTOR_CHECK(c.fotogramas.size() == 3);
    MOTOR_CHECK(c.fotogramas[1].sprite == 14 && c.fotogramas[1].duracionMs == 60);
    MOTOR_CHECK(c.duracionTotal() == 260.0);
    MOTOR_CHECK(c.eventos.size() == 3);
    MOTOR_CHECK(c.eventos[0].id == "impacto" && c.eventos[0].fotograma == 1);
    MOTOR_CHECK(c.eventos[0].tipo == "logico" && c.eventos[0].dato == "danio");
    MOTOR_CHECK(c.eventos[1].tipo == "logico" && c.eventos[1].dato.empty());  // defectos
    MOTOR_CHECK(c.eventos[2].orden == 2 && c.eventos[2].tipo == "sonido");
    MOTOR_CHECK(c.eventos[1].id == c.eventos[2].id);  // mismo id en fotogramas distintos: permitido
}

MOTOR_TEST_CASE(bucle_por_defecto_es_cierto_y_sin_eventos_vale) {
    const auto r = Game::leerClip(json(doc(R"({"id":"idle","fotogramas":[{"sprite":1,"duracionMs":100}]})")));
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().bucle);
    MOTOR_CHECK(r.value().eventos.empty());
}

MOTOR_TEST_CASE(catalogo_de_dos_clips) {
    const std::string clips = std::string(kTres) +
        R"(,{"id":"andar","fotogramas":[{"sprite":2,"duracionMs":70},{"sprite":3,"duracionMs":70}]})";
    const auto r = Game::leerCatalogoClips(json(doc(clips)));
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().size() == 2);
    MOTOR_CHECK(r.value()[0].id.value() == "golpe" && r.value()[1].id.value() == "andar");
    MOTOR_CHECK(!Game::leerClip(json(doc(clips))).isOk());  // leerClip exige uno solo
}

MOTOR_TEST_CASE(frontera_evento_en_el_ultimo_fotograma_y_duracion_uno) {
    const auto r = Game::leerClip(json(doc(
        R"({"id":"x","fotogramas":[{"sprite":1,"duracionMs":1},{"sprite":1,"duracionMs":1}],
            "eventos":[{"id":"fin","fotograma":1}]})")));
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().eventos[0].fotograma == 1);
    MOTOR_CHECK(r.value().duracionTotal() == 2.0);
}

MOTOR_TEST_CASE(error_fotogramas_ausentes_o_vacios) {
    falla(doc(R"({"id":"x"})"), "clips[0].fotogramas");
    falla(doc(R"({"id":"x","fotogramas":[]})"), "clips[0].fotogramas");
    falla(doc(R"({"id":"x","fotogramas":{}})"), "clips[0].fotogramas");
}

MOTOR_TEST_CASE(error_duracion_no_positiva_o_no_entera) {
    const std::string cab = R"({"id":"x","fotogramas":[{"sprite":1,"duracionMs":10},{"sprite":1,"duracionMs":)";
    const char* rd = "clips[0].fotogramas[1].duracionMs";
    falla(doc(cab + "0}]}"), rd, "debe ser > 0");
    falla(doc(cab + "-5}]}"), rd, "debe ser > 0");
    falla(doc(cab + "12.5}]}"), rd, "parte decimal");
    falla(doc(cab + "\"80\"}]}"), rd, "tipo incorrecto");
    falla(doc(cab + "null}]}"), rd, "tipo incorrecto");
    falla(doc(R"({"id":"x","fotogramas":[{"sprite":1}]})"), "clips[0].fotogramas[0].duracionMs", "ausente");
}

MOTOR_TEST_CASE(error_sprite_menor_que_uno) {
    falla(doc(R"({"id":"x","fotogramas":[{"sprite":0,"duracionMs":10}]})"),
          "clips[0].fotogramas[0].sprite", "debe ser >= 1");
    falla(doc(R"({"id":"x","fotogramas":[{"sprite":2,"duracionMs":10},{"sprite":-3,"duracionMs":10}]})"),
          "clips[0].fotogramas[1].sprite", "debe ser >= 1");
    falla(doc(R"({"id":"x","fotogramas":[{"duracionMs":10}]})"), "clips[0].fotogramas[0].sprite", "ausente");
    falla(doc(R"({"id":"x","fotogramas":[{"sprite":"2","duracionMs":10}]})"), "fotogramas[0].sprite", "tipo");
}

MOTOR_TEST_CASE(error_schema_version_ausente_o_distinto_de_uno) {
    const std::string clip = R"({"id":"x","fotogramas":[{"sprite":1,"duracionMs":10}]})";
    falla(doc(clip, ""), "schemaVersion");
    falla(doc(clip, "\"schemaVersion\":2,"), "schemaVersion");
    falla(doc(clip, "\"schemaVersion\":\"1\","), "schemaVersion");
}

MOTOR_TEST_CASE(error_id_de_clip_invalido_o_repetido) {
    const std::string fr = R"("fotogramas":[{"sprite":1,"duracionMs":10}])";
    falla(doc("{\"id\":\"Golpe!\"," + fr + "}"), "clips[0].id");
    falla(doc("{" + fr + "}"), "clips[0].id");
    const auto r = Game::leerCatalogoClips(json(doc("{\"id\":\"a\"," + fr + "},{\"id\":\"a\"," + fr + "}")));
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("clips[1].id") != std::string::npos);
}

MOTOR_TEST_CASE(error_evento_duplicado_id_y_fotograma) {
    falla(doc(R"({"id":"x","fotogramas":[{"sprite":1,"duracionMs":10},{"sprite":1,"duracionMs":10}],
                  "eventos":[{"id":"paso","fotograma":1},{"id":"otro","fotograma":1},{"id":"paso","fotograma":1}]})"),
          "clips[0].eventos[2]", "duplicado");
}

MOTOR_TEST_CASE(error_evento_fuera_del_clip_o_mal_formado) {
    const std::string cab = R"({"id":"x","fotogramas":[{"sprite":1,"duracionMs":10},{"sprite":1,"duracionMs":10},
                              {"sprite":1,"duracionMs":10}],"eventos":[{"id":"a","fotograma":0},)";
    falla(doc(cab + R"({"id":"b","fotograma":3}]})"), "clips[0].eventos[1].fotograma: fuera del clip (3 >= 3)");
    falla(doc(cab + R"({"id":"b","fotograma":-1}]})"), "clips[0].eventos[1].fotograma", "negativo");
    falla(doc(cab + R"({"id":"b","fotograma":1.5}]})"), "clips[0].eventos[1].fotograma", "no entero");
    falla(doc(cab + R"({"id":"b"}]})"), "clips[0].eventos[1].fotograma", "ausente");
    falla(doc(cab + R"({"id":"b","fotograma":null}]})"), "clips[0].eventos[1].fotograma", "tipo incorrecto");
    falla(doc(cab + R"({"id":"B B","fotograma":1}]})"), "clips[0].eventos[1].id");
}

MOTOR_TEST_CASE(error_enteros_fuera_de_rango_sin_comportamiento_indefinido) {
    const std::string cab = R"({"id":"x","fotogramas":[{"sprite":1,"duracionMs":)";
    const char* ruta = "clips[0].fotogramas[0].duracionMs";
    falla(doc(cab + "2147483648}]}"), ruta, "rango");
    falla(doc(cab + "-2147483649}]}"), ruta, "rango");
    falla(doc(cab + "1e300}]}"), ruta, "rango");
    falla(doc(cab + "1.5e300}]}"), ruta, "rango");
    falla(doc(cab + "123456789012.5}]}"), ruta, "no entero");
}

namespace {

// Clip de 4 fotogramas con los eventos dados (JSON de la lista "eventos").
ClipAnimacion clipConEventos(const std::string& eventos) {
    const std::string fr = R"({"sprite":1,"duracionMs":10})";
    const std::string clip = R"({"id":"c","fotogramas":[)" + fr + "," + fr + "," + fr + "," + fr +
                             R"(],"eventos":[)" + eventos + "]}";
    auto r = Game::leerClip(json(doc(clip)));
    MOTOR_CHECK(r.isOk());
    return r.value();
}

}  // namespace

MOTOR_TEST_CASE(eventos_desordenados_salen_ordenados_por_fotograma) {
    const auto c = clipConEventos(R"({"id":"d","fotograma":3},{"id":"b","fotograma":1},
                                    {"id":"a","fotograma":0},{"id":"c","fotograma":2})");
    const auto o = Game::eventosOrdenados(c);
    MOTOR_CHECK(o.size() == 4);
    for (std::size_t i = 0; i < o.size(); ++i) {
        MOTOR_CHECK(o[i].fotograma == static_cast<int>(i));
    }
    MOTOR_CHECK(o[0].id == "a" && o[1].id == "b" && o[2].id == "c" && o[3].id == "d");
    MOTOR_CHECK(c.eventos[0].id == "d");  // el clip original no se modifica
}

MOTOR_TEST_CASE(eventos_del_mismo_fotograma_conservan_orden_declarado) {
    const auto c = clipConEventos(R"({"id":"z","fotograma":2},{"id":"m","fotograma":1},
                                    {"id":"y","fotograma":2},{"id":"a","fotograma":1},
                                    {"id":"x","fotograma":2})");
    const auto o = Game::eventosOrdenados(c);
    MOTOR_CHECK(o.size() == 5);
    MOTOR_CHECK(o[0].id == "m" && o[1].id == "a");
    MOTOR_CHECK(o[2].id == "z" && o[3].id == "y" && o[4].id == "x");
    const auto f2 = Game::eventosDeFotograma(c, 2);
    MOTOR_CHECK(f2.size() == 3);
    MOTOR_CHECK(f2[0]->id == "z" && f2[1]->id == "y" && f2[2]->id == "x");
    MOTOR_CHECK(Game::eventosDeFotograma(c, 0).empty());
    MOTOR_CHECK(Game::eventosDeFotograma(c, 99).empty());
}

MOTOR_TEST_CASE(clip_sin_eventos_da_listas_vacias) {
    const auto c = clipConEventos("");
    MOTOR_CHECK(Game::eventosOrdenados(c).empty());
    MOTOR_CHECK(Game::eventosDeFotograma(c, 0).empty());
}

MOTOR_TEST_CASE(eventos_en_fotograma_cero_y_en_el_ultimo) {
    const auto c = clipConEventos(R"({"id":"fin","fotograma":3},{"id":"ini","fotograma":0})");
    const auto o = Game::eventosOrdenados(c);
    MOTOR_CHECK(o.size() == 2);
    MOTOR_CHECK(o[0].id == "ini" && o[0].fotograma == 0);
    MOTOR_CHECK(o[1].id == "fin" && o[1].fotograma == 3);
    MOTOR_CHECK(Game::eventosDeFotograma(c, 0).size() == 1 && Game::eventosDeFotograma(c, 3).size() == 1);
}

int main() { return motor_test::runAllTests(); }
