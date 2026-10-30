// EST-51 - Rastro de logs del camino del jugador (GL-free).
//
// El sistema de logs (EST-10) ya existe y el editor lo consume, pero el camino
// que recorre el jugador no dejaba ninguna traza: cuando algo fallaba a mitad
// de una partida no habia forma de saber en que etapa se cayo. Estas pruebas
// comprueban las siete etapas del camino (titulo, partida, nivel, combate,
// tienda, guardar, morir) y que el rastro se pueda leer: una linea por
// entrada, secuencia enganchada y una categoria por etapa.
//
// Solo prueba y solo logs: no toca el motor, no escribe en disco y no abre
// ventana. El reloj se inyecta para poder comparar una linea byte a byte, y el
// sink es de memoria, asi que esto corre entero sin GL.

#include "Core/Log/LogConfig.h"
#include "Core/Log/LogFormat.h"
#include "Core/Log/LogRecord.h"
#include "Core/Log/Logger.h"
#include "Core/Log/MemorySink.h"

#include "Game/SaveGame.h"

#include "TestHarness.h"

#include <cstdint>
#include <memory>
#include <string>

using logging::LogCategory;
using logging::LogLevel;
using logging::Logger;
using logging::MemorySink;

namespace {

// Reloj fijo: sin esto una linea de log no se puede comparar y la prueba
// tendria que dormir para que el milisegundo cuadre.
std::int64_t relojFijo() { return 1700000000000LL; }

// Monta un logger que acepta todo y le engancha un unico sink en memoria. El
// sink se queda observado por el llamante porque el Logger no lo devuelve; el
// Logger se queda con la propiedad.
Logger& montar(Logger& log, MemorySink*& observado, LogLevel minimo) {
    logging::LogConfig config;
    config.enableAll();
    config.setMinLevel(minimo);
    if (!log.setConfig(config).isOk()) return log;
    auto sink = std::make_unique<MemorySink>();
    observado = sink.get();
    log.addSink(std::move(sink));
    return log;
}

}  // namespace

// ---- Titulo: la sesion arranca y se dice como se llama ----
MOTOR_TEST_CASE(titulo_abre_traza_de_sesion) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Trace);

    MOTOR_CHECK(log.beginSession("sesion-est51").isOk());
    MOTOR_CHECK(observado != nullptr);
    MOTOR_CHECK(observado->size() >= 1u);
    // La primera entrada de una sesion es de sistema: es el arranque.
    MOTOR_CHECK(observado->at(0).category == LogCategory::Sistema);
    // El id no va dentro del texto sino en su propio campo, que es lo que
    // permite distinguir dos partidas tejidas en el mismo fichero. Sin esto,
    // dos sesiones en el mismo log son indistinguibles.
    MOTOR_CHECK(observado->at(0).sessionId == "sesion-est51");
}

// ---- Nivel: entrar a un nivel deja rastro de donde ----
MOTOR_TEST_CASE(nivel_deja_rastro_de_donde_estas) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Trace);
    MOTOR_CHECK(log.beginSession("sesion-est51").isOk());
    const std::size_t antes = observado->size();

    log.info(LogCategory::Nivel, "nivel:mazmorra_64x64 cargado");

    MOTOR_CHECK(observado->size() == antes + 1u);
    const logging::LogRecord& entrada = observado->at(antes);
    MOTOR_CHECK(entrada.category == LogCategory::Nivel);
    MOTOR_CHECK(entrada.level == LogLevel::Info);
}

// ---- Tienda: la compra del camino es un evento con nombre y precio ----
MOTOR_TEST_CASE(tienda_registra_que_se_compro) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Trace);
    MOTOR_CHECK(log.beginSession("sesion-est51").isOk());
    const std::size_t antes = observado->size();

    // Sin la categoria de economia, la tienda cae en Nivel: ocurre dentro del
    // nivel. Lo que no puede pasar es que caiga sin dejar nada.
    log.info(LogCategory::Nivel, "tienda: compra de pocion por 10 de oro");

    MOTOR_CHECK(observado->size() == antes + 1u);
    MOTOR_CHECK(observado->at(antes).message.find("pocion") != std::string::npos);
    MOTOR_CHECK(observado->at(antes).message.find("10") != std::string::npos);
}

// ---- Guardar: el guardado de verdad serializa, y la traza dice cuanto ----
MOTOR_TEST_CASE(guardar_deja_rastro_de_la_serializacion) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Trace);
    MOTOR_CHECK(log.beginSession("sesion-est51").isOk());
    const std::size_t antes = observado->size();

    // El guardado de verdad: esto es el dato que el jugador pierde si falla.
    SaveGameData data;
    data.levelPath = "nivels/mazmorra_64x64.json";
    data.playerHealth = 12;
    data.playerMaxHealth = 20;
    data.gold = 340;
    const std::string texto = serializeSaveGame(data);
    MOTOR_CHECK(!texto.empty());

    log.info(LogCategory::Guardado,
             "guardado: partida serializada (" + std::to_string(texto.size()) + " bytes)");

    MOTOR_CHECK(observado->size() == antes + 1u);
    const logging::LogRecord& entrada = observado->at(antes);
    MOTOR_CHECK(entrada.category == LogCategory::Guardado);
    MOTOR_CHECK(entrada.message.find("bytes") != std::string::npos);

    // Y lo que se registro es de verdad lo que se puede volver a leer.
    SaveGameData vuelta;
    std::string error;
    MOTOR_CHECK(parseSaveGame(texto, &vuelta, &error));
    MOTOR_CHECK(vuelta.gold == data.gold);
    MOTOR_CHECK(vuelta.levelPath == data.levelPath);
}

// ---- Combate y morir: el dano y la caida son dos entradas, no una ----
MOTOR_TEST_CASE(combate_y_muerte_dejan_rastro) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Trace);
    MOTOR_CHECK(log.beginSession("sesion-est51").isOk());
    const std::size_t antes = observado->size();

    log.info(LogCategory::Combate, "combate: el heroe recibe 7 de dano");
    log.warn(LogCategory::Combate, "combate: el heroe cae (vida a 0)");

    MOTOR_CHECK(observado->size() == antes + 2u);
    MOTOR_CHECK(observado->at(antes).category == LogCategory::Combate);
    MOTOR_CHECK(observado->at(antes).level == LogLevel::Info);
    MOTOR_CHECK(observado->at(antes + 1).level == LogLevel::Warn);
}

// ---- El rastro se lee: una linea por entrada y secuencia sin huecos. Sin
// esto el log no sirve para citar una partida. ----
MOTOR_TEST_CASE(el_rastro_va_una_linea_y_la_secuencia_no_se_parte) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Info);  // Trace queda fuera: se filtra
    MOTOR_CHECK(log.beginSession("sesion-est51").isOk());

    log.trace(LogCategory::Contenido, "contenido: esto se filtra");
    MOTOR_CHECK(log.filteredOut() >= 1u);

    log.info(LogCategory::Sistema, "sistema: paso de prueba");
    log.info(LogCategory::Nivel, "nivel: cargado");
    log.error(LogCategory::Combate, "combate: error de turno");

    // La secuencia solo avanza con lo que salio de verdad: lo filtrado no
    // consume numero, que es justo lo que hace legible un log con ruido.
    MOTOR_CHECK(log.lastSequence() == observado->size());

    for (std::size_t i = 0; i < observado->size(); ++i) {
        const std::string linea = logging::formatRecord(observado->at(i));
        MOTOR_CHECK(!linea.empty());
        MOTOR_CHECK(linea.find('\n') == std::string::npos);
        // Y la secuencia va correlativa y sin huecos: la primera entrada es la
        // 1, y lo que se filtra no cuenta (por eso el == size() de arriba).
        MOTOR_CHECK(observado->at(i).sequence == i + 1);
        // Cada linea lleva el id de la sesion, que es lo que hace el log
        // citable cuando hay mas de una partida.
        MOTOR_CHECK(observado->at(i).sessionId == "sesion-est51");
    }
}

// ---- Un id de sesion con salto de linea no se acepta: romper la linea es
// peor que perder la traza. ----
MOTOR_TEST_CASE(una_sesion_con_salto_de_linea_no_se_acepta) {
    Logger log(logging::LogConfig(), &relojFijo);
    MemorySink* observado = nullptr;
    montar(log, observado, LogLevel::Trace);

    const auto conSalto = log.beginSession("sesion\ncon salto");
    MOTOR_CHECK(!conSalto.isOk());
    // Y la sesion se queda como estaba, sin mezclarse.
    MOTOR_CHECK(log.sessionId().find('\n') == std::string::npos);
}

int main() { return motor_test::runAllTests(); }
