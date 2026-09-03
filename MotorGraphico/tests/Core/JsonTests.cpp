#include "Core/Json/JsonWriter.h"
#include "Core/Json/JsonValue.h"

#include "TestHarness.h"

#include <string>

namespace {

// C-10 — Round-trip: todo lo que escribe JsonWriter debe volver a leerse
// con JsonValue::parse sin perdidas. El caso duro son los caracteres de
// control (el escritor los escapa como \u00xx, que antes el parser
// rechazaba) y el UTF-8 no ASCII (el escritor lo copia tal cual).

MOTOR_TEST_CASE(writer_output_is_reread_without_loss) {
    JsonWriter writer;
    writer.beginObject();
    writer.member("control", std::string("a\x01" "b"));
    writer.member("acentos", std::string("Boundington — año 2000 b.f."));
    writer.member("comillas", std::string("dice \"hola\""));
    writer.member("barras", std::string("assets\\niveles\\boundington.json"));
    writer.member("saltos", std::string("linea1\nlinea2\tfin\r"));
    writer.key("numero");
    writer.value(42);
    writer.key("flotante");
    writer.value(1.5f);
    writer.key("cierto");
    writer.value(true);
    writer.endObject();

    const Result<JsonValue> parsed = JsonValue::parse(writer.str());
    MOTOR_CHECK(parsed.isOk());
    const JsonValue& root = parsed.value();
    MOTOR_CHECK(root["control"].asString() == std::string("a\x01" "b"));
    MOTOR_CHECK(root["acentos"].asString() == std::string("Boundington — año 2000 b.f."));
    MOTOR_CHECK(root["comillas"].asString() == std::string("dice \"hola\""));
    MOTOR_CHECK(root["barras"].asString() == std::string("assets\\niveles\\boundington.json"));
    MOTOR_CHECK(root["saltos"].asString() == std::string("linea1\nlinea2\tfin\r"));
    MOTOR_CHECK(root["numero"].asInt() == 42);
    MOTOR_CHECK(root["flotante"].asNumber() > 1.49 && root["flotante"].asNumber() < 1.51);
    MOTOR_CHECK(root["cierto"].asBool());
}

MOTOR_TEST_CASE(parser_decodes_basic_u_escapes_to_utf8) {
    const Result<JsonValue> parsed = JsonValue::parse("\"\\u0041\\u00e9\"");
    MOTOR_CHECK(parsed.isOk());
    MOTOR_CHECK(parsed.value().asString() == std::string("A\xC3\xA9"));
}

MOTOR_TEST_CASE(parser_combines_surrogate_pairs) {
    const Result<JsonValue> parsed = JsonValue::parse("\"\\uD83D\\uDE00\"");
    MOTOR_CHECK(parsed.isOk());
    MOTOR_CHECK(parsed.value().asString() == std::string("\xF0\x9F\x98\x80"));
}

MOTOR_TEST_CASE(parser_rejects_broken_u_escapes) {
    MOTOR_CHECK(!JsonValue::parse("\"\\uD83D\"").isOk());   // alto sin bajo
    MOTOR_CHECK(!JsonValue::parse("\"\\uDC00\"").isOk());   // bajo sin alto
    MOTOR_CHECK(!JsonValue::parse("\"\\uD83D\\u0041\"").isOk()); // bajo invalido
    MOTOR_CHECK(!JsonValue::parse("\"\\u00ZZ\"").isOk());   // digito no hexadecimal
    MOTOR_CHECK(!JsonValue::parse("\"\\u00\"").isOk());     // incompleto
}

}  // namespace

int main() { return motor_test::runAllTests(); }
